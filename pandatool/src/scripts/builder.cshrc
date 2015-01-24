set prompt='%m:%B%~%b> '
alias ls 'ls -C -F'
alias dir ls -C -F -l
alias vi vim
alias fd ftp dumbo
alias tz tar zxvf
alias spo1 setenv PANDA_OPTIMIZE 1
alias spo2 setenv PANDA_OPTIMIZE 2
alias spo3 setenv PANDA_OPTIMIZE 3
alias spo4 setenv PANDA_OPTIMIZE 4
alias snoi setenv NO_INTERROGATE 1
alias usnoi unsetenv NO_INTERROGATE
alias set_intel setenv USE_COMPILER INTEL
alias set_msvc setenv USE_COMPILER MSVC

alias cpd  'cd $PANDA/src'
#alias emacs xemacs
#alias em xemacs
alias mi make install
alias ni nmake install
alias dh demo /install/models/herc-6000.egg /install/models/HB_1_HE1.egg
set nobeep=1

if(! $?TCSH_NO_CHANGEPATH) then
# have to manually put '\' in front of spaces for this to work
# this is completely reset at file end
set oldpath=($path)
set path=(./ /bin /usr/local/bin /mscommon/Tools/WinNT /mscommon/MSDev98/Bin /mscommon/Tools /msvc98/Bin /mspsdk/Bin /mspsdk/Bin/WinNT /c/WINNT/System32/Wbem /c/PROGRA~1/TCL/bin /c/Program\ Files/Resource\ Pro\ Kit/ /c/Program\ Files/Resource\ Kit/ /c/WINNT/system32 /c/WINNT /c/bin /c/xemacs/bin/i686-pc-cygwin /emacs-20.7/bin)
endif

# my defaults for panda build
if (! $?PANDA_OPTIMIZE ) then
setenv PANDA_OPTIMIZE 2
endif

setenv CL_MAKE_BROWSE_INFO 0
alias setbrowseinfo1 setenv CL_MAKE_BROWSE_INFO 1
alias setbrowseinfo0 setenv CL_MAKE_BROWSE_INFO 0

setenv HAVE_GL yes
setenv HAVE_DX9 yes
setenv USE_NSPR t

if (! $?USER ) then
setenv USER georges
endif

if (! $?CYGWIN_ROOT) then
# may cause tcsh hang
setenv CYGWIN_ROOT `cygpath -w /`
endif

setenv PENV_COMPILER WIN32_VC
setenv PENV WIN32
setenv PANDA_ROOT C:/Cygwin

setenv CVSROOT :pserver:cxgeorge@dimbo:/fit/cvs
#setenv CVS_PASSFILE ~/.cvspass

# check out files read-only
# setenv CVSREAD 1

if (! $?PANDA_BUILD_TYPE) then
setenv PANDA_BUILD_TYPE gmsvc
endif

if (! $?PPREMAKE_CONFIG) then
#setenv PPREMAKE_CONFIG /usr/local/etc/Config.pp
#WINTOOLS may not be set, so do this instead
setenv PPREMAKE_CONFIG $HOME/player/wintools/panda/etc/Config.pp
endif

# for ppremake
#set path=($path /usr/local/panda/bin)
# /usr/local/panda/bin for ppremake
if(! $?TCSH_NO_CHANGEPATH) then
# set path=(/bin /contrib/bin $path ~/scripts $path /usr/local/panda/bin)

# builder.pl doesnt want initial path changed, except to add these things
set path=(~/scripts $path)
endif



if (! $?TCSH_NO_PANDA_ATTACH ) then
# builder scripts don't attach (attach scripts too buggy on cygwin 1.0)
# they set paths manually
# setenv TOOL /install/tool
setenv MYTOOL ~/player/dtool
setenv MYTOOL_CSHRC $MYTOOL/src/attach/dtool.cshrc
#echo "   checking for dtool.cshrc"

if(-e $MYTOOL_CSHRC) then
setenv DTOOL $MYTOOL
#setenv CTATTACH_DEBUG 1
#echo "  found my dtool.cshrc"
source $DTOOL/etc/dtool.cshrc personal
cta wintools personal
cta panda personal
cta direct personal
cta toontown personal
cd ~/player/panda
# else
# echo "WARNING: $TOOL_CSHRC doesnt exist, panda env setup failed!"
# cd /install
else

setenv DTOOL /g/player/install/win2k/dtool
source $DTOOL/src/attach/dtool.cshrc install
endif

alias ctp "cta panda personal;cta wintools personal"
alias ctt "cta direct personal;cta toontown personal"
cta wintools personal  
cta panda personal
cta direct personal
cta toontown personal

if(! $?TCSH_NO_CSHRC_CHDIR ) then
cd $HOME/player/panda
endif

else
  if(! $?TCSH_NO_CHANGEPATH) then
   set path=($oldpath $path)
  endif
endif

# this version of emacs ignores cygwin root, cant give it cygwin paths
#set path=(/cygdrive/c/progra~1/XEmacs/XEmacs-21.1.9/i386-pc-win32 /bin /contrib/bin $path)
#set path=( /bin /contrib/bin /cygdrive/c/python16 $path )
#set path=( /bin /contrib/bin $path ~/scripts /usr/local/winpandatools/win98)
set path=( /bin /contrib/bin $path ~/scripts)
set path=(/intel/compiler60/ia32/bin /intel/iselect/bin /intel/edb $path)

#set path=($path /cygdrive/c/progra~1/micros~1/common/msdev98/bin /cygdrive/c/progra~1/micros~1/common/tools/winnt /cygdrive/c/progra~1/micros~1/common/tools /cygdrive/c/progra~1/micros~1/vc98/bin /cygdrive/c/winnt /cygdrive/c/winnt/system32 /cygdrive/c/bin)
#set lib = (/cygdrive/c/progra~1/micros~1/vc98/lib /cygdrive/c/progra~1/micros~1/vc98/mfc/lib)
# note:  have to set vc envvars in sys env vars
#cd $HOME
unset owd


alias md mkdir
alias pp ppremake
alias ppr ppremake
alias wlastchng ~/scripts/wlastchng.pl
alias ni nmake install
alias listincs-all '(grep -h include `(echo *cxx | sed -e "s/test_[^ ]*//g")` | sort | uniq ) | (grep -v \");(echo ""); (grep -h include `(echo *cxx | sed -e "s/test_[^ ]*//g")` | sort | uniq ) | (grep \");'
alias listincs-rep '(grep -h include `(echo *cxx | sed -e "s/test_[^ ]*//g")` | sort | uniq -d) | (grep -v \");(echo ""); (grep -h include `(echo *cxx | sed -e "s/test_[^ ]*//g")` | sort | uniq -d) | (grep \");'
alias listincs-sing '(grep include `(echo *cxx | sed -e "s/test_[^ \t]*//g")` | /bin/sort +1 | uniq -u -f 1 | /bin/sort);'
alias listincs-old '(grep -h include *cxx | sort | uniq ) | (grep -v \");echo ""; (grep -h include *cxx | sort | uniq ) | (grep \");'
alias listincs '(grep -h include *cxx | sort | uniq ) | (grep -v \"); (grep -h include *cxx | sort | uniq ) | (grep \");'
alias prproc 'echo $NUMBER_OF_PROCESSORS'
alias setproc1 setenv NUMBER_OF_PROCESSORS 1
alias setproc2 setenv NUMBER_OF_PROCESSORS 2
alias setproc4 setenv NUMBER_OF_PROCESSORS 4
alias setgenass setenv GEN_ASSEMBLY 1
alias usetgenass unsetenv GEN_ASSEMBLY
alias mroe more

alias ctp "cta panda personal;cta wintools personal"
alias mktool 'cd $DTOOL/src/build;./initialize make;cd $DTOOL;make install'
#alias cpdll 'cp -p $PANDA/src/all/pandadx/*dll $PANDA/src/all/panda/*dll $PANDA/src/all/pandaegg/*dll $PANDA/lib;cp -p $PANDA/src/all/pandadx/lib*pdb $PANDA/src/all/panda/lib*pdb $PANDA/src/all/pandaegg/lib*pdb $PANDA/lib'
alias mkdx  'cd ~/player/panda/src/dxgsg; make install; cd ../wdxdisplay; make install; cd ../../metalibs/pandadx; make install; cd ~/player/panda/src/dxgsg'
alias mkdemo 'cd ~/player/panda/src/framework; make install; cd ../testbed; rm Opt*/demo.exe; make install; cd ~/player/panda/src/framework'
alias mkpp  'cd ~/player/panda/src/physics; make install; cd ../particlesystem; make install; cd ../../metalibs/pandaphysics; make install; cd ~/player/panda/src/dxgsg'
# alias mkdemo 'cd ~/player/panda/src/framework; make install; cd ../testbed; make clean; make install; cd ~/player/panda/src/framework'
alias mkdemo 'cd ~/player/panda/src/framework; make install; cd ../testbed; rm Opt*/*.exe; make install; cd ~/player/panda/src/framework'
alias mkgl  'cd ~/player/panda/src/glgsg; make install; cd ../wgldisplay; make install; cd ../../metalibs/pandagl; make install; cd ../../src/panda/wgldisplay'
alias mkegg  'cd ~/player/panda/src/egg; make install; cd ../builder; make install; cd ../egg2sg; make install; cd ../../metalibs/pandaegg; make install; cd ~/player/panda/src/dxgsg'
alias cpdxbak 'chmod a+w $PANDA/dx-bak/*;cp $PANDA/src/dxgsg/*.? $PANDA/src/wdxdisplay/*.? $PANDA/dx-bak'
alias tlinit 'cd $DTOOL/src/build; ./initialize make; cd $DTOOL'
alias setctdbg 'setenv CTATTACH_DEBUG 1'
alias update cvs update
alias cvhist cvs history -a
alias log cvs log
alias updn 'cvs -n update -R -d \!* |& egrep -v "Updating|^\?"'
alias cvdiff cvs diff
alias cvlog ~/scripts/cvlog.pl
alias updr 'cvs update -R -d \!* |& egrep -v "Updating|^\?"'
alias upd  'cvs update -d \!* |& egrep -v "Updating|^\?"'
alias chm 'chmod a+w *'
alias lastchng ~/scripts/lastchng.pl
alias wnewdiff 'cvs update -p -r HEAD  \!* > junk ; windiff junk \!* &'
#only works if cur ver differs from checked in version
#alias newdiff 'cvs diff -r`log -h \!* | grep head | sed -e "s/head: //"` \!*'
# turns out all you need is this
alias newdiff 'cvs diff -r HEAD -w \!*'
# need version that picks out last 2 revs and does the diff
alias cvsclean "find \!* -path '*CVS*' -prune -or -not -name 'bldlog*' -and -not -type d -print | xargs --no-run-if-empty -n 40 rm"

alias genpy4 '~/player/direct/bin/genPyCode  win-publish'
alias genpy1 '~/player/direct/bin/genPyCode  win-debug'
alias genpy2 '~/player/direct/bin/genPyCode  win-debug'
alias genpy3 '~/player/direct/bin/genPyCode  win-release'
alias nd newdiff

# setenv LOCAL_PPREMAKE_CONFIG ~/player/MyConfig.pp

alias ppy1 'ppython -d `cygpath -w $DIRECT/bin/generatePythonCode` -v -d `cygpath -w $DIRECT/lib/py` -e `cygpath -w  $DIRECT/src/extensions` -i libdtoolconfig libpandaexpress libpanda libpandaphysics libdirect libtoontown'
setenv ZPAN /z/cygwin/home/builder/player/panda
setenv XPAN /x/cygwin/home/georges/player/panda
unalias emacs

alias setcfgdbg setenv CONFIG_CONFIG ':configdbg=1:configpath=CFG_PATH:configpath=ETC_PATH'
alias usetcfgdbg setenv CONFIG_CONFIG 'configpath=CFG_PATH:configpath=ETC_PATH'
alias s source
