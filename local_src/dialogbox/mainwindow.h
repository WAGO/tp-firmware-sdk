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
/// \version $Id: mainwindow.h 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   dialogbox tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QCheckBox>

#include <QStyle>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    int m_iReturnValue;
    QStringList m_sList;
    void Initialize();
    int m_iSeconds;
    int m_iDefaultBtn;
    bool m_bShowCheckbox;
    QString m_sCbTxt;

private:
    Ui::MainWindow *ui;

    QString m_sMsg;
    int m_iScreenWidth;
    int m_iScreenHeight;

    QLabel * m_pLbl;
    QPushButton * m_pBtn;
    QList<QPushButton *> m_btnList;

    QCheckBox * m_pCheckBox;

    QTimer m_timer;    

    int CalculatePixelWidth(QString sText, QFont fnt, bool bDefBtn = false);
    int CalculatePixelHeight(QFont fnt);

    QString sQtStyleForm;
    QString sQtStyleButton;
    QString sQtStyleCTAButton;
    int iDefaultBtnHeight;

    int iFntSizeForm;
    int iFntSizeBtn;

    QFont fntBtn;

protected:
    void showEvent(QShowEvent *event);

private slots:
    void HandleButton();
    void TimeoutFunction();
    void ActivateX11Window();

};

#endif // MAINWINDOW_H
