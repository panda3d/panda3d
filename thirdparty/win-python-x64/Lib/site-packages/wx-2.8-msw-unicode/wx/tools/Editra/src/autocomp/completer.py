###############################################################################
# Name: completer.py                                                          #
# Purpose: Autcompleter interface base class.                                 #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Base class for autocompletion providers to implement the completion interface.

@summary: Autocompleter base class

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: completer.py 67701 2011-05-04 20:50:14Z CJP $"
__revision__ = "$Revision: 67701 $"

__all__ = [ 'TYPE_FUNCTION', 'TYPE_METHOD', 'TYPE_CLASS', 'TYPE_ATTRIBUTE',
            'TYPE_VARIABLE', 'TYPE_ELEMENT', 'TYPE_PROPERTY', 'TYPE_UNKNOWN',
            'BaseCompleter', 'Symbol', 'CreateSymbols' ]

#--------------------------------------------------------------------------#
# Imports
import wx

#--------------------------------------------------------------------------#

# Image Type Ids
TYPE_FUNCTION, \
TYPE_METHOD, \
TYPE_CLASS, \
TYPE_ATTRIBUTE, \
TYPE_PROPERTY, \
TYPE_VARIABLE, \
TYPE_ELEMENT, \
TYPE_UNKNOWN = range(1, 9)

#--------------------------------------------------------------------------#
class Symbol(object):
    """ Defines a symbol as parsed by the autocompleter.
    Symbols with the same name and different type are EQUAL
    Symbol hash is based on symbol NAME
    
    """
    # we create lots of these so use slots as a performance tweak
    __slots__ = ('_name', '_type')

    def __init__(self, name, symtype):
        """ Constructor
        @param name: Symbol name
        @param symtype: Symbol type, one of the TYPE_FUNCTION ... TYPE_UNKNOWN range
        
        """
        super(Symbol, self).__init__()

        # Attributes
        self._name = unicode(name)
        self._type = symtype
    
    def __eq__(self, other):
        return (self.Name == other.Name)

    def __lt__(self, other):
        return (self.Name < other.Name)

    def __le__(self, other):
        return (self.Name <= other.Name)

    def __ne__(self, other):
        return (self.Name != other.Name)

    def __gt__(self, other):
        return (self.Name > other.Name)

    def __ge__(self, other):
        return (self.Name >= other.Name)

    # TODO: this task should probably be delegated to the ui
    def __str__(self):
        if self.Type != TYPE_UNKNOWN:
            return u'?'.join([self.Name, unicode(self.Type)])
        else:
            return self.Name

    def __hash__(self):
        return hash(self.Name)

    Name = property(lambda self: self._name,
                    lambda self, n: setattr(self, '_name', n))
    Type = property(lambda self: self._type,
                    lambda self, t: setattr(self, '_type', t))

#--------------------------------------------------------------------------#

def CreateSymbols(arglst, symtype=TYPE_UNKNOWN):
    """Convert a list of strings to a list of Symbol objects
    @param arglst: list of strings
    @keyword symtype: TYPE_FOO 
    @return: list of Symbols

    """
    return [ Symbol(obj, symtype) for obj in arglst ]

#--------------------------------------------------------------------------#

class BaseCompleter(object):
    """Base Autocomp provider class"""
    def __init__(self, parent):
        """Initializes the auto-completion service
        @param parent: parent of this service object

        """
        super(BaseCompleter, self).__init__()

        # Attributes
        self._buffer = parent
        self._log = wx.GetApp().GetLog()
        self._case_sensitive = False
        self._autocomp_after = False
        self._choose_single = True

        self._autocomp_keys = list()
        self._autocomp_stop = u''
        self._autocomp_fillup = u''
        self._calltip_keys = list()
        self._calltip_cancel = list()

    #--- Override in subclass ----#

    def GetAutoCompList(self, command):
        """Retrieves the sorted autocomplete list for a command
        @param command: command string to do lookup on
        @return: list of strings

        """
        return list()

    def GetCallTip(self, command):
        """Returns the calltip string for a command
        @param command: command to get calltip for (string)
        @return: string

        """
        return u''

    def OnCompletionInserted(self, pos, text):
        """Called by the buffer when an autocomp selection has been inserted.
        The completer can override this method to 
        @param pos: Position the caret was at before the insertion
        @param text: text that was inserted at pos

        """
        pass

    #--- End override in subclass ----#

    def GetAutoCompKeys(self):
        """Returns the list of key codes for activating the autocompletion.
        @return: list of characters used for activating autocompletion

        """
        return self._autocomp_keys

    def SetAutoCompKeys(self, key_list):
        """Set the keys to provide completions on
        @param key_list: List of key codes

        """
        self._autocomp_keys = key_list

    def GetAutoCompStops(self):
        """Returns a string of characters that should cancel
        the autocompletion lookup.
        @return: string of characters that will hide the autocomp/calltip

        """
        return self._autocomp_stop

    def SetAutoCompStops(self, stops):
        """Set the keys to cancel autocompletions on.
        @param stops: string

        """
        self._autocomp_stop = stops

    def GetAutoCompFillups(self):
        """Get the list of characters to do a fillup on
        @return: string

        """
        return self._autocomp_fillup

    def SetAutoCompFillups(self, fillups):
        """Set the list of characters to do a fillup on
        @param fillups: string

        """
        self._autocomp_fillup = fillups

    def GetCallTipKeys(self):
        """Returns the list of keys to activate a calltip on
        @return: list of calltip activation keys

        """
        return self._calltip_keys

    def SetCallTipKeys(self, keys):
        """Set the list of keys to activate calltips on
        @return: list of calltip activation keys

        """
        self._calltip_keys = keys

    def GetCallTipCancel(self):
        """Get the list of key codes that should stop a calltip"""
        return self._calltip_cancel

    def SetCallTipCancel(self, key_list):
        """Set the list of key codes that should stop a calltip"""
        self._calltip_cancel = key_list

    def GetBuffer(self):
        """Get the reference to the buffer this autocomp object is owned by
        @return: EditraStc

        """
        return self._buffer

    def GetCaseSensitive(self):
        """Are commands case sensitive or not
        @return: bool

        """
        return self._case_sensitive

    def SetCaseSensitive(self, sensitive):
        """Set whether this completer is case sensitive or not
        @param sensitive: bool

        """
        self._case_sensitive = sensitive

    def GetChooseSingle(self):
        """Get whether the completer should automatically choose a selection
        when there is only one symbol in the completion list.
        @return: bool

        """
        return self._choose_single

    def SetChooseSingle(self, single):
        """Set whether the completer should automatically choose a selection
        when there is only one symbol in the completion list.
        @param single: bool

        """
        self._choose_single = single

    def ShouldCheck(self, cpos):
        """Should completions be attempted
        @param cpos: current buffer position
        @return: bool

        """
        buff = self.GetBuffer()
        rval = True
        if buff is not None:
            if buff.IsString(cpos) or buff.IsComment(cpos):
                rval = False
        return rval
