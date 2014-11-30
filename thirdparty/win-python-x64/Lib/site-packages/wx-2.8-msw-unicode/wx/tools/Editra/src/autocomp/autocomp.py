###############################################################################
# Name: autocomp.py                                                           #
# Purpose: Provides the front end interface for autocompletion services for   #
#          the editor.                                                        #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Provides an interface/service for getting autocompletion/calltip data
into an stc control. This is a data provider only it does not do provide
any UI functionality or calls. The user called object from this library
is intended to be the AutoCompService. This service provides the generic
interface into the various language specific autocomplete services, and
makes the calls to the other support objects/functions in this library.

@summary: Autocompletion support interface implementation

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: autocomp.py 66207 2010-11-18 15:56:19Z CJP $"
__revision__ = "$Revision: 66207 $"
__all__ = ['AutoCompService',]

#--------------------------------------------------------------------------#
# Dependencies
import wx
import wx.stc as stc

# Local imports
import simplecomp

#--------------------------------------------------------------------------#

class AutoCompService(object):
    """Interface to retrieve and provide autocompletion and
    calltip information to an stc control. The plain text
    (empty) completion provider is built in. All other providers
    are loaded from external modules on request.

    """
    def __init__(self):
        """Initializes the autocompletion service"""
        super(AutoCompService, self).__init__()

    @staticmethod
    def GetCompleter(buff, extended=False):
        """Get the appropriate completer object for the given buffer.
        @todo: implement dynamic loading mechanism for each comp class

        """
        lex_value = buff.GetLexer()
        if lex_value == stc.STC_LEX_PYTHON:
            import pycomp
            compl = pycomp.Completer
        elif lex_value in (stc.STC_LEX_HTML, stc.STC_LEX_XML):
            import htmlcomp
            compl = htmlcomp.Completer
        elif lex_value == stc.STC_LEX_CSS:
            import csscomp
            compl = csscomp.Completer
        else:
            return simplecomp.Completer(buff)

        if extended:
            compl = CompleterFactory(compl, buff)
        else:
            compl = compl(buff)

        return compl

#--------------------------------------------------------------------------#

class MetaCompleter(type):
    """Meta class for creating custom completer classes at runtime"""
    def __call__(mcs, base, buff):
        """Modify the base class with our new methods at time of
        instantiation.

        """
        obj = type.__call__(mcs, base, buff)
 
        # Set/override attributes on the new completer object.
        setattr(obj, 'BaseGetAutoCompList', obj.GetAutoCompList)
        setattr(obj, 'GetAutoCompList', lambda cmd: GetAutoCompList(obj, cmd))
        setattr(obj, 'scomp', simplecomp.Completer(buff))

        # Return the new augmented completer
        return obj

def GetAutoCompList(self, command):
    """Apply SimpleCompleter results to base results from the
    'smart' completer.

    """
    baseList = self.BaseGetAutoCompList(command)
    scompList = self.scomp.GetAutoCompList(command)
    # Wipeout duplicates by creating a set, then sort data alphabetically
    baseList.extend(scompList)
    rlist = list(set(baseList))
    rlist.sort()
    return rlist

class CompleterFactory(object):
    """Factory for creating composite completer objects"""
    __metaclass__ = MetaCompleter
    def __new__(cls, base, buff):
        """Return an instance of the passed in class type"""
        self = base(buff)
        return self
