set prompt='%m:%B%~%b> '
alias ls 'ls -C -F'
alias dir ls -C -F -l
alias vi vim
alias fd ftp dumbo
alias tz tar zxvf
alias so1 setenv PANDA_OPTIMIZE 1
alias so3 setenv PANDA_OPTIMIZE 3
alias cpd  'cd $PANDA/src'
alias emacs xemacs
alias em xemacs
alias mi make install
alias ni nmake install
alias dh demo /install/models/herc-6000.egg /install/models/HB_1_HE1.egg
set nobeep=1

# my defaults for panda build
if (! $?PANDA_OPTIMIZE ) then
setenv PANDA_OPTIMIZE 1
endif

setenv CL_MAKE_BROWSE_INFO 0
alias setbrowseinfo1 setenv CL_MAKE_BROWSE_INFO 1
alias setbrowseinfo0 setenv CL_MAKE_BROWSE_INFO 0

setenv HAVE_GL yes
setenv HAVE_DX yes
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

setenv CVSROOT :pserver:georges@dimbo:/fit/cvs
#setenv CVS_PASSFILE ~/.cvspass

# check out files read-only
setenv CVSREAD 1

if (! $?PPREMAKE_CONFIG) then
setenv PPREMAKE_CONFIG /usr/local/etc/Config.pp
endif

# /usr/local/panda/bin for ppremake
if(! $?TCSH_NO_CHANGEPATH) then
set path=(/bin /contrib/bin $path ~/scripts $path /usr/local/panda/bin)
endif


if (! $?TCSH_NO_PANDA_ATTACH ) then
# setenv TOOL /install/tool
setenv MYTOOL ~/player/dtool
setenv MYTOOL_CSHRC $MYTOOL/etc/dtool.cshrc
#echo "   checking for dtool.cshrc"

if(-e $MYTOOL_CSHRC) then
setenv DTOOL $MYTOOL
#setenv CTATTACH_DEBUG 1
#echo "  found my dtool.cshrc"
source $DTOOL/etc/dtool.cshrc personal

# else
# echo "WARNING: $TOOL_CSHRC doesnt exist, panda env setup failed!"
# cd /install
else

setenv DTOOL /g/player/install/win2k/dtool
source $DTOOL/etc/dtool.cshrc install
endif

cta panda personal

if(! $?TCSH_NO_CSHRC_CHDIR ) then
cd $PANDA
endif

endif

# this version of emacs ignores cygwin root, cant give it cygwin paths
#set path=(/cygdrive/c/progra~1/XEmacs/XEmacs-21.1.9/i386-pc-win32 /bin /contrib/bin $path)
#set path=( /bin /contrib/bin /cygdrive/c/python16 $path )
#set path=( /bin /contrib/bin $path ~/scripts /usr/sbin /contrib/sbin)  for new cygwin

#set path=($path /cygdrive/c/progra~1/micros~1/common/msdev98/bin /cygdrive/c/progra~1/micros~1/common/tools/winnt /cygdrive/c/progra~1/micros~1/common/tools /cygdrive/c/progra~1/micros~1/vc98/bin /cygdrive/c/winnt /cygdrive/c/winnt/system32 /cygdrive/c/bin)
#set lib = (/cygdrive/c/progra~1/micros~1/vc98/lib /cygdrive/c/progra~1/micros~1/vc98/mfc/lib)
# note:  have to set vc envvars in sys env vars
#cd $HOME
unset owd

alias mktool 'cd $DTOOL/src/build;./initialize make;cd $DTOOL;make install'
#alias cpdll 'cp -p $PANDA/src/all/pandadx/*dll $PANDA/src/all/panda/*dll $PANDA/src/all/pandaegg/*dll $PANDA/lib;cp -p $PANDA/src/all/pandadx/lib*pdb $PANDA/src/all/panda/lib*pdb $PANDA/src/all/pandaegg/lib*pdb $PANDA/lib'
alias mkdx  'cd dxgsg; make install; cd ../wdxdisplay; make install; cd ../../metalibs/pandadx; make install; cd wdxdisplay'
alias mkgl  'cd glgsg; make install; cd ../wgldisplay; make install; cd ../../metalibs/pandagl; make install; cd wgldisplay'
alias cpdxbak 'chmod a+w $PANDA/dx-bak/*;cp $PANDA/src/dxgsg/*.? $PANDA/src/wdxdisplay/*.? $PANDA/dx-bak'
alias tlinit 'cd $DTOOL/src/build; ./initialize make; cd $DTOOL'
alias setctdbg 'setenv CTATTACH_DEBUG 1'
alias update cvs update
alias cvhist cvs history -a
alias log cvs log
alias updn 'cvs -n update -R -d \!* |& egrep -v "Updating|^\?"'
alias cvdiff cvs diff
alias cvlog ~/scripts/cvs2cl.pl
alias updr 'cvs update -R -d \!* |& egrep -v "Updating|^\?"'
alias upd  'cvs update -d \!* |& egrep -v "Updating|^\?"'
alias lastchng ~/scripts/lastchng.pl
alias wnewdiff 'cvs update -p -r HEAD  \!* > junk; windiff junk \!* &'
#only works if cur ver differs from checked in version
#alias newdiff 'cvs diff -r`log -h \!* | grep head | sed -e "s/head: //"` \!*'
# turns out all you need is this
alias newdiff 'cvs diff -r HEAD -w \!*'
# need version that picks out last 2 revs and does the diff
alias cvsclean "find \!* -path '*CVS*' -prune -or -not -name 'bldlog*' -and -not -type d -print | xargs --no-run-if-empty -n 40 rm"

