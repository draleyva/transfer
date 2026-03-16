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
clear
echo -e "Cortex Installation `date +%Y%m%d%H%M%S`">>$CTXUSR/relarc/cortex.install.log
exe() { echo "\$ $@">>$CTXUSR/relarc/cortex.install.log ; "$@" ; }
backupdir=backup`date +%Y%m%d%H%M%S`
checkmodule () {
  cd $CTXUSR/bin
  notfound=$(crm -m $1 2>&1 >/dev/null| grep "Could not find archive file for module")
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
  printf "%-8s %-8s %s\\n" $1 $2 "$checkmessage"
}
checkcontent () {
  if ! [[ -f "$1/$2" ]]; then
    printf "%-35s %s\\n" $2 "was not found"
    echo -e "Aborting installation"
    return 1
  fi
  return 0
}
backupmodule () {
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
# ${client} Installation  shell
echo -e $GREEN"Checking operation parameters"$NORMAL
if [ -z <#noparse>"${CTXUSR}"</#noparse> ]; then
  echo -e "Variable \$CTXUSR is unset or set to empty string"
fi
if ! [ -d "$CTXUSR/bin" ]; then
  echo -e "Directory \$CTXUSR/bin does not exist"
  exit 1
fi
if ! [ -d "$CTXUSR/tuxfbuf" ]; then
  echo -e "Directory \$CTXUSR/tuxfbuf does not exist"
  exit 1
fi
if ! [ -d "$CTXUSR/relarc" ]; then
  echo -e "Directory \$CTXUSR/relarc does not exist"
  exit 1
fi
if [ -z <#noparse>"${CTXTMP}"</#noparse> ]; then
  echo -e "Variable \$CTXTMP is unset or set to empty string"
fi
if ! [ -d "$CTXTMP" ]; then
  echo -e "Directory \$CTXTMP does not exist"
  exit 1
fi
echo -e $NORMAL"Parameters have been verified"
if [ $# -ge 1 ] && [ "$1" = "-a" ]; then
echo -e $GREEN"Activate installed packages"$NORMAL
cd $CTXUSR/relarc
<#list modules as module>
<#switch module.type>
<#case "bin">
<#case "tuxfbuf">
exe crm -a ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz > /dev/null 2>&1
if ! [[ $? -eq 0 ]]; then
  printf "%-8s %-7s %-8s %s\\n" ${module.name} ${module.type} ${module.install} "has not been activated"
  exit 1
else
  printf "%-8s %-7s %-8s %s\\n" ${module.name} ${module.type} ${module.install} "has been activated"
fi
<#break>
</#switch>
</#list>
echo -e $GREEN"Checking activated versions"$NORMAL
<#list modules as module>
<#if module.type == "bin">
checkmodule ${module.name} ${module.install} ${module.name}-${module.install}<#if client != module.name>-${client}</#if>
</#if>
</#list>
echo -e $YELLOW"\nInstallation process has been completed"$NORMAL
exit 0
fi
#checking
echo -e $GREEN"Checking current versions"$NORMAL
<#list modules as module>
<#if module.type == "bin">
checkmodule ${module.name} ${module.version} ${module.name}-${module.version}<#if client != module.name>-${client}</#if>
</#if>
</#list>
echo -e $GREEN"Preparing packages"$NORMAL
cd $CTXUSR
if ! [ -d "$CTXUSR/${installation}" ]; then  
  mkdir ${installation} 2>/dev/null  
fi
<#noparse>ARCHIVE=$(awk '/^__ARCHIVE__/ {print NR + 1; exit 0; }' "${0}")
tail -n+${ARCHIVE} "${0}"</#noparse> | tar xpJ -C $CTXUSR/${installation}
if [[ -f "$CTXUSR/${package}" ]]; then
  moveextractpackage=1
  echo -e "Package ${package} has been found"
elif [[ -f "$CTXUSR/${installation}/${package}" ]]; then
  echo -e "Package ${package} is ready"
else
  echo -e "Installation package \$CTXUSR/${package} was not found"
  exit 1
fi
echo -e "Installing from \$CTXUSR/${installation}"
echo -e "Backup to \$CTXUSR/${installation}/$backupdir"
mkdir ${installation}/$backupdir 2>/dev/null
cd $CTXUSR
if [ "$moveextractpackage" -eq "1" ]; then
  exe mv ${package} ${installation}/ 2>/dev/null
  if ! [[ $? -eq 0 ]]; then
    echo -e Can not move ${package}
    exit 1  
  fi
fi
#extract package
if ! [[ -f "$CTXUSR/${installation}/${package}" ]]; then
  echo -e "Missing ${package} package"
  exit 1
fi
cd $CTXUSR/${installation}
exe tar xzf ${package}
if [[ $? -eq 0 ]]; then
  echo -e "Package ${package} has been extracted"
else
  echo -e "Package ${package} extraction has failed"
  exit
fi
#verify content
<#list modules as module>
checkcontent $CTXUSR/${installation} ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz
if ! [[ $? -eq 0 ]]; then
  exit 1
fi
</#list>
echo -e "Package has been verified"
echo -e $GREEN"Backing up duplicated packages"$NORMAL
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
  echo -e "There is nothing to backup"
fi
# PERFORM BACKUP TASKS
echo -e $GREEN"Perform backup tasks"$NORMAL
backuptaskcounter=0
<#list backup as backupitem>
backuptaskpackage=${backupitem.package}
cd ${backupitem.path}>/dev/null 2>&1; ls ${backupitem.content} > /dev/null 2>&1
if [[ $? -eq 0 ]]; then
  cd $CTXUSR/${installation}/$backupdir
  (cd ${backupitem.path}>/dev/null 2>&1; tar cf - ${backupitem.content} ) | gzip >$backuptaskpackage
  backuptaskcounter+=1
  echo -e "$backuptaskpackage has been created"
else
  echo -e "$backuptaskpackage : ${backupitem.content} has failed"
  exit 1
fi
</#list>
if [ "$backuptaskcounter" -eq "0" ]; then
  echo -e "There are no backup tasks"
else
  echo -e "Backup tasks have been completed"
fi
#install
echo -e $GREEN"Install packages"$NORMAL
cd $CTXUSR/${installation}
<#list modules as module>
<#switch module.type>
<#case "bin">
<#case "tuxfbuf">
exe mv ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz $CTXUSR/relarc 2>/dev/null
<#break>
</#switch>
</#list>
cd $CTXUSR/relarc
<#list modules as module>
<#switch module.type>
<#case "bin">
<#case "tuxfbuf">
exe crm -x ${module.name}-${module.install}-${client}-${moduletype(module.type)}.tgz > /dev/null 2>&1
if ! [[ $? -eq 0 ]]; then
  printf "%-8s %-7s %-8s %s\\n" ${module.name} ${module.type} ${module.install} "has failed"
  exit 1
else
  printf "%-8s %-7s %-8s %s\\n" ${module.name} ${module.type} ${module.install} "has been installed"
fi
<#break>
</#switch>
</#list>
echo -e "Packages have been successfully installed"
echo -e $GREEN"Unpackaging database scripts"$NORMAL
cd $CTXUSR/${installation}
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
  echo -e $YELLOW"\nGo to \$CTXUSR/${installation} and run db command to execute scripts"$NORMAL
fi
echo -e $GREEN"Updating envinronment"$NORMAL
chmod 755 $CTXUSR/bin/*.sh > /dev/null 2>&1
chmod 755 $CTXUSR/tuxfbuf/*.fd > /dev/null 2>&1
echo -e "Permissions have been successfully updated"
echo -e $YELLOW"\nTo continue with the installation process"$NORMAL
echo -e $YELLOW"  1. Stop cortex execution"$NORMAL
echo -e $YELLOW"  2. Run installation script '$0 -a' to activate packages"$NORMAL
exit 0

__ARCHIVE__
