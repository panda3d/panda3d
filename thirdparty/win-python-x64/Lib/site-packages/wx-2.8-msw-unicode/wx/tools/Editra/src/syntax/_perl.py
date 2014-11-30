###############################################################################
# Name: perl.py                                                               #
# Purpose: Define Perl syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: perl.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Perl.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _perl.py 66108 2010-11-10 21:04:54Z CJP $"
__revision__ = "$Revision: 66108 $"

#-----------------------------------------------------------------------------#
# Imports
import wx
import wx.stc as stc

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Perl Keywords
PERL_KW = (0, "if elseif unless else switch eq ne gt lt ge le cmp not and or "
              "xor while for foreach do until continue defined undef and or "
              "not bless ref BEGIN END my local our goto return last next redo "
              "chomp chop chr crypt index lc lcfirst length org pack reverse "
              "rindex sprintf substr uc ucfirst pos quotemet split study abs "
              "atan2 cos exp hex int log oct rand sin sqrt srand spice unshift "
              "shift push pop split join reverse grep map sort unpack each "
              "exists keys values tie tied untie carp confess croak dbmclose "
              "dbmopen die syscall binmode close closedir eof fileno getc "
              "lstat print printf readdir readline readpipe rewinddir select "
              "stat tell telldir write fcntl flock ioctl open opendir read "
              "seek seekdir sysopen sysread sysseek syswrite truncate pack vec "
              "chdir chmod chown chroot glob link mkdir readlink rename rmdir "
              "symlink umask ulink utime caller dump eval exit wanarray "
              "import alarm exec fork getpgrp getppid getpriority kill pipe "
              "setpgrp setpriority sleep system times wait waitpid accept "
              "bind connect getpeername getsockname getsockopt listen recv "
              "send setsockopt shutdown socket socketpair msgctl msgget msgrcv "
              "msgsnd semctl semget semop shmctl shmget shmread shmwrite "
              "endhostent endnetent endprooent endservent gethostbyaddr "
              "gethostbyname gethostent getnetbyaddr getnetbyname getnetent "
              "getprotobyname getprotobynumber getprotoent getervbyname time "
              "getservbyport getservent sethostent setnetent setprotoent "
              "setservent getpwuid getpwnam getpwent setpwent endpwent "
              "getgrgid getlogin getgrnam setgrent endgrent gtime localtime "
              "times warn formline reset scalar delete prototype lock new "
              "NULL __FILE__ __LINE__ __PACKAGE__ __DATA__ __END__ AUTOLOAD "
              "BEGIN CORE DESTROY END EQ GE GT INIT LE LT NE CHECK use sub "
              "elsif require getgrent ")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (stc.STC_PL_DEFAULT, 'default_style'),
                 (stc.STC_PL_ARRAY, 'array_style'),
                 (stc.STC_PL_BACKTICKS, 'btick_style'),
                 (stc.STC_PL_CHARACTER, 'char_style'),
                 (stc.STC_PL_COMMENTLINE, 'comment_style'),
                 (stc.STC_PL_DATASECTION, 'default_style'), # STYLE ME
                 (stc.STC_PL_ERROR, 'error_style'),
                 (stc.STC_PL_HASH, 'global_style'),
                 (stc.STC_PL_HERE_DELIM, 'here_style'),
                 (stc.STC_PL_HERE_Q, 'here_style'),
                 (stc.STC_PL_HERE_QQ, 'here_style'),
                 (stc.STC_PL_HERE_QX, 'here_style'),
                 (stc.STC_PL_IDENTIFIER, 'default_style'),
                 (stc.STC_PL_LONGQUOTE, 'default_style'), # STYLE ME
                 (stc.STC_PL_NUMBER, 'number_style'),
                 (stc.STC_PL_OPERATOR, 'operator_style'),
                 (stc.STC_PL_POD, 'comment_style'),
                 (stc.STC_PL_PREPROCESSOR,  'pre_style' ),
                 (stc.STC_PL_PUNCTUATION, 'default_style'), # STYLE ME
                 (stc.STC_PL_REGEX, 'regex_style'),
                 (stc.STC_PL_REGSUBST, 'regex_style'),
                 (stc.STC_PL_SCALAR, 'scalar_style'),
                 (stc.STC_PL_STRING, 'string_style'),
                 (stc.STC_PL_STRING_Q, 'string_style'),
                 (stc.STC_PL_STRING_QQ, 'string_style'),
                 (stc.STC_PL_STRING_QR, 'string_style'),
                 (stc.STC_PL_STRING_QW, 'string_style'),
                 (stc.STC_PL_STRING_QX, 'string_style'),
                 (stc.STC_PL_SYMBOLTABLE, 'default_style'), # STYLE ME
                 (stc.STC_PL_WORD, 'keyword_style') ]

if wx.VERSION >= (2, 9, 0, 0, ''):
    SYNTAX_ITEMS.append((stc.STC_PL_FORMAT, 'default_style')) #TODO
    SYNTAX_ITEMS.append((stc.STC_PL_FORMAT_IDENT, 'default_style')) #TODO
    SYNTAX_ITEMS.append((stc.STC_PL_SUB_PROTOTYPE, 'default_style')) #TODO

#---- Extra Properties ----#
FOLD = ("fold", "1")
FLD_COMPACT = ("fold.compact", "1")
FLD_COMMENT = ("fold.comment", "1")
FLD_POD = ("fold.perl.pod", "1")
FLD_PKG = ("fold.perl.package", "1")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Perl""" 
    def __init__(self, langid):
        super(SyntaxData, self).__init__(langid)

        # Setup
        self.SetLexer(stc.STC_LEX_PERL)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [PERL_KW]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'#']

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    if option == synglob.ID_LANG_PERL:
        return PERL_KW[1]
    else:
        return u''

#---- End Syntax Modules Internal Functions ----#

#-----------------------------------------------------------------------------#
