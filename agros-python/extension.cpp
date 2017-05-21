#include "../resources_source/python/agros.cpp"

#include <QCoreApplication>

#include <QTranslator>
#include <QTextCodec>
#include <QDir>
#include <QString>

#include "util/global.h"

#include <deal.II/base/multithread_info.h>

#ifdef AGROS_BUILD_STATIC
#include "../plugins/plugins_static.h"
#endif

class LibInstance
{
public:
    LibInstance()
    {
        initSingleton();
    }

    ~LibInstance()
    {
        // qInfo() << __FILE__ << "has been unloaded";
    }
};

Q_GLOBAL_STATIC(LibInstance, libInstance)

class LibExecutor
{
public:
    LibExecutor() { libInstance(); }
};
static LibExecutor libExecutor;
