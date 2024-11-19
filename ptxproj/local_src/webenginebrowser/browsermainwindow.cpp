/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browsermainwindow.h"

#include "autosaver.h"
#include "bookmarks.h"
#include "browserapplication.h"
#include "chasewidget.h"
#include "downloadmanager.h"
#include "history.h"
#include "printtopdfdialog.h"
#include "settings.h"
#include "tabwidget.h"
#include "toolbarsearch.h"
#include "ui_passworddialog.h"
#include "webview.h"

#include <QtCore/QSettings>

#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPlainTextEdit>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QMenuBar>
//#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QInputDialog>

#include <QWebEngineHistory>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QMessageBox>
#include <QtCore/QDebug>

#include <QAbstractSocket>

#include <QThread>
#include <QProcess>
#include <QX11Info>
#include "cmdthread.h"
#include "globals.h"
#include "math.h"

#define MAX_TAB_COUNT           10
#define MIN_RECONNECT_INTERVAL   5
#define MAX_RECONNECT_INTERVAL  99

template<typename Arg, typename R, typename C>
struct InvokeWrapper
{
  R *receiver;
  void (C::*memberFun)(Arg);
  void operator()(Arg result)
  {
    (receiver->*memberFun)(result);
  }
};

template<typename Arg, typename R, typename C>
InvokeWrapper<Arg, R, C> invoke(R *receiver, void (C::*memberFun)(Arg))
{
  InvokeWrapper<Arg, R, C> wrapper = {receiver, memberFun};
  return wrapper;
}

const char *BrowserMainWindow::defaultHome = "https://127.0.0.1/wbm/index.html";

BrowserMainWindow::BrowserMainWindow(QWidget *parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags)
  , m_tabWidget(new TabWidget(this))
  , m_autoSaver(new AutoSaver(this))
  , m_historyBack(0)
  , m_historyForward(0)
  , m_stop(0)
  , m_reload(0)
  , m_currentPrinter(nullptr)
{
  //remove this to avoid black rectangle behind cursor
  //removed setWindowFlags(Qt::FramelessWindowHint);
  //calling setFixedSize makes the fullscreen
  setWindowFlags(Qt::FramelessWindowHint);

  //QTextCodec *codec = QTextCodec::codecForLocale();  
  //if (codec)
  //{
  //  qDebug() << "DEBUG: "  << codec->name();
  //}    
  
  QRect mainScreenSize = QApplication::desktop()->screenGeometry();
  setFixedSize(mainScreenSize.width(), mainScreenSize.height());
  
  //qDebug() << "Desktop width: " << mainScreenSize.width() << " height: " << mainScreenSize.height();
  //qDebug() << "MainWindow width: " << width() << " height: " << height();
    
  pCmdThread = NULL;

  ReadWebengineConfigurationFile(CONF_FILE_WEBENGINE);
  ReadPlcselectConfigFile(CONF_FILE_PLCSELECT);

  setToolButtonStyle(Qt::ToolButtonFollowStyle);
  setAttribute(Qt::WA_DeleteOnClose, true);

  //setAttribute(Qt::WA_AcceptTouchEvents, true);

  //disable statusbar
  setStatusBar(0);

  setupMenu();
  setupToolBar();

  QWidget *centralWidget = new QWidget(this);
  BookmarksModel *bookmarksModel = BrowserApplication::bookmarksManager()->bookmarksModel();
  m_bookmarksToolbar = new BookmarksToolBar(bookmarksModel, this);
  connect(m_bookmarksToolbar, SIGNAL(openUrl(QUrl)),
          m_tabWidget, SLOT(loadUrlInCurrentTab(QUrl)));
  connect(m_bookmarksToolbar->toggleViewAction(), SIGNAL(toggled(bool)),
          this, SLOT(updateBookmarksToolbarActionText(bool)));

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setSpacing(0);
  layout->setMargin(0);
#if defined(Q_OS_OSX)
  layout->addWidget(m_bookmarksToolbar);
  layout->addWidget(new QWidget); // <- OS X tab widget style bug
#else
  addToolBarBreak();
  addToolBar(m_bookmarksToolbar);
#endif
  layout->addWidget(m_tabWidget);
  centralWidget->setLayout(layout);
  setCentralWidget(centralWidget);

  connect(m_tabWidget, SIGNAL(loadPage(QString)),
          this, SLOT(loadPage(QString)));
  connect(m_tabWidget, SIGNAL(setCurrentTitle(QString)),
          this, SLOT(slotUpdateWindowTitle(QString)));

  connect(m_tabWidget, SIGNAL(loadProgress(int)),
          this, SLOT(slotLoadProgress(int)));
  connect(m_tabWidget, SIGNAL(tabsChanged()),
          m_autoSaver, SLOT(changeOccurred()));
  connect(m_tabWidget, SIGNAL(geometryChangeRequested(QRect)),
          this, SLOT(geometryChangeRequested(QRect)));
#if defined(QWEBENGINEPAGE_PRINTREQUESTED)
  connect(m_tabWidget, SIGNAL(printRequested(QWebEngineFrame*)),
          this, SLOT(printRequested(QWebEngineFrame*)));
#endif
  connect(m_tabWidget, SIGNAL(menuBarVisibilityChangeRequested(bool)),
          menuBar(), SLOT(setVisible(bool)));

  connect(m_tabWidget, SIGNAL(toolBarVisibilityChangeRequested(bool)),
          m_navigationBar, SLOT(setVisible(bool)));
  connect(m_tabWidget, SIGNAL(toolBarVisibilityChangeRequested(bool)),
          m_bookmarksToolbar, SLOT(setVisible(bool)));
#if defined(Q_OS_OSX)
  connect(m_tabWidget, SIGNAL(lastTabClosed()),
          this, SLOT(close()));
#else
  connect(m_tabWidget, SIGNAL(lastTabClosed()),
          m_tabWidget, SLOT(newTab()));
#endif

  slotUpdateWindowTitle();
  loadDefaultState();
  m_tabWidget->newTab();

  int size = m_tabWidget->lineEditStack()->sizeHint().height();
  m_navigationBar->setIconSize(QSize(size, size));

  //problem with messageboxes here: setWindowState(this->windowState() | Qt::WindowFullScreen);
  setWindowState(windowState() | Qt::WindowNoState); 

  if (g_webengine.bNavbar == false)
    m_navigationBar->hide();

  if (g_webengine.bMenubar == false)
  {
    QMenuBar * pMenuBar = menuBar();
    if (pMenuBar)
    {
      pMenuBar->hide();
    }
  }

  if (g_webengine.bTabbar == false)
  {
    QTabBar *tabBar = m_tabWidget->findChild<QTabBar *>();
    if (tabBar)
    {
      tabBar->hide();
    }
  }

  if (g_webengine.bBookmarkbar == false)
    m_bookmarksToolbar->hide();
  
  startCmdThread();
  
  InitScrollGestures();  

}

BrowserMainWindow::~BrowserMainWindow()
{
  m_autoSaver->changeOccurred();
  m_autoSaver->saveIfNeccessary();
}

void BrowserMainWindow::InitScrollGestures()
{
  //cat /proc/bus/input/devices | grep "PIXCIR HID Touch Panel"

  bool bCapTouch = false;
  // never use this type of scrolling with a capacitive touch controller
  QFile file("/proc/bus/input/devices");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
     QTextStream in(&file);     
     QString sTxt = in.readAll();
     if (sTxt.contains("PIXCIR HID Touch Panel"))
     {
       bCapTouch = true;
     }
     file.close();
  }  
  
  if (bCapTouch)
  {
      //qDebug() << "---> PIXCIR HID Touch Panel";
      return;
  }

  
// gestures
QSettings settings(CONF_FILE_GESTURE, QSettings::IniFormat);
const QStringList allKeys = settings.allKeys();
bool bGesture = false;
bool bScroll = false;
foreach (const QString &key, allKeys)
{
    QString var = settings.value(key).toString();

    if (var.length() > 0)
    {
      if (key == "state")
      {
          if (var.compare("enabled", Qt::CaseInsensitive) == 0)
          {
              bGesture = true;
          }
      }
      else if (key == "scroll")
      {
          if (var.compare("yes", Qt::CaseInsensitive) == 0)
          {
              bScroll = true;
          }
      }        
      else if (key == "scroll_wheel_value")
      {
        iScrollWheelValue = var.toInt();
        //qDebug() << "iScrollWheelValue " << iScrollWheelValue;
      }
    }
}

if (bScroll && bGesture)
{
    WebView * webview = tabWidget()->currentWebView();
    webview->grabGesture(Qt::SwipeGesture);
    webview->setFocus();
    pRecognizer = new CustomSwipeGesture(this);
    //qDebug() << "pRecognizer " << pRecognizer;
    if (pRecognizer)
    {

      SwipeGestureType = CustomSwipeGesture::registerRecognizer(pRecognizer);

      connect(pRecognizer,
              SIGNAL(signalScrollVUp(QPoint)),
              this,
              SLOT(scrollSlotVUp(QPoint)) );

      connect(pRecognizer,
              SIGNAL(signalScrollVDown(QPoint)),
              this,
              SLOT(scrollSlotVDown(QPoint)) );

      connect(pRecognizer,
              SIGNAL(signalScrollHLeft(QPoint)),
              this,
              SLOT(scrollSlotHLeft(QPoint)) );

      connect(pRecognizer,
              SIGNAL(signalScrollHRight(QPoint)),
              this,
              SLOT(scrollSlotHRight(QPoint)) );

      connect(pRecognizer,
              SIGNAL(signalExecuteJsScrollbarVertCode()),
              this,
              SLOT(slotExecuteJsScrollbarVertCode()) );

      connect(pRecognizer,
              SIGNAL(signalExecuteJsScrollbarHorizCode()),
              this,
              SLOT(slotExecuteJsScrollbarHorizCode()) );
    }
}

}

void BrowserMainWindow::loadDefaultState()
{
  QSettings settings;
  settings.beginGroup(QLatin1String("BrowserMainWindow"));
  QByteArray data = settings.value(QLatin1String("defaultState")).toByteArray();
  restoreState(data);
  settings.endGroup();
}

QSize BrowserMainWindow::sizeHint() const
{
  QRect desktopRect = QApplication::desktop()->screenGeometry();
  QSize size = desktopRect.size() * qreal(0.9);
  return size;
}

void BrowserMainWindow::save()
{
  BrowserApplication::instance()->saveSession();

  QSettings settings;
  settings.beginGroup(QLatin1String("BrowserMainWindow"));
  QByteArray data = saveState(false);
  settings.setValue(QLatin1String("defaultState"), data);
  settings.endGroup();
}

static const qint32 BrowserMainWindowMagic = 0xba;

QByteArray BrowserMainWindow::saveState(bool withTabs) const
{
  int version = 2;
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);

  stream << qint32(BrowserMainWindowMagic);
  stream << qint32(version);

  stream << size();
  stream << !m_navigationBar->isHidden();
  stream << !m_bookmarksToolbar->isHidden();

  if (withTabs)
    stream << tabWidget()->saveState();
  else
    stream << QByteArray();
  return data;
}

bool BrowserMainWindow::restoreState(const QByteArray &state)
{
  int version = 2;
  QByteArray sd = state;
  QDataStream stream(&sd, QIODevice::ReadOnly);
  if (stream.atEnd())
    return false;

  qint32 marker;
  qint32 v;
  stream >> marker;
  stream >> v;
  if (marker != BrowserMainWindowMagic || v != version)
    return false;

  QSize size;
  bool showToolbar;
  bool showBookmarksBar;
  QByteArray tabState;

  stream >> size;
  stream >> showToolbar;
  stream >> showBookmarksBar;
  stream >> tabState;

  showBookmarksBar = g_webengine.bBookmarkbar;
  showToolbar = g_webengine.bNavbar;

  resize(size);

  m_navigationBar->setVisible(showToolbar);
  updateToolbarActionText(showToolbar);

  m_bookmarksToolbar->setVisible(showBookmarksBar);
  updateBookmarksToolbarActionText(showBookmarksBar);

  if (!tabWidget()->restoreState(tabState))
    return false;

  return true;
}

void BrowserMainWindow::runScriptOnOpenViews(const QString &source)
{
  for (int i =0; i < tabWidget()->count(); ++i)
    tabWidget()->webView(i)->page()->runJavaScript(source);
}

void BrowserMainWindow::setupMenu()
{
  new QShortcut(QKeySequence(Qt::Key_F6), this, SLOT(slotSwapFocus()));

  // File
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

  fileMenu->addAction(tr("&New Window"), this, SLOT(slotFileNew()), QKeySequence::New);
  fileMenu->addAction(m_tabWidget->newTabAction());
  fileMenu->addAction(tr("&Open File..."), this, SLOT(slotFileOpen()), QKeySequence::Open);
  fileMenu->addAction(tr("Open &Location..."), this,
                      SLOT(slotSelectLineEdit()), QKeySequence(Qt::ControlModifier + Qt::Key_L));
  fileMenu->addSeparator();
  fileMenu->addAction(m_tabWidget->closeTabAction());
  fileMenu->addSeparator();
#if defined(QWEBENGINE_SAVE_AS_FILE)
  fileMenu->addAction(tr("&Save As..."), this,
                      SLOT(slotFileSaveAs()), QKeySequence(QKeySequence::Save));
  fileMenu->addSeparator();
#endif
  BookmarksManager *bookmarksManager = BrowserApplication::bookmarksManager();
  fileMenu->addAction(tr("&Import Bookmarks..."), bookmarksManager, SLOT(importBookmarks()));
  fileMenu->addAction(tr("&Export Bookmarks..."), bookmarksManager, SLOT(exportBookmarks()));
  fileMenu->addSeparator();
#if defined(QWEBENGINEPAGE_PRINT)
  fileMenu->addAction(tr("P&rint Preview..."), this, SLOT(slotFilePrintPreview()));
#endif
  fileMenu->addAction(tr("&Print..."), this, SLOT(slotFilePrint()), QKeySequence::Print);
  fileMenu->addAction(tr("&Print to PDF..."), this, SLOT(slotFilePrintToPDF()));
  fileMenu->addSeparator();

  QAction *action = fileMenu->addAction(tr("Private &Browsing..."), this, SLOT(slotPrivateBrowsing()));
  action->setCheckable(true);
  action->setChecked(BrowserApplication::instance()->privateBrowsing());
  connect(BrowserApplication::instance(), SIGNAL(privateBrowsingChanged(bool)), action, SLOT(setChecked(bool)));
  fileMenu->addSeparator();

#if defined(Q_OS_OSX)
  fileMenu->addAction(tr("&Quit"), BrowserApplication::instance(), SLOT(quitBrowser()),
                      QKeySequence(Qt::CTRL | Qt::Key_Q));
#else
  fileMenu->addAction(tr("&Quit"), this, SLOT(close()), QKeySequence(Qt::CTRL | Qt::Key_Q));
#endif

  // Edit
  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
  QAction *m_undo = editMenu->addAction(tr("&Undo"));
  m_undo->setShortcuts(QKeySequence::Undo);
  m_tabWidget->addWebAction(m_undo, QWebEnginePage::Undo);
  QAction *m_redo = editMenu->addAction(tr("&Redo"));
  m_redo->setShortcuts(QKeySequence::Redo);
  m_tabWidget->addWebAction(m_redo, QWebEnginePage::Redo);
  editMenu->addSeparator();
  QAction *m_cut = editMenu->addAction(tr("Cu&t"));
  m_cut->setShortcuts(QKeySequence::Cut);
  m_tabWidget->addWebAction(m_cut, QWebEnginePage::Cut);
  QAction *m_copy = editMenu->addAction(tr("&Copy"));
  m_copy->setShortcuts(QKeySequence::Copy);
  m_tabWidget->addWebAction(m_copy, QWebEnginePage::Copy);
  QAction *m_paste = editMenu->addAction(tr("&Paste"));
  m_paste->setShortcuts(QKeySequence::Paste);
  m_tabWidget->addWebAction(m_paste, QWebEnginePage::Paste);
  editMenu->addSeparator();

  QAction *m_find = editMenu->addAction(tr("&Find"));
  m_find->setShortcuts(QKeySequence::Find);
  connect(m_find, SIGNAL(triggered()), this, SLOT(slotEditFind()));

  QAction *m_findNext = editMenu->addAction(tr("&Find Next"));
  m_findNext->setShortcuts(QKeySequence::FindNext);
  connect(m_findNext, SIGNAL(triggered()), this, SLOT(slotEditFindNext()));

  QAction *m_findPrevious = editMenu->addAction(tr("&Find Previous"));
  m_findPrevious->setShortcuts(QKeySequence::FindPrevious);
  connect(m_findPrevious, SIGNAL(triggered()), this, SLOT(slotEditFindPrevious()));
  editMenu->addSeparator();

  editMenu->addAction(tr("&Preferences"), this, SLOT(slotPreferences()), tr("Ctrl+,"));

  // View
  QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

  m_viewBookmarkBar = new QAction(this);
  updateBookmarksToolbarActionText(true);
  m_viewBookmarkBar->setShortcut(tr("Shift+Ctrl+B"));
  connect(m_viewBookmarkBar, SIGNAL(triggered()), this, SLOT(slotViewBookmarksBar()));
  viewMenu->addAction(m_viewBookmarkBar);

  m_viewToolbar = new QAction(this);
  updateToolbarActionText(true);
  m_viewToolbar->setShortcut(tr("Ctrl+|"));
  connect(m_viewToolbar, SIGNAL(triggered()), this, SLOT(slotViewToolbar()));
  viewMenu->addAction(m_viewToolbar);

  viewMenu->addSeparator();

  m_stop = viewMenu->addAction(tr("&Stop"));
  QList<QKeySequence> shortcuts;
  shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Period));
  shortcuts.append(Qt::Key_Escape);
  m_stop->setShortcuts(shortcuts);
  m_tabWidget->addWebAction(m_stop, QWebEnginePage::Stop);

  m_reload = viewMenu->addAction(tr("Reload Page"));
  m_reload->setShortcuts(QKeySequence::Refresh);
  m_tabWidget->addWebAction(m_reload, QWebEnginePage::Reload);

  viewMenu->addAction(tr("Zoom &In"), this, SLOT(slotViewZoomIn()), QKeySequence(Qt::CTRL | Qt::Key_Plus));
  viewMenu->addAction(tr("Zoom &Out"), this, SLOT(slotViewZoomOut()), QKeySequence(Qt::CTRL | Qt::Key_Minus));
  viewMenu->addAction(tr("Reset &Zoom"), this, SLOT(slotViewResetZoom()), QKeySequence(Qt::CTRL | Qt::Key_0));

  viewMenu->addSeparator();
  QAction *m_pageSource = viewMenu->addAction(tr("Page S&ource"));
  m_pageSource->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_U));
  m_tabWidget->addWebAction(m_pageSource, QWebEnginePage::ViewSource);

  // History
  HistoryMenu *historyMenu = new HistoryMenu(this);
  connect(historyMenu, SIGNAL(openUrl(QUrl)),
          m_tabWidget, SLOT(loadUrlInCurrentTab(QUrl)));
  historyMenu->setTitle(tr("Hi&story"));
  menuBar()->addMenu(historyMenu);
  QList<QAction*> historyActions;

  m_historyBack = new QAction(tr("Back"), this);
  m_tabWidget->addWebAction(m_historyBack, QWebEnginePage::Back);
  QList<QKeySequence> backShortcuts = QKeySequence::keyBindings(QKeySequence::Back);
  for (auto it = backShortcuts.begin(); it != backShortcuts.end();)
  {
    // Chromium already handles navigate on backspace when appropriate.
    if ((*it)[0] == Qt::Key_Backspace)
      it = backShortcuts.erase(it);
    else
      ++it;
  }
  // For some reason Qt doesn't bind the dedicated Back key to Back.
  backShortcuts.append(QKeySequence(Qt::Key_Back));
  m_historyBack->setShortcuts(backShortcuts);
  m_historyBack->setIconVisibleInMenu(false);
  historyActions.append(m_historyBack);

  m_historyForward = new QAction(tr("Forward"), this);
  m_tabWidget->addWebAction(m_historyForward, QWebEnginePage::Forward);
  QList<QKeySequence> fwdShortcuts = QKeySequence::keyBindings(QKeySequence::Forward);
  for (auto it = fwdShortcuts.begin(); it != fwdShortcuts.end();)
  {
    if (((*it)[0] & Qt::Key_unknown) == Qt::Key_Backspace)
      it = fwdShortcuts.erase(it);
    else
      ++it;
  }
  fwdShortcuts.append(QKeySequence(Qt::Key_Forward));
  m_historyForward->setShortcuts(fwdShortcuts);
  m_historyForward->setIconVisibleInMenu(false);
  historyActions.append(m_historyForward);

  QAction *m_historyHome = new QAction(tr("Home"), this);
  connect(m_historyHome, SIGNAL(triggered()), this, SLOT(slotHome()));
  m_historyHome->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
  historyActions.append(m_historyHome);

#if defined(QWEBENGINEHISTORY_RESTORESESSION)
  m_restoreLastSession = new QAction(tr("Restore Last Session"), this);
  connect(m_restoreLastSession, SIGNAL(triggered()), BrowserApplication::instance(), SLOT(restoreLastSession()));
  m_restoreLastSession->setEnabled(BrowserApplication::instance()->canRestoreSession());
  historyActions.append(m_tabWidget->recentlyClosedTabsAction());
  historyActions.append(m_restoreLastSession);
#endif

  historyMenu->setInitialActions(historyActions);

  // Bookmarks
  BookmarksMenu *bookmarksMenu = new BookmarksMenu(this);
  connect(bookmarksMenu, SIGNAL(openUrl(QUrl)),
          m_tabWidget, SLOT(loadUrlInCurrentTab(QUrl)));

  bookmarksMenu->setTitle(tr("&Bookmarks"));
  menuBar()->addMenu(bookmarksMenu);

  QList<QAction*> bookmarksActions;

  QAction *showAllBookmarksAction = new QAction(tr("Show All Bookmarks"), this);
  connect(showAllBookmarksAction, SIGNAL(triggered()), this, SLOT(slotShowBookmarksDialog()));
  m_addBookmark = new QAction(QIcon(QLatin1String(":addbookmark.png")), tr("Add Bookmark..."), this);
  m_addBookmark->setIconVisibleInMenu(false);

  connect(m_addBookmark, SIGNAL(triggered()), this, SLOT(slotAddBookmark()));
  m_addBookmark->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));

  bookmarksActions.append(showAllBookmarksAction);
  bookmarksActions.append(m_addBookmark);
  bookmarksMenu->setInitialActions(bookmarksActions);

  // Window
  m_windowMenu = menuBar()->addMenu(tr("&Window"));
  connect(m_windowMenu, SIGNAL(aboutToShow()),
          this, SLOT(slotAboutToShowWindowMenu()));
  slotAboutToShowWindowMenu();

  QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
#if defined(QWEBENGINEINSPECTOR)
  QAction * a = toolsMenu->addAction(tr("Enable Web &Inspector"), this, SLOT(slotToggleInspector(bool)));
  a->setCheckable(true);
#endif

  
}

void BrowserMainWindow::setupToolBar()
{
  m_navigationBar = addToolBar(tr("Navigation"));
  connect(m_navigationBar->toggleViewAction(), SIGNAL(toggled(bool)),
          this, SLOT(updateToolbarActionText(bool)));

  m_historyBack->setIcon(style()->standardIcon(QStyle::SP_ArrowBack, 0, this));
  m_historyBackMenu = new QMenu(this);
  m_historyBack->setMenu(m_historyBackMenu);
  connect(m_historyBackMenu, SIGNAL(aboutToShow()),
          this, SLOT(slotAboutToShowBackMenu()));
  connect(m_historyBackMenu, SIGNAL(triggered(QAction*)),
          this, SLOT(slotOpenActionUrl(QAction*)));
  m_navigationBar->addAction(m_historyBack);

  m_historyForward->setIcon(style()->standardIcon(QStyle::SP_ArrowForward, 0, this));
  m_historyForwardMenu = new QMenu(this);
  connect(m_historyForwardMenu, SIGNAL(aboutToShow()),
          this, SLOT(slotAboutToShowForwardMenu()));
  connect(m_historyForwardMenu, SIGNAL(triggered(QAction*)),
          this, SLOT(slotOpenActionUrl(QAction*)));
  m_historyForward->setMenu(m_historyForwardMenu);
  m_navigationBar->addAction(m_historyForward);

  m_stopReload = new QAction(this);
  m_reloadIcon = style()->standardIcon(QStyle::SP_BrowserReload);
  m_stopReload->setIcon(m_reloadIcon);

  m_navigationBar->addAction(m_stopReload);

  m_navigationBar->addWidget(m_tabWidget->lineEditStack());

  //m_toolbarSearch = new ToolbarSearch(m_navigationBar);
  //m_navigationBar->addWidget(m_toolbarSearch);
  //connect(m_toolbarSearch, SIGNAL(search(QUrl)), SLOT(loadUrl(QUrl)));

  m_chaseWidget = new ChaseWidget(this);
  m_navigationBar->addWidget(m_chaseWidget);
}

void BrowserMainWindow::slotShowBookmarksDialog()
{
  BookmarksDialog *dialog = new BookmarksDialog(this);
  connect(dialog, SIGNAL(openUrl(QUrl)),
          m_tabWidget, SLOT(loadUrlInCurrentTab(QUrl)));
  dialog->show();
}

void BrowserMainWindow::slotAddBookmark()
{
  WebView *webView = currentTab();
  QString url = webView->url().toString();
  QString title = webView->title();
  AddBookmarkDialog dialog(url, title);
  dialog.exec();
}

void BrowserMainWindow::slotViewToolbar()
{
  if (m_navigationBar->isVisible())
  {
    updateToolbarActionText(false);
    m_navigationBar->close();
  }
  else
  {
    updateToolbarActionText(true);
    m_navigationBar->show();
  }
  m_autoSaver->changeOccurred();
}

void BrowserMainWindow::slotViewBookmarksBar()
{
  if (m_bookmarksToolbar->isVisible())
  {
    updateBookmarksToolbarActionText(false);
    m_bookmarksToolbar->close();
  }
  else
  {
    updateBookmarksToolbarActionText(true);
    m_bookmarksToolbar->show();
  }
  m_autoSaver->changeOccurred();
}

void BrowserMainWindow::handleFindTextResult(bool found)
{

}

void BrowserMainWindow::updateToolbarActionText(bool visible)
{
  m_viewToolbar->setText(!visible ? tr("Show Toolbar") : tr("Hide Toolbar"));
}

void BrowserMainWindow::updateBookmarksToolbarActionText(bool visible)
{
  m_viewBookmarkBar->setText(!visible ? tr("Show Bookmarks bar") : tr("Hide Bookmarks bar"));
}

void BrowserMainWindow::loadUrl(const QUrl &url)
{
  if (!currentTab() || !url.isValid())
    return;

  WebView * pView = m_tabWidget->currentWebView();
  if (pView)
  {
    if (pView->m_pTimerReconnect)
      pView->m_pTimerReconnect->stop();

    pView->stop();
  }

  m_tabWidget->currentLineEdit()->setText(QString::fromUtf8(url.toEncoded()));
  m_tabWidget->loadUrlInCurrentTab(url);
}

void BrowserMainWindow::slotDownloadManager()
{
  BrowserApplication::downloadManager()->show();
}

void BrowserMainWindow::slotSelectLineEdit()
{
  m_tabWidget->currentLineEdit()->selectAll();
  m_tabWidget->currentLineEdit()->setFocus();
}

void BrowserMainWindow::slotFileSaveAs()
{
  // not implemented yet.
}

void BrowserMainWindow::slotPreferences()
{
  SettingsDialog *s = new SettingsDialog(this);
  s->show();
}


void BrowserMainWindow::slotUpdateWindowTitle(const QString &title)
{
  if (title.isEmpty())
  {
    setWindowTitle(tr("Webenginebrowser"));
  }
  else
  {
#if defined(Q_OS_OSX)
    setWindowTitle(title);
#else
    setWindowTitle(tr("%1 - Webenginebrowser", "Page title and Browser name").arg(title));
#endif
  }
}

void BrowserMainWindow::slotFileNew()
{
  BrowserApplication::instance()->newMainWindow();
  BrowserMainWindow *mw = BrowserApplication::instance()->mainWindow();
  mw->slotHome();
}

void BrowserMainWindow::slotFileOpen()
{
  QString file = QFileDialog::getOpenFileName(this, tr("Open Web Resource"), QString(),
                 tr("Web Resources (*.html *.htm *.svg *.png *.gif *.svgz);;All files (*.*)"));

  if (file.isEmpty())
    return;

  loadPage(file);
}

void BrowserMainWindow::slotFilePrintPreview()
{
#ifndef QT_NO_PRINTPREVIEWDIALOG
  if (!currentTab())
    return;
  QPrintPreviewDialog *dialog = new QPrintPreviewDialog(this);
  connect(dialog, SIGNAL(paintRequested(QPrinter*)),
          currentTab(), SLOT(print(QPrinter*)));
  dialog->exec();
#endif
}

void BrowserMainWindow::slotFilePrint()
{
  if (!currentTab())
    return;
  printRequested(currentTab()->page());
}

void BrowserMainWindow::slotHandlePdfPrinted(const QByteArray& result)
{
  if (!result.size())
    return;

  QFile file(m_printerOutputFileName);

  m_printerOutputFileName.clear();
  if (!file.open(QFile::WriteOnly))
    return;

  file.write(result.data(), result.size());
  file.close();
}

void BrowserMainWindow::slotFilePrintToPDF()
{
  if (!currentTab() || !m_printerOutputFileName.isEmpty())
    return;

  QFileInfo info(QStringLiteral("printout.pdf"));
  PrintToPdfDialog *dialog = new PrintToPdfDialog(info.absoluteFilePath(), this);
  dialog->setWindowTitle(tr("Print to PDF"));
  if (dialog->exec() != QDialog::Accepted || dialog->filePath().isEmpty())
    return;

  m_printerOutputFileName = dialog->filePath();
  currentTab()->page()->printToPdf(invoke(this, &BrowserMainWindow::slotHandlePdfPrinted), dialog->pageLayout());
}

void BrowserMainWindow::slotHandlePagePrinted(bool result)
{
  Q_UNUSED(result);

  delete m_currentPrinter;
  m_currentPrinter = nullptr;
}


void BrowserMainWindow::printRequested(QWebEnginePage *page)
{
  if (m_currentPrinter)
    return;
  m_currentPrinter = new QPrinter();
  QScopedPointer<QPrintDialog> dialog(new QPrintDialog(m_currentPrinter, this));
  dialog->setWindowTitle(tr("Print Document"));
  if (dialog->exec() != QDialog::Accepted)
  {
    slotHandlePagePrinted(false);
    return;
  }
  page->print(m_currentPrinter, invoke(this, &BrowserMainWindow::slotHandlePagePrinted));
}

void BrowserMainWindow::slotPrivateBrowsing()
{
  if (!BrowserApplication::instance()->privateBrowsing())
  {   
    BrowserApplication::instance()->setPrivateBrowsing(true);
  }
  else
  {
    // TODO: Also ask here
    BrowserApplication::instance()->setPrivateBrowsing(false);
  }
}

void BrowserMainWindow::closeEvent(QCloseEvent *event)
{
  pCmdThread->quit();
  
  event->accept();
  deleteLater();

  pCmdThread->terminate();
}

void BrowserMainWindow::slotEditFind()
{
  if (!currentTab())
    return;
  bool ok;
  QString search = QInputDialog::getText(this, tr("Find"),
                                         tr("Text:"), QLineEdit::Normal,
                                         m_lastSearch, &ok);
  if (ok && !search.isEmpty())
  {
    m_lastSearch = search;
    currentTab()->findText(m_lastSearch, 0, invoke(this, &BrowserMainWindow::handleFindTextResult));
  }
}

void BrowserMainWindow::slotEditFindNext()
{
  if (!currentTab() && !m_lastSearch.isEmpty())
    return;
  currentTab()->findText(m_lastSearch);
}

void BrowserMainWindow::slotEditFindPrevious()
{
  if (!currentTab() && !m_lastSearch.isEmpty())
    return;
  currentTab()->findText(m_lastSearch, QWebEnginePage::FindBackward);
}

void BrowserMainWindow::slotViewZoomIn()
{
  if (!currentTab())
    return;
  currentTab()->setZoomFactor(currentTab()->zoomFactor() + 0.1);
}

void BrowserMainWindow::slotViewZoomOut()
{
  if (!currentTab())
    return;
  currentTab()->setZoomFactor(currentTab()->zoomFactor() - 0.1);
}

void BrowserMainWindow::slotViewResetZoom()
{
  if (!currentTab())
    return;
  currentTab()->setZoomFactor(1.0);
}


void BrowserMainWindow::slotHome()
{
  QSettings settings;
  settings.beginGroup(QLatin1String("MainWindow"));
  QString home = settings.value(QLatin1String("home"), QLatin1String(defaultHome)).toString();

  int count = g_webengine.slUrls.count();
  if (count > 0)
  {
    if (g_webengine.iPlcSelected < count)
      home = g_webengine.slUrls.at(g_webengine.iPlcSelected);
    else
      home = g_webengine.slUrls.at(0);
  }

  loadPage(home);
}


void BrowserMainWindow::slotToggleInspector(bool enable)
{
#if defined(QWEBENGINEINSPECTOR)
  QWebEngineSettings::globalSettings()->setAttribute(QWebEngineSettings::DeveloperExtrasEnabled, enable);
  if (enable)
  {
    QMessageBox::setWindowModality(Qt::ApplicationModal);
    int result = QMessageBox::question(this, tr("Web Inspector"),
                                       tr("The web inspector will only work correctly for pages that were loaded after enabling.\n"
                                          "Do you want to reload all pages?"),
                                       QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes)
    {
      m_tabWidget->reloadAllTabs();
    }
  }
#else
  Q_UNUSED(enable);
#endif
}

void BrowserMainWindow::slotSwapFocus()
{
  if (currentTab()->hasFocus())
    m_tabWidget->currentLineEdit()->setFocus();
  else
    currentTab()->setFocus();
}


void BrowserMainWindow::loadPage(const QString &page)
{
  QUrl url = QUrl::fromUserInput(page);
  loadUrl(url);

}

TabWidget *BrowserMainWindow::tabWidget() const
{
  return m_tabWidget;
}

WebView *BrowserMainWindow::currentTab() const
{
  return m_tabWidget->currentWebView();
}

void BrowserMainWindow::slotLoadProgress(int progress)
{
  if (progress < 100 && progress > 0)
  {
    m_chaseWidget->setAnimated(true);
    disconnect(m_stopReload, SIGNAL(triggered()), m_reload, SLOT(trigger()));
    if (m_stopIcon.isNull())
      m_stopIcon = style()->standardIcon(QStyle::SP_BrowserStop);
    m_stopReload->setIcon(m_stopIcon);
    connect(m_stopReload, SIGNAL(triggered()), m_stop, SLOT(trigger()));
    m_stopReload->setToolTip(tr("Stop loading the current page"));
  }
  else
  {
    m_chaseWidget->setAnimated(false);
    disconnect(m_stopReload, SIGNAL(triggered()), m_stop, SLOT(trigger()));
    m_stopReload->setIcon(m_reloadIcon);
    connect(m_stopReload, SIGNAL(triggered()), m_reload, SLOT(trigger()));
    m_stopReload->setToolTip(tr("Reload the current page"));
  }
}

void BrowserMainWindow::slotAboutToShowBackMenu()
{
  m_historyBackMenu->clear();
  if (!currentTab())
    return;
  QWebEngineHistory *history = currentTab()->history();
  int historyCount = history->count();
  for (int i = history->backItems(historyCount).count() - 1; i >= 0; --i)
  {
    QWebEngineHistoryItem item = history->backItems(history->count()).at(i);
    QAction *action = new QAction(this);
    action->setData(-1*(historyCount-i-1));
    QIcon icon = BrowserApplication::instance()->icon(item.url());
    action->setIcon(icon);
    action->setText(item.title());
    m_historyBackMenu->addAction(action);
  }
}

void BrowserMainWindow::slotAboutToShowForwardMenu()
{
  m_historyForwardMenu->clear();
  if (!currentTab())
    return;
  QWebEngineHistory *history = currentTab()->history();
  int historyCount = history->count();
  for (int i = 0; i < history->forwardItems(history->count()).count(); ++i)
  {
    QWebEngineHistoryItem item = history->forwardItems(historyCount).at(i);
    QAction *action = new QAction(this);
    action->setData(historyCount-i);
    QIcon icon = BrowserApplication::instance()->icon(item.url());
    action->setIcon(icon);
    action->setText(item.title());
    m_historyForwardMenu->addAction(action);
  }
}

void BrowserMainWindow::slotAboutToShowWindowMenu()
{
  m_windowMenu->clear();
  m_windowMenu->addAction(m_tabWidget->nextTabAction());
  m_windowMenu->addAction(m_tabWidget->previousTabAction());
  m_windowMenu->addSeparator();
  m_windowMenu->addAction(tr("Downloads"), this, SLOT(slotDownloadManager()), QKeySequence(tr("Alt+Ctrl+L",
                          "Download Manager")));
  m_windowMenu->addSeparator();

  QList<BrowserMainWindow*> windows = BrowserApplication::instance()->mainWindows();
  for (int i = 0; i < windows.count(); ++i)
  {
    BrowserMainWindow *window = windows.at(i);
    QAction *action = m_windowMenu->addAction(window->windowTitle(), this, SLOT(slotShowWindow()));
    action->setData(i);
    action->setCheckable(true);
    if (window == this)
      action->setChecked(true);
  }
}

void BrowserMainWindow::slotShowWindow()
{
  if (QAction *action = qobject_cast<QAction*>(sender()))
  {
    QVariant v = action->data();
    if (v.canConvert<int>())
    {
      int offset = qvariant_cast<int>(v);
      QList<BrowserMainWindow*> windows = BrowserApplication::instance()->mainWindows();
      windows.at(offset)->activateWindow();
      windows.at(offset)->currentTab()->setFocus();
    }
  }
}

void BrowserMainWindow::slotOpenActionUrl(QAction *action)
{
  int offset = action->data().toInt();
  QWebEngineHistory *history = currentTab()->history();
  if (offset < 0)
    history->goToItem(history->backItems(-1*offset).first()); // back
  else if (offset > 0)
    history->goToItem(history->forwardItems(history->count() - offset + 1).back()); // forward
}

void BrowserMainWindow::geometryChangeRequested(const QRect &geometry)
{
  setGeometry(geometry);
}

/// \brief starting cmd thread
///
void BrowserMainWindow::startCmdThread()
{
  //call only once

  if (pCmdThread)
    return;

  pCmdThread = new cmdThread(this);

  if (! pCmdThread)
    return;

  connect(pCmdThread,
          SIGNAL(cmdSignalReceived(QString)),
          this,
          SLOT(slotCmdReceived(QString)) );

  //start thread function run
  pCmdThread->start(QThread::NormalPriority);

}



void BrowserMainWindow::slotCmdReceived(QString s)
{
  QString sCmd = s.trimmed();
  //qDebug() << "slotCmdReceived: " << sCmd;
  
  if (sCmd.left(5) == "load=")
  {
    sCmd.remove(0,5);
    if (!sCmd.isEmpty())
    {     
      loadPage(sCmd);
      //bBringToFront = true;
    }
  }
  else if (sCmd.left(10) == "tcpmonitor")
  {
    sCmd.remove(0,10);
    if (!sCmd.isEmpty())
    {
      QStringList sList = sCmd.split("=");
      if (sList.count()==2)
      {
        WebView * pView = m_tabWidget->currentWebView();
        pView->SetTcpRxTimestamp(sList[0], sList[1].toInt());
      }
    }
  }
  else if (sCmd.left(7) == "newtab=")
  {
    sCmd.remove(0,7);
    if (!sCmd.isEmpty())
    {
      if (m_tabWidget->count() <= MAX_TAB_COUNT)
      {
        m_tabWidget->newTab(true);
        loadPage(sCmd);
      }
    }

  }
  else if (sCmd.left(9) == "tabindex=")
  {
    sCmd.remove(0,9);
    if (!sCmd.isEmpty())
    {
      int idx = sCmd.toInt();
      if ((idx < MAX_TAB_COUNT)&&(idx >= 0))
      {
        m_tabWidget->setCurrentIndex(idx);
        //bBringToFront = true;
      }
    }

  }
  else if (sCmd.left(14) == "closetabindex=")
  {
    sCmd.remove(0,14);
    if (!sCmd.isEmpty())
    {
      int idx = sCmd.toInt();
      if ((idx < MAX_TAB_COUNT)&&(idx >= 0))
      {
        m_tabWidget->closeTab(idx);
      }
    }

  }
  else if (sCmd.left(8) == "closetab")
  {
    //close the current active tab
    sCmd.remove(0,8);
    if (sCmd.isEmpty())
    {
      int idx = m_tabWidget->currentIndex();
      if (idx >= 0)
      {
        m_tabWidget->closeTab(idx);
      }
    }

  }
  else if (sCmd.left(8) == "closetabs")
  {
    //close all tabs except the current active tab
    sCmd.remove(0,9);
    if (sCmd.isEmpty())
    {
      //close all tabs above current index
      int idx = m_tabWidget->currentIndex();
      int idx2remove = m_tabWidget->count() -1;
      while (idx2remove > idx)
      {
        m_tabWidget->closeTab(idx2remove);
        idx2remove = m_tabWidget->count() -1;
      }

      //close all tabs under current index
      while ((idx > 0) && (m_tabWidget->count() > 1))
      {
        m_tabWidget->closeTab(0);
      }
    }

  }
  else if (sCmd == "stop")
  {
    if (currentTab())
    {
      WebView * pView = m_tabWidget->currentWebView();
      pView->stop();
    }
  }  
  else if (sCmd == "reload")
  {
    if (currentTab())
    {
      WebView * pView = m_tabWidget->currentWebView();
      pView->reload();
      //bBringToFront = true;
    }
  }
  else if (sCmd == "back")
  {
    if (currentTab())
    {
      WebView * pView = m_tabWidget->currentWebView();
      pView->back();
    }
  }
  else if (sCmd == "forward")
  {
    if (currentTab())
    {
      WebView * pView = m_tabWidget->currentWebView();
      pView->forward();
    }
  }
  else if (sCmd.left(7) == "navbar=")
  {
    // Hide and Show Navbar
    sCmd.remove(0,7);
    if (sCmd == "show")
    {
      m_navigationBar->show();
    }
    else if (sCmd == "hide")
    {
      m_navigationBar->hide();
    }
  }
  else if (sCmd.left(8) == "menubar=")
  {
    // Hide and Show Menubar
    sCmd.remove(0,8);
    if (sCmd == "show")
    {
      menuBar()->show();
    }
    else if (sCmd == "hide")
    {
      menuBar()->hide();
    }
  }
  else if (sCmd.left(10) == "bookmarks=")
  {
    // Hide and Show bookmarks
    sCmd.remove(0,10);
    if (sCmd == "show")
    {
      m_bookmarksToolbar->show();
    }
    else if (sCmd == "hide")
    {
      m_bookmarksToolbar->hide();
    }
  }
  else if (sCmd.left(7) == "tabbar=")
  {
    // Hide and Show Tabbar
    sCmd.remove(0,7);
    QTabBar *tabBar = m_tabWidget->findChild<QTabBar *>();
    if (sCmd == "show")
    {
      if (tabBar)
      {
        tabBar->show();
      }
    }
    else if (sCmd == "hide")
    {
      if (tabBar)
      {
        tabBar->hide();
      }
    }

  }
  else if (sCmd == "close")
  {
    //quitBrowser();
    close();
  }
  else if (sCmd == "clearcache")
  {
    QWebEngineProfile::defaultProfile()->clearHttpCache();
  }
  else if (sCmd == "reloadplcconfig")
  {
    ReadPlcselectConfigFile(CONF_FILE_PLCSELECT, true);
  }
  else
  {
    qDebug() << "Tabcount: " << m_tabWidget->count();
    qDebug() << "unknown: " << sCmd;
  }
  
}


bool BrowserMainWindow::isNavbarVisible()
{
  return m_navigationBar->isVisible();
}

void BrowserMainWindow::ReadPlcselectConfigFile(QString sFile, bool bReload)
{
    //default settings
    if (bReload == false)
    {
      g_webengine.bReconnect = true;
      g_webengine.iReconnectInterval = 10;
      g_webengine.iPlcSelected = 0;          //WBM
    }

    g_webengine.slMonitor.clear();
    g_webengine.slUrls.clear();
    g_webengine.slTitles.clear();
    g_webengine.slVirtualKeyboard.clear();

    //read settings from file
    QSettings settings(sFile, QSettings::IniFormat);
    const QStringList allKeys = settings.allKeys();
    foreach (const QString &key, allKeys)
    {
        QString var = settings.value(key).toString();

        if (var.length() > 0)
        {
          //qDebug() << key << " " << var;

          if (key == "reconnect_interval")
          {
            g_webengine.iReconnectInterval = var.toInt();
          }
          else if ((bReload == false) && (key == "plc_selected"))
          {
            g_webengine.iPlcSelected = var.toInt();
          }
          else if (key == "statemonitor")
          {
            if (var.length() > 0)
            {
              if (var.compare("enabled", Qt::CaseInsensitive) == 0)
              {
                g_webengine.bReconnect = true;
              }
              else
              {
                g_webengine.bReconnect = false;
              }
            }
          }
          else if (
            (key == "url00") || (key == "url01") || (key == "url02") || (key == "url03") || (key == "url04") ||
            (key == "url05") || (key == "url06") || (key == "url07") || (key == "url08") || (key == "url09") ||
            (key == "url10") || (key == "url11")
          )
          {
            if (var.length() > 0)
              g_webengine.slUrls.append(var);
          }
          else if (
            (key == "txt00") || (key == "txt01") || (key == "txt02") || (key == "txt03") || (key == "txt04") ||
            (key == "txt05") || (key == "txt06") || (key == "txt07") || (key == "txt08") || (key == "txt09") ||
            (key == "txt10") || (key == "txt11")
          )
          {

            if (var.length() > 0)
            {
              g_webengine.slTitles.append(var);
            }
            else
            {
              g_webengine.slTitles.append("no title");
            }

          }
          else if (
            (key == "mon00") || (key == "mon01") || (key == "mon02") || (key == "mon03") || (key == "mon04") ||
            (key == "mon05") || (key == "mon06") || (key == "mon07") || (key == "mon08") || (key == "mon09") ||
            (key == "mon10") || (key == "mon11")
          )
          {            
            if (var.length() > 0)
            {
              g_webengine.slMonitor.append(var);
            }
          }
          else if (
            (key == "vkb00") || (key == "vkb01") || (key == "vkb02") || (key == "vkb03") || (key == "vkb04") ||
            (key == "vkb05") || (key == "vkb06") || (key == "vkb07") || (key == "vkb08") || (key == "vkb09") ||
            (key == "vkb10") || (key == "vkb11")
          )
          {
            if (var.length() > 0)
            {
              g_webengine.slVirtualKeyboard.append(var);
            }
          }

        }
    }

    g_webengine.iReconnectInterval = qBound(MIN_RECONNECT_INTERVAL, g_webengine.iReconnectInterval, MAX_RECONNECT_INTERVAL);
    //deactivate if value is 0
    if (g_webengine.iReconnectInterval == 0)
    {
      g_webengine.bReconnect = false;
    }


}

void BrowserMainWindow::ReadWebengineConfigurationFile(QString sFile)
{
  g_webengine.bBookmarkbar = false;
  g_webengine.bMenubar = false;
  g_webengine.bNavbar = false;
  g_webengine.bTabbar = false;
  g_webengine.bZoom = 0;  

// startup configuration settings
  QSettings settings(sFile, QSettings::IniFormat);
  const QStringList allKeys = settings.allKeys();
  foreach (const QString &key, allKeys)
  {
    QString var = settings.value(key).toString();

    //qDebug() << "key: " << key << " var: " << var;

    if (key == "tabbar")
    {
      if (var.length() > 0)
      {
        if (var.compare("1", Qt::CaseInsensitive) == 0)
        {
          g_webengine.bTabbar = true;
        }
        else
        {
          g_webengine.bTabbar = false;
        }
      }
    }
    else if (key == "navbar")
    {
      if (var.length() > 0)
      {
        if (var.compare("1", Qt::CaseInsensitive) == 0)
        {
          g_webengine.bNavbar = true;
        }
        else
        {
          g_webengine.bNavbar = false;
        }
      }
    }
    else if (key == "bookmarks")
    {
      if (var.length() > 0)
      {
        if (var.compare("1", Qt::CaseInsensitive) == 0)
        {
          g_webengine.bBookmarkbar = true;
        }
        else
        {
          g_webengine.bBookmarkbar = false;
        }
      }
    }
    else if (key == "menubar")
    {
      if (var.length() > 0)
      {
        if (var.compare("1", Qt::CaseInsensitive) == 0)
        {
          g_webengine.bMenubar = true;
        }
        else
        {
          g_webengine.bMenubar = false;
        }
      }
    }
    else if (key == "tabindex")
    {
      if (var.length() > 0)
        g_webengine.iTabIndex = var.toInt();
    }
    else if (key == "zoom")
    {
      if (var.length() > 0)
      {
        if (var.compare("1", Qt::CaseInsensitive) == 0)
        {
          g_webengine.bZoom = true;
        }
        else
        {
          g_webengine.bZoom = false;
        }
      }
    }
    else if (key == "script")
    {
      if (var.length() > 0)
        g_webengine.sJsScriptFilename = var;
    }
    else if (key.left(12) == "Error500Text")
    {
      if (var.length() > 0)
      {
        QString sNew = var.trimmed();
        if (sNew.length() > 0)
          g_webengine.slError5Text.append(sNew);
      }
    }
    else if (key == "ConnectText")
    {
      //ConnectText=Try to connect: %U
      if (var.length() > 0)
      {
        g_webengine.sConnectedText = var.trimmed();
      }
    }
    else if (key == "DisconnectedText")
    {
      //DisconnectedText=Connection failed: %U
      if (var.length() > 0)
      {
        g_webengine.sDisconnectedText = var.trimmed();
      }
    }

  }


  //deactivate if value is 0
  if (g_webengine.iReconnectInterval == 0)
  {
    g_webengine.bReconnect = false;
  }
  else if (g_webengine.iReconnectInterval < 5)
  {
    //min value 5
    g_webengine.iReconnectInterval = 5;
  }
  else if (g_webengine.iReconnectInterval > 99)
  {
    //max value 99
    g_webengine.iReconnectInterval = 99;
  }

  //fill with defaults if empty
  if (g_webengine.slError5Text.count() < 1)
  {
     g_webengine.slError5Text.append("500 - Internal Server Error");
     g_webengine.slError5Text.append("503 - Service Not Available");
  }

  if (g_webengine.sConnectedText.length() < 1)
  {
      g_webengine.sConnectedText = "Try to connect: %U";
  }

  if (g_webengine.sDisconnectedText.length() < 1)
  {
      g_webengine.sDisconnectedText = "Connection failed: %U";
  }

}

bool BrowserMainWindow::IsCursorOnScrollbarHorizontal(QPoint pos)
{
  bool bRet = false;
  int srollbarWidth = 16;
  int addition = 7;
  if (pos.y() > (height() - srollbarWidth - addition))
    return true;

  WebPage * wp = qobject_cast<WebPage*>(tabWidget()->currentWebView()->page());
  if (!wp)
    return false;

  //qDebug() << "IsCursorOnScrollbarHorizontal Count: " << wp->listScrollbarHorizPositions.count();
  foreach (const int &ypos, wp->listScrollbarHorizPositions)
  {
     if (ypos > 30)
     {
       int topMargin = (ypos - srollbarWidth - addition);
       int bottomMargin = (ypos + addition);
       //qDebug() << "yc: " << pos.y() << " tm: " << topMargin << " bm: " << bottomMargin;
       if ( (pos.y() > topMargin) && (pos.y() < bottomMargin) )
       {
         //qDebug() << "on scrollbar YES";
         bRet = true;
         break;
       }
     }
  }

  return bRet;
}

bool BrowserMainWindow::IsCursorOnScrollbarVertical(QPoint pos)
{
  bool bRet = false;
  int srollbarWidth = 16;
  int addition = 7;
  if (pos.x() > (width() - srollbarWidth - addition))
    return true;

  WebPage * wp = qobject_cast<WebPage*>(tabWidget()->currentWebView()->page());
  if (!wp)
    return false;

  //qDebug() << "IsCursorOnScrollbarVertical Count: " << wp->listScrollbarVertPositions.count();
  foreach (const int &xpos, wp->listScrollbarVertPositions)
  {
     //qDebug() << xpos;
     if (xpos > 30)
     {
       int leftMargin = (xpos - srollbarWidth - addition);
       int rightMargin = (xpos + addition);
       //qDebug() << "xc: " << pos.x() << " lm: " << leftMargin << " rm: " << rightMargin;

       if ( (pos.x() > leftMargin) && (pos.x() < rightMargin) )
       {
         //qDebug() << "on scrollbar YES";
         bRet = true;
         break;
       }
     }
  }

  return bRet;
}

void BrowserMainWindow::slotExecuteJsScrollbarVertCode()
{
  WebPage * wp = qobject_cast<WebPage*>(tabWidget()->currentWebView()->page());
  if (wp)
  {
    wp->ExecuteJsScrollbarVertCode();
  }
}

void BrowserMainWindow::slotExecuteJsScrollbarHorizCode()
{
  WebPage * wp = qobject_cast<WebPage*>(tabWidget()->currentWebView()->page());
  if (wp)
  {
    wp->ExecuteJsScrollbarHorizCode();
  }
}

/// \brief slot scroll gesture horizontal left received
/// \param[in]  delta scroll distance
///
void BrowserMainWindow::scrollSlotHLeft(QPoint pos)
{  
  QWidget * pWidget = QApplication::focusWidget();
  if (pWidget)
  {
    QPoint posCursor = QCursor::pos();
    if (IsCursorOnScrollbarHorizontal(posCursor))
      return;
    int delta = 30; //120
    QPointF posG(posCursor);
    QWheelEvent evWheel(posG, posG, QPoint(0,0), QPoint(delta,0), delta, Qt::Horizontal, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(pWidget, &evWheel);
    pWidget->update();        
  }
}

/// \brief slot scroll gesture horizontal right received
/// \param[in]  delta scroll distance
///
void BrowserMainWindow::scrollSlotHRight(QPoint pos)
{
  QWidget * pWidget = QApplication::focusWidget();
  if (pWidget)
  {
    QPoint posCursor = QCursor::pos();
    if (IsCursorOnScrollbarHorizontal(posCursor))
      return;
    int delta = -30; //-120
    QPointF posG(posCursor);
    QWheelEvent evWheel(posG, posG, QPoint(0,0), QPoint(delta,0), delta, Qt::Horizontal, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(pWidget, &evWheel);
    pWidget->update();       
  }
}

/// \brief slot scroll gesture vertical up received
/// \param[in]  delta scroll distance
///
void BrowserMainWindow::scrollSlotVUp(QPoint pos)
{
  QWidget * pWidget = QApplication::focusWidget();  
  if (pWidget)
  {
    QPoint posCursor = QCursor::pos();
    if (IsCursorOnScrollbarVertical(posCursor))
      return;
    int delta = 30; //120
    QPointF posG(posCursor);
    QWheelEvent evWheel(posG, posG, QPoint(0,0), QPoint(0,delta), delta, Qt::Vertical, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(pWidget, &evWheel);
    pWidget->update();
  }
}

/// \brief slot scroll gesture vertical down received
/// \param[in]  delta scroll distance
///
void BrowserMainWindow::scrollSlotVDown(QPoint pos)
{
  QWidget * pWidget = QApplication::focusWidget();  
  if (pWidget)
  {
    QPoint posCursor = QCursor::pos();
    if (IsCursorOnScrollbarVertical(posCursor))
      return;
    int delta = -30; //-120
    QPointF posG(posCursor);
    QWheelEvent evWheel(posG, posG, QPoint(0,0), QPoint(0,delta), delta, Qt::Vertical, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(pWidget, &evWheel);
    pWidget->update();   
  }
}

void BrowserMainWindow::showEvent(QShowEvent *event)
{
  //DEPRECATED QTimer::singleShot(250, this, SLOT(ActivateX11Window()));
}
