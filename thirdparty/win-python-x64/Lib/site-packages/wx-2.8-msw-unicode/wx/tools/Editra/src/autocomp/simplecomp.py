###############################################################################
# Name: simplecomp.py                                                         #
# Purpose: Simple autocompletion based on buffer words (SciTE docet)          #
# Author: Giuseppe "Cowo" Corbelli                                            #
# Copyright: (c) 2009 Giuseppe "Cowo" Corbelli                                #
# License: wxWindows License                                                  #
###############################################################################

"""
Simple Generic autocompleter for completing words found in the current buffer.

"""

__author__ = "Giuseppe \"Cowo\" Corbelli"
__cvsid__ = "$Id: simplecomp.py 67123 2011-03-04 00:02:35Z CJP $"
__revision__ = "$Revision: 67123 $"

#--------------------------------------------------------------------------#
# Imports
import string
import wx.stc as stc

# Local Imports
import completer

#--------------------------------------------------------------------------#

class Completer(completer.BaseCompleter):
    """Generic word completer provider"""
    wordCharacters = "".join(['_', string.letters])

    def __init__(self, stc_buffer):
        super(Completer, self).__init__(stc_buffer)

        # Setup
        self.SetAutoCompKeys([])
        self.SetAutoCompStops(' \'"\\`):')
        self.SetAutoCompFillups('.,:;([]){}<>%^&+-=*/|$')
        self.SetCallTipKeys([])
        self.SetCallTipCancel([])
        self.SetCaseSensitive(False)

    def _GetCompletionInfo(self, command, calltip=False):
        """Get Completion list or Calltip
        @return: list or string

        """
        bf = self.GetBuffer()
        # A list of Symbol(keyword, TYPE_UNKNOWN)
        kwlst = map(
            lambda kw: completer.Symbol(kw, completer.TYPE_UNKNOWN),
            bf.GetKeywords()
        )

        if command in (None, u''):
            return kwlst

        fillups = self.GetAutoCompFillups()
        if command[0].isdigit() or (command[-1] in fillups):
            return list()

        currentPos = bf.GetCurrentPos()

        # Get the real word: segment using autocompFillup
        tmp = command
        for ch in fillups:
            tmp = command.strip(ch)
        ls = list(tmp)
        ls.reverse()

        idx = 0
        for c in ls:
            if c in fillups:
                break
            idx += 1
        ls2 = ls[:idx]
        ls2.reverse()
        command = u"".join(ls2)

        # Available completions so far
        wordsNear = []
        maxWordLength = 0
        nWords = 0
        minPos = 0
        maxPos = bf.GetLength()
        flags = stc.STC_FIND_WORDSTART
        if self.GetCaseSensitive():
            flags |= stc.STC_FIND_MATCHCASE

        posFind = bf.FindText(minPos, maxPos, command, flags)
        while posFind >= 0 and posFind < maxPos:
            wordEnd = posFind + len(command)
            if posFind != currentPos:
                while -1 != Completer.wordCharacters.find(chr(bf.GetCharAt(wordEnd))):
                    wordEnd += 1

                wordLength = wordEnd - posFind
                if wordLength > len(command):
                    word = bf.GetTextRange(posFind, wordEnd)
                    sym = completer.Symbol(word, completer.TYPE_UNKNOWN)
                    if not wordsNear.count(sym):
                        wordsNear.append(sym)
                        maxWordLength = max(maxWordLength, wordLength)
                        nWords += 1

            minPos = wordEnd
            posFind = bf.FindText(minPos, maxPos, command, flags)

        if len(wordsNear) > 0 and (maxWordLength > len(command)):
            return wordsNear

        return kwlst

    def GetAutoCompList(self, command):
        """Returns the list of possible completions for a
        command string. If namespace is not specified the lookup
        is based on the locals namespace
        @param command: commadn lookup is done on
        @keyword namespace: namespace to do lookup in

        """
        rlist = self._GetCompletionInfo(command)
        return sorted(list(set(rlist)))
