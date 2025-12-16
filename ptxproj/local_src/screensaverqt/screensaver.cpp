//------------------------------------------------------------------------------
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
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
/// \file    screensaver.cpp
///
/// \version $Id$
///
/// \brief   screensaver Qt tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include "screensaver.h"
#include <QWidget>
#include <QApplication>
#include <QMainWindow>
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
#include <QScreen>
#include <QRect>
#else
#include <QDesktopWidget>
#endif
#include <QTextStream>
#include <QProcess>
#include <QPainter>
#include <QTime>
#include <QFile>
#include <QDateTime>
#include <QEvent>
#include <QRandomGenerator>
//#include <QDebug>

ScreenSaver::ScreenSaver(QWidget *parent) :
    QMainWindow(parent)
{
  m_pTimer = NULL;
  m_iReturnValue = 0;
  m_iThemeId = 0;
  m_iCounter = 0;
  m_bFirst = true;

#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
  //screen dimensions
  QRect r = QApplication::primaryScreen()->geometry();
#else
  //screen dimensions
  QRect r = QApplication::desktop()->screenGeometry();
#endif
  m_iScreenWidth = r.width();
  m_iScreenHeight = r.height();

  setAutoFillBackground(false);

  //setWindowState(Qt::WindowFullScreen);
  //setWindowModality(Qt::ApplicationModal);
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

  setMouseTracking(true);

}


ScreenSaver::~ScreenSaver()
{
  if (m_pTimer)
    delete (m_pTimer);
}

/// \brief initialize screensaver
///
void ScreenSaver::Initialize()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
  srand(QTime::currentTime().msec());
#else
  qsrand(QTime::currentTime().msec());
#endif

  if (m_iThemeId == THEME_IMAGE)
  {
    QFile sFile(m_sParameter);
    if (sFile.exists())
    {
      if(!m_image.load(m_sParameter))
      {
        m_iThemeId = THEME_NONE;
      }
      else
      {
        if ((m_image.width() >= (m_iScreenWidth - 10)) || (m_image.height() >= (m_iScreenHeight - 10)))
        {
          //qDebug() << "QImage SCALED";
          m_scaledImage = m_image.scaledToHeight(m_iScreenHeight * 0.68);
          if (m_scaledImage.width() >= (m_iScreenWidth-10))
          {
            m_scaledImage = m_image.scaledToWidth(m_iScreenWidth * 0.68);
          }
          m_image = m_scaledImage;
        }
      }
    }
  } 

  //set background
  if (m_iThemeId == THEME_NONE)
  {
    QString sImage; // /home/user/logo.png
    ReadImageFileNameFromConfigTool(sImage);

    QString sStyle;
    sStyle = "background-image: url("+ sImage +"); background-repeat:no-repeat; background-position:center";
    //QMessageBox msgBox;
    //msgBox.setText(sStyle);
    //msgBox.exec();

    //transparent in backlight mode
    //setStyleSheet("background:transparent;");
    setStyleSheet(sStyle);

    //setStyleSheet("background-color:green;");
    //setAttribute(Qt::WA_NoSystemBackground);
    //setAttribute(Qt::WA_TranslucentBackground);
    //setAttribute(Qt::WA_OpaquePaintEvent);

    //setWindowOpacity(0.0);
  }
  else
  {            
    //black in bitmap / theme mode
    setStyleSheet("background-color: black; color:white;");

    QFont fnt = this->font();
    fnt.setPointSize(18);
    fnt.setBold(true);
    this->setFont(fnt);

  }

  //Set QTimer timeout to 1000 milliseconds (Update Number display each second)
  m_pTimer =new QTimer(this);
  if ((m_pTimer) && (m_iThemeId != THEME_NONE))
  {
      QObject::connect(m_pTimer ,  SIGNAL(timeout ()),this,SLOT(setPosition()));
      if (m_iThemeId == THEME_TIME)
      {
        m_pTimer->start(1000);
      }
      else
      {
        m_pTimer->start(5000);
      }
  }

  installEventFilter(this);

  resize(m_iScreenWidth, m_iScreenHeight);
  setFocus();
}

/// \brief listen on events
/// \param[in] obj pointer to object
/// \param[in] event pointer to event
/// \retval false process event
/// \retval true do not process event
bool ScreenSaver::eventFilter(QObject *obj, QEvent *event)
{
  int type = event->type();
  switch (type)
  {
    case QEvent::MouseButtonRelease:
    case QEvent::KeyRelease:
      m_iReturnValue = 1;
      QApplication::exit(m_iReturnValue);
      break;
    default:
      break;
  }
  return false;
}

/// \brief listen on mouse events
/// reimplementation
/// \param[in] event pointer to QMouseEvent
void ScreenSaver::mouseMoveEvent(QMouseEvent * event)
{
  //qDebug() << event->type();
  if (m_bFirst)
  {
    m_bFirst = false;
  }
  else
  {
    m_iReturnValue = 1;
    QApplication::exit(m_iReturnValue);
  }
}

/// \brief update screen
/// refresh position
void ScreenSaver::setPosition()
{
   update();
};

/// \brief on paint 
/// \param[in]  event pointer to QPaintEvent
void ScreenSaver::paintEvent(QPaintEvent*)
{
  if (m_iThemeId == THEME_IMAGE)
  {
    QPainter painter(this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    m_xRandom = rand() % (m_iScreenWidth - m_image.width() - 2);
    m_yRandom = rand() % (m_iScreenHeight - m_image.height() - 2);
#else
    m_xRandom = qrand() % (m_iScreenWidth - m_image.width() - 2);
    m_yRandom = qrand() % (m_iScreenHeight - m_image.height() - 2); 
#endif
    m_xRandom++;
    m_yRandom++;

    QRect r;
    r.setX(m_xRandom);
    r.setY(m_yRandom);
    r.setWidth(m_image.width());
    r.setHeight(m_image.height());

    painter.drawImage(r, m_image);

  }
  else if (m_iThemeId == THEME_TIME)
  {
      QPainter painter(this);

      QTime t = QTime::currentTime();
      QString s = t.toString();

      int iPixelWidth = CalculatePixelWidth(s, font());
      int iPixelHeight = CalculatePixelHeight(font());

      if ((m_iCounter % 5) == 0)
      {
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
        m_xRandom = rand() % (m_iScreenWidth - iPixelWidth - 2);
        m_yRandom = rand() % (m_iScreenHeight - iPixelHeight - 2);
#else
        m_xRandom = qrand() % (m_iScreenWidth - iPixelWidth - 2);
        m_yRandom = qrand() % (m_iScreenHeight - iPixelHeight - 2);
#endif
        m_xRandom++;
        m_yRandom++;
      }

      QRect r;
      r.setX(m_xRandom);
      r.setY(m_yRandom);
      r.setWidth(iPixelWidth);
      r.setHeight(iPixelHeight);

      painter.drawText(r, s);

      m_iCounter++;

  }
  else if (m_iThemeId == THEME_TEXT)
  {
      QPainter painter(this);

      int iPixelWidth = qMin(CalculatePixelWidth(m_sParameter, font()), m_iScreenWidth - 2);
      int iPixelHeight = CalculatePixelHeight(font());

      m_xRandom = QRandomGenerator::global()->bounded(2, m_iScreenWidth - iPixelWidth);
      m_yRandom = QRandomGenerator::global()->bounded(2, m_iScreenHeight - iPixelHeight);

      QRect r;
      r.setX(m_xRandom);
      r.setY(m_yRandom);
      r.setWidth(iPixelWidth);
      r.setHeight(iPixelHeight);

      painter.drawText(r, m_sParameter);
  }

}

/// \brief calculate string width depending on font
///
/// \param[in] sText    calculating width for this string
/// \param[in] fnt      calculating width using this font
/// \retval  pixel width of the string
int ScreenSaver::CalculatePixelWidth(QString sText, QFont fnt)
{
    //QFont font("times", 24);
    QFontMetrics fm(fnt);
    int pixelsWide = fm.horizontalAdvance(sText);
    return (pixelsWide + 2);
}

/// \brief calculate height depending on font
///
/// \param[in] fnt      calculating width using this font
/// \retval  pixel height of the font
int ScreenSaver::CalculatePixelHeight(QFont fnt)
{
    QFontMetrics fm(fnt);
    int pixelsHigh = fm.height();
    return (pixelsHigh + 1);
}

/// \brief read image file name from config-tool
/// \param[out]  sImageFileName
/// \retval true   SUCCESS
/// \retval false  ERROR
bool ScreenSaver::ReadImageFileNameFromConfigTool(QString & sImageFileName)
{
  bool bRet = false;
  QByteArray output;
  QProcess System_Call;
  System_Call.start("/etc/config-tools/get_screensaver", QStringList() << "image");
  System_Call.waitForFinished(5000);
  output = System_Call.readAllStandardOutput();
  System_Call.close();

  if (output.count() > 0)
  {
    sImageFileName = output;
    bRet = true;
  }

  return bRet;
}

void ScreenSaver::showEvent(QShowEvent *event)
{
  //DEPRECATED QTimer::singleShot(250, this, SLOT(ActivateX11Window()));
}
