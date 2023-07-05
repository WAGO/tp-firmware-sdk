///------------------------------------------------------------------------------
/// \file    qvkimplatforminputcontext.h 
///
/// \brief   platform input context
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#ifndef QVKIMPLATFORMINPUTCONTEXT_H
#define QVKIMPLATFORMINPUTCONTEXT_H

#include <qpa/qplatforminputcontext.h>

#include <QVariant>
#include <QDBusVariant>


QT_BEGIN_NAMESPACE


class QDBusInterface;

class QVkImPlatformInputContext : public QPlatformInputContext
{
    Q_OBJECT

public:
    QVkImPlatformInputContext();
    ~QVkImPlatformInputContext();

    bool isValid() const Q_DECL_OVERRIDE;
    void setFocusObject(QObject *object) Q_DECL_OVERRIDE;

    void showInputPanel() Q_DECL_OVERRIDE;
    void hideInputPanel() Q_DECL_OVERRIDE;
    bool isInputPanelVisible() const Q_DECL_OVERRIDE;

    QDBusInterface * m_dbusInterface;

private slots:
    void keyboardSpecialKeyClicked(int key);
    void keyboardKeyClicked(const QString &character);

    void slotOnUpdateText();
    void slotShowVirtualKeyboardFromTv();

private:
    QObject *m_focusObject;
    void showVirtualKeyboard();

protected:
    void updateText();


};

QT_END_NAMESPACE

#endif
