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
__svnid__ = "$Id: _inno.py 66104 2010-11-10 20:18:29Z CJP $"
__revision__ = "$Revision: 66104 $"

#-----------------------------------------------------------------------------#
# Imports
import wx
import wx.stc as stc
import re

# Local Imports
import synglob
import syndata

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
SYNTAX_ITEMS = [(stc.STC_INNO_COMMENT, 'comment_style'),
                (stc.STC_INNO_COMMENT_PASCAL, 'comment_style'),
                (stc.STC_INNO_DEFAULT, 'default_style'),
                (stc.STC_INNO_IDENTIFIER, 'default_style'),
                (stc.STC_INNO_KEYWORD, 'keyword_style'),
                (stc.STC_INNO_KEYWORD_PASCAL, 'keyword4_style'),
                (stc.STC_INNO_KEYWORD_USER, 'default_style'),
                (stc.STC_INNO_PARAMETER, 'keyword2_style'),
                (stc.STC_INNO_PREPROC, 'pre_style'),
                (stc.STC_INNO_SECTION, 'scalar_style'),
                (stc.STC_INNO_STRING_DOUBLE, 'string_style'),
                (stc.STC_INNO_STRING_SINGLE, 'char_style')]

if wx.VERSION >= (2, 9, 0, 0, ''):
    SYNTAX_ITEMS.append((stc.STC_INNO_INLINE_EXPANSION, 'default_style')) #TODO
else:
    SYNTAX_ITEMS.append((stc.STC_INNO_PREPROC_INLINE, 'pre_style'))

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_COMP = ("fold.compact", "1")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Inno Setup Scripts""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_INNOSETUP)
        self.RegisterFeature(synglob.FEATURE_AUTOINDENT, AutoIndenter)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [SECTION_KW, KEYWORDS, PARAM_KW, PREPROC_KW, PASCAL_KW]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        # Note: Inno can also use pascal comments (i.e {})
        return [u';']

#-----------------------------------------------------------------------------#

def AutoIndenter(estc, pos, ichar):
    """Auto indent Inno Setup Scripts.
    @param estc: EditraStyledTextCtrl
    @param pos: current carat position
    @param ichar: Indentation character

    """
    rtxt = u''
    line = estc.GetCurrentLine()
    text = estc.GetTextRange(estc.PositionFromLine(line), pos)
    eolch = estc.GetEOLChar()

    indent = estc.GetLineIndentation(line)
    if ichar == u"\t":
        tabw = estc.GetTabWidth()
    else:
        tabw = estc.GetIndent()

    i_space = indent / tabw
    ndent = eolch + ichar * i_space
    rtxt = ndent + ((indent - (tabw * i_space)) * u' ')

    if_pat = re.compile('if\s+.*\sthen')
    text = text.strip()
    if text == u'begin' or if_pat.match(text):
        rtxt += ichar

    # Put text in the buffer
    estc.AddText(rtxt)
