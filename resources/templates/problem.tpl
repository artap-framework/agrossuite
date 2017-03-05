<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros Suite" />
	<style type="text/css">
		{{STYLESHEET}}
	</style>
    <!--
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.js"></script>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.axislabels.js"></script>
    -->
</head>
<body>

<img style="float: right; margin-right: 10px; margin-top: 12px;" src="{{AGROS}}" /> 
<h1>{{NAME}}</h1>

<table>
<tr>
<!-- general -->
<td style="width: 50%;">
<div class="section">
<h2>{{GENERAL_LABEL}}</h2>
<hr/>
<table>
    <tr><td><b>{{COORDINATE_TYPE_LABEL}}</b></td><td>{{COORDINATE_TYPE}}</td></tr>
    <tr><td><b>{{MESH_TYPE_LABEL}}</b></td><td>{{MESH_TYPE}}</td></tr>
</table>
</div>

<!-- geometry -->
<div class="section">
<h2>{{GEOMETRY_LABEL}}</h2>
<hr/>
    <div class="figure">{{GEOMETRY_SVG}}</div>
</div>

<!-- coupling -->
{{#COUPLING}}
<div class="section">
<h2>{{COUPLING_MAIN_LABEL}}</h2>
<hr/>
{{#COUPLING_SECTION}}
<table>
	<tr><td><b>{{COUPLING_LABEL}}</b></td><td>&nbsp;</td>
	<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<b>{{COUPLING_SOURCE_LABEL}}</b></td><td>{{COUPLING_SOURCE}}</td></tr>
	<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<b>{{COUPLING_TARGET_LABEL}}</b></td><td>{{COUPLING_TARGET}}</td></tr>
	<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<b>{{COUPLING_TYPE_LABEL}}</b></td><td>{{COUPLING_TYPE}}</td></tr>
</table>
{{/COUPLING_SECTION}}
</div>
{{/COUPLING}}

<!-- harmonic -->
{{#HARMONIC}}
<div class="section">
<h2>{{HARMONIC_LABEL}}</h2>
<hr/>
<table>
    <tr><td><b>{{HARMONIC_FREQUENCY_LABEL}}</b></td><td>{{HARMONIC_FREQUENCY}}</td></tr>
</table>
</div>
{{/HARMONIC}}

<!-- transient -->
{{#TRANSIENT}}
<div class="section">
<h2>{{TRANSIENT_LABEL}}</h2>
<hr/>
<table>
    <tr><td><b>{{TRANSIENT_STEP_METHOD_LABEL}}</b></td><td>{{TRANSIENT_STEP_METHOD}}</td></tr>
    <tr><td><b>{{TRANSIENT_STEP_ORDER_LABEL}}</b></td><td>{{TRANSIENT_STEP_ORDER}}</td></tr>
    <tr><td><b>{{TRANSIENT_TOLERANCE_LABEL}}</b></td><td>{{TRANSIENT_TOLERANCE}}</td></tr>
    <tr><td><b>{{TRANSIENT_INITIALTIMESTEP_LABEL}}</b></td><td>{{TRANSIENT_INITIALTIMESTEP}}</td></tr>
    <tr><td><b>{{TRANSIENT_CONSTANT_NUM_STEPS_LABEL}}</b></td><td>{{TRANSIENT_CONSTANT_NUM_STEPS}}</td></tr>
    <tr><td><b>{{TRANSIENT_CONSTANT_STEP_LABEL}}</b></td><td>{{TRANSIENT_CONSTANT_STEP}}</td></tr>
    <tr><td><b>{{TRANSIENT_TOTAL_LABEL}}</b></td><td>{{TRANSIENT_TOTAL}}</td></tr>
</table>
</div>
{{/TRANSIENT}}

<!-- parameters -->
{{#PARAMETERS}}
<div class="section">
<h2>{{PARAMETERS_MAIN_LABEL}}</h2>
<hr/>
<table>
{{#PARAMETERS_SECTION}}
    <tr><td><b>{{PARAMETERS_VARIABLE_NAME}}</b></td><td>{{PARAMETERS_VARIABLE_VALUE}}</td></tr>
{{/PARAMETERS_SECTION}}
</table>
</div>
{{/PARAMETERS}}

{{#RESULTS}}
<div class="section">
<h2>{{RESULTS_MAIN_LABEL}}</h2>
<hr/>
<table>
{{#RESULTS_SECTION}}
    <tr><td><b>{{RESULTS_VARIABLE_NAME}}</b></td><td>{{RESULTS_VARIABLE_VALUE}}</td></tr>
{{/RESULTS_SECTION}}
</table>
</div>
{{/RESULTS}}

</td>
<td style="width: 50%;">

<!-- fields -->
{{#FIELD}}
{{#FIELD_SECTION}}
<div class="section">
<h2>{{PHYSICAL_FIELD_LABEL}}</h2>
<hr/>
<table>
    <tr><td><b>{{ANALYSIS_TYPE_LABEL}}</b></td><td>{{ANALYSIS_TYPE}}</td></tr>
    <tr><td><b>{{REFINEMENTS_NUMBER_TYPE_LABEL}}</b></td><td>{{REFINEMENTS_NUMBER_TYPE}}</td></tr>
    <tr><td><b>{{POLYNOMIAL_ORDER_TYPE_LABEL}}</b></td><td>{{POLYNOMIAL_ORDER_TYPE}}</td></tr>
    <tr><td><b>{{LINEARITY_TYPE_LABEL}}</b></td><td>{{LINEARITY_TYPE}}</td></tr>
    <tr><td><b>{{MATRIX_SOLVER_TYPE_LABEL}}</b></td><td>{{MATRIX_SOLVER_TYPE}}</td></tr>
    <tr><td><b>{{ADAPTIVITY_TYPE_LABEL}}</b></td><td>{{ADAPTIVITY_TYPE}}</td></tr>
</table>
{{/FIELD_SECTION}}
</div>
{{/FIELD}}

<!-- studies -->
{{#STUDY}}
<h2>{{STUDIES_MAIN_LABEL}}</h2>
<hr/>
<div class="section">
<table>
{{#STUDY_SECTION}}
    <tr><td><b>{{STUDY_TYPE_LABEL}}</b></td><td>{{STUDY_TYPE}}</td></tr>
{{/STUDY_SECTION}}
</table>
</div>
{{/STUDY}}

</td>
</tr>
</table>

{{PROBLEM_DETAILS}}

<div class="cleaner"></div>

</body>
</html>

