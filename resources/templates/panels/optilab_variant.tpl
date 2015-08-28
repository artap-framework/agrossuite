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

<h1>{{NAME}}</h1>

<div class="section">
<h2>{{PARAMETER_LABEL}}</h2>
<hr/>
<table>
{{#PARAMETER_SECTION}}
	<tr><td><b>{{PARAMETER_LABEL}}</b></td><td>{{PARAMETER_VALUE}}</td></td>
{{/PARAMETER_SECTION}}
</table>
</div>

<div class="section">
<h2>{{VARIABLE_LABEL}}</h2>
<hr/>
<table>
{{#VARIABLE_SECTION}}
	<tr>
		<td><b>{{VARIABLE_LABEL}}</b></td>
		<td>{{VARIABLE_VALUE}}</td>
	</tr>		
{{/VARIABLE_SECTION}}
</table>
{{#VARIABLE_CHART_SECTION}}
<div id="{{VAR_CHART_DIV}}" style="width:95%;height:150px;"></div>{{VAR_CHART}}
{{/VARIABLE_CHART_SECTION}}
</div>

<div class="section">
<h2>{{INFO_LABEL}}</h2>
<hr/>
<table>
{{#INFO_SECTION}}
	<tr><td><b>{{INFO_LABEL}}</b></td><td>{{INFO_VALUE}}</td></td>
{{/INFO_SECTION}}
</table>
</div>

<div class="cleaner"></div>

</body>
</html>
