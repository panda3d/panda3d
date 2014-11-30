###############################################################################
# Name: generator.py                                                          #
# Purpose: Utility classes for creating various formatted plain text          #
#          from the contents of a EdStc text buffer (i.e HTML, LaTeX, Rtf)    #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Provides various methods and classes for generating code and transforming code
to different formats such as html, latex, rtf with all the styling and formating
intact from how the view is shown in the editor.

It also provides a plugin interface that allows for plugins that wish to provide
similar services for manipulating and transforming text.

@summary: Editra's Generator interface and implementations

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: generator.py 66957 2011-02-18 22:20:00Z CJP $"
__revision__ = "$Revision: 66957 $"

#--------------------------------------------------------------------------#
# Imports
import wx
import wx.stc
import time

# Editra Libraries
import ed_glob
import ed_menu
from ed_style import StyleItem
import util
import plugin
import ebmlib
import eclib

#--------------------------------------------------------------------------#
# Globals
_ = wx.GetTranslation

FONT_FALLBACKS = "Trebuchet, Tahoma, sans-serif"

#--------------------------------------------------------------------------#
# Plugin Interface
class GeneratorI(plugin.Interface):
    """Plugins that are to be used for generating code/document need
    to impliment this interface.

    """
    def Generate(self, stc):
        """Generates the code. The txt_ctrl parameter is a reference
        to an ED_STC object (see ed_stc.py). The return value of this
        function needs to be a 2 item tuple with the first item being
        an associated file extention to use for setting highlighting
        if available and the second item is the string of the new document.
        @param stc: reference to an an stc defined in ed_stc.py
        @see: L{ed_stc}

        """
        pass

    def GetId(self):
        """Must return the Id used for the generator objects
        menu id. This is used to identify which Generator to
        call on a menu event.
        @return: menu id that identifies the implemented generator

        """
        pass

    def GetMenuEntry(self, menu):
        """Returns the MenuItem entry for this generator
        @return: menu entry for the implemented generator
        @rtype: wx.MenuItem

        """
        pass

#-----------------------------------------------------------------------------#

class Generator(plugin.Plugin):
    """Plugin Interface Extension Point for Generator
    type plugin objects. Generator objects are used
    to generate a document/code from one type to another.

    """
    observers = plugin.ExtensionPoint(GeneratorI)

    def InstallMenu(self, menu):
        """Appends the menu of available Generators onto
        the given menu.
        @param menu: menu to install entries into
        @type menu: wx.Menu

        """
        # Fetch all the menu items for each generator object
        menu_items = list()
        for observer in self.observers:
            try:
                menu_i = observer.GetMenuEntry(menu)
                if menu_i:
                    menu_items.append((menu_i.GetItemLabel(), menu_i))
            except Exception, msg:
                util.Log("[generator][err] %s" % str(msg))

        # Construct the menu
        menu_items.sort()
        genmenu = ed_menu.EdMenu()
        for item in menu_items:
            genmenu.AppendItem(item[1])
        menu.AppendMenu(ed_glob.ID_GENERATOR, _("Generator"), genmenu,
                             _("Generate Code and Documents"))

    def GenerateText(self, e_id, txt_ctrl):
        """Generates the new document text based on the given
        generator id and contents of the given ED_STC text control.
        @param e_id: event id originating from menu entry
        @param txt_ctrl: reference document to generate from
        @type txt_ctrl: EditraStc
        @return: the generated text
        @rtype: string

        """
        gentext = None
        start = time.time()
        # Find the correct generator and run its generate method on the
        # given text control.
        for observer in self.observers:
            if observer.GetId() == e_id:
                gentext = observer.Generate(txt_ctrl)
                util.Log("[generator][info] Generation time %f" % (time.time() - start))
        return gentext

#-----------------------------------------------------------------------------#

class Html(plugin.Plugin):
    """Transforms the text from a given Editra stc to a fully
    styled html page. Inline CSS is generated and inserted into
    the head of the Html to style the text regions by default
    unless requested to generate a separate sheet.

    """
    plugin.Implements(GeneratorI)
    def __init__(self, mgr):
        """Creates the Html object from an Editra stc text control
        @param mgr: This generators plugin manager

        """
        plugin.Plugin.__init__(self)

        # Attributes
        self._id = ed_glob.ID_HTML_GEN
        self.stc = None
        self.head = wx.EmptyString
        self.css = dict()
        self.body = wx.EmptyString

    def __str__(self):
        """Returns the string of html
        @return: string version of html object

        """
        # Assemble the embedded html
        style = "<style type=\"text/css\">\n%s</style>"
        css = wx.EmptyString
        for key in self.css:
            css += str(self.css[key]) + "\n"
        css = css % self.stc.GetFontDictionary()
        style = style % css

        # Insert the css into the head
        head = self.head.replace('</head>', style + "\n</head>")

        # Assemble the body of the html
        html = "<html>\n%s\n%s\n</html>"
        html = html % (head, self.body)
        return html

    def Unicode(self):
        """Returns the html as unicode
        @return: unicode string of html

        """
        return unicode(self.__str__())

    def Generate(self, stc_ctrl):
        """Generates and returns the document
        @param stc_ctrl: text control to get text from

        """
        self.stc = stc_ctrl
        self.head = self.GenerateHead()
        self.body = self.GenerateBody()
        return ("html", self.__str__())

    def GenerateHead(self):
        """Generates the html head block
        @return: html header information
        @rtype: string

        """
        return "<head>\n<title>%s</title>\n" \
               "<meta name=\"Generator\" content=\"Editra/%s\">\n" \
               "<meta http-equiv=\"content-type\" content=\"text/html; " \
               "charset=utf-8\">" \
               "\n</head>" % (ebmlib.GetFileName(self.stc.GetFileName()),
                              ed_glob.VERSION)

    def GenerateBody(self):
        """Generates the body of the html from the stc's content. To do
        this it does a character by character parse of the stc to determine
        style regions and generate css and and styled spans of html in order
        to generate an 'exact' html reqresentation of the stc's window.
        @return: the body section of the html generated from the text control

        """
        html = list()
        parse_pos = 0
        style_start = 0
        style_end = 0
        last_pos = self.stc.GetLineEndPosition(self.stc.GetLineCount()) + 1

        # Get Document start point info
        last_id = self.stc.GetStyleAt(parse_pos)
        tag = self.stc.FindTagById(last_id)
        if tag != wx.EmptyString:
            s_item = StyleItem()
            s_item.SetAttrFromStr(self.stc.GetStyleByName(tag))
            self.css[tag] = CssItem(tag.split('_')[0], s_item)

        # Optimizations
        stc = self.stc
        GetStyleAt = stc.GetStyleAt

        # Build Html
        while parse_pos < last_pos:
            parse_pos += 1
            curr_id = GetStyleAt(parse_pos)
            style_end = parse_pos
            # If style region has changed close section
            if curr_id == 0 and GetStyleAt(parse_pos + 1) == last_id:
                curr_id = last_id

            if curr_id != last_id or parse_pos == last_pos:
                tmp = stc.GetTextRange(style_start, style_end)
                tmp = self.TransformText(tmp)
                if tmp.isspace() or tag in ["default_style", "operator_style"]:
                    html.append(tmp)
                else:
                    tmp2 = "<span class=\"%s\">%s</span>"
                    html.append(tmp2 % (tag.split('_')[0], tmp))

                last_id = curr_id
                style_start = style_end
                tag = stc.FindTagById(last_id)
                if tag not in self.css:
                    s_item = StyleItem()
                    s_item.SetAttrFromStr(stc.GetStyleByName(tag))
                    self.css[tag] = CssItem(tag.split('_')[0], s_item)

        # Case for unstyled documents
        if len(html) == 0:
            s_item = StyleItem()
            s_item.SetAttrFromStr(stc.GetStyleByName('default_style'))
            self.css['default_style'] = CssItem('default', s_item)
            html.append(self.TransformText(stc.GetText()))
        else:
            self.OptimizeCss()

        return "<body class=\"default\">\n<pre>\n%s\n</pre>\n</body>" % \
                                                                   "".join(html)

    def GetId(self):
        """Returns the menu identifier for the HTML generator
        @return: id of this object

        """
        return self._id

    def GetMenuEntry(self, menu):
        """Returns the Menu control for the HTML generator
        @return: menu entry for this generator

        """
        return wx.MenuItem(menu, self._id, _("Generate %s") % u"HTML",
                           _("Generate a %s version of the " \
                             "current document") % u"HTML")

    def OptimizeCss(self):
        """Optimizes the CSS Set
        @postcondition: css is optimized to remove any redundant entries

        """
        # Must have the default style defined
        if 'default_style' not in self.css:
            return

        # Don't style operators. This is to optimize the html size
        if 'operator_style' in self.css:
            self.css.pop('operator_style')

        # All other css elements will inheirit from the default
        default = self.css['default_style']
        for key in self.css:
            if key == 'default_style':
                continue
            if default.GetFont() == self.css[key].GetFont():
                self.css[key].SetFont(wx.EmptyString)
            if default.GetFontSize() == self.css[key].GetFontSize():
                self.css[key].SetFontSize(wx.EmptyString)
            if default.GetBackground() == self.css[key].GetBackground():
                self.css[key].SetBackground(wx.EmptyString)
            if default.GetColor() == self.css[key].GetColor():
                self.css[key].SetColor(wx.EmptyString)
            for item in default.GetDecorators():
                if item in self.css[key].GetDecorators():
                    self.css[key].RemoveDecorator(item)

    def TransformText(self, text):
        """Does character substitution on a string and returns
        the html equivlant of the given string.
        @param text: text to transform
        @return: text with all special characters transformed

        """
        text = text.replace('&', "&amp;")      # Ampersands
        text = text.replace('<', "&lt;")       # Less Than Symbols
        text = text.replace('>', "&gt;")       # Greater Than Symbols
        text = text.replace("\"", "&quot;")
        return text

#-----------------------------------------------------------------------------#

class CssItem:
    """Converts an Edtira StyleItem to a Css item for use in
    generating html.

    """
    def __init__(self, class_tag, style_item):
        """Initilizes a Css object equivilant of an Editra StyleItem
        @note: it is left up to the caller to do any string substituition
        for font faces and size values as this class will contruct the css
        item as a mere reformation of StyleItem
        @param class_tag: StyleItem tag name
        @param style_item: style item to convert to css
        @type style_item: ed_style.StyleItem
        @see: L{ed_style}

        """

        # Attributes
        self._tag = class_tag
        self._back = style_item.GetBack()
        self._fore = style_item.GetFore()
        self._font = style_item.GetFace()
        self._size = style_item.GetSize()

        # List of additional style specs
        self._decor = self.ExtractDecorators()
        self._decor.extend(style_item.GetModifierList())

    def __eq__(self, css2):
        """Defines the == operator for the CssItem class
        @param css2: CssItem to compare to
        @return: whether the two items are equivalant
        @rtype: bool

        """
        return self.__str__() == str(css2)

    def __str__(self):
        """Outputs the css item as a formatted css block
        @return: CssItem as a string

        """
        # Generate the main style attribures
        css = ".%s {\n%s}"
        css_body = wx.EmptyString
        if self._font != wx.EmptyString:
            font = self._font.split(',')
            css_body += u"\tfont-family: %s, %s;\n" % (font[0], FONT_FALLBACKS)
        if self._size != wx.EmptyString:
            size = self._size.split(',')
            css_body += u"\tfont-size: %s;\n" % str(size[0])
        if self._fore != wx.EmptyString:
            fore = self._fore.split(',')
            css_body += u"\tcolor: %s;\n" % fore[0]
        if self._back != wx.EmptyString:
            back = self._back.split(',')
            css_body += u"\tbackground-color: %s;\n" % back[0]

        # Add additional style modifiers
        for item in self._decor:
            if item == u'bold':
                css_body += u"\tfont-weight: %s;\n" % item
            elif item == u'italic':
                css_body += u"\tfont-style: %s;\n" % item
            elif item == u'underline':
                css_body += u"\ttext-decoration: %s;\n" % item
            else:
                pass

        # Format the tag and body into the css def
        if css_body != wx.EmptyString:
            return css % (self._tag, css_body)
        else:
            return css_body

    def ExtractDecorators(self):
        """Pulls additional style specs from the StyleItem such
        as bold, italic, and underline styles.
        @return: all decorators in the StyleItem (bold, underline, ect...)

        """
        decor = list()
        for val in [ self._back, self._fore, self._font, self._size ]:
            tmp = val.split(u',')
            if len(tmp) < 2:
                continue
            else:
                decor.append(tmp[1])
        return decor

    def GetBackground(self):
        """Returns the Background value
        @return: background color attribute

        """
        return self._back

    def GetColor(self):
        """Returns the Font/Fore Color
        @return: foreground color attribute

        """
        return self._fore

    def GetDecorators(self):
        """Returns the list of decorators
        @return: list of decorators item uses

        """
        return self._decor

    def GetFont(self):
        """Returns the Font Name
        @return: font name attribute

        """
        return self._font

    def GetFontSize(self):
        """Returns the Font Size
        @return: font size attribute

        """
        return self._size

    def RemoveDecorator(self, item):
        """Removes a specifed decorator from the decorator set
        @param item: decorator item to remove
        @type item: string

        """
        if item in self._decor:
            self._decor.remove(item)
        else:
            pass

    def SetBackground(self, hex_str):
        """Sets the Background Color
        @param hex_str: hex color string to set backround attribute with

        """
        self._back = hex_str

    def SetColor(self, hex_str):
        """Sets the Font/Fore Color
        @param hex_str: hex color string to set foreround attribute with

        """
        self._fore = hex_str

    def SetFont(self, font_face):
        """Sets the Font Face
        @param font_face: font face name to set font attribute with

        """
        self._font = font_face

    def SetFontSize(self, size_str):
        """Sets the Font Point Size
        @param size_str: point size to use for font in style
        @type size_str: string

        """
        self._size = size_str

#-----------------------------------------------------------------------------#

class LaTeX(plugin.Plugin):
    """Creates a LaTeX document object from the contents of the
    supplied document reference.
    @todo: performance improvements and wordwrap in generated document

    """
    plugin.Implements(GeneratorI)
    def __init__(self, plgmgr):
        """Initializes the LaTeX object
        @param plgmgr: pluginmanger for this object

        """
        plugin.Plugin.__init__(self)

        # Attributes
        self._stc = None
        self._id = ed_glob.ID_TEX_GEN
        self._dstyle = StyleItem()
        self._cmds = dict()

    def CreateCmdName(self, name):
        """Creates and returns a proper cmd name
        @param name: name to construct command from
        @return: latex formated command string

        """
        name = name.replace('_', '')
        tmp = list()
        alpha = "ABCDEFGHIJ"
        for char in name:
            if char.isdigit():
                tmp.append(alpha[int(char)])
            else:
                tmp.append(char)
        return "".join(tmp)

    def GenDoc(self):
        """Generates the document body of the LaTeX document
        @returns: the main body of the reference document marked up with latex

        """
        tex = list()
        tmp = u''
        start = parse_pos = 0
        last_pos = self._stc.GetLineEndPosition(self._stc.GetLineCount())

        # Define the default style
        self.RegisterStyleCmd('default_style', \
                              self._stc.GetItemByName('default_style'))

        # Get Document start point info
        last_id = self._stc.GetStyleAt(parse_pos)
        tmp = self.TransformText(self._stc.GetTextRange(parse_pos,
                                                        parse_pos + 1))
        tag = self._stc.FindTagById(last_id)
        if tag != wx.EmptyString:
            self.RegisterStyleCmd(tag, self._stc.GetItemByName(tag))

        # Optimizations
        stc = self._stc
        GetStyleAt = stc.GetStyleAt
        GetTextRange = stc.GetTextRange
        TransformText = self.TransformText

        # Build LaTeX
        for parse_pos in xrange(last_pos + 1):
            curr_id = GetStyleAt(parse_pos)
            if parse_pos > 1:
                # This is the performance bottleneck, changeing the text
                # collection to when the style changes is much faster as
                # it only needs to be done once per style section instead
                # of once per character. Doing that however causes problems
                # with the style and resulting document formatting.
                tmp = TransformText(GetTextRange((parse_pos - 1), parse_pos))

            if curr_id == 0 and GetStyleAt(parse_pos + 1) == last_id:
                curr_id = last_id

            # If style region has changed close section
            if curr_id != last_id or tmp[-1] == "\n":
                tmp_tex = TransformText(GetTextRange(start, parse_pos))
#                 tmp_tex = u"".join(tmp)
                if tag == "operator_style" or \
                   (tag == "default_style" and \
                    tmp_tex.isspace() and len(tmp_tex) <= 2):
                    tex.append(tmp_tex)
                else:
                    if "\\\\\n" in tmp_tex:
                        tmp_tex = tmp_tex.replace("\\\\\n", "")
                        tmp2 = "\\%s{%s}\\\\\n"
                    else:
                        tmp2 = "\\%s{%s}"

                    cmd = self.CreateCmdName(tag)
                    if cmd in [None, wx.EmptyString]:
                        cmd = "defaultstyle"
                    tex.append(tmp2 % (cmd, tmp_tex))

                last_id = curr_id
                tag = stc.FindTagById(last_id)
                if tag not in [None, wx.EmptyString]:
                    self.RegisterStyleCmd(tag, stc.GetItemByName(tag))
                tmp = list()
                start = parse_pos

        # Case for unstyled documents
        if tex == wx.EmptyString:
            tex.append(self.TransformText(stc.GetText()))
        return "\\begin{document}\n%s\n\\end{document}" % "".join(tex)

    def Generate(self, stc_doc):
        """Generates the LaTeX document
        @param stc_doc: text control to generate latex from
        @return: the reference document marked up in LaTeX.

        """
        self._stc = stc_doc
        default_si = self._stc.GetItemByName('default_style')
        self._dstyle.SetBack(default_si.GetBack().split(',')[0])
        self._dstyle.SetFore(default_si.GetFore().split(',')[0])
        self._dstyle.SetFace(default_si.GetFace().split(',')[0])
        self._dstyle.SetSize(default_si.GetSize().split(',')[0])
        body = self.GenDoc()
        preamble = self.GenPreamble()
        return ("tex", u"".join([preamble, body]))

    def GenPreamble(self):
        """Generates the Preamble of the document
        @return: the LaTeX document preamble

        """
        # Preamble template
        pre = ("%% \iffalse meta-comment\n"
               "%%\n%% Generated by Editra %s\n"
               "%% This is generator is Very Experimental.\n"
               "%% The code should compile in most cases but there may\n"
               "%% be some display issues when rendered.\n"
               "%%\n%%\n\n"
               "\\documentclass[11pt, a4paper]{article}\n"
               "\\usepackage[a4paper, margin=2cm]{geometry}\n"
               "\\usepackage[T1]{fontenc}\n"
#               "\\usepackage{ucs}\n"
#               "\\usepackage[utf8]{inputenc}\n"
               "\\usepackage{color}\n"
               "\\usepackage{alltt}\n"
               "\\usepackage{times}\n") % ed_glob.VERSION

        # Set the background color
        pre += ("\\pagecolor[rgb]{%s}\n" % \
                self.HexToRGB(self._dstyle.GetBack()))
        pre += "\\parindent=0in\n\n"

        # Insert all styling commands
        pre += "%% Begin Styling Command Definitions"
        for cmd in self._cmds:
            pre += ("\n" + self._cmds[cmd])
        pre += "\n%% End Styling Command Definitions\n\n"
        return pre

    def GetId(self):
        """Returns the menu identifier for the LaTeX generator
        @return: id of that identifies this generator

        """
        return self._id

    def GetMenuEntry(self, menu):
        """Returns the Menu control for the LaTeX generator
        @param menu: menu to create MenuItem for

        """
        return wx.MenuItem(menu, self._id, _("Generate %s") % u"LaTeX",
                           _("Generate an %s version of the " \
                             "current document") % u"LaTeX")

    def HexToRGB(self, hex_str):
        """Returns a comma separated rgb string representation
        of the input hex string. 1.0 = White, 0.0 = Black.
        @param hex_str: hex string to convert to latex rgb format

        """
        r_hex = hex_str
        if r_hex[0] == u"#":
            r_hex = r_hex[1:]
        ldiff = 6 - len(r_hex)
        r_hex += ldiff * u"0"
        # Convert hex values to integer
        red = round(float(float(int(r_hex[0:2], 16)) / 255), 2)
        green = round(float(float(int(r_hex[2:4], 16)) / 255), 2)
        blue = round(float(float(int(r_hex[4:], 16)) / 255), 2)
        return "%s,%s,%s" % (str(red), str(green), str(blue))

    def RegisterStyleCmd(self, cmd_name, s_item):
        """Registers and generates a command from the
        supplied StyleItem.
        @param cmd_name: name of command
        @param s_item: style item to create command for
        @postcondition: new styling command is created and registered for use

        """
        cmd_name = self.CreateCmdName(cmd_name)

        # If we already made a command for this style return
        if cmd_name in self._cmds:
            return

        # Templates
        uline_tmp = u"\\underline{%s}"
        ital_tmp = u"\\emph{%s}"
        bold_tmp = u"\\textbf{%s}"
        fore_tmp = u"\\textcolor[rgb]{%s}{%s}"
        back_tmp = u"\\colorbox[rgb]{%s}{#1}"
        cmd_tmp = u"\\newcommand{%s}[1]{%s}"

        # Get Style Attributes
        fore = s_item.GetFore()
        if fore == wx.EmptyString:
            fore = self._dstyle.GetFore()
        back = s_item.GetBack()
        if back == wx.EmptyString:
            back = self._dstyle.GetBack()
        face = s_item.GetFace()
        if face == wx.EmptyString:
            face = self._dstyle.GetFace()
        size = s_item.GetSize()
        if size == wx.EmptyString:
            size = self._dstyle.GetSize()

        back = back_tmp % self.HexToRGB(back.split(u',')[0])
        fore = fore_tmp % (self.HexToRGB(fore.split(u',')[0]), back)
        if u"bold" in unicode(s_item):
            fore = bold_tmp % fore
        if u"underline" in unicode(s_item):
            fore = uline_tmp % fore
        if u"italic" in unicode(s_item):
            fore = ital_tmp % fore
        cmd = cmd_tmp % ((u"\\" + cmd_name), u"\\texttt{\\ttfamily{%s}}" % fore)
        self._cmds[cmd_name] = cmd

    def TransformText(self, txt):
        """Transforms the given text into LaTeX format, by
        escaping all special characters and sequences.
        @param txt: text to transform
        @return: txt with all special characters transformed

        """
        ch_map = { "#" : "\\#", "$" : "\\$", "^" : "\\^",
                   "%" : "\\%", "&" : "\\&", "_" : "\\_",
                   "{" : "\\{", "}" : "\\}", "~" : "\\~",
                   "\\": "$\\backslash$", "\n" : "\\\\\n",
                   "@" : "$@$", "<" : "$<$", ">" : "$>$",
                   "-" : "$-$", "|" : "$|$"
                 }
        tmp = list()
        for char in txt:
            tmp.append(ch_map.get(char, char))
        return u''.join(tmp)

#-----------------------------------------------------------------------------#

class Rtf(plugin.Plugin):
    """Generates a fully styled RTF document from the given text
    controls contents.
    @todo: add support for bold/italic/underline and multiple fonts

    """
    plugin.Implements(GeneratorI)
    def __init__(self, mgr):
        """Initializes and declares the attribute values for
        this generator.
        @param mgr: plugin manager of this object

        """
        plugin.Plugin.__init__(self)

        # Attributes
        self._stc = None
        self._id = ed_glob.ID_RTF_GEN
        self._colortbl = RtfColorTbl()

    def __str__(self):
        """Returns the RTF object as a string
        @return: rtf object as a string

        """
        return self._GenRtf()

    #---- Protected Member Functions ----#
    def _GenRtf(self):
        """Generates the RTF equivalent of the displayed text in the current
        stc document window.
        @precondition: self._stc must have been set by a call to Generate
        @return: generated rtf marked up text

        """
        # Buffer hasn't been set
        if self._stc is None:
            return u''

        # Optimizations
        stc = self._stc
        def_fore = stc.GetDefaultForeColour(as_hex=True)
        self._colortbl.AddColor(def_fore)
        def_back = stc.GetDefaultBackColour(as_hex=True)
        self._colortbl.AddColor(def_back)
        last_pos = stc.GetLineEndPosition(stc.GetLineCount())
        parse_pos = 0
        last_id = None
        last_fore = None
        last_back = None
        start = end = 0
        tmp_txt = list()
        font_tmp = "\\f0"
        fore_tmp = "\\cf%d"
        back_tmp = "\\cb%d"
        AddColor = self._colortbl.AddColor
        GetColorIndex = self._colortbl.GetColorIndex
        GetStyleAt = stc.GetStyleAt

        # Parse all characters/style bytes in document
        for parse_pos in xrange(last_pos + 1):
            sty_id = GetStyleAt(parse_pos)
            end = parse_pos

            # If style has changed build the previous section
            if sty_id != last_id:
                tag = stc.FindTagById(last_id)
                s_item = stc.GetItemByName(tag)
                AddColor(s_item.GetFore())
                AddColor(s_item.GetBack())
                tplate = font_tmp
                fid = GetColorIndex(s_item.GetFore())
                if fid != last_fore:
                    last_fore = fid
                    tplate = tplate + (fore_tmp % fid)
                bid = GetColorIndex(s_item.GetBack())
                if bid != last_back:
                    last_back = bid
                    tplate = tplate + (back_tmp % bid)
                tmp_txt.append(tplate + " " + \
                               self.TransformText(stc.GetTextRange(start, end)))
                start = end
            last_id = sty_id

        head = "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 %s;}}" % \
                stc.GetDefaultFont().GetFaceName()
        return u"%s%s%s}" % (head, self._colortbl, "".join(tmp_txt))

    #---- End Protected Member Functions ----#

    def Generate(self, stc_doc):
        """Implements the GeneratorI's Generator Function by
        returning the RTF equvialent of the given stc_doc
        @param stc_doc: document to generate text from
        @return: document marked up in rtf

        """
        self._stc = stc_doc
        return ('rtf', self._GenRtf())

    def GetId(self):
        """Implements the GeneratorI's GetId function by returning
        the identifier for this generator.
        @return: identifier for this generator

        """
        return self._id

    def GetMenuEntry(self, menu):
        """Implements the GeneratorI's GetMenuEntry function by
        returning the MenuItem to associate with this object.
        @return: menu entry item for this generator

        """
        return wx.MenuItem(menu, self._id, _("Generate %s") % u"RTF",
                           _("Generate a %s version of the " \
                             "current document") % u"RTF")

    def TransformText(self, text):
        """Transforms the given text by converting it to RTF format
        @param text: text to transform
        @return: text with all special characters transformed
        """
        chmap = { "\t" : "\\tab ", "{" : "\\{", "}" : "\\}",
                  "\\" : "\\\\", "\n" : "\\par\n", "\r" : "\\par\n"}
        text = text.replace('\r\n', '\n')
        tmp = u''
        for char in text:
            tmp = tmp + chmap.get(char, char)
        return tmp

#-----------------------------------------------------------------------------#

class RtfColorTbl:
    """A storage class to help with generating the color table for
    the Rtf Generator Class.
    @see: Rtf

    """
    def __init__(self):
        """Initialize the color table
        @summary: creates an object for managing an rtf documents color table

        """
        # Attributes
        self._index = list() # manages the order of the tables keys
        self._tbl = dict()   # map of style item color vals to rtf defs

    def __str__(self):
        """Returns the string representation of the table
        @return: rtf color table object as an rtf formatted string

        """
        rstr = u''
        for item in self._index:
            rstr = rstr + self._tbl[item]
        return u"{\\colortbl%s}" % rstr

    def AddColor(self, si_color):
        """Takes a style item and adds it to the table if
        has not already been defined in the table.
        @param si_color: color to add to table
        @type si_color: hex color string

        """
        if si_color not in self._index:
            rgb = eclib.HexToRGB(si_color.split(u',')[0])
            color = "\\red%d\\green%d\\blue%d;" % tuple(rgb)
            self._index.append(si_color)
            self._tbl[si_color] = color
        else:
            pass

    def GetColorIndex(self, si_color):
        """Gets the index of a particular style items color
        definition from the color table. Returns -1 if item is
        not found.
        @param si_color: style item color to find index in table for
        @return: the colors index in the table

        """
        if si_color in self._index:
            return self._index.index(si_color)
        else:
            return -1
