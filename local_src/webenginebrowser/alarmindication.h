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
///
/// \file    alarmindication.h
///
/// \version $Id: alarmindication.h 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   show connection lost informations
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#ifndef ALARMINDICATION_H
#define ALARMINDICATION_H

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStyle>
#include <QStringList>
#include <QSettings>
#include <QProcess>
#include <QUrl>

#include <QLCDNumber>
#include <QTimer>
//#include <QMutex>

#include <QDebug>

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

class AlarmIndication : public QWidget
{
  Q_OBJECT
public:
  explicit AlarmIndication(QWidget *parent = 0);
  ~AlarmIndication();

  QRect rScreen;
  //QLabel * m_pLbl;
  QPlainTextEdit * m_pPlainTextEdit;
  QPushButton * m_pBtnWBM;
  QLCDNumber * m_pLcdNum;

  void Countdown(int count);
  void Show();
  void Hide();

  void SetWbmUrl(QString sUrl);

  bool bLEDEnabled;
  void SetLabelText(QString sText);

signals:
  void signalStartWbm(QString);

public slots:
  void OnShowWBM();
  void setNumber();

private:
  int m_iNumber;
  QTimer * m_pTimer;
  QString sUrlWbm;
  int m_iWbmBtnState;

  void ReadWbmBtnState();

protected:
  void showEvent (QShowEvent * event);
  //void hideEvent (QHideEvent * event);

};

#endif // ALARMINDICATION_H
