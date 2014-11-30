###############################################################################
# Name: ed_style.py                                                           #
# Purpose: Editra's style management system. Implements the interpretation of #
#          Editra Style Sheets to the StyledTextCtrl.                         #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008-2011 Cody Precord <staff@editra.org>                    #
# License: wxWindows License                                                  #
###############################################################################

"""
Provides a system for managing styles in the text control. Compiles the data
in an Editra Style Sheet to a format that Scintilla can understand. The
specification of Editra Style Sheets that this module implements can be found
either in the _docs_ folder of the source distribution or on Editra's home page
U{http://editra.org/editra_style_sheets}.

@summary: Style management system for managing the syntax highlighting of all
          buffers

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_style.py 67815 2011-05-31 19:08:47Z CJP $"
__revision__ = "$Revision: 67815 $"

#--------------------------------------------------------------------------#
# Imports
import os
import re
import wx

# Editra Libraries
import ed_glob
import util
from profiler import Profile_Get, Profile_Set
import eclib

# Globals
STY_ATTRIBUTES     = (u"face", u"fore", u"back", u"size", u"modifiers")
STY_EX_ATTRIBUTES  = (u"eol", u"bold", u"italic", u"underline")

# Parser Values
RE_ESS_COMMENT = re.compile("\/\*[^*]*\*+([^/][^*]*\*+)*\/")
RE_ESS_SCALAR = re.compile("\%\([a-zA-Z0-9]+\)")
RE_HEX_STR = re.compile("#[0-9a-fA-F]{3,6}")

#--------------------------------------------------------------------------#

class StyleItem(object):
    """A storage class for holding styling information """
    __slots__ = ('null', 'fore', 'face', 'back', 'size', '_exattr')
    def __init__(self, fore=u"", back=u"", face=u"", size=u"", ex=None):
        """Initializes the Style Object.

        @keyword fore: Specifies the foreground color (hex string)
        @keyword face: Specifies the font face (string face name)
        @keyword back: Specifies the background color (hex string)
        @keyword size: Specifies font point size (int/formatted string)
        @keyword ex: Specify modifiers

        SPECIFICATION:
          - DATA FORMATS:
            - #123456       = hex color code
            - Monaco        = Font Face Name
            - %(primary)s   = Format string to be swapped at runtime
            - 10            = A font point size
            - %(size)s      = Format string to be swapped at runtime
            - ex            = bold underline italic eol

        """
        super(StyleItem, self).__init__()

        if ex is None:
            ex = list()
            
        # Attributes
        self.null = False
        self.fore = fore     # Foreground color hex code
        self.face = face     # Font face name
        self.back = back     # Background color hex code
        self.size = size     # Font point size
        self._exattr = ex    # Extra attributes

    def __eq__(self, other):
        """Defines the == operator for the StyleItem Class
        @param si2: style item to compare to
        @return: whether the two items are equal
        @rtype: bool

        """
        return unicode(self) == unicode(other)

    def __ne__(self, other):
        """Defines != operator for the StyleItem Class"""
        return unicode(self) != unicode(other)

    def __str__(self):
        """Convert StyleItem to string"""
        uni = unicode(self)
        return uni.encode('utf-8')

    def __unicode__(self):
        """Converts StyleItem to Unicode
        @note: This return string is in a format that can be accepted by
               Scintilla. No spaces may be in the string after the ':'.
        @return: Unicode representation of the StyleItem

        """
        style_str = list()
        if self.fore:
            style_str.append(u"fore:%s" % self.fore)
        if self.back:
            style_str.append(u"back:%s" % self.back)
        if self.face:
            style_str.append(u"face:%s" % self.face)
        if self.size:
            style_str.append(u"size:%s" % unicode(self.size))
        if len(self._exattr):
            style_str.append(u"modifiers:" +  u','.join(self._exattr))

        style_str = u",".join(style_str)
        return style_str.rstrip(u",")

    def Clone(self):
        """Make and return a copy of this object"""
        nitem = StyleItem(self.fore, self.back,
                          self.face, self.size,
                          self._exattr)
        if self.null:
            nitem.Nullify()
        return nitem
        
    #---- Get Functions ----#
    def GetAsList(self):
        """Returns a list of attr:value strings
        this style item.
        @return: list attribute values usable for building stc or ess values

        """
        retval = list()
        for attr in ('fore', 'back', 'face', 'size'):
            val = getattr(self, attr, None)
            if val not in ( None, wx.EmptyString ):
                retval.append(attr + ':' + val)

        if len(self._exattr):
            retval.append("modifiers:" + u",".join(self._exattr))
        return retval

    def GetBack(self):
        """Returns the value of the back attribute
        @return: style items background attribute

        """
        return self.back

    def GetFace(self):
        """Returns the value of the face attribute
        @return: style items font face attribute

        """
        return self.face

    def GetFore(self):
        """Returns the value of the fore attribute
        @return: style items foreground attribute

        """
        return self.fore

    def GetSize(self):
        """Returns the value of the size attribute as a string
        @return: style items font size attribute

        """
        return self.size

    def GetModifiers(self):
        """Get the modifiers string
        @return: string

        """
        return u",".join(self.GetModifierList())

    def GetModifierList(self):
        """Get the list of modifiers
        @return: list

        """
        return self._exattr

    def GetNamedAttr(self, attr):
        """Get the value of the named attribute
        @param attr: named attribute to get value of

        """
        return getattr(self, attr, None)

    #---- Utilities ----#
    def IsNull(self):
        """Return whether the item is null or not
        @return: bool

        """
        return self.null

    def IsOk(self):
        """Check if the style item is ok or not, if it has any of its
        attributes set it is perceived as ok.
        @return: bool

        """
        return len(unicode(self))

    def Nullify(self):
        """Clear all values and set item as Null
        @postcondition: item is turned into a NullStyleItem

        """
        self.null = True
        for attr in ('fore', 'face', 'back', 'size'):
            setattr(self, attr, u'')
        self._exattr = list()

    #---- Set Functions ----#
    def SetAttrFromStr(self, style_str):
        """Takes style string and sets the objects attributes
        by parsing the string for the values. Only sets or
        overwrites values does not zero out previously set values.
        Returning True if value(s) are set or false otherwise.
        @param style_str: style information string (i.e fore:#888444)
        @type style_str: string

        """
        self.null = False
        last_set = wx.EmptyString
        for atom in style_str.split(u','):
            attrib = atom.split(u':')
            if len(attrib) == 2 and attrib[0] in STY_ATTRIBUTES:
                last_set = attrib[0]
                if last_set == u"modifiers":
                    self.SetExAttr(attrib[1])
                else:
                    setattr(self, attrib[0], attrib[1])
            else:
                for attr in attrib:
                    if attr in STY_EX_ATTRIBUTES:
                        self.SetExAttr(attr)

        return last_set != wx.EmptyString

    def SetBack(self, back, ex=wx.EmptyString):
        """Sets the Background Value
        @param back: hex color string, or None to clear attribute
        @keyword ex: extra attribute (i.e bold, italic, underline)

        """
        self.null = False
        if back is None:
            back = u''
        self.back = back
        if ex and ex not in self._exattr:
            self._exattr.append(ex)

    def SetFace(self, face, ex=wx.EmptyString):
        """Sets the Face Value
        @param face: font name string, or None to clear attribute
        @keyword ex: extra attribute (i.e bold, italic, underline)

        """
        self.null = False
        if face is None:
            face = u''
        self.face = face
        if ex and ex not in self._exattr:
            self._exattr.append(ex)

    def SetFore(self, fore, ex=wx.EmptyString):
        """Sets the Foreground Value
        @param fore: hex color string, or None to clear attribute
        @keyword ex: extra attribute (i.e bold, italic, underline)

        """
        self.null = False
        if fore is None:
            fore = u''
        self.fore = fore
        if ex and ex not in self._exattr:
            self._exattr.append(ex)

    def SetSize(self, size, ex=wx.EmptyString):
        """Sets the Font Size Value
        @param size: font point size, or None to clear attribute
        @type size: string or int
        @keyword ex: extra attribute (i.e bold, italic, underline)

        """
        self.null = False
        if size is None:
            size = u''
        self.size = unicode(size)
        if ex and ex not in self._exattr:
            self._exattr.append(ex)

    def SetExAttr(self, ex_attr, add=True):
        """Adds an extra text attribute to a StyleItem. Currently
        (bold, eol, italic, underline) are supported. If the optional
        add value is set to False the attribute will be removed from
        the StyleItem.
        @param ex_attr: extra style attribute (bold, eol, italic, underline)
        @type ex_attr: string
        @keyword add: Add a style (True) or remove a style (False)

        """
        # Get currently set attributes
        self.null = False
        if ex_attr not in STY_EX_ATTRIBUTES:
            return

        if add and ex_attr not in self._exattr:
            self._exattr.append(ex_attr)
        elif not add and ex_attr in self._exattr:
            self._exattr.remove(ex_attr)
        else:
            pass

    def SetNamedAttr(self, attr, value):
        """Sets a StyleItem attribute by named string.
        @note: This is not intended to be used for setting extra
               attributes such as bold, eol, ect..
        @param attr: a particular attribute to set (i.e fore, face, back, size)
        @param value: value to set the attribute to contain. None to clear the
                      value.

        """
        self.null = False
        if value is None:
            value = u''
        cur_val = getattr(self, attr, None)
        if cur_val is not None:
            if u"," in value:
                modifiers = value.split(u",")
                value = modifiers.pop(0)
                for ex in modifiers:
                    self.SetExAttr(ex)
            setattr(self, attr, value)

#-----------------------------------------------------------------------------#

class StyleMgr(object):
    """Manages style definitions and provides them on request.
    Also provides functionality for loading custom style sheets and
    modifying styles during run time.

    """
    STYLES         = dict()         # Static cache for loaded style set(s)
    FONT_PRIMARY   = u"primary"
    FONT_SECONDARY = u"secondary"
    FONT_SIZE      = u"size"
    FONT_SIZE2     = u"size2"
    FONT_SIZE3     = u"size3"

    def __init__(self, custom=wx.EmptyString):
        """Initializes the Style Manager
        @keyword custom: path to custom style sheet to use

        """
        super(StyleMgr, self).__init__()

        # Attributes
        self.fonts = self.GetFontDictionary()
        self.style_set = custom
        self.syntax_set = list()
        self.LOG = wx.GetApp().GetLog()

        # Get the Style Set
        if custom != wx.EmptyString and self.LoadStyleSheet(custom):
            self.LOG("[ed_style][info] Loaded custom style sheet %s" % custom)
        elif custom == wx.EmptyString:
            self.SetStyles('default', DEF_STYLE_DICT)
        else:
            self.LOG("[ed_style][err] Failed to import styles from %s" % custom)

    def BlankStyleDictionary(self):
        """Returns a dictionary of unset style items based on the
        tags defined in the current dictionary.
        @return: dictionary of unset style items using the current tag set
                 as keys.

        """
        sty_dict = dict()
        for key in DEF_STYLE_DICT.keys():
            if key in ('select_style',): # special styles
                sty_dict[key] = NullStyleItem()
            else:
                sty_dict[key] = StyleItem("#000000", "#FFFFFF",
                                          "%(primary)s", "%(size)d")
        return sty_dict

    def FindTagById(self, style_id):
        """Find the style tag that is associated with the given
        Id. Return value defaults to default_style .
        @param style_id: id of tag to look for
        @return: style tag string

        """
        for data in self.syntax_set:
            if style_id == data[0]:
                return data[1]
        return 'default_style'

    def GetFontDictionary(self, default=True):
        """Does a system lookup to build a default set of fonts using
        ten point fonts as the standard size.
        @keyword default: return the default dictionary of fonts, else return
                          the current running dictionary of fonts if it exists.
        @type default: bool
        @return: font dictionary (primary, secondary) + (size, size2)

        """
        if hasattr(self, 'fonts') and not default:
            return self.fonts

        font = Profile_Get('FONT1', 'font', None)
        if font is not None:
            mfont = font
        else:
            mfont = wx.Font(10, wx.FONTFAMILY_MODERN, wx.FONTSTYLE_NORMAL,
                            wx.FONTWEIGHT_NORMAL)
            Profile_Set('FONT1', mfont, 'font')
        primary = mfont.GetFaceName()

        font = Profile_Get('FONT2', 'font', None)
        if font is None:
            font = wx.Font(10, wx.FONTFAMILY_SWISS, wx.FONTSTYLE_NORMAL,
                            wx.FONTWEIGHT_NORMAL)
            Profile_Set('FONT2', font, 'font')
        secondary = font.GetFaceName()
        faces = {
                  self.FONT_PRIMARY   : primary,
                  self.FONT_SECONDARY : secondary,
                  self.FONT_SIZE  : mfont.GetPointSize(),
                  self.FONT_SIZE2 : font.GetPointSize(),
                  self.FONT_SIZE3 : mfont.GetPointSize() - 2
                 }
        return faces

    def GetDefaultFont(self):
        """Constructs and returns a wxFont object from the settings
        of the default_style object.
        @return: font object of default style
        @rtype: wx.Font

        """
        if self.HasNamedStyle('default_style'):
            style_item = self.GetItemByName('default_style')
            face = style_item.GetFace()
            if face[0] == u"%":
                face = face % self.fonts
            size = style_item.GetSize()
            if isinstance(size, basestring):
                size = size % self.fonts
            font = wx.FFont(int(size), wx.MODERN, face=face)
        else:
            font = wx.FFont(self.fonts[self.FONT_SIZE], wx.MODERN)
        return font

    def GetDefaultForeColour(self, as_hex=False):
        """Gets the foreground color of the default style and returns
        a Colour object. Otherwise returns Black if the default
        style is not found.
        @keyword as_hex: return a hex string or colour object
        @type as_hex: bool
        @return: wx.Colour of default style foreground or hex value
        @rtype: wx.Colour or string

        """
        fore = self.GetItemByName('default_style').GetFore()
        if not fore:
            fore = u"#000000"

        if not as_hex:
            rgb = eclib.HexToRGB(fore[1:])
            fore = wx.Colour(red=rgb[0], green=rgb[1], blue=rgb[2])
        return fore

    def GetCurrentStyleSetName(self):
        """Get the name of the currently set style
        @return: string

        """
        return self.style_set

    def GetDefaultBackColour(self, as_hex=False):
        """Gets the background color of the default style and returns
        a Colour object. Otherwise returns white if the default
        style is not found.
        @keyword hex: return a hex string or colour object
        @type hex: bool
        @return: wx.Colour of default style background or hex value
        @rtype: wx.Colour or string

        """
        back = self.GetItemByName('default_style').GetBack()
        if not back:
            back = u"#FFFFFF"

        if not as_hex:
            rgb = eclib.HexToRGB(back[1:])
            back = wx.Colour(red=rgb[0], green=rgb[1], blue=rgb[2])
        return back

    def GetItemByName(self, name):
        """Gets and returns a style item using its name for the search
        @param name: tag name of style item to get
        @return: style item (may be empty/null style item)
        @rtype: L{StyleItem}

        """
        scheme = self.GetStyleSet()
        if name in scheme:
            item = scheme[name]

            # Set font value if need be
            ival = unicode(item)
            if u"%" in ival:
                val = ival % self.fonts
                item = StyleItem()
                item.SetAttrFromStr(val)

            return item
        else:
            return StyleItem()

    def GetStyleFont(self, primary=True):
        """Returns the primary font facename by default
        @keyword primary: Get Primary(default) or Secondary Font
        @return face name of current font in use

        """
        if primary:
            font = wx.FFont(self.fonts[self.FONT_SIZE], wx.DEFAULT,
                            face=self.fonts[self.FONT_PRIMARY])
        else:
            font = wx.FFont(self.fonts[self.FONT_SIZE2], wx.DEFAULT,
                            face=self.fonts[self.FONT_SECONDARY])
        return font

    def GetStyleByName(self, name):
        """Gets and returns a style string using its name for the search
        @param name: tag name of style to get
        @type name: string
        @return: style item in string form
        @rtype: string

        """
        if self.HasNamedStyle(name):
            stystr = unicode(self.GetItemByName(name))
            return stystr.replace("modifiers:", "")
        else:
            return u""

    def GetStyleSet(self):
        """Returns the current set of styles or the default set if
        there is no current set.
        @return: current style set dictionary
        @rtype: dict

        """
        return StyleMgr.STYLES.get(self.style_set, DEF_STYLE_DICT)

    @staticmethod
    def GetStyleSheet(sheet_name=None):
        """Finds the current style sheet and returns its path. The
        lookup is done by first looking in the users config directory
        and if it is not found there it looks for one on the system
        level and if that fails it returns None.
        @param sheet_name: style sheet to look for
        @return: full path to style sheet

        """
        if sheet_name:
            style = sheet_name
            if sheet_name.split(u'.')[-1] != u"ess":
                style += u".ess"
        elif Profile_Get('SYNTHEME', 'str').split(u'.')[-1] != u"ess":
            style = (Profile_Get('SYNTHEME', 'str') + u".ess").lower()
        else:
            style = Profile_Get('SYNTHEME', 'str').lower()

        # Get Correct Filename if it exists
        for sheet in util.GetResourceFiles(u'styles', False, True, title=False):
            if sheet.lower() == style.lower():
                style = sheet
                break

        user = os.path.join(ed_glob.CONFIG['STYLES_DIR'], style)
        sysp = os.path.join(ed_glob.CONFIG['SYS_STYLES_DIR'], style)
        if os.path.exists(user):
            return user
        elif os.path.exists(sysp):
            return sysp
        else:
            return None

    def GetSyntaxParams(self):
        """Get the set of syntax parameters
        @return: list

        """
        return self.syntax_set

    def HasNamedStyle(self, name):
        """Checks if a style has been set/loaded or not
        @param name: tag name of style to look for
        @return: whether item is in style set or not

        """
        return name in self.GetStyleSet()

    def LoadStyleSheet(self, style_sheet, force=False):
        """Loads a custom style sheet and returns True on success
        @param style_sheet: path to style sheet to load
        @keyword force: Force re-parse of style sheet, default is to use cached
                        data when available
        @return: whether style sheet was loaded or not
        @rtype: bool

        """
        if isinstance(style_sheet, basestring) and \
           os.path.exists(style_sheet) and \
           ((force or style_sheet not in StyleMgr.STYLES) or \
             style_sheet != self.style_set):
            reader = util.GetFileReader(style_sheet)
            if reader == -1:
                self.LOG("[ed_style][err] Failed to open style sheet: %s" % style_sheet)
                return False
            style_data = None
            try:
                style_data = self.ParseStyleData(reader.read())
            except Exception, msg:
                self.LOG("[ed_style][err] Failed to parse style data for %s:" % style_sheet)
                return False
            ret_val = self.SetStyles(style_sheet, style_data)
            reader.close()
            return ret_val
        elif style_sheet not in StyleMgr.STYLES:
            self.LOG("[ed_style][warn] Style sheet %s does not exists" % style_sheet)
            # Reset to default style
            if Profile_Get('SYNTHEME') != 'default':
                Profile_Set('SYNTHEME', 'default')
                self.SetStyles('default', DEF_STYLE_DICT)
            return False
        else:
            self.LOG("[ed_style][info] Using cached style data")
            return True

    def PackStyleSet(self, style_set):
        """Checks the difference of each item in the style set as
        compared to the default_style tag and packs any unset value
        in the item to be equal to the default style.
        @param style_set: style set to pack
        @return: style_set with all unset attributes set to match default style

        """
        if isinstance(style_set, dict) and 'default_style' in style_set:
            default = style_set['default_style']
            for tag in style_set:
                if style_set[tag].IsNull():
                    continue
                if not style_set[tag].GetFace():
                    style_set[tag].SetFace(default.GetFace())
                if not style_set[tag].GetFore():
                    style_set[tag].SetFore(default.GetFore())
                if not style_set[tag].GetBack():
                    style_set[tag].SetBack(default.GetBack())
                if not style_set[tag].GetSize():
                    style_set[tag].SetSize(default.GetSize())

            # Now need to pack in undefined styles that are part of
            # the standard set.
            for tag in DEF_STYLE_DICT.keys():
                if tag not in style_set:
                    if tag == 'select_style':
                        style_set[tag] = NullStyleItem()
                    else:
                        style_set[tag] = default.Clone()
        else:
            pass
        return style_set

    def ParseStyleData(self, style_data):
        """Parses a string style definitions read from an Editra Style Sheet.
        @param style_data: style sheet data string
        @type style_data: string
        @return: dictionary of StyleItems constructed from the style sheet
                 data.

        """
        # Remove all comments
        style_data = RE_ESS_COMMENT.sub(u'', style_data)

        # Compact data into a contiguous string
        style_data = style_data.replace(u"\r\n", u"").replace(u"\n", u"")
        style_data = style_data.replace(u"\t", u"")
#        style_data = style_data.replace(u" ", u"") # support old style

        ## Build style data tree
        # Tree Level 1 split tag from data
        style_tree = [style.split(u"{") for style in style_data.split(u'}')]
        if len(style_tree) and len(style_tree[-1]) and not style_tree[-1][0]:
            style_tree.pop()

        # Tree Level 2 Build small trees of tag and style attributes
        # Tree Level 3 Branch tree into TAG => Attr => Value String
        ttree = list(style_tree)
        for branch in ttree:
            # Check for level 1 syntax errors
            if len(branch) != 2:
                self.LOG("[ed_style][err] There was an error parsing "
                         "the syntax data from " + self.style_set)
                self.LOG("[ed_style][err] Missing a { or } in Def: " + repr(branch[0]))
                ttree.remove(branch)
                continue

            tmp2 = [leaf.strip().split(u":")
                    for leaf in branch[1].strip().split(u";")]
            if len(tmp2) and not tmp2[-1][0]:
                tmp2.pop()
            branch[1] = tmp2
        style_tree = ttree

        # Check for L2/L3 Syntax errors and build a clean dictionary
        # of Tags => Valid Attributes
        style_dict = dict()
        for branch in style_tree:
            value = list()
            tag = branch[0].replace(u" ", u"")
            for leaf in branch[1]:
                # Remove any remaining whitespace
                leaf = [part.strip() for part in leaf]
                if len(leaf) != 2:
                    self.LOG("[ed_style][err] Missing a : or ; in the "
                             "declaration of %s" % tag)
                elif leaf[0] not in STY_ATTRIBUTES:
                    self.LOG(("[ed_style][warn] Unknown style attribute: %s"
                             ", In declaration of %s") % (leaf[0], tag))
                else:
                    value.append(leaf)

            # Skip all leafless branches
            if len(value) != 0:
                style_dict[tag] = value

        # Validate leaf values and format into style string
        rdict = dict()
        for style_def in style_dict:
            if not style_def[0][0].isalpha():
                self.LOG("[ed_style][err] The style def %s is not a "
                         "valid name" % style_def[0])
            else:
                style_str = u""
                # Check each definition and validate its items
                for attrib in style_dict[style_def]:
                    values = [ val for val in attrib[1].split()
                               if val != u"" ]

                    v1ok = v2ok = False
                    # Check that colors are a hex string
                    n_values = len(values)
                    if n_values and \
                       attrib[0] in "fore back" and RE_HEX_STR.match(values[0]):
                        v1ok = True
                    elif n_values and attrib[0] == "size":
                        if RE_ESS_SCALAR.match(values[0]) or values[0].isdigit():
                            v1ok = True
                        else:
                            self.LOG("[ed_style][warn] Bad value in %s"
                                     " the value %s is invalid." % \
                                     (attrib[0], values[0]))
                    elif n_values and attrib[0] == "face":
                        # Font names may have spaces in them so join the
                        # name of the font into one item.
                        if n_values > 1 and values[1] not in STY_EX_ATTRIBUTES:
                            tmp = list()
                            for val in list(values):
                                if val not in STY_EX_ATTRIBUTES:
                                    tmp.append(val)
                                    values.remove(val)
                                else:
                                    break
                            values = [u' '.join(tmp),] + values
                        v1ok = True
                    elif n_values and attrib[0] == "modifiers":
                        v1ok = True

                    # Check extra attributes
                    if len(values) > 1:
                        for value in values[1:]:
                            if value not in STY_EX_ATTRIBUTES:
                                self.LOG("[ed_style][warn] Unknown extra " + \
                                         "attribute '" + values[1] + \
                                         "' in attribute: " + attrib[0])
                                break
                            else:
                                v2ok = True

                    if v1ok and v2ok:
                        value = u",".join(values)
                    elif v1ok:
                        value = values[0]
                    else:
                        continue

                    style_str = u",".join([style_str,
                                           u":".join([attrib[0], value])])

                # Build up the StyleItem Dictionary
                if style_str != u"":
                    new_item = StyleItem()
                    value = style_str.strip(u",")
                    if isinstance(value, basestring):
                        new_item.SetAttrFromStr(value)
                    rdict[style_def] = new_item

        return rdict

    def SetGlobalFont(self, fonttag, fontface, size=-1):
        """Sets one of the fonts in the global font set by tag
        and sets it to the named font. Returns true on success.
        @param fonttag: font type identifier key
        @param fontface: face name to set global font to

        """
        if hasattr(self, 'fonts'):
            self.fonts[fonttag] = fontface
            if size > 0:
                self.fonts[self.FONT_SIZE] = size
            return True
        else:
            return False

    def SetStyleFont(self, wx_font, primary=True):
        """Sets the\primary or secondary font and their respective
        size values.
        @param wx_font: font object to set styles font info from
        @keyword primary: Set primary(default) or secondary font

        """
        if primary:
            self.fonts[self.FONT_PRIMARY] = wx_font.GetFaceName()
            self.fonts[self.FONT_SIZE] = wx_font.GetPointSize()
        else:
            self.fonts[self.FONT_SECONDARY] = wx_font.GetFaceName()
            self.fonts[self.FONT_SIZE2] = wx_font.GetPointSize()

    def SetStyleTag(self, style_tag, value):
        """Sets the value of style tag by name
        @param style_tag: desired tag name of style definition
        @param value: style item to set tag to
        @return: bool

        """
        if not isinstance(value, StyleItem):
            self.LOG("[ed_style][warn] Bad data in SetStyleTag(%s)" % repr(value))
            return False

        StyleMgr.STYLES[self.style_set][style_tag] = value
        return True

    def SetStyles(self, name, style_dict, nomerge=False):
        """Sets the managers style data and returns True on success.
        @param name: name to store dictionary in cache under
        @param style_dict: dictionary of style items to use as managers style
                           set.
        @keyword nomerge: merge against default set or not
        @type nomerge: bool

        """
        if nomerge:
            self.style_set = name
            StyleMgr.STYLES[name] = self.PackStyleSet(style_dict)
            return True

        # Merge the given style set with the default set to fill in any
        # unset attributes/tags
        if isinstance(style_dict, dict):
            # Check for bad data
            for style in style_dict.values():
                if not isinstance(style, StyleItem):
                    self.LOG("[ed_style][err] Invalid data in style dictionary")
                    self.style_set = 'default'
                    return False

            self.style_set = name
            defaultd = DEF_STYLE_DICT
            dstyle = style_dict.get('default_style', None)
            if dstyle is None:
                self.LOG("[ed_style][warn] default_style is undefined")
                style_dict['default_style'] = defaultd['default_style'].Clone()

            # Set any undefined styles to match the default_style
            for tag in defaultd:
                if tag not in style_dict:
                    if tag in ('select_style',):
                        style_dict[tag] = NullStyleItem()
                    else:
                        style_dict[tag] = style_dict['default_style'].Clone()

            StyleMgr.STYLES[name] = self.PackStyleSet(style_dict)
            return True
        else:
            self.LOG("[ed_style][err] SetStyles expects a " \
                     "dictionary of StyleItems")
            return False

    def SetSyntax(self, synlst):
        """Sets the Syntax Style Specs from a list of specifications
        @param synlst: [(STYLE_ID, "STYLE_TYPE"), (STYLE_ID2, "STYLE_TYPE2)]

        """
        # Parses Syntax Specifications list, ignoring all bad values
        self.UpdateBaseStyles()
        valid_settings = list()
        for syn in synlst:
            if len(syn) != 2:
                self.LOG("[ed_style][warn] Bogus Syntax Spec %s" % repr(syn))
                continue
            else:
                self.StyleSetSpec(syn[0], self.GetStyleByName(syn[1]))
                valid_settings.append(syn)

        self.syntax_set = valid_settings
        return True

    def StyleDefault(self):
        """Clears the editor styles to default
        @postcondition: style is reset to default

        """
        self.StyleClearAll()
        self.SetCaretForeground(wx.BLACK)
        self.Colourise(0, -1)

    def UpdateAllStyles(self, spec_style=None):
        """Refreshes all the styles and attributes of the control
        @param spec_style: style scheme name
        @postcondition: style scheme is set to specified style

        """
        if spec_style and (spec_style != self.style_set):
            self.LoadStyleSheet(self.GetStyleSheet(spec_style), force=True)
        self.SetSyntax(self.GetSyntaxParams())
        self.Refresh()

    def UpdateBaseStyles(self):
        """Updates the base styles of editor to the current settings
        @postcondition: base style info is updated

        """
        self.StyleDefault()
        self.SetMargins(4, 0)

        # Global default styles for all languages
        self.StyleSetSpec(0, self.GetStyleByName('default_style'))
        self.StyleSetSpec(wx.stc.STC_STYLE_DEFAULT, \
                          self.GetStyleByName('default_style'))
        self.StyleSetSpec(wx.stc.STC_STYLE_LINENUMBER, \
                          self.GetStyleByName('line_num'))
        self.StyleSetSpec(wx.stc.STC_STYLE_CONTROLCHAR, \
                          self.GetStyleByName('ctrl_char'))
        self.StyleSetSpec(wx.stc.STC_STYLE_BRACELIGHT, \
                          self.GetStyleByName('brace_good'))
        self.StyleSetSpec(wx.stc.STC_STYLE_BRACEBAD, \
                          self.GetStyleByName('brace_bad'))
        self.StyleSetSpec(wx.stc.STC_STYLE_INDENTGUIDE, \
                          self.GetStyleByName('guide_style'))

        # wx.stc.STC_STYLE_CALLTIP doesn't seem to do anything
        calltip = self.GetItemByName('calltip')
        self.CallTipSetBackground(calltip.GetBack())
        self.CallTipSetForeground(calltip.GetFore())

        sback = self.GetItemByName('select_style')
        if not sback.IsNull() and len(sback.GetBack()):
            sback = sback.GetBack()
            sback = eclib.HexToRGB(sback)
            sback = wx.Colour(*sback)
        else:
            sback = wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT)

        # If selection colour is dark make the foreground white
        # else use the default settings.
        if sum(sback.Get()) < 384:
            self.SetSelForeground(True, wx.WHITE)
        else:
            self.SetSelForeground(False, wx.BLACK)
        self.SetSelBackground(True, sback)

        wspace = self.GetItemByName('whitespace_style')
        self.SetWhitespaceBackground(True, wspace.GetBack())
        self.SetWhitespaceForeground(True, wspace.GetFore())

        default_fore = self.GetDefaultForeColour()
        edge_colour = self.GetItemByName('edge_style')
        self.SetEdgeColour(edge_colour.GetFore())
        self.SetCaretForeground(default_fore)
        self.SetCaretLineBack(self.GetItemByName('caret_line').GetBack())
        self.Colourise(0, -1)

#-----------------------------------------------------------------------------#
# Utility Functions

def NullStyleItem():
    """Create a null style item
    @return: empty style item that cannot be merged

    """
    item = StyleItem()
    item.null = True
    return item

DEF_STYLE_DICT = \
        {'brace_good' : StyleItem("#FFFFFF", "#0000FF", ex=["bold",]),
         'brace_bad'  : StyleItem(back="#FF0000", ex=["bold",]),
         'calltip'    : StyleItem("#404040", "#FFFFB8"),
         'caret_line' : StyleItem(back="#D8F8FF"),
         'ctrl_char'  : StyleItem(),
         'line_num'   : StyleItem(back="#C0C0C0", face="%(secondary)s", \
                                  size="%(size3)d"),
         'array_style': StyleItem("#EE8B02",
                                  face="%(secondary)s",
                                  ex=["bold",]),
         'btick_style': StyleItem("#8959F6", size="%(size)d", ex=["bold",]),
         'default_style': StyleItem("#000000", "#F6F6F6", \
                                    "%(primary)s", "%(size)d"),
         'char_style' : StyleItem("#FF3AFF"),
         'class_style' : StyleItem("#2E8B57", ex=["bold",]),
         'class2_style' : StyleItem("#2E8B57", ex=["bold",]),
         'comment_style' : StyleItem("#838383"),
         'decor_style' : StyleItem("#BA0EEA", face="%(secondary)s",
                                   ex=["italic",]),
         'directive_style' : StyleItem("#0000FF", face="%(secondary)s",
                                       ex=["bold",]),
         'dockey_style' : StyleItem("#0000FF"),
         'edge_style'   : StyleItem(), # inherit from default
         'error_style' : StyleItem("#DD0101", face="%(secondary)s",
                                    ex=["bold",]),
         'foldmargin_style' : StyleItem(back="#D1D1D1"),
         'funct_style' : StyleItem("#008B8B", ex=["italic",]),
         'global_style' : StyleItem("#007F7F", face="%(secondary)s",
                                    ex=["bold",]),
         'guide_style' : StyleItem("#838383"),
         'here_style' : StyleItem("#CA61CA", face="%(secondary)s",
                                  ex=["bold",]),
         'ideol_style' : StyleItem("#E0C0E0", face="%(secondary)s"),
         'keyword_style' : StyleItem("#A52B2B", ex=["bold",]),
         'keyword2_style' : StyleItem("#2E8B57", ex=["bold",]),
         'keyword3_style' : StyleItem("#008B8B", ex=["bold",]),
         'keyword4_style' : StyleItem("#9D2424"),
         'marker_style' : StyleItem("#FFFFFF", "#000000"),
         'number_style' : StyleItem("#DD0101"),
         'number2_style' : StyleItem("#DD0101", ex=["bold",]),
         'operator_style' : StyleItem("#000000", face="%(primary)s",
                                      ex=["bold",]),
         'pre_style' : StyleItem("#AB39F2", ex=["bold",]),
         'pre2_style' : StyleItem("#AB39F2", "#FFFFFF", ex=["bold",]),
         'regex_style' : StyleItem("#008B8B"),
         'scalar_style' : StyleItem("#AB37F2", face="%(secondary)s",
                                    ex=["bold",]),
         'scalar2_style' : StyleItem("#AB37F2", face="%(secondary)s"),
         'select_style' : NullStyleItem(), # Use system default colour
         'string_style' : StyleItem("#FF3AFF", ex=["bold",]),
         'stringeol_style' : StyleItem("#000000", "#EEC0EE",
                                       "%(secondary)s", ex=["bold", "eol"]),
         'unknown_style' : StyleItem("#FFFFFF", "#DD0101", ex=["bold", "eol"]),
         'userkw_style' : StyleItem(),
         'whitespace_style' : StyleItem('#838383')
         }

def MergeFonts(style_dict, font_dict):
    """Does any string substitution that the style dictionary
    may need to have fonts and their sizes set.
    @param style_dict: dictionary of L{StyleItem}
    @param font_dict: dictionary of font data
    @return: style dictionary with all font format strings substituted in

    """
    for style in style_dict:
        st_str = unicode(style_dict[style])
        if u'%' in st_str:
            style_dict[style].SetAttrFromStr(st_str % font_dict)
    return style_dict

def MergeStyles(styles1, styles2):
    """Merges the styles from styles2 into styles1 overwriting
    any duplicate values already set in styles1 with the new
    data from styles2.
    @param styles1: dictionary of StyleItems to receive merge
    @param styles2: dictionary of StyleItems to merge from
    @return: style1 with all values from styles2 merged into it

    """
    for style in styles2:
        styles1[style] = styles2[style]
    return styles1
