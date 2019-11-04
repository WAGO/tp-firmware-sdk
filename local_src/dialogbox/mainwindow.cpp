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
/// \file    mainwindow.cpp
///
/// \version $Id: mainwindow.cpp 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief   dialogbox tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <QDesktopWidget>
#include <QDebug>
#include <QSettings>
#include <QProcess>
#include <QX11Info>

#include "globals.h"
#include "tools.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define H_SPACE            12
#define V_SPACE            10
#define S_COUNTDOWN             " [999] " //only to calculate width

#define MIN_BTN_WIDTH           50
#define RETVAL_CHECKBOX_OFFSET  100

#define CONF_FILE               "/etc/specific/qtstyle.conf"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_pLbl = NULL;
    m_pBtn = NULL;
    m_pCheckBox = NULL;

    m_bShowCheckbox = false;

    m_iReturnValue = 0; //zero means countdown ended no default button
    m_iSeconds = 0;     //zero meand no timeout
    m_iDefaultBtn = 0;

    iFntSizeForm = 16;
    iFntSizeBtn = 16;

    //screen dimensions
    QRect r = QApplication::desktop()->screenGeometry();
    m_iScreenWidth = r.width();
    m_iScreenHeight = r.height();

    setGeometry(QRect(QPoint(0, 0), QSize(m_iScreenWidth, m_iScreenHeight)));

    Qt::WindowFlags flags = Qt::FramelessWindowHint;

    //flags |= Qt::CustomizeWindowHint;
    flags |= Qt::WindowStaysOnTopHint;
    //flags |= Qt::WindowTitleHint;

    //flags &~ Qt::WindowMinMaxButtonsHint;
    //flags &~ Qt::WindowCloseButtonHint;
    //flags &~ Qt::WindowContextHelpButtonHint;
    //flags &~ Qt::WindowSystemMenuHint;

    setWindowFlags(flags);

    //setWindowState(Qt::WindowNoState);
    setWindowModality(Qt::ApplicationModal);



    sQtStyleForm = DEFAULT_FORM_STYLE;
    sQtStyleButton = DEFAULT_FORM_STYLE;
    sQtStyleCTAButton = DEFAULT_FORM_STYLE;
    iDefaultBtnHeight = 40;

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

    QString sIdFormStyle = sPrefix + "form_style_dialogbox";
    QString sIdButtonStyle = sPrefix + "btn_enabled_style";
    QString sIdButtonCTAStyle = sPrefix + "btn_calltoaction_style";
    QString sBtnTextPlcList = "PLC";
    QString sBtnTextCleaning = "clean"; //display cleaning

    QSettings settings(CONF_FILE, QSettings::IniFormat);
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
          else if (key == sIdButtonCTAStyle)
          {
            sQtStyleCTAButton = var;
          }
          else if (key == sIdButtonStyle)
          {
            sQtStyleButton = var;
          }
          else if (key == QString(sPrefix + "btn_height"))
          {
            iDefaultBtnHeight = var.toInt();
          }

        }
        //qDebug() << key << "  " << var;
    }

    setStyleSheet(sQtStyleForm);
    //btnTmp.setStyleSheet(sQtStyleButton);

    QString sVal;
    QString sTmp = sQtStyleForm;
    sTmp.remove(QChar(' '));
    int iPos = sTmp.indexOf("font-size:", Qt::CaseInsensitive);
    if (iPos >= 0)
    {
      iPos += 10;
      while ((sTmp[iPos] >= QChar('0')) && (sTmp[iPos] <= QChar('9')))
      {
        sVal += sTmp[iPos];
        iPos++;
      }
      if (sVal.length()>0)
      {
        iFntSizeForm = sVal.toInt();
      }
    }

    sVal.clear();
    sTmp = sQtStyleButton;
    sTmp.remove(QChar(' '));
    iPos = sTmp.indexOf("font-size:", Qt::CaseInsensitive);
    if (iPos >= 0)
    {
      iPos += 10;
      while ((sTmp[iPos] >= QChar('0')) && (sTmp[iPos] <= QChar('9')))
      {
        sVal += sTmp[iPos];
        iPos++;
      }
      if (sVal.length()>0)
      {
        iFntSizeBtn = sVal.toInt();
      }
    }

    //set font
    QFont fnt = this->font();
    fnt.setPointSize(iFntSizeForm);
    setFont(fnt);

    fntBtn.setPointSize(iFntSizeBtn);
   
    style()->unpolish(this);
    style()->polish(this);    
      
    ui->setupUi(this);
    update();        
}

MainWindow::~MainWindow()
{
    //free label memory
    if (m_pLbl)
        delete m_pLbl;

    //free button memory
    QPushButton * ptr;
    for (int i=0; i < m_btnList.count(); i++)
    {
        ptr = m_btnList.at(i);
        delete ptr;
    }

    m_btnList.clear();

    delete ui;
}

/// \brief create layout, buttons etc.
///
void MainWindow::Initialize()
{
    int i, iWidth, iHeight;

    //qDebug() << sQtStyleForm;

    //qDebug() << "btn pt: " << fntBtn.pointSize() << "btn px: " << fntBtn.pixelSize();

    int xpos = 10;
    int ypos = 10;
    int btn_width = 0;
    int btn_height = iDefaultBtnHeight; //qMax(iDefaultBtnHeight, qRound(CalculatePixelHeight(fntBtn) * 1.2));

    //get text pixel width
    int iTextWidth = 0;
    int iTextHeight = qRound(CalculatePixelHeight(font()) * 1.2);
    int iLines = 0;

    if (m_sList.count() > 0)
    {
        m_sMsg = m_sList.at(0);

        if ((m_iDefaultBtn == 0) && (m_iSeconds > 0))
        {
          //countdown [s] will be appended to the textfield
          iTextWidth = CalculatePixelWidth(m_sMsg, font(), true) + 20;
        }
        else
        {
          iTextWidth = CalculatePixelWidth(m_sMsg, font()) + 20;
        }

        iLines = qRound((double)(iTextWidth / (m_iScreenWidth-30.0)));
        iLines++;        
    }

    //create label
    if (!m_pLbl)
      m_pLbl = new QLabel(this);

    if (!m_pLbl)
      QApplication::exit();


    //get btn line pixel width summary
    int iBtnLineWidth = xpos;


    if (m_sList.count() > 1)
    {
      for (i=1; i < m_sList.count(); i++)
      {
        btn_width = qMax(MIN_BTN_WIDTH, CalculatePixelWidth(m_sList.at(i), fntBtn));
        iBtnLineWidth += btn_width;
        iBtnLineWidth += H_SPACE;
      }
    }

    //qDebug() << "iTextWidth " << iTextWidth << " iBtnLineWidth " << iBtnLineWidth;

    //label settings and fill in text
    int iLabelWidth = qMax(iTextWidth, iBtnLineWidth);
    iLabelWidth = qMin(m_iScreenWidth - 30, iLabelWidth);    

    int iLabelHeight = iLines * iTextHeight; //btn_height
    iLabelHeight += 10;

    //qDebug() << "iLabelHeight " << iLabelHeight << " btn_height " << btn_height  << " iLines " << iLines;

    iHeight = iLabelHeight;
    iWidth = iLabelWidth;

    m_pLbl->setWordWrap(true);
    m_pLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    //m_pLbl->setGeometry(QRect(QPoint(xpos, ypos), QSize(iLabelWidth, iLabelHeight)));
    ypos += iLabelHeight;
    //m_pLbl->setText(m_sMsg);

    ypos += V_SPACE / 2;

    //create buttons
    int window_width = H_SPACE;
    int max_window_width = 0;

    if (m_sList.count() > 1)
    {

        for (i=1; i < m_sList.count(); i++)
        {
            m_pBtn = new QPushButton(m_sList.at(i), this);
            if (m_pBtn)
            {
                m_btnList.append(m_pBtn);
                m_pBtn->setStyleSheet(sQtStyleButton);
                
                m_pBtn->style()->unpolish(m_pBtn);
                m_pBtn->style()->polish(m_pBtn);
                m_pBtn->update();
               
                //m_pBtn->setFont(fnt);

                if ((i == m_iDefaultBtn) && (m_iSeconds > 0))
                {
                  btn_width = qMax(MIN_BTN_WIDTH, CalculatePixelWidth(m_sList.at(i), m_pBtn->font(), true));
                }
                else
                {
                  btn_width = qMax(MIN_BTN_WIDTH, CalculatePixelWidth(m_sList.at(i), m_pBtn->font()));
                }
                btn_width += H_SPACE;
                window_width += btn_width;

                if (window_width  > (m_iScreenWidth - 20))
                {
                  //line feed
                  window_width = H_SPACE + btn_width;
                  xpos = 10;
                  ypos += (btn_height + V_SPACE);
                  iHeight = ypos; // + V_SPACE;
                }

                // set size and location of the button
                m_pBtn->setGeometry(QRect(QPoint(xpos, ypos), QSize(btn_width, btn_height)));

                // Connect button signal to appropriate slot
                connect(m_pBtn, SIGNAL(released()), this, SLOT(HandleButton()));

                xpos += btn_width;
                xpos += H_SPACE;

                window_width += H_SPACE;

                //m_pBtn->setStyleSheet("border-style:dotted;");

                if ((i) == m_iDefaultBtn)
                {
                  //selected button with another color
                  //m_pBtn->setStyleSheet("color:white; background:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #efef10, stop:1 #444444);");
                  m_pBtn->setStyleSheet(sQtStyleCTAButton);
                  m_pBtn->setFocus();
                }


                if (window_width > max_window_width)
                  max_window_width = window_width;
            }
        }

        if (m_iDefaultBtn > (m_sList.count() - 1 ))
          m_iDefaultBtn = 0; //out of btn count

    }
    else
    {
        qDebug() << "not enough parameters";
        QApplication::exit();
    }

    ypos += btn_height + V_SPACE;

    //qDebug() << "iWidth " << iWidth << " max_window_width " << max_window_width << " lines " << iLines << " ypos " << ypos;

    iWidth = qMax(iWidth, max_window_width);
    iWidth = qMin(iWidth, m_iScreenWidth - 10);
    iHeight = qMin(ypos, m_iScreenHeight - 10);

    QSize newSize( iWidth, iHeight );

    m_pLbl->setGeometry(QRect(QPoint(10,10), QSize(iWidth - 20, iLabelHeight)));
    m_pLbl->setText(m_sMsg);

    //add checkbox if parameter -c is used
    if (m_bShowCheckbox)
    {
      m_pCheckBox = new QCheckBox(m_sCbTxt, this);
      if (m_pCheckBox)
      {
        m_pCheckBox->setStyleSheet(sQtStyleButton);

/* works not - no QLabel found in Checkbox
        QList<QLabel*> lables = m_pCheckBox->findChildren<QLabel*>();
        for(int i=0; i < lables.size(); i++)
          qDebug() << lables.at(i)->text();
        qDebug() << "size: " << lables.size();
*/
        int iCbWidth = CalculatePixelWidth(m_sCbTxt, m_pCheckBox->font()) + 30;
        int iCbHeight = iTextHeight + 4;
        int iCbMaxWidth = m_iScreenWidth - 30;

        if (iCbWidth  > newSize.width())
        {
          //expand width if checkbox needs it
          newSize.setWidth(qMin(iCbWidth + 30, iCbMaxWidth));
        }

        QSize sizeCb(newSize.width() - 20, iCbHeight);
        newSize.setHeight(newSize.height() + sizeCb.height());

        QPoint pointCbTopLeft(10,newSize.height() - iCbHeight);
        QRect cbRect (pointCbTopLeft, sizeCb); //top left, size

        m_pCheckBox->setGeometry(cbRect);

      }
    }

    setGeometry(QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter,
                newSize, qApp->desktop()->availableGeometry()) );

    if (m_iSeconds > 0)
    {
      connect(&m_timer, SIGNAL(timeout()), this, SLOT(TimeoutFunction()));
      m_timer.start(1000);
    }

}

/// \brief check which button is clicked, exit with suiting return value
///
void MainWindow::HandleButton()
{
    int i = 0;
    QObject * ptr = QObject::sender();

    for (int k=0; k < m_btnList.count(); k++)
    {
        if (ptr == m_btnList.at(k))
        {
            i = k+1;
            m_iReturnValue = i;
            break;
        }
    }

    if (i > 0)
    {
      if ((m_bShowCheckbox) && (m_pCheckBox) && (m_pCheckBox->checkState() == Qt::Checked))
      {
        m_iReturnValue += RETVAL_CHECKBOX_OFFSET;
      }
      QApplication::exit(m_iReturnValue);
    }
}

/// \brief countdown if parameter time is used
///
void MainWindow::TimeoutFunction()
{
    QString s;
    //s = m_sTitle + " [" +  QString::number(m_iSeconds) + "] ";
    //this->setWindowTitle(s);

    if (m_iDefaultBtn > 0)
    {
        QPushButton * pBtn = m_btnList.at(m_iDefaultBtn - 1);
        if (pBtn)
        {
            s = m_sList.at(m_iDefaultBtn) + " [" +  QString::number(m_iSeconds) + "] ";
            pBtn->setText(s);
        }
    }
    else
    {
        if (m_pLbl)
        {
            s = m_sMsg + " [" +  QString::number(m_iSeconds) + "] ";
            m_pLbl->setText(s);
        }
    }

    this->update();

    if (m_iSeconds <= 0)
    {
        m_timer.stop();
        m_iReturnValue = m_iDefaultBtn;
        if ((m_bShowCheckbox) && (m_pCheckBox) && (m_pCheckBox->checkState() == Qt::Checked))
        {
          m_iReturnValue += RETVAL_CHECKBOX_OFFSET;
        }
        QApplication::exit(m_iReturnValue);
    }

    m_iSeconds -= 1;
}

/// \brief calculate string width depending on font
///
/// \param[in] sText    calculating width for this string
/// \param[in] fnt      calculating width using this font
/// \retval  pixel width of the string
int MainWindow::CalculatePixelWidth(QString sText, QFont fnt, bool bDefBtn)
{
    QFontMetrics fm(fnt);
    int pixelsWide = fm.width(sText);
    if (bDefBtn)
      pixelsWide += fm.width(S_COUNTDOWN);
    return (pixelsWide);
}

/// \brief calculate height depending on font
///
/// \param[in] fnt      calculating width using this font
/// \retval  pixel height of the font
int MainWindow::CalculatePixelHeight(QFont fnt)
{
    QFontMetrics fm(fnt);
    int pixelsHigh = fm.height();
    return (pixelsHigh);
}

void MainWindow::showEvent(QShowEvent *event)
{
  QTimer::singleShot(250, this, SLOT(ActivateX11Window()));
}

/// \brief ensure X11 window to be shown in front
/// \param[in]  X11 window id
void MainWindow::ActivateX11Window()
{
  if (QX11Info::isPlatformX11())
  {
    int id = QWidget::winId ();
    if (id > 0)
    {
      QProcess proc;
      proc.start("/usr/bin/xdotool", QStringList() << "windowactivate" << QString::number(id) );
      proc.waitForFinished(5000);
      proc.close();
      //qDebug() << "ActivateX11Window: " << id;
    }
  }
}
