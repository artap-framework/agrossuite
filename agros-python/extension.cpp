#include "Python.h"
#include "../resources_source/python/agros.cpp"

#include <QCoreApplication>

#include <QTranslator>
#include <QTextCodec>
#include <QDir>
#include <QString>

#include "util/system_utils.h"
#include "logview.h"

#include <deal.II/base/multithread_info.h>

#ifdef AGROS_BUILD_STATIC
#include "../plugins/plugins_static.h"
#endif

static QCoreApplication *app = nullptr;
static LogStdOut *logStd = nullptr;

class LibInstance
{
public:
    LibInstance()
    {
        // dealii::MultithreadInfo::set_thread_limit(1);

        // qInfo() << __FILE__ << "has been initialized";
        setlocale(LC_NUMERIC, "C");

        char *argv[] = {(char *) QString("%1/agros_python").arg(getenv("PWD")).toStdString().c_str(), NULL};
        int argc = sizeof(argv) / sizeof(char*) - 1;

        // app = new QCoreApplication(argc, argv);
        QCoreApplication::setApplicationVersion(versionString());
        QCoreApplication::setOrganizationName("agros");
        QCoreApplication::setOrganizationDomain("agros");
        QCoreApplication::setApplicationName("Agros Suite");

        // std::string codec
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

        // force number format
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

        // init singleton
        Agros::createSingleton(QSharedPointer<Log>(new LogStdOut()));
    }

    ~LibInstance()
    {
        delete app;
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