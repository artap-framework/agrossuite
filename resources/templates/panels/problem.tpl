<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<style type="text/css">
		{{STYLESHEET}}
	</style>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.js"></script>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.axislabels.js"></script>
</head>
<body>

<img style="float: right; margin-right: 10px; margin-top: 12px;" src="{{AGROS2D}}" /> 
<h1>{{NAME}}</h1>

<table>
<tr>
<!-- general -->
<td style="width: 25%;">
<div class="section">
<h2>{{GENERAL_LABEL}}</h2>
<hr/>
<table>
    <tr><td><b>{{COORDINATE_TYPE_LABEL}}</b></td><td>{{COORDINATE_TYPE}}</td></tr>
    <tr><td><b>{{MESH_TYPE_LABEL}}</b></td><td>{{MESH_TYPE}}</td></tr>
</table>
</td>

<!-- harmonic -->
{{#HARMONIC}}
<td style="width: 15%;">
<div class="section">
<h2>{{HARMONIC_LABEL}}</h2>
<hr/>
<table>
    <tr><td><b>{{HARMONIC_FREQUENCY_LABEL}}</b></td><td>{{HARMONIC_FREQUENCY}}</td></tr>
</table>
</td>
{{/HARMONIC}}

{{#TRANSIENT}}
<td style="width: 40%;">
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
</td>
{{/TRANSIENT}}

</tr>

<tr>
<!-- geometry -->
<td style="width: 25%;">
<div class="section">
<h2>{{GEOMETRY_LABEL}}</h2>
<hr/>
<table>
    <tr><td rowspan="6"><div class="figure">{{GEOMETRY_SVG}}</div></td><td><b>{{GEOMETRY_NODES_LABEL}}</b></td><td>{{GEOMETRY_NODES}}</td></tr>
    <tr><td><b>{{GEOMETRY_EDGES_LABEL}}</b></td><td>{{GEOMETRY_EDGES}}</td></tr>
    <tr><td><b>{{GEOMETRY_LABELS_LABEL}}</b></td><td>{{GEOMETRY_LABELS}}</td></tr>
    <tr><td><b>&nbsp;</b></td><td>&nbsp;</td></tr>
    <tr><td><b>{{GEOMETRY_MATERIALS_LABEL}}</b></td><td>{{GEOMETRY_MATERIALS}}</td></tr>
    <tr><td><b>{{GEOMETRY_BOUNDARIES_LABEL}}</b></td><td>{{GEOMETRY_BOUNDARIES}}</td></tr>
</table>
</div>

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

<!-- parameters -->
<td style="width: 15%;">
<div class="section">
<h2>{{PARAMETERS_MAIN_LABEL}}</h2>
<hr/>
<table>
{{#PARAMETERS_SECTION}}
    <tr><td><b>{{PARAMETERS_VARIABLE_NAME}}</b></td><td>{{PARAMETERS_VARIABLE_VALUE}}</td>
{{/PARAMETERS_SECTION}}
</table>
</div>
</td>

<!-- fields -->
<td style="width: 40%;">
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
{{/FIELD}}
</td>
</tr>
</table>

{{#SOLUTION_PARAMETERS_SECTION}}
<div class="section">
<h2>{{SOLUTION_LABEL}}</h2>
<hr/>
<table>
    <tr><td><b>{{SOLUTION_ELAPSED_TIME_LABEL}}</b></td><td>{{SOLUTION_ELAPSED_TIME}}</td></tr>
    <tr><td><b>{{NUM_THREADS_LABEL}}</b></td><td>{{NUM_THREADS}}</td></tr>
</table>
{{#TRANSIENT_ADAPTIVE}}
<div style="text-align: center; width: 50%; height: 160px;">Time step length<br/><div id="chart_time_step_length" style="width: 100%; height: 90%;"></div></div>
{{TIME_STEPS_CHART}}
{{/TRANSIENT_ADAPTIVE}}
</div>
{{/SOLUTION_PARAMETERS_SECTION}}
</td>

{{PROBLEM_DETAILS}}

<div class="cleaner"></div>

<!--
<script type="text/javascript">
function showTooltip(x, y, contents) 
{
    $('<div id="tooltip">' + contents + '</div>').css({
        position: 'absolute',
        display: 'none',
        top: y + 5,
        left: x + 5,
        border: '1px solid #fdd',
        padding: '2px', 
        background-color: '#fee',
        opacity: 0.80
    }).appendTo("body").fadeIn(200);
}
</script>
-->
</body>
</html>

