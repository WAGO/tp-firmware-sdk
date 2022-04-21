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
///
/// \file    alarmindication.cpp
///
/// \version $Id: alarmindication.cpp 63391 2021-12-03 08:40:35Z wrueckl_elrest $
///
/// \brief   show connection lost informations
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "alarmindication.h"
#include "globals.h"
#include "tools.h"
#include "msgtool.h"

#include <QProcess>
#include <QX11Info>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

AlarmIndication::AlarmIndication(QWidget *parent) : QWidget(parent)
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

  bLEDEnabled = true;
  m_iWbmBtnState = 1;
  m_pBtnWBM = NULL;

  rScreen = QApplication::desktop()->screenGeometry();
  QSize newSize( rScreen.width(), rScreen.height() );
  setGeometry(QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter,
                                   newSize, QApplication::desktop()->availableGeometry()) );

  ReadWbmBtnState();
  sUrlWbm = "http://127.0.0.1/wbm/index.html";
  m_iNumber = 10;
  int iButtonHeight = 40;

  QString sQtStyleForm = DEFAULT_FORM_STYLE;
  QString sQtStyleButton = DEFAULT_FORM_STYLE;
  QString sBtnText = "WBM";

  DISPLAY_SIZE displaySize = GetDisplaySize();
  QString sPrefix = "small_";
  if (displaySize == DISPLAY_MEDIUM)
  {
    sPrefix = "medium_";
  }
  else if (displaySize == DISPLAY_BIG)
  {
    sPrefix = "big_";
  }

  QString sIdFormStyle = sPrefix + "form_style_alarm";
  QString sIdButtonStyle = sPrefix + "btn_calltoaction_style";

  QSettings settings(CONF_FILE_STYLE, QSettings::IniFormat);
  const QStringList allKeys = settings.allKeys();
  foreach (const QString &key, allKeys)
  {
    QString var = settings.value(key).toString();

    if (var.length() > 0)
    {
      if (key == sIdFormStyle)
      {
        sQtStyleForm = var;
      }
      else if (key == sIdButtonStyle)
      {
        sQtStyleButton = var;
      }
      else if (key == "btn_text_show_wbm")
      {
        sBtnText = var;
      }
      else if (key == QString(sPrefix + "btn_height"))
      {
        iButtonHeight = var.toInt();
      }

    }
    //qDebug() << "webkit: " << key << "  " << var;
  }

  //qDebug() << "sQtStyleForm: " << sQtStyleForm;
  setStyleSheet(sQtStyleForm);
  
  this->style()->unpolish(this);
  this->style()->polish(this);
  this->update();

  QFont font = this->font();

  //Label
  m_pPlainTextEdit = new QPlainTextEdit(this);
  if (m_pPlainTextEdit)
  {
    QRect r;
    m_pPlainTextEdit->setFrameShape(QFrame::NoFrame);
    m_pPlainTextEdit->setReadOnly(true);
    m_pPlainTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
    m_pPlainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pPlainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pPlainTextEdit->setWordWrapMode(QTextOption::WrapAnywhere);
    //m_pPlainTextEdit->setWordWrap(true);

    r.setTop(10);
    r.setLeft(10);
    r.setWidth(width()-20);
    r.setHeight(CalculatePixelHeight(m_pPlainTextEdit->font())*3 + 30);
    m_pPlainTextEdit->setGeometry(r);
    //m_pPlainTextEdit->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  }
  else
  {
    //__ setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
  }

  //WBM Button
  if (m_iWbmBtnState)
  {
    m_pBtnWBM = new QPushButton(sBtnText, this);
    if (m_pBtnWBM)
    {
      m_pBtnWBM->setStyleSheet(sQtStyleButton);

      m_pBtnWBM->style()->unpolish(m_pBtnWBM);
      m_pBtnWBM->style()->polish(m_pBtnWBM);
      m_pBtnWBM->update();

      //m_pBtnWBM->setFont(font);
      m_pBtnWBM->setFocusPolicy(Qt::NoFocus);

      int iBtnWidth = CalculatePixelWidth(m_pBtnWBM->text(), m_pBtnWBM->font()) + 40;
      int iBtnHeight = iButtonHeight; //qMax(iButtonHeight, CalculatePixelHeight(m_pBtnWBM->font()) * 2);
      QSize btnSize(iBtnWidth, iBtnHeight);

      // set size and location of the button
      m_pBtnWBM->setGeometry(QRect(QPoint(rScreen.width() - iBtnWidth - 30, rScreen.height() - iBtnHeight - 20), btnSize));

      connect( m_pBtnWBM ,
               SIGNAL(clicked()),
               this,
               SLOT(OnShowWBM())
             );
    }
    else
    {
      //__ setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }

  //Set QTimer timeout to 1000 milliseconds (Update Number display each second)
  m_pTimer =new QTimer(this);
  if (m_pTimer)
  {
    QObject::connect(m_pTimer ,  SIGNAL(timeout ()),this,SLOT(setNumber()));
  }
  else
  {
    //__ setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
  }

  m_pLcdNum = new QLCDNumber(this);
  if (m_pLcdNum)
  {
    m_pLcdNum->hide();
    m_pLcdNum->display(m_iNumber);
    m_pLcdNum->setDigitCount(2);

    int lcdwidth, lcdheight;
    lcdwidth = rScreen.width() / 4;
    lcdheight = rScreen.height() / 4;

    int x = (rScreen.width() - lcdwidth)/2;
    int y = (rScreen.height() - lcdheight)*3/5;

    m_pLcdNum->setGeometry(QRect(QPoint(x, y), QSize(lcdwidth, lcdheight)));

  }
  else
  {
    //__ setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
  }

}

AlarmIndication::~AlarmIndication()
{
  if (m_pTimer)
    delete (m_pTimer);

  if (m_pLcdNum)
    delete (m_pLcdNum);

  if (m_pPlainTextEdit)
    delete m_pPlainTextEdit;

  if (m_pBtnWBM)
    delete m_pBtnWBM;
}

/// \brief show WBM if button is clicked
///
void AlarmIndication::OnShowWBM()
{
  emit(signalStartWbm(sUrlWbm));
  Hide();
}

/// \brief countdown LCD number on screen
/// \param[in] LCD number start value
///
void AlarmIndication::Countdown(int count)
{
  m_iNumber = count;

  if ((m_pLcdNum)&&(m_pTimer))
  {
    m_pTimer->stop();
    m_pLcdNum->display(m_iNumber);
    m_pLcdNum->show();
    m_pTimer->start(1000);
  }
}

/// \brief set LCD number on screen
///
void AlarmIndication::setNumber()
{
  if (m_pLcdNum)
  {
    m_pLcdNum->display(--m_iNumber);

    if (m_iNumber == 0)
    {
      //m_iNumber = 0;
      m_pTimer->stop();
    }
  }
};

/// \brief show widget
///
void AlarmIndication::Show()
{
  if (!isVisible())
  {
    show();
  }
}

/// \brief hide widget
///
void AlarmIndication::Hide()
{  
  if (isVisible())
  {
    hide();
    m_iNumber = 0;
    if (m_pTimer)
      m_pTimer->stop();
  }  
}

/*
void AlarmIndication::hideEvent (QHideEvent * event)
{
  //moved to mainwindow

  //deactivate disconnected LED
  //if (getRgbLed() == RGB_LED_STATE_BL_BLINK)
  //{
    //QByteArray ba = "PLC connected: " + g_sCurrentUrl.toAscii();
    //setRgbLed(RGB_LED_STATE_GN, ba.data());
  //}
}

void AlarmIndication::showEvent (QShowEvent * event)
{
  //activate disconnected LED
  if (bLEDEnabled)
  {
    QByteArray ba = "PLC disconnected: " + g_sCurrentUrl.toLatin1(); //toAscii();
    //__ setRgbLed(RGB_LED_STATE_BL_BLINK, ba.data());
  }
}
*/

/// \brief set WBM Url
///
void AlarmIndication::SetWbmUrl(QString sUrl)
{  
  sUrlWbm = sUrl;
}

void AlarmIndication::SetLabelText(QString sText)
{
  if (m_pPlainTextEdit)
  {
    m_pPlainTextEdit->setPlainText(sText);
  }
}

void AlarmIndication::ReadWbmBtnState()
{
  tConfList * pList = NULL;
  char szOut[256] = "";
  int ret;

  //create list
  pList = ConfCreateList();
  if (pList)
  {
    ret = ConfReadValues(pList, (char*) CONF_FILE_GESTURE);
    if (ret != SUCCESS)
    {
      ConfDestroyList(pList);
      return;
    }

    if (ConfGetValue(pList, (char*)"btn0", szOut, sizeof(szOut)) == SUCCESS)
    {
       if (strcmp(szOut, "0") == 0)
       {
         m_iWbmBtnState = 0;
       }
       else
       {
         m_iWbmBtnState = 1;
       }
    }

    ConfDestroyList(pList);
  }

}

void AlarmIndication::showEvent(QShowEvent *event)
{
  //DEPRECATED QTimer::singleShot(250, this, SLOT(ActivateX11Window()));
}
