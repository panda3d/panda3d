#! /bin/sh
#
# This script generates the panda3d-date.tar.gz tarball for a file
# release of panda3d onto the SourceForge download site.
#
# Options:
#
#   -d cvsroot
#        Specifies the CVSROOT string to use to tag and export the
#        tree.  The default is $SFROOT if it is defined, or $CVSROOT
#        otherwise.
#
#   -r tag
#        Specifies the tag to export from.  If this parameter is
#        specified, the tree is not tagged again; otherwise, the
#        current head of the CVS tree is tagged with the file version
#        name.
#
#   -m module
#        Specifies the module to check out and build.  The default is 
#        panda3d.
#
#ENDCOMMENT

if [ ! -z "$SFROOT" ]; then
  cvsroot="$SFROOT"
else
  cvsroot="$CVSROOT"
fi
origtag=
module=panda3d
while getopts d:r:m:h OPT; do
    case $OPT in
       d) cvsroot="$OPTARG";;
       r) origtag="$OPTARG";;
       m) module="$OPTARG";;
       h) sed -e '/#ENDCOMMENT/,$d' <$0
          exit 1
          ;;
       ?) echo Use $0 -h to see options.
          exit 1
          ;;
    esac
done
shift `expr $OPTIND - 1`

if [ ! -z "$*" ]; then
  echo Unexpected options: $*
  echo Use $0 -h to see options.
  exit 1
fi

basename="$module"-`date +%F`
tarfile="$basename".tar.gz
zipfile="$basename".zip

if [ -e "$basename" ]; then
  echo "$basename" already exists in the local directory!
  exit 1
fi

if [ -z "$origtag" ]; then
  # If we weren't given a starting tag, make one.
  newtag="$basename"
  echo Tagging sources.
  cvs -f -d "$cvsroot" rtag -F -r HEAD "$newtag" "$module" || exit
else
  # Otherwise, we were given a starting tag, so use it.
  newtag="$origtag"
fi

echo Checking out "$module" as "$basename".
cvs -z3 -f -d "$cvsroot" export -r "$newtag" -d "$basename" "$module" || exit

# Move the contents of the doc module into the root directory where
# people will expect to see it.
if [ -d "$basename"/doc ]; then
  mv "$basename"/doc/* "$basename" || exit
  rmdir "$basename"/doc || exit
fi

# Generate the tarball.
tar cvzf "$tarfile" "$basename" || exit

# Also a zip file for the convenience of Windows users.
rm -f "$zipfile"
zip -r "$zipfile" "$basename" || exit

rm -rf "$basename"

