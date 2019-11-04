//------------------------------------------------------------------------------
/// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
///
/// This program is free software: you can redistribute it and/or modify  
/// it under the terms of the GNU General Public License as published by  
/// the Free Software Foundation, version 3.
///
/// This program is distributed in the hope that it will be useful, but 
/// WITHOUT ANY WARRANTY; without even the implied warranty of 
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
/// General Public License for more details.
///
/// You should have received a copy of the GNU General Public License 
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file    virtualkeyb.cpp
///
/// \version $Id: virtualkeyb.cpp 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief   virtual keyboard
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <QCoreApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>
#include <QStyle>
#include <QDebug>

#include "keyboard.h"
#include "virtualkeyb.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define DEFAULT_KEY_REPETITION_RATE             200
#define TRIGGER_KEY_REPETITON                   300

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------
extern int g_InputMethodHints;

//extern QMyWebPage *g_pWebPage;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

QString VirtualKeyb::m_sLastCharacter = "";

CTextBox::CTextBox(QWidget * parent) : QPlainTextEdit(parent)
{
  //QPlainTextEdit::QPlainTextEdit(parent);
}


CVKButton::CVKButton(const QString & text, QWidget * parent) :
    QPushButton(text, parent), m_iCode(0), m_iCodeShifted(0),
    m_iRepetitionAllowed(0), m_iSpecialBtn(0)
{
  //QPushButton::QPushButton(text, parent);
}

VirtualKeyb::VirtualKeyb() :
    QWidget(0, Qt::WindowDoesNotAcceptFocus | Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint)
{
  m_pGroupBox = NULL;
  m_bHiddenText = false;
  m_bEnabled = true;
  m_pCheckPw = NULL;
  m_pEdit = NULL;
  m_iShift = 0;
  m_iShiftLock = 0;
  m_iRowCount = 4;
  m_iColCount = 10;
  m_iMinFontSize = 8;
  m_iMinBtnWidth = 25;
  m_iMinBtnHeight = 15;
  m_btn_xpos = 0;
  m_btn_ypos = 0;

  m_hspace = 0;
  m_vspace = 0;

  m_bSwapButtonCalled = false;

  m_iRepetitionRate = DEFAULT_KEY_REPETITION_RATE;

  m_af = Qt::AlignBottom;

  //screen dimensions
  QRect r = QApplication::desktop()->screenGeometry();
  m_iScreenWidth = r.width();
  m_iScreenHeight = r.height();

  //setWindowModality(Qt::ApplicationModal);
  
  connect(&timerKeyRepetition, SIGNAL(timeout()), this, SLOT(OnTimerKeyRepetition()));
}

VirtualKeyb::~VirtualKeyb()
{

  timerKeyRepetition.stop();

  //free button memory
  CVKButton * ptr;
  for (int i = 0; i < m_btnList.count(); i++)
  {
    ptr = m_btnList.at(i);
    delete ptr;
  }

  for (int i = 0; i < m_btnListSpecial.count(); i++)
  {
    ptr = m_btnListSpecial.at(i);
    delete ptr;
  }

  m_btnList.clear();
  m_btnListSpecial.clear();

  if (m_pEdit)
    delete m_pEdit;

  if (m_pCheckPw)
    delete m_pCheckPw;

  if (m_pGroupBox)
    delete m_pGroupBox;

  //if (m_pVBoxLayout)
  //  delete m_pVBoxLayout;
}


/// \brief set button dimensions
///
void VirtualKeyb::SetBtnDimensions()
{

  QFont f = font();
  m_iBtnWidth = m_iScreenWidth / m_iColCount;
  m_iBtnWidth -= 1;
  m_iBtnHeight = CalculatePixelHeight(f) * 2;

  m_iBtnWidth = qMax(m_iBtnWidth, m_iMinBtnWidth);
  m_iBtnHeight = qMax(m_iBtnHeight, m_iMinBtnHeight);

  if (!m_pEdit)
  {
    if (!m_pGroupBox)
    {
      m_pGroupBox = new QGroupBox(tr(""), this);
    }

    if (!m_pCheckPw)
    {
      QString sText = "Show password";
      
      m_pCheckPw = new QCheckBox(sText, m_pGroupBox);
      m_pCheckPw->setFont(f);
      connect(m_pCheckPw, SIGNAL(stateChanged(int)), this, SLOT(OnCheckBoxStateChanged(int)));

      QVBoxLayout *vbox = new QVBoxLayout;
      vbox->addWidget(m_pCheckPw);
      vbox->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
      m_pGroupBox->setLayout(vbox);

    }
    //always show edit field
    //if ((m_iScreenHeight - (m_iBtnHeight * m_iRowCount)) < m_iFreeVPixels)
    //{
      //insert edit field
      m_pEdit = new CTextBox(this);
      if (m_pEdit)
      {
        QRect r1(QRect(QPoint(1, 1), QSize(m_iScreenWidth - 2, m_iBtnHeight * 1.5 - 4)));
        QRect r2(r1);

        //r1.setWidth(r1.width() * 0.75);
        r2.setRect(r1.topRight().x() + 1, r1.topRight().y(), r2.width() - r1.width() - 1, r1.height());

        m_pEdit->setTextInteractionFlags(Qt::TextEditable | Qt::TextSelectableByKeyboard);
        m_pEdit->setGeometry(r1);
        m_pGroupBox->setGeometry(r2);
        m_pGroupBox->hide();

        m_pEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_pEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      }
      else
      {
        //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
      }
      m_btn_ypos += (m_iBtnHeight * 1.5 - 4);
    //}
  }

  //qDebug() << "webkit: " << "SetDimensions pointSize: " << f.pointSize();
  //qDebug() << "webkit: " << "SetDimensions m_iBtnWidth: " << m_iBtnWidth << " m_iBtnHeight: " << m_iBtnHeight ;

}

/// \brief set panel dimensions
///
void VirtualKeyb::SetPanelDimensions(QSize newSize)
{
  if (newSize.width() < m_iScreenWidth)
  {
    //center
    int x = (m_iScreenWidth - newSize.width()) / 2;
    int y = (m_iScreenHeight - newSize.height());
    setGeometry(x, y, newSize.width(), newSize.height());
  }
  else
  {
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, m_af, newSize, qApp->desktop()->availableGeometry()));
  }
}


void VirtualKeyb::OnCheckBoxStateChanged(int iState)
{
  emit vkPasswordCbStateChanged(iState);
}


/// \brief handle button
///
void VirtualKeyb::OnButtonPressed()
{  

  QObject * ptr = QObject::sender();

  for (int k = 0; k < m_btnList.count(); k++)
  {
    if (ptr == m_btnList.at(k))
    {
      m_pBtn = (CVKButton *) ptr;    

      if (m_iShift)
      {
        if (m_pBtn->m_iCodeShifted)
          sendKeyCode(m_pBtn->m_iCodeShifted, m_pBtn->m_iSpecialBtn);
        else
          sendCharacter(m_pBtn->m_sKeyShifted.at(0));
      }
      else
      {
        if (m_pBtn->m_iCode)
          sendKeyCode(m_pBtn->m_iCode, m_pBtn->m_iSpecialBtn);
        else
          sendCharacter(m_pBtn->m_sKey.at(0));
      }

      if (m_pBtn->m_iRepetitionAllowed)
      {
        KeyRepetitionStartTimer();
      }

      break;
    }
  }

  if ((!m_iShiftLock) && (m_iShift))
  {
    //deactivate shift
    HandleShift();
  }
}

/// \brief handle backspace
///
void VirtualKeyb::HandleBksp()
{

  QObject * ptr = QObject::sender();
  if (ptr)
  {
    m_sLastCharacter = "";
    m_pBtn = (CVKButton *) ptr;
    int iCode = Qt::Key_Backspace;
    if (m_iShift)
    {
      if (m_pBtn->m_iCodeShifted)
        iCode = m_pBtn->m_iCodeShifted;
    }
    else
    {
      if (m_pBtn->m_iCode)
        iCode = m_pBtn->m_iCode;
    }
    sendKeyCode(iCode, m_pBtn->m_iSpecialBtn);

    if (m_pBtn->m_iRepetitionAllowed)
    {
      KeyRepetitionStartTimer();
    }
  }

}

/// \brief handle space
///
void VirtualKeyb::HandleSpace()
{
  QObject * ptr = QObject::sender();
  if (ptr)
  {
    //m_pBtn = (CVKButton *) ptr;
    //int iCode = Qt::Key_Space;
    //if (m_iShift)
    //{
    //  if (m_pBtn->m_iCodeShifted)
    //    iCode = m_pBtn->m_iCodeShifted;
    //}
    //else
    //{
    //  if (m_pBtn->m_iCode)
    //    iCode = m_pBtn->m_iCode;
    //}
    //sendKeyCode(iCode);
    sendCharacter(QChar(' '));
  }
}

/// \brief handle shift
///
void VirtualKeyb::HandleShift()
{

  m_iShift = !m_iShift;
  if (m_iShiftLock)
  {
    m_iShiftLock = 0;
    m_iShift = 0;
  }
  CVKButton * ptr;

  for (int k = 0; k < m_btnList.count(); k++)
  {
    ptr = m_btnList.at(k);
    if (m_iShift)
    {
      ptr->setText(ptr->m_sKeyShifted);
    }
    else
    {
      ptr->setText(ptr->m_sKey);
    }
  }
}

/// \brief handle shift lock
///
void VirtualKeyb::HandleShiftLock()
{

  m_iShift = 1;
  m_iShiftLock = !m_iShiftLock;
  CVKButton * ptr;

  for (int k = 0; k < m_btnList.count(); k++)
  {
    ptr = m_btnList.at(k);
    ptr->setText(ptr->m_sKeyShifted);
  }
}

/// \brief handle escape
///
void VirtualKeyb::HandleEsc()
{
  //Qt::Key_Escape
  //QApplication::exit(0);    
  
  QObject * ptr = QObject::sender();
  if (ptr)
  {
    m_sLastCharacter = "";
    m_pBtn = (CVKButton *) ptr;
    int iCode = Qt::Key_Escape;   
    sendKeyCode(iCode, m_pBtn->m_iSpecialBtn);
  }
  
  Hide();
}

/// \brief handle left
///
void VirtualKeyb::HandleLeft()
{
  QObject * ptr = QObject::sender();
  if (ptr)
  {
    m_sLastCharacter = "";
    m_pBtn = (CVKButton *) ptr;
    int iCode = Qt::Key_Left;
    if (m_iShift)
    {
      if (m_pBtn->m_iCodeShifted)
        iCode = m_pBtn->m_iCodeShifted;
    }
    else
    {
      if (m_pBtn->m_iCode)
        iCode = m_pBtn->m_iCode;
    }
    sendKeyCode(iCode, m_pBtn->m_iSpecialBtn);

    if (m_pBtn->m_iRepetitionAllowed)
    {
      KeyRepetitionStartTimer();
    }
  }

}

/// \brief handle right
///
void VirtualKeyb::HandleRight()
{
  QObject * ptr = QObject::sender();
  if (ptr)
  {
    m_sLastCharacter = "";
    m_pBtn = (CVKButton *) ptr;
    int iCode = Qt::Key_Right;
    if (m_iShift)
    {
      if (m_pBtn->m_iCodeShifted)
        iCode = m_pBtn->m_iCodeShifted;
    }
    else
    {
      if (m_pBtn->m_iCode)
        iCode = m_pBtn->m_iCode;
    }
    sendKeyCode(iCode, m_pBtn->m_iSpecialBtn);

    if (m_pBtn->m_iRepetitionAllowed)
    {
      KeyRepetitionStartTimer();
    }
  }

}

/// \brief handle up
///
void VirtualKeyb::HandleUp()
{
  QObject * ptr = QObject::sender();
  if (ptr)
  {
    m_sLastCharacter = "";
    m_pBtn = (CVKButton *) ptr;
    int iCode = Qt::Key_Up;
    if (m_iShift)
    {
      if (m_pBtn->m_iCodeShifted)
        iCode = m_pBtn->m_iCodeShifted;
    }
    else
    {
      if (m_pBtn->m_iCode)
        iCode = m_pBtn->m_iCode;
    }
    sendKeyCode(iCode, m_pBtn->m_iSpecialBtn);

    if (m_pBtn->m_iRepetitionAllowed)
    {
      KeyRepetitionStartTimer();
    }
  }

}

/// \brief handle down
///
void VirtualKeyb::HandleDown()
{
  QObject * ptr = QObject::sender();
  if (ptr)
  {
    m_sLastCharacter = "";
    m_pBtn = (CVKButton *) ptr;
    int iCode = Qt::Key_Down;
    if (m_iShift)
    {
      if (m_pBtn->m_iCodeShifted)
        iCode = m_pBtn->m_iCodeShifted;
    }
    else
    {
      if (m_pBtn->m_iCode)
        iCode = m_pBtn->m_iCode;
    }
    sendKeyCode(iCode, m_pBtn->m_iSpecialBtn);

    if (m_pBtn->m_iRepetitionAllowed)
    {
      KeyRepetitionStartTimer();
    }
  }

}

/// \brief handle ok
///
void VirtualKeyb::HandleOk()
{
  m_sLastCharacter = "";
  Hide();  

}

/// \brief handle clear
///
void VirtualKeyb::HandleClear()
{
  m_sLastCharacter = "";
  //if (m_pEdit)
  //  m_pEdit->setText("");
}

/// \brief handle tab
///
void VirtualKeyb::HandleTab()
{
  sendKeyCode(Qt::Key_Tab, 1);
}

/// \brief handle enter
///
void VirtualKeyb::HandleEnter()
{
  QObject * ptr = QObject::sender();
  if (ptr)
  {
    m_pBtn = (CVKButton *) ptr;
    int iCode = Qt::Key_Enter; //Return;
    if (m_iShift)
    {
      if (m_pBtn->m_iCodeShifted)
        iCode = m_pBtn->m_iCodeShifted;
    }
    else
    {
      if (m_pBtn->m_iCode)
        iCode = m_pBtn->m_iCode;
    }
    //qDebug() << "webkit: " << "enter " << iCode;
    sendKeyCode(iCode, m_pBtn->m_iSpecialBtn);
  }

}

/// \brief send character to focused widget
///
void VirtualKeyb::sendCharacter(QChar character)
{  
  m_sLastCharacter = character;
  emit vkKeyClicked(character);
}

/// \brief send key number to focused widget
///
void VirtualKeyb::sendKeyCode(int iKey, int iSpecialBtn)
{
  if (iSpecialBtn)
  {
    emit vkSpecialKeyClicked(iKey);
  }
  else
  {
    m_sLastCharacter = QString::number(iKey);
    emit vkKeyClicked(QString::number(iKey) );
  }
}


/// \brief show keyboard
///
void VirtualKeyb::Show()
{
  m_sLastCharacter = "";
  m_bSwapButtonCalled = false;

  if (m_bEnabled)
  {
    show();
    //ShowEditCursor();
  }

}

/// \brief hide keyboard
///
void VirtualKeyb::Hide()
{
  m_sLastCharacter = "";
  timerKeyRepetition.stop();
  hide();

}

/// \brief button released received
///
void VirtualKeyb::OnButtonReleased()
{
  timerKeyRepetition.stop();
}

/// \brief send key repetition to focused widget
///
void VirtualKeyb::OnTimerKeyRepetition()
{
  if ((m_pBtn) && (m_pBtn->m_iRepetitionAllowed))
  {    
    if (m_iShift)
    {
      if (m_pBtn->m_iCodeShifted)
        sendKeyCode(m_pBtn->m_iCodeShifted, m_pBtn->m_iSpecialBtn);
      else
        sendCharacter(m_pBtn->m_sKeyShifted.at(0));
    }
    else
    {
      if (m_pBtn->m_iCode)
        sendKeyCode(m_pBtn->m_iCode, m_pBtn->m_iSpecialBtn);
      else
        sendCharacter(m_pBtn->m_sKey.at(0));
    }

    if (timerKeyRepetition.interval() != m_iRepetitionRate)
    {
      timerKeyRepetition.setInterval(m_iRepetitionRate);
    }
  }
  else
  {
    timerKeyRepetition.stop();
  }

}

/// \brief get total number of buttons
///
int VirtualKeyb::GetBtnCount()
{
  return (m_btnList.count() + m_btnListSpecial.count());
}

/// \brief set cursor position in the line edit field
///
void VirtualKeyb::SetCursorPos(int cursorPosition)
{

  if (m_pEdit)
  {
    //qDebug() << "webkit: " << "SetCursorPos " << cursorPosition;
    QTextCursor cursor(m_pEdit->textCursor());

    cursor.setPosition(cursorPosition);
    cursor.movePosition(QTextCursor::PreviousCharacter);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    m_pEdit->setTextCursor(cursor);    
  }

}


/// \brief get cursor position of the line edit field
///
int VirtualKeyb::GetCursorPos()
{
  int iRet = -1;
  if (m_pEdit)
  {
    QTextCursor cursor = m_pEdit->textCursor();
    iRet = cursor.position();
  }
  return iRet;
}

/// \brief make cursor visible in the line edit field
///
void VirtualKeyb::ShowEditCursor()
{  
  if (m_pEdit)
  {
    m_pEdit->setFocus();
  }
}

/// \brief start key repetition timer
///
void VirtualKeyb::KeyRepetitionStartTimer()
{  
  if (!timerKeyRepetition.isActive())
  {
    timerKeyRepetition.start(TRIGGER_KEY_REPETITON);
  }
}


/// \brief calculate string width depending on font
///
/// \param[in] sText    calculating width for this string
/// \param[in] fnt      calculating width using this font
/// \retval  pixel width of the string
int VirtualKeyb::CalculatePixelWidth(QString sText, QFont fnt)
{
  QFontMetrics fm(fnt);
  int pixelsWide = fm.width(sText);
  return (pixelsWide);
}

/// \brief calculate height depending on font
///
/// \param[in] fnt      calculating width using this font
/// \retval  pixel height of the font
int VirtualKeyb::CalculatePixelHeight(QFont fnt)
{
  QFontMetrics fm(fnt);
  int pixelsHigh = fm.height();
  return (pixelsHigh);
}

void VirtualKeyb::slotTextChanged(const QString &sTxt, int iCursorPos)
{
  if (m_pEdit)
  {
    if (m_bHiddenText && m_pCheckPw->isChecked() && (sTxt.length() > 0) && (sTxt.at(0).isLetterOrNumber()==false))
    {
        //qDebug() << "char: " << sTxt.at(0);        
        //qDebug() << "int: " << sTxt.at(0).digitValue();        
        //        char:  '\u2022'
        //        int:  -1        
        if (m_sLastCharacter.length() > 0)
        {
          QString s = sTxt;
          s.chop(1);
          m_pEdit->setPlainText(s + m_sLastCharacter);
        }
        else
        {
          m_pEdit->setPlainText(sTxt);
        }
        SetCursorPos(iCursorPos);
    }
    else
    {
        m_pEdit->setPlainText(sTxt);
        SetCursorPos(iCursorPos);    
    }          
  }
}

void VirtualKeyb::ShowCbPasswd(bool bShow)
{
  m_bHiddenText = bShow;
  if (m_pGroupBox->isVisible() != bShow)
  {
    QRect r1(QRect(QPoint(1, 1), QSize(m_iScreenWidth - 2, m_iBtnHeight * 1.5 - 4)));
    QRect r2(r1);

    if (bShow)
    {
      /* OLD
      r1.setWidth(r1.width() * 0.75);
      r2.setRect(r1.topRight().x() + 1, r1.topRight().y(), r2.width() - r1.width() - 1, r1.height());
      m_pEdit->setGeometry(r1);
      m_pEdit->show();
      m_pGroupBox->setGeometry(r2);      
      m_pGroupBox->show();
      */
      
      //NEW workaround do not show checkbox field and set to true
      r2.setRect(r1.topRight().x() + 1, r1.topRight().y(), r2.width() - r1.width() - 1, r1.height());
      m_pEdit->setGeometry(r1);
      m_pGroupBox->setGeometry(r2);
      m_pGroupBox->hide();
      m_pEdit->show();
      m_pCheckPw->setChecked(true);
      
    }
    else
    {
      r2.setRect(r1.topRight().x() + 1, r1.topRight().y(), r2.width() - r1.width() - 1, r1.height());
      m_pEdit->setGeometry(r1);
      m_pGroupBox->setGeometry(r2);
      m_pGroupBox->hide();
      m_pEdit->show();

      m_pCheckPw->setChecked(false);
    }
  }
}

void VirtualKeyb::SetVkEnabled(bool bEnabled)
{
    m_bEnabled = bEnabled;
}
