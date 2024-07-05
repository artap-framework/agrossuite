// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include <QTextEdit>

#include "infowidget.h"

#include "util/constants.h"
#include "util/global.h"
#include "util/point.h"

#include "gui/common.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "solver/module.h"
#include "solver/coupling.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/problem_result.h"
#include "solver/problem_parameter.h"
#include "solver/problem_function.h"
#include "solver/solutionstore.h"

#include "optilab/study.h"

#include "ctemplate/template.h"

#include "app/sceneview_common.h"
#include "app/sceneview_geometry.h"
#include "app/scenemarkerdialog.h"
#include "app/examplesdialog.h"

InfoWidgetGeneral::InfoWidgetGeneral(QWidget *parent)
    : QWidget(parent)
{
    webEdit = new QTextEdit();
    webEdit->setReadOnly(true);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(webEdit);

    setLayout(layoutMain);
}

InfoWidgetGeneral::~InfoWidgetGeneral()
{
    // QFile::remove(tempProblemDir() + "/info.html");
}

void InfoWidgetGeneral::clear()
{    
    webEdit->setHtml("");
}

void InfoWidgetGeneral::showProblemInfo(ProblemBase *problem, const QString &name)
{
    // return;

    // template
    std::string info;
    ctemplate::TemplateDictionary problemInfo("info");

    problemInfo.SetValue("AGROS", compatibleFilename(QDir(Agros::dataDir() + TEMPLATEROOT + "/agros_logo.png").absolutePath()).toStdString());

    problemInfo.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(Agros::dataDir()).absolutePath()).arg(TEMPLATEROOT)).toString().toStdString());

    problemInfo.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

    problemInfo.SetValue("NAME_LABEL", tr("Name:").toStdString());
    if (!name.isEmpty())
        problemInfo.SetValue("NAME", name.toStdString());
    else
        if (Problem *preprocessor = dynamic_cast<Problem *>(problem))
            problemInfo.SetValue("NAME", QFileInfo(preprocessor->archiveFileName()).baseName().toStdString());

    // general
    problemInfo.SetValue("GENERAL_LABEL", tr("General").toStdString());
    problemInfo.SetValue("COORDINATE_TYPE_LABEL", tr("Coordinate type:").toStdString());
    problemInfo.SetValue("COORDINATE_TYPE", coordinateTypeString(problem->config()->coordinateType()).toStdString());
    problemInfo.SetValue("MESH_TYPE_LABEL", tr("Mesh type:").toStdString());
    problemInfo.SetValue("MESH_TYPE", meshTypeString(problem->config()->meshType()).toStdString());

    // harmonic analysis
    if (problem->isHarmonic())
        problemInfo.ShowSection("HARMONIC");

    problemInfo.SetValue("HARMONIC_LABEL", tr("Harmonic analysis").toStdString());
    problemInfo.SetValue("HARMONIC_FREQUENCY_LABEL", tr("Frequency:").toStdString());
    problemInfo.SetValue("HARMONIC_FREQUENCY", problem->config()->value(ProblemConfig::Frequency).value<Value>().toString().toStdString() + " Hz");

    // transient analysis
    if (problem->isTransient())
        problemInfo.ShowSection("TRANSIENT");

    problemInfo.SetValue("TRANSIENT_LABEL", tr("Transient analysis").toStdString());
    problemInfo.SetValue("TRANSIENT_STEP_METHOD_LABEL", tr("Method:").toStdString());
    problemInfo.SetValue("TRANSIENT_STEP_METHOD", timeStepMethodString((TimeStepMethod) problem->config()->value(ProblemConfig::TimeMethod).toInt()).toStdString());
    problemInfo.SetValue("TRANSIENT_STEP_ORDER_LABEL", tr("Order:").toStdString());
    problemInfo.SetValue("TRANSIENT_STEP_ORDER", QString::number(problem->config()->value(ProblemConfig::TimeOrder).toInt()).toStdString());
    problemInfo.SetValue("TRANSIENT_TOLERANCE_LABEL", tr("Tolerance (%):").toStdString());
    problemInfo.SetValue("TRANSIENT_TOLERANCE", QString::number(problem->config()->value(ProblemConfig::TimeMethodTolerance).toDouble()).toStdString());
    problemInfo.SetValue("TRANSIENT_INITIALTIMESTEP_LABEL", tr("Initial step size:").toStdString());
    problemInfo.SetValue("TRANSIENT_INITIALTIMESTEP", QString::number(problem->config()->value(ProblemConfig::TimeInitialStepSize).toDouble()).toStdString());
    problemInfo.SetValue("TRANSIENT_CONSTANT_STEP_LABEL", tr("Constant time step:").toStdString());
    problemInfo.SetValue("TRANSIENT_CONSTANT_STEP", QString::number(problem->config()->constantTimeStepLength()).toStdString() + " s");
    problemInfo.SetValue("TRANSIENT_CONSTANT_NUM_STEPS_LABEL", tr("Number of const. time steps:").toStdString());
    problemInfo.SetValue("TRANSIENT_CONSTANT_NUM_STEPS", QString::number(problem->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()).toStdString());
    problemInfo.SetValue("TRANSIENT_TOTAL_LABEL", tr("Total time:").toStdString());
    problemInfo.SetValue("TRANSIENT_TOTAL", QString::number(problem->config()->value(ProblemConfig::TimeTotal).toDouble()).toStdString() + " s");

    // geometry
    problemInfo.SetValue("GEOMETRY_LABEL", tr("Geometry").toStdString());

    // Load your SVG
    QSvgRenderer renderer(generateSvgGeometry(problem->scene()->faces->items()).toUtf8());

    // Prepare a QImage with desired characteritisc
    RectPoint boundingBox = SceneFaceContainer::boundingBox(problem->scene()->faces->items());

    double width = 340;
    double height = 340;
    if (boundingBox.width() >= boundingBox.height())
        height = height * boundingBox.height() / boundingBox.width();
    else
        width = width * boundingBox.width() / boundingBox.height();

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    // Get QPainter that paints to the image
    QPainter painter(&image);
    renderer.render(&painter);

    // Save, image format based on file extension
    QString imgName = tempProblemDir() + QString("/geometry_%1.png").arg(QRandomGenerator::global()->generate());
    image.save(imgName);

    problemInfo.SetValue("GEOMETRY_PNG", QString(imgName).toStdString());

    // parameters
    QMap<QString, ProblemParameter> parameters = problem->config()->parameters()->items();
    if (parameters.count() > 0)
    {
        problemInfo.SetValue("PARAMETERS_MAIN_LABEL", tr("Parameters").toStdString());
        foreach (QString key, parameters.keys())
        {
            ctemplate::TemplateDictionary *parametersSection = problemInfo.AddSectionDictionary("PARAMETERS_SECTION");

            parametersSection->SetValue("PARAMETERS_VARIABLE_NAME", key.toStdString());
            parametersSection->SetValue("PARAMETERS_VARIABLE_VALUE", QString::number(parameters[key].value()).toStdString());
        }
        problemInfo.ShowSection("PARAMETERS");
    }

    // functions
    QMap<QString, ProblemFunction *> functions = problem->config()->functions()->items();
    if (functions.count() > 0)
    {
        problemInfo.SetValue("FUNCTIONS_MAIN_LABEL", tr("Functions").toStdString());
        foreach (QString key, functions.keys())
        {
            ctemplate::TemplateDictionary *functionsSection = problemInfo.AddSectionDictionary("FUNCTIONS_SECTION");

            functionsSection->SetValue("FUNCTIONS_VARIABLE_NAME", key.toStdString());
            // functionsSection->SetValue("FUNCTIONS_VARIABLE_VALUE", QString::number(functions[key]->value()).toStdString());
        }
        problemInfo.ShowSection("FUNCTIONS");
    }

    // fields
    if (problem->fieldInfos().count() > 0)
    {
        problemInfo.SetValue("PHYSICAL_FIELD_MAIN_LABEL", tr("Physical fields").toStdString());

        foreach (FieldInfo *fieldInfo, problem->fieldInfos())
        {
            ctemplate::TemplateDictionary *field = problemInfo.AddSectionDictionary("FIELD_SECTION");

            field->SetValue("PHYSICAL_FIELD_LABEL", fieldInfo->name().toStdString());
            field->SetValue("PHYSICAL_FIELD_ID", fieldInfo->fieldId().toStdString());
            field->SetValue("ANALYSIS_TYPE_LABEL", tr("Analysis:").toStdString());
            field->SetValue("ANALYSIS_TYPE", analysisTypeString(fieldInfo->analysisType()).toStdString());
            field->SetValue("LINEARITY_TYPE_LABEL", tr("Solver:").toStdString());
            field->SetValue("LINEARITY_TYPE", linearityTypeString(fieldInfo->linearityType()).toStdString());
            field->SetValue("REFINEMENTS_NUMBER_TYPE_LABEL", tr("Number of refinements:").toStdString());
            field->SetValue("REFINEMENTS_NUMBER_TYPE", QString::number(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt()).toStdString());
            field->SetValue("POLYNOMIAL_ORDER_TYPE_LABEL", tr("Polynomial order:").toStdString());
            field->SetValue("POLYNOMIAL_ORDER_TYPE", QString::number(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()).toStdString());
            field->SetValue("ADAPTIVITY_TYPE_LABEL", tr("Adaptivity:").toStdString());
            field->SetValue("ADAPTIVITY_TYPE", adaptivityTypeString(fieldInfo->adaptivityType()).toStdString());
            field->SetValue("MATRIX_SOLVER_TYPE_LABEL", tr("Matrix solver:").toStdString());
            field->SetValue("MATRIX_SOLVER_TYPE", matrixSolverTypeString(fieldInfo->matrixSolver()).toStdString());
        }

        problemInfo.ShowSection("FIELD");
    }

    if (problem->couplingInfos().count() > 0)
    {
        problemInfo.SetValue("COUPLING_MAIN_LABEL", tr("Coupled fields").toStdString());

        foreach (CouplingInfo *couplingInfo, problem->couplingInfos())
        {
            ctemplate::TemplateDictionary *couplingSection = problemInfo.AddSectionDictionary("COUPLING_SECTION");

            couplingSection->SetValue("COUPLING_LABEL", couplingInfo->name().toStdString());

            couplingSection->SetValue("COUPLING_SOURCE_LABEL", tr("Source:").toStdString());
            couplingSection->SetValue("COUPLING_SOURCE", couplingInfo->sourceFieldId().toStdString());
            couplingSection->SetValue("COUPLING_TARGET_LABEL", tr("Target:").toStdString());
            couplingSection->SetValue("COUPLING_TARGET", couplingInfo->targetFieldId().toStdString());
            couplingSection->SetValue("COUPLING_TYPE_LABEL", tr("Coupling type:").toStdString());
            couplingSection->SetValue("COUPLING_TYPE", QString("%1").arg(couplingTypeString(couplingInfo->couplingType())).toStdString());
        }
        problemInfo.ShowSection("COUPLING");
    }

    // details
    if (Problem *preprocessor = dynamic_cast<Problem *>(problem))
    {
        // studies
        if (preprocessor->studies()->items().count() > 0)
        {
            problemInfo.SetValue("STUDIES_MAIN_LABEL", tr("Studies").toStdString());

            foreach (Study *study, preprocessor->studies()->items())
            {
                ctemplate::TemplateDictionary *studySection = problemInfo.AddSectionDictionary("STUDY_SECTION");

                studySection->SetValue("STUDY_TYPE_LABEL", tr("Type:").toStdString());
                studySection->SetValue("STUDY_TYPE", studyTypeString(study->type()).toStdString());
            }

            problemInfo.ShowSection("STUDY");
        }

        if (!preprocessor->archiveFileName().isEmpty())
        {
            QFileInfo fileInfo(preprocessor->archiveFileName());
            QString detailsFilename(QString("%1/%2/index.html").arg(fileInfo.absolutePath()).arg(fileInfo.baseName()));
            if (QFile::exists(detailsFilename))
            {
                // replace current path in index.html
                QString detail = readFileContent(detailsFilename);
                detail = detail.replace("{{DIR}}", QString("%1/%2").arg(QUrl::fromLocalFile(fileInfo.absolutePath()).toString()).arg(fileInfo.baseName()));
                detail = detail.replace("{{RESOURCES}}", QUrl::fromLocalFile(QString("%1/resources/").arg(QDir(Agros::dataDir()).absolutePath())).toString());

                problemInfo.SetValue("PROBLEM_DETAILS", detail.toStdString());
            }
        }
    }

    ctemplate::ExpandTemplate(compatibleFilename(Agros::dataDir() + TEMPLATEROOT + "/problem.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &problemInfo, &info);

    webEdit->clear();
    webEdit->setHtml(QString::fromStdString(info));
    // qInfo() << QString::fromStdString(info);

    // remove image
    QFile::remove(imgName);
}

// info

InfoWidget::InfoWidget(QWidget *parent)
    : InfoWidgetGeneral(parent)
{
    welcome();
}

InfoWidget::~InfoWidget()
{
    QFile::remove(tempProblemDir() + "/info.html");
}

void InfoWidget::welcome()
{
    // template
    std::string info;
    ctemplate::TemplateDictionary problemInfo("welcome");

    problemInfo.SetValue("AGROS", "file:///" + compatibleFilename(QDir(Agros::dataDir() + TEMPLATEROOT + "/agros_logo.png").absolutePath()).toStdString());
    problemInfo.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(Agros::dataDir()).absolutePath()).arg(TEMPLATEROOT)).toString().toStdString());

    ctemplate::ExpandTemplate(compatibleFilename(Agros::dataDir() + TEMPLATEROOT + "/welcome.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &problemInfo, &info);

    webEdit->setHtml(QString::fromStdString(info));
}

