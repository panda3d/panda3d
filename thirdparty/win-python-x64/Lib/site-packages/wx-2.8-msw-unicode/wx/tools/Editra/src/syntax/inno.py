###############################################################################
# Name: inno.py                                                               #
# Purpose: Syntax configuration module for Inno Setup Scripts                 #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: inno.py
AUTHOR: Cody Preord
@summary: Lexer configuration module for Inno Setup Scripts

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: inno.py 55356 2008-08-29 16:27:34Z CJP $"
__revision__ = "$Revision: 55356 $"

#-----------------------------------------------------------------------------#
# Imports
import re

# Syntax Package Imports
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
SECTION_KW = (0, "code components custommessages dirs files icons ini "
                 "installdelete langoptions languages messages registry run "
                 "setup types tasks uninstalldelete uninstallrun _istool")

KEYWORDS = (1, "allowcancelduringinstall allownoicons allowrootdirectory "
            "allowuncpath alwaysrestart alwaysshowcomponentslist "
            "alwaysshowdironreadypage alwaysshowgrouponreadypage "
            "alwaysusepersonalgroup appcomments appcontact appcopyright "
            "appenddefaultdirname appenddefaultgroupname appid appmodifypath "
            "appmutex appname apppublisher apppublisherurl appreadmefile "
            "appsupporturl appupdatesurl appvername appversion "
            "architecturesallowed architecturesinstallin64bitmode backcolor "
            "backcolor2 backcolordirection backsolid changesassociations "
            "changesenvironment compression copyrightfontname "
            "copyrightfontsize createappdir createuninstallregkey "
            "defaultdirname defaultgroupname defaultuserinfoname "
            "defaultuserinfoorg defaultuserinfoserial dialogfontname "
            "dialogfontsize direxistswarning disabledirpage "
            "disablefinishedpage disableprogramgrouppage disablereadymemo "
            "disablereadypage disablestartupprompt diskclustersize "
            "diskslicesize diskspanning enablesdirdoesntexistwarning "
            "encryption extradiskspacerequired flatcomponentslist "
            "infoafterfile infobeforefile internalcompresslevel "
            "languagedetectionmethod languagecodepage languageid languagename "
            "licensefile mergeduplicatefiles minversion onlybelowversion "
            "outputbasefilename outputdir outputmanifestfile password "
            "privilegesrequired reservebytes restartifneededbyrun "
            "setupiconfile showcomponentsizes showlanguagedialog "
            "showtaskstreelines slicesperdisk solidcompression sourcedir "
            "timestamprounding timestampsinutc titlefontname titlefontsize "
            "touchdate touchtime uninstallable uninstalldisplayicon "
            "uninstalldisplayname uninstallfilesdir uninstalllogmode "
            "uninstallrestartcomputer updateuninstalllogappname "
            "usepreviousappdir usepreviousgroup useprevioussetuptype "
            "useprevioustasks useprevioususerinfo userinfopage usesetupldr "
            "versioninfocompany versioninfocopyright versioninfodescription "
            "versioninfotextversion versioninfoversion welcomefontname "
            "welcomefontsize windowshowcaption windowstartmaximized "
            "windowresizable windowvisible wizardimagebackcolor "
            "wizardimagefile wizardimagestretch wizardsmallimagefile")

PARAM_KW = (2, "afterinstall attribs beforeinstall check comment components "
               "copymode description destdir destname excludes "
               "extradiskspacerequired filename flags fontinstall "
               "groupdescription hotkey infoafterfile infobeforefile "
               "iconfilename iconindex key languages licensefile messagesfile "
               "minversion name onlybelowversion parameters permissions root "
               "runonceid section source statusmsg string subkey tasks type "
               "types valuedata valuename valuetype workingdir")

PREPROC_KW = (3, "append define dim else emit endif endsub error expr file for "
                 "if ifdef ifexist ifndef ifnexist include insert pragma sub "
                 "undef")

PASCAL_KW = (4, "begin break case const continue do downto else end except "
                "finally for function if of procedure repeat then to try until "
                "uses var while with")

USER_DEF = (5, "")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_INNO_COMMENT', 'comment_style'),
                ('STC_INNO_COMMENT_PASCAL', 'comment_style'),
                ('STC_INNO_DEFAULT', 'default_style'),
                ('STC_INNO_IDENTIFIER', 'default_style'),
                ('STC_INNO_KEYWORD', 'keyword_style'),
                ('STC_INNO_KEYWORD_PASCAL', 'keyword4_style'),
                ('STC_INNO_KEYWORD_USER', 'default_style'),
                ('STC_INNO_PARAMETER', 'keyword2_style'),
                ('STC_INNO_PREPROC', 'pre_style'),
                ('STC_INNO_PREPROC_INLINE', 'pre_style'),
                ('STC_INNO_SECTION', 'scalar_style'),
                ('STC_INNO_STRING_DOUBLE', 'string_style'),
                ('STC_INNO_STRING_SINGLE', 'char_style')]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_COMP = ("fold.compact", "1")

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_INNO:
        return [SECTION_KW, KEYWORDS, PARAM_KW, PREPROC_KW, PASCAL_KW]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_INNO:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_INNO:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    # Note: Inno can also use pascal comments (i.e {})
    if lang_id == synglob.ID_LANG_INNO:
        return [u';']
    else:
        return list()

#---- End Required Module Functions ----#

def AutoIndenter(stc, pos, ichar):
    """Auto indent Inno Setup Scripts. uses \n the text buffer will
    handle any eol character formatting.
    @param stc: EditraStyledTextCtrl
    @param pos: current carat position
    @param ichar: Indentation character
    @return: string

    """
    rtxt = u''
    line = stc.GetCurrentLine()
    text = stc.GetTextRange(stc.PositionFromLine(line), pos)

    indent = stc.GetLineIndentation(line)
    if ichar == u"\t":
        tabw = stc.GetTabWidth()
    else:
        tabw = stc.GetIndent()

    i_space = indent / tabw
    ndent = u"\n" + ichar * i_space
    rtxt = ndent + ((indent - (tabw * i_space)) * u' ')

    if_pat = re.compile('if\s+.*\sthen')
    text = text.strip()
    if text == u'begin' or if_pat.match(text):
        rtxt += ichar

    return rtxt

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
