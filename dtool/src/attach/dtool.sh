#! /bin/sh

OS=`uname`
export OS

# Setup the initial path
if [ $OS = "Linux" ]; then
  PATH=/var/local/bin:~/bin:.:/usr/sbin:/sbin:/usr/bin:/bin:/usr/bin/X11:/usr/etc:/usr/local/bin
elif [ $OS = "IRIX64" ]; then
  PATH=/var/local/bin:/usr/local/bin/ptools:~/bin:/usr/local/prman/bin:.:/usr/sbin:/usr/bsd:/sbin:/usr/bin:/bin:/usr/bin/X11:/usr/etc:/usr/demos/bin:/usr/local/bin
elif [ $OS = "CYGWIN_98-4.10" ]; then
  PATH=/usr/local/bin:/bin:/CYGNUS/CYGWIN~1/H-I586~1/BIN:/WINDOWS:/WINDOWS:/WINDOWS/COMMAND:/DMI/BIN:/KATANA/UTL/DEV/MAKE:/KATANA/UTL/DEV/HITACHI
else
  PATH=/var/local/bin:/usr/local/bin/ptools:~/bin:/usr/local/prman/bin:.:/usr/sbin:/usr/bsd:/sbin:/usr/bin:/bin:/usr/bin/X11:/usr/etc:/usr/demos/bin:/usr/local/bin
fi

# Setup the initial manpath
#if [ $OS = "Linux" ]; then
#  MANPATH=/usr/local/man:/usr/man/preformat:/usr/man:/usr/X11R6/man
#elif [ $OS = "IRIX64" ]; then
#  MANPATH=/usr/share/catman:/usr/catman:/usr/local/share/catman:/usr/local/share/man:/usr/local/man
#elif [ $OS = "CYGWIN_98-4.10" ]; then
#  MANPATH=/usr/local/man
#else
#  MANPATH=/usr/share/catman:/usr/catman:/usr/local/share/catman:/usr/local/share/man:/usr/local/man
#fi
#export MANPATH

LD_LIBRARY_PATH="."
export LD_LIBRARY_PATH
DYLD_LIBRARY_PATH="."
export DYLD_LIBRARY_PATH
CT_INCLUDE_PATH="."
export CT_INCLUDE_PATH
#cdpath=.
#CDPATH="."
#export CDPATH
DC_PATH="."
export DC_PATH
SSPATH="."
export SSPATH
STKPATH="."
export STKPATH
SHELL_TYPE="sh"
export SHELL_TYPE

if [ -e /usr/atria ]; then
  if /usr/atria/bin/cleartool mount -all > /dev/null 2>&1; then
    HAVE_ATRIA=yes
    export HAVE_ATRIA
  fi
fi

if [ -z "$CTDEFAULT_FLAV" ]; then
  CTDEFAULT_FLAV="default"
  export CTDEFAULT_FLAV
fi

if [ -z "$DTOOL" ]; then
  DTOOL=/beta/player/bootstrap/tool
  export DTOOL
fi

if [ -z "$PENV" ]; then
  if [ $OS = "Linux" ]; then
    PENV="Linux"
  elif [ $OS = "IRIX64" ]; then
    PENV="SGI"
  elif [ $OS = "CYGWIN_98-4.10" ]; then
    PENV="WIN32_DREAMCAST"
  else
    PENV="SGI"
  fi
fi
export PENV

if [ -z "$1" ]; then
  source `$DTOOL/built/bin/ctattach.drv dtool default`
else
  source `$DTOOL/built/bin/ctattach.drv dtool $1`
fi
