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
/// \file    screencare.h
///
/// \version $Id: screencare.h 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   Application main window - header
///
/// \author  Nicoleta Nething, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#ifndef SCREENCARE_H
#define SCREENCARE_H


#include <QMainWindow>
#include <QEvent>

#define NROF_COLORCHANGES        2

class ScreenCare : public QMainWindow
{
    Q_OBJECT
public:

    explicit ScreenCare(long l_numberOfColorChanges,  QWidget *parent = 0);
    ~ ScreenCare();

    long numberOfColorChanges;
    int iCounter;

signals:
public slots:
    void OnCloseScreenCare();

protected:
    long idxNumberOfColorChanges;
    void showEvent(QShowEvent *event);
    void paintEvent(QPaintEvent* event);

};

#endif // SCREENCARE_H
