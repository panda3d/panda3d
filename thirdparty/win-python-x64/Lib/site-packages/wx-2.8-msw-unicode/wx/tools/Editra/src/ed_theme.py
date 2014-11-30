###############################################################################
# Name: ed_theme.py                                                           #
# Purpose: Icon theme management for Editra                                   #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Provide an interface for creating icon themes for Editra. This will allow for
themes to be created, installed, and managed as plugins, which means that they
can be installed as single file instead of dozens of individual image files.

@summary: Editra's theme interface and implementation

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_theme.py 66815 2011-01-29 20:46:20Z CJP $"
__revision__ = "$Revision: 66815 $"

#--------------------------------------------------------------------------#
# Imports
import os
import wx

# Local Imports
import ed_glob
import util
import plugin
from profiler import Profile_Get, Profile_Set
import syntax.synglob as synglob
from syntax.syntax import SYNTAX_IDS

#--------------------------------------------------------------------------#

class ThemeI(plugin.Interface):
    """Interface for defining an icon theme in Editra
    When a icon theme is active Editra's ArtProvider will ask the active
    theme that implements this interface to give it a bitmap. The requests
    for bitmaps will be numerical ID values that come from ed_glob. These
    ID's are associated with different objects in the interface. The names
    are descriptive of the object they represent, for reference however
    see the implementation of the two main themes (Tango and Nuovo).

    @see: L{ed_glob}
    @see: L{syntax.synglob}

    """
    def GetName(self):
        """Return the name of this theme. This is used to identify the
        theme when the provider looks for resources based on user preferences

        @return: name string

        """

    def GetMenuBitmap(self, bmp_id):
        """Get the menu bitmap associated with the object id
        If this theme does not have a resource to provide for this
        object return a wx.NullBitmap.

        @return: 16x16 pixel bitmap

        """
        return wx.NullBitmap

    def GetFileBitmap(self, bmp_id):
        """Get the filetype bitmap associated with the object id, the
        valid object ids are defined in the module syntax.synglob and
        are used to request images for menu's and page tabs. The theme
        implimenting this interface should at least be able to
        provide an image for plain text files and return that for any
        unmapped types.

        If this theme does not have a resource to provide for this
        object return a wx.NullBitmap.

        @return: 16x16 pixel bitmap

        """
        return wx.NullBitmap

    def GetOtherBitmap(self, bmp_id):
        """Get the bitmap from the 'other' icon resources. Valid id's are
        identified by a mapping in the ART dictionary.
        
        If this theme does not have a resource to provide for this
        object return a wx.NullBitmap.

        @return: wx.Bitmap

        """
        return wx.NullBitmap

    def GetToolbarBitmap(self, bmp_id):
        """Get the toolbar bitmap associated with the object id. The
        toolbar icons must be returned as a 32x32 pixel bitmap any
        scaling that is needed will be handled by the art provider that
        requests the resource.

        If this theme does not have a resource to provide for this
        object return a wx.NullBitmap.

        @return: 32x32 pixel bitmap

        """
        return wx.NullBitmap

#-----------------------------------------------------------------------------#

class BitmapProvider(plugin.Plugin):
    """Plugin that fetches requested icons from the current active theme.

    """
    observers = plugin.ExtensionPoint(ThemeI)

    def __GetCurrentProvider(self):
        """Gets the provider of the current theme resources
        @return: ThemeI object

        """
        theme = Profile_Get('ICONS', 'str', u'')
        for prov in self.observers:
            if theme == prov.GetName():
                return prov

        # Case if a theme was deleted while it was the active theme
        if theme.lower() != u'default':
            Profile_Set('ICONS', u'Default')

        return None

    def _GetTango(self, bmp_id, client):
        """Try to get the icon from the default tango theme"""
        theme = None
        bmp = wx.NullBitmap
        for prov in self.observers:
            if prov.GetName() == TangoTheme.name:
                theme = prov
                break
        else:
            return bmp

        if client == wx.ART_TOOLBAR:
            bmp = theme.GetToolbarBitmap(bmp_id)
        elif client == wx.ART_MENU:
            bmp = theme.GetMenuBitmap(bmp_id)
        elif client == wx.ART_OTHER:
            bmp = theme.GetOtherBitmap(bmp_id)
        else:
            pass

        return bmp

    def GetThemes(self):
        """Gets a list of the installed and activated themes
        @return: list of strings

        """
        return [ name.GetName() for name in self.observers ]

    def GetBitmap(self, bmp_id, client):
        """Gets a 16x16 or 32x32 pixel bitmap depending on client value.
        May return a NullBitmap if no suitable bitmap can be
        found.

        @param bmp_id: id of bitmap to lookup
        @param client: wxART_MENU, wxART_TOOLBAR
        @see: L{ed_glob}

        """
        prov = self.__GetCurrentProvider()
        if prov is not None:
            if client == wx.ART_MENU:
                bmp = prov.GetMenuBitmap(bmp_id)
            elif client == wx.ART_OTHER:
                # Backwards compatibility for older interface
                if hasattr(prov, 'GetOtherBitmap'):
                    bmp = prov.GetOtherBitmap(bmp_id)
                else:
                    bmp = wx.NullBitmap
            else:
                bmp = prov.GetToolbarBitmap(bmp_id)

            if bmp.IsOk():
                return bmp

        # Try to fallback to tango theme when icon lookup fails
        bmp = self._GetTango(bmp_id, client)
        if bmp.IsOk():
            return bmp

        return wx.NullBitmap

#-----------------------------------------------------------------------------#
# Default theme data maps
ART = { ed_glob.ID_ABOUT  : u'about.png',
        ed_glob.ID_ADD    : u'add.png',
        ed_glob.ID_ADD_BM : u'bmark_add.png',
        ed_glob.ID_ADVANCED : u'advanced.png',
        ed_glob.ID_BACKWARD : u'backward.png',
        ed_glob.ID_BIN_FILE : u'bin_file.png',
        ed_glob.ID_CDROM  : u'cdrom.png',
        ed_glob.ID_CONTACT : u'mail.png',
        ed_glob.ID_COPY   : u'copy.png',
        ed_glob.ID_COMPUTER : u'computer.png',
        ed_glob.ID_CUT    : u'cut.png',
        ed_glob.ID_DELETE : u'delete.png',
        ed_glob.ID_DOCPROP : u'doc_props.png',
        ed_glob.ID_DOCUMENTATION : u'docs.png',
        ed_glob.ID_DOWN   : u'down.png',
        ed_glob.ID_EXIT   : u'quit.png',
        ed_glob.ID_FILE   : u'file.png',
        ed_glob.ID_FIND   : u'find.png',
        ed_glob.ID_FIND_REPLACE : u'findr.png',
        ed_glob.ID_FIND_RESULTS : u'find.png',
        ed_glob.ID_FLOPPY : u'floppy.png',
        ed_glob.ID_FOLDER : u'folder.png',
        ed_glob.ID_FONT   : u'font.png',
        ed_glob.ID_FORWARD : u'forward.png',
        ed_glob.ID_HARDDISK : u'harddisk.png',
        ed_glob.ID_HOMEPAGE : u'web.png',
        ed_glob.ID_HTML_GEN : u'html_gen.png',
        ed_glob.ID_INDENT : u'indent.png',
        ed_glob.ID_LOGGER : u'log.png',
        ed_glob.ID_NEW    : u'new.png',
        ed_glob.ID_NEW_WINDOW: u'newwin.png',
        ed_glob.ID_NEXT_MARK : u'bmark_next.png',
        ed_glob.ID_NEXT_POS : u'forward.png',
        ed_glob.ID_OPEN    : u'open.png',
        ed_glob.ID_PACKAGE : u'package.png',
        ed_glob.ID_PASTE   : u'paste.png',
        ed_glob.ID_PLUGMGR : u'plugin.png',
        ed_glob.ID_PRE_MARK : u'bmark_pre.png',
        ed_glob.ID_PRE_POS : u'backward.png',
        ed_glob.ID_PREF    : u'pref.png',
        ed_glob.ID_PRINT   : u'print.png',
        ed_glob.ID_PRINT_PRE : u'printpre.png',
        ed_glob.ID_PYSHELL : u'pyshell.png',
        ed_glob.ID_REDO    : u'redo.png',
        ed_glob.ID_REFRESH : u'refresh.png',
        ed_glob.ID_REMOVE  : u'remove.png',
        ed_glob.ID_RTF_GEN : u'rtf_gen.png',
        ed_glob.ID_SAVE    : u'save.png',
        ed_glob.ID_SAVEALL : u'saveall.png',
        ed_glob.ID_SAVEAS  : u'saveas.png',
        ed_glob.ID_SELECTALL : u'selectall.png',
        ed_glob.ID_STOP    : u'stop.png',
        ed_glob.ID_STYLE_EDIT : u'style_edit.png',
        ed_glob.ID_TEX_GEN : u'tex_gen.png',
        ed_glob.ID_THEME  : u'theme.png',
        ed_glob.ID_UNDO   : u'undo.png',
        ed_glob.ID_UNINDENT : u'outdent.png',
        ed_glob.ID_UP     : u'up.png',
        ed_glob.ID_USB    : u'usb.png',
        ed_glob.ID_WEB    : u'web.png',
        ed_glob.ID_ZOOM_IN : u'zoomi.png',
        ed_glob.ID_ZOOM_OUT : u'zoomo.png',
        ed_glob.ID_ZOOM_NORMAL : u'zoomd.png',
        ed_glob.ID_READONLY : u'readonly.png',

        # code elements
        ed_glob.ID_CLASS_TYPE : u'class.png',
        ed_glob.ID_FUNCT_TYPE : u'function.png',
        ed_glob.ID_ELEM_TYPE : u'element.png',
        ed_glob.ID_VARIABLE_TYPE : u'variable.png',
        ed_glob.ID_ATTR_TYPE : u'attribute.png',
        ed_glob.ID_PROPERTY_TYPE : u'property.png',
        ed_glob.ID_METHOD_TYPE : u'method.png'
}

# File Type Art
MIME_ART = { synglob.ID_LANG_ADA : u'ada.png',
             synglob.ID_LANG_BASH : u'shell.png',
             synglob.ID_LANG_BOO : u'boo.png',
             synglob.ID_LANG_BOURNE : u'shell.png',
             synglob.ID_LANG_C : u'c.png',
             synglob.ID_LANG_CPP : u'cpp.png',
             synglob.ID_LANG_CSH : u'shell.png',
             synglob.ID_LANG_CSS : u'css.png',
             synglob.ID_LANG_DIFF : u'diff.png',
             synglob.ID_LANG_HTML : u'html.png',
             synglob.ID_LANG_JAVA : u'java.png',
             synglob.ID_LANG_KSH : u'shell.png',
             synglob.ID_LANG_LATEX : u'tex.png',
             synglob.ID_LANG_MAKE : u'makefile.png',
             synglob.ID_LANG_PASCAL : u'pascal.png',
             synglob.ID_LANG_PERL : u'perl.png',
             synglob.ID_LANG_PHP : u'php.png',
             synglob.ID_LANG_PS : u'postscript.png',
             synglob.ID_LANG_PYTHON : u'python.png',
             synglob.ID_LANG_RUBY : u'ruby.png',
             synglob.ID_LANG_TCL : u'tcl.png',
             synglob.ID_LANG_TEX : u'tex.png',
             synglob.ID_LANG_TXT : u'text.png',
             synglob.ID_LANG_XML : u'xml.png'
 }

#-----------------------------------------------------------------------------#

class TangoTheme(plugin.Plugin):
    """Represents the Tango Icon theme for Editra"""
    plugin.Implements(ThemeI)

    name = u'Tango'

    def __GetArtPath(self, client, mime=False):
        """Gets the path of the resource directory to get
        the bitmaps from.
        @param client: wx.ART_MENU/wx.ART_TOOLBAR
        @keyword mime: is this a filetype icon lookup
        @return: path of art resource
        @rtype: string

        """
        clients = { wx.ART_MENU : u"menu",
                    wx.ART_TOOLBAR : u"toolbar",
                    wx.ART_OTHER : u"other" }

        # Get the path
        if ed_glob.CONFIG['THEME_DIR'] == u'':
            theme = util.ResolvConfigDir(os.path.join(u"pixmaps", u"theme"))
            ed_glob.CONFIG['THEME_DIR'] = theme

        if mime:
            path = os.path.join(ed_glob.CONFIG['THEME_DIR'],
                                Profile_Get('ICONS'), u'mime')
        else:
            path = os.path.join(ed_glob.CONFIG['THEME_DIR'],
                                self.GetName(),
                                clients.get(client, u"menu"))

        path += os.sep
        if os.path.exists(path):
            return path
        else:
            return None

    def GetName(self):
        """Get the name of this theme
        @return: string

        """
        return TangoTheme.name

    def GetMenuBitmap(self, bmp_id):
        """Get a menu bitmap
        @param bmp_id: Id of bitmap to look for

        """
        if bmp_id in ART:
            path = self.__GetArtPath(wx.ART_MENU, mime=False)
            if path is not None:
                path = path + ART[bmp_id]
                if os.path.exists(path):
                    return wx.Bitmap(path, wx.BITMAP_TYPE_PNG)
        else:
            return self.GetFileBitmap(bmp_id)

        return wx.NullBitmap

    def GetFileBitmap(self, bmp_id):
        """Get a mime type bitmap from the theme
        @param bmp_id: Id of filetype bitmap to look up
        @see: L{syntax.synglob}

        """
        path = self.__GetArtPath(wx.ART_MENU, mime=True)
        if path is not None and bmp_id in SYNTAX_IDS:
            if bmp_id in MIME_ART:
                req = path + MIME_ART[bmp_id]
                if os.path.exists(req):
                    return wx.Bitmap(req, wx.BITMAP_TYPE_PNG)

            # Try to fall back to bmp for plain text when above is not found
            bkup = path + MIME_ART[synglob.ID_LANG_TXT]
            if os.path.exists(bkup):
                return wx.Bitmap(bkup, wx.BITMAP_TYPE_PNG)

        return wx.NullBitmap

    def GetOtherBitmap(self, bmp_id):
        """Get a other catagory bitmap.
        @param bmp_id: Id of art resource

        """
        if bmp_id in ART:
            path = self.__GetArtPath(wx.ART_OTHER, mime=False)
            if path is not None:
                path = path + ART[bmp_id]
                if os.path.exists(path):
                    return wx.Bitmap(path, wx.BITMAP_TYPE_PNG)

        return wx.NullBitmap

    def GetToolbarBitmap(self, bmp_id):
        """Get a toolbar bitmap
        @param bmp_id: Id of bitmap to look for
        @return: wx.NullBitmap or a 32x32 bitmap

        """
        if bmp_id in ART:
#            size = Profile_Get('ICON_SZ', default=(24, 24))
            path = self.__GetArtPath(wx.ART_TOOLBAR, mime=False)
            if path is not None:
#                tpath = os.path.join(path, '24', ART[bmp_id])
#                if size[0] == 24 and os.path.exists(tpath):
#                    path = tpath
#                else:
                path = path + ART[bmp_id]

                if os.path.exists(path):
                    return wx.Bitmap(path, wx.BITMAP_TYPE_PNG)

        return wx.NullBitmap
