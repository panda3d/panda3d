###############################################################################
# Name: doctools.py                                                           #
# Purpose: Tools for managing document services                               #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Provides helper functions and classes for managing documents and their services.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: doctools.py 66159 2010-11-15 02:24:06Z CJP $"
__revision__ = "$Revision: 66159 $"

#--------------------------------------------------------------------------#
# Imports
import os
import sys

# Editra Libraries
import util
from profiler import Profile_Get
import ebmlib

#--------------------------------------------------------------------------#

class DocPositionMgr(object):
    """Object for managing the saving and setting of a collection of
    documents positions between sessions. Through the use of an in memory
    dictionary during run time and on disk dictionary to use when starting
    and stopping the editor.
    @note: saves config to ~/.Editra/cache/

    """
    _poscache = ebmlib.HistoryCache(100)

    def __init__(self):
        """Creates the position manager object"""
        super(DocPositionMgr, self).__init__()

        # Attributes
        self._init = False
        self._book = None
        self._records = dict()

    def InitPositionCache(self, book_path):
        """Initialize and load the on disk document position cache.
        @param book_path: path to on disk cache

        """
        self._init = True
        self._book = book_path
        if Profile_Get('SAVE_POS'):
            self.LoadBook(book_path)

    @classmethod
    def AddNaviPosition(cls, fname, pos):
        """Add a new position to the navigation cache
        @param fname: file name
        @param pos: position

        """
        # Don't put two identical positions in the cache next to each other
        pre = cls._poscache.PeekPrevious()
        next = cls._poscache.PeekNext()
        if (fname, pos) in (pre, next):
            return

        cls._poscache.PutItem((fname, pos))

    def AddRecord(self, vals):
        """Adds a record to the dictionary from a list of the
        filename vals[0] and the position value vals[1].
        @param vals: file path, cursor position
        @type vals: tuple (str, int)

        """
        if len(vals) == 2:
            self._records[vals[0]] = vals[1]
            return True
        else:
            return False

    @classmethod
    def CanNavigateNext(cls):
        """Are there more cached navigation positions?
        @return: bool

        """
        return cls._poscache.HasNext()

    @classmethod
    def CanNavigatePrev(cls):
        """Are there previous cached navigation positions?
        @return: bool

        """
        return cls._poscache.HasPrevious()

    @classmethod
    def FlushNaviCache(cls):
        """Clear the navigation cache"""
        cls._poscache.Clear()

    @classmethod
    def GetNaviCacheSize(cls):
        return cls._poscache.GetSize()

    def GetBook(self):
        """Returns the current book used by this object
        @return: path to book used by this manager

        """
        return self._book        

    @classmethod
    def GetNextNaviPos(cls, fname=None):
        """Get the next stored navigation position
        The optional fname parameter will get the next found position for
        the given file.
        @param fname: filename (note currently not supported)
        @return: int or None
        @note: fname is currently not used

        """
        item = cls._poscache.GetNextItem()
        return item

    @classmethod
    def GetPreviousNaviPos(cls, fname=None):
        """Get the last stored navigation position
        The optional fname parameter will get the last found position for
        the given file.
        @param fname: filename (note currently not supported)
        @return: int or None
        @note: fname is currently not used

        """
        item = cls._poscache.GetPreviousItem()
        return item

    def GetPos(self, name):
        """Get the position record for a given filename
        returns 0 if record is not found.
        @return: position value for the given filename

        """
        return self._records.get(name, 0)

    def IsInitialized(self):
        """Has the cache been initialized
        @return: bool

        """
        return self._init

    def LoadBook(self, book):
        """Loads a set of records from an on disk dictionary
        the entries are formated as key=value with one entry
        per line in the file.
        @return: whether book was loaded or not
        @rtype: boolean

        """
        # If file does not exist create it and return
        if not os.path.exists(book):
            try:
                tfile = util.GetFileWriter(book)
                tfile.close()
            except (IOError, OSError):
                util.Log("[docpositionmgr][err] failed to load book: %s" % book)
                return False
            except AttributeError:
                util.Log("[docpositionmgr][err] Failed to create: %s" % book)
                return False

        reader = util.GetFileReader(book, sys.getfilesystemencoding())
        if reader != -1:
            lines = list()
            try:
                lines = reader.readlines()
            except:
                reader.close()
                return False
            else:
                reader.close()

            for line in lines:
                line = line.strip()
                vals = line.rsplit(u'=', 1)
                if len(vals) != 2 or not os.path.exists(vals[0]):
                    continue

                try:
                    vals[1] = int(vals[1])
                except (TypeError, ValueError), msg:
                    util.Log("[docpositionmgr][err] %s" % str(msg))
                    continue
                else:
                    self._records[vals[0]] = vals[1]

            util.Log("[docpositionmgr][info] successfully loaded book")
            return True

    @classmethod
    def PeekNavi(cls, pre=False):
        """Peek into the navigation cache
        @param pre: bool

        """
        if pre:
            if cls._poscache.HasPrevious():
                return cls._poscache.PeekPrevious()
        else:
            if cls._poscache.HasNext():
                return cls._poscache.PeekNext()
        return None, None

    def WriteBook(self):
        """Writes the collection of files=pos to the config file
        @postcondition: in memory doc data is written out to disk

        """
        writer = util.GetFileWriter(self.GetBook(), sys.getfilesystemencoding())
        if writer != -1:
            try:
                for key, val in self._records.iteritems():
                    try:
                        writer.write(u"%s=%d\n" % (key, val))
                    except UnicodeDecodeError:
                        continue
                writer.close()
            except IOError, msg:
                util.Log("[docpositionmgr][err] %s" % str(msg))
        else:
            util.Log("[docpositionmgr][err] Failed to open %s" % self.GetBook())
