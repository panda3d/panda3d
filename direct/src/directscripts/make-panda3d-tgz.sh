#! /bin/sh
#
# This script generates the panda3d-date.tar.gz tarball for a file
# release of panda3d onto the SourceForge download site.
#

module=panda3d
basename=$module-`date +%F`
cvsroot=:pserver:anonymous@nit:/cvsroot/panda3d

if [ -e $basename ]; then
  echo $basename already exists in the local directory!
  exit 1
fi

echo Checking out $module as $basename.
cvs -f -d $cvsroot export -r HEAD -d $basename $module || exit

# Move the contents of the doc module into the root directory where
# people will expect to see it.
mv $basename/doc/* $basename || exit
rmdir $basename/doc || exit

# Generate the tarball.
tarfile=$basename.tar.gz
tar cvzf $tarfile $basename || exit
rm -rf $basename

