###############################################################################
# Name: txtutil.py                                                            #
# Purpose: Text Utilities.                                                    #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Business Model Library: Text Utilities

Utility functions for managing and working with text.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: txtutil.py 67991 2011-06-20 23:48:01Z CJP $"
__revision__ = "$Revision: 67991 $"

__all__ = [ 'IsUnicode', 'DecodeString']

#-----------------------------------------------------------------------------#
# Imports
import types

#-----------------------------------------------------------------------------#

def IsUnicode(txt):
    """Is the given string a unicode string
    @param txt: object
    @return: bool

    """
    return isinstance(txt, types.UnicodeType)

def DecodeString(txt, enc):
    """Decode the given string with the given encoding,
    only attempts to decode if the given txt is not already Unicode
    @param txt: string
    @param enc: encoding 'utf-8'
    @return: unicode

    """
    if IsUnicode(txt):
        txt = txt.decode(enc)
    return txt
