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
/// \file    mainwindow.h
///
/// \version $Id$
///
/// \brief   WaitHitTouch tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <QMainWindow>

#include <QLCDNumber>
#include <QLabel>
#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool eventFilter(QObject *obj, QEvent *event);

    int m_iNumber;
    int m_iReturnValue;
    QString m_sText;

    void Initialize();

public slots:

    void setNumber();

protected:
    void showEvent(QShowEvent *event);

private:
    QLCDNumber * m_pLcdNum;
    QTimer * m_pTimer;
    QLabel * m_pLabel;

    int m_iScreenWidth;
    int m_iScreenHeight;

};

#endif // MAINWINDOW_H
