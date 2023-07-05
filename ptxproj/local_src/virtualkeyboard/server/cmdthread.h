//------------------------------------------------------------------------------
/// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
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
/// \file cmdthread.h
///
/// \version $Id$
///
/// \brief Thread for receiving command f.e. open menu
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#ifndef CMDTHREAD_H
#define CMDTHREAD_H

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <QObject>
#include <QThread>

class cmdThread : public QThread
{
  Q_OBJECT
public:
  explicit cmdThread(QObject *parent = 0);
  ~cmdThread();

  void run();
  char isRunning;

private:
  bool bInitialized;

signals:
  void cmdSignalReceived(QString);
};

#endif // CMDTHREAD_H

