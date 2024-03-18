// #include "../resources_source/python/_agros.cpp"

#include <QCoreApplication>

#include <QTranslator>
#include <QDir>
#include <QString>

#include "util/global.h"

class LibInstance
{
public:
    LibInstance()
    {
        initSingleton();
    }

    ~LibInstance()
    {        
    }
};

Q_GLOBAL_STATIC(LibInstance, libInstance)

class LibExecutor
{
public:
    LibExecutor() { libInstance(); }
};
static LibExecutor libExecutor;
