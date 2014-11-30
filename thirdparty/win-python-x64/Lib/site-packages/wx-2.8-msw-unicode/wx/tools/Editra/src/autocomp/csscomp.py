###############################################################################
# Name: csscomp.py                                                            #
# Purpose: Simple input assistant for CSS                                     #
# Author: Cody Precord                                                        #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Simple autocompletion support for Cascading Style Sheets.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__cvsid__ = "$Id: csscomp.py 67123 2011-03-04 00:02:35Z CJP $"
__revision__ = "$Revision: 67123 $"

#--------------------------------------------------------------------------#
# Imports
import re
import wx
import wx.stc

# Local Imports
import completer

#--------------------------------------------------------------------------#

# Regular Expressions
RE_LINK_PSEUDO = re.compile("a:(link|visited|active|hover|focus)*")
RE_CSS_COMMENT = re.compile("\/\*[^*]*\*+([^/][^*]*\*+)*\/")
RE_CSS_BLOCK = re.compile("\{[^}]*\}")

PSUEDO_SYMBOLS = completer.CreateSymbols([ u'active', u'focus', u'hover', 
                                           u'link', u'visited' ],
                                         )

#--------------------------------------------------------------------------#

class Completer(completer.BaseCompleter):
    """CSS Code completion provider"""
    def __init__(self, stc_buffer):
        super(Completer, self).__init__(stc_buffer)

        # Setup
        self.SetAutoCompKeys([ord(':'), ord('.') ])
        self.SetAutoCompStops(' {}#')
        self.SetAutoCompFillups('')
        self.SetCallTipKeys([ord('('), ])
        self.SetCallTipCancel([ord(')'), wx.WXK_RETURN])
        
    def GetAutoCompList(self, command):
        """Returns the list of possible completions for a
        command string. If namespace is not specified the lookup
        is based on the locals namespace
        @param command: command lookup is done on
        @keyword namespace: namespace to do lookup in

        """
        buff = self.GetBuffer()
        keywords = buff.GetKeywords()
        if command in [None, u'']:
            return completer.CreateSymbols(keywords, completer.TYPE_UNKNOWN)

        cpos = buff.GetCurrentPos()
        cline = buff.GetCurrentLine()
        lstart = buff.PositionFromLine(cline)
        tmp = buff.GetTextRange(lstart, cpos).rstrip()

        # Check for the case of a pseudo class
        if IsPsuedoClass(command, tmp):
            return PSUEDO_SYMBOLS

        # Give some help on some common properties
        if tmp.endswith(u':'):
            word = GetWordLeft(tmp.rstrip().rstrip(u':'))
            comps = PROP_OPTS.get(word, list())
            comps = list(set(comps))
            comps.sort()
            return completer.CreateSymbols(comps, completer.TYPE_PROPERTY)

        # Look for if we are completing a tag class
        if tmp.endswith(u'.'):
            classes = list()
            if not buff.IsString(cpos):
                txt = buff.GetText()
                txt = RE_CSS_COMMENT.sub(u'', txt)
                txt = RE_CSS_BLOCK.sub(u' ', txt)
                for token in txt.split():
                    if u'.' in token:
                        classes.append(token.split(u'.', 1)[-1])

                classes = list(set(classes))
                classes.sort()
            return completer.CreateSymbols(classes, completer.TYPE_CLASS)

        return completer.CreateSymbols(keywords, completer.TYPE_UNKNOWN)

    def GetCallTip(self, command):
        """Returns the formated calltip string for the command.
        If the namespace command is unset the locals namespace is used.

        """
        if command == u'url':
            return u'url(\'../path\')'
        else:
            return u''

    def ShouldCheck(self, cpos):
        """Should completions be attempted
        @param cpos: current buffer position
        @return: bool

        """
        buff = self.GetBuffer()
        rval = True
        if buff is not None:
            if buff.IsComment(cpos):
                rval =  False
        return rval

#--------------------------------------------------------------------------#

def IsPsuedoClass(cmd, line):
    """Check the line to see if its a link pseudo class
    @param cmd: current command
    @param line: line of the command
    @return: bool

    """
    if cmd.endswith(u':'):
        token = line.split()[-1]
        pieces = token.split(u":")
        if pieces[0] == 'a' or pieces[0].startswith('a.'):
            return True
    return False

def GetWordLeft(line):
    """Get the first valid word to the left of the end of line
    @return: string

    """
    for idx in range(1, len(line)+1):
        ch = line[idx*-1]
        if ch.isspace() or ch in u'{;':
            return line[-1*idx:].strip()
    else:
        return u''

#--------------------------------------------------------------------------#

# Properties to provide some input help on
PROP_OPTS = { u'border-style' : [u'none', u'hidden', u'dotted', u'dashed',
                                 u'solid', u'double', u'groove', u'ridge',
                                 u'inset', u'outset'],
              u'float' : [u'left', u'right', u'none'],
              u'font-style' : [u'normal', u'italic', u'oblique'],
              u'font-weight' : [u'normal', u'bold', u'lighter', u'bolder'],
              u'list-style-type' : [u'none', u'disc', u'circle', u'square',
                                    u'decimal', u'decimal-leading-zero',
                                    u'lower-roman', u'upper-roman',
                                    u'lower-alpha', u'upper-alpha',
                                    u'lower-greek', u'lower-latin', u'hebrew',
                                    u'armenian', u'georgian', u'cjk-ideographic',
                                    u'hiragana', u'katakana',
                                    u'hiragana-iroha', u'katakana-iroha'],
              u'text-decoration' : [u'none', u'underline', u'line-through',
                                    u'overline', u'blink'],
              u'text-align' : [u'left', u'right', u'center', u'justify'],
              u'vertical-align' : [u'baseline', u'sub', u'super', u'top',
                                   u'text-top', u'middle', u'bottom',
                                   u'text-bottom', ]
              }

