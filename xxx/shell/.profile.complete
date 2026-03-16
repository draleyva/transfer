# sh setup file / BoKS
#
if [ -n "${BOKSPATH:-""}" ]; then
        if [ -n "$PATH" -a "$PATH" != "$BOKSPATH" ]; then
                PATH=$BOKSPATH:$PATH
        else
                PATH=$BOKSPATH
        fi
fi
if [ -n "${BOKS_UMASK:-""}" ]; then
	umask $BOKS_UMASK
fi
unset BOKSPATH BOKS_UMASK

test -n "$TERM" || eval `tset -s -Q -m ':?'`

ENV=$HOME/.kshrc; export ENV	# To set up korn shell (ksh)

# Add personal setup here.

echo "Printing path"
echo $PATH

PATH=/ctxtools/cortex/bin-4.2:.:$PATH:/opt/bin:/usr/local/bin:/usr/bin:/usr/local/perforce/bin:/etc:/usr/sbin:/usr/ucb:$HOME/bin:/usr/bin/X11:/sbin:/bin
export PATH

echo "Printing new path"
echo $PATH

ctxhost=`ctxhostname`

. choosesys -a

# Misc. environment variables
export QA=/cortex/qa
export BACKINGTREE=$CTXUSR
export PDIR=/usr/local/perforce/bin

. /home/e5706717/.sete5706717

# Aliases
alias glim2='/opt/bin/glim'
alias make2='make 2>&1'
alias make3='make2 | grep -v'
alias eptest='/cortex/build/product/mainline_tux81_ifx94/base/ep/bin/test/eptest'
alias sp='. /home/e5706717/.profile'
alias setwork='export WORKINGTREE=$PWD'

tree_end=`echo $BACKINGTREE | sed -e 's!.*/!!'`
maint=`echo $BACKINGTREE | sed -e 's!.*maint.*!maint!'`
if [ "$maint" = "maint" ]; then
	export WORKINGTREE=/home/e5706717/cortex/maint/latam/$tree_end ;
else
	export WORKINGTREE=/home/e5706717/cortex/project/latam/$tree_end ;
fi

# Set up the CDPATH so it looks in $HOME first
export CDPATH=.:$HOME/cortex:$HOME/cortex/src:$HOME/cortex/src/htdocs:$CTXUSR:$CTXUSR/src:$CTXUSR/src/accell:$HOME:$CTXUSR/src/bgif:${WORKINGTREE}/..

spolddir=$PWD
: ${OLDPWD:=$PWD}
spoldolddir="$OLDPWD"
if [ -e "$WORKINGTREE" ]; then
	cd $WORKINGTREE
fi
cd $spolddir
export OLDPWD=$spoldolddir

export BASE=${WORKINGTREE}/base
export SRC=${WORKINGTREE}/src

# java5
#export JAVA_HOME=/usr/java5_64/jre
#export PATH=$PATH:/usr/java5_64/jre/bin

export JAVA_HOME=/usr/java6_64/jre
export PATH=$PATH:/usr/java6_64/bin

export P4USER=rtorres
export P4TICKETS=$HOME/.p4/p4tickets

# BOKS: Please don't remove these lines
xxstart=${BOKS_START_PRG:-""}
unset BOKS_START_PRG
if [ -n "$xxstart" ]; then
        eval "exec $xxstart"
fi                            

