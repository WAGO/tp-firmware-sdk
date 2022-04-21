//------------------------------------------------------------------------------
// Copyright 2019 WAGO Kontakttechnik GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file    toolbarmenu.cpp
///
/// \version $Id: toolbarmenu.cpp 66399 2022-04-07 11:59:58Z wrueckl_elrest $
///
/// \brief   show menu bar in order to activate plc selection list
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include "globals.h"
#include "toolbarmenu.h"

#include "tools.h"

#include <QDebug>
#include <QFontDatabase>

#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMimeData>

#include <QDrag>
#include <QDropEvent>

#include <QEvent>
#include <QTouchEvent>
#include <QProcess>
#include <QX11Info>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include "msgtool.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define DEFAULT_BTN_HEIGHT          40
#define DEFAULT_BTN_WIDTH          140
#define DEFAULT_BTN_SPACE           20

#define MENU_MIN_TIMEOUT          3000

#define MENU_XML_FILE             "/etc/specific/menuqt.xml"
#define CONF_FILE_PLCSELECT       "/etc/specific/plcselect.conf"

//#define URL_PLC_LIST              "http://127.0.0.1/plclist/plclist.html"

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------
bool g_bEventDetection = true;
bool g_bWbmAllowed = true;

ToolbarButton::ToolbarButton(const QString & text, QWidget * parent) :
  QPushButton(text, parent), m_iLeftSpacing(DEFAULT_BTN_SPACE)
{
  m_iHeight = DEFAULT_BTN_HEIGHT;
  m_iWidth = DEFAULT_BTN_WIDTH;
}


//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
ToolBarMenu::ToolBarMenu(QWidget *parent) : QWidget(parent)
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint );
  pCmdThread = NULL;
  m_pSender = NULL;
  m_iFadeHeight = 0;
  m_iBtnCount = 0;

  m_DesktopRect = QApplication::desktop()->screenGeometry();
  m_WndRect = m_DesktopRect;
  
  int iLine = 1; //Mehrzeiligkeit bzw. Zeilenumbruch Portrait

  QString sQtStyleForm = DEFAULT_FORM_STYLE;
  QString sQtStyleCTAButton = DEFAULT_FORM_STYLE;
  m_iInitialWndHeight = 5;
  m_LastPosition.setX(1);
  m_LastPosition.setY(1);

  m_menu.iMenuTimeout = 5;     //seconds
  m_menu.iMarginLeft  = 0;     //percent
  m_menu.iMarginRight = 100;   //percent
  m_menu.orientation = TOOLBAR_TOP;
  m_menu.sLanguage = "en_US";  //locale default value

  ReadPlcList();
  //ReadLanguage(CONF_FILE_SYSTEM);
  parseXML();
  ReadGestureConfigurationFile(CONF_FILE_GESTURE);

  if (!m_menu.sWindowStyle.isEmpty())
    sQtStyleForm = m_menu.sWindowStyle;

  m_iTimeout = qMax(MENU_MIN_TIMEOUT, m_menu.iMenuTimeout * 1000);

  setStyleSheet(sQtStyleForm);

  QFont font = this->font();

  m_pTimer =new QTimer(this);
  if (m_pTimer)
  {
    m_pTimer->setSingleShot(true);
    QObject::connect(m_pTimer ,  SIGNAL(timeout()),this,SLOT(slotTimeout()));
  }
  else
  {
    //setRgbLed(RGB_LED_STATE_RE_BLINK, "menu: not enough memory");
  }

  m_pTimerCursorPos = new QTimer(this);
  if (m_pTimerCursorPos)
  {
    m_pTimerCursorPos->setSingleShot(true);
    QObject::connect(m_pTimerCursorPos ,  SIGNAL(timeout()),this,SLOT(slotTimeoutCursorPos()));
  }

  int xpos = 0;
  int ypos = 0;
  bool bFirst = true;

  for (int i=0; i < m_menu.m_btnList.count(); i++)
  {
    ToolbarButton * pButton = m_menu.m_btnList.at(i);
    if (pButton == NULL)
        continue;

    if (m_menu.m_btnList[i]->m_bVisible == false)
    {
      pButton->setVisible(false);
      //skip this button
      continue;
    }

    m_iBtnCount++;

    //WAT28648 DENIED
    //no VISU Button if no Bootproject found f.e. WP Panel
    //QString sOut;
    //if (ReadBootApp(sOut) == true)
    //{
    //  if (sOut.contains("yes") == false)
    //  {
    //    if (pButton->m_sAction.compare("ID_TARGETVISU", Qt::CaseInsensitive) == 0)
    //    {
    //      //no bootproject, hide VISU Btn ID_TARGETVISU
    //      pButton->setVisible(false);
    //      continue;
    //    }
    //  }
    //}

    if (!pButton->m_sStyle.isEmpty())
      sQtStyleCTAButton = pButton->m_sStyle;
    pButton->setStyleSheet(sQtStyleCTAButton);

    if ((m_menu.orientation == TOOLBAR_LEFT) || (m_menu.orientation == TOOLBAR_RIGHT))
    {
      //show only icon
      if (!pButton->m_sIconFilename.isEmpty())
        pButton->m_sText.clear();
    }
    
    pButton->setText(pButton->m_sText);

    //To load values from Qt Stylesheet you should call this methods
    pButton->style()->unpolish(pButton);
    pButton->style()->polish(pButton);
    pButton->update();

    pButton->setFocusPolicy(Qt::NoFocus);

    int iCalculatedBtnWidth = CalculatePixelWidth(pButton->m_sText, pButton->font()) + 16;
    int iCalculatedBtnHeight = CalculatePixelHeight(pButton->font()) * 2;

    int iBtnWidth = qMax(pButton->m_iWidth, iCalculatedBtnWidth);
    int iBtnHeight = qMax(pButton->m_iHeight, iCalculatedBtnHeight);

    if (!pButton->m_sIconFilename.isEmpty())
    {
      //pButton->setText("");
      QPixmap pix(pButton->m_sIconFilename);
      QIcon icon(pix);
      pButton->setIcon(icon);
      //pButton->setIconSize( QSize(pButton->size().width() - 6, pButton->size().height() - 4));

      //qDebug() << "Available sizes for" << pButton->m_sIconFilename << ":" << icon.availableSizes();     
      
      if (pButton->m_sText.isEmpty())
      {
        //only image without text
        iBtnWidth = pix.width() + 4;
        iBtnHeight = pix.height() + 4;
        pButton->setIconSize(pix.size());
      }
      else
      {
        //Image + Text
        if ((iCalculatedBtnWidth + pButton->iconSize().width()) >= iBtnWidth)
        {
          iBtnWidth += pButton->iconSize().width();
        }
      }
    }

    if (bFirst)
    {
      bFirst = false;
      int wndHeight = iBtnHeight * 2;
      ypos += pButton->m_iLeftSpacing;

      if (m_menu.orientation == TOOLBAR_BOTTOM)
      {
        m_WndRect.setTopLeft(QPoint(0, m_DesktopRect.height() - wndHeight));
        m_WndRect.setHeight(wndHeight);
        m_WndRect.setWidth(m_DesktopRect.width());
      }
      else if (m_menu.orientation == TOOLBAR_LEFT)
      {
        m_WndRect.setTopLeft(QPoint(0,0));
        m_WndRect.setHeight(m_DesktopRect.height());
        m_WndRect.setWidth(wndHeight);
      }
      else if (m_menu.orientation == TOOLBAR_RIGHT)
      {
        m_WndRect.setTopLeft(QPoint(m_DesktopRect.width() - wndHeight, 0));
        m_WndRect.setHeight(m_DesktopRect.height());
        m_WndRect.setWidth(wndHeight);
      }
      else
      {
        //TOOLBAR_TOP
        QSize newSize( m_DesktopRect.width(), wndHeight );
        m_WndRect.setSize(newSize);
      }
      //qDebug() << "toolbar: m_WndRect: " << m_WndRect;
      setGeometry(m_WndRect);
    }

    switch (m_menu.orientation)
    {
    case TOOLBAR_LEFT:
    case TOOLBAR_RIGHT:
      iBtnWidth = m_WndRect.width() - (2*(pButton->m_iLeftSpacing));
      if (iBtnWidth < 10)
        iBtnWidth = m_WndRect.width() - 20;
      if (i==0)
        xpos += pButton->m_iLeftSpacing;
      break;
    default:
      xpos += pButton->m_iLeftSpacing;
      break;
    }
    QSize btnSize(iBtnWidth, iBtnHeight);

    if ((xpos + iBtnWidth) > m_DesktopRect.width())
    {
      //Mehrzeiligkeit
      if (m_menu.orientation == TOOLBAR_TOP)
      {
        if (iLine == 1)
        {
          iLine++;
          xpos = m_menu.m_btnList.at(0)->geometry().x();
          ypos = 0;
          ypos += (pButton->m_iLeftSpacing + iBtnHeight + pButton->m_iLeftSpacing);
          m_WndRect.setHeight(m_WndRect.height() + iBtnHeight + pButton->m_iLeftSpacing);
          setGeometry(m_WndRect);
        }
      }
      else
      {
        //only a single line, break if screen width is passed over
        pButton->setEnabled(false);
        pButton->setVisible(false);
        break;
      }
    }

    // set size and location of the button
    switch (m_menu.orientation)
    {
    case TOOLBAR_LEFT:
    case TOOLBAR_RIGHT:
      pButton->setGeometry(QRect(QPoint(xpos, ypos), btnSize));
      ypos += btnSize.height();
      ypos += pButton->m_iLeftSpacing;
      break;
    case TOOLBAR_BOTTOM:
      pButton->setGeometry(QRect(QPoint(xpos,(m_WndRect.height()-iBtnHeight)/2), btnSize));
      xpos += iBtnWidth;
      break;
    default:
      // top
      //pButton->setGeometry(QRect(QPoint(xpos,(m_WndRect.height()-iBtnHeight)/2), btnSize));
      pButton->setGeometry(QRect(QPoint(xpos,ypos), btnSize));
      xpos += iBtnWidth;
    }

    connect( pButton ,
             SIGNAL(pressed()),
             this,
             SLOT(OnButtonPressed())
           );



  }

  //PlcList Url is placed in /etc/specific/menuqt.xml
  //ReadPLCUrl();

}




ToolBarMenu::~ToolBarMenu()
{
  if (m_pTimer)
    delete (m_pTimer);

  while (m_menu.m_btnList.count() > 0)
  {
    ToolbarButton * pBtn = m_menu.m_btnList.at(0);
    delete pBtn;
    m_menu.m_btnList.removeAt(0);
  }

}


/// \brief read Paneltype from typelabel
/// \param[out]  sOut
/// \retval true   SUCCESS
/// \retval false  ERROR
bool ToolBarMenu::ReadPanelType(QString &sOut)
{      
  bool bRet = false;
  QByteArray output;
  QProcess System_Call;
  System_Call.start("/etc/config-tools/get_typelabel_value", QStringList() << "PANELTYPE");
  System_Call.waitForFinished(5000);
  output = System_Call.readAllStandardOutput();
  System_Call.close();
  sOut = output;
  sOut = sOut.trimmed();
  if (sOut.length() > 0)
  {
    bRet = true;
  }
  //qDebug() << "ReadPanelType: " << sOut;
  return bRet;
}

/// \brief read bootapp state from device
/// \param[out]  sOut (yes or no)
/// \retval true   SUCCESS
/// \retval false  ERROR
bool ToolBarMenu::ReadBootApp(QString &sOut)
{
  bool bRet = false;
  QByteArray output;
  QProcess System_Call;
  System_Call.start("/etc/config-tools/get_eruntime", QStringList() << "bootapp");
  System_Call.waitForFinished(5000);
  output = System_Call.readAllStandardOutput();
  System_Call.close();
  sOut = output;
  sOut = sOut.trimmed();
  if (sOut.length() > 0)
  {
    bRet = true;
  }
  //qDebug() << "ReadBootApp: " << sOut;
  return bRet;
}

/// \brief slot timeout received
/// hide menu
///
void ToolBarMenu::slotTimeout()
{
#ifdef DEBUG_MSG
  qDebug() << "Hide: ToolBarMenu::slotTimeout";
#endif
  //qDebug() << "Hide " << __LINE__;
  Hide();
}

/// \brief slot timeout received
/// hide menu
///
void ToolBarMenu::slotTimeoutCursorPos()
{
  if (m_pTimer->isActive())
  {
    unsigned int msecs = m_LastSlideEvent.time().msecsTo(QDateTime::currentDateTime().time());
    if (msecs >= 1000)
    {
      bool bShow = true;
      switch (m_menu.orientation)
      {
      case TOOLBAR_BOTTOM:
        if (height() <= (m_WndRect.height()/2))
        {
          //close
          bShow = false;
        }
        break;
      case TOOLBAR_LEFT:
        if (width() <= (m_WndRect.width()/2))
        {
          //close
          bShow = false;
        }
        break;
      case TOOLBAR_RIGHT:
        if (width() <= (m_WndRect.width()/2))
        {
          //close
          bShow = false;
        }
        break;
      default:
        if (height() <= (m_WndRect.height()/2))
        {
          //close
          bShow = false;
        }
      }

      if (bShow)
      {
        //open full
        setGeometry(m_WndRect);
        RestartTimeout();
      }
      else
      {
        //qDebug() << "Hide " << __LINE__;
        Hide();
      }
    }
    else
    {
      Slide(QCursor::pos());
      RestartTimeoutCursorPos();
    }
  }
}

/// \brief show menu
///
void ToolBarMenu::Show()
{
  // check bounds (margin left and right)
  QPoint pnt = QCursor::pos();
  int xLeft  = m_DesktopRect.width() * m_menu.iMarginLeft / 100.00;
  int xRight = m_DesktopRect.width() * m_menu.iMarginRight / 100.00;

  int yTop  = m_DesktopRect.height() * m_menu.iMarginLeft / 100.00;
  int yBottom = m_DesktopRect.height() * m_menu.iMarginRight / 100.00;

  if ((m_menu.orientation == TOOLBAR_LEFT) || (m_menu.orientation == TOOLBAR_RIGHT))
  {
    if (pnt.y() < yTop || pnt.y() > yBottom)
    {
      //leave, position x is not in configured area
      return;
    }
  }
  else
  {
    if (pnt.x() < xLeft || pnt.x() > xRight)
    {
      //leave, position x is not in configured area
      return;
    }
  }


  QRect r;
  if (m_menu.orientation == TOOLBAR_BOTTOM)
  {
    r.setTopLeft(QPoint(0, m_DesktopRect.height() - m_iInitialWndHeight));
    r.setHeight(m_iInitialWndHeight);
    r.setWidth(m_DesktopRect.width());
  }
  else if (m_menu.orientation == TOOLBAR_LEFT)
  {
    r.setTopLeft(QPoint(0,0));
    r.setHeight(m_DesktopRect.height());
    r.setWidth(m_iInitialWndHeight);
  }
  else if (m_menu.orientation == TOOLBAR_RIGHT)
  {
    r.setTopLeft(QPoint(m_DesktopRect.width() - m_iInitialWndHeight, 0));
    r.setHeight(m_DesktopRect.height());
    r.setWidth(m_iInitialWndHeight);
  }
  else
  {
    //TOOLBAR_TOP
    r.setTopLeft(QPoint(0,0));
    r.setHeight(m_iInitialWndHeight);
    r.setWidth(m_DesktopRect.width());
  }

  setGeometry(r);

  m_LastSlideEvent = QDateTime::currentDateTime();

  show();

  RestartTimeout();
  RestartTimeoutCursorPos();
}


/// \brief hide menu
///
void ToolBarMenu::Hide()
{
  if (m_pTimer->isActive())
    m_pTimer->stop();

  if (m_pTimerCursorPos->isActive())
    m_pTimerCursorPos->stop();

  if (isVisible())
  {
    hide();
    if (g_bEventDetection)
      emit signalMinimize();
  }

}


/// \brief handle button
///
void ToolBarMenu::OnButtonPressed()
{
  m_pSender = QObject::sender();
  //call async to avoid leaving buttons in color mode clicked
  QTimer::singleShot(100, this, SLOT(OnHide()));
  QTimer::singleShot(150, this, SLOT(OnAction()));
}


bool ToolBarMenu::Slide(QPoint pos)
{
  bool bStop = false;

  switch (m_menu.orientation)
  {
  case TOOLBAR_BOTTOM:
    if ((height() >= m_WndRect.height()))
      bStop = true;
    break;
  case TOOLBAR_LEFT:
    if ((width() >= m_WndRect.width()))
      bStop = true;
    break;
  case TOOLBAR_RIGHT:
    if ((width() >= m_WndRect.width()))
      bStop = true;
    break;
  default:
    //TOP
    if ((height() >= m_WndRect.height()))
      bStop = true;
  }

  if (bStop)
  {
    //qDebug() << "ToolBarMenu::Slide: max height reached";
    m_pTimerCursorPos->stop();
    return false;
  }


  if (pos != m_LastPosition)
  {
    m_LastSlideEvent = QDateTime::currentDateTime();
    QRect r = geometry();

    //qDebug() << "Slide: " << pos;

    switch (m_menu.orientation)
    {
    case TOOLBAR_BOTTOM:
      r.setTopLeft(QPoint(0, pos.y()));
      r.setHeight(qBound(m_iInitialWndHeight, m_DesktopRect.height() - pos.y(), m_WndRect.height()));
      r.setWidth(m_DesktopRect.width());
      //r.setHeight(m_DesktopRect.height() - pos.y());
      break;
    case TOOLBAR_LEFT:
      r.setWidth(qBound(m_iInitialWndHeight, pos.x(), m_WndRect.width()));
      //r.setWidth(pos.x());
      break;
    case TOOLBAR_RIGHT:
    {
      int xWidth = m_DesktopRect.width() - pos.x();
      r.setTopLeft(QPoint(pos.x(), 0));
      r.setHeight(m_DesktopRect.height());
      r.setWidth(qBound(m_iInitialWndHeight, xWidth, m_WndRect.width()));
      break;
    }
    default:
      //TOP
      r.setHeight(qBound(m_iInitialWndHeight, pos.y(), m_WndRect.height()));
    }
    //qDebug() << "r: " << r;
    setGeometry(r);
    //if (! isVisible()) show();

    RestartTimeout();
  }

  m_LastPosition = pos;

  return true;
}



/// \brief receive events
///
bool ToolBarMenu::event(QEvent *e)
{

  switch (e->type())
  {
  //case QEvent::Enter:
  case QEvent::Leave:
  //case QEvent::DragEnter:
  //case QEvent::DragLeave:
  case QEvent::FocusOut:
  //case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  {
    unsigned int msecs = m_LastSlideEvent.time().msecsTo(QDateTime::currentDateTime().time());
    //qDebug() << e << " msecs: " << msecs;
    if (msecs >= 300)
    {
      switch (m_menu.orientation)
      {
      case TOOLBAR_BOTTOM:
        if ((height() >= m_WndRect.height()))
        {
          //qDebug() << "Hide " << __LINE__;
          QTimer::singleShot(250, this, SLOT(OnHide()));
        }
        break;
      case TOOLBAR_LEFT:
        if ((width() >= m_WndRect.width()))
        {
          //qDebug() << "Hide " << __LINE__;
          QTimer::singleShot(250, this, SLOT(OnHide()));
        }
        break;
      case TOOLBAR_RIGHT:
        if ((width() >= m_WndRect.width()))
        {
          //qDebug() << "Hide " << __LINE__;
          QTimer::singleShot(250, this, SLOT(OnHide()));
        }
        break;
      default:
        //TOOLBAR_TOP
        if ((height() >= m_WndRect.height()))
        {
          //qDebug() << "Hide " << __LINE__;
          QTimer::singleShot(250, this, SLOT(OnHide()));
        }

      }
    }

    break;
  }

  default:
    break;
  }
  return QWidget::event(e);
}


/*
/// \brief read PLC-List url from config-tool
/// \retval true  SUCCESS
/// \retval false ERROR
///
bool ToolBarMenu::ReadPLCUrl()
{
  //get plc-list url from config-tool index 1
  bool bRet = false;

  QString sUrl = ReadPlcListUrl(CONF_FILE_WEBENGINE);
  QUrl u(sUrl);
  if (u.isValid())
  {
    g_sUrlPLCList = sUrl;
#ifdef DEBUG_MSG
    qDebug() << "menu: QProcess g_sUrlPLCList: " << g_sUrlPLCList;
#endif
    bRet = true;
  }
  return bRet;
}
*/


/*
void ToolBarMenu::touchEvent(QTouchEvent *ev)
{
  switch (ev->type())
  {
  case QEvent::TouchBegin:
    qDebug() << "Touch Event began.";
    break;
  case QEvent::TouchEnd:
    qDebug() << "Touch Event ended.";
    break;
  case QEvent::TouchCancel:
    qDebug() << "Touch Event canceled.";
    break;
  case QEvent::TouchUpdate:
    qDebug() << "Touch Event updated.";
    break;
  }
}
*/

void ToolBarMenu::RestartTimeout()
{
  if (m_pTimer->isActive())
  {
    m_pTimer->stop();
  }
  m_pTimer->setSingleShot(true);
  m_pTimer->start(m_iTimeout);
  //qDebug() << "RestartTimeout";
}


void ToolBarMenu::RestartTimeoutCursorPos()
{
  if (m_pTimerCursorPos->isActive())
  {
    m_pTimerCursorPos->stop();
  }

  if ((m_menu.orientation == TOOLBAR_LEFT)||(m_menu.orientation == TOOLBAR_RIGHT))
  {
    if ((width() < m_WndRect.width()))
    {
      m_pTimerCursorPos->setSingleShot(true);
      m_pTimerCursorPos->start(50);
      //qDebug() << "RestartTimeout";
    }
  }
  else
  {
    //top or bottom
    if ((height() < m_WndRect.height()))
    {
      m_pTimerCursorPos->setSingleShot(true);
      m_pTimerCursorPos->start(50);
      //qDebug() << "RestartTimeout";
    }
  }
}

bool ToolBarMenu::IsCompleteVisible()
{
  if (height() < m_WndRect.height())
  {
    return false;
  }
  else
  {
    return true;
  }
}


/// \brief parse virtualkeyboard.xml file
/// read panels, layout, key definitions
///
void ToolBarMenu::parseXML()
{
  DISPLAY_SIZE displaySize = GetDisplaySize();
  QString sIdentifier = "small";
  if (displaySize == DISPLAY_MEDIUM)
  {
    sIdentifier = "medium";
  }
  else if (displaySize == DISPLAY_BIG)
  {
    sIdentifier = "big";
  }

  if (m_WndRect.width() < 300)
  {
    sIdentifier = "small_portrait";
  }

  QFile* file = new QFile(QString(MENU_XML_FILE));
  if (!file)
  {
    //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    return;
  }

  if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
  {

    //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: could not read virtual keyboard xml file");
    return;

  }
  QXmlStreamReader xml(file);

  bool bSection = false;

  while (!xml.atEnd() && !xml.hasError())
  {
    /* Read next element.*/
    QXmlStreamReader::TokenType token = xml.readNext();
    /* If token is just StartDocument, we'll go to next.*/
    if (token == QXmlStreamReader::StartDocument)
    {
      continue;
    }
    /* If token is StartElement, we'll see if we can read it.*/
    if (token == QXmlStreamReader::StartElement)
    {

      if (xml.name().compare(sIdentifier, Qt::CaseInsensitive) == 0)
      {
        bSection = true;
        //qDebug() << "StartElement: " << xml.tokenString() << " xml.name(): " << xml.name();
        //continue;
      }

      if (bSection)
      {
        if (xml.name() == "item")
        {
          parseItem(xml);
        }
        else if (xml.name() == "menu_slide_px_height")
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            m_iInitialWndHeight = xml.text().toString().toInt();
            //qDebug() << "menuqtslide: " << "m_iInitialWndHeight " << m_iInitialWndHeight;
          }
        }
        else if (xml.name() == "qt_form_style")
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            m_menu.sWindowStyle = xml.text().toString();
            //qDebug() << "menuqtslide: " << "sWindowStyle " << m_menu.sWindowStyle;
          }
        }
      }
      else
      {
        if (xml.name() == "globals")
        {
          parseGlobals(xml);
        }
        else if (xml.name() == "translation")
        {
          parseTranslation(xml);
        }
      }

    }
    else if (token == QXmlStreamReader::EndElement)
    {


      if (xml.name().compare(sIdentifier, Qt::CaseInsensitive) == 0)
      {
        //qDebug() << "EndElement: " << xml.tokenString() << " xml.name(): " << xml.name();
        bSection = false;
      }

    }
  }
  // Error handling. 
  if (xml.hasError())
  {
    qDebug() << "menuqtslide: " << "xml parse errors";
  }
  // Removes any device() or data from the reader and resets its internal state to the initial state. 
  xml.clear();

  if (file)
  {
    file->close();
    delete file;
  }

}

/// \brief parse layout from xml file
///
void ToolBarMenu::parseItem(QXmlStreamReader &xml)
{
  if (xml.tokenType() != QXmlStreamReader::StartElement
      && xml.name() == "item")
  {
    return;
  }

  // Next element...
  xml.readNext();

  ToolbarButton * pItemButton = new ToolbarButton("", this);
  m_menu.m_btnList.append(pItemButton);

  while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "item"))
  {
    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "width")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        pItemButton->m_iWidth = xml.text().toString().toInt();
        //qDebug() << "m_iWidth: " << pItemButton->m_iWidth;
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "height")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        pItemButton->m_iHeight = xml.text().toString().toInt();
        //qDebug() << "m_iHeight: " << pItemButton->m_iHeight;
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "leftspacing")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        pItemButton->m_iLeftSpacing = xml.text().toString().toInt();
        //qDebug() << "m_iLeftSpacing: " << pItemButton->m_iLeftSpacing;
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "text")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        pItemButton->m_sText = xml.text().toString();
        //ID_WBM
        //ID_PLCLIST
        //ID_TARGETVISU
        //ID_DISPLAYCLEANING
        if (g_bWbmAllowed == false)
        {
          if (pItemButton->m_sText.compare("ID_WBM", Qt::CaseInsensitive) == 0)
          {
            pItemButton->setEnabled(false);
          }
          else if (pItemButton->m_sText.compare("ID_PLCLIST", Qt::CaseInsensitive) == 0)
          {
            pItemButton->setEnabled(false);
          }
          else if (pItemButton->m_sText.compare("ID_TARGETVISU", Qt::CaseInsensitive) == 0)
          {
            pItemButton->setEnabled(false);
          }
        }

        //qDebug() << "m_sText: " << pItemButton->m_sText;
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "style")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        pItemButton->m_sStyle = xml.text().toString();
        //qDebug() << "m_sStyle: " << pItemButton->m_sStyle;
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "icon")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        pItemButton->m_sIconFilename = xml.text().toString();
        //qDebug() << "m_sIconFilename: " << pItemButton->m_sIconFilename;
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "action")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        pItemButton->m_sAction = xml.text().toString();
        pItemButton->m_sAction = pItemButton->m_sAction.trimmed();        
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::Invalid)
    {
      break;
    }

    // ...and next...
    xml.readNext();
  }
  return;
}



/// \brief parse layout from xml file
///
void ToolBarMenu::parseGlobals(QXmlStreamReader &xml)
{
  if (xml.tokenType() != QXmlStreamReader::StartElement
      && xml.name() == "globals")
  {
    return;
  }

  // Next element...
  xml.readNext();

  QString s;

  while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "globals"))
  {
    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "menu_timeout")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        m_menu.iMenuTimeout = xml.text().toString().toInt();
        //qDebug() << "iMenuTimeout: " << m_menu.iMenuTimeout;
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "margin_left")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        m_menu.iMarginLeft = qBound(0, xml.text().toString().toInt(), 90); //between 0 and 90 %
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "margin_right")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        m_menu.iMarginRight = qBound(10, xml.text().toString().toInt(), 100); //between 10 and 100 %
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "orientation")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        QString s = xml.text().toString().toLower();
        if (s.compare("left")== 0)
        {
          m_menu.orientation = TOOLBAR_LEFT;
        }
        else if (s.compare("right")== 0)
        {
          m_menu.orientation = TOOLBAR_RIGHT;
        }
        else if (s.compare("bottom")== 0)
        {
          m_menu.orientation = TOOLBAR_BOTTOM;
        }
        else
        {
          m_menu.orientation = TOOLBAR_TOP;
        }
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::Invalid)
    {
      break;
    }

    // ...and next...
    xml.readNext();
  }
  return;
}

void ToolBarMenu::ReadGestureConfigurationFile(QString sFile)
{
  int iPosBtn = 0;
  int iPosId = 0;

// startup configuration settings
  QSettings settings(sFile, QSettings::IniFormat);
  const QStringList allKeys = settings.allKeys();
  foreach (const QString &key, allKeys)
  {
    QString var = settings.value(key).toString();
    QString sBtn = "btn" + QString::number(iPosBtn);
    QString sId = "id" + QString::number(iPosId);
    if (var.length() > 0)
    {
      if (key == "menu-timeout")
      {
        m_menu.iMenuTimeout = var.toInt();
      }
      else if (key == "menu-margin-left")
      {
        m_menu.iMarginLeft = qBound(0, var.toInt(), 90); //between 0 and 90 %
      }
      else if (key == "menu-margin-right")
      {
        m_menu.iMarginRight = qBound(10, var.toInt(), 100); //between 10 and 100 %
      }
      else if (key == "menu-orientation")
      {
        QString s = var;
        if (s.compare("left")== 0)
        {
          m_menu.orientation = TOOLBAR_LEFT;
        }
        else if (s.compare("right")== 0)
        {
          m_menu.orientation = TOOLBAR_RIGHT;
        }
        else if (s.compare("bottom")== 0)
        {
          m_menu.orientation = TOOLBAR_BOTTOM;
        }
        else
        {
          m_menu.orientation = TOOLBAR_TOP;
        }
      }
      else if (key == sBtn)
      {
        //visibility 0 or 1
        QString s = var;
        if (iPosBtn < m_menu.m_btnList.count())
        {
          if (s.compare("1")== 0)
          {
            m_menu.m_btnList[iPosBtn]->m_bVisible = true;
          }
          else
          {
            m_menu.m_btnList[iPosBtn]->m_bVisible = false;
          }
        }

        iPosBtn++;
      }
      else if (key == sId)
      {
        //text id
        int idx = m_menu.slLeft.indexOf(var);
        if (idx >= 0)
        {
          for (int i=0; i < m_menu.m_btnList.count(); i++)
          {
            if (m_menu.m_btnList[i]->m_sText.compare(var, Qt::CaseInsensitive) == 0)
            {
              m_menu.m_btnList[i]->m_sText = m_menu.slRight[idx];
              break;
            }
          }
        }
        
        iPosId++;
      }

    }
  }
}

void ToolBarMenu::ReadLanguage(QString sFile)
{

// startup configuration settings
  QSettings settings(sFile, QSettings::IniFormat);
  const QStringList allKeys = settings.allKeys();
  foreach (const QString &key, allKeys)
  {
    QString var = settings.value(key).toString();

    if (var.length() > 0)
    {
      if (key == "language")
      {
        m_menu.sLanguage = var.trimmed();
      }

    }
  }
}

/*
QString ToolBarMenu::ReadPlcListUrl(QString sFile)
{
  QString sRet;
  // startup configuration settings
  QSettings settings(sFile, QSettings::IniFormat);
  const QStringList allKeys = settings.allKeys();
  foreach (const QString &key, allKeys)
  {
    QString var = settings.value(key).toString();

    if (var.length() > 0)
    {
      //url0 is reserved for WBM
      //url1 is reserved for PLCList
      if (key == "url1")
      {
        sRet = var.trimmed();
        break;
      }

    }
  }
  return sRet;
}
*/

/// \brief parse layout from xml file
///
void ToolBarMenu::parseTranslation(QXmlStreamReader &xml)
{
  if (xml.tokenType() != QXmlStreamReader::StartElement
      && xml.name() == "translation")
  {
    return;
  }

  // Next element...
  xml.readNext();

  QString s;

  while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "translation"))
  {
    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "textid")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        m_menu.slLeft << xml.text().toString();
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == m_menu.sLanguage)
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        m_menu.slRight << xml.text().toString();
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::Invalid)
    {
      break;
    }

    // ...and next...
    xml.readNext();
  }
  return;
}

/// \brief starting cmd thread
///
void ToolBarMenu::startCmdThread()
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
          SLOT(cmdSlotReceived(QString)) );

  //start thread function run
  pCmdThread->start(QThread::NormalPriority);

}

/// \brief cmd message string received
///
void ToolBarMenu::cmdSlotReceived(QString s)
{
  //qDebug() << "cmdSlotReceived: " << s;

  if (s.left(4).compare("open", Qt::CaseInsensitive) == 0)
  {
    if (g_bEventDetection)
    {
      //open full
      setGeometry(m_WndRect);
      RestartTimeout();
      show();
      m_LastSlideEvent = QDateTime::currentDateTime();
    }
    else
    {
      m_iFadeHeight = 0;
      QTimer::singleShot(15, this, SLOT(OnFadeIn()));
    }
    //QTimer::singleShot(150, this, SLOT(ActivateX11Window()));
  }
}

bool ToolBarMenu::FindProcess(const char * pszName)
{
   bool bRet = false;
   const char* directory = "/proc";
   size_t      taskNameSize = 1024;
   char*       taskName = (char*) calloc(1, taskNameSize);

   DIR* dir = opendir(directory);

   if (dir)
   {
      struct dirent* de = 0;

      while ((de = readdir(dir)) != 0)
      {
         if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

         int pid = -1;
         int res = sscanf(de->d_name, "%d", &pid);

         if (res == 1)
         {
            // we have a valid pid

            // open the cmdline file to determine what's the name of the process running
            char cmdline_file[1024] = {0};
            sprintf(cmdline_file, "%s/%d/cmdline", directory, pid);

            FILE* cmdline = fopen(cmdline_file, "r");
            if (cmdline)
            {
              if (getline(&taskName, &taskNameSize, cmdline) > 0)
              {
                // is it the process we care about?
                if (strstr(taskName, pszName) != 0)
                {
                  //fprintf(stdout, "A %s process, with PID %d, has been detected.\n", pszName, pid);
                  //found
                  bRet = true;
                }
              }
              
              fclose(cmdline);
            }
         }
         if (bRet)
           break;
      }

      closedir(dir);
   }

   free(taskName);

   return bRet;
}

void ToolBarMenu::OnFadeIn()
{
    QRect r = m_WndRect;

    if (m_iFadeHeight == 0)
        m_iFadeHeight = 10;
    else
        m_iFadeHeight += 4;

    r.setHeight(qMin(m_iFadeHeight,m_WndRect.height()));
    setGeometry(r);
    show();
    update();

    if (m_iFadeHeight >= m_WndRect.height())
    {
      m_iFadeHeight = 0;
      RestartTimeout();
      m_LastSlideEvent = QDateTime::currentDateTime();
    }
    else
    {
      QTimer::singleShot(15, this, SLOT(OnFadeIn()));
    }
}

void ToolBarMenu::OnHide()
{
  //qDebug() << "OnHide";
  Hide();
}

void ToolBarMenu::ReadPlcList()
{
  //defaults
  m_sUrlWbm = "http://127.0.0.1/wbm/index.html";
  m_sUrlPlc = "http://127.0.0.1/plclist/plclist.html";
  m_bVkbWbm = true;

  QSettings settings(CONF_FILE_PLCSELECT, QSettings::IniFormat);
  const QStringList allKeys = settings.allKeys();
  foreach (const QString &key, allKeys)
  {
    QString var = settings.value(key).toString();
    if (var.length() > 0)
    {
      if ((key == "url0") || (key == "url00"))
      {
         m_sUrlWbm = var;
      }
      else if ((key == "url1") || (key == "url01"))
      {
         m_sUrlPlc = var;
      }
      else if ((key == "vkb0") || (key == "vkb00"))
      {
        if (var.contains("disabled", Qt::CaseInsensitive))
        {
          m_bVkbWbm = false;
        }
      }
    }
  }
}

void ToolBarMenu::OnAction()
{

  //qDebug() << "OnAction: " << m_pSender;

  if (!m_pSender)
    return;

  ToolbarButton * pButton = (ToolbarButton *) m_pSender;
  char szCmd[256] = "";

  if (!pButton)
    return;

  pButton->setDown(false);

  for (int k = 0; k < m_menu.m_btnList.count(); k++)
  {
    if (m_pSender == m_menu.m_btnList.at(k))
    {
      QString sAction;
      if (pButton->m_sAction.compare("ID_WBM", Qt::CaseInsensitive) == 0)
      {
        //skip if not allowed
        if (g_bWbmAllowed == false)
          continue;

        if (FindProcess("webenginebrowser") == true)
        {
           if (m_bVkbWbm)
           {
             char * pCmd = "enable\n";
             Write2PipedFifo(DEV_VIRTUALKEYBOARD, pCmd);
           }

           QByteArray ba;
           ba.append(m_sUrlWbm);
           //browser already running
           sprintf(szCmd, "load=%s\n", ba.data());
           Write2PipedFifo(DEV_WEBENGINEBROWSER, szCmd);
           sAction = "/etc/script/show_webbrowser.sh";
        }
        else
        {
           sAction = "/etc/script/start_wbm.sh";
        }
      }
      else if (pButton->m_sAction.compare("ID_PLCLIST", Qt::CaseInsensitive) == 0)
      {
        if (FindProcess("webenginebrowser") == true)
        {
          QByteArray ba;
          ba.append(m_sUrlPlc);
          //browser already running
          sprintf(szCmd, "load=%s\n", ba.data());
          Write2PipedFifo(DEV_WEBENGINEBROWSER, szCmd);
          sAction = "/etc/script/show_webbrowser.sh";
        }
        else
        {
          sAction = "/etc/script/start_plclist.sh";
        }
      }
      else if (pButton->m_sAction.compare("ID_TARGETVISU", Qt::CaseInsensitive) == 0)
      {
        sAction = "/etc/script/start_targetvisu.sh";
      }
      else if (pButton->m_sAction.compare("ID_DISPLAYCLEANING", Qt::CaseInsensitive) == 0)
      {
        sAction = "/usr/bin/clear_screen";
      }

      //call to action
      if (!sAction.isEmpty())
      {
        if (sAction.right(1) != "&")
        {
          sAction.append(" &");
        }
        QByteArray ba = sAction.toLatin1();
        system(ba.data());
      }

      break;
    }
  }

}

void ToolBarMenu::showEvent(QShowEvent *event)
{
}
