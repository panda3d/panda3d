###############################################################################
# Name: postscript.py                                                         #
# Purpose: Define Postscript syntax for highlighting and other features       #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: postscript.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for PostScript. (case sensitive)
@todo: l3 keywords and ghostscript

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: postscript.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# PS Level 1 Operators
PS_L1 = (0, "$error = == FontDirectory StandardEncoding UserObjects abs add "
            "aload anchorsearch and arc arcn arcto array ashow astore atan "
            "awidthshow begin bind bitshift bytesavailable cachestatus ceiling "
            "charpath clear cleardictstack cleartomark clip clippath closefile "
            "closepath concat concatmatrix copy copypage cos count "
            "countdictstack countexecstack counttomark currentcmykcolor "
            "currentcolorspace currentdash currentdict currentfile currentflat "
            "currentfont currentgray currenthsbcolor currentlinecap "
            "currentlinejoin currentlinewidth currentmatrix currentmiterlimit "
            "currentpagedevice currentpoint currentrgbcolor currentscreen "
            "currenttransfer cvi cvlit cvn cvr cvrs cvs cvx def defaultmatrix "
            "definefont dict dictstack div dtransform dup echo end eoclip "
            "eofill eq erasepage errordict exch exec execstack executeonly "
            "executive exit exp false file fill findfont flattenpath floor "
            "flush flushfile for forall ge get getinterval grestore "
            "grestoreall gsave gt idetmatrix idiv idtransform if ifelse image "
            "imagemask index initclip initgraphics initmatrix inustroke "
            "invertmatrix itransform known kshow le length lineto ln load log "
            "loop lt makefont mark matrix maxlength mod moveto mul ne neg "
            "newpath noaccess nor not null nulldevice or pathbbox pathforall "
            "pop print prompt pstack put putinterval quit rand rcheck rcurveto "
            "read readhexstring readline readonly readstring rectstroke repeat "
            "resetfile restore reversepath rlineto rmoveto roll rotate round "
            "rrand run save scale scalefont search setblackgeneration "
            "setcachedevice setcachelimit setcharwidth setcolorscreen "
            "setcolortransfer setdash setflat setfont setgray sethsbcolor "
            "setlinecap setlinejoin setlinewidth setmatrix setmiterlimit "
            "setpagedevice setrgbcolor setscreen settransfer setvmthreshold "
            "show showpage sin sqrt srand stack start status statusdict stop "
            "stopped store string stringwidth stroke strokepath sub systemdict "
            "token token transform translate true truncate type ueofill "
            "undefineresource userdict usertime version vmstatus wcheck where "
            "widthshow write writehexstring writestring xcheck xor")

# PS Level 2 Operators
PS_L2 = (1, "GlobalFontDirectory ISOLatin1Encoding SharedFontDirectory "
            "UserObject arct colorimage cshow currentblackgeneration "
            "currentcacheparams currentcmykcolor currentcolor "
            "currentcolorrendering currentcolorscreen currentcolorspace "
            "currentcolortransfer currentdevparams currentglobal currentgstate "
            "currenthalftone currentobjectformat currentoverprint "
            "currentpacking currentpagedevice currentshared "
            "currentstrokeadjust currentsystemparams currentundercolorremoval "
            "currentuserparams defineresource defineuserobject deletefile "
            "execform execuserobject filenameforall fileposition filter "
            "findencoding findresource gcheck globaldict glyphshow gstate "
            "ineofill infill instroke inueofill inufill inustroke "
            "languagelevel makepattern packedarray printobject product "
            "realtime rectclip rectfill rectstroke renamefile resourceforall "
            "resourcestatus revision rootfont scheck selectfont serialnumber "
            "setbbox setblackgeneration setcachedevice2 setcacheparams "
            "setcmykcolor setcolor setcolorrendering setcolorscreen "
            "setcolorspace setcolortranfer setdevparams setfileposition "
            "setglobal setgstate sethalftone setobjectformat setoverprint "
            "setpacking setpagedevice setpattern setshared setstrokeadjust "
            "setsystemparams setucacheparams setundercolorremoval "
            "setuserparams setvmthreshold shareddict startjob uappend ucache "
            "ucachestatus ueofill ufill undef undefinefont undefineresource "
            "undefineuserobject upath ustroke ustrokepath vmreclaim "
            "writeobject xshow xyshow yshow")

# PS 3 Operators
PS_L3 = (2, "cliprestore clipsave composefont currentsmoothness "
            "findcolorrendering setsmoothness shfill")

# RIP-specific operators
RIP_OP = (3, ".begintransparencygroup .begintransparencymask .bytestring "
             ".charboxpath .currentaccuratecurves .currentblendmode "
             ".currentcurvejoin .currentdashadapt .currentdotlength "
             ".currentfilladjust2 .currentlimitclamp .currentopacityalpha "
             ".currentoverprintmode .currentrasterop .currentshapealpha "
             ".currentsourcetransparent .currenttextknockout "
             ".currenttexturetransparent .dashpath .dicttomark "
             ".discardtransparencygroup .discardtransparencymask "
             ".endtransparencygroup .endtransparencymask .execn .filename "
             ".filename .fileposition .forceput .forceundef .forgetsave "
             ".getbitsrect .getdevice .inittransparencymask .knownget "
             ".locksafe .makeoperator .namestring .oserrno .oserrorstring "
             ".peekstring .rectappend .runandhide .setaccuratecurves ."
             "setblendmode .setcurvejoin .setdashadapt .setdebug "
             ".setdefaultmatrix .setdotlength .setfilladjust2 .setlimitclamp "
             ".setmaxlength .setopacityalpha .setoverprintmode .setrasterop "
             ".setsafe .setshapealpha .setsourcetransparent .settextknockout "
             ".settexturetransparent .stringbreak .stringmatch .tempfile "
             ".type1decrypt .type1encrypt .type1execchar .unread arccos arcsin "
             "copydevice copyscanlines currentdevice finddevice findlibfile "
             "findprotodevice flushpage getdeviceprops getenv makeimagedevice "
             "makewordimagedevice max min putdeviceprops setdevice")

# User Defined Operators
USER_DEF = (4, "")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_PS_DEFAULT', 'default_style'),
                 ('STC_PS_BADSTRINGCHAR', 'unknown_style'),
                 ('STC_PS_BASE85STRING', 'string_style'),
                 ('STC_PS_COMMENT', 'comment_style'),
                 ('STC_PS_DSC_COMMENT', 'comment_style'),
                 ('STC_PS_DSC_VALUE', 'comment_style'), # STYLE ME
                 ('STC_PS_HEXSTRING', 'number_style'),
                 ('STC_PS_IMMEVAL', 'comment_style'), # STYLE ME
                 ('STC_PS_KEYWORD', 'class_style'),
                 ('STC_PS_LITERAL', 'scalar2_style'),
                 ('STC_PS_NAME', 'keyword_style'),
                 ('STC_PS_NUMBER', 'number_style'),
                 ('STC_PS_PAREN_ARRAY', 'default_style'), # STYLE ME
                 ('STC_PS_PAREN_DICT', 'default_style'), # STYLE ME
                 ('STC_PS_PAREN_PROC', 'default_style'), # STYLE ME
                 ('STC_PS_TEXT', 'default_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    return [PS_L1, PS_L2]

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    return SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    return [FOLD]

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    return [u'%']
#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
