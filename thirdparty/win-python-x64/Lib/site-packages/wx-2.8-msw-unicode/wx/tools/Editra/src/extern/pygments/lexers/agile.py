# -*- coding: utf-8 -*-
"""
    pygments.lexers.agile
    ~~~~~~~~~~~~~~~~~~~~~

    Lexers for agile languages.

    :copyright: Copyright 2006-2010 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re

from pygments.lexer import Lexer, RegexLexer, ExtendedRegexLexer, \
     LexerContext, include, combined, do_insertions, bygroups, using
from pygments.token import Error, Text, Other, \
     Comment, Operator, Keyword, Name, String, Number, Generic, Punctuation
from pygments.util import get_bool_opt, get_list_opt, shebang_matches
from pygments import unistring as uni


__all__ = ['PythonLexer', 'PythonConsoleLexer', 'PythonTracebackLexer',
           'RubyLexer', 'RubyConsoleLexer', 'PerlLexer', 'LuaLexer',
           'MiniDLexer', 'IoLexer', 'TclLexer', 'ClojureLexer',
           'Python3Lexer', 'Python3TracebackLexer', 'FactorLexer', 'IokeLexer']

# b/w compatibility
from pygments.lexers.functional import SchemeLexer

line_re  = re.compile('.*?\n')


class PythonLexer(RegexLexer):
    """
    For `Python <http://www.python.org>`_ source code.
    """

    name = 'Python'
    aliases = ['python', 'py']
    filenames = ['*.py', '*.pyw', '*.sc', 'SConstruct', 'SConscript', '*.tac']
    mimetypes = ['text/x-python', 'application/x-python']

    tokens = {
        'root': [
            (r'\n', Text),
            (r'^(\s*)([rRuU]{,2}"""(?:.|\n)*?""")', bygroups(Text, String.Doc)),
            (r"^(\s*)([rRuU]{,2}'''(?:.|\n)*?''')", bygroups(Text, String.Doc)),
            (r'[^\S\n]+', Text),
            (r'#.*$', Comment),
            (r'[]{}:(),;[]', Punctuation),
            (r'\\\n', Text),
            (r'\\', Text),
            (r'(in|is|and|or|not)\b', Operator.Word),
            (r'!=|==|<<|>>|[-~+/*%=<>&^|.]', Operator),
            include('keywords'),
            (r'(def)((?:\s|\\\s)+)', bygroups(Keyword, Text), 'funcname'),
            (r'(class)((?:\s|\\\s)+)', bygroups(Keyword, Text), 'classname'),
            (r'(from)((?:\s|\\\s)+)', bygroups(Keyword.Namespace, Text), 'fromimport'),
            (r'(import)((?:\s|\\\s)+)', bygroups(Keyword.Namespace, Text), 'import'),
            include('builtins'),
            include('backtick'),
            ('(?:[rR]|[uU][rR]|[rR][uU])"""', String, 'tdqs'),
            ("(?:[rR]|[uU][rR]|[rR][uU])'''", String, 'tsqs'),
            ('(?:[rR]|[uU][rR]|[rR][uU])"', String, 'dqs'),
            ("(?:[rR]|[uU][rR]|[rR][uU])'", String, 'sqs'),
            ('[uU]?"""', String, combined('stringescape', 'tdqs')),
            ("[uU]?'''", String, combined('stringescape', 'tsqs')),
            ('[uU]?"', String, combined('stringescape', 'dqs')),
            ("[uU]?'", String, combined('stringescape', 'sqs')),
            include('name'),
            include('numbers'),
        ],
        'keywords': [
            (r'(assert|break|continue|del|elif|else|except|exec|'
             r'finally|for|global|if|lambda|pass|print|raise|'
             r'return|try|while|yield|as|with)\b', Keyword),
        ],
        'builtins': [
            (r'(?<!\.)(__import__|abs|all|any|apply|basestring|bin|bool|buffer|'
             r'bytearray|bytes|callable|chr|classmethod|cmp|coerce|compile|'
             r'complex|delattr|dict|dir|divmod|enumerate|eval|execfile|exit|'
             r'file|filter|float|frozenset|getattr|globals|hasattr|hash|hex|id|'
             r'input|int|intern|isinstance|issubclass|iter|len|list|locals|'
             r'long|map|max|min|next|object|oct|open|ord|pow|property|range|'
             r'raw_input|reduce|reload|repr|reversed|round|set|setattr|slice|'
             r'sorted|staticmethod|str|sum|super|tuple|type|unichr|unicode|'
             r'vars|xrange|zip)\b', Name.Builtin),
            (r'(?<!\.)(self|None|Ellipsis|NotImplemented|False|True'
             r')\b', Name.Builtin.Pseudo),
            (r'(?<!\.)(ArithmeticError|AssertionError|AttributeError|'
             r'BaseException|DeprecationWarning|EOFError|EnvironmentError|'
             r'Exception|FloatingPointError|FutureWarning|GeneratorExit|IOError|'
             r'ImportError|ImportWarning|IndentationError|IndexError|KeyError|'
             r'KeyboardInterrupt|LookupError|MemoryError|NameError|'
             r'NotImplemented|NotImplementedError|OSError|OverflowError|'
             r'OverflowWarning|PendingDeprecationWarning|ReferenceError|'
             r'RuntimeError|RuntimeWarning|StandardError|StopIteration|'
             r'SyntaxError|SyntaxWarning|SystemError|SystemExit|TabError|'
             r'TypeError|UnboundLocalError|UnicodeDecodeError|'
             r'UnicodeEncodeError|UnicodeError|UnicodeTranslateError|'
             r'UnicodeWarning|UserWarning|ValueError|VMSError|Warning|'
             r'WindowsError|ZeroDivisionError)\b', Name.Exception),
        ],
        'numbers': [
            (r'(\d+\.\d*|\d*\.\d+)([eE][+-]?[0-9]+)?', Number.Float),
            (r'\d+[eE][+-]?[0-9]+', Number.Float),
            (r'0[0-7]+', Number.Oct),
            (r'0[xX][a-fA-F0-9]+', Number.Hex),
            (r'\d+L', Number.Integer.Long),
            (r'\d+', Number.Integer)
        ],
        'backtick': [
            ('`.*?`', String.Backtick),
        ],
        'name': [
            (r'@[a-zA-Z0-9_.]+', Name.Decorator),
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
        ],
        'funcname': [
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name.Function, '#pop')
        ],
        'classname': [
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name.Class, '#pop')
        ],
        'import': [
            (r'((?:\s|\\\s)+)(as)((?:\s|\\\s)+)',
             bygroups(Text, Keyword.Namespace, Text)),
            (r'[a-zA-Z_][a-zA-Z0-9_.]*', Name.Namespace),
            (r'(\s*)(,)(\s*)', bygroups(Text, Operator, Text)),
            (r'', Text, '#pop') # all else: go back
        ],
        'fromimport': [
            (r'((?:\s|\\\s)+)(import)\b', bygroups(Text, Keyword.Namespace), '#pop'),
            (r'[a-zA-Z_.][a-zA-Z0-9_.]*', Name.Namespace),
        ],
        'stringescape': [
            (r'\\([\\abfnrtv"\']|\n|N{.*?}|u[a-fA-F0-9]{4}|'
             r'U[a-fA-F0-9]{8}|x[a-fA-F0-9]{2}|[0-7]{1,3})', String.Escape)
        ],
        'strings': [
            (r'%(\([a-zA-Z0-9_]+\))?[-#0 +]*([0-9]+|[*])?(\.([0-9]+|[*]))?'
             '[hlL]?[diouxXeEfFgGcrs%]', String.Interpol),
            (r'[^\\\'"%\n]+', String),
            # quotes, percents and backslashes must be parsed one at a time
            (r'[\'"\\]', String),
            # unhandled string formatting sign
            (r'%', String)
            # newlines are an error (use "nl" state)
        ],
        'nl': [
            (r'\n', String)
        ],
        'dqs': [
            (r'"', String, '#pop'),
            (r'\\\\|\\"|\\\n', String.Escape), # included here again for raw strings
            include('strings')
        ],
        'sqs': [
            (r"'", String, '#pop'),
            (r"\\\\|\\'|\\\n", String.Escape), # included here again for raw strings
            include('strings')
        ],
        'tdqs': [
            (r'"""', String, '#pop'),
            include('strings'),
            include('nl')
        ],
        'tsqs': [
            (r"'''", String, '#pop'),
            include('strings'),
            include('nl')
        ],
    }

    def analyse_text(text):
        return shebang_matches(text, r'pythonw?(2\.\d)?')


class Python3Lexer(RegexLexer):
    """
    For `Python <http://www.python.org>`_ source code (version 3.0).

    *New in Pygments 0.10.*
    """

    name = 'Python 3'
    aliases = ['python3', 'py3']
    filenames = []  # Nothing until Python 3 gets widespread
    mimetypes = ['text/x-python3', 'application/x-python3']

    flags = re.MULTILINE | re.UNICODE

    uni_name = "[%s][%s]*" % (uni.xid_start, uni.xid_continue)

    tokens = PythonLexer.tokens.copy()
    tokens['keywords'] = [
        (r'(assert|break|continue|del|elif|else|except|'
         r'finally|for|global|if|lambda|pass|raise|'
         r'return|try|while|yield|as|with|True|False|None)\b', Keyword),
    ]
    tokens['builtins'] = [
        (r'(?<!\.)(__import__|abs|all|any|bin|bool|bytearray|bytes|'
         r'chr|classmethod|cmp|compile|complex|delattr|dict|dir|'
         r'divmod|enumerate|eval|filter|float|format|frozenset|getattr|'
         r'globals|hasattr|hash|hex|id|input|int|isinstance|issubclass|'
         r'iter|len|list|locals|map|max|memoryview|min|next|object|oct|'
         r'open|ord|pow|print|property|range|repr|reversed|round|'
         r'set|setattr|slice|sorted|staticmethod|str|sum|super|tuple|type|'
         r'vars|zip)\b', Name.Builtin),
        (r'(?<!\.)(self|Ellipsis|NotImplemented)\b', Name.Builtin.Pseudo),
        (r'(?<!\.)(ArithmeticError|AssertionError|AttributeError|'
         r'BaseException|BufferError|BytesWarning|DeprecationWarning|'
         r'EOFError|EnvironmentError|Exception|FloatingPointError|'
         r'FutureWarning|GeneratorExit|IOError|ImportError|'
         r'ImportWarning|IndentationError|IndexError|KeyError|'
         r'KeyboardInterrupt|LookupError|MemoryError|NameError|'
         r'NotImplementedError|OSError|OverflowError|'
         r'PendingDeprecationWarning|ReferenceError|'
         r'RuntimeError|RuntimeWarning|StopIteration|'
         r'SyntaxError|SyntaxWarning|SystemError|SystemExit|TabError|'
         r'TypeError|UnboundLocalError|UnicodeDecodeError|'
         r'UnicodeEncodeError|UnicodeError|UnicodeTranslateError|'
         r'UnicodeWarning|UserWarning|ValueError|VMSError|Warning|'
         r'WindowsError|ZeroDivisionError)\b', Name.Exception),
    ]
    tokens['numbers'] = [
        (r'(\d+\.\d*|\d*\.\d+)([eE][+-]?[0-9]+)?', Number.Float),
        (r'0[oO][0-7]+', Number.Oct),
        (r'0[bB][01]+', Number.Bin),
        (r'0[xX][a-fA-F0-9]+', Number.Hex),
        (r'\d+', Number.Integer)
    ]
    tokens['backtick'] = []
    tokens['name'] = [
        (r'@[a-zA-Z0-9_]+', Name.Decorator),
        (uni_name, Name),
    ]
    tokens['funcname'] = [
        (uni_name, Name.Function, '#pop')
    ]
    tokens['classname'] = [
        (uni_name, Name.Class, '#pop')
    ]
    tokens['import'] = [
        (r'(\s+)(as)(\s+)', bygroups(Text, Keyword, Text)),
        (r'\.', Name.Namespace),
        (uni_name, Name.Namespace),
        (r'(\s*)(,)(\s*)', bygroups(Text, Operator, Text)),
        (r'', Text, '#pop') # all else: go back
    ]
    tokens['fromimport'] = [
        (r'(\s+)(import)\b', bygroups(Text, Keyword), '#pop'),
        (r'\.', Name.Namespace),
        (uni_name, Name.Namespace),
    ]
    # don't highlight "%s" substitutions
    tokens['strings'] = [
        (r'[^\\\'"%\n]+', String),
        # quotes, percents and backslashes must be parsed one at a time
        (r'[\'"\\]', String),
        # unhandled string formatting sign
        (r'%', String)
        # newlines are an error (use "nl" state)
    ]

    def analyse_text(text):
        return shebang_matches(text, r'pythonw?3(\.\d)?')


class PythonConsoleLexer(Lexer):
    """
    For Python console output or doctests, such as:

    .. sourcecode:: pycon

        >>> a = 'foo'
        >>> print a
        foo
        >>> 1 / 0
        Traceback (most recent call last):
          File "<stdin>", line 1, in <module>
        ZeroDivisionError: integer division or modulo by zero

    Additional options:

    `python3`
        Use Python 3 lexer for code.  Default is ``False``.
        *New in Pygments 1.0.*
    """
    name = 'Python console session'
    aliases = ['pycon']
    mimetypes = ['text/x-python-doctest']

    def __init__(self, **options):
        self.python3 = get_bool_opt(options, 'python3', False)
        Lexer.__init__(self, **options)

    def get_tokens_unprocessed(self, text):
        if self.python3:
            pylexer = Python3Lexer(**self.options)
            tblexer = Python3TracebackLexer(**self.options)
        else:
            pylexer = PythonLexer(**self.options)
            tblexer = PythonTracebackLexer(**self.options)

        curcode = ''
        insertions = []
        curtb = ''
        tbindex = 0
        tb = 0
        for match in line_re.finditer(text):
            line = match.group()
            if line.startswith(u'>>> ') or line.startswith(u'... '):
                tb = 0
                insertions.append((len(curcode),
                                   [(0, Generic.Prompt, line[:4])]))
                curcode += line[4:]
            elif line.rstrip() == u'...' and not tb:
                # only a new >>> prompt can end an exception block
                # otherwise an ellipsis in place of the traceback frames
                # will be mishandled
                insertions.append((len(curcode),
                                   [(0, Generic.Prompt, u'...')]))
                curcode += line[3:]
            else:
                if curcode:
                    for item in do_insertions(insertions,
                                    pylexer.get_tokens_unprocessed(curcode)):
                        yield item
                    curcode = ''
                    insertions = []
                if (line.startswith(u'Traceback (most recent call last):') or
                    re.match(ur'  File "[^"]+", line \d+\n$', line)):
                    tb = 1
                    curtb = line
                    tbindex = match.start()
                elif line == 'KeyboardInterrupt\n':
                    yield match.start(), Name.Class, line
                elif tb:
                    curtb += line
                    if not (line.startswith(' ') or line.strip() == u'...'):
                        tb = 0
                        for i, t, v in tblexer.get_tokens_unprocessed(curtb):
                            yield tbindex+i, t, v
                else:
                    yield match.start(), Generic.Output, line
        if curcode:
            for item in do_insertions(insertions,
                                      pylexer.get_tokens_unprocessed(curcode)):
                yield item


class PythonTracebackLexer(RegexLexer):
    """
    For Python tracebacks.

    *New in Pygments 0.7.*
    """

    name = 'Python Traceback'
    aliases = ['pytb']
    filenames = ['*.pytb']
    mimetypes = ['text/x-python-traceback']

    tokens = {
        'root': [
            (r'^Traceback \(most recent call last\):\n', Generic.Traceback, 'intb'),
            # SyntaxError starts with this.
            (r'^(?=  File "[^"]+", line \d+)', Generic.Traceback, 'intb'),
            (r'^.*\n', Other),
        ],
        'intb': [
            (r'^(  File )("[^"]+")(, line )(\d+)(, in )(.+)(\n)',
             bygroups(Text, Name.Builtin, Text, Number, Text, Name, Text)),
            (r'^(  File )("[^"]+")(, line )(\d+)(\n)',
             bygroups(Text, Name.Builtin, Text, Number, Text)),
            (r'^(    )(.+)(\n)',
             bygroups(Text, using(PythonLexer), Text)),
            (r'^([ \t]*)(...)(\n)',
             bygroups(Text, Comment, Text)), # for doctests...
            (r'^(.+)(: )(.+)(\n)',
             bygroups(Generic.Error, Text, Name, Text), '#pop'),
            (r'^([a-zA-Z_][a-zA-Z0-9_]*)(:?\n)',
             bygroups(Generic.Error, Text), '#pop')
        ],
    }


class Python3TracebackLexer(RegexLexer):
    """
    For Python 3.0 tracebacks, with support for chained exceptions.

    *New in Pygments 1.0.*
    """

    name = 'Python 3.0 Traceback'
    aliases = ['py3tb']
    filenames = ['*.py3tb']
    mimetypes = ['text/x-python3-traceback']

    tokens = {
        'root': [
            (r'\n', Text),
            (r'^Traceback \(most recent call last\):\n', Generic.Traceback, 'intb'),
            (r'^During handling of the above exception, another '
             r'exception occurred:\n\n', Generic.Traceback),
            (r'^The above exception was the direct cause of the '
             r'following exception:\n\n', Generic.Traceback),
        ],
        'intb': [
            (r'^(  File )("[^"]+")(, line )(\d+)(, in )(.+)(\n)',
             bygroups(Text, Name.Builtin, Text, Number, Text, Name, Text)),
            (r'^(    )(.+)(\n)',
             bygroups(Text, using(Python3Lexer), Text)),
            (r'^([ \t]*)(...)(\n)',
             bygroups(Text, Comment, Text)), # for doctests...
            (r'^(.+)(: )(.+)(\n)',
             bygroups(Generic.Error, Text, Name, Text), '#pop'),
            (r'^([a-zA-Z_][a-zA-Z0-9_]*)(:?\n)',
             bygroups(Generic.Error, Text), '#pop')
        ],
    }


class RubyLexer(ExtendedRegexLexer):
    """
    For `Ruby <http://www.ruby-lang.org>`_ source code.
    """

    name = 'Ruby'
    aliases = ['rb', 'ruby', 'duby']
    filenames = ['*.rb', '*.rbw', 'Rakefile', '*.rake', '*.gemspec',
                 '*.rbx', '*.duby']
    mimetypes = ['text/x-ruby', 'application/x-ruby']

    flags = re.DOTALL | re.MULTILINE

    def heredoc_callback(self, match, ctx):
        # okay, this is the hardest part of parsing Ruby...
        # match: 1 = <<-?, 2 = quote? 3 = name 4 = quote? 5 = rest of line

        start = match.start(1)
        yield start, Operator, match.group(1)        # <<-?
        yield match.start(2), String.Heredoc, match.group(2)  # quote ", ', `
        yield match.start(3), Name.Constant, match.group(3)   # heredoc name
        yield match.start(4), String.Heredoc, match.group(4)  # quote again

        heredocstack = ctx.__dict__.setdefault('heredocstack', [])
        outermost = not bool(heredocstack)
        heredocstack.append((match.group(1) == '<<-', match.group(3)))

        ctx.pos = match.start(5)
        ctx.end = match.end(5)
        # this may find other heredocs
        for i, t, v in self.get_tokens_unprocessed(context=ctx):
            yield i, t, v
        ctx.pos = match.end()

        if outermost:
            # this is the outer heredoc again, now we can process them all
            for tolerant, hdname in heredocstack:
                lines = []
                for match in line_re.finditer(ctx.text, ctx.pos):
                    if tolerant:
                        check = match.group().strip()
                    else:
                        check = match.group().rstrip()
                    if check == hdname:
                        for amatch in lines:
                            yield amatch.start(), String.Heredoc, amatch.group()
                        yield match.start(), Name.Constant, match.group()
                        ctx.pos = match.end()
                        break
                    else:
                        lines.append(match)
                else:
                    # end of heredoc not found -- error!
                    for amatch in lines:
                        yield amatch.start(), Error, amatch.group()
            ctx.end = len(ctx.text)
            del heredocstack[:]


    def gen_rubystrings_rules():
        def intp_regex_callback(self, match, ctx):
            yield match.start(1), String.Regex, match.group(1)    # begin
            nctx = LexerContext(match.group(3), 0, ['interpolated-regex'])
            for i, t, v in self.get_tokens_unprocessed(context=nctx):
                yield match.start(3)+i, t, v
            yield match.start(4), String.Regex, match.group(4)    # end[mixounse]*
            ctx.pos = match.end()

        def intp_string_callback(self, match, ctx):
            yield match.start(1), String.Other, match.group(1)
            nctx = LexerContext(match.group(3), 0, ['interpolated-string'])
            for i, t, v in self.get_tokens_unprocessed(context=nctx):
                yield match.start(3)+i, t, v
            yield match.start(4), String.Other, match.group(4)    # end
            ctx.pos = match.end()

        states = {}
        states['strings'] = [
            # easy ones
            (r'\:([a-zA-Z_][\w_]*[\!\?]?|\*\*?|[-+]@?|'
             r'[/%&|^`~]|\[\]=?|<<|>>|<=?>|>=?|===?)', String.Symbol),
            (r":'(\\\\|\\'|[^'])*'", String.Symbol),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
            (r':"', String.Symbol, 'simple-sym'),
            (r'"', String.Double, 'simple-string'),
            (r'(?<!\.)`', String.Backtick, 'simple-backtick'),
        ]

        # double-quoted string and symbol
        for name, ttype, end in ('string', String.Double, '"'), \
                                ('sym', String.Symbol, '"'), \
                                ('backtick', String.Backtick, '`'):
            states['simple-'+name] = [
                include('string-intp-escaped'),
                (r'[^\\%s#]+' % end, ttype),
                (r'[\\#]', ttype),
                (end, ttype, '#pop'),
            ]

        # braced quoted strings
        for lbrace, rbrace, name in ('\\{', '\\}', 'cb'), \
                                    ('\\[', '\\]', 'sb'), \
                                    ('\\(', '\\)', 'pa'), \
                                    ('<', '>', 'ab'):
            states[name+'-intp-string'] = [
                (r'\\[\\' + lbrace + rbrace + ']', String.Other),
                (r'(?<!\\)' + lbrace, String.Other, '#push'),
                (r'(?<!\\)' + rbrace, String.Other, '#pop'),
                include('string-intp-escaped'),
                (r'[\\#' + lbrace + rbrace + ']', String.Other),
                (r'[^\\#' + lbrace + rbrace + ']+', String.Other),
            ]
            states['strings'].append((r'%[QWx]?' + lbrace, String.Other,
                                      name+'-intp-string'))
            states[name+'-string'] = [
                (r'\\[\\' + lbrace + rbrace + ']', String.Other),
                (r'(?<!\\)' + lbrace, String.Other, '#push'),
                (r'(?<!\\)' + rbrace, String.Other, '#pop'),
                (r'[\\#' + lbrace + rbrace + ']', String.Other),
                (r'[^\\#' + lbrace + rbrace + ']+', String.Other),
            ]
            states['strings'].append((r'%[qsw]' + lbrace, String.Other,
                                      name+'-string'))
            states[name+'-regex'] = [
                (r'\\[\\' + lbrace + rbrace + ']', String.Regex),
                (r'(?<!\\)' + lbrace, String.Regex, '#push'),
                (r'(?<!\\)' + rbrace + '[mixounse]*', String.Regex, '#pop'),
                include('string-intp'),
                (r'[\\#' + lbrace + rbrace + ']', String.Regex),
                (r'[^\\#' + lbrace + rbrace + ']+', String.Regex),
            ]
            states['strings'].append((r'%r' + lbrace, String.Regex,
                                      name+'-regex'))

        # these must come after %<brace>!
        states['strings'] += [
            # %r regex
            (r'(%r([^a-zA-Z0-9]))((?:\\\2|(?!\2).)*)(\2[mixounse]*)',
             intp_regex_callback),
            # regular fancy strings with qsw
            (r'%[qsw]([^a-zA-Z0-9])((?:\\\1|(?!\1).)*)\1', String.Other),
            (r'(%[QWx]([^a-zA-Z0-9]))((?:\\\2|(?!\2).)*)(\2)',
             intp_string_callback),
            # special forms of fancy strings after operators or
            # in method calls with braces
            (r'(?<=[-+/*%=<>&!^|~,(])(\s*)(%([\t ])(?:(?:\\\3|(?!\3).)*)\3)',
             bygroups(Text, String.Other, None)),
            # and because of fixed width lookbehinds the whole thing a
            # second time for line startings...
            (r'^(\s*)(%([\t ])(?:(?:\\\3|(?!\3).)*)\3)',
             bygroups(Text, String.Other, None)),
            # all regular fancy strings without qsw
            (r'(%([^a-zA-Z0-9\s]))((?:\\\2|(?!\2).)*)(\2)',
             intp_string_callback),
        ]

        return states

    tokens = {
        'root': [
            (r'#.*?$', Comment.Single),
            (r'=begin\s.*?\n=end', Comment.Multiline),
            # keywords
            (r'(BEGIN|END|alias|begin|break|case|defined\?|'
             r'do|else|elsif|end|ensure|for|if|in|next|redo|'
             r'rescue|raise|retry|return|super|then|undef|unless|until|when|'
             r'while|yield)\b', Keyword),
            # start of function, class and module names
            (r'(module)(\s+)([a-zA-Z_][a-zA-Z0-9_]*(::[a-zA-Z_][a-zA-Z0-9_]*)*)',
             bygroups(Keyword, Text, Name.Namespace)),
            (r'(def)(\s+)', bygroups(Keyword, Text), 'funcname'),
            (r'def(?=[*%&^`~+-/\[<>=])', Keyword, 'funcname'),
            (r'(class)(\s+)', bygroups(Keyword, Text), 'classname'),
            # special methods
            (r'(initialize|new|loop|include|extend|raise|attr_reader|'
             r'attr_writer|attr_accessor|attr|catch|throw|private|'
             r'module_function|public|protected|true|false|nil)\b', Keyword.Pseudo),
            (r'(not|and|or)\b', Operator.Word),
            (r'(autoload|block_given|const_defined|eql|equal|frozen|include|'
             r'instance_of|is_a|iterator|kind_of|method_defined|nil|'
             r'private_method_defined|protected_method_defined|'
             r'public_method_defined|respond_to|tainted)\?', Name.Builtin),
            (r'(chomp|chop|exit|gsub|sub)!', Name.Builtin),
            (r'(?<!\.)(Array|Float|Integer|String|__id__|__send__|abort|ancestors|'
             r'at_exit|autoload|binding|callcc|caller|'
             r'catch|chomp|chop|class_eval|class_variables|'
             r'clone|const_defined\?|const_get|const_missing|const_set|constants|'
             r'display|dup|eval|exec|exit|extend|fail|fork|'
             r'format|freeze|getc|gets|global_variables|gsub|'
             r'hash|id|included_modules|inspect|instance_eval|'
             r'instance_method|instance_methods|'
             r'instance_variable_get|instance_variable_set|instance_variables|'
             r'lambda|load|local_variables|loop|'
             r'method|method_missing|methods|module_eval|name|'
             r'object_id|open|p|print|printf|private_class_method|'
             r'private_instance_methods|'
             r'private_methods|proc|protected_instance_methods|'
             r'protected_methods|public_class_method|'
             r'public_instance_methods|public_methods|'
             r'putc|puts|raise|rand|readline|readlines|require|'
             r'scan|select|self|send|set_trace_func|singleton_methods|sleep|'
             r'split|sprintf|srand|sub|syscall|system|taint|'
             r'test|throw|to_a|to_s|trace_var|trap|untaint|untrace_var|'
             r'warn)\b', Name.Builtin),
            (r'__(FILE|LINE)__\b', Name.Builtin.Pseudo),
            # normal heredocs
            (r'(?<!\w)(<<-?)(["`\']?)([a-zA-Z_]\w*)(\2)(.*?\n)', heredoc_callback),
            # empty string heredocs
            (r'(<<-?)("|\')()(\2)(.*?\n)', heredoc_callback),
            (r'__END__', Comment.Preproc, 'end-part'),
            # multiline regex (after keywords or assignments)
            (r'(?:^|(?<=[=<>~!])|'
                 r'(?<=(?:\s|;)when\s)|'
                 r'(?<=(?:\s|;)or\s)|'
                 r'(?<=(?:\s|;)and\s)|'
                 r'(?<=(?:\s|;|\.)index\s)|'
                 r'(?<=(?:\s|;|\.)scan\s)|'
                 r'(?<=(?:\s|;|\.)sub\s)|'
                 r'(?<=(?:\s|;|\.)sub!\s)|'
                 r'(?<=(?:\s|;|\.)gsub\s)|'
                 r'(?<=(?:\s|;|\.)gsub!\s)|'
                 r'(?<=(?:\s|;|\.)match\s)|'
                 r'(?<=(?:\s|;)if\s)|'
                 r'(?<=(?:\s|;)elsif\s)|'
                 r'(?<=^when\s)|'
                 r'(?<=^index\s)|'
                 r'(?<=^scan\s)|'
                 r'(?<=^sub\s)|'
                 r'(?<=^gsub\s)|'
                 r'(?<=^sub!\s)|'
                 r'(?<=^gsub!\s)|'
                 r'(?<=^match\s)|'
                 r'(?<=^if\s)|'
                 r'(?<=^elsif\s)'
             r')(\s*)(/)', bygroups(Text, String.Regex), 'multiline-regex'),
            # multiline regex (in method calls)
            (r'(?<=\(|,)/', String.Regex, 'multiline-regex'),
            # multiline regex (this time the funny no whitespace rule)
            (r'(\s+)(/[^\s=])', String.Regex, 'multiline-regex'),
            # lex numbers and ignore following regular expressions which
            # are division operators in fact (grrrr. i hate that. any
            # better ideas?)
            # since pygments 0.7 we also eat a "?" operator after numbers
            # so that the char operator does not work. Chars are not allowed
            # there so that you can use the ternary operator.
            # stupid example:
            #   x>=0?n[x]:""
            (r'(0_?[0-7]+(?:_[0-7]+)*)(\s*)([/?])?',
             bygroups(Number.Oct, Text, Operator)),
            (r'(0x[0-9A-Fa-f]+(?:_[0-9A-Fa-f]+)*)(\s*)([/?])?',
             bygroups(Number.Hex, Text, Operator)),
            (r'(0b[01]+(?:_[01]+)*)(\s*)([/?])?',
             bygroups(Number.Bin, Text, Operator)),
            (r'([\d]+(?:_\d+)*)(\s*)([/?])?',
             bygroups(Number.Integer, Text, Operator)),
            # Names
            (r'@@[a-zA-Z_][a-zA-Z0-9_]*', Name.Variable.Class),
            (r'@[a-zA-Z_][a-zA-Z0-9_]*', Name.Variable.Instance),
            (r'\$[a-zA-Z0-9_]+', Name.Variable.Global),
            (r'\$[!@&`\'+~=/\\,;.<>_*$?:"]', Name.Variable.Global),
            (r'\$-[0adFiIlpvw]', Name.Variable.Global),
            (r'::', Operator),
            include('strings'),
            # chars
            (r'\?(\\[MC]-)*' # modifiers
             r'(\\([\\abefnrstv#"\']|x[a-fA-F0-9]{1,2}|[0-7]{1,3})|\S)'
             r'(?!\w)',
             String.Char),
            (r'[A-Z][a-zA-Z0-9_]+', Name.Constant),
            # this is needed because ruby attributes can look
            # like keywords (class) or like this: ` ?!?
            (r'(\.|::)([a-zA-Z_]\w*[\!\?]?|[*%&^`~+-/\[<>=])',
             bygroups(Operator, Name)),
            (r'[a-zA-Z_][\w_]*[\!\?]?', Name),
            (r'(\[|\]|\*\*|<<?|>>?|>=|<=|<=>|=~|={3}|'
             r'!~|&&?|\|\||\.{1,3})', Operator),
            (r'[-+/*%=<>&!^|~]=?', Operator),
            (r'[(){};,/?:\\]', Punctuation),
            (r'\s+', Text)
        ],
        'funcname': [
            (r'\(', Punctuation, 'defexpr'),
            (r'(?:([a-zA-Z_][a-zA-Z0-9_]*)(\.))?'
             r'([a-zA-Z_][\w_]*[\!\?]?|\*\*?|[-+]@?|'
             r'[/%&|^`~]|\[\]=?|<<|>>|<=?>|>=?|===?)',
             bygroups(Name.Class, Operator, Name.Function), '#pop'),
            (r'', Text, '#pop')
        ],
        'classname': [
            (r'\(', Punctuation, 'defexpr'),
            (r'<<', Operator, '#pop'),
            (r'[A-Z_][\w_]*', Name.Class, '#pop'),
            (r'', Text, '#pop')
        ],
        'defexpr': [
            (r'(\))(\.|::)?', bygroups(Punctuation, Operator), '#pop'),
            (r'\(', Operator, '#push'),
            include('root')
        ],
        'in-intp': [
            ('}', String.Interpol, '#pop'),
            include('root'),
        ],
        'string-intp': [
            (r'#{', String.Interpol, 'in-intp'),
            (r'#@@?[a-zA-Z_][a-zA-Z0-9_]*', String.Interpol),
            (r'#\$[a-zA-Z_][a-zA-Z0-9_]*', String.Interpol)
        ],
        'string-intp-escaped': [
            include('string-intp'),
            (r'\\([\\abefnrstv#"\']|x[a-fA-F0-9]{1,2}|[0-7]{1,3})', String.Escape)
        ],
        'interpolated-regex': [
            include('string-intp'),
            (r'[\\#]', String.Regex),
            (r'[^\\#]+', String.Regex),
        ],
        'interpolated-string': [
            include('string-intp'),
            (r'[\\#]', String.Other),
            (r'[^\\#]+', String.Other),
        ],
        'multiline-regex': [
            include('string-intp'),
            (r'\\\\', String.Regex),
            (r'\\/', String.Regex),
            (r'[\\#]', String.Regex),
            (r'[^\\/#]+', String.Regex),
            (r'/[mixounse]*', String.Regex, '#pop'),
        ],
        'end-part': [
            (r'.+', Comment.Preproc, '#pop')
        ]
    }
    tokens.update(gen_rubystrings_rules())

    def analyse_text(text):
        return shebang_matches(text, r'ruby(1\.\d)?')


class RubyConsoleLexer(Lexer):
    """
    For Ruby interactive console (**irb**) output like:

    .. sourcecode:: rbcon

        irb(main):001:0> a = 1
        => 1
        irb(main):002:0> puts a
        1
        => nil
    """
    name = 'Ruby irb session'
    aliases = ['rbcon', 'irb']
    mimetypes = ['text/x-ruby-shellsession']

    _prompt_re = re.compile('irb\([a-zA-Z_][a-zA-Z0-9_]*\):\d{3}:\d+[>*"\'] '
                            '|>> |\?> ')

    def get_tokens_unprocessed(self, text):
        rblexer = RubyLexer(**self.options)

        curcode = ''
        insertions = []
        for match in line_re.finditer(text):
            line = match.group()
            m = self._prompt_re.match(line)
            if m is not None:
                end = m.end()
                insertions.append((len(curcode),
                                   [(0, Generic.Prompt, line[:end])]))
                curcode += line[end:]
            else:
                if curcode:
                    for item in do_insertions(insertions,
                                    rblexer.get_tokens_unprocessed(curcode)):
                        yield item
                    curcode = ''
                    insertions = []
                yield match.start(), Generic.Output, line
        if curcode:
            for item in do_insertions(insertions,
                                      rblexer.get_tokens_unprocessed(curcode)):
                yield item


class PerlLexer(RegexLexer):
    """
    For `Perl <http://www.perl.org>`_ source code.
    """

    name = 'Perl'
    aliases = ['perl', 'pl']
    filenames = ['*.pl', '*.pm']
    mimetypes = ['text/x-perl', 'application/x-perl']

    flags = re.DOTALL | re.MULTILINE
    # TODO: give this a perl guy who knows how to parse perl...
    tokens = {
        'balanced-regex': [
            (r'/(\\\\|\\/|[^/])*/[egimosx]*', String.Regex, '#pop'),
            (r'!(\\\\|\\!|[^!])*![egimosx]*', String.Regex, '#pop'),
            (r'\\(\\\\|[^\\])*\\[egimosx]*', String.Regex, '#pop'),
            (r'{(\\\\|\\}|[^}])*}[egimosx]*', String.Regex, '#pop'),
            (r'<(\\\\|\\>|[^>])*>[egimosx]*', String.Regex, '#pop'),
            (r'\[(\\\\|\\\]|[^\]])*\][egimosx]*', String.Regex, '#pop'),
            (r'\((\\\\|\\\)|[^\)])*\)[egimosx]*', String.Regex, '#pop'),
            (r'@(\\\\|\\\@|[^\@])*@[egimosx]*', String.Regex, '#pop'),
            (r'%(\\\\|\\\%|[^\%])*%[egimosx]*', String.Regex, '#pop'),
            (r'\$(\\\\|\\\$|[^\$])*\$[egimosx]*', String.Regex, '#pop'),
        ],
        'root': [
            (r'\#.*?$', Comment.Single),
            (r'^=[a-zA-Z0-9]+\s+.*?\n=cut', Comment.Multiline),
            (r'(case|continue|do|else|elsif|for|foreach|if|last|my|'
             r'next|our|redo|reset|then|unless|until|while|use|'
             r'print|new|BEGIN|CHECK|INIT|END|return)\b', Keyword),
            (r'(format)(\s+)([a-zA-Z0-9_]+)(\s*)(=)(\s*\n)',
             bygroups(Keyword, Text, Name, Text, Punctuation, Text), 'format'),
            (r'(eq|lt|gt|le|ge|ne|not|and|or|cmp)\b', Operator.Word),
            # common delimiters
            (r's/(\\\\|\\/|[^/])*/(\\\\|\\/|[^/])*/[egimosx]*', String.Regex),
            (r's!(\\\\|\\!|[^!])*!(\\\\|\\!|[^!])*![egimosx]*', String.Regex),
            (r's\\(\\\\|[^\\])*\\(\\\\|[^\\])*\\[egimosx]*', String.Regex),
            (r's@(\\\\|\\@|[^@])*@(\\\\|\\@|[^@])*@[egimosx]*', String.Regex),
            (r's%(\\\\|\\%|[^%])*%(\\\\|\\%|[^%])*%[egimosx]*', String.Regex),
            # balanced delimiters
            (r's{(\\\\|\\}|[^}])*}\s*', String.Regex, 'balanced-regex'),
            (r's<(\\\\|\\>|[^>])*>\s*', String.Regex, 'balanced-regex'),
            (r's\[(\\\\|\\\]|[^\]])*\]\s*', String.Regex, 'balanced-regex'),
            (r's\((\\\\|\\\)|[^\)])*\)\s*', String.Regex, 'balanced-regex'),

            (r'm?/(\\\\|\\/|[^/\n])*/[gcimosx]*', String.Regex),
            (r'm(?=[/!\\{<\[\(@%\$])', String.Regex, 'balanced-regex'),
            (r'((?<==~)|(?<=\())\s*/(\\\\|\\/|[^/])*/[gcimosx]*', String.Regex),
            (r'\s+', Text),
            (r'(abs|accept|alarm|atan2|bind|binmode|bless|caller|chdir|'
             r'chmod|chomp|chop|chown|chr|chroot|close|closedir|connect|'
             r'continue|cos|crypt|dbmclose|dbmopen|defined|delete|die|'
             r'dump|each|endgrent|endhostent|endnetent|endprotoent|'
             r'endpwent|endservent|eof|eval|exec|exists|exit|exp|fcntl|'
             r'fileno|flock|fork|format|formline|getc|getgrent|getgrgid|'
             r'getgrnam|gethostbyaddr|gethostbyname|gethostent|getlogin|'
             r'getnetbyaddr|getnetbyname|getnetent|getpeername|getpgrp|'
             r'getppid|getpriority|getprotobyname|getprotobynumber|'
             r'getprotoent|getpwent|getpwnam|getpwuid|getservbyname|'
             r'getservbyport|getservent|getsockname|getsockopt|glob|gmtime|'
             r'goto|grep|hex|import|index|int|ioctl|join|keys|kill|last|'
             r'lc|lcfirst|length|link|listen|local|localtime|log|lstat|'
             r'map|mkdir|msgctl|msgget|msgrcv|msgsnd|my|next|no|oct|open|'
             r'opendir|ord|our|pack|package|pipe|pop|pos|printf|'
             r'prototype|push|quotemeta|rand|read|readdir|'
             r'readline|readlink|readpipe|recv|redo|ref|rename|require|'
             r'reverse|rewinddir|rindex|rmdir|scalar|seek|seekdir|'
             r'select|semctl|semget|semop|send|setgrent|sethostent|setnetent|'
             r'setpgrp|setpriority|setprotoent|setpwent|setservent|'
             r'setsockopt|shift|shmctl|shmget|shmread|shmwrite|shutdown|'
             r'sin|sleep|socket|socketpair|sort|splice|split|sprintf|sqrt|'
             r'srand|stat|study|substr|symlink|syscall|sysopen|sysread|'
             r'sysseek|system|syswrite|tell|telldir|tie|tied|time|times|tr|'
             r'truncate|uc|ucfirst|umask|undef|unlink|unpack|unshift|untie|'
             r'utime|values|vec|wait|waitpid|wantarray|warn|write'
             r')\b', Name.Builtin),
            (r'((__(DATA|DIE|WARN)__)|(STD(IN|OUT|ERR)))\b', Name.Builtin.Pseudo),
            (r'<<([\'"]?)([a-zA-Z_][a-zA-Z0-9_]*)\1;?\n.*?\n\2\n', String),
            (r'__END__', Comment.Preproc, 'end-part'),
            (r'\$\^[ADEFHILMOPSTWX]', Name.Variable.Global),
            (r"\$[\\\"\[\]'&`+*.,;=%~?@$!<>(^|/-](?!\w)", Name.Variable.Global),
            (r'[$@%#]+', Name.Variable, 'varname'),
            (r'0_?[0-7]+(_[0-7]+)*', Number.Oct),
            (r'0x[0-9A-Fa-f]+(_[0-9A-Fa-f]+)*', Number.Hex),
            (r'0b[01]+(_[01]+)*', Number.Bin),
            (r'(?i)(\d*(_\d*)*\.\d+(_\d*)*|\d+(_\d*)*\.\d+(_\d*)*)(e[+-]?\d+)?',
             Number.Float),
            (r'(?i)\d+(_\d*)*e[+-]?\d+(_\d*)*', Number.Float),
            (r'\d+(_\d+)*', Number.Integer),
            (r"'(\\\\|\\'|[^'])*'", String),
            (r'"(\\\\|\\"|[^"])*"', String),
            (r'`(\\\\|\\`|[^`])*`', String.Backtick),
            (r'<([^\s>]+)>', String.Regex),
            (r'(q|qq|qw|qr|qx)\{', String.Other, 'cb-string'),
            (r'(q|qq|qw|qr|qx)\(', String.Other, 'rb-string'),
            (r'(q|qq|qw|qr|qx)\[', String.Other, 'sb-string'),
            (r'(q|qq|qw|qr|qx)\<', String.Other, 'lt-string'),
            (r'(q|qq|qw|qr|qx)([^a-zA-Z0-9])(.|\n)*?\2', String.Other),
            (r'package\s+', Keyword, 'modulename'),
            (r'sub\s+', Keyword, 'funcname'),
            (r'(\[\]|\*\*|::|<<|>>|>=|<=|<=>|={3}|!=|=~|'
             r'!~|&&?|\|\||\.{1,3})', Operator),
            (r'[-+/*%=<>&^|!\\~]=?', Operator),
            (r'[\(\)\[\]:;,<>/\?\{\}]', Punctuation), # yes, there's no shortage
                                                      # of punctuation in Perl!
            (r'(?=\w)', Name, 'name'),
        ],
        'format': [
            (r'\.\n', String.Interpol, '#pop'),
            (r'[^\n]*\n', String.Interpol),
        ],
        'varname': [
            (r'\s+', Text),
            (r'\{', Punctuation, '#pop'), # hash syntax?
            (r'\)|,', Punctuation, '#pop'), # argument specifier
            (r'[a-zA-Z0-9_]+::', Name.Namespace),
            (r'[a-zA-Z0-9_:]+', Name.Variable, '#pop'),
        ],
        'name': [
            (r'[a-zA-Z0-9_]+::', Name.Namespace),
            (r'[a-zA-Z0-9_:]+', Name, '#pop'),
            (r'[A-Z_]+(?=[^a-zA-Z0-9_])', Name.Constant, '#pop'),
            (r'(?=[^a-zA-Z0-9_])', Text, '#pop'),
        ],
        'modulename': [
            (r'[a-zA-Z_][\w_]*', Name.Namespace, '#pop')
        ],
        'funcname': [
            (r'[a-zA-Z_][\w_]*[\!\?]?', Name.Function),
            (r'\s+', Text),
            # argument declaration
            (r'(\([$@%]*\))(\s*)', bygroups(Punctuation, Text)),
            (r'.*?{', Punctuation, '#pop'),
            (r';', Punctuation, '#pop'),
        ],
        'cb-string': [
            (r'\\[\{\}\\]', String.Other),
            (r'\\', String.Other),
            (r'\{', String.Other, 'cb-string'),
            (r'\}', String.Other, '#pop'),
            (r'[^\{\}\\]+', String.Other)
        ],
        'rb-string': [
            (r'\\[\(\)\\]', String.Other),
            (r'\\', String.Other),
            (r'\(', String.Other, 'rb-string'),
            (r'\)', String.Other, '#pop'),
            (r'[^\(\)]+', String.Other)
        ],
        'sb-string': [
            (r'\\[\[\]\\]', String.Other),
            (r'\\', String.Other),
            (r'\[', String.Other, 'sb-string'),
            (r'\]', String.Other, '#pop'),
            (r'[^\[\]]+', String.Other)
        ],
        'lt-string': [
            (r'\\[\<\>\\]', String.Other),
            (r'\\', String.Other),
            (r'\<', String.Other, 'lt-string'),
            (r'\>', String.Other, '#pop'),
            (r'[^\<\>]+', String.Other)
        ],
        'end-part': [
            (r'.+', Comment.Preproc, '#pop')
        ]
    }

    def analyse_text(text):
        if shebang_matches(text, r'perl'):
            return True
        if 'my $' in text:
            return 0.9
        return 0.1 # who knows, might still be perl!


class LuaLexer(RegexLexer):
    """
    For `Lua <http://www.lua.org>`_ source code.

    Additional options accepted:

    `func_name_highlighting`
        If given and ``True``, highlight builtin function names
        (default: ``True``).
    `disabled_modules`
        If given, must be a list of module names whose function names
        should not be highlighted. By default all modules are highlighted.

        To get a list of allowed modules have a look into the
        `_luabuiltins` module:

        .. sourcecode:: pycon

            >>> from pygments.lexers._luabuiltins import MODULES
            >>> MODULES.keys()
            ['string', 'coroutine', 'modules', 'io', 'basic', ...]
    """

    name = 'Lua'
    aliases = ['lua']
    filenames = ['*.lua', '*.wlua']
    mimetypes = ['text/x-lua', 'application/x-lua']

    tokens = {
        'root': [
            # lua allows a file to start with a shebang
            (r'#!(.*?)$', Comment.Preproc),
            (r'', Text, 'base'),
        ],
        'base': [
            (r'(?s)--\[(=*)\[.*?\]\1\]', Comment.Multiline),
            ('--.*$', Comment.Single),

            (r'(?i)(\d*\.\d+|\d+\.\d*)(e[+-]?\d+)?', Number.Float),
            (r'(?i)\d+e[+-]?\d+', Number.Float),
            ('(?i)0x[0-9a-f]*', Number.Hex),
            (r'\d+', Number.Integer),

            (r'\n', Text),
            (r'[^\S\n]', Text),
            # multiline strings
            (r'(?s)\[(=*)\[.*?\]\1\]', String),

            (r'(==|~=|<=|>=|\.\.|\.\.\.|[=+\-*/%^<>#])', Operator),
            (r'[\[\]\{\}\(\)\.,:;]', Punctuation),
            (r'(and|or|not)\b', Operator.Word),

            ('(break|do|else|elseif|end|for|if|in|repeat|return|then|until|'
             r'while)\b', Keyword),
            (r'(local)\b', Keyword.Declaration),
            (r'(true|false|nil)\b', Keyword.Constant),

            (r'(function)(\s+)', bygroups(Keyword, Text), 'funcname'),
            (r'(class)(\s+)', bygroups(Keyword, Text), 'classname'),

            (r'[A-Za-z_][A-Za-z0-9_]*(\.[A-Za-z_][A-Za-z0-9_]*)?', Name),

            ("'", String.Single, combined('stringescape', 'sqs')),
            ('"', String.Double, combined('stringescape', 'dqs'))
        ],

        'funcname': [
            ('(?:([A-Za-z_][A-Za-z0-9_]*)(\.))?([A-Za-z_][A-Za-z0-9_]*)',
             bygroups(Name.Class, Punctuation, Name.Function), '#pop'),
            # inline function
            ('\(', Punctuation, '#pop'),
        ],

        'classname': [
            ('[A-Za-z_][A-Za-z0-9_]*', Name.Class, '#pop')
        ],

        # if I understand correctly, every character is valid in a lua string,
        # so this state is only for later corrections
        'string': [
            ('.', String)
        ],

        'stringescape': [
            (r'''\\([abfnrtv\\"']|\d{1,3})''', String.Escape)
        ],

        'sqs': [
            ("'", String, '#pop'),
            include('string')
        ],

        'dqs': [
            ('"', String, '#pop'),
            include('string')
        ]
    }

    def __init__(self, **options):
        self.func_name_highlighting = get_bool_opt(
            options, 'func_name_highlighting', True)
        self.disabled_modules = get_list_opt(options, 'disabled_modules', [])

        self._functions = set()
        if self.func_name_highlighting:
            from pygments.lexers._luabuiltins import MODULES
            for mod, func in MODULES.iteritems():
                if mod not in self.disabled_modules:
                    self._functions.update(func)
        RegexLexer.__init__(self, **options)

    def get_tokens_unprocessed(self, text):
        for index, token, value in \
            RegexLexer.get_tokens_unprocessed(self, text):
            if token is Name:
                if value in self._functions:
                    yield index, Name.Builtin, value
                    continue
                elif '.' in value:
                    a, b = value.split('.')
                    yield index, Name, a
                    yield index + len(a), Punctuation, u'.'
                    yield index + len(a) + 1, Name, b
                    continue
            yield index, token, value


class MiniDLexer(RegexLexer):
    """
    For `MiniD <http://www.dsource.org/projects/minid>`_ (a D-like scripting
    language) source.
    """
    name = 'MiniD'
    filenames = ['*.md']
    aliases = ['minid']
    mimetypes = ['text/x-minidsrc']

    tokens = {
        'root': [
            (r'\n', Text),
            (r'\s+', Text),
            # Comments
            (r'//(.*?)\n', Comment.Single),
            (r'/(\\\n)?[*](.|\n)*?[*](\\\n)?/', Comment.Multiline),
            (r'/\+', Comment.Multiline, 'nestedcomment'),
            # Keywords
            (r'(as|assert|break|case|catch|class|continue|coroutine|default'
             r'|do|else|finally|for|foreach|function|global|namespace'
             r'|if|import|in|is|local|module|return|super|switch'
             r'|this|throw|try|vararg|while|with|yield)\b', Keyword),
            (r'(false|true|null)\b', Keyword.Constant),
            # FloatLiteral
            (r'([0-9][0-9_]*)?\.[0-9_]+([eE][+\-]?[0-9_]+)?', Number.Float),
            # IntegerLiteral
            # -- Binary
            (r'0[Bb][01_]+', Number),
            # -- Octal
            (r'0[Cc][0-7_]+', Number.Oct),
            # -- Hexadecimal
            (r'0[xX][0-9a-fA-F_]+', Number.Hex),
            # -- Decimal
            (r'(0|[1-9][0-9_]*)', Number.Integer),
            # CharacterLiteral
            (r"""'(\\['"?\\abfnrtv]|\\x[0-9a-fA-F]{2}|\\[0-9]{1,3}"""
             r"""|\\u[0-9a-fA-F]{4}|\\U[0-9a-fA-F]{8}|.)'""",
             String.Char
            ),
            # StringLiteral
            # -- WysiwygString
            (r'@"(""|.)*"', String),
            # -- AlternateWysiwygString
            (r'`(``|.)*`', String),
            # -- DoubleQuotedString
            (r'"(\\\\|\\"|[^"])*"', String),
            # Tokens
            (
             r'(~=|\^=|%=|\*=|==|!=|>>>=|>>>|>>=|>>|>=|<=>|\?=|-\>'
             r'|<<=|<<|<=|\+\+|\+=|--|-=|\|\||\|=|&&|&=|\.\.|/=)'
             r'|[-/.&$@|\+<>!()\[\]{}?,;:=*%^~#\\]', Punctuation
            ),
            # Identifier
            (r'[a-zA-Z_]\w*', Name),
        ],
        'nestedcomment': [
            (r'[^+/]+', Comment.Multiline),
            (r'/\+', Comment.Multiline, '#push'),
            (r'\+/', Comment.Multiline, '#pop'),
            (r'[+/]', Comment.Multiline),
        ],
    }


class IoLexer(RegexLexer):
    """
    For `Io <http://iolanguage.com/>`_ (a small, prototype-based
    programming language) source.

    *New in Pygments 0.10.*
    """
    name = 'Io'
    filenames = ['*.io']
    aliases = ['io']
    mimetypes = ['text/x-iosrc']
    tokens = {
        'root': [
            (r'\n', Text),
            (r'\s+', Text),
            # Comments
            (r'//(.*?)\n', Comment.Single),
            (r'#(.*?)\n', Comment.Single),
            (r'/(\\\n)?[*](.|\n)*?[*](\\\n)?/', Comment.Multiline),
            (r'/\+', Comment.Multiline, 'nestedcomment'),
            # DoubleQuotedString
            (r'"(\\\\|\\"|[^"])*"', String),
            # Operators
            (r'::=|:=|=|\(|\)|;|,|\*|-|\+|>|<|@|!|/|\||\^|\.|%|&|\[|\]|\{|\}',
             Operator),
            # keywords
            (r'(clone|do|doFile|doString|method|for|if|else|elseif|then)\b',
             Keyword),
            # constants
            (r'(nil|false|true)\b', Name.Constant),
            # names
            ('(Object|list|List|Map|args|Sequence|Coroutine|File)\b',
             Name.Builtin),
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
            # numbers
            (r'(\d+\.?\d*|\d*\.\d+)([eE][+-]?[0-9]+)?', Number.Float),
            (r'\d+', Number.Integer)
        ],
        'nestedcomment': [
            (r'[^+/]+', Comment.Multiline),
            (r'/\+', Comment.Multiline, '#push'),
            (r'\+/', Comment.Multiline, '#pop'),
            (r'[+/]', Comment.Multiline),
        ]
    }


class TclLexer(RegexLexer):
    """
    For Tcl source code.

    *New in Pygments 0.10.*
    """

    keyword_cmds_re = (
        r'\b(after|apply|array|break|catch|continue|elseif|else|error|'
        r'eval|expr|for|foreach|global|if|namespace|proc|rename|return|'
        r'set|switch|then|trace|unset|update|uplevel|upvar|variable|'
        r'vwait|while)\b'
        )

    builtin_cmds_re = (
        r'\b(append|bgerror|binary|cd|chan|clock|close|concat|dde|dict|'
        r'encoding|eof|exec|exit|fblocked|fconfigure|fcopy|file|'
        r'fileevent|flush|format|gets|glob|history|http|incr|info|interp|'
        r'join|lappend|lassign|lindex|linsert|list|llength|load|loadTk|'
        r'lrange|lrepeat|lreplace|lreverse|lsearch|lset|lsort|mathfunc|'
        r'mathop|memory|msgcat|open|package|pid|pkg::create|pkg_mkIndex|'
        r'platform|platform::shell|puts|pwd|re_syntax|read|refchan|'
        r'regexp|registry|regsub|scan|seek|socket|source|split|string|'
        r'subst|tell|time|tm|unknown|unload)\b'
        )

    name = 'Tcl'
    aliases = ['tcl']
    filenames = ['*.tcl']
    mimetypes = ['text/x-tcl', 'text/x-script.tcl', 'application/x-tcl']

    def _gen_command_rules(keyword_cmds_re, builtin_cmds_re, context=""):
        return [
            (keyword_cmds_re, Keyword, 'params' + context),
            (builtin_cmds_re, Name.Builtin, 'params' + context),
            (r'([\w\.\-]+)', Name.Variable, 'params' + context),
            (r'#', Comment, 'comment'),
        ]

    tokens = {
        'root': [
            include('command'),
            include('basic'),
            include('data'),
            (r'}', Keyword),  # HACK: somehow we miscounted our braces
        ],
        'command': _gen_command_rules(keyword_cmds_re, builtin_cmds_re),
        'command-in-brace': _gen_command_rules(keyword_cmds_re,
                                               builtin_cmds_re,
                                               "-in-brace"),
        'command-in-bracket': _gen_command_rules(keyword_cmds_re,
                                                 builtin_cmds_re,
                                                 "-in-bracket"),
        'command-in-paren': _gen_command_rules(keyword_cmds_re,
                                               builtin_cmds_re,
                                               "-in-paren"),
        'basic': [
            (r'\(', Keyword, 'paren'),
            (r'\[', Keyword, 'bracket'),
            (r'\{', Keyword, 'brace'),
            (r'"', String.Double, 'string'),
            (r'(eq|ne|in|ni)\b', Operator.Word),
            (r'!=|==|<<|>>|<=|>=|&&|\|\||\*\*|[-+~!*/%<>&^|?:]', Operator),
        ],
        'data': [
            (r'\s+', Text),
            (r'0x[a-fA-F0-9]+', Number.Hex),
            (r'0[0-7]+', Number.Oct),
            (r'\d+\.\d+', Number.Float),
            (r'\d+', Number.Integer),
            (r'\$([\w\.\-\:]+)', Name.Variable),
            (r'([\w\.\-\:]+)', Text),
        ],
        'params': [
            (r';', Keyword, '#pop'),
            (r'\n', Text, '#pop'),
            (r'(else|elseif|then)', Keyword),
            include('basic'),
            include('data'),
        ],
        'params-in-brace': [
            (r'}', Keyword, ('#pop', '#pop')),
            include('params')
        ],
        'params-in-paren': [
            (r'\)', Keyword, ('#pop', '#pop')),
            include('params')
        ],
        'params-in-bracket': [
            (r'\]', Keyword, ('#pop', '#pop')),
            include('params')
        ],
        'string': [
            (r'\[', String.Double, 'string-square'),
            (r'(?s)(\\\\|\\[0-7]+|\\.|[^"\\])', String.Double),
            (r'"', String.Double, '#pop')
        ],
        'string-square': [
            (r'\[', String.Double, 'string-square'),
            (r'(?s)(\\\\|\\[0-7]+|\\.|\\\n|[^\]\\])', String.Double),
            (r'\]', String.Double, '#pop')
        ],
        'brace': [
            (r'}', Keyword, '#pop'),
            include('command-in-brace'),
            include('basic'),
            include('data'),
        ],
        'paren': [
            (r'\)', Keyword, '#pop'),
            include('command-in-paren'),
            include('basic'),
            include('data'),
        ],
        'bracket': [
            (r'\]', Keyword, '#pop'),
            include('command-in-bracket'),
            include('basic'),
            include('data'),
        ],
        'comment': [
            (r'.*[^\\]\n', Comment, '#pop'),
            (r'.*\\\n', Comment),
        ],
    }

    def analyse_text(text):
        return shebang_matches(text, r'(tcl)')


class ClojureLexer(RegexLexer):
    """
    Lexer for `Clojure <http://clojure.org/>`_ source code.

    *New in Pygments 0.11.*
    """
    name = 'Clojure'
    aliases = ['clojure', 'clj']
    filenames = ['*.clj']
    mimetypes = ['text/x-clojure', 'application/x-clojure']

    keywords = [
        'fn', 'def', 'defn', 'defmacro', 'defmethod', 'defmulti', 'defn-',
        'defstruct',
        'if', 'cond',
        'let', 'for'
    ]
    builtins = [
        '.', '..',
        '*', '+', '-', '->', '..', '/', '<', '<=', '=', '==', '>', '>=',
        'accessor', 'agent', 'agent-errors', 'aget', 'alength', 'all-ns',
        'alter', 'and', 'append-child', 'apply', 'array-map', 'aset',
        'aset-boolean', 'aset-byte', 'aset-char', 'aset-double', 'aset-float',
        'aset-int', 'aset-long', 'aset-short', 'assert', 'assoc', 'await',
        'await-for', 'bean', 'binding', 'bit-and', 'bit-not', 'bit-or',
        'bit-shift-left', 'bit-shift-right', 'bit-xor', 'boolean', 'branch?',
        'butlast', 'byte', 'cast', 'char', 'children', 'class',
        'clear-agent-errors', 'comment', 'commute', 'comp', 'comparator',
        'complement', 'concat', 'conj', 'cons', 'constantly',
        'construct-proxy', 'contains?', 'count', 'create-ns', 'create-struct',
        'cycle', 'dec',  'deref', 'difference', 'disj', 'dissoc', 'distinct',
        'doall', 'doc', 'dorun', 'doseq', 'dosync', 'dotimes', 'doto',
        'double', 'down', 'drop', 'drop-while', 'edit', 'end?', 'ensure',
        'eval', 'every?', 'false?', 'ffirst', 'file-seq', 'filter', 'find',
        'find-doc', 'find-ns', 'find-var', 'first', 'float', 'flush',
        'fnseq', 'frest', 'gensym', 'get', 'get-proxy-class',
        'hash-map', 'hash-set', 'identical?', 'identity', 'if-let', 'import',
        'in-ns', 'inc', 'index', 'insert-child', 'insert-left', 'insert-right',
        'inspect-table', 'inspect-tree', 'instance?', 'int', 'interleave',
        'intersection', 'into', 'into-array', 'iterate', 'join', 'key', 'keys',
        'keyword', 'keyword?', 'last', 'lazy-cat', 'lazy-cons', 'left',
        'lefts', 'line-seq', 'list', 'list*', 'load', 'load-file',
        'locking', 'long', 'loop', 'macroexpand', 'macroexpand-1',
        'make-array', 'make-node', 'map', 'map-invert', 'map?', 'mapcat',
        'max', 'max-key', 'memfn', 'merge', 'merge-with', 'meta', 'min',
        'min-key', 'name', 'namespace', 'neg?', 'new', 'newline', 'next',
        'nil?', 'node', 'not', 'not-any?', 'not-every?', 'not=', 'ns-imports',
        'ns-interns', 'ns-map', 'ns-name', 'ns-publics', 'ns-refers',
        'ns-resolve', 'ns-unmap', 'nth', 'nthrest', 'or', 'parse', 'partial',
        'path', 'peek', 'pop', 'pos?', 'pr', 'pr-str', 'print', 'print-str',
        'println', 'println-str', 'prn', 'prn-str', 'project', 'proxy',
        'proxy-mappings', 'quot', 'rand', 'rand-int', 'range', 're-find',
        're-groups', 're-matcher', 're-matches', 're-pattern', 're-seq',
        'read', 'read-line', 'reduce', 'ref', 'ref-set', 'refer', 'rem',
        'remove', 'remove-method', 'remove-ns', 'rename', 'rename-keys',
        'repeat', 'replace', 'replicate', 'resolve', 'rest', 'resultset-seq',
        'reverse', 'rfirst', 'right', 'rights', 'root', 'rrest', 'rseq',
        'second', 'select', 'select-keys', 'send', 'send-off', 'seq',
        'seq-zip', 'seq?', 'set', 'short', 'slurp', 'some', 'sort',
        'sort-by', 'sorted-map', 'sorted-map-by', 'sorted-set',
        'special-symbol?', 'split-at', 'split-with', 'str', 'string?',
        'struct', 'struct-map', 'subs', 'subvec', 'symbol', 'symbol?',
        'sync', 'take', 'take-nth', 'take-while', 'test', 'time', 'to-array',
        'to-array-2d', 'tree-seq', 'true?', 'union', 'up', 'update-proxy',
        'val', 'vals', 'var-get', 'var-set', 'var?', 'vector', 'vector-zip',
        'vector?', 'when', 'when-first', 'when-let', 'when-not',
        'with-local-vars', 'with-meta', 'with-open', 'with-out-str',
        'xml-seq', 'xml-zip', 'zero?', 'zipmap', 'zipper']

    # valid names for identifiers
    # well, names can only not consist fully of numbers
    # but this should be good enough for now
    valid_name = r'[a-zA-Z0-9!$%&*+,/:<=>?@^_~-]+'

    tokens = {
        'root' : [
            # the comments - always starting with semicolon
            # and going to the end of the line
            (r';.*$', Comment.Single),

            # whitespaces - usually not relevant
            (r'\s+', Text),

            # numbers
            (r'-?\d+\.\d+', Number.Float),
            (r'-?\d+', Number.Integer),
            # support for uncommon kinds of numbers -
            # have to figure out what the characters mean
            #(r'(#e|#i|#b|#o|#d|#x)[\d.]+', Number),

            # strings, symbols and characters
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'" + valid_name, String.Symbol),
            (r"\\([()/'\".'_!Â§$%& ?;=#+-]{1}|[a-zA-Z0-9]+)", String.Char),

            # constants
            (r'(#t|#f)', Name.Constant),

            # special operators
            (r"('|#|`|,@|,|\.)", Operator),

            # highlight the keywords
            ('(%s)' % '|'.join([
                re.escape(entry) + ' ' for entry in keywords]),
                Keyword
            ),

            # first variable in a quoted string like
            # '(this is syntactic sugar)
            (r"(?<='\()" + valid_name, Name.Variable),
            (r"(?<=#\()" + valid_name, Name.Variable),

            # highlight the builtins
            ("(?<=\()(%s)" % '|'.join([
                re.escape(entry) + ' ' for entry in builtins]),
                Name.Builtin
            ),

            # the remaining functions
            (r'(?<=\()' + valid_name, Name.Function),
            # find the remaining variables
            (valid_name, Name.Variable),

            # Clojure accepts vector notation
            (r'(\[|\])', Punctuation),

            # Clojure accepts map notation
            (r'(\{|\})', Punctuation),

            # the famous parentheses!
            (r'(\(|\))', Punctuation),
        ],
    }


class FactorLexer(RegexLexer):
    """
    Lexer for the `Factor <http://factorcode.org>`_ language.

    *New in Pygments 1.4.*
    """
    name = 'Factor'
    aliases = ['factor']
    filenames = ['*.factor']
    mimetypes = ['text/x-factor']

    flags = re.MULTILINE | re.UNICODE

    builtin_kernel = (
        r'(?:or|2bi|2tri|while|wrapper|nip|4dip|wrapper\\?|bi\\*|'
        r'callstack>array|both\\?|hashcode|die|dupd|callstack|'
        r'callstack\\?|3dup|tri@|pick|curry|build|\\?execute|3bi|'
        r'prepose|>boolean|\\?if|clone|eq\\?|tri\\*|\\?|=|swapd|'
        r'2over|2keep|3keep|clear|2dup|when|not|tuple\\?|dup|2bi\\*|'
        r'2tri\\*|call|tri-curry|object|bi@|do|unless\\*|if\\*|loop|'
        r'bi-curry\\*|drop|when\\*|assert=|retainstack|assert\\?|-rot|'
        r'execute|2bi@|2tri@|boa|with|either\\?|3drop|bi|curry\\?|'
        r'datastack|until|3dip|over|3curry|tri-curry\\*|tri-curry@|swap|'
        r'and|2nip|throw|bi-curry|\\(clone\\)|hashcode\\*|compose|2dip|if|3tri|'
        r'unless|compose\\?|tuple|keep|2curry|equal\\?|assert|tri|2drop|'
        r'most|<wrapper>|boolean\\?|identity-hashcode|identity-tuple\\?|'
        r'null|new|dip|bi-curry@|rot|xor|identity-tuple|boolean)\s'
        )

    builtin_assocs = (
        r'(?:\\?at|assoc\\?|assoc-clone-like|assoc=|delete-at\\*|'
        r'assoc-partition|extract-keys|new-assoc|value\\?|assoc-size|'
        r'map>assoc|push-at|assoc-like|key\\?|assoc-intersect|'
        r'assoc-refine|update|assoc-union|assoc-combine|at\\*|'
        r'assoc-empty\\?|at\\+|set-at|assoc-all\\?|assoc-subset\\?|'
        r'assoc-hashcode|change-at|assoc-each|assoc-diff|zip|values|'
        r'value-at|rename-at|inc-at|enum\\?|at|cache|assoc>map|<enum>|'
        r'assoc|assoc-map|enum|value-at\\*|assoc-map-as|>alist|'
        r'assoc-filter-as|clear-assoc|assoc-stack|maybe-set-at|'
        r'substitute|assoc-filter|2cache|delete-at|assoc-find|keys|'
        r'assoc-any\\?|unzip)\s'
        )

    builtin_combinators = (
        r'(?:case|execute-effect|no-cond|no-case\\?|3cleave>quot|2cleave|'
        r'cond>quot|wrong-values\\?|no-cond\\?|cleave>quot|no-case|'
        r'case>quot|3cleave|wrong-values|to-fixed-point|alist>quot|'
        r'case-find|cond|cleave|call-effect|2cleave>quot|recursive-hashcode|'
        r'linear-case-quot|spread|spread>quot)\s'
        )

    builtin_math = (
        r'(?:number=|if-zero|next-power-of-2|each-integer|\\?1\\+|'
        r'fp-special\\?|imaginary-part|unless-zero|float>bits|number\\?|'
        r'fp-infinity\\?|bignum\\?|fp-snan\\?|denominator|fp-bitwise=|\\*|'
        r'\\+|power-of-2\\?|-|u>=|/|>=|bitand|log2-expects-positive|<|'
        r'log2|>|integer\\?|number|bits>double|2/|zero\\?|(find-integer)|'
        r'bits>float|float\\?|shift|ratio\\?|even\\?|ratio|fp-sign|bitnot|'
        r'>fixnum|complex\\?|/i|/f|byte-array>bignum|when-zero|sgn|>bignum|'
        r'next-float|u<|u>|mod|recip|rational|find-last-integer|>float|'
        r'(all-integers\\?)|2^|times|integer|fixnum\\?|neg|fixnum|sq|'
        r'bignum|(each-integer)|bit\\?|fp-qnan\\?|find-integer|complex|'
        r'<fp-nan>|real|double>bits|bitor|rem|fp-nan-payload|all-integers\\?|'
        r'real-part|log2-expects-positive\\?|prev-float|align|unordered\\?|'
        r'float|fp-nan\\?|abs|bitxor|u<=|odd\\?|<=|/mod|rational\\?|>integer|'
        r'real\\?|numerator)\s'
        )

    builtin_sequences = (
        r'(?:member-eq\\?|append|assert-sequence=|find-last-from|trim-head-slice|'
        r'clone-like|3sequence|assert-sequence\\?|map-as|last-index-from|'
        r'reversed|index-from|cut\\*|pad-tail|remove-eq!|concat-as|'
        r'but-last|snip|trim-tail|nths|nth|2selector|sequence|slice\\?|'
        r'<slice>|partition|remove-nth|tail-slice|empty\\?|tail\\*|'
        r'if-empty|find-from|virtual-sequence\\?|member\\?|set-length|'
        r'drop-prefix|unclip|unclip-last-slice|iota|map-sum|'
        r'bounds-error\\?|sequence-hashcode-step|selector-for|'
        r'accumulate-as|map|start|midpoint@|\\(accumulate\\)|rest-slice|'
        r'prepend|fourth|sift|accumulate!|new-sequence|follow|map!|'
        r'like|first4|1sequence|reverse|slice|unless-empty|padding|'
        r'virtual@|repetition\\?|set-last|index|4sequence|max-length|'
        r'set-second|immutable-sequence|first2|first3|replicate-as|'
        r'reduce-index|unclip-slice|supremum|suffix!|insert-nth|'
        r'trim-tail-slice|tail|3append|short|count|suffix|concat|'
        r'flip|filter|sum|immutable\\?|reverse!|2sequence|map-integers|'
        r'delete-all|start\\*|indices|snip-slice|check-slice|sequence\\?|'
        r'head|map-find|filter!|append-as|reduce|sequence=|halves|'
        r'collapse-slice|interleave|2map|filter-as|binary-reduce|'
        r'slice-error\\?|product|bounds-check\\?|bounds-check|harvest|'
        r'immutable|virtual-exemplar|find|produce|remove|pad-head|last|'
        r'replicate|set-fourth|remove-eq|shorten|reversed\\?|'
        r'map-find-last|3map-as|2unclip-slice|shorter\\?|3map|find-last|'
        r'head-slice|pop\\*|2map-as|tail-slice\\*|but-last-slice|'
        r'2map-reduce|iota\\?|collector-for|accumulate|each|selector|'
        r'append!|new-resizable|cut-slice|each-index|head-slice\\*|'
        r'2reverse-each|sequence-hashcode|pop|set-nth|\\?nth|'
        r'<flat-slice>|second|join|when-empty|collector|'
        r'immutable-sequence\\?|<reversed>|all\\?|3append-as|'
        r'virtual-sequence|subseq\\?|remove-nth!|push-either|new-like|'
        r'length|last-index|push-if|2all\\?|lengthen|assert-sequence|'
        r'copy|map-reduce|move|third|first|3each|tail\\?|set-first|'
        r'prefix|bounds-error|any\\?|<repetition>|trim-slice|exchange|'
        r'surround|2reduce|cut|change-nth|min-length|set-third|produce-as|'
        r'push-all|head\\?|delete-slice|rest|sum-lengths|2each|head\\*|'
        r'infimum|remove!|glue|slice-error|subseq|trim|replace-slice|'
        r'push|repetition|map-index|trim-head|unclip-last|mismatch)\s'
        )

    builtin_namespaces = (
        r'(?:global|\\+@|change|set-namestack|change-global|init-namespaces|'
        r'on|off|set-global|namespace|set|with-scope|bind|with-variable|'
        r'inc|dec|counter|initialize|namestack|get|get-global|make-assoc)\s'
        )

    builtin_arrays = (
        r'(?:<array>|2array|3array|pair|>array|1array|4array|pair\\?|'
        r'array|resize-array|array\\?)\s'
        )

    builtin_io = (
        r'(?:\\+character\\+|bad-seek-type\\?|readln|each-morsel|stream-seek|'
        r'read|print|with-output-stream|contents|write1|stream-write1|'
        r'stream-copy|stream-element-type|with-input-stream|'
        r'stream-print|stream-read|stream-contents|stream-tell|'
        r'tell-output|bl|seek-output|bad-seek-type|nl|stream-nl|write|'
        r'flush|stream-lines|\\+byte\\+|stream-flush|read1|'
        r'seek-absolute\\?|stream-read1|lines|stream-readln|'
        r'stream-read-until|each-line|seek-end|with-output-stream\\*|'
        r'seek-absolute|with-streams|seek-input|seek-relative\\?|'
        r'input-stream|stream-write|read-partial|seek-end\\?|'
        r'seek-relative|error-stream|read-until|with-input-stream\\*|'
        r'with-streams\\*|tell-input|each-block|output-stream|'
        r'stream-read-partial|each-stream-block|each-stream-line)\s'
        )

    builtin_strings = (
        r'(?:resize-string|>string|<string>|1string|string|string\\?)\s'
        )

    builtin_vectors = (
        r'(?:vector\\?|<vector>|\\?push|vector|>vector|1vector)\s'
        )

    builtin_continuations = (
        r'(?:with-return|restarts|return-continuation|with-datastack|'
        r'recover|rethrow-restarts|<restart>|ifcc|set-catchstack|'
        r'>continuation<|cleanup|ignore-errors|restart\\?|'
        r'compute-restarts|attempt-all-error|error-thread|continue|'
        r'<continuation>|attempt-all-error\\?|condition\\?|'
        r'<condition>|throw-restarts|error|catchstack|continue-with|'
        r'thread-error-hook|continuation|rethrow|callcc1|'
        r'error-continuation|callcc0|attempt-all|condition|'
        r'continuation\\?|restart|return)\s'
        )

    tokens = {
        'root': [
            # TODO: (( inputs -- outputs ))
            # TODO: << ... >>

            # defining words
            (r'(\s*)(:|::|MACRO:|MEMO:)(\s+)(\S+)',
                bygroups(Text, Keyword, Text, Name.Function)),
            (r'(\s*)(M:)(\s+)(\S+)(\s+)(\S+)',
                bygroups(Text, Keyword, Text, Name.Class, Text, Name.Function)),
            (r'(\s*)(GENERIC:)(\s+)(\S+)',
                bygroups(Text, Keyword, Text, Name.Function)),
            (r'(\s*)(HOOK:|GENERIC#)(\s+)(\S+)(\s+)(\S+)',
                bygroups(Text, Keyword, Text, Name.Function, Text, Name.Function)),
            (r'(\()(\s+)', bygroups(Name.Function, Text), 'stackeffect'),
            (r'\;\s', Keyword),

            # imports and namespaces
            (r'(USING:)((?:\s|\\\s)+)', bygroups(Keyword.Namespace, Text), 'import'),
            (r'(USE:)(\s+)(\S+)', bygroups(Keyword.Namespace, Text, Name.Namespace)),
            (r'(UNUSE:)(\s+)(\S+)', bygroups(Keyword.Namespace, Text, Name.Namespace)),
            (r'(QUALIFIED:)(\s+)(\S+)',
                bygroups(Keyword.Namespace, Text, Name.Namespace)),
            (r'(QUALIFIED-WITH:)(\s+)(\S+)',
                bygroups(Keyword.Namespace, Text, Name.Namespace)),
            (r'(FROM:|EXCLUDE:)(\s+)(\S+)(\s+)(=>)',
                bygroups(Keyword.Namespace, Text, Name.Namespace, Text, Text)),
            (r'(IN:)(\s+)(\S+)', bygroups(Keyword.Namespace, Text, Name.Namespace)),
            (r'(?:ALIAS|DEFER|FORGET|POSTPONE):', Keyword.Namespace),

            # tuples and classes
            (r'(TUPLE:)(\s+)(\S+)(\s+<\s+)(\S+)',
                bygroups(Keyword, Text, Name.Class, Text, Name.Class), 'slots'),
            (r'(TUPLE:)(\s+)(\S+)', bygroups(Keyword, Text, Name.Class), 'slots'),
            (r'(UNION:)(\s+)(\S+)', bygroups(Keyword, Text, Name.Class)),
            (r'(INTERSECTION:)(\s+)(\S+)', bygroups(Keyword, Text, Name.Class)),
            (r'(PREDICATE:)(\s+)(\S+)(\s+<\s+)(\S+)',
                bygroups(Keyword, Text, Name.Class, Text, Name.Class)),
            (r'(C:)(\s+)(\S+)(\s+)(\S+)',
                bygroups(Keyword, Text, Name.Function, Text, Name.Class)),
            (r'INSTANCE:', Keyword),
            (r'SLOT:', Keyword),
            (r'MIXIN:', Keyword),
            (r'(?:SINGLETON|SINGLETONS):', Keyword),

            # other syntax
            (r'CONSTANT:', Keyword),
            (r'(?:SYMBOL|SYMBOLS):', Keyword),
            (r'ERROR:', Keyword),
            (r'SYNTAX:', Keyword),
            (r'(HELP:)(\s+)(\S+)', bygroups(Keyword, Text, Name.Function)),
            (r'(MAIN:)(\s+)(\S+)', bygroups(Keyword.Namespace, Text, Name.Function)),
            (r'(?:ALIEN|TYPEDEF|FUNCTION|STRUCT):', Keyword),

            # vocab.private
            # TODO: words inside vocab.private should have red names?
            (r'(?:<PRIVATE|PRIVATE>)', Keyword.Namespace),

            # strings
            (r'"""\s+(?:.|\n)*?\s+"""', String),
            (r'"(?:\\\\|\\"|[^"])*"', String),
            (r'CHAR:\s+(\\[\\abfnrstv]*|\S)\s', String.Char),

            # comments
            (r'\!\s+.*$', Comment),
            (r'#\!\s+.*$', Comment),

            # boolean constants
            (r'(t|f)\s', Name.Constant),

            # numbers
            (r'-?\d+\.\d+\s', Number.Float),
            (r'-?\d+\s', Number.Integer),
            (r'HEX:\s+[a-fA-F\d]+\s', Number.Hex),
            (r'BIN:\s+[01]+\s', Number.Integer),
            (r'OCT:\s+[0-7]+\s', Number.Oct),

            # operators
            (r'[-+/*=<>^]\s', Operator),

            # keywords
            (r'(?:deprecated|final|foldable|flushable|inline|recursive)\s', Keyword),

            # builtins
            (builtin_kernel, Name.Builtin),
            (builtin_assocs, Name.Builtin),
            (builtin_combinators, Name.Builtin),
            (builtin_math, Name.Builtin),
            (builtin_sequences, Name.Builtin),
            (builtin_namespaces, Name.Builtin),
            (builtin_arrays, Name.Builtin),
            (builtin_io, Name.Builtin),
            (builtin_strings, Name.Builtin),
            (builtin_vectors, Name.Builtin),
            (builtin_continuations, Name.Builtin),

            # whitespaces - usually not relevant
            (r'\s+', Text),

            # everything else is text
            (r'\S+', Text),
        ],

        'stackeffect': [
            (r'\s*\(', Name.Function, 'stackeffect'),
            (r'\)', Name.Function, '#pop'),
            (r'\-\-', Name.Function),
            (r'\s+', Text),
            (r'\S+', Name.Variable),
        ],

        'slots': [
            (r'\s+', Text),
            (r';\s', Keyword, '#pop'),
            (r'\S+', Name.Variable),
        ],

        'import': [
            (r';', Keyword, '#pop'),
            (r'\S+', Name.Namespace),
            (r'\s+', Text),
        ],
    }


class IokeLexer(RegexLexer):
    """
    For `Ioke <http://ioke.org/>`_ (a strongly typed, dynamic,
    prototype based programming language) source.

    *New in Pygments 1.4.*
    """
    name = 'Ioke'
    filenames = ['*.ik']
    aliases = ['ioke', 'ik']
    mimetypes = ['text/x-iokesrc']
    tokens = {
        'interpolatableText': [
            (r'(\\b|\\e|\\t|\\n|\\f|\\r|\\"|\\\\|\\#|\\\Z|\\u[0-9a-fA-F]{1,4}'
             r'|\\[0-3]?[0-7]?[0-7])', String.Escape),
            (r'#{', Punctuation, 'textInterpolationRoot')
            ],

        'text': [
            (r'(?<!\\)"', String, '#pop'),
            include('interpolatableText'),
            (r'[^"]', String)
            ],

        'documentation': [
            (r'(?<!\\)"', String.Doc, '#pop'),
            include('interpolatableText'),
            (r'[^"]', String.Doc)
            ],

        'textInterpolationRoot': [
            (r'}', Punctuation, '#pop'),
            include('root')
            ],

        'slashRegexp': [
            (r'(?<!\\)/[oxpniums]*', String.Regex, '#pop'),
            include('interpolatableText'),
            (r'\\/', String.Regex),
            (r'[^/]', String.Regex)
            ],

        'squareRegexp': [
            (r'(?<!\\)][oxpniums]*', String.Regex, '#pop'),
            include('interpolatableText'),
            (r'\\]', String.Regex),
            (r'[^\]]', String.Regex)
            ],

        'squareText': [
            (r'(?<!\\)]', String, '#pop'),
            include('interpolatableText'),
            (r'[^\]]', String)
            ],

        'root': [
            (r'\n', Text),
            (r'\s+', Text),

            # Comments
            (r';(.*?)\n', Comment),
            (r'\A#!(.*?)\n', Comment),

            #Regexps
            (r'#/', String.Regex, 'slashRegexp'),
            (r'#r\[', String.Regex, 'squareRegexp'),

            #Symbols
            (r':[a-zA-Z0-9_!:?]+', String.Symbol),
            (r'[a-zA-Z0-9_!:?]+:(?![a-zA-Z0-9_!?])', String.Other),
            (r':"(\\\\|\\"|[^"])*"', String.Symbol),

            #Documentation
            (r'((?<=fn\()|(?<=fnx\()|(?<=method\()|(?<=macro\()|(?<=lecro\()'
             r'|(?<=syntax\()|(?<=dmacro\()|(?<=dlecro\()|(?<=dlecrox\()'
             r'|(?<=dsyntax\())[\s\n\r]*"', String.Doc, 'documentation'),

            #Text
            (r'"', String, 'text'),
            (r'#\[', String, 'squareText'),

            #Mimic
            (r'[a-zA-Z0-9_][a-zA-Z0-9!?_:]+(?=\s*=.*mimic\s)', Name.Entity),

            #Assignment
            (r'[a-zA-Z_][a-zA-Z0-9_!:?]*(?=[\s]*[+*/-]?=[^=].*($|\.))', Name.Variable),

            # keywords
            (r'(break|cond|continue|do|ensure|for|for:dict|for:set|if|let|'
             r'loop|p:for|p:for:dict|p:for:set|return|unless|until|while|'
             r'with)(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            # Origin
            (r'(eval|mimic|print|println)(?![a-zA-Z0-9!:_?])', Keyword),

            # Base
            (r'(cell\?|cellNames|cellOwner\?|cellOwner|cells|cell|'
             r'documentation|hash|identity|mimic|removeCell\!|undefineCell\!)'
             r'(?![a-zA-Z0-9!:_?])', Keyword),

            # Ground
            (r'(stackTraceAsText)(?![a-zA-Z0-9!:_?])', Keyword),

            #DefaultBehaviour Literals
            (r'(dict|list|message|set)(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            #DefaultBehaviour Case
            (r'(case|case:and|case:else|case:nand|case:nor|case:not|case:or|'
             r'case:otherwise|case:xor)(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            #DefaultBehaviour Reflection
            (r'(asText|become\!|derive|freeze\!|frozen\?|in\?|is\?|kind\?|'
             r'mimic\!|mimics|mimics\?|prependMimic\!|removeAllMimics\!|'
             r'removeMimic\!|same\?|send|thaw\!|uniqueHexId)'
             r'(?![a-zA-Z0-9!:_?])', Keyword),

            #DefaultBehaviour Aspects
            (r'(after|around|before)(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            # DefaultBehaviour
            (r'(kind|cellDescriptionDict|cellSummary|genSym|inspect|notice)'
             r'(?![a-zA-Z0-9!:_?])', Keyword),
            (r'(use|destructuring)', Keyword.Reserved),

            #DefaultBehavior BaseBehavior
            (r'(cell\?|cellOwner\?|cellOwner|cellNames|cells|cell|'
             r'documentation|identity|removeCell!|undefineCell)'
             r'(?![a-zA-Z0-9!:_?])', Keyword),

            #DefaultBehavior Internal
            (r'(internal:compositeRegexp|internal:concatenateText|'
             r'internal:createDecimal|internal:createNumber|'
             r'internal:createRegexp|internal:createText)'
             r'(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            #DefaultBehaviour Conditions
            (r'(availableRestarts|bind|error\!|findRestart|handle|'
             r'invokeRestart|rescue|restart|signal\!|warn\!)'
             r'(?![a-zA-Z0-9!:_?])', Keyword.Reserved),

            # constants
            (r'(nil|false|true)(?![a-zA-Z0-9!:_?])', Name.Constant),

            # names
            (r'(Arity|Base|Call|Condition|DateTime|Aspects|Pointcut|'
             r'Assignment|BaseBehavior|Boolean|Case|AndCombiner|Else|'
             r'NAndCombiner|NOrCombiner|NotCombiner|OrCombiner|XOrCombiner|'
             r'Conditions|Definitions|FlowControl|Internal|Literals|'
             r'Reflection|DefaultMacro|DefaultMethod|DefaultSyntax|Dict|'
             r'FileSystem|Ground|Handler|Hook|IO|IokeGround|Struct|'
             r'LexicalBlock|LexicalMacro|List|Message|Method|Mixins|'
             r'NativeMethod|Number|Origin|Pair|Range|Reflector|Regexp Match|'
             r'Regexp|Rescue|Restart|Runtime|Sequence|Set|Symbol|'
             r'System|Text|Tuple)(?![a-zA-Z0-9!:_?])', Name.Builtin),

            # functions
            (ur'(generateMatchMethod|aliasMethod|\u03bb|\u028E|fnx|fn|method|'
             ur'dmacro|dlecro|syntax|macro|dlecrox|lecrox|lecro|syntax)'
             ur'(?![a-zA-Z0-9!:_?])', Name.Function),

            # Numbers
            (r'-?0[xX][0-9a-fA-F]+', Number.Hex),
            (r'-?(\d+\.?\d*|\d*\.\d+)([eE][+-]?[0-9]+)?', Number.Float),
            (r'-?\d+', Number.Integer),

            (r'#\(', Punctuation),

             # Operators
            (ur'(&&>>|\|\|>>|\*\*>>|:::|::|\.\.\.|===|\*\*>|\*\*=|&&>|&&=|'
             ur'\|\|>|\|\|=|\->>|\+>>|!>>|<>>>|<>>|&>>|%>>|#>>|@>>|/>>|\*>>|'
             ur'\?>>|\|>>|\^>>|~>>|\$>>|=>>|<<=|>>=|<=>|<\->|=~|!~|=>|\+\+|'
             ur'\-\-|<=|>=|==|!=|&&|\.\.|\+=|\-=|\*=|\/=|%=|&=|\^=|\|=|<\-|'
             ur'\+>|!>|<>|&>|%>|#>|\@>|\/>|\*>|\?>|\|>|\^>|~>|\$>|<\->|\->|'
             ur'<<|>>|\*\*|\?\||\?&|\|\||>|<|\*|\/|%|\+|\-|&|\^|\||=|\$|!|~|'
             ur'\?|#|\u2260|\u2218|\u2208|\u2209)', Operator),
            (r'(and|nand|or|xor|nor|return|import)(?![a-zA-Z0-9_!?])',
             Operator),

            # Punctuation
            (r'(\`\`|\`|\'\'|\'|\.|\,|@|@@|\[|\]|\(|\)|{|})', Punctuation),

            #kinds
            (r'[A-Z][a-zA-Z0-9_!:?]*', Name.Class),

            #default cellnames
            (r'[a-z_][a-zA-Z0-9_!:?]*', Name)
        ]
    }
