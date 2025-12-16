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
/// \file    toolbarmenu.h
///
/// \version $Id$
///
/// \brief   show menu bar in order to activate plc selection list
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#ifndef TOOLBARMENU_H
#define TOOLBARMENU_H

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QStyle>
#include <QStringList>
#include <QSettings>
#include <QPushButton>
#include <QUrl>
#include <QTouchEvent>
#include <QProcess>
#include <QDateTime>
#include <QDebug>

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QIcon>

#include "cmdthread.h"
#include "globals.h"



//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

class ToolbarButton: public QPushButton
{
public:
  ToolbarButton(const QString & text, QWidget * parent = 0);

  QString m_sStyle;
  QString m_sText;
  QString m_sIconFilename;
  QString m_sAction;

  int m_iLeftSpacing;
  int m_iWidth;
  int m_iHeight;

  bool m_bVisible;
};


typedef struct
{
  QList<ToolbarButton *> m_btnList;
  int iMenuTimeout;     //seconds
  int iMarginLeft;      //percent
  int iMarginRight;     //percent
  QString sWindowStyle;
  QString sButtonStyle;
  QString sLanguage;
  TOOLBAR_ORIENTATION orientation;
  QStringList slLeft;
  QStringList slRight;
} tMenuItems;


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


class ToolBarMenu : public QWidget
{
  Q_OBJECT
public:
  explicit ToolBarMenu(QWidget *parent = 0);
  ~ToolBarMenu();

  void Show();
  void Hide();
  bool IsCompleteVisible();
  void startCmdThread();  

  int m_iBtnCount;
  int m_iInitialWndHeight;
  tMenuItems m_menu;
  QRect m_DesktopRect;
  cmdThread * pCmdThread;

private:
  QTimer * m_pTimer;
  QTimer * m_pTimerCursorPos;
  QObject * m_pSender;
  int m_iTimeout;
  int m_iFadeHeight;
  bool m_bVkbWbm;

  QRect m_WndRect;  
  QString m_sUrlWbm;
  QString m_sUrlPlc;

  void RestartTimeout();
  void RestartTimeoutCursorPos();

  bool Slide(QPoint pos);

  QPoint m_LastPosition;

  QDateTime m_LastSlideEvent;

  void parseXML();
  void parseItem(QXmlStreamReader &xml);
  void parseTranslation(QXmlStreamReader &xml);
  void parseGlobals(QXmlStreamReader &xml);  
  void ReadPlcList();
  void ReadGestureConfigurationFile(QString sFile);
  void ReadLanguage(QString sFile);
  bool FindProcess(const char * pszName);  
  bool ReadPanelType(QString &sOut);
  bool ReadBootApp(QString &sOut);

signals:
  void signalMinimize();
  void signalFocus();


public slots:
  void slotTimeout();
  void slotTimeoutCursorPos();
  void OnButtonPressed();  
  void cmdSlotReceived(QString s);  

  void OnFadeIn();
  void OnHide();
  void OnAction();

protected:
  bool event(QEvent *e);
  void showEvent(QShowEvent *event);

};

#endif // TOOLBARMENU_H
