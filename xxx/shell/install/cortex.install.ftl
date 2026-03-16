<#assign modulesjson = JsonPathTool.parse(Documents.get(0))>
<#assign client = modulesjson.read("$.client")>
<#assign installation = modulesjson.read("$.installation")>
<#assign package = modulesjson.read("$.package")>
<#assign modules = modulesjson.read("$.modules")>
<#assign backup = modulesjson.read("$.backup")>
<#assign TYPE = SystemTool.getProperty("TYPE", "")>
#!/bin/sh
BLACK='\033[0;30m'
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
PINK='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[0;37m'
NORMAL='\033[0;39m'
<#function moduletype type>
  <#switch type>
    <#case "bin">
      <#return "bin.crm">
    <#break>
    <#case "tuxfbuf">
      <#return "tuxfbuf.crm">
    <#break>
    <#case "db">
      <#return "db">
    <#break>
  </#switch>
</#function>
backupdir=backup`date +%Y%m%d%H%M%S`
LOGFILE=$PWD/cortex.install.log
exe() { echo $(date  +"%Y-%m-%dT%T.%6N"%z)"|$@">>$LOGFILE ; "$@" ; }
title() { echo $(date  +"%Y-%m-%dT%T.%6N"%z)"|$@">>$LOGFILE ; echo -e $GREEN"$@"$NORMAL ; }
log() { echo $(date  +"%Y-%m-%dT%T.%6N"%z)"|$@">>$LOGFILE ; }
msg() { echo $(date  +"%Y-%m-%dT%T.%6N"%z)"|$@">>$LOGFILE ; echo -e $NORMAL"$@" ; }
note() { echo $(date  +"%Y-%m-%dT%T.%6N"%z)"|$@">>$LOGFILE ; echo -e $YELLOW"$@"$NORMAL ; }
checkmodule () {
  log cd \$CTXUSR/bin
  cd $CTXUSR/bin
  log crm -m $1
  printf "%-8s %-8s " $1 $2
  crmoutput=$(crm -m $1 2>&1)
  log "$crmoutput"
  notfound=$($crmoutpu | grep "Could not find archive file for module")
  if [[ -n $notfound ]]; then
    checkmessage="not installed"
  else
    installed=$(crm -m $1 | grep $3)
    if [[ -n $installed ]]; then
      checkmessage="installed"
    else
      checkmessage="missing"
    fi
  fi
  printf "%s\\n" "$checkmessage"
}
checkcontent () {
  if ! [[ -f "$1/$2" ]]; then
    printf "%-35s %s\\n" $2 "was not found"
    msg "Aborting installation"
    return 1
  fi
  return 0
}
backupmodule () {
  log cd \$CTXUSR/relarc
  cd $CTXUSR/relarc
  modulefile=$2
  if [[ -f "$modulefile" ]]; then
    exe mv $modulefile $1 2>/dev/null
    if ! [[ $? -eq 0 ]]; then
      echo -e Can not move $modulefile
      ((backuperrorcounter+=1))
    else
      echo -e Backing up $modulefile
      ((backupcounter+=1))
    fi
  else
    if [[ "$3" -eq "1" ]]; then
      echo -e "Can not find $modulefile"
    fi
  fi
}
clear
log "Cortex Installation `date +%Y%m%d%H%M%S`"
# ${client} Installation  shell
title "Checking operation parameters"
if [ -z <#noparse>"${CTXUSR}"</#noparse> ]; then
  msg "Variable \$CTXUSR is unset or set to empty string"
  exit 1
fi
if ! [ -d "$CTXUSR/relarc" ]; then
  msg "Directory \$CTXUSR/relarc does not exist"
  exit 1
fi
# Moving LOGFILE to relarc
cat $LOGFILE>>$CTXUSR/relarc/cortex.install.log
rm -rf $LOGFILE
LOGFILE=$CTXUSR/relarc/cortex.install.log
if ! [ -d "$CTXUSR/bin" ]; then
  msg "Directory \$CTXUSR/bin does not exist"
  exit 1
fi
if ! [ -d "$CTXUSR/tuxfbuf" ]; then
  msg "Directory \$CTXUSR/tuxfbuf does not exist"
  exit 1
fi
if [ -z <#noparse>"${CTXTMP}"</#noparse> ]; then
  msg "Variable \$CTXTMP is unset or set to empty string"
fi
if ! [ -d "$CTXTMP" ]; then
  msg "Directory \$CTXTMP does not exist"
  exit 1
fi
exe cd $CTXUSR
log perl -e 'do "./opt/crm"; print &getos()'
OS=$(perl -e 'do "./opt/crm"; print &getos()')
if ! [[ $? -eq 0 ]]; then
  msg "Can not resolve OS"
  exit
fi
exe cd $CTXUSR
log perl -e 'do "./opt/crm"; print &getosver()'
OSVER=$(perl -e 'do "./opt/crm"; print &getosver()')
if ! [[ $? -eq 0 ]]; then
  msg "Can not resolve OSVER"
  exit
fi
if [ "$OS" = "UNSUPPORTED" ]; then
  msg "OS not supported" 
  exit 1
fi
if [ "$OSVER" = "UNSUPPORTED" ]; then
  msg "OSVER not supported" 
  exit 1
fi
msg "System : $OS-$OSVER"
msg "Parameters have been verified"
if [ $# -ge 1 ] && [ "$1" = "-a" ]; then
title "Activate installed packages"
exe cd $CTXUSR/relarc
<#list modules as module>
<#switch module.type>
<#case "bin">
<#case "tuxfbuf">
printf "%-8s %-7s %-8s " ${module.name} ${module.type} ${module.install}
log crm -a ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz
crmoutput=$(crm -a ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz 2>&1)
log "$crmoutput"
if ! [[ $? -eq 0 ]]; then
  printf "%s\\n" "has not been activated"
  exit 1
else
  printf "%s\\n" "has been activated"
fi
<#break>
</#switch>
</#list>
title "Checking activated versions"
<#list modules as module>
<#if module.type == "bin">
checkmodule ${module.name} ${module.install} ${module.name}-${module.install}<#if client != module.name>-${client}</#if>
</#if>
</#list>
note "\nInstallation process has been completed"
exit 0
fi
#checking
title "Checking current versions"
<#list modules as module>
<#if module.type == "bin">
checkmodule ${module.name} ${module.version} ${module.name}-${module.version}<#if client != module.name>-${client}</#if>
</#if>
</#list>
title "Preparing packages"
log cd \$CTXUSR
cd $CTXUSR
if ! [ -d "$CTXUSR/${installation}" ]; then
  exe mkdir ${installation} 2>/dev/null  
fi
msg "Extracting package ${package} -> \$CTXUSR/${installation}"
<#noparse>TARFILE=$(awk '/^__TARFILE__/ {print NR + 1; exit 0; }' "${0}")
tail -n+${TARFILE} "${0}"</#noparse> | tar xJp -C $CTXUSR/${installation}
#extract package
if [[ $? -eq 0 ]]; then
  msg "Package ${package} has been extracted"
else
  msg "Package ${package} extraction has failed"
  exit
fi
msg "Installing from \$CTXUSR/${installation}"
msg "Backup to \$CTXUSR/${installation}/$backupdir"
exe mkdir ${installation}/$backupdir 2>/dev/null
#verify content
<#list modules as module>
checkcontent $CTXUSR/${installation} ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz
if ! [[ $? -eq 0 ]]; then
  exit 1
fi
</#list>
msg "Package has been verified"
title "Backing up duplicated packages"
backupcounter=0
backuperrorcounter=0
<#list modules as module>
<#switch module.type>
  <#case "bin">
  <#case "tuxbuf">
  backupmodule $CTXUSR/${installation}/$backupdir ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz <#if module.version == module.install>1<#else>0</#if>
  <#break>
</#switch>
</#list>
if [ "$backuperrorcounter" -eq "0" ] && [ "$backupcounter" -eq "0" ]; then
  msg "There is nothing to backup"
fi
# PERFORM BACKUP TASKS
title "Perform backup tasks"
backuptaskcounter=0
<#list backup as backupitem>
backuptaskpackage=${backupitem.package}
cd ${backupitem.path}>/dev/null 2>&1; ls ${backupitem.content} > /dev/null 2>&1
if [[ $? -eq 0 ]]; then
  cd $CTXUSR/${installation}/$backupdir
  (cd ${backupitem.path}>/dev/null 2>&1; tar cf - ${backupitem.content} ) | gzip >$backuptaskpackage
  backuptaskcounter+=1
  msg "$backuptaskpackage has been created"
else
  msg "$backuptaskpackage : ${backupitem.content} has failed"
  exit 1
fi
</#list>
if [ "$backuptaskcounter" -eq "0" ]; then
  msg "There are no backup tasks"
else
  msg "Backup tasks have been completed"
fi
#install
title "Install packages"
exe cd $CTXUSR/${installation}
<#list modules as module>
<#switch module.type>
<#case "bin">
<#case "tuxfbuf">
exe mv ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz $CTXUSR/relarc 2>/dev/null
<#break>
</#switch>
</#list>
exe cd $CTXUSR/relarc
<#list modules as module>
<#switch module.type>
<#case "bin">
<#case "tuxfbuf">
log crm -x ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz
printf "%-8s %-7s %-8s " ${module.name} ${module.type} ${module.install}
crmoutput=$(crm -x ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz 2>&1)
log "$crmoutput"
if ! [[ $? -eq 0 ]]; then
  printf "%s\\n" "has failed"
  exit 1
else
  printf "%s\\n" "has been installed"
fi
<#break>
</#switch>
</#list>
msg "Packages have been successfully installed"
title "Unpackaging database scripts"
exe cd $CTXUSR/${installation}
scriptcounter=0
<#list modules as module>
<#switch module.type>
<#case "db">
exe tar xf ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz > /dev/null 2>&1
if ! [[ $? -eq 0 ]]; then
  printf "%-8s %-7s %-8s %s\\n" ${module.name} ${module.type} ${module.install} "has failed"
  exit 1
else
  scriptcounter+=1
  printf "%-8s %-7s %-8s %s\\n" ${module.name} ${module.type} ${module.install} "has been unpacked"
  tar tzf ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz | awk -F/ '{ if($NF != "") print $NF }' | sort
fi
<#break>
</#switch>
</#list>
if ! [ "$scriptcounter" -eq "0" ]; then
  note "\nGo to \$CTXUSR/${installation} and Contact database administrator\nor run db command to execute scripts."
fi
title "Updating envinronment"
exe chmod 0755 $CTXUSR/bin/*.sh > /dev/null 2>&1
exe chmod 0755 $CTXUSR/tuxfbuf/*.fd > /dev/null 2>&1
msg "Permissions have been successfully updated"
note "\nTo continue with the installation process"
note "  1. Stop cortex execution"
note "  2. Run installation script with argument '-a' to activate packages"
exit 0
__TARFILE__