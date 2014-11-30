###############################################################################
# Name: sh.py                                                                 #
# Purpose: Define Bourne/Bash/Csh/Korn Shell syntaxes for highlighting and    #
#          other features.                                                    #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: sh.py
AUTHOR: Cody Precord
@summary: Lexer configuration file for Bourne, Bash, Kornshell and
          C-Shell scripts.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: sh.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

# Bourne Shell Keywords (bash and kornshell have these too)
COMM_KEYWORDS = ("break eval newgrp return ulimit cd exec pwd shift umask "
                 "chdir exit read test wait continue kill readonly trap "
                 "contained elif else then case esac do done for in if fi "
                 "until while set export unset")

# Bash/Kornshell extensions (in bash/kornshell but not bourne)
EXT_KEYWORDS = ("function alias fg integer printf times autoload functions "
                "jobs r true bg getopts let stop type false hash nohup suspend "
                "unalias fc history print time whence typeset while select")

# Bash Only Keywords
BSH_KEYWORDS = ("bind disown local popd shopt builtin enable logout pushd "
                "source dirs help declare")

# Bash Shell Commands (statements)
BCMD_KEYWORDS = ("chmod chown chroot clear du egrep expr fgrep find gnufind "
                 "gnugrep grep install less ls mkdir mv reload restart rm "
                 "rmdir rpm sed su sleep start status sort strip tail touch "
                 "complete stop echo")

# Korn Shell Only Keywords
KSH_KEYWORDS = "login newgrp"

# Korn Shell Commands (statements)
KCMD_KEYWORDS = ("cat chmod chown chroot clear cp du egrep expr fgrep find "
                 "grep install killall less ls mkdir mv nice printenv rm rmdir "
                 "sed sort strip stty su tail touch tput")

# C-Shell Keywords
CSH_KEYWORDS = ("alias cd chdir continue dirs echo break breaksw foreach end "
                "eval exec exit glob goto case default history kill login "
                "logout nice nohup else endif onintr popd pushd rehash repeat "
                "endsw setenv shift source time umask switch unalias unhash "
                "unsetenv wait")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_SH_DEFAULT', 'default_style'),
                 ('STC_SH_BACKTICKS', 'scalar_style'),
                 ('STC_SH_CHARACTER', 'char_style'),
                 ('STC_SH_COMMENTLINE', 'comment_style'),
                 ('STC_SH_ERROR', 'error_style'),
                 ('STC_SH_HERE_DELIM', 'here_style'),
                 ('STC_SH_HERE_Q', 'here_style'),
                 ('STC_SH_IDENTIFIER', 'default_style'),
                 ('STC_SH_NUMBER', 'number_style'),
                 ('STC_SH_OPERATOR', 'operator_style'),
                 ('STC_SH_PARAM', 'scalar_style'),
                 ('STC_SH_SCALAR', 'scalar_style'),
                 ('STC_SH_STRING', 'string_style'),
                 ('STC_SH_WORD', 'keyword_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FLD_COMMENT = ("fold.comment", "1")
FLD_COMPACT = ("fold.compact", "0")

#------------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    keywords = list()
    keyw_str = [COMM_KEYWORDS]
    if lang_id == synglob.ID_LANG_CSH:
        keyw_str.append(CSH_KEYWORDS)
    else:
        if lang_id != synglob.ID_LANG_BOURNE:
            keyw_str.append(EXT_KEYWORDS)
        if lang_id == synglob.ID_LANG_BASH:
            keyw_str.append(BSH_KEYWORDS)
            keyw_str.append(BCMD_KEYWORDS)
        elif lang_id == synglob.ID_LANG_KSH:
            keyw_str.append(KSH_KEYWORDS)
            keyw_str.append(KCMD_KEYWORDS)
        else:
            pass
    keywords.append((0, " ".join(keyw_str)))
    return keywords

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    return SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    return [FOLD, FLD_COMMENT, FLD_COMPACT]

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    return [u'#']
#---- End Required Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
