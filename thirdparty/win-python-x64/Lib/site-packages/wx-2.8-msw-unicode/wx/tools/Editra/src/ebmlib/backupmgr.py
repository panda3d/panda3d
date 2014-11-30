###############################################################################
# Name: backupmgr.py                                                          #
# Purpose: File Backup Manager                                                #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Business Model Library: FileBackupMgr

Helper class for managing and creating backups of files.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__cvsid__ = "$Id: backupmgr.py 67646 2011-04-29 03:07:20Z CJP $"
__revision__ = "$Revision: 67646 $"

__all__ = [ 'FileBackupMgr', ]

#-----------------------------------------------------------------------------#
# Imports
import os
import shutil

# Local Imports
import fileutil
import fchecker

#-----------------------------------------------------------------------------#

class FileBackupMgr(object):
    """File backup creator and manager"""
    def __init__(self, header=None, template=u"%s~"):
        """Create a BackupManager
        @keyword header: header to id backups with (Text files only!!)
        @keyword template: template string for naming backup file with

        """
        super(FileBackupMgr, self).__init__()

        # Attributes
        self.checker = fchecker.FileTypeChecker()
        self.header = header       # Backup id header
        self.template = template   # Filename template
        self.bkupdir = u""

    def _CheckHeader(self, fname):
        """Check if the backup file has a header that matches the
        header used to identify backup files.
        @param fname: name of file to check
        @return: bool (True if header is ok, False otherwise)

        """
        isok = False
        handle = None
        try:
            handle = open(fname, 'r')
            line = handle.readline()
            handle.close()
            isok = line.startswith(self.header)
        except Exception, msg:
            isok = False
            if handle:
                handle.close()
        return isok

    def GetBackupFilename(self, fname):
        """Get the unique name for the files backup copy
        @param fname: string (file path)
        @return: string

        """
        if self.bkupdir:
            tmp = fileutil.GetFileName(fname)
            fname = os.path.join(self.bkupdir, tmp)

        rname = self.template % fname
        if self.header is not None and \
           not self.checker.IsBinary(fname) and \
           os.path.exists(rname):
            # Make sure that the template backup name does not match
            # an existing file that is not a backup file.
            while os.path.exists(rname):
                if not self._CheckHeader(rname):
                    rname = self.template % rname
                else:
                    break
                
        return rname

    def GetBackupWriter(self, fileobj):
        """Create a backup filewriter method to backup a files contents
        with.
        @param fileobj: object implementing fileimpl.FileObjectImpl interface
        @return: callable(text) to create backup with

        """
        nfile = fileobj.Clone()
        fname = self.GetBackupFilename(nfile.GetPath())
        nfile.SetPath(fname)
        # Write the header if it is enabled
        if self.header and not self.checker.IsBinary(fname):
            nfile.Write(self.header + os.linesep)
        return nfile.Write

    def HasBackup(self, fname):
        """Check if a given file has a backup file available or not
        @param fname: string (file path)

        """
        backup = self.GetBackupFilename(fname)
        return os.path.exists(backup)

    def IsBackupNewer(self, fname):
        """Is the backup of this file newer than the saved version
        of the file?
        @param fname: string (file path)
        @return: bool

        """
        backup = self.GetBackupFilename(fname)
        if os.path.exists(fname) and os.path.exists(backup):
            mod1 = fileutil.GetFileModTime(backup)
            mod2 = fileutil.GetFileModTime(fname)
            return mod1 > mod2
        else:
            return False

    def MakeBackupCopy(self, fname):
        """Create a backup copy of the given filename
        @param fname: string (file path)
        @return: bool (True == Success)

        """
        backup = self.GetBackupFilename(fname)
        try:
            if os.path.exists(backup):
                os.remove(backup)

            shutil.copy2(fname, backup)
            if self.header:
                handle = open(backup, 'r')
                txt = handle.read()
                handle.close()
                handle = open(backup, 'w')
                handle.write(self.header + os.linesep)
                handle.write(txt)
                handle.close()
        except:
            return False
        else:
            return True

    def SetBackupDirectory(self, path):
        """Set the backup directory to use for all backups created by this
        manager instance. Setting the path to an empty string will set the
        default behavior to write the backup to the same directory as the
        where the file that is being backedup is located.

        """
        self.bkupdir = path 

    def SetBackupFileTemplate(self, tstr):
        """Set the filename template for generating the backupfile name
        @param tstr: template string i.e) %s~

        """
        assert tstr.count("%s") == 1, "Format statment must only have one arg"
        self.template = tstr

    def SetHeader(self, header):
        """Set the header string for identifying a file as a backup
        @param header: string (single line only)

        """
        assert '\n' not in header, "Header must only be a single line"
        self.header = header
