///------------------------------------------------------------------------------
/// \file    keyboard.h
///
/// \brief   platform input context
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QWidget>
#include "cmdthread.h"
#include "virtualkeyb.h"

#include <QVariant>
#include <QDBusVariant>

class Keyboard : public QWidget
{
  Q_OBJECT

public:
  explicit Keyboard(QWidget *parent = Q_NULLPTR);
  //explicit Keyboard();
  ~Keyboard();

  void SetEnabled(bool bValue = true);
  bool GetEnabled();    

  VirtualKeyb * inputPanel;
  QList<VirtualKeyb *> m_panelList;

public slots:
  void showKeyboard();
  void hideKeyboard();
  bool keyboardVisible() const;
  void inputFieldTextChanged(const QString &s, int iCursorPos);
  void slotSpecialKeyClicked(int key);
  void slotKeyClicked(const QString &text);
  void slotOpenFromPlugin();


signals:
  void signalDbusSpecialKeyClicked(int key);
  void signalDbusKeyClicked(const QString &text);
  void signalDbusShowVirtualKeyboardFromTv();
  void textChanged(QString, int);

private slots:
  void cmdSlotReceived(QString s);
  void HandleSwap();


private:
  void updatePosition();
  void Initialize();
  void parseXML();
  void parseLayout(QXmlStreamReader &xml);
  void parseRows(QXmlStreamReader &xml);

  void CreateButton(tKeyValues * ptr, int &xpos, int &ypos);
  void CreateSpecialButton(tKeyValues * pKeyVal, int &xpos, int &ypos);

  Qt::AlignmentFlag m_af;
  bool bEnabled;
  int m_cbState;

  cmdThread * pCmdThread;
  void startCmdThread();
  
};

#endif
