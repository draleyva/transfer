#!/bin/sh
BLACK="\033[30m"
RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
PINK="\033[35m"
CYAN="\033[36m"
WHITE="\033[37m"
NORMAL="\033[0;39m"
# BRE Installation  shell
echo -e $GREEN"Checking operation parameters"$NORMAL
if ! [ -d "$CTXUSR/bin" ]; then
  echo "Directory $$CTXUSR/bin does not exist"
  exit 1
fi
if ! [ -d "$CTXUSR/tuxfbuf" ]; then
  echo "Directory $$CTXUSR/tuxfbuf does not exist"
  exit 1
fi
if ! [ -d "$CTXUSR/relarc" ]; then
  echo "Directory $$CTXUSR/relarc does not exist"
  exit 1
fi
echo -e $GREEN"Checking current versions"$NORMAL
cd $CTXUSR/bin
checkmodule () {
notfound=$(crm -m $1 2>&1 >/dev/null| grep "Could not find archive file for module")
if [[ -n $notfound ]]; then
  echo "$1 has not been installed"
else
  installed=$(crm -m $1 | grep $2)
  if [[ -n $installed ]]; then
    echo "$1 has been installed"
  else
    echo "$1 has been missing"
  fi
fi
}
checkmodule ACS ACS-6.2.14-BRE
checkmodule AD AD-6.2.16-BRE
checkmodule AU AU-6.2.82-BRE
checkmodule BRE BRE-2.0.9-BRE
checkmodule CMNFNC CMNFNC-6.2.24-BRE
checkmodule CO CO-6.2.86-BRE
checkmodule CRD CRD-6.2.55-BRE
checkmodule DFC DFC-6.2.35-BRE
checkmodule INTEG INTEG-6.2.32-BRE
checkmodule ISO ISO-6.2.57-BRE
checkmodule ISS ISS-6.2.37-BRE
checkmodule MC MC-6.2.71-BRE
checkmodule PAR PAR-6.2.13-BRE
echo -e $GREEN"Backing up current versions"$NORMAL
cd $CTXUSR/relarc
mkdir migrationOracle19c 2>/dev/null
mv ACS-6.2.14-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move ACS-6.2.14-BRE
fi
mv AD-6.2.16-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move AD-6.2.16-BRE
fi
mv AU-6.2.82-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move AU-6.2.82-BRE
fi
mv BRE-2.0.9-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move BRE-2.0.9-BRE
fi
mv CMNFNC-6.2.24-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move CMNFNC-6.2.24-BRE
fi
mv CO-6.2.86-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move CO-6.2.86-BRE
fi
mv CRD-6.2.55-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move CRD-6.2.55-BRE
fi
mv DFC-6.2.35-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move DFC-6.2.35-BRE
fi
mv INTEG-6.2.32-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move INTEG-6.2.32-BRE
fi
mv ISO-6.2.57-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move ISO-6.2.57-BRE
fi
mv ISS-6.2.37-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move ISS-6.2.37-BRE
fi
mv MC-6.2.71-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move MC-6.2.71-BRE
fi
mv PAR-6.2.13-BRE-bin.crm.tgz ./migrationOracle19c 2>/dev/null
if ! [ $? -eq 0 ]; then
  echo Can not move PAR-6.2.13-BRE
fi
