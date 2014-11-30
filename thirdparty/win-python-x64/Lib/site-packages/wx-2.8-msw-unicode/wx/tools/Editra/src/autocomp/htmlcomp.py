###############################################################################
# Name: htmlcomp.py                                                           #
# Purpose: Simple input assistant for html and xml.                           #
# Author: Cody Precord                                                        #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Simple autocompletion support for HTML and XML documents.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__cvsid__ = "$Id: htmlcomp.py 67123 2011-03-04 00:02:35Z CJP $"
__revision__ = "$Revision: 67123 $"

#--------------------------------------------------------------------------#
# Imports
import re
import wx
import wx.stc

# Local Imports
import completer

#--------------------------------------------------------------------------#
# Standard Html Tags
TAGS = ['!--', 'a', 'abbr', 'accept', 'accesskey', 'acronym', 'action',
        'address', 'align', 'alink', 'alt', 'applet', 'archive', 'area',
        'article', 'aside', 'audio', 'axis', 'b', 'background', 'base',
        'basefont', 'bdo', 'bgcolor', 'big', 'blockquote', 'body', 'border',
        'bordercolor', 'br', 'button', 'canvas', 'caption', 'cellpadding',
        'cellspacing', 'center', 'char', 'charoff', 'charset', 'checked',
        'cite', 'cite', 'class', 'classid', 'clear', 'code', 'codebase',
        'codetype', 'col', 'colgroup', 'color', 'cols', 'colspan', 'command',
        'compact', 'content', 'coords', 'data', 'datetime', 'datalist', 'dd',
        'declare', 'defer', 'del', 'details', 'dfn', 'dialog', 'dir', 'dir',
        'disabled', 'div', 'dl', 'dt', 'dtml-call', 'dtml-comment', 'dtml-if',
        'dtml-in', 'dtml-let', 'dtml-raise', 'dtml-tree', 'dtml-try',
        'dtml-unless', 'dtml-var', 'dtml-with', 'em', 'embed', 'enctype',
        'face', 'fieldset', 'figcaption', 'figure', 'font', 'for', 'form',
        'footer', 'frame', 'gutter', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'head',
        'header', 'headers', 'height', 'hgroup', 'hr', 'href', 'hreflang',
        'hspace', 'html', 'http-equiv', 'i', 'id', 'iframe', 'img', 'input',
        'ins', 'isindex', 'ismap', 'kbd', 'keygen', 'label', 'lang', 'language',
        'legend', 'li', 'link', 'link', 'longdesc', 'lowsrc', 'map',
        'marginheight', 'marginwidth', 'mark', 'maxlength', 'menu', 'meta',
        'meter', 'method', 'multiple', 'name', 'nav', 'nohref', 'noscript',
        'nowrap', 'object', 'ol', 'optgroup', 'option', 'output', 'p', 'param',
        'pre', 'profile', 'progress', 'prompt', 'q', 'readonly', 'rel', 'rev',
        'rows', 'rowspan', 'rp', 'rt', 'ruby', 'rules', 's', 'samp', 'scheme',
        'scope', 'script', 'scrolling', 'section', 'select', 'selected',
        'shape', 'size', 'small', 'source', 'span', 'src', 'standby', 'start',
        'strike', 'strong', 'style', 'sub', 'summary', 'sup', 'tabindex',
        'table', 'target', 'tbody', 'td', 'text', 'textarea', 'tfoot', 'th',
        'thead', 'time', 'title', 'tr', 'tt', 'type', 'u', 'ul', 'url',
        'usemap', 'valign', 'value', 'valuetype', 'var', 'version', 'video',
        'vlink', 'vspace', 'width', 'wrap', 'xmp']

# Tags that usually have a new line inbetween them
NLINE_TAGS = ('body', 'head', 'html', 'ol', 'style', 'table', 'tbody', 'ul')

TAG_RE = re.compile("\<\s*([a-zA-Z][a-zA-Z0-9]*)")

PHP_AREA = [wx.stc.STC_HPHP_COMMENT, wx.stc.STC_HPHP_COMMENTLINE,
            wx.stc.STC_HPHP_COMPLEX_VARIABLE, wx.stc.STC_HPHP_DEFAULT,
            wx.stc.STC_HPHP_HSTRING, wx.stc.STC_HPHP_HSTRING_VARIABLE,
            wx.stc.STC_HPHP_NUMBER, wx.stc.STC_HPHP_OPERATOR,
            wx.stc.STC_HPHP_SIMPLESTRING,
            wx.stc.STC_HPHP_VARIABLE, wx.stc.STC_HPHP_WORD]

HTML_AREA = [wx.stc.STC_H_ASP, wx.stc.STC_H_ASPAT, wx.stc.STC_H_ATTRIBUTE,
             wx.stc.STC_H_ATTRIBUTEUNKNOWN, wx.stc.STC_H_CDATA,
             wx.stc.STC_H_COMMENT, wx.stc.STC_H_DEFAULT,
             wx.stc.STC_H_DOUBLESTRING, wx.stc.STC_H_ENTITY,
             wx.stc.STC_H_NUMBER, wx.stc.STC_H_OTHER, wx.stc.STC_H_QUESTION,
             wx.stc.STC_H_SCRIPT, wx.stc.STC_H_SGML_1ST_PARAM,
             wx.stc.STC_H_SGML_1ST_PARAM_COMMENT,
             wx.stc.STC_H_SGML_BLOCK_DEFAULT, wx.stc.STC_H_SGML_COMMAND,
             wx.stc.STC_H_SGML_COMMENT, wx.stc.STC_H_SGML_DEFAULT,
             wx.stc.STC_H_SGML_DOUBLESTRING, wx.stc.STC_H_SGML_ENTITY,
             wx.stc.STC_H_SGML_ERROR, wx.stc.STC_H_SGML_SIMPLESTRING,
             wx.stc.STC_H_SGML_SPECIAL, wx.stc.STC_H_SINGLESTRING,
             wx.stc.STC_H_TAG, wx.stc.STC_H_TAGEND,
             wx.stc.STC_H_TAGUNKNOWN, wx.stc.STC_H_VALUE,
             wx.stc.STC_H_XCCOMMENT, wx.stc.STC_H_XMLEND,
             wx.stc.STC_H_XMLSTART]

#--------------------------------------------------------------------------#

class Completer(completer.BaseCompleter):
    """HTML/XML Code completion provider"""
    def __init__(self, stc_buffer):
        super(Completer, self).__init__(stc_buffer)

        # Setup
        self.SetAutoCompKeys([ord('>'), ord('<')])
        self.SetAutoCompStops(' ')
        self.SetAutoCompFillups('')

    def GetAutoCompList(self, command):
        """Returns the list of possible completions for a
        command string. If namespace is not specified the lookup
        is based on the locals namespace
        @param command: command lookup is done on
        @keyword namespace: namespace to do lookup in

        """
        if command in [None, u'']:
            return list()

        buff = self.GetBuffer()
        cpos = buff.GetCurrentPos()

        # Check if we are in a php region or not
        if buff.GetStyleAt(cpos) not in HTML_AREA:
            return list()

        # Get current context
        cline = buff.GetCurrentLine()
        ccol = buff.GetColumn(cpos)
        tmp = buff.GetLine(cline).rstrip()
        if ccol < len(tmp):
            tmp = tmp[:ccol].rstrip()

        # Check if we are completing an open tag (i.e < was typed)
        if tmp.endswith('<'):
            if buff.GetLexer() == wx.stc.STC_LEX_XML:
                taglst = _FindXmlTags(buff.GetText())
            else:
                taglst = TAGS
            return completer.CreateSymbols(taglst, completer.TYPE_ELEMENT)

        # Check for a self closing tag (i.e />)
        endchk = tmp.strip().replace(u" ", u"").replace(u"\t", u"")
        if endchk.endswith(u"/>"):
            return list()

        # Try to autocomplete a closing tag (if necessary)
        tmp = tmp.rstrip('>').rstrip()
        if len(tmp) and (tmp[-1] in '"\' \t' or tmp[-1].isalpha()):
            # Walk backwards from the current line
            for line in range(cline, -1, -1):
                txt = buff.GetLine(line)
                if line == cline:
                    txt = txt[:buff.GetColumn(cpos)]

                idx = txt.rfind('<')
                if idx != -1:
                    parts = txt[idx:].lstrip('<').strip().split()
                    if len(parts):
                        tag = parts[0].rstrip('>')
                        if len(tag) and \
                           tag not in ('img', 'br', '?php', '?xml', '?') and \
                           not tag[0] in ('!', '/'):
                            rtag = u"</" + tag + u">"

                            if not parts[-1].endswith('>'):
                                rtag = u">" + rtag
                            return [ completer.Symbol(rtag, completer.TYPE_ELEMENT) ]
                    break

        return list()

    def OnCompletionInserted(self, pos, text):
        """Handle adjusting caret position after some insertions.
        @param pos: position caret was at before insertion
        @param text: text that was inserted

        """
        buff = self.GetBuffer()
        if text.strip().startswith(u"</"):
            buff.SetCurrentPos(pos) # move caret back between the tags
            # HACK: SetCurrentPos causes text to be selected
            buff.SetSelection(pos, pos)

#--------------------------------------------------------------------------#

def _FindXmlTags(text):
    """Dynamically generate a list of possible xml tags based on tags found in
    the given text.
    @param text: string
    @return: sorted list

    """
    matches = TAG_RE.findall(text)
    if len(matches):
        matches.append(u'!--')
        matches = list(set(matches))
        matches.sort()
    else:
        matches = [u'!--', ]
    return matches
