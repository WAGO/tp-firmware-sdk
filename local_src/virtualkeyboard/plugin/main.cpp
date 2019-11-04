///------------------------------------------------------------------------------
/// \file    main.cpp
///
/// \brief   platform input context
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include "qvkimplatforminputcontext.h"

#include <qpa/qplatforminputcontextplugin_p.h>
#include <QtCore/QStringList>
#include <QDebug>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

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


QT_BEGIN_NAMESPACE

class QVkImPlatformInputContextPlugin : public QPlatformInputContextPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID QPlatformInputContextFactoryInterface_iid FILE "vkim.json")

public:
  QVkImPlatformInputContext *create(const QString&, const QStringList&) Q_DECL_OVERRIDE;
};

QVkImPlatformInputContext *QVkImPlatformInputContextPlugin::create(const QString& system, const QStringList& paramList)
{
  Q_UNUSED(paramList);
  QVkImPlatformInputContext * pInputContext = NULL;

  if (system == QLatin1String("vkim"))
  {
    pInputContext = new QVkImPlatformInputContext;
    //qDebug() << "PLUGIN: pInputContext: " << pInputContext;
    return pInputContext;
  }
  else
  {
    qDebug() << "PLUGIN: vkim not found";
  }

  return 0;
}

QT_END_NAMESPACE

#include "main.moc"
