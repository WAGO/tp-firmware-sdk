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
/// \file    screensaver.h
///
/// \version $Id: screensaver.h 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief   screensaver Qt tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QMainWindow>
#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

enum eTHEME_ID
{
    THEME_NONE = 0,  ///< no screensaver theme (backlight)
    THEME_IMAGE,     ///< screensaver theme image
    THEME_TIME,      ///< screensaver theme time
    THEME_TEXT       ///< screensaver theme text 
};

class ScreenSaver : public QMainWindow
{
    Q_OBJECT
public:
    explicit ScreenSaver(QWidget *parent = 0);
    ~ScreenSaver();

    bool eventFilter(QObject *obj, QEvent *event);
    void mouseMoveEvent(QMouseEvent * event);

    void Initialize();
    int m_iThemeId;
    QString m_sParameter;
    bool ReadImageFileNameFromConfigTool(QString & sImageFileName);

signals:


public slots:
    //bool event(QEvent *event);
    void setPosition();
    void ActivateX11Window();

protected:
    void paintEvent(QPaintEvent*);        
    void showEvent(QShowEvent *event);

private:
    QTimer * m_pTimer;
    QImage m_image;
    QImage m_scaledImage;

    int m_iScreenWidth;
    int m_iScreenHeight;
    int m_iReturnValue;

    int m_xRandom;
    int m_yRandom;

    unsigned int m_iCounter;

    int CalculatePixelWidth(QString sText, QFont fnt);
    int CalculatePixelHeight(QFont fnt);

    bool m_bFirst;
};

#endif // SCREENSAVER_H
