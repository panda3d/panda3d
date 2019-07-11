########################################################################
#
# Documentation generator for panda.
#
# How to use this module:
#
#   from direct.directscripts import gendocs
#   gendocs.generate(version, indirlist, directdirlist, docdir, header, footer, urlprefix, urlsuffix)
#
#   - version is the panda version number
#
#   - indirlist is the name of a directory, or a list of directories,
#     containing the "xxx.in" files that interrogate generates.  No
#     slash at end.
#
#   - directdirlist is the name of a directory, or a list of
#     directories, containing the source code for "direct," as well as
#     for other Python-based trees that should be included in the
#     documentation pages.  No slash at end.
#
#   - docdir is the name of a directory into which HTML files
#     will be emitted.  No slash at end.
#
#   - header is a string that will be placed at the front of
#     every HTML page.
#
#   - footer is a string that will be placed at the end of
#     every HTML page.
#
#   - urlprefix is a string that will be appended to the front of
#     every URL.
#
#   - urlsuffix is a string that will be appended to the end of
#     every URL.
#
########################################################################
#
# The major subsystems are:
#
# * The module that loads interrogate databases.
#
# * The module that loads python parse-trees.
#
# * The "code database", which provides a single access point
#   for both interrogate databases and python parse trees.
#
# * The HTML generator.
#
########################################################################

import os, sys, parser, symbol, token, re

########################################################################
#
# assorted utility functions
#
########################################################################

SECHEADER = re.compile("^[A-Z][a-z]+\\s*:")
JUNKHEADER = re.compile("^((Function)|(Access))\\s*:")
IMPORTSTAR = re.compile("^from\\s+([a-zA-Z0-9_.]+)\\s+import\\s+[*]\\s*$")
IDENTIFIER = re.compile("[a-zA-Z0-9_]+")
FILEHEADER = re.compile(
r"""^// Filename: [a-zA-Z.]+
// Created by:  [a-zA-Z. ()0-9]+(
//)?
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright \(c\) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////""")

def readFile(fn):
    try:
        srchandle = open(fn, "r")
        data = srchandle.read()
        srchandle.close()
        return data
    except:
        sys.exit("Cannot read "+fn)

def writeFile(wfile, data):
    try:
        dsthandle = open(wfile, "wb")
        dsthandle.write(data)
        dsthandle.close()
    except:
        sys.exit("Cannot write "+wfile)

def writeFileLines(wfile, lines):
    try:
        dsthandle = open(wfile, "wb")
        for x in lines:
            dsthandle.write(x)
            dsthandle.write("\n")
        dsthandle.close()
    except:
        sys.exit("Cannot write "+wfile)

def findFiles(dirlist, ext, ign, list):
    if isinstance(dirlist, str):
        dirlist = [dirlist]
    for dir in dirlist:
        for file in os.listdir(dir):
            full = dir + "/" + file
            if full not in ign and file not in ign:
                if (os.path.isfile(full)):
                    if (file.endswith(ext)):
                        list.append(full)
                elif (os.path.isdir(full)):
                    findFiles(full, ext, ign, list)

def pathToModule(result):
    if (result[-3:]==".py"): result=result[:-3]
    result = result.replace("/src/","/")
    result = result.replace("/",".")
    return result

def textToHTML(comment, sep, delsection=None):
    sections = [""]
    included = {}
    for line in comment.split("\n"):
        line = line.lstrip(" ").lstrip(sep).lstrip(" ").rstrip("\r").rstrip(" ")
        if (line == ""):
            sections.append("")
        elif (line[0]=="*") or (line[0]=="-"):
            sections.append(line)
            sections.append("")
        elif (SECHEADER.match(line)):
            sections.append(line)
        else:
            sections[-1] = sections[-1] + " " + line
    total = ""
    for sec in sections:
        if (sec != ""):
            sec = sec.replace("&","&amp;")
            sec = sec.replace("<","&lt;")
            sec = sec.replace(">","&gt;")
            sec = sec.replace("  "," ")
            sec = sec.replace("  "," ")
            if (delsection != None) and (delsection.match(sec)):
                included[sec] = 1
            if sec not in included:
                included[sec] = 1
                total = total + sec + "<br>\n"
    return total

def linkTo(link, text):
    return '<a href="' + link + '">' + text + '</a>'

def convertToPythonFn(fn):
    result = ""
    lastc = 0
    for c in fn:
        if (c!="_"):
            if (lastc=="_"):
                result = result + c.upper()
            else:
                result = result + c
        lastc = c
    return result

def removeFileLicense(content):
    # Removes the license part at the top of a file.
    return re.sub(FILEHEADER, "", content).strip()

########################################################################
#
# Interrogate Database Tokenizer
#
########################################################################

class InterrogateTokenizer:
    """
    A big string, with a "parse pointer", and routines to
    extract integers and strings.  The token syntax is that
    used by interrogate databases.
    """

    def __init__(self, fn):
        self.fn = fn
        self.pos = 0
        self.data = readFile(fn)

    def readint(self):
        neg = 0
        while (self.data[self.pos].isspace()):
            self.pos += 1
        if (self.data[self.pos] == "-"):
            neg = 1
            self.pos += 1
        if (self.data[self.pos].isdigit()==0):
            print("File position " + str(self.pos))
            print("Text: " + self.data[self.pos:self.pos+50])
            sys.exit("Syntax error in interrogate file format 0")
        value = 0
        while (self.data[self.pos].isdigit()):
            value = value*10 + int(self.data[self.pos])
            self.pos += 1
        if (neg): value = -value
        return value

    def readstring(self):
        length = self.readint()
        if (self.data[self.pos].isspace()==0):
            sys.exit("Syntax error in interrogate file format 1")
        self.pos += 1
        body = self.data[self.pos:self.pos+length]
        if (len(body) != length):
            sys.exit("Syntax error in interrogate file format 2")
        self.pos += length
        return body

########################################################################
#
# Interrogate Database Storage/Parsing
#
########################################################################

def parseInterrogateIntVec(tokzr):
    length = tokzr.readint()
    result = []
    for i in range(length):
        result.append(tokzr.readint())
    return result

class InterrogateFunction:
    def __init__(self, tokzr, db):
        self.db = db
        self.index = tokzr.readint()
        self.componentname = tokzr.readstring()
        self.flags = tokzr.readint()
        self.classindex = tokzr.readint()
        self.scopedname = tokzr.readstring()
        self.cwrappers = parseInterrogateIntVec(tokzr)
        self.pythonwrappers = parseInterrogateIntVec(tokzr)
        self.comment = tokzr.readstring()
        self.prototype = tokzr.readstring()

class InterrogateEnumValue:
    def __init__(self, tokzr):
        self.name = tokzr.readstring()
        self.scopedname = tokzr.readstring()
        self.value = tokzr.readint()

class InterrogateDerivation:
    def __init__(self, tokzr):
        self.flags = tokzr.readint()
        self.base = tokzr.readint()
        self.upcast = tokzr.readint()
        self.downcast = tokzr.readint()

class InterrogateType:
    def __init__(self, tokzr, db):
        self.db = db
        self.index = tokzr.readint()
        self.componentname = tokzr.readstring()
        self.flags = tokzr.readint()
        self.scopedname = tokzr.readstring()
        self.truename = tokzr.readstring()
        self.outerclass = tokzr.readint()
        self.atomictype = tokzr.readint()
        self.wrappedtype = tokzr.readint()
        self.constructors = parseInterrogateIntVec(tokzr)
        self.destructor = tokzr.readint()
        self.elements = parseInterrogateIntVec(tokzr)
        self.methods = parseInterrogateIntVec(tokzr)
        self.casts = parseInterrogateIntVec(tokzr)
        self.derivations = []
        nderivations = tokzr.readint()
        for i in range(nderivations):
            self.derivations.append(InterrogateDerivation(tokzr))
        self.enumvalues = []
        nenumvalues = tokzr.readint()
        for i in range(nenumvalues):
            self.enumvalues.append(InterrogateEnumValue(tokzr))
        self.nested = parseInterrogateIntVec(tokzr)
        self.comment = tokzr.readstring()

class InterrogateParameter:
    def __init__(self, tokzr):
        self.name = tokzr.readstring()
        self.parameterflags = tokzr.readint()
        self.type = tokzr.readint()

class InterrogateWrapper:
    def __init__(self, tokzr, db):
        self.db = db
        self.index = tokzr.readint()
        self.componentname = tokzr.readstring()
        self.flags = tokzr.readint()
        self.function = tokzr.readint()
        self.returntype = tokzr.readint()
        self.returnvaluedestructor = tokzr.readint()
        self.uniquename = tokzr.readstring()
        self.parameters = []
        nparameters = tokzr.readint()
        for i in range(nparameters):
            self.parameters.append(InterrogateParameter(tokzr))

class InterrogateDatabase:
    def __init__(self, tokzr):
        self.fn = tokzr.fn
        self.magic = tokzr.readint()
        version1 = tokzr.readint()
        version2 = tokzr.readint()
        if (version1 != 2) or (version2 != 2):
            sys.exit("This program only understands interrogate file format 2.2")
        self.library = tokzr.readstring()
        self.libhash = tokzr.readstring()
        self.module = tokzr.readstring()
        self.functions = {}
        self.wrappers = {}
        self.types = {}
        self.namedtypes = {}
        count_functions = tokzr.readint()
        for i in range(count_functions):
            fn = InterrogateFunction(tokzr, self)
            self.functions[fn.index] = fn
        count_wrappers = tokzr.readint()
        for i in range(count_wrappers):
            wr = InterrogateWrapper(tokzr, self)
            self.wrappers[wr.index] = wr
        count_types = tokzr.readint()
        for i in range(count_types):
            tp = InterrogateType(tokzr, self)
            self.types[tp.index] = tp
            self.namedtypes[tp.scopedname] = tp

########################################################################
#
# Pattern Matching for Python Parse Trees
#
########################################################################

def printTree(tree, indent):
    spacing = "                                                        "[:indent]
    if isinstance(tree, tuple) and isinstance(tree[0], int):
        if tree[0] in symbol.sym_name:
            for i in range(len(tree)):
                if (i==0):
                    print(spacing + "(symbol." + symbol.sym_name[tree[0]] + ",")
                else:
                    printTree(tree[i], indent+1)
            print(spacing + "),")
        elif tree[0] in token.tok_name:
            print(spacing + "(token." + token.tok_name[tree[0]] + ", '" + tree[1] + "'),")
        else:
            print(spacing + str(tree))
    else:
        print(spacing + str(tree))


COMPOUND_STMT_PATTERN = (
    symbol.stmt,
    (symbol.compound_stmt, ['compound'])
    )


DOCSTRING_STMT_PATTERN = (
    symbol.stmt,
    (symbol.simple_stmt,
     (symbol.small_stmt,
      (symbol.expr_stmt,
       (symbol.testlist,
        (symbol.test,
         (symbol.or_test,
           (symbol.and_test,
            (symbol.not_test,
             (symbol.comparison,
              (symbol.expr,
               (symbol.xor_expr,
                (symbol.and_expr,
                 (symbol.shift_expr,
                  (symbol.arith_expr,
                   (symbol.term,
                    (symbol.factor,
                     (symbol.power,
                      (symbol.atom,
                       (token.STRING, ['docstring'])
                       ))))))))))))))))),
     (token.NEWLINE, '')
     ))

DERIVATION_PATTERN = (
    symbol.test,
    (symbol.or_test,
     (symbol.and_test,
      (symbol.not_test,
       (symbol.comparison,
        (symbol.expr,
         (symbol.xor_expr,
          (symbol.and_expr,
           (symbol.shift_expr,
            (symbol.arith_expr,
             (symbol.term,
              (symbol.factor,
               (symbol.power,
                (symbol.atom,
                 (token.NAME, ['classname'])
   ))))))))))))))

ASSIGNMENT_STMT_PATTERN = (
    symbol.stmt,
    (symbol.simple_stmt,
     (symbol.small_stmt,
      (symbol.expr_stmt,
       (symbol.testlist,
        (symbol.test,
         (symbol.or_test,
           (symbol.and_test,
            (symbol.not_test,
             (symbol.comparison,
              (symbol.expr,
               (symbol.xor_expr,
                (symbol.and_expr,
                 (symbol.shift_expr,
                  (symbol.arith_expr,
                   (symbol.term,
                    (symbol.factor,
                     (symbol.power,
                      (symbol.atom,
                       (token.NAME, ['varname']),
       ))))))))))))))),
       (token.EQUAL, '='),
       (symbol.testlist, ['rhs']))),
     (token.NEWLINE, ''),
   ))

class ParseTreeInfo:
    docstring = ''
    name = ''

    def __init__(self, tree, name, file):
        """
        The code can be a string (in which case it is parsed), or it
        can be in parse tree form already.
        """
        self.name = name
        self.file = file
        self.class_info = {}
        self.function_info = {}
        self.assign_info = {}
        self.derivs = {}
        if isinstance(tree, str):
            try:
                tree = parser.suite(tree+"\n").totuple()
                if (tree):
                    found, vars = self.match(DOCSTRING_STMT_PATTERN, tree[1])
                    if found:
                        self.docstring = vars["docstring"]
            except:
                print("CAUTION --- Parse failed: " + name)
        if isinstance(tree, tuple):
            self.extract_info(tree)

    def match(self, pattern, data, vars=None):
        """
        pattern
            Pattern to match against, possibly containing variables.
        data
            Data to be checked and against which variables are extracted.
        vars
            Dictionary of variables which have already been found.  If not
            provided, an empty dictionary is created.

        The `pattern' value may contain variables of the form ['varname']
        which are allowed to parseTreeMatch anything.  The value that is
        parseTreeMatched is returned as part of a dictionary which maps
        'varname' to the parseTreeMatched value.  'varname' is not required
        to be a string object, but using strings makes patterns and the code
        which uses them more readable.  This function returns two values: a
        boolean indicating whether a parseTreeMatch was found and a
        dictionary mapping variable names to their associated values.
        """
        if vars is None:
            vars = {}
        if type(pattern) is list:       # 'variables' are ['varname']
            vars[pattern[0]] = data
            return 1, vars
        if type(pattern) is not tuple:
            return (pattern == data), vars
        if len(data) != len(pattern):
            return 0, vars
        for pattern, data in map(None, pattern, data):
            same, vars = self.match(pattern, data, vars)
            if not same:
                break
        return same, vars

    def extract_info(self, tree):
        # extract docstring
        found = 0
        if len(tree) == 2:
            found, vars = self.match(DOCSTRING_STMT_PATTERN[1], tree[1])
        elif len(tree) >= 4:
            found, vars = self.match(DOCSTRING_STMT_PATTERN, tree[3])
        if found:
            self.docstring = eval(vars['docstring'])
        # discover inner definitions
        for node in tree[1:]:
            found, vars = self.match(ASSIGNMENT_STMT_PATTERN, node)
            if found:
                self.assign_info[vars['varname']] = 1
            found, vars = self.match(COMPOUND_STMT_PATTERN, node)
            if found:
                cstmt = vars['compound']
                if cstmt[0] == symbol.funcdef:
                    name = cstmt[2][1]
                    # Workaround for a weird issue with static and classmethods
                    if name == "def":
                        name = cstmt[3][1]
                        self.function_info[name] = ParseTreeInfo(cstmt and cstmt[-1] or None, name, self.file)
                        self.function_info[name].prototype = self.extract_tokens("", cstmt[4])
                    else:
                        self.function_info[name] = ParseTreeInfo(cstmt and cstmt[-1] or None, name, self.file)
                        self.function_info[name].prototype = self.extract_tokens("", cstmt[3])
                elif cstmt[0] == symbol.classdef:
                    name = cstmt[2][1]
                    self.class_info[name] = ParseTreeInfo(cstmt and cstmt[-1] or None, name, self.file)
                    self.extract_derivs(self.class_info[name], cstmt)

    def extract_derivs(self, classinfo, tree):
        if (len(tree)==8):
            derivs = tree[4]
            for deriv in derivs[1:]:
                found, vars = self.match(DERIVATION_PATTERN, deriv)
                if (found):
                    classinfo.derivs[vars["classname"]] = 1

    def extract_tokens(self, str, tree):
        if (isinstance(tree, tuple)):
            if tree[0] in token.tok_name:
                str = str + tree[1]
                if (tree[1]==","): str=str+" "
            elif tree[0] in symbol.sym_name:
                for sub in tree[1:]:
                    str = self.extract_tokens(str, sub)
        return str

########################################################################
#
# The code database contains:
#
#  - a list of InterrogateDatabase objects representing C++ modules.
#  - a list of ParseTreeInfo objects representing python modules.
#
# Collectively, these make up all the data about all the code.
#
########################################################################

class CodeDatabase:
    def __init__(self, cxxlist, pylist):
        self.types = {}
        self.funcs = {}
        self.goodtypes = {}
        self.funcExports = {}
        self.typeExports = {}
        self.varExports = {}
        self.globalfn = []
        self.formattedprotos = {}
        print("Reading C++ source files")
        for cxx in cxxlist:
            tokzr = InterrogateTokenizer(cxx)
            idb = InterrogateDatabase(tokzr)
            for type in idb.types.values():
                if (type.flags & 8192) or type.scopedname not in self.types:
                    self.types[type.scopedname] = type
                if (type.flags & 8192) and (type.atomictype == 0) and (type.scopedname.count(" ")==0) and (type.scopedname.count(":")==0):
                    self.goodtypes[type.scopedname] = type
                    self.typeExports.setdefault("pandac.PandaModules", []).append(type.scopedname)
            for func in idb.functions.values():
                type = idb.types.get(func.classindex)
                func.pyname = convertToPythonFn(func.componentname)
                if (type == None):
                    self.funcs["GLOBAL."+func.pyname] = func
                    self.globalfn.append("GLOBAL."+func.pyname)
                    self.funcExports.setdefault("pandac.PandaModules", []).append(func.pyname)
                else:
                    self.funcs[type.scopedname+"."+func.pyname] = func
        print("Reading Python sources files")
        for py in pylist:
            pyinf = ParseTreeInfo(readFile(py), py, py)
            mod = pathToModule(py)
            for type in pyinf.class_info.keys():
                typinf = pyinf.class_info[type]
                self.types[type] = typinf
                self.goodtypes[type] = typinf
                self.typeExports.setdefault(mod, []).append(type)
                for func in typinf.function_info.keys():
                    self.funcs[type+"."+func] = typinf.function_info[func]
            for func in pyinf.function_info.keys():
                self.funcs["GLOBAL."+func] = pyinf.function_info[func]
                self.globalfn.append("GLOBAL."+func)
                self.funcExports.setdefault(mod, []).append(func)
            for var in pyinf.assign_info.keys():
                self.varExports.setdefault(mod, []).append(var)

    def getClassList(self):
        return list(self.goodtypes.keys())

    def getGlobalFunctionList(self):
        return self.globalfn

    def getClassComment(self, cn):
        type = self.types.get(cn)
        if (isinstance(type, InterrogateType)):
            return textToHTML(type.comment,"/")
        elif (isinstance(type, ParseTreeInfo)):
            return textToHTML(type.docstring,"#")
        else:
            return ""

    def getClassParents(self, cn):
        type = self.types.get(cn)
        if (isinstance(type, InterrogateType)):
            parents = []
            for deriv in type.derivations:
                basetype = type.db.types[deriv.base]
                parents.append(basetype.scopedname)
            return parents
        elif (isinstance(type, ParseTreeInfo)):
            return list(type.derivs.keys())
        else:
            return []

    def getClassConstants(self, cn):
        type = self.types.get(cn)
        if (isinstance(type, InterrogateType)):
            result = []
            for subtype in type.nested:
                enumtype = type.db.types[subtype]
                if (len(enumtype.enumvalues)):
                    for enumvalue in enumtype.enumvalues:
                        name = convertToPythonFn(enumvalue.name)
                        result.append((name, "("+enumtype.componentname+")"))
                    result.append(("",""))
            return result
        else:
            return []

    def buildInheritance(self, inheritance, cn):
        if (inheritance.count(cn) == 0):
            inheritance.append(cn)
            for parent in self.getClassParents(cn):
                self.buildInheritance(inheritance, parent)

    def getInheritance(self, cn):
        inheritance = []
        self.buildInheritance(inheritance, cn)
        return inheritance

    def getClassImport(self, cn):
        type = self.types.get(cn)
        if (isinstance(type, InterrogateType)):
            return "pandac.PandaModules"
        else:
            return pathToModule(type.file)

    def getClassConstructors(self, cn):
        # Only detects C++ constructors, not Python constructors, since
        # those are treated as ordinary methods.
        type = self.types.get(cn)
        result = []
        if (isinstance(type, InterrogateType)):
            for constructor in type.constructors:
                func = type.db.functions[constructor]
                if (func.classindex == type.index):
                    result.append(type.scopedname+"."+func.pyname)
        return result

    def getClassMethods(self, cn):
        type = self.types.get(cn)
        result = []
        if (isinstance(type, InterrogateType)):
            for method in type.methods:
                func = type.db.functions[method]
                if (func.classindex == type.index):
                    result.append(type.scopedname+"."+func.pyname)
        elif (isinstance(type, ParseTreeInfo)):
            for method in type.function_info.keys():
                result.append(type.name + "." + method)
        return result

    def getFunctionName(self, fn):
        func = self.funcs.get(fn)
        if (isinstance(func, InterrogateFunction)):
            return func.pyname
        elif (isinstance(func, ParseTreeInfo)):
            return func.name
        else:
            return fn

    def getFunctionImport(self, fn):
        func = self.funcs.get(fn)
        if (isinstance(func, InterrogateFunction)):
            return "pandac.PandaModules"
        else:
            return pathToModule(func.file)

    def getFunctionPrototype(self, fn, urlprefix, urlsuffix):
        func = self.funcs.get(fn)
        if (isinstance(func, InterrogateFunction)):
            if fn in self.formattedprotos:
                proto = self.formattedprotos[fn]
            else:
                proto = func.prototype
                proto = proto.replace(" inline "," ")
                if (proto.startswith("inline ")): proto = proto[7:]
                proto = proto.replace("basic_string< char >", "string")
                proto = textToHTML(proto,"")
                if "." in fn:
                    for c in self.goodtypes.keys():
                        if c != fn.split(".")[0] and (c in proto):
                            proto = re.sub("\\b%s\\b" % c, linkTo(urlprefix+c+urlsuffix, c), proto)
                self.formattedprotos[fn] = proto
            return proto
        elif (isinstance(func, ParseTreeInfo)):
            return textToHTML("def "+func.name+func.prototype,"")
        return fn

    def getFunctionComment(self, fn):
        func = self.funcs.get(fn)
        if (isinstance(func, InterrogateFunction)):
            return textToHTML(removeFileLicense(func.comment), "/", JUNKHEADER)
        elif (isinstance(func, ParseTreeInfo)):
            return textToHTML(func.docstring, "#")
        return fn

    def isFunctionPython(self, fn):
        func = self.funcs.get(fn)
        if (isinstance(func, InterrogateFunction)):
            return False
        elif (isinstance(func, ParseTreeInfo)):
            return True
        return False

    def getFuncExports(self, mod):
        return self.funcExports.get(mod, [])

    def getTypeExports(self, mod):
        return self.typeExports.get(mod, [])

    def getVarExports(self, mod):
        return self.varExports.get(mod, [])

########################################################################
#
# The "Class Rename Dictionary" - Yech.
#
########################################################################

CLASS_RENAME_DICT = {
    # No longer used, now empty.
}


########################################################################
#
# HTML generation
#
########################################################################

def makeCodeDatabase(indirlist, directdirlist):
    if isinstance(directdirlist, str):
        directdirlist = [directdirlist]
    ignore = {}
    ignore["__init__.py"] = 1
    for directdir in directdirlist:
        ignore[directdir + "/src/directscripts"] = 1
        ignore[directdir + "/src/extensions"] = 1
        ignore[directdir + "/src/extensions_native"] = 1
        ignore[directdir + "/src/ffi"] = 1
        ignore[directdir + "/built"] = 1
    cxxfiles = []
    pyfiles = []
    findFiles(indirlist,     ".in", ignore, cxxfiles)
    findFiles(directdirlist, ".py", ignore, pyfiles)
    return CodeDatabase(cxxfiles, pyfiles)

def generateFunctionDocs(code, method, urlprefix, urlsuffix):
    name = code.getFunctionName(method)
    proto = code.getFunctionPrototype(method, urlprefix, urlsuffix)
    comment = code.getFunctionComment(method)
    if (comment == ""): comment = "Undocumented function.<br>\n"
    chunk = '<table bgcolor="e8e8e8" border=0 cellspacing=0 cellpadding=5 width="100%"><tr><td>' + "\n"
    chunk = chunk + '<a name="' + name + '"><b>' + name + "</b></a><br>\n"
    chunk = chunk + proto + "<br>\n"
    chunk = chunk + comment
    chunk = chunk + "</td></tr></table><br>\n"
    return chunk

def generateLinkTable(link, text, cols, urlprefix, urlsuffix):
    column = (len(link)+cols-1)/cols
    percent = 100 / cols
    result = '<table width="100%">\n'
    for i in range(column):
        line = ""
        for j in range(cols):
            slot = i + column*j
            linkval = ""
            textval = ""
            if (slot < len(link)): linkval = link[slot]
            if (slot < len(text)): textval = text[slot]
            if (i==0):
                line = line + '<td width="' + str(percent) + '%">' + linkTo(urlprefix+linkval+urlsuffix, textval) + "</td>"
            else:
                line = line + '<td>' + linkTo(urlprefix+linkval+urlsuffix, textval) + "</td>"
        result = result + "<tr>" + line + "</tr>\n"
    result = result + "</table>\n"
    return result

def generate(pversion, indirlist, directdirlist, docdir, header, footer, urlprefix, urlsuffix):
    code = makeCodeDatabase(indirlist, directdirlist)
    classes = code.getClassList()[:]
    classes.sort(None, str.lower)
    xclasses = classes[:]
    print("Generating HTML pages")
    for type in classes:
        body = "<h1>" + type + "</h1>\n"
        comment = code.getClassComment(type)
        body = body + "<ul>\nfrom " + code.getClassImport(type) + " import " + type + "</ul>\n\n"
        body = body + "<ul>\n" + comment + "</ul>\n\n"
        inheritance = code.getInheritance(type)
        body = body + "<h2>Inheritance:</h2>\n<ul>\n"
        for inh in inheritance:
            line = "  " + linkTo(urlprefix+inh+urlsuffix, inh) + ": "
            for parent in code.getClassParents(inh):
                line = line + linkTo(urlprefix+parent+urlsuffix, parent) + " "
            body = body + line + "<br>\n"
        body = body + "</ul>\n"
        for sclass in inheritance:
            methods = code.getClassMethods(sclass)[:]
            methods.sort(None, str.lower)
            constructors = code.getClassConstructors(sclass)
            if (len(methods) > 0 or len(constructors) > 0):
                body = body + "<h2>Methods of "+sclass+":</h2>\n<ul>\n"
                if len(constructors) > 0:
                    fn = code.getFunctionName(constructors[0])
                    body = body + '<a href="#' + fn + '">' + fn + " (Constructor)</a><br>\n"
                for method in methods:
                    fn = code.getFunctionName(method)
                    body = body + '<a href="#' + fn + '">' + fn + "</a><br>\n"
                body = body + "</ul>\n"
        for sclass in inheritance:
            enums = code.getClassConstants(sclass)[:]
            if (len(enums) > 0):
                body = body + "<h2>Constants in "+sclass+":</h2>\n<ul><table>\n"
                for (value, comment) in enums:
                    body = body + "<tr><td>" + value + "</td><td>" + comment + "</td></tr>\n"
                body = body + "</table></ul>"
        for sclass in inheritance:
            constructors = code.getClassConstructors(sclass)
            for constructor in constructors:
                body = body + generateFunctionDocs(code, constructor, urlprefix, urlsuffix)
            methods = code.getClassMethods(sclass)[:]
            methods.sort(None, str.lower)
            for method in methods:
                body = body + generateFunctionDocs(code, method, urlprefix, urlsuffix)
        body = header + body + footer
        writeFile(docdir + "/" + type + ".html", body)
        if type in CLASS_RENAME_DICT:
            modtype = CLASS_RENAME_DICT[type]
            writeFile(docdir + "/" + modtype + ".html", body)
            xclasses.append(modtype)
    xclasses.sort(None, str.lower)

    index = "<h1>List of Classes - Panda " + pversion + "</h1>\n"
    index = index + generateLinkTable(xclasses, xclasses, 3, urlprefix, urlsuffix)
    fnlist = code.getGlobalFunctionList()[:]
    fnlist.sort(None, str.lower)
    fnnames = []
    for i in range(len(fnlist)):
        fnnames.append(code.getFunctionName(fnlist[i]))
    index = header + index + footer
    writeFile(docdir + "/classes.html", index)

    index = "<h1>List of Global Functions - Panda " + pversion + "</h1>\n"
    index = index + generateLinkTable(fnnames, fnnames, 3,"#","")
    for func in fnlist:
        index = index + generateFunctionDocs(code, func, urlprefix, urlsuffix)
    index = header + index + footer
    writeFile(docdir + "/functions.html", index)

    table = {}
    for type in classes:
        for method in code.getClassMethods(type)[:]:
            name = code.getFunctionName(method)
            prefix = name[0].upper()
            if prefix not in table:
                table[prefix] = {}
            if name not in table[prefix]:
                table[prefix][name] = []
            table[prefix][name].append(type)

    index = "<h1>List of Methods - Panda " + pversion + "</h1>\n"

    prefixes = list(table.keys())
    prefixes.sort(None, str.lower)
    for prefix in prefixes:
        index = index + linkTo("#"+prefix, prefix) + " "
    index = index + "<br><br>"
    for prefix in prefixes:
        index = index + '<a name="' + prefix + '">' + "\n"
        names = list(table[prefix].keys())
        names.sort(None, str.lower)
        for name in names:
            line = '<b>' + name + ":</b><ul>\n"
            ctypes = table[prefix][name]
            ctypes.sort(None, str.lower)
            for type in ctypes:
                line = line + "<li>" + linkTo(urlprefix+type+urlsuffix+"#"+name, type) + "\n"
            line = line + "</ul>\n"
            index = index + line + "\n"
    index = header + index + footer
    writeFile(docdir + "/methods.html", index)

    index = "<h1>Panda " + pversion + "</h1>\n"
    index = index + "<ul>\n"
    index = index + "<li>" + linkTo(urlprefix+"classes"+urlsuffix, "List of all Classes") + "\n"
    index = index + "</ul>\n"
    index = index + "<ul>\n"
    index = index + "<li>" + linkTo(urlprefix+"functions"+urlsuffix, "List of all Global Functions") + "\n"
    index = index + "</ul>\n"
    index = index + "<ul>\n"
    index = index + "<li>" + linkTo(urlprefix+"methods"+urlsuffix, "List of all Methods (very long)") + "\n"
    index = index + "</ul>\n"
    writeFile(docdir + "/index.html", index)


########################################################################
#
# IMPORT repair
#
########################################################################

def expandImports(indirlist, directdirlist, fixdirlist):
    code = makeCodeDatabase(indirlist, directdirlist)
    fixfiles = []
    findFiles(fixdirlist, ".py", {}, fixfiles)
    for fixfile in fixfiles:
        if (os.path.isfile(fixfile+".orig")):
            text = readFile(fixfile+".orig")
        else:
            text = readFile(fixfile)
            writeFile(fixfile+".orig", text)
        text = text.replace("\r","")
        lines = text.split("\n")
        used = {}
        for id in IDENTIFIER.findall(text):
            used[id] = 1
        result = []
        for line in lines:
            mat = IMPORTSTAR.match(line)
            if (mat):
                module = mat.group(1)
                if (fixfile.count("/")!=0) and (module.count(".")==0):
                    modfile = os.path.dirname(fixfile)+"/"+module+".py"
                    if (os.path.isfile(modfile)):
                        module = pathToModule(modfile)
                typeExports = code.getTypeExports(module)
                funcExports = code.getFuncExports(module)
                varExports = code.getVarExports(module)
                if (len(typeExports)+len(funcExports)+len(varExports)==0):
                    result.append(line)
                    print(fixfile + " : " + module + " : no exports")
                else:
                    print(fixfile + " : " + module + " : repairing")
                    for x in funcExports:
                        fn = code.getFunctionName(x)
                        if fn in used:
                            result.append("from "+module+" import "+fn)
                    for x in typeExports:
                        if x in used:
                            result.append("from "+module+" import "+x)
                    for x in varExports:
                        if x in used:
                            result.append("from "+module+" import "+x)
            else:
                result.append(line)
        writeFileLines(fixfile, result)
