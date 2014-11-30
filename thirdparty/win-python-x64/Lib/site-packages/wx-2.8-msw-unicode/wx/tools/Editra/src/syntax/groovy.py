###############################################################################
# Name: groovy.py                                                             #
# Purpose: Define Groovy syntax for highlighting and other features           #
# Author: Omar Gomez <omar.gomez@gmail.com>                                   #
# Copyright: (c) 2009 Omar Gomez <omar.gomez@gmail.com>                       #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: groovy.py
AUTHOR: Omar Gomez
@summary: Lexer configuration module for Groovy (based on the Java one).

"""

__author__ = "Omar Gomez <omar.gomez@gmail.com>"
__svnid__ = "$Id: groovy.py 60019 2009-04-04 18:58:40Z CJP $"
__revision__ = "$Revision: 60019 $"

#-----------------------------------------------------------------------------#
# Local Imports
import synglob
from cpp import AutoIndenter

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

MAIN_KEYWORDS = (0, 
"""
as
assert
Boolean
Byte
Character
Class
Double
Float
Integer
Long
Number
Object
Short
String
property
void
abstract
assert
boolean
break
byte
case
catch
char
class
const
continue
default
do
double
else
extends
false
final
finally
float
for
goto
if
implements
import
instanceof
in
int
interface
long
native
new
null
package
private
protected
public
return
short
static
strictfp
super
switch
synchronized
this
throw
throws
transient
true
try
void
volatile
while
def
"""
)

SECONDARY_KEYWORDS= (1, 
"""
abs
accept
allProperties
and
any
append
asImmutable
asSynchronized
asWritable
center
collect
compareTo
contains
count
decodeBase64
div
dump
each
eachByte
eachFile
eachFileRecurse
eachLine
eachMatch
eachProperty
eachPropertyName
eachWithIndex
encodeBase64
every
execute
filterLine
find
findAll
findIndexOf
flatten
getErr
getIn
getOut
getText
inject
inspect
intersect
intdiv
invokeMethod
isCase
join
leftShift
max
min
minus
mod
multiply
negate
newInputStream
newOutputStream
newPrintWriter
newReader
newWriter
next
or
padLeft
padRight
plus
pop
previous
print
println
readBytes
readLine
readLines
reverse
reverseEach
rightShift
rightShiftUnsigned
round
size
sort
splitEachLine
step
subMap
times
toDouble
toFloat
toInteger
tokenize
toList
toLong
toURL
transformChar
transformLine
upto
use
waitForOrKill
withInputStream
withOutputStream
withPrintWriter
withReader
withStream
withStreams
withWriter
withWriterAppend
write
writeLine
"""
)


#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_C_DEFAULT', 'default_style'),
                 ('STC_C_COMMENT', 'comment_style'),
                 ('STC_C_COMMENTDOC', 'comment_style'),
                 ('STC_C_COMMENTDOCKEYWORD', 'dockey_style'),
                 ('STC_C_COMMENTDOCKEYWORDERROR', 'error_style'),
                 ('STC_C_COMMENTLINE', 'comment_style'),
                 ('STC_C_COMMENTLINEDOC', 'comment_style'),
                 ('STC_C_CHARACTER', 'char_style'),
                 ('STC_C_GLOBALCLASS', 'global_style'),
                 ('STC_C_IDENTIFIER', 'default_style'),
                 ('STC_C_NUMBER', 'number_style'),
                 ('STC_C_OPERATOR', 'operator_style'),
                 ('STC_C_PREPROCESSOR', 'pre_style'),
                 ('STC_C_REGEX', 'pre_style'),
                 ('STC_C_STRING', 'string_style'),
                 ('STC_C_STRINGEOL', 'stringeol_style'),
                 ('STC_C_UUID', 'pre_style'),
                 ('STC_C_VERBATIM', "number2_style"),
                 ('STC_C_WORD', 'keyword_style'),
                 ('STC_C_WORD2', 'keyword2_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_PRE = ("styling.within.preprocessor", "0")
FOLD_COM = ("fold.comment", "1")
FOLD_COMP = ("fold.compact", "1")
FOLD_ELSE = ("fold.at.else", "0")
#------------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_GROOVY:
        return [MAIN_KEYWORDS, SECONDARY_KEYWORDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_GROOVY:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    return list()
    if lang_id == synglob.ID_LANG_GROOVY:
        return [FOLD, FOLD_PRE]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_GROOVY:
        return [ u'//' ]
    else:
        return list()

#---- End Required Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
