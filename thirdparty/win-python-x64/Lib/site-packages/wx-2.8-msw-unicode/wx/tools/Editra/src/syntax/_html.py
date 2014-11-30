###############################################################################
# Name: html.py                                                               #
# Purpose: Define HTML syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: html.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for HTML/DHTML/SGML.
@todo: Add Netscape/Microsoft Tag Extenstions (maybe)
@todo: Styleing needs lots of tweaking

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _html.py 64591 2010-06-15 04:00:50Z CJP $"
__revision__ = "$Revision: 64591 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata
import _javascript
import _vbscript

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# HTML Tags (HTML4)
HTML_TAGS = (0, "address applet area a base basefont big blockquote br caption "
                "center cite code dd dfn dir div dl dt font form hr html img "
                "input isindex kbd li link map menu meta ol option param pre p "
                "samp span select small strike sub sup table td textarea th tr "
                "script noscript tt ul var xmp b i u h1 h2 h3 h4 h5 h6 em "
                "strong head body title "
                # HTML 4.0 Tags
                "abbr acronym bdo button col label colgroup del fieldset "
                "iframe ins legend object optgroup q s tbody tfoot thead "
                # HTML 5 Tags
                "article aside audio canvas command datalist details dialog "
                "embed figcaption figure footer header hgroup keygen mark "
                "meter nav output progress rp rt ruby section source time "
                "video "
                # Tag Attributes / Arguments
                "action align alink alt archive background bgcolor border "
                "bordercolor cellpadding cellspacing checked class clear "
                "codebase color cols colspan content coords enctype face "
                "gutter height hspace id link lowsrc marginheight marginwidth "
                "maxlength method name prompt rel rev rows rowspan scrolling "
                "selected shape size src start target text type url usemap "
                "ismap valign value vlink vspace width wrap href http-equiv "
                # HTML 4 Tag Attributes /Arguments
                "accept accesskey axis char charoff charset cite classid "
                "codetype compact data datetime declare defer dir disabled for "
                "frame headers hreflang lang language longdesc multiple nohref "
                "nowrap profile readonly rules scheme scope standby style "
                "summary tabindex valuetype version "
                # HTML 5 Tag Attributes / Arguments
                "async autocomplete contenteditable contextmenu date "
                "datetime-local draggable email formaction formenctype "
                "formmethod formnovalidate formtarget hidden list manifest max "
                "media min month novalidate number pattern ping range required "
                "reversed role sandbox scoped seamless search sizes spellcheck "
                "srcdoc step tel week "
                # DHTML Support
                "dtml-var dtml-if dtml-unless dtml-in dtml-with dtml-let "
                "dtml-call dtml-raise dtml-try dtml-comment dtml-tree")

#---- Extra defs ----#
# ColdFusion Tags
CF_TAGS = ("cfabort cfapplet cfapplication cfargument cfassociate cfbreak "
           "cfcache cfcalendar cfcase cfcatch cfchart cfchartdata "
           "cfchartseries cfcol cfcollection cfcomponent cfcontent cfcookie "
           "cfdefaultcase cfdirectory cfdocument cfdocumentitem "
           "cfdocumentsection cfdump cfelse cfelseif cferror cfexecute cfexit "
           "cffile cfflush cfform cfformgroup cfformitem cfftp cffunction "
           "cfgrid cfgridcolumn cfgridrow cfgridupdate cfheader cfhtmlhead "
           "cfhttp cfhttpparam cfif cfimport cfinclude cfindex cfinput "
           "cfinsert cfinvoke cfinvokeargument cfldap cflocation cflock cflog "
           "cflogin cfloginuser cflogout cfloop cfmail cfmailparam cfmailpart "
           "cfmodule cfNTauthenticate cfobject cfobjectcache cfoutput cfparam "
           "cfpop cfprocessingdirective cfprocparam cfprocresult cfproperty "
           "cfquery cfqueryparam cfregistry cfreport cfreportparam cfrethrow "
           "cfreturn cfsavecontent cfschedule cfscript cfsearch cfselect cfset "
           "cfsetting cfsilent cfslider cfstoredproc cfswitch cftable "
           "cftextarea cfthrow cftimer cftrace cftransaction cftree cftreeitem "
           "cftry cfupdate cfwddx cfxml")

# JavaScript Keywords (see javascript.py)
JS_KEYWORDS = (1, _javascript.KeywordString(synglob.ID_LANG_JS))

# VBScript Keywords (currently unsupported)
VBS_KEYWORDS = (2, _vbscript.VBS_KW)

# Python Keywords (see python.py)
PY_KEYWORDS = (3, "")

# PHP Keywords (see php.py)
# This module is loaded for files with a .html/htm extension so it is assumed
# that there is no php in the file. On the other hand the php module loads
# this module so that it can support embedded html. This behavior may be changed
# in the future

# XML Keywords (see xml.py)
# XML files are handled independantly from html although there is support for
# embedded xml highlighting it is currently not being used.

# SGML Keywords
SGML_KEYWORDS = (5, "ELEMENT DOCTYPE ATTLIST ENTITY NOTATION")
#SGML_KEYWORDS = (5, "#CURRENT #IMPLIED #REQUIRED ATTLIST CDATA DOCTYPE "
#                    "ELEMENT ENTITY HTML IDREF INCLUDE IGNORE NMTOKEN NUMBER "
#                    "RCDATA TEMP")

# SGML Block Keywords
SGML_BLOCK = (7, "")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (stc.STC_H_DEFAULT, 'default_style'),
                 (stc.STC_H_ASP, 'array_style'),
                 (stc.STC_H_ASPAT, 'array_style'),
                 (stc.STC_H_ATTRIBUTE, 'keyword2_style'),
                 (stc.STC_H_ATTRIBUTEUNKNOWN, 'error_style'),
                 (stc.STC_H_CDATA, 'default_style'), # Style ME
                 (stc.STC_H_COMMENT, 'comment_style'),
                 (stc.STC_H_DOUBLESTRING, 'string_style'),
                 (stc.STC_H_ENTITY, 'default_style'), # Style ME
                 (stc.STC_H_NUMBER, 'number_style'),
                 (stc.STC_H_OTHER, 'default_style'),  # Style ME
                 (stc.STC_H_QUESTION, 'scalar_style'),
                 (stc.STC_H_SCRIPT, 'funct_style'), # STYLE ME
                 (stc.STC_H_SGML_1ST_PARAM, 'keyword2_style'), # STYLE ME
                 (stc.STC_H_SGML_1ST_PARAM_COMMENT, 'comment_style'),
                 (stc.STC_H_SGML_BLOCK_DEFAULT, 'default_style'), # STYLE ME
                 (stc.STC_H_SGML_COMMAND, 'keyword_style'), # STYLE ME
                 (stc.STC_H_SGML_COMMENT, 'comment_style'),
                 (stc.STC_H_SGML_DEFAULT, 'array_style'), # STYLE ME
                 (stc.STC_H_SGML_DOUBLESTRING, 'string_style'),
                 (stc.STC_H_SGML_ENTITY, 'default_style'), # STYLE ME
                 (stc.STC_H_SGML_ERROR, 'error_style'),
                 (stc.STC_H_SGML_SIMPLESTRING, 'string_style'),
                 (stc.STC_H_SGML_SPECIAL, 'default_style'), # STYLE ME
                 (stc.STC_H_SINGLESTRING, 'string_style'),
                 (stc.STC_H_TAG, 'keyword_style'),
                 (stc.STC_H_TAGEND, 'keyword_style'),
                 (stc.STC_H_TAGUNKNOWN, 'error_style'),
                 (stc.STC_H_VALUE, 'number_style'),
                 (stc.STC_H_XCCOMMENT, 'comment_style'),
                 (stc.STC_H_XMLEND, 'scalar_style'),
                 (stc.STC_H_XMLSTART, 'scalar_style'),

                 # Embedded JavaScript
                 (stc.STC_HJ_COMMENT, 'comment_style'),
                 (stc.STC_HJ_COMMENTDOC, 'comment_style'),
                 (stc.STC_HJ_COMMENTLINE, 'comment_style'),
                 (stc.STC_HJ_DEFAULT, 'default_style'),
                 (stc.STC_HJ_DOUBLESTRING, 'default_style'), # STYLE ME
                 (stc.STC_HJ_KEYWORD, 'default_style'), # STYLE ME
                 (stc.STC_HJ_NUMBER, 'default_style'), # STYLE ME
                 (stc.STC_HJ_REGEX, 'default_style'), # STYLE ME
                 (stc.STC_HJ_SINGLESTRING, 'default_style'), # STYLE ME
                 (stc.STC_HJ_START, 'default_style'), # STYLE ME
                 (stc.STC_HJ_STRINGEOL, 'default_style'), # STYLE ME
                 (stc.STC_HJ_SYMBOLS, 'default_style'), # STYLE ME
                 (stc.STC_HJ_WORD, 'default_style'), # STYLE ME

                 (stc.STC_HJA_COMMENT, 'comment_style'),
                 (stc.STC_HJA_COMMENTDOC, 'comment_style'),
                 (stc.STC_HJA_COMMENTLINE, 'comment_style'),
                 (stc.STC_HJA_DEFAULT, 'default_style'),
                 (stc.STC_HJA_DOUBLESTRING, 'default_style'), # STYLE ME
                 (stc.STC_HJA_KEYWORD, 'default_style'), # STYLE ME
                 (stc.STC_HJA_NUMBER, 'default_style'), # STYLE ME
                 (stc.STC_HJA_REGEX, 'default_style'), # STYLE ME # STYLE ME
                 (stc.STC_HJA_SINGLESTRING, 'default_style'), # STYLE ME
                 (stc.STC_HJA_START, 'default_style'), # STYLE ME
                 (stc.STC_HJA_STRINGEOL, 'default_style'), # STYLE ME
                 (stc.STC_HJA_SYMBOLS, 'default_style'), # STYLE ME
                 (stc.STC_HJA_WORD, 'default_style'),
                 
                 (stc.STC_HBA_DEFAULT, 'operator_style'), # Styles ( ) ?
                 (stc.STC_HBA_COMMENTLINE, 'comment_style'),
                 (stc.STC_HBA_IDENTIFIER, 'default_style'), # TODO
                 (stc.STC_HBA_NUMBER, 'number_style'),
                 (stc.STC_HBA_START, 'default_style'), # TODO
                 (stc.STC_HBA_STRING, 'string_style'),
                 (stc.STC_HBA_STRINGEOL, 'stringeol_style'),
                 (stc.STC_HBA_WORD, 'keyword_style')  ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FLD_HTML = ("fold.html", "1")

#------------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Html and related languages""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_HTML)
        self.RegisterFeature(synglob.FEATURE_AUTOINDENT, AutoIndenter)

    def GetKeywords(self):
        """Returns Specified Keywords List"""
        if self.LangId == synglob.ID_LANG_COLDFUSION:
            return [(HTML_TAGS[0], HTML_TAGS[1] + " " + CF_TAGS), JS_KEYWORDS]
        else:
            return [HTML_TAGS, JS_KEYWORDS, SGML_KEYWORDS, VBS_KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications"""
        return SYNTAX_ITEMS + _javascript.SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set"""
        return [FOLD, FLD_HTML]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code"""
        return [u'<!--', u'-->']

#-----------------------------------------------------------------------------#

def AutoIndenter(estc, pos, ichar):
    """Auto indent python code.
    @param estc: EditraStyledTextCtrl
    @param pos: current carat position
    @param ichar: Indentation character

    """
    rtxt = u''
    line = estc.GetCurrentLine()
    spos = estc.PositionFromLine(line)
    text = estc.GetTextRange(spos, pos)
    eolch = estc.GetEOLChar()
    inspace = text.isspace()

    # Cursor is in the indent area somewhere
    if inspace:
        estc.AddText(eolch + text)
        return

    # Check if the cursor is in column 0 and just return newline.
    if not len(text):
        estc.AddText(eolch)
        return

    if ichar == u"\t":
        tabw = estc.GetTabWidth()
    else:
        tabw = estc.GetIndent()

    # Standard indent to match previous line
    indent = estc.GetLineIndentation(line)
    levels = indent / tabw
    end_spaces = ((indent - (tabw * levels)) * u" ")
    rtxt = eolch + (ichar * levels) + end_spaces

    # Check if we need some 'special' indentation
    tmp = text.rstrip()
    if tmp.endswith(u">"):
        # At a tag check for if we need extra indentation
        tagstart = tmp.rfind(u"<")
        if tagstart >= 0:
            tagval = tmp[tagstart:]
            if not tagval.startswith(u"</") and \
               not tagval.endswith(u"/>") and \
               not tagval.endswith(u"?>"):
                # Cursor is after an opening tag so we need to indent more
                # First match to the starting tag
                levels = (tagstart / tabw) # Add an extra level
                end_spaces = ((tagstart - (tabw * levels)) * u" ")
                rtxt = eolch + (ichar * (levels+1)) + end_spaces

    # Put text in the buffer
    estc.AddText(rtxt)

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @param option: specific subset of keywords to get

    """
    if option == synglob.ID_LANG_SGML:
        return SGML_KEYWORDS[1]
    else:
        return HTML_TAGS[1]

#---- End Syntax Modules Internal Functions ----#
