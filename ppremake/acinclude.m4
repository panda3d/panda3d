AC_DEFUN(AC_HEADER_IOSTREAM,
[AC_CHECK_HEADERS(iostream,[have_iostream=yes],[have_iostream=no])])

AC_DEFUN(AC_HEADER_SSTREAM,
[AC_CHECK_HEADERS(sstream,[have_sstream=yes],[have_sstream=no])])

AC_DEFUN(AC_IOS_BINARY,
[AC_CACHE_CHECK([for ios::binary],
  ac_cv_ios_binary,
[
if test $have_iostream = yes; then
  AC_TRY_COMPILE([
#include <iostream>
  ],[
  int x; x = std::ios::binary;
  ], ac_cv_ios_binary=yes, ac_cv_ios_binary=no)
else
  AC_TRY_COMPILE([
#include <iostream.h>
  ],[
  int x; x = ios::binary;
  ], ac_cv_ios_binary=yes, ac_cv_ios_binary=no)
fi

])
if test $ac_cv_ios_binary = yes; then
  AC_DEFINE(HAVE_IOS_BINARY, 1, [Define if the C++ iostream library supports ios::binary.])
fi
])

AC_DEFUN(AC_OPEN_MASK,
[AC_CACHE_CHECK([for third umask parameter to open],
  ac_cv_open_mask,
[
if test $have_iostream = yes; then
  AC_TRY_COMPILE([
#include <fstream>
  ],[
  std::ofstream x; x.open("foo", std::ios::out, 0666);
  ], ac_cv_open_mask=yes, ac_cv_open_mask=no)
else
  AC_TRY_COMPILE([
#include <fstream.h>
  ],[
  ofstream x; x.open("foo", ios::out, 0666);
  ], ac_cv_open_mask=yes, ac_cv_open_mask=no)
fi

])
if test $ac_cv_open_mask = yes; then
  AC_DEFINE(HAVE_OPEN_MASK, 1, [Define if fstream::open() accepts a third parameter for umask.])
fi
])


AC_DEFUN(AC_NAMESPACE,
[AC_CACHE_CHECK([for compiler namespace support],
  ac_cv_namespace,
[AC_TRY_COMPILE(
[namespace std { };
using namespace std;],
[],
  ac_cv_namespace=yes, ac_cv_namespace=no)])
if test $ac_cv_namespace = yes; then
  AC_DEFINE(HAVE_NAMESPACE, 1, [Define if the C++ compiler uses namespaces])
fi
])


dnl A handy function to see if a library is in a particular directory.
dnl AC_CHECK_LIB_LOC(directory, library, function, action-if-found, action-if-not-found, other-libraries)
dnl
AC_DEFUN(AC_CHECK_LIB_LOC,
[AC_MSG_CHECKING([for lib$2 in $1])
ac_lib_var=`echo $1['_']$2 | sed 'y%./+-%__p_%'`
AC_CACHE_VAL(ac_cv_lib_loc_$ac_lib_var,
[ac_save_LIBS="$LIBS"
LIBS="-L$1 -l$2 $6 $LIBS"
AC_TRY_LINK(dnl
ifelse(AC_LANG, CPLUSPLUS, [#ifdef __cplusplus
extern "C"
#endif
])dnl
[/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char $3();
],
            [$3()],
            eval "ac_cv_lib_loc_$ac_lib_var=yes",
            eval "ac_cv_lib_loc_$ac_lib_var=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test \"`echo '$ac_cv_lib_loc_'$ac_lib_var`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$4], ,
[LIBS="-L$1 -l$2 $LIBS"
], [$4])
else
  AC_MSG_RESULT(no)
ifelse([$5], , , [$5
])dnl
fi
])

dnl A handy function to search a number of possible locations for a library.
dnl AC_SEARCH_LIB(search-dirs, library, function, package, other-libraries)
dnl 
dnl Sets $package_LIB to the directory containing the library, or to the
dnl empty string if the library cannot be found.
AC_DEFUN(AC_SEARCH_LIB, [
ac_found_lib=""
for ac_check_dir in $1; do
  if test "$ac_found_lib" = ""; then
    AC_CHECK_LIB_LOC($ac_check_dir, $2, $3, [ ac_found_lib="$ac_check_dir"; ],, $5)
  fi
done
$4_LIB="$ac_found_lib"
])

dnl A handy function to see if a header file is in a particular directory.
dnl AC_CHECK_HEADER_LOC(directory, header, action-if-found, action-if-not-found)
dnl
AC_DEFUN(AC_CHECK_HEADER_LOC, [
  AC_MSG_CHECKING([for $2 in $1])
  ac_include_var=`echo $1['_']$2 | sed 'y%./+-%__p_%'`
  AC_CACHE_VAL(ac_cv_include_loc_$ac_include_var, [
    ac_save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="-I$1 $CPPFLAGS"
    AC_TRY_CPP([#include <$2>], ac_ch_found_it="yes", ac_ch_found_it="no")
    if test "$ac_ch_found_it" = "yes"; then
      AC_MSG_RESULT(yes)
      ifelse([$3], , :, [$3])
    else
      AC_MSG_RESULT(no)
      ifelse([$4], , , [$4])
    fi
    CPPFLAGS="$ac_save_CPPFLAGS"
  ])
])


dnl A handy function to search a number of possible locations for a header
dnl file.
dnl
dnl AC_SEARCH_HEADER(search-dirs, header, package)
dnl
dnl Sets $package_INCLUDE to the directory containing the header, or to
dnl the empty string if the header cannot be found.
AC_DEFUN(AC_SEARCH_HEADER, [
ac_found_header=""
for ac_check_dir in $1; do
  if test "$ac_found_header" = ""; then
    AC_CHECK_HEADER_LOC($ac_check_dir, $2, [ ac_found_header="$ac_check_dir";])
  fi
done
$3_INCLUDE="$ac_found_header"
])


dnl A handy function to scan for a third-party package, consisting of at
dnl least a library and an include file.  A few assumptions are made about
dnl the relationships between lib and include directories.
dnl
dnl AC_SEARCH_PACKAGE(search-dirs, package-names, header, library, function, package, other-libraries)
dnl
dnl search-dirs is the whitespace-separated list of directory prefixes to
dnl   check.
dnl package-names is a whitespace-separated list of possible names the
dnl   package may have been installed under.
dnl
dnl For each combination of ${search-dir} and ${package-name}, the following
dnl directories are searched:
dnl
dnl   ${search-dir}
dnl   ${search-dir}/lib
dnl   ${search-dir}/${package-name}
dnl   ${search-dir}/${package-name}/lib
dnl   ${search-dir}/lib/${package-name}
dnl
dnl And similarly for include.
dnl
dnl Sets the variables $package_INCLUDE and $package_LIB to the directories
dnl containing the header file and library, respectively.  If both pieces
dnl are located, also sets the variable $package_PKG to "yes"; otherwise,
dnl sets the variable $package_PKG to "no".
dnl
AC_DEFUN(AC_SEARCH_PACKAGE, [
$6_LIB=""
$6_INCLUDE=""
$6_PKG="no"

dnl Look for the library.
for ac_sp_dir in $1; do
  if test "[$]$6_LIB" = ""; then
    AC_SEARCH_LIB("$ac_sp_dir" "$ac_sp_dir/lib", $4, $5, $6, $7)
    for ac_sp_pkg in $2; do
      if test "[$]$6_LIB" = ""; then
        AC_SEARCH_LIB("$ac_sp_dir/$ac_sp_pkg" "$ac_sp_dir/$ac_sp_pkg/lib" \
                      "$ac_sp_dir/lib/$ac_sp_pkg", $4, $5, $6, $7)
      fi
    done
  fi
done

dnl Now look for the header file.  Don't bother looking if the library
dnl wasn't found.
if test "[$]$6_LIB" != ""; then
  dnl First look in the obvious directory corresponding to the lib dir.
  ac_sp_testinc=`echo [$]$6_LIB | sed 's:/lib:/include:'`
  AC_SEARCH_HEADER("$ac_sp_testinc", $3, $6)

  dnl If it wasn't found there, cast about.
  if test "[$]$6_INCLUDE" = ""; then
    for ac_sp_dir in $1; do
      if test "[$]$6_INCLUDE" = ""; then
        AC_SEARCH_HEADER("$ac_sp_dir" "$ac_sp_dir/include", $3, $6)
        for ac_sp_pkg in $2; do
          if test "[$]$6_INCLUDE" = ""; then
            AC_SEARCH_HEADER("$ac_sp_dir/$ac_sp_pkg" \
                             "$ac_sp_dir/$ac_sp_pkg/include" \
                             "$ac_sp_dir/include/$ac_sp_pkg", $3, $6)
          fi
        done
      fi
    done
  fi

  dnl If we got both a header and a library, set the PKG variable.
  if test "[$]$6_INCLUDE" != ""; then
    $6_PKG="yes"
  fi
fi
])  




# Configure paths for GTK--
# Erik Andersen	30 May 1998
# Modified by Tero Pulkkinen (added the compiler checks... I hope they work..)

dnl Test for GTKMM, and define GTKMM_CFLAGS and GTKMM_LIBS
dnl   to be used as follows:
dnl AM_PATH_GTKMM([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
AC_DEFUN(AM_PATH_GTKMM,
[dnl 
dnl Get the cflags and libraries from the gtkmm-config script
dnl
AC_ARG_WITH(gtkmm-prefix,[  --with-gtkmm-prefix=PREFIX
                          Prefix where GTK-- is installed (optional)],
            gtkmm_config_prefix="$withval", gtkmm_config_prefix="")
AC_ARG_WITH(gtkmm-exec-prefix,[  --with-gtkmm-exec-prefix=PREFIX
                          Exec prefix where GTK-- is installed (optional)],
            gtkmm_config_exec_prefix="$withval", gtkmm_config_exec_prefix="")
AC_ARG_ENABLE(gtkmmtest, [  --disable-gtkmmtest     Do not try to compile and run a test GTK-- program],
		    , enable_gtkmmtest=yes)

  if test x$gtkmm_config_exec_prefix != x ; then
     gtkmm_config_args="$gtkmm_config_args --exec-prefix=$gtkmm_config_exec_prefix"
     if test x${GTKMM_CONFIG+set} != xset ; then
        GTKMM_CONFIG=$gtkmm_config_exec_prefix/bin/gtkmm-config
     fi
  fi
  if test x$gtkmm_config_prefix != x ; then
     gtkmm_config_args="$gtkmm_config_args --prefix=$gtkmm_config_prefix"
     if test x${GTKMM_CONFIG+set} != xset ; then
        GTKMM_CONFIG=$gtkmm_config_prefix/bin/gtkmm-config
     fi
  fi

  AC_PATH_PROG(GTKMM_CONFIG, gtkmm-config, no)
  min_gtkmm_version=ifelse([$1], ,0.10.0,$1)

  AC_MSG_CHECKING(for GTK-- - version >= $min_gtkmm_version)
  no_gtkmm=""
  if test "$GTKMM_CONFIG" = "no" ; then
    no_gtkmm=yes
  else
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS

    GTKMM_CFLAGS=`$GTKMM_CONFIG $gtkmm_config_args --cflags`
    GTKMM_LIBS=`$GTKMM_CONFIG $gtkmm_config_args --libs`
    gtkmm_config_major_version=`$GTKMM_CONFIG $gtkmm_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gtkmm_config_minor_version=`$GTKMM_CONFIG $gtkmm_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gtkmm_config_micro_version=`$GTKMM_CONFIG $gtkmm_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gtkmmtest" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $GTKMM_CFLAGS"
      LIBS="$LIBS $GTKMM_LIBS"
dnl
dnl Now check if the installed GTK-- is sufficiently new. (Also sanity
dnl checks the results of gtkmm-config to some extent
dnl
      rm -f conf.gtkmmtest
      AC_TRY_RUN([
#include <gtk--.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gtkmmtest");

  /* HP/UX 0 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_gtkmm_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gtkmm_version");
     exit(1);
   }

  if ((gtkmm_major_version != $gtkmm_config_major_version) ||
      (gtkmm_minor_version != $gtkmm_config_minor_version) ||
      (gtkmm_micro_version != $gtkmm_config_micro_version))
    {
      printf("\n*** 'gtkmm-config --version' returned %d.%d.%d, but GTK-- (%d.%d.%d)\n", 
             $gtkmm_config_major_version, $gtkmm_config_minor_version, $gtkmm_config_micro_version,
             gtkmm_major_version, gtkmm_minor_version, gtkmm_micro_version);
      printf ("*** was found! If gtkmm-config was correct, then it is best\n");
      printf ("*** to remove the old version of GTK--. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If gtkmm-config was wrong, set the environment variable GTKMM_CONFIG\n");
      printf("*** to point to the correct copy of gtkmm-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
/* GTK-- does not have the GTKMM_*_VERSION constants */
/* 
  else if ((gtkmm_major_version != GTKMM_MAJOR_VERSION) ||
	   (gtkmm_minor_version != GTKMM_MINOR_VERSION) ||
           (gtkmm_micro_version != GTKMM_MICRO_VERSION))
    {
      printf("*** GTK-- header files (version %d.%d.%d) do not match\n",
	     GTKMM_MAJOR_VERSION, GTKMM_MINOR_VERSION, GTKMM_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     gtkmm_major_version, gtkmm_minor_version, gtkmm_micro_version);
    }
*/
  else
    {
      if ((gtkmm_major_version > major) ||
        ((gtkmm_major_version == major) && (gtkmm_minor_version > minor)) ||
        ((gtkmm_major_version == major) && (gtkmm_minor_version == minor) && (gtkmm_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GTK-- (%d.%d.%d) was found.\n",
               gtkmm_major_version, gtkmm_minor_version, gtkmm_micro_version);
        printf("*** You need a version of GTK-- newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** GTK-- is always available from ftp://ftp.gtk.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the gtkmm-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GTK--, but you can also set the GTKMM_CONFIG environment to point to the\n");
        printf("*** correct copy of gtkmm-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_gtkmm=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gtkmm" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GTKMM_CONFIG" = "no" ; then
       echo "*** The gtkmm-config script installed by GTK-- could not be found"
       echo "*** If GTK-- was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GTKMM_CONFIG environment variable to the"
       echo "*** full path to gtkmm-config."
       echo "*** The gtkmm-config script was not available in GTK-- versions"
       echo "*** prior to 0.9.12. Perhaps you need to update your installed"
       echo "*** version to 0.9.12 or later"
     else
       if test -f conf.gtkmmtest ; then
        :
       else
          echo "*** Could not run GTK-- test program, checking why..."
          CXXFLAGS="$CFLAGS $GTKMM_CXXFLAGS"
          LIBS="$LIBS $GTKMM_LIBS"
          AC_TRY_LINK([
#include <gtk--.h>
#include <stdio.h>
],      [ return ((gtkmm_major_version) || (gtkmm_minor_version) || (gtkmm_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GTK-- or finding the wrong"
          echo "*** version of GTK--. If it is not finding GTK--, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GTK-- was incorrectly installed"
          echo "*** or that you have moved GTK-- since it was installed. In the latter case, you"
          echo "*** may want to edit the gtkmm-config script: $GTKMM_CONFIG" ])
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GTKMM_CFLAGS=""
     GTKMM_LIBS=""
     ifelse([$3], , :, [$3])
     AC_LANG_RESTORE
  fi
  AC_SUBST(GTKMM_CFLAGS)
  AC_SUBST(GTKMM_LIBS)
  rm -f conf.gtkmmtest
])


