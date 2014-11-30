###############################################################################
# Name: flagship.py                                                           #
# Purpose: Define Flagship syntax for highlighting and other features         #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: flagship.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for the Flagship programming language and
          other XBase dialects.
@todo: Custom style defs

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: flagship.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
FS_COMMANDS = (0, "? @ accept access all alternate announce ansi any append as "
                  "assign autolock average begin bell bitmap blank box call "
                  "cancel case century charset checkbox clear close cls color "
                  "combobox commit confirm console constant continue copy "
                  "count create cursor date dbread dbwrite decimals declare "
                  "default delete deleted delimiters device dir directory "
                  "display do draw edit else elseif eject end endcase enddo "
                  "endif endtext epoch erase error escape eval eventmask exact "
                  "exclusive extended external extra field file filter find "
                  "fixed font for form format from get gets global "
                  "global_extern go goto gotop guialign guicolor guicursor "
                  "guitransl html htmltext if image index input intensity join "
                  "key keyboard keytransl label lines list listbox local "
                  "locate margin memory memvar menu message method multibyte "
                  "multilocks next nfs nfslock nfs_force note on openerror "
                  "order outmode pack parameters path pixel pop printer "
                  "private prompt public push pushbutton quit radiobutton "
                  "radiogroup read recall refresh reindex relation release "
                  "rename replace report request restore richtext rowadapt "
                  "rowalign run save say scoreboard scrcompress screen seek "
                  "select sequence set setenhanced setstandard setunselected "
                  "skip softseek sort source static store struct structure sum "
                  "tag tbrowse text to total type typeahead unique unlock "
                  "update use wait while with wrap xml zap zerobyteout")

FS_STDLIB = (1, "_displarr _displarrerr _displarrstd _displobj _displobjerr "
                "_displobjstd aadd abs achoice aclone acopy adel adir "
                "aelemtype aeval afields afill ains alert alias alltrim altd "
                "ansi2oem appiomode appmdimode appobject array asc ascan asize "
                "asort at atail atanychar autoxlock between bin2i bin2l bin2w "
                "binand binlshift binor binrshift binxor bof break browse cdow "
                "chr chr2screen cmonth col col2pixel color2rgb colorselect "
                "colvisible consoleopen consolesize crc32 ctod curdir date "
                "datevalid day dbappend dbclearfilter dbclearindex "
                "dbclearrelation dbcloseall dbclosearea dbcommit dbcommitall "
                "dbcreate dbcreateindex dbdelete dbedit dbeval dbf dbfilter "
                "dbfinfo dbflock dbfused dbgetlocate dbgobottom dbgoto dbgotop "
                "dbobject dbrecall dbreindex dbrelation dbrlock dbrlocklist "
                "dbrselect dbrunlock dbseek dbselectarea dbsetdriver "
                "dbsetfilter dbsetindex dbsetlocate dbsetorder dbsetrelation "
                "dbskip dbstruct dbunlock dbunlockall dbusearea default "
                "deleted descend devout devoutpict devpos directory diskspace "
                "dispbegin dispbox dispcount dispend dispout doserror "
                "doserror2str dow drawline dtoc dtos empty eof errorblock "
                "errorlevel eval execname execpidnum exp fattrib fclose fcount "
                "fcreate ferase ferror ferror2str fieldblock fielddeci "
                "fieldget fieldgetarr fieldlen fieldname fieldpos fieldput "
                "fieldputarr fieldtype fieldwblock file findexefile fklabel "
                "fkmax flagship_dir flock flockf fopen found fread freadstdin "
                "freadstr freadtxt frename fs_set fseek fwrite getactive "
                "getalign getapplykey getdosetkey getenv getenvarr getfunction "
                "getpostvalid getprevalid getreader guidrawline hardcr header "
                "hex2num i2bin iif indexcheck indexcount indexdbf indexext "
                "indexkey indexnames indexord infobox inkey inkey2read "
                "inkey2str inkeytrap instdchar instdstring int int2num isalpha "
                "isbegseq iscolor isdbexcl isdbflock isdbmultip isdbmultiple "
                "isdbmultipleopen isdbrlock isdigit isfunction isguimode "
                "islower isobjclass isobjequiv isobjproperty isprinter isupper "
                "l2bin lastkey lastrec left len listbox lock log lower ltrim "
                "lupdate macroeval macrosubst max max_col max_row maxcol "
                "maxrow mcol mdblck mdiclose mdiopen mdiselect memocode "
                "memodecode memoedit memoencode memoline memoread memory "
                "memotran memowrit memvarblock mhide min minmax mlcount "
                "mlctopos mleftdown mlpos mod month mpostolc mpresent "
                "mreststate mrightdown mrow msavestate msetcursor msetpos "
                "mshow mstate neterr netname nextkey num2hex num2int objclone "
                "oem2ansi onkey ordbagext ordbagname ordcond ordcondset "
                "ordcreate orddescend orddestroy ordfor ordisinique ordkey "
                "ordkeyadd ordkeycount ordkeydel ordkeygoto ordkeyno ordkeyval "
                "ordlistadd ordlistclear ordlistrebui ordname ordnumber "
                "ordscope ordsetfocu ordsetrelat ordskipunique os outerr "
                "outstd padc padl padr param parameters pcalls pcol pcount "
                "pixel2col pixel2row printstatus procfile procline procname "
                "procstack proper prow qout qout2 qqout qqout2 rat rddlist "
                "rddname rddsetdefault readexit readinsert readkey readkill "
                "readmodal readsave readupdated readvar reccount recno recsize "
                "replicate restscreen right rlock rlockverify round row "
                "row2pixel rowadapt rowvisible rtrim savescreen scrdos2unix "
                "screen2chr scroll scrunix2dos seconds secondscpu select "
                "serial set setansi setblink setcancel setcol2get setcolor "
                "setcolorba setcursor setevent setguicursor setkey setmode "
                "setpos setprc setvarempty sleep sleepms soundex space sqrt "
                "statbarmsg statusmessage stod str strlen strlen2col "
                "strlen2pix strlen2space strpeek strpoke strtran strzero stuff "
                "substr tbcolumnnew tbmouse tbrowsearr tbrowsedb tbrowsenew "
                "tempfilename time tone transform trim truepath type updated "
                "upper used usersactive usersdbf usersmax val valtype version "
                "webdate weberrorhandler webgetenvir webgetformdata "
                "webhtmlbegin webhtmlend weblogerr webmaildomain weboutdata "
                "websendmail word year")

FS_FUNC = (2, "function procedure return exit")

FS_CLASS = (3, "class instance export hidden protect prototype")
#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_FS_ASM', ''),
                ('STC_FS_BINNUMBER', 'number_style'),
                ('STC_FS_COMMENT', 'comment_style'),
                ('STC_FS_COMMENTDOC', 'dockey_style'),
                ('STC_FS_COMMENTDOCKEYWORD', 'dockey_style'),
                ('STC_FS_COMMENTDOCKEYWORDERROR', 'error_style'),
                ('STC_FS_COMMENTLINE', 'comment_style'),
                ('STC_FS_COMMENTLINEDOC', 'comment_style'),
                ('STC_FS_CONSTANT', 'default_style'),
                ('STC_FS_DATE', 'default_style'),
                ('STC_FS_DEFAULT', 'default_style'),
                ('STC_FS_ERROR', 'error_style'),
                ('STC_FS_HEXNUMBER', 'number_style'),
                ('STC_FS_IDENTIFIER', 'default_style'),
                ('STC_FS_KEYWORD', 'keyword_style'),
                ('STC_FS_KEYWORD2', 'keyword2_style'),
                ('STC_FS_KEYWORD3', 'keyword3_style'),
                ('STC_FS_KEYWORD4', 'keyword4_style'),
                ('STC_FS_LABEL', 'default_style'),
                ('STC_FS_NUMBER', 'number_style'),
                ('STC_FS_OPERATOR', 'operator_style'),
                ('STC_FS_PREPROCESSOR', 'pre_style'),
                ('STC_FS_STRING', 'string_style'),
                ('STC_FS_STRINGEOL', 'stringeol_style')]

#---- Extra Properties ----#
FOLD = ('fold', '1')
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_FLAGSHIP:
        return [FS_COMMANDS, FS_STDLIB, FS_FUNC, FS_CLASS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_FLAGSHIP:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_FLAGSHIP:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_FLAGSHIP:
        return [u'//']
    else:
        return list()

#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
