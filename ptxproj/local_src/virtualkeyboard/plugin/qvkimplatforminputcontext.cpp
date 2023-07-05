///------------------------------------------------------------------------------
/// \file    qvkimplatforminputcontext.cpp
///
/// \brief   platform input context
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include "qvkimplatforminputcontext.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QGuiApplication>
#include <QInputMethodEvent>
#include <QWidget>
#include <QInputMethodQueryEvent>
#include <QVariant>
#include <QTimer>
#include <QQuickItem>
#include <QtQuickWidgets>
#include <QtWebEngine>
#include <QWebEngineView>

#include "msgtool.h"
#include <QDebug>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define DELAY_GET_TEXT_FROM_EDIT_FIELD    100   //needed to synchronize virtual keyboards edit field correctly

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

template <class T> class VPtr
{
public:
  static T* asPtr(QVariant v)
  {
      return (T *) v.value<void *>();
  }

  static QVariant asQVariant(T* ptr)
  {
      return qVariantFromValue((void *) ptr);
  }
};

QVkImPlatformInputContext::QVkImPlatformInputContext()
    : m_focusObject(0)
{ 
  if (!QDBusConnection::sessionBus().isConnected()) {
      qDebug() << "PLUGIN: Cannot connect to the D-Bus session bus. To start it, run: eval `dbus-launch --auto-syntax`";
  }

  m_dbusInterface = new QDBusInterface("com.kdab.inputmethod", "/VirtualKeyboard", "local.virtualkeyboard.Keyboard", QDBusConnection::sessionBus(), this);

  if (m_dbusInterface)
  {
    connect(m_dbusInterface, SIGNAL(keyClicked(QString)), SLOT(keyboardKeyClicked(QString)));
    connect(m_dbusInterface, SIGNAL(specialKeyClicked(int)), SLOT(keyboardSpecialKeyClicked(int)));
    connect(m_dbusInterface, SIGNAL(signalShowVirtualKeyboardFromTv()), SLOT(slotShowVirtualKeyboardFromTv()));
  }
  else
  {
    qDebug() << "PLUGIN: Creating DBus Interface FAILED !!!";
  }
}

QVkImPlatformInputContext::~QVkImPlatformInputContext()
{
  //qDebug() << "PLUGIN: DESTRUCTOR: QVkImPlatformInputContext::~QVkImPlatformInputContext()";
}

bool QVkImPlatformInputContext::isValid() const
{
  return m_dbusInterface->isValid();
}

void QVkImPlatformInputContext::setFocusObject(QObject *object)
{
  QQuickItem * quickitem = qobject_cast<QQuickItem*>(object);
  if (!quickitem)
  {
      m_focusObject = object;
  }
  //else
  //{
  //    qDebug() << "!!! QQuickItem not useable !!! ";
  //}
}

void QVkImPlatformInputContext::showInputPanel()
{
  //coming from PlatformInputContext
  showVirtualKeyboard();
  m_dbusInterface->call("slotOpenFromPlugin");
}

void QVkImPlatformInputContext::showVirtualKeyboard()
{
  QPoint globalPos(0, 0);
  QWidget *widget = qobject_cast<QWidget*>(m_focusObject);

  if (IsUsbKeyboardPresent()==0)
  {
    //do not show if USB-Keyboard is available
    return;
  }

  if (widget)
  {
    globalPos = widget->mapToGlobal(QPoint(0, widget->height()));

    //open virtual keyboard sending dbus cmd to server
    m_dbusInterface->call("showKeyboard");
    QTimer::singleShot(DELAY_GET_TEXT_FROM_EDIT_FIELD, this, SLOT(slotOnUpdateText()));

  }
}

void QVkImPlatformInputContext::hideInputPanel()
{
  m_dbusInterface->call("hideKeyboard");
}

bool QVkImPlatformInputContext::isInputPanelVisible() const
{
  const QDBusReply<bool> reply = m_dbusInterface->call("keyboardVisible");

  if (reply.isValid())
    return reply.value();
  else
    return false;
}

void QVkImPlatformInputContext::updateText()
{     
  if (m_focusObject)
  {
    
    QQuickWidget * webengine_obj = qobject_cast<QQuickWidget*>(m_focusObject);  
    if (webengine_obj)
    {
      //qDebug() << "===== webengine_obj ===== " << webengine_obj->isWindow();
      QVariant variant = webengine_obj->inputMethodQuery(Qt::ImSurroundingText);
      QString  sTxt = variant.value<QString>();

      QVariant variant1 = webengine_obj->inputMethodQuery(Qt::ImCursorPosition);
      int iPos = variant1.value<int>();
  
      int hints = webengine_obj->inputMethodHints();
      if ((hints & Qt::ImhHiddenText) || (hints & Qt::ImhSensitiveData))
      {
        for (int i=0; i< sTxt.length(); i++)
          sTxt[i] = QChar(Qt::Key_Asterisk);
      }
  
      m_dbusInterface->call("inputFieldTextChanged", sTxt, iPos);
      
      return;
    }
      
    QWidget *widget = qobject_cast<QWidget*>(m_focusObject);
    if (widget)
    {        
      QVariant variant = widget->inputMethodQuery(Qt::ImSurroundingText);
      QString  sTxt = variant.value<QString>();

      QVariant variant1 = widget->inputMethodQuery(Qt::ImCursorPosition);
      int iPos = variant1.value<int>();

      int hints = widget->inputMethodHints();
      if ((hints & Qt::ImhHiddenText) || (hints & Qt::ImhSensitiveData))
      {
        for (int i=0; i< sTxt.length(); i++)
          sTxt[i] = QChar(Qt::Key_Asterisk); //'*';
      }
      m_dbusInterface->call("inputFieldTextChanged", sTxt, iPos);
    }
    else
    {     
      qDebug() << "!!! QVkImPlatformInputContext::updateText widget: is NULL";
    }
  }
  else
  {
    qDebug() << "!!! QVkImPlatformInputContext::updateText m_focusObject: is NULL";
  }

}

void QVkImPlatformInputContext::keyboardSpecialKeyClicked(int key)
{
  if (!m_focusObject)
      return;

  QKeyEvent *pressEvent = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier);
  if (pressEvent)
  {
    QGuiApplication::sendEvent(m_focusObject, pressEvent);
    delete pressEvent;
  }

  QKeyEvent *releaseEvent = new QKeyEvent(QEvent::KeyRelease, key, Qt::NoModifier);
  if (releaseEvent)
  {
    QGuiApplication::sendEvent(m_focusObject, releaseEvent);
    delete releaseEvent;
  }

  QTimer::singleShot(DELAY_GET_TEXT_FROM_EDIT_FIELD, this, SLOT(slotOnUpdateText()));
}

void QVkImPlatformInputContext::keyboardKeyClicked(const QString &characters)
{
  if (!m_focusObject)
      return;

  QInputMethodEvent event;
  event.setCommitString(characters);
  QGuiApplication::sendEvent(m_focusObject, &event);

  QTimer::singleShot(DELAY_GET_TEXT_FROM_EDIT_FIELD, this, SLOT(slotOnUpdateText()));
}

void QVkImPlatformInputContext::slotOnUpdateText()
{
  updateText();
}

void QVkImPlatformInputContext::slotShowVirtualKeyboardFromTv()
{
  //open vk from targetvisu
  showVirtualKeyboard();
}
