//------------------------------------------------------------------------------
/// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
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
/// \file virtualkeybcontext.h
///
/// \version $Id: virtualkeybcontext.h 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief virtual keyboard input context
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#ifndef VIRTUALKEYBCONTEXT_H
#define VIRTUALKEYBCONTEXT_H

#include <QInputMethodQueryEvent>
#include "virtualkeyb.h"
#include <QWebEngineView>

class VirtualKeybContext: public QInputMethodQueryEvent
{
  Q_OBJECT

public:
  VirtualKeybContext();
  ~VirtualKeybContext();

  bool filterEvent(const QEvent* event);

  QString identifierName();
  QString language();

  bool isComposing() const;
  void reset();
  void SetEnabled(bool bValue = true);
  bool GetEnabled();

private slots:
  void HandleSwap();
  void ShowSip();

private:
  void updatePosition();

  VirtualKeyb * inputPanel;
  QList<VirtualKeyb *> m_panelList;

  void Initialize();
  void parseXML();
  void parseLayout(QXmlStreamReader &xml);
  void parseRows(QXmlStreamReader &xml);

  void CreateButton(tKeyValues * ptr, int &xpos, int &ypos);
  void CreateSpecialButton(tKeyValues * pKeyVal, int &xpos, int &ypos);

  Qt::AlignmentFlag m_af;

  bool bEnabled;
  //int m_nMouseClick_X;
  //int m_nMouseClick_Y;

};

#endif // VIRTUALKEYBCONTEXT_H
