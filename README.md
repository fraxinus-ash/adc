# adc
Simple access for c++ developers to Adabas using direct calls

<h3>Content</h3>
The repository contains the three classes ADC, ADCSession and ADCMainTest. ADC wraps the direct call to Adabas, ADCSession provides session handling and ADCMainTest shows how all can be used.

<h3>Prerequisites</h3>
<ul>
  <li>Adabas (Adaptable Database System) by Software AG. There is a free trial version on http://techcommunity.softwareag.com/</li>
  <li>C++ compiler</li>
</ul>

<h3>Hints for compilation</h3>
The classes were tested on solaris and windows. In both cases the program have to be linked to an Adabas interface, usually adalnkx. Best is to adapt the makefile which comes with Adabase installation files (section client examples).

<h3>Running the application</h3>
The application runs against the Adabas demo file “Employees”. Since you have to specify the appropriate database and file number:<br>
&ltprogram&gt &ltdemo-dbid&gt &ltemployee-demo file&gt
