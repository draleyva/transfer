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
<#macro createverctxline mylist>
<#list mylist as entry><#if entry.module??>
${entry.module?right_pad(24)}<#if entry.base??>${entry.base}-<#if entry.moduleversion??>${entry.moduleversion}<#else>${entry.version}</#if><#else>${entry.name}-<#if entry.moduleversion??>${entry.moduleversion}<#else>${entry.version}</#if></#if>
</#if></#list>
</#macro>
<#assign aDateTime = .now>
# Last Updated : ${aDateTime?iso("America/Lima")}
#***********************************************************************
# File: verctx
#
# Purpose: To direct 'p4 sync [-n] [-f] default [...| files ] to which
#          labels code areas should be synchronised to.
#
# Label options: A label can be one of the following:
#          latest       (this will synchronise to the latest revisions)
#
#          label        (where label has the following format)
#
# Label format:
#       (code section)_(cortex code branch)
#       .(major).(minor).(point release)-(consolidated patch).(patch)
#
#       e.g. visa_main.1.0.0-0.2 , new_bph25.1.0.0-0.2 ,
#            ep_main.1.1.0-0.3 , common_main.2.5.1-0.4 ,
#            custom_main.1.1.0-0.3 , custom_bph25.1.0.0-0.2
#
# NOTE: The code section custom refers to module src
#       If modules are not mentioned here , they will not by sync'd
#***********************************************************************
#
#code area      label
#
<@createverctxline modulelist/>
<@createverctxline customlist/>
