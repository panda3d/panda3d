 #-*- coding: utf-8 -*-
###############################################################################
# Name: syndata.py                                                            #
# Purpose: Syntax Data Base                                                   #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

""" Interface definition for syntax data

@summary: Editra Syntax Data Interface Definition

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: syndata.py 67392 2011-04-04 21:06:43Z CJP $"
__revision__ = "$Revision: 67392 $"

__all__ = ['SyntaxDataBase',]

#-----------------------------------------------------------------------------#
# Imports
import collections
import wx.stc as stc

# Local Imports
import synglob

#-----------------------------------------------------------------------------#

class SyntaxDataBase(object):
    """Syntax data container object base class"""
    def __init__(self, langid=synglob.ID_LANG_TXT):
        object.__init__(self)

        # Attributes
        self._langid = langid
        self._lexer = stc.STC_LEX_NULL
        self._features = dict()

    @property
    def CommentPattern(self):
        return self.GetCommentPattern()

    @property
    def Keywords(self):
        return self.GetKeywords()

    @property
    def LangId(self):
        return self.GetLangId()

    @property
    def Lexer(self):
        return self.GetLexer()

    @property
    def Properties(self):
        return self.GetProperties()

    @property
    def SyntaxSpec(self):
        return self.GetSyntaxSpec()

    #---- Interface Methods ----#

    def GetCommentPattern(self):
        """Get the comment pattern
        @return: list of strings ['/*', '*/']

        """
        return list()

    def GetKeywords(self):
        """Get the Keyword List(s)
        @return: list of tuples [(1, ['kw1', kw2']),]

        """
        return list()

    def GetLangId(self):
        """Get the language id
        @return: int

        """
        return self._langid

    def GetLexer(self):
        """Get the lexer id
        @return: wx.stc.STC_LEX_

        """
        return self._lexer

    def GetProperties(self):
        """Get the Properties List
        @return: list of tuples [('fold', '1'),]

        """
        return list()

    def GetSyntaxSpec(self):
        """Get the the syntax specification list
        @return: list of tuples [(int, 'style_tag'),]
        @note: required override for subclasses

        """
        raise NotImplementedError

    #---- End Interface Methods ----#

    def GetFeature(self, name):
        """Get a registered features callable
        @param name: feature name
        @return: callable or None

        """
        return self._features.get(name, None)

    def RegisterFeature(self, name, funct):
        """Register an extension feature with the factory
        @param name: feature name
        @param funct: callable

        """
        assert isinstance(funct, collections.Callable), "funct must be callable object"
        self._features[name] = funct

    def SetLexer(self, lex):
        """Set the lexer object for this data object"""
        self._lexer = lex

    def SetLangId(self, lid):
        """Set the language identifier
        @param lid: int

        """
        self._langid = lid

#-----------------------------------------------------------------------------#
