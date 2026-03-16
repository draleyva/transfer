<#assign productjson = JsonPathTool.parse(Documents.get(0))>
<#assign hostname = productjson.read("$.hostname")>
<#assign p4client = productjson.read("$.p4client")>
<#assign p4root = productjson.read("$.p4root")>
<#assign utilist = productjson.read("$.entry.util")>
<#assign baselist = productjson.read("$.entry.base")>
<#assign customlist = productjson.read("$.entry.custom")>
<#assign modulelist = productjson.read("$.entry.module")>
<#assign COMPUTERNAME = SystemTool.envs["COMPUTERNAME"]!"">
<#assign USERNAME = SystemTool.envs["USERNAME"]!"">
<#assign TARGETOS = SystemTool.getProperty("TARGETOS", "Windows_NT")>
<#macro createviewline mylist>
<#list mylist as entry>
    ${entry.p4} //${p4client}${entry.target}
</#list>
</#macro>
<#assign aDateTime = .now>
# Last Updated : ${aDateTime?iso("America/Lima")}
Client: ${p4client}
Owner: ${USERNAME}
Host: <#if TARGETOS="Windows_NT">${COMPUTERNAME}<#else>${hostname}</#if>
Root: ${p4root}
Options: noallwrite noclobber nocompress unlocked nomodtime normdir
SubmitOptions: submitunchanged
LineEnd: unix
View:
<@createviewline utilist/>
<@createviewline baselist/>
<@createviewline modulelist/>
<@createviewline customlist/>
