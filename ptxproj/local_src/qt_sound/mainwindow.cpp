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
/// \file    mainwindow.cpp
///
/// \brief   qsound test prg
///
/// \author  Ralf Gliese, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QObject::connect(ui->Play, SIGNAL(clicked(bool)), this, SLOT(on_Play_clicked()));
    QObject::connect(ui->Stop, SIGNAL(clicked(bool)), this, SLOT(on_Stop_clicked()));
    //QObject::connect(ui->Stop, SIGNAL(clicked(bool)), this, SLOT(SoundActive->stop()));
#if 1
    player = new QMediaPlayer;
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Play_clicked()
{
#if 0
  SoundActive = new QSound(SoundFilePath);
  SoundActive->play();
#else
    //player->setMedia(QUrl::fromLocalFile(SoundFilePath));
    //QMediaContent content(QUrl::fromLocalFile("/opt/pga/track00.wav"));
				//qDebug() << "MediaContent: isNull = " << content.isNull();
				//player->setMedia(content);
    player->setMedia(QMediaContent(QUrl::fromLocalFile("/home/Track01.wav")));

    player->setVolume(50);
    player->play();
#endif
}

void MainWindow::on_Stop_clicked()
{
#if 0
	SoundActive->stop();
#else
	player->stop();
#endif
}

//gst-launch-1.0 filesrc location=/home/Track01.wav ! wavparse ! audioconvert ! alsasink 
