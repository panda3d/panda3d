###############################################################################
# Name: ed_thread.py                                                          #
# Purpose: Provides a base class for managing XML files and data.             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2011 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
XML base class

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id:  $"
__revision__ = "$Revision:  $"

#-----------------------------------------------------------------------------#
# Imports
import types
from xml.dom import minidom
import extern.dexml as dexml
from extern.dexml.fields import *

#-----------------------------------------------------------------------------#

class EdXml(dexml.Model):
    """XML base class"""
    def __init__(self, **kwds):
        super(EdXml, self).__init__(**kwds)

    Xml = property(lambda self: self.GetXml(),
                   lambda self, xstr: self.parse(xstr))
    PrettyXml = property(lambda self: self.GetPrettyXml(),
                         lambda self, xstr: self.parse(xstr))

    def GetPrettyXml(self):
        """Get a nicely formatted version of the rendered xml string
        @return: string

        """
        txt = self.render()
        txt = minidom.parseString(txt).toprettyxml()
        txt = txt.replace('\t', '   ') # DeTabify
        return txt

    def GetXml(self):
        """Get the XML string for this object
        @return: string

        """
        return self.render()

    def Write(self, path):
        """Write the xml to a file
        @param path: string
        @return: success (bool)

        """
        suceeded = True
        try:
            xmlstr = self.PrettyXml
            if isinstance(xmlstr, types.UnicodeType):
                xmlstr = xmlstr.encode('utf-8')
            handle = open(path, 'wb')
            handle.write(xmlstr)
            handle.close()
        except (IOError, OSError, UnicodeEncodeError):
            suceeded = False
        return suceeded

    @classmethod
    def Load(cls, path):
        """Load this object from a file
        @return: instance

        """
        instance = None
        try:
            handle = open(path, 'rb')
            xmlstr = handle.read()
            handle.close()
            instance = cls.parse(xmlstr)
        except (IOError, OSError):
            instance = None
        return instance
