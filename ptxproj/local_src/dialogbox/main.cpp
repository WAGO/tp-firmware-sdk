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
/// \file    main.cpp
///
/// \version $Id: main.cpp 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   dialogbox tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
int ParseCmdLine(MainWindow * mw);
void ShowHelpText();

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables’ and constants’ definitions
//------------------------------------------------------------------------------

int g_iSeconds = 0;
int g_iDefBtn = 0;

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    //QStringList * pList = &w.m_sList;
    if (ParseCmdLine(&w) < 0)
        return -1;

    if (w.m_sList.count() < 2)
        return -1;

    w.m_iDefaultBtn = g_iDefBtn;
    w.m_iSeconds = g_iSeconds;
    w.Initialize();
    w.show();        
    a.exec();

    //return 0        means countdown ended no default button
    //return 1 .. n   means button number or default button if countdown expired
    return w.m_iReturnValue;
}

/// \brief commandline parser
/// \param[in]  pList arguments
/// \retval 0  SUCCESS
int ParseCmdLine(MainWindow * mw)
{
    int state = 0;
    QStringList * pList = &mw->m_sList;
    //QApplication::arguments()
    QString sTxt;
    int i = 1;
    while (i < QApplication::arguments().count())
    {
       //todo
       sTxt = QApplication::arguments().at(i);
       if (sTxt[0] == '-')
       {
           if (sTxt[1].toLower() == 's')
           {
               if (sTxt.length() > 2)
               {
                   qDebug() << "invalid option " << sTxt << "\n";
                   state = -1;
               }
               else
               {
                   i++;
                   if (i < QApplication::arguments().count())
                   {
                       sTxt = QApplication::arguments().at(i);
                       g_iSeconds = sTxt.toInt();
                   }
               }

           }
           else if (sTxt[1].toLower() == 'd')
           {
               if (sTxt.length() > 2)
               {
                   qDebug() << "invalid option " << sTxt << "\n";
                   state = -1;
               }
               else
               {
                   i++;
                   if (i < QApplication::arguments().count())
                   {
                       sTxt = QApplication::arguments().at(i);
                       g_iDefBtn = sTxt.toInt();
                   }
               }
           }
           else if (sTxt[1].toLower() == 'c')
           {
               if (sTxt.length() > 2)
               {
                   qDebug() << "invalid option " << sTxt << "\n";
                   state = -1;
               }
               else
               {
                   i++;
                   if (i < QApplication::arguments().count())
                   {
                       sTxt = QApplication::arguments().at(i);
                       if (sTxt.length() > 0)
                       {
                         mw->m_bShowCheckbox = true;
                         mw->m_sCbTxt = sTxt;
                       }
                   }
               }
           }
           else if (sTxt[1].toLower() == 'h')
           {
               ShowHelpText();
               state = -1;
           }
           else if (sTxt[1] == '-')
           {
               ShowHelpText();
               state = -1;
           }
           else
           {
               qDebug() << "invalid option " << sTxt << "\n";
               state = -1;
           }


       }
       else
       {
         pList->append(sTxt);
       }
       i++;
    }
    return state;
}

/// \brief show cmdline help text
void ShowHelpText()
{
    qDebug("\n* dialogbox * \n");
    qDebug("Usage: dialogbox <parameter> [options]");
    qDebug("");
    qDebug("options:");
    qDebug("  -s countdown [s] using default button ");
    qDebug("  -d default button number using countdown");
    qDebug("  -c [text] show checkbox at the bottom");
    qDebug("  -h print this help");
    qDebug("");
    qDebug("parameter: MessageText Button1 Button2 ButtonN -s 20 -d 1");
    qDebug("");
}

