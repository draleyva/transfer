<#assign productjson = JsonPathTool.parse(Documents.get(0))>
<#assign client = productjson.read("$.client")>
<#assign p4client = productjson.read("$.p4client")>
<#assign p4root = productjson.read("$.p4root")>
<#assign p4security = productjson.read("$.p4security")!"">
<#assign p4sync = productjson.read("$.p4sync")>
<#assign target = productjson.read("$.target")>
<#assign util = productjson.read("$.util")>
<#assign utilist = productjson.read("$.entry.util")>
<#assign baselist = productjson.read("$.entry.base")>
<#assign customlist = productjson.read("$.entry.custom")>
<#assign modulelist = productjson.read("$.entry.module")>
<#assign CORTEXVC = SystemTool.envs["CORTEXVC"]!"">
<#assign USERNAME = SystemTool.envs["USERNAME"]!"">
<#assign TARGETOS = SystemTool.getProperty("TARGETOS", "Windows_NT")>
<#macro getlabel entry>
<@compress single_line=true>
<#if entry.version??><#if entry.base??>@${entry.base}-${entry.version}<#else>@${entry.name}-${entry.version}</#if><#else></#if>
</@compress>
</#macro>
<#macro getname entry>
<@compress single_line=true>
<#if entry.version??><#if entry.base??>${entry.name} ${entry.base}-${entry.version}<#else>${entry.name}-${entry.version}</#if><#else>${entry.name}</#if>
</@compress>
</#macro>
<#macro createsyncline mylist>
<#list mylist as entry>
@echo downloading <@getname entry/>
@${p4sync} ${entry.p4}<@getlabel entry/>
</#list>
</#macro>
<#macro createlinuxsyncline mylist>
<#list mylist as entry>
echo downloading <@getname entry/>
${p4sync} ${entry.p4}<@getlabel entry/> >/dev/null
</#list>
</#macro>
<#assign aDateTime = .now>
<#if TARGETOS="Windows_NT" >
# Last Updated : ${aDateTime?iso("America/Lima")}
@setlocal
@echo off
:: set environment
set P4ROOT=${p4root}
set P4PORT=linkous.fnfis.com:1666
set P4CLIENT=${p4client}
@call %FISDEVELOPMENT%\${USERNAME}\shell\p4.credentials.cmd
set UTIL=${util}

:: login
p4 login -a < %FISDEVELOPMENT%\${USERNAME}\shell\%P4SECURITY%

if NOT ["%errorlevel%"]==["0"] (
@echo Connection to p4 has failed
exit /b %errorlevel%
)

:: workspace
@p4 client -d %P4CLIENT%
@p4 client -i < ${CORTEXVC}.template

@cd %P4ROOT%

<@createsyncline utilist/>
<@createsyncline baselist/>
<@createsyncline modulelist/>
<@createsyncline customlist/>

@p4 logout

@echo Creating packages
@cd ${p4root}${target}
@del /q /s ${util}-${p4client}.tar 2>NUL
@del /q /s core-${p4client}.tar 2>NUL
@del /q /s custom-${p4client}.tar 2>NUL
@echo Creating bin package
@tar -mcf ${util}-${p4client}.tar ${util}
@echo Creating core package
@tar -cf core-${p4client}.tar modules combined
@cd ${p4root}${target}\${client}
@echo Creating custom package
@tar -mcf custom-${p4client}.tar cust_prod glimpse_index src
@move /Y custom-${p4client}.tar ..\
<#else>
# Last Updated : ${aDateTime?iso("America/Lima")}
#!/bin/bash
export P4ROOT=${p4root}
export P4PORT=linkous.fnfis.com:1666
export P4CLIENT=${p4client}
export P4USER=${USERNAME}

p4 login -a

RETURN=$?

if [ $RETURN -ne 0 ]; then
  echo "Connection to p4 has failed"
  exit 
fi

p4 client -d $P4CLIENT >/dev/null
p4 client -i < ${CORTEXVC}.template >/dev/null

mkdir -p $P4ROOT

cd $P4ROOT

<@createlinuxsyncline utilist/>
<@createlinuxsyncline baselist/>
<@createlinuxsyncline modulelist/>
<@createlinuxsyncline customlist/>

p4 logout

echo Creating packages
cd ${p4root}${target}
rm -rf ${util}-${p4client}.* 2>/dev/null
rm -rf core-${p4client}.* 2>/dev/null
rm -rf custom-${p4client}.* 2>/dev/null
echo Creating bin package
tar -mcf ${util}-${p4client}.tar ${util}
gzip ${util}-${p4client}.tar
echo Creating core package
tar -cf core-${p4client}.tar modules combined
gzip core-${p4client}.tar
cd ${p4root}${target}/${client}
echo Creating custom package
tar -mcf custom-${p4client}.tar cust_prod glimpse_index src
gzip custom-${p4client}.tar
mv custom-${p4client}.* ../
</#if>
