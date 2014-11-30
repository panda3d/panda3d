###############################################################################
# Name: miscutil.py                                                           #
# Purpose: Various helper functions.                                          #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Business Model Library: MiscUtil

Various helper functions

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__cvsid__ = "$Id: miscutil.py 67329 2011-03-28 23:40:48Z CJP $"
__revision__ = "$Revision: 67329 $"

__all__ = [ 'MinMax', 'Singleton']

#-----------------------------------------------------------------------------#
# Imports

#-----------------------------------------------------------------------------#

class Singleton(type):
    """Singleton metaclass for creating singleton classes
    @note: class being applied to must have a SetupWindow method

    """
    def __init__(cls, name, bases, dict):
        super(Singleton, cls).__init__(name, bases, dict)
        cls.instance = None

    def __call__(cls, *args, **kw):
        if not cls.instance:
            # Not created or has been Destroyed
            obj = super(Singleton, cls).__call__(*args, **kw)
            cls.instance = obj

        return cls.instance

#-----------------------------------------------------------------------------#

def MinMax(arg1, arg2):
    """Return an ordered tuple of the minimum and maximum value
    of the two args.
    @return: tuple

    """
    return min(arg1, arg2), max(arg1, arg2)
