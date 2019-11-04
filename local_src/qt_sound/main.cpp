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
/// \file    main.cpp
///
/// \brief   qsound test prg
///
/// \author  Ralf Gliese, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "ui_mainwindow.h"

void ShowHelpText();

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	if (QApplication::arguments().count() > 1)
	{
		QString sTxt = QApplication::arguments().at(1);
		if ((sTxt == "-h") || (sTxt == "--help"))
		{
			ShowHelpText();
			return -1;
		}
	}

	if (QApplication::arguments().count() < 1)
	{
		qDebug() << "not enough parameter";
		return -1;
	}

	MainWindow w;
	w.SoundFilePath = QApplication::arguments().at(1);
	qDebug() << "SoundFilePath=" << w.SoundFilePath << "+++";
    w.ui->lineEdit->setPlaceholderText(w.SoundFilePath);
    QObject::connect(w.ui->Stop, SIGNAL(clicked(bool)), &a, SLOT(quit()));
	w.show();

	return a.exec();
}

/// \brief Print help text / usage to stdout
///
void ShowHelpText()
{
	qDebug("\n* qt_sound * \n");
	qDebug("Usage: qt_sound <file>");
	qDebug() << "";
}
