###############################################################################
# Name: __init__.py                                                           #
# Purpose: Puts external modules in the namespace                             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################
"""External Module Package
Modules that Editra depends on that are where not developed for the project but
are distributed with it, either because of slight customizations made to them
or to reduce what is needed to be installed when installing Editra.

@note: modules in this directory are dependancies and addons that are not
       part of the core code.

"""
__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: __init__.py 52855 2008-03-27 14:53:06Z CJP $"
__revision__ = "$Revision: 52855 $"

__all__ = ['ez_setup', 'pkg_resources', 'events', 'flatnotebook']

