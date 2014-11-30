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
__svnid__ = "$Id: html.py 59542 2009-03-15 00:23:37Z CJP $"
__revision__ = "$Revision: 59542 $"

#-----------------------------------------------------------------------------#
# Dependancies
import synglob

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
import javascript
JS_KEYWORDS = (1, javascript.KeywordString(synglob.ID_LANG_JS))

# VBScript Keywords (currently unsupported)
VBS_KEYWORDS = (2, "")

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
SYNTAX_ITEMS = [ ('STC_H_DEFAULT', 'default_style'),
                 ('STC_H_ASP', 'array_style'),
                 ('STC_H_ASPAT', 'array_style'),
                 ('STC_H_ATTRIBUTE', 'keyword2_style'),
                 ('STC_H_ATTRIBUTEUNKNOWN', 'error_style'),
                 ('STC_H_CDATA', 'default_style'), # Style ME
                 ('STC_H_COMMENT', 'comment_style'),
                 ('STC_H_DOUBLESTRING', 'string_style'),
                 ('STC_H_ENTITY', 'default_style'), # Style ME
                 ('STC_H_NUMBER', 'number_style'),
                 ('STC_H_OTHER', 'default_style'),  # Style ME
                 ('STC_H_QUESTION', 'scalar_style'),
                 ('STC_H_SCRIPT', 'funct_style'), # STYLE ME
                 ('STC_H_SGML_1ST_PARAM', 'keyword2_style'), # STYLE ME
                 ('STC_H_SGML_1ST_PARAM_COMMENT', 'comment_style'),
                 ('STC_H_SGML_BLOCK_DEFAULT', 'default_style'), # STYLE ME
                 ('STC_H_SGML_COMMAND', 'keyword_style'), # STYLE ME
                 ('STC_H_SGML_COMMENT', 'comment_style'),
                 ('STC_H_SGML_DEFAULT', 'array_style'), # STYLE ME
                 ('STC_H_SGML_DOUBLESTRING', 'string_style'),
                 ('STC_H_SGML_ENTITY', 'default_style'), # STYLE ME
                 ('STC_H_SGML_ERROR', 'error_style'),
                 ('STC_H_SGML_SIMPLESTRING', 'string_style'),
                 ('STC_H_SGML_SPECIAL', 'default_style'), # STYLE ME
                 ('STC_H_SINGLESTRING', 'string_style'),
                 ('STC_H_TAG', 'keyword_style'),
                 ('STC_H_TAGEND', 'keyword_style'),
                 ('STC_H_TAGUNKNOWN', 'error_style'),
                 ('STC_H_VALUE', 'number_style'),
                 ('STC_H_XCCOMMENT', 'comment_style'),
                 ('STC_H_XMLEND', 'scalar_style'),
                 ('STC_H_XMLSTART', 'scalar_style'),
                 # Embedded JavaScript
                 ('STC_HJ_COMMENT', 'comment_style'),
                 ('STC_HJ_COMMENTDOC', 'comment_style'),
                 ('STC_HJ_COMMENTLINE', 'comment_style'),
                 ('STC_HJ_DEFAULT', 'default_style'),
                 ('STC_HJ_DOUBLESTRING', 'default_style'), # STYLE ME
                 ('STC_HJ_KEYWORD', 'default_style'), # STYLE ME
                 ('STC_HJ_NUMBER', 'default_style'), # STYLE ME
                 ('STC_HJ_REGEX', 'default_style'), # STYLE ME
                 ('STC_HJ_SINGLESTRING', 'default_style'), # STYLE ME
                 ('STC_HJ_START', 'default_style'), # STYLE ME
                 ('STC_HJ_STRINGEOL', 'default_style'), # STYLE ME
                 ('STC_HJ_SYMBOLS', 'default_style'), # STYLE ME
                 ('STC_HJ_WORD', 'default_style'), # STYLE ME
                 ('STC_HJA_COMMENT', 'comment_style'),
                 ('STC_HJA_COMMENTDOC', 'comment_style'),
                 ('STC_HJA_COMMENTLINE', 'comment_style'),
                 ('STC_HJA_DEFAULT', 'default_style'),
                 ('STC_HJA_DOUBLESTRING', 'default_style'), # STYLE ME
                 ('STC_HJA_KEYWORD', 'default_style'), # STYLE ME
                 ('STC_HJA_NUMBER', 'default_style'), # STYLE ME
                 ('STC_HJA_REGEX', 'default_style'), # STYLE ME # STYLE ME
                 ('STC_HJA_SINGLESTRING', 'default_style'), # STYLE ME
                 ('STC_HJA_START', 'default_style'), # STYLE ME
                 ('STC_HJA_STRINGEOL', 'default_style'), # STYLE ME
                 ('STC_HJA_SYMBOLS', 'default_style'), # STYLE ME
                 ('STC_HJA_WORD', 'default_style')  ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FLD_HTML = ("fold.html", "1")

#------------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_COLDFUSION:
        return [(HTML_TAGS[0], HTML_TAGS[1] + " " + CF_TAGS), JS_KEYWORDS]
    else:
        return [HTML_TAGS, JS_KEYWORDS, SGML_KEYWORDS]

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    return SYNTAX_ITEMS + javascript.SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    return [FOLD, FLD_HTML]

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    return [u'<!--', u'-->']
#---- End Required Functions ----#

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
