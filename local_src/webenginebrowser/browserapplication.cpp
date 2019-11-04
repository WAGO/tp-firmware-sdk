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

#include "browserapplication.h"

#include "bookmarks.h"
#include "browsermainwindow.h"
#include "cookiejar.h"
#include "downloadmanager.h"
#include "history.h"
#include "tabwidget.h"
#include "webview.h"
#include "tools.h"

#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtCore/QTranslator>

#include <QtGui/QDesktopServices>
#include <QtGui/QFileOpenEvent>
#include <QtWidgets/QMessageBox>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QSslSocket>

#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

#include <QtCore/QDebug>

#include <sys/file.h>
#include <errno.h>

DownloadManager *BrowserApplication::s_downloadManager = 0;
HistoryManager *BrowserApplication::s_historyManager = 0;
QNetworkAccessManager *BrowserApplication::s_networkAccessManager = 0;
BookmarksManager *BrowserApplication::s_bookmarksManager = 0;

static void setUserStyleSheet(QWebEngineProfile *profile, const QString &styleSheet, BrowserMainWindow *mainWindow = 0)
{
  Q_ASSERT(profile);
  QString scriptName(QStringLiteral("userStyleSheet"));
  QWebEngineScript script;
  QList<QWebEngineScript> styleSheets = profile->scripts()->findScripts(scriptName);
  if (!styleSheets.isEmpty())
    script = styleSheets.first();
  Q_FOREACH (const QWebEngineScript &s, styleSheets)
    profile->scripts()->remove(s);

  if (script.isNull())
  {
    script.setName(scriptName);
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setRunsOnSubFrames(true);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
  }
  QString source = QString::fromLatin1("(function() {"\
                                       "var css = document.getElementById(\"_qt_testBrowser_userStyleSheet\");"\
                                       "if (css == undefined) {"\
                                       "    css = document.createElement(\"style\");"\
                                       "    css.type = \"text/css\";"\
                                       "    css.id = \"_qt_testBrowser_userStyleSheet\";"\
                                       "    document.head.appendChild(css);"\
                                       "}"\
                                       "css.innerText = \"%1\";"\
                                       "})()").arg(styleSheet);
  script.setSourceCode(source);
  profile->scripts()->insert(script);
  // run the script on the already loaded views
  // this has to be deferred as it could mess with the storage initialization on startup
  if (mainWindow)
    QMetaObject::invokeMethod(mainWindow, "runScriptOnOpenViews", Qt::QueuedConnection, Q_ARG(QString, source));
}

BrowserApplication::BrowserApplication(int &argc, char **argv)
  : QApplication(argc, argv)
  , m_localServer(0)
  , m_privateProfile(0)
  , m_privateBrowsing(false)
{

  int pid_file = open("/var/run/webenginebrowser.pid", O_CREAT | O_RDWR, 0666);
  int rc = flock(pid_file, LOCK_EX | LOCK_NB);
  if(rc)
  {
    if(EWOULDBLOCK == errno)
    {
      qDebug() << "Another instance is running. Exit.";
      QApplication::exit(1);
    }
  }


  QCoreApplication::setOrganizationName(QLatin1String("Qt"));
  QCoreApplication::setApplicationName(QLatin1String("webenginebrowser"));
  QCoreApplication::setApplicationVersion(QLatin1String("0.1"));
  QString serverName = QCoreApplication::applicationName()
                       + QString::fromLatin1(QT_VERSION_STR).remove('.') + QLatin1String("webengine");
  QLocalSocket socket;
  socket.connectToServer(serverName);
  if (socket.waitForConnected(500))
  {
    QTextStream stream(&socket);
    stream << getCommandLineUrlArgument();
    stream.flush();
    socket.waitForBytesWritten();
    return;
  }

#if defined(Q_OS_OSX)
  QApplication::setQuitOnLastWindowClosed(false);
#else
  QApplication::setQuitOnLastWindowClosed(true);
#endif

  m_localServer = new QLocalServer(this);
  connect(m_localServer, SIGNAL(newConnection()),
          this, SLOT(newLocalSocketConnection()));
  if (!m_localServer->listen(serverName)
      && m_localServer->serverError() == QAbstractSocket::AddressInUseError)
  {
    QLocalServer::removeServer(serverName);
    if (!m_localServer->listen(serverName))
      qWarning("Could not create local socket %s.", qPrintable(serverName));
  }

#ifndef QT_NO_OPENSSL
  if (!QSslSocket::supportsSsl())
  {
    qDebug() << "This system does not support OpenSSL";    
  }
#endif

  QDesktopServices::setUrlHandler(QLatin1String("http"), this, "openUrl");
  QString localSysName = QLocale::system().name();

  installTranslator(QLatin1String("qt_") + localSysName);

  QSettings settings;
  settings.beginGroup(QLatin1String("sessions"));
  m_lastSession = settings.value(QLatin1String("lastSession")).toByteArray();
  settings.endGroup();

#if defined(Q_OS_OSX)
  connect(this, SIGNAL(lastWindowClosed()),
          this, SLOT(lastWindowClosed()));
#endif

  QString sCacheDir = "/tmp/webengine";
  bool bExists = QDir(sCacheDir).exists(); 
  if (! bExists)
  {
    QDir().mkdir(sCacheDir);    
  }
  QWebEngineProfile::defaultProfile()->setCachePath(sCacheDir);
  
  if (!m_privateProfile)
    m_privateProfile = new QWebEngineProfile(this); 
  
  QRect r = QApplication::desktop()->screenGeometry();
  CreateUserAgentString(r);
  
  QTimer::singleShot(0, this, SLOT(postLaunch()));
}

BrowserApplication::~BrowserApplication()
{
  delete s_downloadManager;
  for (int i = 0; i < m_mainWindows.size(); ++i)
  {
    BrowserMainWindow *window = m_mainWindows.at(i);
    delete window;
  }
  delete s_networkAccessManager;
  delete s_bookmarksManager;
}

void BrowserApplication::lastWindowClosed()
{
#if defined(Q_OS_OSX)
  clean();
  BrowserMainWindow *mw = new BrowserMainWindow;
  mw->slotHome();
  m_mainWindows.prepend(mw);
#endif
}

BrowserApplication *BrowserApplication::instance()
{
  return (static_cast<BrowserApplication *>(QCoreApplication::instance()));
}

void BrowserApplication::quitBrowser()
{

}

/*!
    Any actions that can be delayed until the window is visible
 */
void BrowserApplication::postLaunch()
{
  QString directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
  if (directory.isEmpty())
    directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
#if defined(QWEBENGINESETTINGS_PATHS)
  QWebEngineSettings::setIconDatabasePath(directory);
  QWebEngineSettings::setOfflineStoragePath(directory);
#endif

  setWindowIcon(QIcon(QLatin1String(":demobrowser.svg")));

  loadSettings();

  // newMainWindow() needs to be called in main() for this to happen
  if (m_mainWindows.count() > 0)
  {
    const QString url = getCommandLineUrlArgument();
    if (!url.isEmpty())
    {
      mainWindow()->loadPage(url);
    }
    else
    {
      mainWindow()->slotHome();
    }

  }
  BrowserApplication::historyManager();
}

QString BrowserApplication::ReadUserStyleSheetFromFile()
{
  QString sTxt;
  QFile file("/etc/specific/webengine/userstylesheet.conf");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
     QTextStream in(&file);
     sTxt = in.readAll();
     file.close();
  }
  return sTxt;
}

void BrowserApplication::loadSettings()
{
  QSettings settings;
  settings.beginGroup(QLatin1String("websettings"));

  QWebEngineSettings *defaultSettings = QWebEngineSettings::globalSettings();
  QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();

  QString standardFontFamily = defaultSettings->fontFamily(QWebEngineSettings::StandardFont);
  int standardFontSize = defaultSettings->fontSize(QWebEngineSettings::DefaultFontSize);
  QFont standardFont = QFont(standardFontFamily, standardFontSize);
  standardFont = qvariant_cast<QFont>(settings.value(QLatin1String("standardFont"), standardFont));
  defaultSettings->setFontFamily(QWebEngineSettings::StandardFont, standardFont.family());
  defaultSettings->setFontSize(QWebEngineSettings::DefaultFontSize, standardFont.pointSize());

  QString fixedFontFamily = defaultSettings->fontFamily(QWebEngineSettings::FixedFont);
  int fixedFontSize = defaultSettings->fontSize(QWebEngineSettings::DefaultFixedFontSize);
  QFont fixedFont = QFont(fixedFontFamily, fixedFontSize);
  fixedFont = qvariant_cast<QFont>(settings.value(QLatin1String("fixedFont"), fixedFont));
  defaultSettings->setFontFamily(QWebEngineSettings::FixedFont, fixedFont.family());
  defaultSettings->setFontSize(QWebEngineSettings::DefaultFixedFontSize, fixedFont.pointSize());

  //defaultSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, settings.value(QLatin1String("enableJavascript"), true).toBool());
  //defaultSettings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, settings.value(QLatin1String("enableScrollAnimator"), false).toBool());
  //defaultSettings->setAttribute(QWebEngineSettings::PluginsEnabled, settings.value(QLatin1String("enablePlugins"), false).toBool());  
  defaultSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
  defaultSettings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, false);
  defaultSettings->setAttribute(QWebEngineSettings::PluginsEnabled, true);

  defaultSettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, false);
  defaultSettings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
  defaultSettings->setAttribute(QWebEngineSettings::WebGLEnabled, true);

  QString css = ReadUserStyleSheetFromFile();
  css.remove('\r');
  css.remove('\n');
  if (css.length() < 10)
  {
    css = settings.value(QLatin1String("userStyleSheet")).toString();
  }
  setUserStyleSheet(defaultProfile, css, mainWindow());

  //qDebug() << "-------------------------------------------";
  //qDebug() << sUserAgent;
  //qDebug() << "-------------------------------------------";

  defaultProfile->setHttpUserAgent(sUserAgent);
  //old defaultProfile->setHttpUserAgent(settings.value(QLatin1String("httpUserAgent")).toString());
  
  defaultProfile->setHttpAcceptLanguage(settings.value(QLatin1String("httpAcceptLanguage")).toString());

  switch (settings.value(QLatin1String("faviconDownloadMode"), 1).toInt())
  {
  case 0:
    defaultSettings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    break;
  case 1:
    defaultSettings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, true);
    defaultSettings->setAttribute(QWebEngineSettings::TouchIconsEnabled, false);
    break;
  case 2:
    defaultSettings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, true);
    defaultSettings->setAttribute(QWebEngineSettings::TouchIconsEnabled, true);
    break;
  }

  settings.endGroup();
  settings.beginGroup(QLatin1String("cookies"));
  
  QWebEngineProfile::PersistentCookiesPolicy persistentCookiesPolicy =
             QWebEngineProfile::PersistentCookiesPolicy(settings.value(QLatin1String("persistentCookiesPolicy"),
                                                                       QWebEngineProfile::AllowPersistentCookies).toInt());
  defaultProfile->setPersistentCookiesPolicy(persistentCookiesPolicy); 
  QString pdataPath = settings.value(QLatin1String("persistentDataPath")).toString();
  defaultProfile->setPersistentStoragePath(pdataPath);

  settings.endGroup();

  settings.beginGroup(QLatin1String("proxy"));
  QNetworkProxy proxy;
  if (settings.value(QLatin1String("enabled"), false).toBool())
  {
    if (settings.value(QLatin1String("type"), 0).toInt() == 0)
      proxy = QNetworkProxy::Socks5Proxy;
    else
      proxy = QNetworkProxy::HttpProxy;
    proxy.setHostName(settings.value(QLatin1String("hostName")).toString());
    proxy.setPort(settings.value(QLatin1String("port"), 1080).toInt());
    proxy.setUser(settings.value(QLatin1String("userName")).toString());
    proxy.setPassword(settings.value(QLatin1String("password")).toString());
    QNetworkProxy::setApplicationProxy(proxy);
  }
  
  settings.endGroup();
}

QList<BrowserMainWindow*> BrowserApplication::mainWindows()
{
  clean();
  QList<BrowserMainWindow*> list;
  for (int i = 0; i < m_mainWindows.count(); ++i)
    list.append(m_mainWindows.at(i));
  return list;
}

void BrowserApplication::clean()
{
  // cleanup any deleted main windows first
  for (int i = m_mainWindows.count() - 1; i >= 0; --i)
    if (m_mainWindows.at(i).isNull())
      m_mainWindows.removeAt(i);
}

void BrowserApplication::saveSession()
{
  if (m_privateBrowsing)
    return;

  clean();

  QSettings settings;
  settings.beginGroup(QLatin1String("sessions"));

  QByteArray data;
  QBuffer buffer(&data);
  QDataStream stream(&buffer);
  buffer.open(QIODevice::ReadWrite);

  stream << m_mainWindows.count();
  for (int i = 0; i < m_mainWindows.count(); ++i)
    stream << m_mainWindows.at(i)->saveState();
  settings.setValue(QLatin1String("lastSession"), data);
  settings.endGroup();
}

bool BrowserApplication::canRestoreSession() const
{
  return !m_lastSession.isEmpty();
}

void BrowserApplication::restoreLastSession()
{
  QList<QByteArray> windows;
  QBuffer buffer(&m_lastSession);
  QDataStream stream(&buffer);
  buffer.open(QIODevice::ReadOnly);
  int windowCount;
  stream >> windowCount;
  for (int i = 0; i < windowCount; ++i)
  {
    QByteArray windowState;
    stream >> windowState;
    windows.append(windowState);
  }
  for (int i = 0; i < windows.count(); ++i)
  {
    BrowserMainWindow *newWindow = 0;
    if (m_mainWindows.count() == 1
        && mainWindow()->tabWidget()->count() == 1
        && mainWindow()->currentTab()->url() == QUrl())
    {
      newWindow = mainWindow();
    }
    else
    {
      newWindow = newMainWindow();
    }
    newWindow->restoreState(windows.at(i));
  }
}

bool BrowserApplication::isTheOnlyBrowser() const
{
  return (m_localServer != 0);
}

void BrowserApplication::installTranslator(const QString &name)
{
  QTranslator *translator = new QTranslator(this);
  translator->load(name, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  QApplication::installTranslator(translator);
}

QString BrowserApplication::getCommandLineUrlArgument() const
{
  const QStringList args = QCoreApplication::arguments();
  if (args.count() > 1)
  {
    const QString lastArg = args.last();
    const bool isValidUrl = QUrl::fromUserInput(lastArg).isValid();
    if (isValidUrl)
      return lastArg;
  }

  return QString();
}

#if defined(Q_OS_OSX)
bool BrowserApplication::event(QEvent* event)
{
  switch (event->type())
  {
  case QEvent::ApplicationActivate:
  {
    clean();
    if (!m_mainWindows.isEmpty())
    {
      BrowserMainWindow *mw = mainWindow();
      if (mw && !mw->isMinimized())
      {
        mainWindow()->show();
      }
      return true;
    }
  }
  case QEvent::FileOpen:
    if (!m_mainWindows.isEmpty())
    {
      mainWindow()->loadPage(static_cast<QFileOpenEvent *>(event)->file());
      return true;
    }
  default:
    break;
  }
  return QApplication::event(event);
}
#endif

void BrowserApplication::openUrl(const QUrl &url)
{
  mainWindow()->loadPage(url.toString());
}

BrowserMainWindow *BrowserApplication::newMainWindow()
{
  BrowserMainWindow *browser = new BrowserMainWindow();
  m_mainWindows.prepend(browser);
  browser->show();
  return browser;
}

BrowserMainWindow *BrowserApplication::mainWindow()
{
  clean();
  if (m_mainWindows.isEmpty())
    newMainWindow();
  return m_mainWindows[0];
}

void BrowserApplication::newLocalSocketConnection()
{
  QLocalSocket *socket = m_localServer->nextPendingConnection();
  if (!socket)
    return;
  socket->waitForReadyRead(1000);
  QTextStream stream(socket);
  QString url;
  stream >> url;
  if (!url.isEmpty())
  {
    QSettings settings;
    settings.beginGroup(QLatin1String("general"));
    int openLinksIn = settings.value(QLatin1String("openLinksIn"), 0).toInt();
    settings.endGroup();
    if (openLinksIn == 1)
      newMainWindow();
    else
      mainWindow()->tabWidget()->newTab();
    openUrl(url);
  }
  delete socket;
  mainWindow()->raise();
  mainWindow()->activateWindow();
}

CookieJar *BrowserApplication::cookieJar()
{
#if defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
  return (CookieJar*)networkAccessManager()->cookieJar();
#else
  return 0;
#endif
}

DownloadManager *BrowserApplication::downloadManager()
{
  if (!s_downloadManager)
  {
    s_downloadManager = new DownloadManager();
  }
  return s_downloadManager;
}

QNetworkAccessManager *BrowserApplication::networkAccessManager()
{
  if (!s_networkAccessManager)
  {
    s_networkAccessManager = new QNetworkAccessManager();
  }
  return s_networkAccessManager;
}

HistoryManager *BrowserApplication::historyManager()
{
  if (!s_historyManager)
    s_historyManager = new HistoryManager();
  return s_historyManager;
}

BookmarksManager *BrowserApplication::bookmarksManager()
{
  if (!s_bookmarksManager)
  {
    s_bookmarksManager = new BookmarksManager;
  }
  return s_bookmarksManager;
}

QIcon BrowserApplication::icon(const QUrl &url) const
{
#if defined(QTWEBENGINE_ICONDATABASE)
  QIcon icon = QWebEngineSettings::iconForUrl(url);
  if (!icon.isNull())
    return icon.pixmap(16, 16);
#else
  Q_UNUSED(url);
#endif
  return defaultIcon();
}

QIcon BrowserApplication::defaultIcon() const
{
  if (m_defaultIcon.isNull())
    m_defaultIcon = QIcon(QLatin1String(":defaulticon.png"));
  return m_defaultIcon;
}

void BrowserApplication::setPrivateBrowsing(bool privateBrowsing)
{
  if (m_privateBrowsing == privateBrowsing)
  {    
    return;
  }  
  
  if (!m_privateProfile)
    m_privateProfile = new QWebEngineProfile(this);
  
  m_privateBrowsing = privateBrowsing;
  if (privateBrowsing)
  {   
    Q_FOREACH (BrowserMainWindow* window, mainWindows())
    {
      window->tabWidget()->setProfile(m_privateProfile);
    }
  }
 
  emit privateBrowsingChanged(privateBrowsing);
}

/// \brief create a custom user-agent-string
///
/// \param[in]  QRect rScreen screen coordinates in pixel
void BrowserApplication::CreateUserAgentString(QRect rScreen)
{
  //description
  //… Wago/Major.Minor.Bugfix (Display=<big, medium, small>,<Portait, Landscape>)
  //Beispiel:
  //Mozilla/5.0 (X11; N; Linux) AppleWebKit/534.34 (KHTML, like Gecko) Qt/4.8.4 Safari/534.34 Wago/00.01.02 (Display=big, Landscape)
  
  //Mozilla/5.0 (X11; N; Linux) AppleWebKit/537.36 (KHTML, like Gecko) QtWebEngine/5.9.4 Chrome/53.0.2785.148 Safari/537.36

  QString sFwVer = "00.00.00";
  ReadSwVersionFromConfigTool(sFwVer);

  QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();
  if (defaultProfile)
  {
    sUserAgent = defaultProfile->httpUserAgent();
  }
  else
  {
    sUserAgent = "Mozilla/5.0 (X11; N; Linux) AppleWebKit/537.36 (KHTML, like Gecko) QtWebEngine/5.9.4 Chrome/56.0.2924.122 Safari/537.36";
  }

  DISPLAY_SIZE displaySize = GetDisplaySize();
  DISPLAY_ORIENTATION displayOrientation = GetDisplayOrientation();

  QString sOrientation = "Landscape";
  if (displayOrientation == DISPLAY_PORTRAIT)
  {
    sOrientation = "Portrait";
  }

  switch (displaySize)
  {
    case DISPLAY_SMALL:
      sUserAgent += " Wago/" + sFwVer + " (Display=small, " + sOrientation + ")";
    break;

    case DISPLAY_MEDIUM:
      sUserAgent += " Wago/" + sFwVer + " (Display=medium, " + sOrientation + ")";
    break;

    case DISPLAY_BIG:
      sUserAgent += " Wago/" + sFwVer + " (Display=big, " + sOrientation + ")";
    break;

  }

}

/// \brief read software version from config-tool
/// \param[out]  sFirmwareVersion
/// \retval true   SUCCESS
/// \retval false  ERROR
bool BrowserApplication::ReadSwVersionFromConfigTool(QString &sFirmwareVersion)
{
  //get plc-list url from config-tool index 1
  bool bRet = false;
  QByteArray output;
  QProcess System_Call;
  System_Call.start("/etc/config-tools/get_coupler_details", QStringList() << "firmware-revision");
  System_Call.waitForFinished(5000);
  output = System_Call.readAllStandardOutput();
  System_Call.close();
  QString sVer = output;

  QUrl u(sVer);
  if (u.isValid())
  {
    sFirmwareVersion = sVer;
    //qDebug() << "webkit: QProcess sFirmwareVersion: " << sFirmwareVersion;
    bRet = true;
  }
  return bRet;
}
