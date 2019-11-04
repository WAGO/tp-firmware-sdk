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
/// \file    virtualkeyb.h
///
/// \version $Id: virtualkeyb.h 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief   virtual keyboard
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#ifndef VIRTUALKEYB_H
#define VIRTUALKEYB_H

#include <QtGui>
#include <QtCore>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QIcon>

#include <QPushButton>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>

typedef struct
{
  QString sKey;
  QString sCode;
  QString sKeyShifted;
  QString sCodeShifted;
  QString sIcon;
  QString sAutoRepeat;
  QString sControlText;
  double widthscale;
} tKeyValues;

class CVKButton: public QPushButton
{
  Q_OBJECT

public:
  CVKButton(const QString & text, QWidget * parent = 0);

  QString m_sKey;
  QString m_sKeyShifted;
  int m_iCode;
  int m_iCodeShifted;
  int m_iRepetitionAllowed;
  int m_iSpecialBtn;
};

class CTextBox: public QPlainTextEdit
{
  Q_OBJECT

public:
  CTextBox( QWidget * parent = 0);


/*
protected:
  void mousePressEvent(QMouseEvent *event)
  {
    //avoid mouse interaction
  }
  void mouseReleaseEvent(QMouseEvent *event)
  {
    //avoid mouse interaction
  }
*/

};


class VirtualKeyb: public QWidget
{
  Q_OBJECT

public:
  VirtualKeyb();
  ~VirtualKeyb();

  //QObject * m_pFocusedObject;
static QString m_sLastCharacter;

  //QString m_sLanguageId;
  CVKButton * m_pBtn;
  QList<CVKButton *> m_btnList;
  QList<CVKButton *> m_btnListSpecial;

  int m_iScreenWidth;
  int m_iScreenHeight;
  int m_iShift;
  int m_iShiftLock;

  int m_iBtnWidth;
  int m_iBtnHeight;

  int m_iRowCount;
  int m_iColCount;
  int m_iMinFontSize;
  int m_iMinBtnWidth;
  int m_iMinBtnHeight;

  int m_iRepetitionRate;
  bool m_bSwapButtonCalled;

  Qt::AlignmentFlag m_af;

  QString m_sWndStyle;
  QString m_sBtnStyle;
  QString m_sBtnStyleControl;

  void SetBtnDimensions();
  void SetPanelDimensions(QSize newSize);
  void Show();
  void Hide();
  void ShowCbPasswd(bool bShow);

  QTimer timerKeyRepetition;

  int m_btn_xpos;
  int m_btn_ypos;

  int m_hspace;
  int m_vspace;

  int GetBtnCount(); 
  void SetCursorPos(int cursorPosition);
  int GetCursorPos();
  void ShowEditCursor();  

  QCheckBox * m_pCheckPw;

  void SetVkEnabled(bool bEnabled);

signals:
  void vkSpecialKeyClicked(int key);
  void vkKeyClicked(const QString &text);

  void vkPasswordCbStateChanged(int);

//protected:
//  bool event(QEvent *e);


public slots:
  void OnButtonPressed();
  void OnButtonReleased();
  void HandleBksp();
  void HandleShift();
  void HandleShiftLock();
  void HandleEsc();
  void HandleClear();
  void HandleEnter();
  void HandleTab();
  void HandleSpace();
  void HandleLeft();
  void HandleRight();
  void HandleUp();
  void HandleDown();
  void HandleOk();

  void OnCheckBoxStateChanged(int iState);

  void OnTimerKeyRepetition();

private slots:
  void slotTextChanged(const QString &sTxt, int iCursorPos);

private:

  CTextBox * m_pEdit;
  QGroupBox * m_pGroupBox;
  bool m_bHiddenText;  
  bool m_bEnabled;

  int m_nMouseClick_X_Coordinate;
  int m_nMouseClick_Y_Coordinate;

  void sendCharacter(QChar character);
  void sendKeyCode(int iKey, int iSpecialBtn = 0);

  void KeyRepetitionStartTimer();

  int CalculatePixelWidth(QString sText, QFont fnt);
  int CalculatePixelHeight(QFont fnt);
  
  

};

#endif // VIRTUALKEYB_H
