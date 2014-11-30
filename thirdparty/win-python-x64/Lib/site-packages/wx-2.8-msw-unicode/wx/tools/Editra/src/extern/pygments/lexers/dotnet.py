# -*- coding: utf-8 -*-
"""
    pygments.lexers.dotnet
    ~~~~~~~~~~~~~~~~~~~~~~

    Lexers for .net languages.

    :copyright: Copyright 2006-2010 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""
import re

from pygments.lexer import RegexLexer, DelegatingLexer, bygroups, using, this
from pygments.token import Punctuation, \
     Text, Comment, Operator, Keyword, Name, String, Number, Literal, Other
from pygments.util import get_choice_opt
from pygments import unistring as uni

from pygments.lexers.web import XmlLexer

__all__ = ['CSharpLexer', 'BooLexer', 'VbNetLexer', 'CSharpAspxLexer',
           'VbNetAspxLexer']


def _escape(st):
    return st.replace(u'\\', ur'\\').replace(u'-', ur'\-').\
           replace(u'[', ur'\[').replace(u']', ur'\]')

class CSharpLexer(RegexLexer):
    """
    For `C# <http://msdn2.microsoft.com/en-us/vcsharp/default.aspx>`_
    source code.

    Additional options accepted:

    `unicodelevel`
      Determines which Unicode characters this lexer allows for identifiers.
      The possible values are:

      * ``none`` -- only the ASCII letters and numbers are allowed. This
        is the fastest selection.
      * ``basic`` -- all Unicode characters from the specification except
        category ``Lo`` are allowed.
      * ``full`` -- all Unicode characters as specified in the C# specs
        are allowed.  Note that this means a considerable slowdown since the
        ``Lo`` category has more than 40,000 characters in it!

      The default value is ``basic``.

      *New in Pygments 0.8.*
    """

    name = 'C#'
    aliases = ['csharp', 'c#']
    filenames = ['*.cs']
    mimetypes = ['text/x-csharp'] # inferred

    flags = re.MULTILINE | re.DOTALL | re.UNICODE

    # for the range of allowed unicode characters in identifiers,
    # see http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-334.pdf

    levels = {
        'none': '@?[_a-zA-Z][a-zA-Z0-9_]*',
        'basic': ('@?[_' + uni.Lu + uni.Ll + uni.Lt + uni.Lm + uni.Nl + ']' +
                  '[' + uni.Lu + uni.Ll + uni.Lt + uni.Lm + uni.Nl +
                  uni.Nd + uni.Pc + uni.Cf + uni.Mn + uni.Mc + ']*'),
        'full': ('@?(?:_|[^' +
                 _escape(uni.allexcept('Lu', 'Ll', 'Lt', 'Lm', 'Lo', 'Nl')) + '])'
                 + '[^' + _escape(uni.allexcept('Lu', 'Ll', 'Lt', 'Lm', 'Lo',
                                                'Nl', 'Nd', 'Pc', 'Cf', 'Mn',
                                                'Mc')) + ']*'),
    }

    tokens = {}
    token_variants = True

    for levelname, cs_ident in levels.items():
        tokens[levelname] = {
            'root': [
                # method names
                (r'^([ \t]*(?:' + cs_ident + r'(?:\[\])?\s+)+?)' # return type
                 r'(' + cs_ident + ')'                           # method name
                 r'(\s*)(\()',                               # signature start
                 bygroups(using(this), Name.Function, Text, Punctuation)),
                (r'^\s*\[.*?\]', Name.Attribute),
                (r'[^\S\n]+', Text),
                (r'\\\n', Text), # line continuation
                (r'//.*?\n', Comment.Single),
                (r'/[*](.|\n)*?[*]/', Comment.Multiline),
                (r'\n', Text),
                (r'[~!%^&*()+=|\[\]:;,.<>/?-]', Punctuation),
                (r'[{}]', Punctuation),
                (r'@"(\\\\|\\"|[^"])*"', String),
                (r'"(\\\\|\\"|[^"\n])*["\n]', String),
                (r"'\\.'|'[^\\]'", String.Char),
                (r"[0-9](\.[0-9]*)?([eE][+-][0-9]+)?"
                 r"[flFLdD]?|0[xX][0-9a-fA-F]+[Ll]?", Number),
                (r'#[ \t]*(if|endif|else|elif|define|undef|'
                 r'line|error|warning|region|endregion|pragma)\b.*?\n',
                 Comment.Preproc),
                (r'\b(extern)(\s+)(alias)\b', bygroups(Keyword, Text,
                 Keyword)),
                (r'(abstract|as|base|break|case|catch|'
                 r'checked|const|continue|default|delegate|'
                 r'do|else|enum|event|explicit|extern|false|finally|'
                 r'fixed|for|foreach|goto|if|implicit|in|interface|'
                 r'internal|is|lock|new|null|operator|'
                 r'out|override|params|private|protected|public|readonly|'
                 r'ref|return|sealed|sizeof|stackalloc|static|'
                 r'switch|this|throw|true|try|typeof|'
                 r'unchecked|unsafe|virtual|void|while|'
                 r'get|set|new|partial|yield|add|remove|value)\b', Keyword),
                (r'(global)(::)', bygroups(Keyword, Punctuation)),
                (r'(bool|byte|char|decimal|double|float|int|long|object|sbyte|'
                 r'short|string|uint|ulong|ushort)\b\??', Keyword.Type),
                (r'(class|struct)(\s+)', bygroups(Keyword, Text), 'class'),
                (r'(namespace|using)(\s+)', bygroups(Keyword, Text), 'namespace'),
                (cs_ident, Name),
            ],
            'class': [
                (cs_ident, Name.Class, '#pop')
            ],
            'namespace': [
                (r'(?=\()', Text, '#pop'), # using (resource)
                ('(' + cs_ident + r'|\.)+', Name.Namespace, '#pop')
            ]
        }

    def __init__(self, **options):
        level = get_choice_opt(options, 'unicodelevel', self.tokens.keys(), 'basic')
        if level not in self._all_tokens:
            # compile the regexes now
            self._tokens = self.__class__.process_tokendef(level)
        else:
            self._tokens = self._all_tokens[level]

        RegexLexer.__init__(self, **options)


class BooLexer(RegexLexer):
    """
    For `Boo <http://boo.codehaus.org/>`_ source code.
    """

    name = 'Boo'
    aliases = ['boo']
    filenames = ['*.boo']
    mimetypes = ['text/x-boo']

    tokens = {
        'root': [
            (r'\s+', Text),
            (r'(#|//).*$', Comment.Single),
            (r'/[*]', Comment.Multiline, 'comment'),
            (r'[]{}:(),.;[]', Punctuation),
            (r'\\\n', Text),
            (r'\\', Text),
            (r'(in|is|and|or|not)\b', Operator.Word),
            (r'/(\\\\|\\/|[^/\s])/', String.Regex),
            (r'@/(\\\\|\\/|[^/])*/', String.Regex),
            (r'=~|!=|==|<<|>>|[-+/*%=<>&^|]', Operator),
            (r'(as|abstract|callable|constructor|destructor|do|import|'
             r'enum|event|final|get|interface|internal|of|override|'
             r'partial|private|protected|public|return|set|static|'
             r'struct|transient|virtual|yield|super|and|break|cast|'
             r'continue|elif|else|ensure|except|for|given|goto|if|in|'
             r'is|isa|not|or|otherwise|pass|raise|ref|try|unless|when|'
             r'while|from|as)\b', Keyword),
            (r'def(?=\s+\(.*?\))', Keyword),
            (r'(def)(\s+)', bygroups(Keyword, Text), 'funcname'),
            (r'(class)(\s+)', bygroups(Keyword, Text), 'classname'),
            (r'(namespace)(\s+)', bygroups(Keyword, Text), 'namespace'),
            (r'(?<!\.)(true|false|null|self|__eval__|__switch__|array|'
             r'assert|checked|enumerate|filter|getter|len|lock|map|'
             r'matrix|max|min|normalArrayIndexing|print|property|range|'
             r'rawArrayIndexing|required|typeof|unchecked|using|'
             r'yieldAll|zip)\b', Name.Builtin),
            ('"""(\\\\|\\"|.*?)"""', String.Double),
            ('"(\\\\|\\"|[^"]*?)"', String.Double),
            ("'(\\\\|\\'|[^']*?)'", String.Single),
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
            (r'(\d+\.\d*|\d*\.\d+)([fF][+-]?[0-9]+)?', Number.Float),
            (r'[0-9][0-9\.]*(m|ms|d|h|s)', Number),
            (r'0\d+', Number.Oct),
            (r'0x[a-fA-F0-9]+', Number.Hex),
            (r'\d+L', Number.Integer.Long),
            (r'\d+', Number.Integer),
        ],
        'comment': [
            ('/[*]', Comment.Multiline, '#push'),
            ('[*]/', Comment.Multiline, '#pop'),
            ('[^/*]', Comment.Multiline),
            ('[*/]', Comment.Multiline)
        ],
        'funcname': [
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name.Function, '#pop')
        ],
        'classname': [
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'namespace': [
            ('[a-zA-Z_][a-zA-Z0-9_.]*', Name.Namespace, '#pop')
        ]
    }


class VbNetLexer(RegexLexer):
    """
    For
    `Visual Basic.NET <http://msdn2.microsoft.com/en-us/vbasic/default.aspx>`_
    source code.
    """

    name = 'VB.net'
    aliases = ['vb.net', 'vbnet']
    filenames = ['*.vb', '*.bas']
    mimetypes = ['text/x-vbnet', 'text/x-vba'] # (?)

    flags = re.MULTILINE | re.IGNORECASE
    tokens = {
        'root': [
            (r'^\s*<.*?>', Name.Attribute),
            (r'\s+', Text),
            (r'\n', Text),
            (r'rem\b.*?\n', Comment),
            (r"'.*?\n", Comment),
            (r'#If\s.*?\sThen|#ElseIf\s.*?\sThen|#End\s+If|#Const|'
             r'#ExternalSource.*?\n|#End\s+ExternalSource|'
             r'#Region.*?\n|#End\s+Region|#ExternalChecksum',
             Comment.Preproc),
            (r'[\(\){}!#,.:]', Punctuation),
            (r'Option\s+(Strict|Explicit|Compare)\s+'
             r'(On|Off|Binary|Text)', Keyword.Declaration),
            (r'(?<!\.)(AddHandler|Alias|'
             r'ByRef|ByVal|Call|Case|Catch|CBool|CByte|CChar|CDate|'
             r'CDec|CDbl|CInt|CLng|CObj|Continue|CSByte|CShort|'
             r'CSng|CStr|CType|CUInt|CULng|CUShort|Declare|'
             r'Default|Delegate|DirectCast|Do|Each|Else|ElseIf|'
             r'EndIf|Erase|Error|Event|Exit|False|Finally|For|'
             r'Friend|Get|Global|GoSub|GoTo|Handles|If|'
             r'Implements|Inherits|Interface|'
             r'Let|Lib|Loop|Me|MustInherit|'
             r'MustOverride|MyBase|MyClass|Narrowing|New|Next|'
             r'Not|Nothing|NotInheritable|NotOverridable|Of|On|'
             r'Operator|Option|Optional|Overloads|Overridable|'
             r'Overrides|ParamArray|Partial|Private|Protected|'
             r'Public|RaiseEvent|ReadOnly|ReDim|RemoveHandler|Resume|'
             r'Return|Select|Set|Shadows|Shared|Single|'
             r'Static|Step|Stop|SyncLock|Then|'
             r'Throw|To|True|Try|TryCast|Wend|'
             r'Using|When|While|Widening|With|WithEvents|'
             r'WriteOnly)\b', Keyword),
            (r'(?<!\.)End\b', Keyword, 'end'),
            (r'(?<!\.)(Dim|Const)\b', Keyword, 'dim'),
            (r'(?<!\.)(Function|Sub|Property)(\s+)',
             bygroups(Keyword, Text), 'funcname'),
            (r'(?<!\.)(Class|Structure|Enum)(\s+)',
             bygroups(Keyword, Text), 'classname'),
            (r'(?<!\.)(Module|Namespace|Imports)(\s+)',
             bygroups(Keyword, Text), 'namespace'),
            (r'(?<!\.)(Boolean|Byte|Char|Date|Decimal|Double|Integer|Long|'
             r'Object|SByte|Short|Single|String|Variant|UInteger|ULong|'
             r'UShort)\b', Keyword.Type),
            (r'(?<!\.)(AddressOf|And|AndAlso|As|GetType|In|Is|IsNot|Like|Mod|'
             r'Or|OrElse|TypeOf|Xor)\b', Operator.Word),
            (r'&=|[*]=|/=|\\=|\^=|\+=|-=|<<=|>>=|<<|>>|:=|'
             r'<=|>=|<>|[-&*/\\^+=<>]',
             Operator),
            ('"', String, 'string'),
            ('[a-zA-Z_][a-zA-Z0-9_]*[%&@!#$]?', Name),
            ('#.*?#', Literal.Date),
            (r'(\d+\.\d*|\d*\.\d+)([fF][+-]?[0-9]+)?', Number.Float),
            (r'\d+([SILDFR]|US|UI|UL)?', Number.Integer),
            (r'&H[0-9a-f]+([SILDFR]|US|UI|UL)?', Number.Integer),
            (r'&O[0-7]+([SILDFR]|US|UI|UL)?', Number.Integer),
            (r'_\n', Text), # Line continuation
        ],
        'string': [
            (r'""', String),
            (r'"C?', String, '#pop'),
            (r'[^"]+', String),
        ],
        'dim': [
            (r'[a-z_][a-z0-9_]*', Name.Variable, '#pop'),
            (r'', Text, '#pop'),  # any other syntax
        ],
        'funcname': [
            (r'[a-z_][a-z0-9_]*', Name.Function, '#pop'),
        ],
        'classname': [
            (r'[a-z_][a-z0-9_]*', Name.Class, '#pop'),
        ],
        'namespace': [
            (r'[a-z_][a-z0-9_.]*', Name.Namespace, '#pop'),
        ],
        'end': [
            (r'\s+', Text),
            (r'(Function|Sub|Property|Class|Structure|Enum|Module|Namespace)\b',
             Keyword, '#pop'),
            (r'', Text, '#pop'),
        ]
    }

class GenericAspxLexer(RegexLexer):
    """
    Lexer for ASP.NET pages.
    """

    name = 'aspx-gen'
    filenames = []
    mimetypes = []

    flags = re.DOTALL

    tokens = {
        'root': [
            (r'(<%[@=#]?)(.*?)(%>)', bygroups(Name.Tag, Other, Name.Tag)),
            (r'(<script.*?>)(.*?)(</script>)', bygroups(using(XmlLexer),
                                                        Other,
                                                        using(XmlLexer))),
            (r'(.+?)(?=<)', using(XmlLexer)),
            (r'.+', using(XmlLexer)),
        ],
    }

#TODO support multiple languages within the same source file
class CSharpAspxLexer(DelegatingLexer):
    """
    Lexer for highligting C# within ASP.NET pages.
    """

    name = 'aspx-cs'
    aliases = ['aspx-cs']
    filenames = ['*.aspx', '*.asax', '*.ascx', '*.ashx', '*.asmx', '*.axd']
    mimetypes = []

    def __init__(self, **options):
        super(CSharpAspxLexer, self).__init__(CSharpLexer,GenericAspxLexer,
                                              **options)

    def analyse_text(text):
        if re.search(r'Page\s*Language="C#"', text, re.I) is not None:
            return 0.2
        elif re.search(r'script[^>]+language=["\']C#', text, re.I) is not None:
            return 0.15
        return 0.001 # TODO really only for when filename matched...

class VbNetAspxLexer(DelegatingLexer):
    """
    Lexer for highligting Visual Basic.net within ASP.NET pages.
    """

    name = 'aspx-vb'
    aliases = ['aspx-vb']
    filenames = ['*.aspx', '*.asax', '*.ascx', '*.ashx', '*.asmx', '*.axd']
    mimetypes = []

    def __init__(self, **options):
        super(VbNetAspxLexer, self).__init__(VbNetLexer,GenericAspxLexer,
                                              **options)

    def analyse_text(text):
        if re.search(r'Page\s*Language="Vb"', text, re.I) is not None:
            return 0.2
        elif re.search(r'script[^>]+language=["\']vb', text, re.I) is not None:
            return 0.15
