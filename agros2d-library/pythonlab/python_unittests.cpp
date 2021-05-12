// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "pythonengine_agros.h"
#include "python_unittests.h"
#include "util/constants.h"
#include "gui/common.h"
#include "logview.h"

#include <ctemplate/template.h>

const static QString DATE_FORMAT = "yyyy-MM-dd hh.mm.ss";

UnitTestsWidget::UnitTestsWidget(QWidget *parent)
    : QDialog(parent), m_test(XMLTest::tests()), m_isAborted(false)
{
    setWindowTitle(tr("Unit tests"));
    setModal(true);

    webView = new QWebView();
    webView->page()->setNetworkAccessManager(new QNetworkAccessManager());
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    webView->setMinimumSize(200, 200);

    logWidget = new LogWidget(this);
    logWidget->setMemoryLabelVisible(false);

    trvTests = new QTreeWidget(this);
    trvTests->setMouseTracking(true);
    trvTests->setColumnCount(1);
    trvTests->setIndentation(15);
    trvTests->setIconSize(QSize(24, 24));
    trvTests->setHeaderHidden(true);
    trvTests->setMinimumWidth(360);
    trvTests->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(trvTests, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    // connect(trvTests, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    // connect(trvTests, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    btnRunTests = new QPushButton(tr("Run tests"));
    connect(btnRunTests, SIGNAL(clicked()), this, SLOT(runTestsFromSuite()));

    // dialog buttons
    btnScenarios = new QPushButton(tr("Scenarios..."));
    btnUncheckTests = new QPushButton(tr("Uncheck tests"));
    connect(btnUncheckTests, SIGNAL(clicked()), this, SLOT(uncheckTests()));
    btnStopTest = new QPushButton(tr("Stop"));
    btnStopTest->setEnabled(false);
    connect(btnStopTest, SIGNAL(clicked()), this, SLOT(stopTest()));

    QGridLayout *leftLayout = new QGridLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->addWidget(trvTests, 0, 0, 1, 5);
    leftLayout->addWidget(btnScenarios, 1, 0);
    leftLayout->addWidget(btnUncheckTests, 1, 1);
    leftLayout->setColumnStretch(2, 1);
    leftLayout->addWidget(btnStopTest, 1, 3);
    leftLayout->addWidget(btnRunTests, 1, 4);

    QWidget *leftWidget = new QWidget();
    leftWidget->setLayout(leftLayout);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(webView, 3);
    rightLayout->addWidget(logWidget, 1);

    QWidget *rightWidget = new QWidget();
    rightWidget->setLayout(rightLayout);

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(splitter, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    QSettings settings;
    splitter->restoreState(settings.value("UnitTestsWidget/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("UnitTestsWidget/SplitterGeometry").toByteArray());
    restoreGeometry(settings.value("UnitTestsWidget/Geometry", saveGeometry()).toByteArray());

    // init ctemplate
    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize() + 1).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_results.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

    // read tests from test_suite
    readTestsFromSuite();
    readScenariosFromSuite();

    // read last test
    QDir dirUser(QString("%1/tests").arg(userDataDir()));
    if (!dirUser.exists())
        QDir(userDataDir()).mkpath(dirUser.absolutePath());

    dirUser.setFilter(QDir::Files | QDir::NoSymLinks);
    if (!dirUser.entryInfoList().isEmpty())
    {
        QString file = QString("%1").arg(dirUser.entryInfoList().last().absoluteFilePath());
        readTestFromDisk(file);
    }
}

UnitTestsWidget::~UnitTestsWidget()
{
    QSettings settings;
    settings.setValue("UnitTestsWidget/SplitterState", splitter->saveState());
    settings.setValue("UnitTestsWidget/SplitterGeometry", splitter->saveGeometry());
    settings.setValue("UnitTestsWidget/Geometry", saveGeometry());
}

QList<QTreeWidgetItem *> UnitTestsWidget::availableTests()
{
    QList<QTreeWidgetItem *> list;

    for (int i = 0; i < trvTests->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *module = trvTests->topLevelItem(i);

        for (int j = 0; j < module->childCount(); j++)
        {
            list.append(module->child(j));
        }
    }

    return list;
}

void UnitTestsWidget::doContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *current = trvTests->itemAt(pos);

    if (current)
    {
        QStringList tests = current->data(2, Qt::UserRole).toStringList();

        QMenu *menu = new QMenu();
        connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(showInfoTest(QAction *)));
        foreach (QString test, tests)
        {
            QAction *act = new QAction(test, menu);
            act->setData(QString("%1.%2.%3").
                         arg(current->data(0, Qt::UserRole).toString()).
                         arg(current->data(1, Qt::UserRole).toString()).
                         arg(test));

            menu->addAction(act);
        }

        menu->exec(QCursor::pos());
    }
}

void UnitTestsWidget::readTestsSettingsFromScenario(QAction *action)
{
    QStringList modules;
    QStringList clss;

    // run expression
    currentPythonEngine()->runExpression(QString("import test_suite; agros2d_scenario = test_suite.%1").arg(action->data().toString()));

    // extract values
    PyObject *result = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_scenario");
    if (result)
    {
        Py_INCREF(result);
        for (int i = 0; i < PyList_Size(result); i++)
        {
            PyObject *obj = PyList_GetItem(result, i);
            Py_INCREF(obj);

            modules << QString::fromStdString(PyBytes_AsString(PyObject_GetAttrString(obj, "__module__")));
            clss << QString::fromStdString(PyBytes_AsString(PyObject_GetAttrString(obj, "__name__")));

            Py_XDECREF(obj);
        }
        Py_XDECREF(result);
    }

    foreach (QTreeWidgetItem *item, availableTests())
    {
        item->setCheckState(0, Qt::Unchecked);

        for (int j = 0; j < modules.count(); j++)
        {
            if ((item->data(0, Qt::UserRole).toString() == modules.at(j)) &&
                    (item->data(1, Qt::UserRole).toString() == clss.at(j)))
            {
                item->setCheckState(0, Qt::Checked);

                // remove module and class
                modules.removeAt(j);
                clss.removeAt(j);

                break;
            }
        }
    }

    // remove variables
    currentPythonEngine()->runExpression("del agros2d_scenario");
}

void UnitTestsWidget::uncheckTests()
{
    foreach (QTreeWidgetItem *item, availableTests())
        item->setCheckState(0, Qt::Unchecked);
}

void UnitTestsWidget::runTestsFromSuite()
{
    m_isAborted = false;

    setEnabledControls(false);

    // save settings
    saveTestsSettings();

    // clean test suite
    m_test = XMLTest::test(XMLTest::tests());
    webView->setHtml("");
    logWidget->clear();

    // set date
    QString date = QString("%1").arg(QDateTime::currentDateTime().toString(DATE_FORMAT));
    m_test.date() = date.toStdString();

    QDir dirUser(QString("%1/tests").arg(userDataDir()));
    QString file = QString("%1/%2.res").arg(dirUser.absolutePath()).arg(date);

    try
    {
        foreach (QTreeWidgetItem *item, availableTests())
        {
            if (item->checkState(0) == Qt::Checked)
            {
                runTestFromSuite(item->data(0, Qt::UserRole).toString(),
                                 item->data(1, Qt::UserRole).toString());

                showInfoTests();
                QApplication::processEvents();
            }

            if (m_isAborted)
            {
                m_isAborted = false;
                break;
            }
        }

        std::string mesh_schema_location("");

        mesh_schema_location.append(QString("%1/test_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
        ::xml_schema::namespace_info namespace_info_mesh("XMLTest", mesh_schema_location);

        ::xml_schema::namespace_infomap namespace_info_map;
        namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("test", namespace_info_mesh));

        std::ofstream out(compatibleFilename(file).toStdString().c_str());
        XMLTest::test_(out, m_test, namespace_info_map);
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }

    setEnabledControls(true);
}

void UnitTestsWidget::runTestFromSuite(const QString &module, const QString &cls)
{
    QString str = QString("import unittest as ut; agros2d_suite = ut.TestSuite(); import %1; agros2d_suite.addTest(ut.TestLoader().loadTestsFromTestCase(%1.%2)); agros2d_result = test_suite.scenario.Agros2DTestResult(); agros2d_suite.run(agros2d_result); agros2d_result_report = agros2d_result.report()").
            arg(module).arg(cls);

    currentPythonEngine()->runScript(str);

    PyObject *result = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_result_report");
    if (result)
    {
        Py_INCREF(result);
        for (int i = 0; i < PyList_Size(result); i++)
        {
            PyObject *list = PyList_GetItem(result, i);
            Py_INCREF(list);

            QString tmodule = PyBytes_AsString(PyList_GetItem(list, 0));
            QString tcls = PyBytes_AsString(PyList_GetItem(list, 1));
            QString ttest = PyBytes_AsString(PyList_GetItem(list, 2));
            double telapsedTime = PyFloat_AsDouble(PyList_GetItem(list, 3));
            QString tstatus = PyBytes_AsString(PyList_GetItem(list, 4));
            QString terror = PyBytes_AsString(PyList_GetItem(list, 5));

            // add to the file
            XMLTest::item item(ttest.toStdString(),
                               tmodule.toStdString(),
                               tcls.toStdString(),
                               telapsedTime,
                               tstatus == "OK",
                               terror.toStdString());

            m_test.tests().item().push_back(item);

            Py_XDECREF(list);

        }
        Py_XDECREF(result);
    }

    currentPythonEngine()->runExpression("del agros2d_suite; del agros2d_result; del agros2d_result_report");
}

void UnitTestsWidget::showInfoTests(const QString &testID)
{
    // template
    std::string results;
    ctemplate::TemplateDictionary testsTemplate("tests");

    testsTemplate.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    testsTemplate.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());

    testsTemplate.SetValue("LABEL_DATE", tr("Date").toStdString());
    if (m_test.date().present())
        testsTemplate.SetValue("DATE", QString::fromStdString(m_test.date().get()).replace(".", ":").toStdString());
    testsTemplate.SetValue("LABEL_TOTAL_TIME", tr("Total time").toStdString());
    testsTemplate.SetValue("LABEL_TEST_PASSED", tr("Passed tests").toStdString());
    testsTemplate.SetValue("TEST_ALL", QString::number(m_test.tests().item().size()).toStdString());

    double totalTime = 0.0;
    int successful = 0;
    for (unsigned int i = 0; i < m_test.tests().item().size(); i++)
    {
        XMLTest::item item = m_test.tests().item().at(i);

        ctemplate::TemplateDictionary *itemTemplate = testsTemplate.AddSectionDictionary("ITEM");


#if QT_VERSION < 0x050000
        QString err = Qt::escape(QString::fromStdString(item.error()));
#else
        QString err = QString(QString::fromStdString(item.error())).toHtmlEscaped();
#endif

        itemTemplate->SetValue("MODULE", item.module());
        itemTemplate->SetValue("CLS", item.cls());
        itemTemplate->SetValue("NAME", item.name());
        itemTemplate->SetValue("TIME", milisecondsToTime(item.time()).toString("mm:ss.zzz").toStdString());
        itemTemplate->SetValue("STATUS", item.successful() == 1 ?
                                   tr("OK").toStdString() :
                                   tr("<span style=\"color: red;\">ERROR</span>").toStdString());

        if (item.successful() == 0)
        {
            ctemplate::TemplateDictionary *itemTemplateError = itemTemplate->AddSectionDictionary("ITEM_ERROR");
            itemTemplateError->SetValue("ERROR", err.toStdString());
        }
        else
        {
            successful++;
        }

        totalTime += item.time();
        testsTemplate.SetValue("TOTAL_TIME", milisecondsToTime(totalTime).toString("mm:ss.zzz").toStdString());
        testsTemplate.SetValue("TEST_PASSED", QString::number(successful).toStdString());
    }

    // current test info
    if (!testID.isEmpty())
    {
        // read last test
        QDir dirUser(QString("%1/tests").arg(userDataDir()));

        dirUser.setFilter(QDir::Files | QDir::NoSymLinks);
        if (!dirUser.entryInfoList().isEmpty())
        {
            QList<uint> dates;
            QList<double> times;

            for (int i = 0; i < dirUser.entryInfoList().size(); ++i)
            {
                QFileInfo fileInfo = dirUser.entryInfoList().at(i);
                if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                    continue;

                try
                {
                    std::auto_ptr<XMLTest::test> testlocal_xsd = XMLTest::test_(compatibleFilename(fileInfo.absoluteFilePath()).toStdString(), xml_schema::flags::dont_validate);
                    XMLTest::test *testlocal = testlocal_xsd.get();

                    for (unsigned int j = 0; j < testlocal->tests().item().size(); j++)
                    {
                        XMLTest::item item = testlocal->tests().item().at(j);

                        QString itemID = QString("%1.%2.%3").arg(QString::fromStdString(item.module())).
                                arg(QString::fromStdString(item.cls())).
                                arg(QString::fromStdString(item.name()));

                        if (itemID == testID)
                        {
                            dates.append(QDateTime::fromString(QString::fromStdString(testlocal->date().get()), DATE_FORMAT).toTime_t());
                            times.append(item.time());

                            break;
                        }
                    }
                }
                catch (const xml_schema::exception& e)
                {
                    std::cerr << e << std::endl;
                }
            }

            QString dataTimeSteps = "[";
            for (int i = 0; i < times.size(); i++)
                if (times.at(i) > 0.0)
                    dataTimeSteps += QString("[%1*1000, %2], ").arg(dates.at(i)).arg(times.at(i));
            dataTimeSteps += "]";

            // chart time step vs. steps
            QString testChart = QString("<script type=\"text/javascript\">$(function () { $.plot($(\"#chart_test\"), [ { data: %1, color: \"rgb(61, 61, 251)\", lines: { show: false }, points: { show: false }, bars: { show: true } } ], { grid: { hoverable : true }, xaxes: [ { mode: 'time', timeformat: '%d/%m/%y', axisLabel: 'Date' } ], yaxes: [ { axisLabel: 'Elapsed time (ms)' } ] });});</script>").
                    arg(dataTimeSteps);

            testsTemplate.ShowSection("TEST_CHART_JS");

            ctemplate::TemplateDictionary *chartTemplate = testsTemplate.AddSectionDictionary("TEST_CHART_SECTION");
            chartTemplate->SetValue("TEST_CHART_LABEL", testID.toStdString());
            chartTemplate->SetValue("TEST_CHART", testChart.toStdString());
        }
    }

    // expand template
    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/tests.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &testsTemplate, &results);

    if (testID.isEmpty())
    {
        webView->setHtml(QString::fromStdString(results));
    }
    else
    {
        // load(...) works
        writeStringContent(tempProblemDir() + "/tests.html", QString::fromStdString(results));
        webView->load(QUrl::fromLocalFile(tempProblemDir() + "/tests.html"));
    }
    // scroll down
    webView->page()->mainFrame()->setScrollBarValue(Qt::Vertical, webView->page()->mainFrame()->scrollBarMaximum(Qt::Vertical));
}

void UnitTestsWidget::showInfoTest(QAction *act)
{
    showInfoTests(act->data().toString());
}

void UnitTestsWidget::readTestFromDisk(const QString& fileName)
{
    if (!QFile::exists(fileName))
        return;

    try
    {
        std::auto_ptr<XMLTest::test> test_xsd = XMLTest::test_(compatibleFilename(fileName).toStdString(), xml_schema::flags::dont_validate);
        m_test = *test_xsd.get();

        showInfoTests();
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }
}

void UnitTestsWidget::readTestsFromSuite()
{
    QSettings settings;

    trvTests->clear();

    // run expression
    currentPythonEngine()->runExpression(QString("import test_suite; agros2d_tests = []; test_suite.scenario.find_all_tests(test_suite, agros2d_tests)"));

    // extract values
    PyObject *result = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_tests");
    if (result)
    {
        QTreeWidgetItem *moduleItem = NULL;
        QString previousModule;

        QFont fnt = trvTests->font();
        fnt.setBold(true);

        Py_INCREF(result);
        for (int i = 0; i < PyList_Size(result); i++)
        {
            PyObject *list = PyList_GetItem(result, i);
            Py_INCREF(list);

            QString module = PyBytes_AsString(PyList_GetItem(list, 1));
            QString name = PyBytes_AsString(PyList_GetItem(list, 0));

            QString key = QString("UnitTestsWidget/Tests/%1/%2").arg(module).arg(name);

            // create module item
            if (previousModule.isEmpty() || previousModule != module)
            {
                moduleItem = new QTreeWidgetItem(trvTests);
                moduleItem->setText(0, module.right(module.count() - module.indexOf(".") - 1));
                moduleItem->setData(0, Qt::UserRole, module);
                moduleItem->setData(1, Qt::UserRole, "");
                moduleItem->setFont(0, fnt);
                moduleItem->setExpanded(true);

                // set previous module
                previousModule = module;
            }

            QTreeWidgetItem *classItem = new QTreeWidgetItem(moduleItem);
            classItem->setText(0, QString("%1").arg(name));
            classItem->setData(0, Qt::UserRole, module);
            classItem->setData(1, Qt::UserRole, name);
            if (settings.value(key, false).toBool())
                classItem->setCheckState(0, Qt::Checked);
            else
                classItem->setCheckState(0, Qt::Unchecked);

            QStringList testsList;
            PyObject *tests = PyList_GetItem(list, 2);
            Py_INCREF(tests);
            for (int j = 0; j < PyList_Size(tests); j++)
            {
                PyObject *test = PyList_GetItem(tests, j);
                Py_INCREF(test);

                QString name = PyBytes_AsString(PyList_GetItem(tests, j));
                testsList << name;

                Py_XDECREF(test);
            }
            Py_XDECREF(tests);
            Py_XDECREF(list);

            classItem->setData(2, Qt::UserRole, testsList);
        }
        Py_XDECREF(result);
    }

    // remove variables
    currentPythonEngine()->runExpression("del agros2d_tests");
}

void UnitTestsWidget::readScenariosFromSuite()
{
    QMenu *menu = new QMenu();
    connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(readTestsSettingsFromScenario(QAction *)));

    QStringList list = currentPythonEngineAgros()->testSuiteScenarios();

    foreach (QString test, list)
    {
        QString testName = test;
        testName.remove("test_");

        QAction *act = new QAction(testName, this);
        act->setData(test);

        menu->addAction(act);
    }

    btnScenarios->setMenu(menu);
}

void UnitTestsWidget::saveTestsSettings()
{
    QSettings settings;

    foreach (QTreeWidgetItem *item, availableTests())
    {
        QString key = QString("UnitTestsWidget/Tests/%1/%2").
                arg(item->data(0, Qt::UserRole).toString()).
                arg(item->data(1, Qt::UserRole).toString());
        if (item->checkState(0) == Qt::Checked)
            settings.setValue(key, true);
        else
            settings.remove(key);
    }
}

void UnitTestsWidget::stopTest()
{
    m_isAborted = true;
}

void UnitTestsWidget::setEnabledControls(bool state)
{
    trvTests->setEnabled(state);
    btnRunTests->setEnabled(state);
    btnStopTest->setEnabled(!state);
    btnScenarios->setEnabled(state);
    btnUncheckTests->setEnabled(state);
    buttonBox->setEnabled(state);
}

void UnitTestsWidget::doAccept()
{
    saveTestsSettings();
    accept();
}

void UnitTestsWidget::doReject()
{
    reject();
}

