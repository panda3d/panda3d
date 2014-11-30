###############################################################################
# Name: searcheng.py                                                          #
# Purpose: Text search engine and utilities                                   #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Business Model Library: SearchEngine

Text Search Engine for finding text and grepping files

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__cvsid__ = "$Id: searcheng.py 68232 2011-07-12 02:08:53Z CJP $"
__revision__ = "$Revision: 68232 $"

__all__ = [ 'SearchEngine', ]

#-----------------------------------------------------------------------------#
# Imports
import os
import re
import fnmatch
import types
import unicodedata
from StringIO import StringIO

# Local imports
import fchecker

#-----------------------------------------------------------------------------#

class SearchEngine(object):
    """Text Search Engine
    All Search* methods are iterable generators
    All Find* methods do a complete search and return the match collection
    @summary: Text Search Engine
    @todo: Add file filter support

    """
    def __init__(self, query, regex=True, down=True,
                  matchcase=True, wholeword=False):
        """Initialize a search engine object
        @param query: search string
        @keyword regex: Is a regex search
        @keyword down: Search down or up
        @keyword matchcase: Match case
        @keyword wholeword: Match whole word

        """
        super(SearchEngine, self).__init__()

        # Attributes
        self._isregex = regex
        self._next = down
        self._matchcase = matchcase
        self._wholeword = wholeword
        self._query = query
        self._regex = u''
        self._pool = u''
        self._lmatch = None             # Last match object
        self._filters = None            # File Filters
        self._formatter = lambda f, l, m: u"%s %d: %s" % (f, l+1, m)
        self._CompileRegex()

    def _CompileRegex(self):
        """Prepare and compile the regex object based on the current state
        and settings of the engine.
        @postcondition: the engines regular expression is created

        """
        tmp = self._query

        uquery = type(tmp) is types.UnicodeType
        upool = type(self._pool) is types.UnicodeType
        if uquery and upool:
            tmp = unicodedata.normalize("NFD", tmp)

        if not self._isregex:
            tmp = re.escape(tmp)

        if self._wholeword:
            if uquery:
                tmp = u"\\b%s\\b" % tmp
            else:
                tmp = "\\b%s\\b" % tmp

        flags = re.MULTILINE
        if not self._matchcase:
            flags |= re.IGNORECASE

        if upool:
            flags |= re.UNICODE
            # Normalize
            self._pool = unicodedata.normalize("NFD", self._pool)
        else:
            # If the pools is not Unicode also make sure that the
            # query is a string too.
            if uquery:
                try:
                    tmp = tmp.encode('utf-8')
                except UnicodeEncodeError:
                    # TODO: better error reporting about encoding issue
                    self._regex = None
                    return
        try:
            self._regex = re.compile(tmp, flags)
        except:
                self._regex = None
        self._data = (tmp, self._pool)

    def ClearPool(self):
        """Clear the search pool"""
        del self._pool
        self._pool = u""

    def Find(self, spos=0):
        """Find the next match based on the state of the search engine
        @keyword spos: search start position
        @return: tuple (match start pos, match end pos) or None if no match
        @note: L{SetSearchPool} has been called to set search string

        """
        if self._regex is None:
            return None

        if self._next:
            return self.FindNext(spos)
        else:
            if spos == 0:
                spos = -1
            return self.FindPrev(spos)

    def FindAll(self):
        """Find all the matches in the current context
        @return: list of tuples [(start1, end1), (start2, end2), ]

        """
        if self._regex is None:
            return list()

        matches = [match for match in self._regex.finditer(self._pool)]
        return matches

    def FindAllLines(self):
        """Find all the matches in the current context
        @return: list of strings

        """
        rlist = list()
        if self._regex is None:
            return rlist

        for lnum, line in enumerate(StringIO(self._pool)):
            if self._regex.search(line) is not None:
                rlist.append(self._formatter(u"Untitled", lnum, line))

        return rlist

    def FindNext(self, spos=0):
        """Find the next match of the query starting at spos
        @keyword spos: search start position in string
        @return: tuple (match start pos, match end pos) or None if no match
        @note: L{SetSearchPool} has been called to set the string to search in.

        """
        if self._regex is None:
            return None

        if spos < len(self._pool):
            match = self._regex.search(self._pool[spos:])
            if match is not None:
                self._lmatch = match
                return match.span()
        return None

    def FindPrev(self, spos=-1):
        """Find the previous match of the query starting at spos
        @keyword spos: search start position in string
        @return: tuple (match start pos, match end pos)

        """
        if self._regex is None:
            return None

        if spos+1 < len(self._pool):
            matches = [match for match in
                       self._regex.finditer(self._pool[:spos])]
            if len(matches):
                lmatch = matches[-1]
                self._lmatch = lmatch
                return (lmatch.start(), lmatch.end())
        return None

    def GetLastMatch(self):
        """Get the last found match object from the previous L{FindNext} or
        L{FindPrev} action.
        @return: match object or None

        """
        return self._lmatch

    def GetOptionsString(self):
        """Get a string describing the search engines options"""
        rstring = u"\"%s\" [ " % self._query
        for desc, attr in (("regex: %s", self._isregex),
                           ("match case: %s", self._matchcase),
                           ("whole word: %s", self._wholeword)):
            if attr:
                rstring += (desc % u"on; ")
            else:
                rstring += (desc % u"off; ")
        rstring += u"]"

        return rstring

    def GetQuery(self):
        """Get the raw query string used by the search engine
        @return: string

        """
        return self._query

    def GetQueryObject(self):
        """Get the regex object used for the search. Will return None if
        there was an error in creating the object.
        @return: pattern object

        """
        return self._regex

    def GetSearchPool(self):
        """Get the search pool string for this L{SearchEngine}.
        @return: string

        """
        return self._pool

    def IsMatchCase(self):
        """Is the engine set to a case sensitive search
        @return: bool

        """
        return self._matchcase

    def IsRegEx(self):
        """Is the engine searching with the query as a regular expression
        @return: bool

        """
        return self._isregex

    def IsWholeWord(self):
        """Is the engine set to search for wholeword matches
        @return: bool

        """
        return self._wholeword

    def SearchInBuffer(self, sbuffer):
        """Search in the buffer
        @param sbuffer: buffer like object
        @todo: implement

        """
        raise NotImplementedError

    def SearchInDirectory(self, directory, recursive=True):
        """Search in all the files found in the given directory
        @param directory: directory path
        @keyword recursive: search recursivly

        """
        if self._regex is None:
            return

        # Get all files in the directories
        paths = [os.path.join(directory, fname)
                for fname in os.listdir(directory) if not fname.startswith('.')]

        # Filter out files that don't match the current filter(s)
        if self._filters is not None and len(self._filters):
            filtered = list()
            for fname in paths:
                if os.path.isdir(fname):
                    filtered.append(fname)
                    continue

                for pat in self._filters:
                    if fnmatch.fnmatch(fname, pat):
                        filtered.append(fname)
            paths = filtered

        # Begin searching in the paths
        for path in paths:
            if recursive and os.path.isdir(path):
                # Recursive call to decend into directories
                for match in self.SearchInDirectory(path, recursive):
                    yield match
            else:
                for match in self.SearchInFile(path):
                    yield match
        return

    def SearchInFile(self, fname):
        """Search in a file for all lines with matches of the set query and
        yield the results as they are found.
        @param fname: filename
        @todo: unicode handling

        """
        if self._regex is None:
            return

        checker = fchecker.FileTypeChecker()
        if checker.IsReadableText(fname):
            try:
                fobj = open(fname, 'rb')
            except (IOError, OSError):
                return
            else:
                # Special token to signify start of a search
                yield (None, fname)

            for lnum, line in enumerate(fobj):
                if self._regex.search(line) is not None:
                    yield self._formatter(fname, lnum, line)
            fobj.close()
        return

    def SearchInFiles(self, flist):
        """Search in a list of files and yield results as they are found.
        @param flist: list of file names

        """
        if self._regex is None:
            return

        for fname in flist:
            for match in self.SearchInFile(fname):
                yield match
        return

    def SearchInString(self, sstring, startpos=0):
        """Search in a string
        @param sstring: string to search in
        @keyword startpos: search start position

        """
        raise NotImplementedError

    def SetFileFilters(self, filters):
        """Set the file filters to specify what type of files to search in
        the filter should be a list of wild card patterns to match.
        @param filters: list of strings ['*.py', '*.pyw']

        """
        self._filters = filters

    def SetFlags(self, isregex=None, matchcase=None, wholeword=None, down=None):
        """Set the search engine flags. Leaving the parameter set to None
        will not change the flag. Setting it to non None will change the value.
        @keyword isregex: is regex search
        @keyword matchcase: matchcase search
        @keyword wholeword: wholeword search
        @keyword down: search down or up

        """
        for attr, val in (('_isregex', isregex), ('_matchcase', matchcase),
                          ('_wholeword', wholeword), ('_next', down)):
            if val is not None:
                setattr(self, attr, val)
        self._CompileRegex()

    def SetMatchCase(self, case=True):
        """Set whether the engine will use case sensative searches
        @keyword case: bool

        """
        self._matchcase = case
        self._CompileRegex()

    def SetResultFormatter(self, funct):
        """Set the result formatter function
        @param funct: callable(filename, linenum, matchstr)

        """
        assert callable(funct)
        self._formatter = funct

    def SetSearchPool(self, pool):
        """Set the search pool used by the Find methods
        @param pool: string to search in

        """
        del self._pool
        self._pool = pool
        self._CompileRegex()

    def SetQuery(self, query):
        """Set the search query
        @param query: string

        """
        self._query = query
        self._CompileRegex()

    def SetUseRegex(self, use=True):
        """Set whether the engine is using regular expresion searches or
        not.
        @keyword use: bool

        """
        self._isregex = use
        self._CompileRegex()
