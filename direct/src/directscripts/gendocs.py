########################################################################
#
# Documentation generator for panda.  There are three
# major subsystems:
#
# * The module that reads interrogate databases.
#
# * The module that reads python source-files.
#
# * 
# 
########################################################################

import os,sys,parser,symbol,token,types,re

########################################################################
#
# assorted utility functions
#
########################################################################

SECHEADER = re.compile("^[A-Z][a-z]+\s*:")
JUNKHEADER = re.compile("^((Function)|(Access))\s*:")

def readFile(fn):
    try:
        srchandle = open(fn, "r")
        data = srchandle.read()
        srchandle.close()
        return data
    except:
        sys.exit("Cannot read "+fn)

def writeFile(wfile,data):
    try:
        dsthandle = open(wfile, "wb")
        dsthandle.write(data)
        dsthandle.close()
    except:
        sys.exit("Cannot write "+wfile)

def findFiles(dir, ext, ign, list):
    for file in os.listdir(dir):
        full = dir + "/" + file
        if (ign.has_key(full)==0) and (ign.has_key(file)==0):
            if (os.path.isfile(full)):
                if (file.endswith(ext)):
                    list.append(full)
            elif (os.path.isdir(full)):
                findFiles(full, ext, ign, list)

def textToHTML(comment,sep,delsection=None):
    sections = [""]
    included = {}
    for line in comment.split("\n"):
        line = line.lstrip(" ").lstrip(sep).lstrip(" ").rstrip("\r").rstrip(" ")
        if (line == ""):
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
            if (included.has_key(sec)==0):
                included[sec] = 1
                total = total + sec + "<br>\n"
    return total

def linkToPage(page, text):
    return '<a href="/apiref.php?page=' + page + '">' + text + '</a>'

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

def getPandaVersion():
    try:
        f = file("dtool/PandaVersion.pp","r")
        pattern = re.compile('^[ \t]*[#][ \t]*define[ \t]+PANDA_VERSION[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)')
        for line in f:
            match = pattern.match(line,0)
            if (match):
                version = match.group(1)+"."+match.group(2)+"."+match.group(3)
                break
        f.close()
    except: sys.exit("Cannot read version number from dtool/PandaVersion.pp")
    return version

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

    def __init__(self,fn):
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
            print "File position "+str(self.pos)
            print "Text: "+self.data[self.pos:self.pos+50]
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

def printTree(tree,indent):
    spacing = "                                                        "[:indent]
    if isinstance(tree,types.TupleType) and isinstance(tree[0], types.IntType):
        if symbol.sym_name.has_key(tree[0]):
            for i in range(len(tree)):
                if (i==0):
                    print spacing + "(symbol." + symbol.sym_name[tree[0]] + ","
                else:
                    printTree(tree[i], indent+1)
            print spacing + "),"
        elif token.tok_name.has_key(tree[0]):
            print spacing + "(token." + token.tok_name[tree[0]] + ", '" + tree[1] + "'),"
        else:
            print spacing + str(tree)
    else:
        print spacing + str(tree)


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
                     )))))))))))))))),
     (token.NEWLINE, '')
     ))

DERIVATION_PATTERN = (
    symbol.test,
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
   )))))))))))))



class ParseTreeInfo:
    docstring = ''
    name = ''

    def __init__(self, tree, name):
        """
        The code can be a string (in which case it is parsed), or it
        can be in parse tree form already.
        """
        self.name = name
        self.class_info = {}
        self.function_info = {}
        self.derivs = {}
        if isinstance(tree, types.StringType):
            try:
                tree = parser.suite(tree+"\n").totuple()
                if (tree):
                    found, vars = self.match(DOCSTRING_STMT_PATTERN, tree[1])
                    if found:
                        self.docstring = vars["docstring"]
            except:
                print "CAUTION --- Parse failed: "+name
        if isinstance(tree, types.TupleType):
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
        if type(pattern) is types.ListType:       # 'variables' are ['varname']
            vars[pattern[0]] = data
            return 1, vars
        if type(pattern) is not types.TupleType:
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
            found, vars = self.match(COMPOUND_STMT_PATTERN, node)
            if found:
                cstmt = vars['compound']
                if cstmt[0] == symbol.funcdef:
                    name = cstmt[2][1]
                    self.function_info[name] = ParseTreeInfo(cstmt and cstmt[-1] or None, name)
                    self.function_info[name].prototype = self.extract_tokens("", cstmt[3])
                elif cstmt[0] == symbol.classdef:
                    name = cstmt[2][1]
                    self.class_info[name] = ParseTreeInfo(cstmt and cstmt[-1] or None, name)
                    self.extract_derivs(self.class_info[name],cstmt)

    def extract_derivs(self, classinfo, tree):
        if (len(tree)==8):
            derivs = tree[4]
            for deriv in derivs[1:]:
                found, vars = self.match(DERIVATION_PATTERN, deriv)
                if (found):
                    classinfo.derivs[vars["classname"]] = 1

    def extract_tokens(self, str, tree):
        if (isinstance(tree, types.TupleType)):
            if (token.tok_name.has_key(tree[0])):
                str = str + tree[1]
                if (tree[1]==","): str=str+" "
            elif (symbol.sym_name.has_key(tree[0])):
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
        self.globalfn = []
        for cxx in cxxlist:
            print "Reading source file "+cxx
            tokzr = InterrogateTokenizer(cxx)
            idb = InterrogateDatabase(tokzr)
            for type in idb.types.values():
                if (type.flags & 8192) or (self.types.has_key(type.scopedname)==0):
                    self.types[type.scopedname] = type
                if (type.flags & 8192) and (type.atomictype == 0) and (type.scopedname.count(" ")==0) and (type.scopedname.count(":")==0):
                    self.goodtypes[type.scopedname] = type
            for func in idb.functions.values():
                type = idb.types.get(func.classindex)
                func.pyname = convertToPythonFn(func.componentname)
                if (type == None):
                    self.funcs["GLOBAL."+func.pyname] = func
                    self.globalfn.append("GLOBAL."+func.pyname)
                else:
                    self.funcs[type.scopedname+"."+func.pyname] = func
        for py in pylist:
            print "Reading source file "+py
            pyinf = ParseTreeInfo(readFile(py), py)
            for type in pyinf.class_info.keys():
                typinf = pyinf.class_info[type]
                self.types[type] = typinf
                self.goodtypes[type] = typinf
                for func in typinf.function_info.keys():
                    self.funcs[type+"."+func] = typinf.function_info[func]
            for func in pyinf.function_info.keys():
                self.funcs["GLOBAL."+func] = pyinf.function_info[func]
                self.globalfn.append("GLOBAL."+func)

    def getClassList(self):
        return self.goodtypes.keys()

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
            return type.derivs.keys()
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

    def getFunctionPrototype(self, fn):
        func = self.funcs.get(fn)
        if (isinstance(func, InterrogateFunction)):
            proto = func.prototype
            proto = proto.replace(" inline "," ")
            if (proto.startswith("inline ")): proto = proto[7:]
            proto = proto.replace("basic_string< char >", "string")
            return textToHTML(proto,"")
        elif (isinstance(func, ParseTreeInfo)):
            return textToHTML("def "+func.name+func.prototype,"")
        return fn

    def getFunctionComment(self, fn):
        func = self.funcs.get(fn)
        if (isinstance(func, InterrogateFunction)):
            return textToHTML(func.comment, "/", JUNKHEADER)
        elif (isinstance(func, ParseTreeInfo)):
            return textToHTML(func.docstring, "#")
        return fn

########################################################################
#
# The "Class Rename Dictionary" - Yech.
#
########################################################################

CLASS_RENAME_DICT = {
 "Loader"                    : "PandaLoader" ,
 "String"                    : "CString" ,
 "LMatrix4f"                 : "Mat4" ,
 "LMatrix3f"                 : "Mat3" ,
 "LVecBase4f"                : "VBase4" ,
 "LVector4f"                 : "Vec4" ,
 "LPoint4f"                  : "Point4" ,
 "LVecBase3f"                : "VBase3" ,
 "LVector3f"                 : "Vec3" ,
 "LPoint3f"                  : "Point3" ,
 "LVecBase2f"                : "VBase2" ,
 "LVector2f"                 : "Vec2" ,
 "LPoint2f"                  : "Point2" ,
 "LQuaternionf"              : "Quat" ,
 "LMatrix4d"                 : "Mat4D" ,
 "LMatrix3d"                 : "Mat3D" ,
 "LVecBase4d"                : "VBase4D" ,
 "LVector4d"                 : "Vec4D" ,
 "LPoint4d"                  : "Point4D" ,
 "LVecBase3d"                : "VBase3D" ,
 "LVector3d"                 : "Vec3D" ,
 "LPoint3d"                  : "Point3D" ,
 "LVecBase2d"                : "VBase2D" ,
 "LVector2d"                 : "Vec2D" ,
 "LPoint2d"                  : "Point2D" ,
 "LQuaterniond"              : "QuatD" ,
 "Plane"                     : "PlaneBase" ,
 "Planef"                    : "Plane" ,
 "Planed"                    : "PlaneD" ,
 "Frustum"                   : "FrustumBase" ,
 "Frustumf"                  : "Frustum" ,
 "Frustumd"                  : "FrustumD" 
}


########################################################################
#
# Main Program
#
########################################################################

def getCodeDatabase():
    ignore = {}
    ignore["__init__.py"] = 1
    ignore["direct/src/directscripts"] = 1
    ignore["direct/src/extensions"] = 1
    ignore["direct/src/extensions_native"] = 1
    ignore["direct/src/ffi"] = 1
    cxxfiles = []
    pyfiles = []
    findFiles("built/pandac/input", ".in", ignore, cxxfiles)
    findFiles("direct",             ".py", ignore, pyfiles)
    return CodeDatabase(cxxfiles, pyfiles)


def generateFunctionDocs(code, method):
    name = code.getFunctionName(method)
    proto = code.getFunctionPrototype(method)
    comment = code.getFunctionComment(method)
    if (comment == ""): comment = "Undocumented function.<br>\n"
    chunk = '<table class="codecomment1" width="100%"><tr><td>' + "\n"
    chunk = chunk + '<a name="' + name + '"><b>' + name + "</b></a><br>\n"
    chunk = chunk + proto + "<br>\n"
    chunk = chunk + comment
    chunk = chunk + "</td></tr></table>\n"
    return chunk

def generateDocs(code):
    dir = "built/pandac/docs"
    classes = code.getClassList()[:]
    classes.sort()
    xclasses = classes[:]
    for type in classes:
        print "Generating page for class "+type
        body = "<h1>" + type + "</h1>\n"
        comment = code.getClassComment(type)
        body = body + "<ul>\n" + comment + "</ul>\n\n"
        inheritance = code.getInheritance(type)
        body = body + "<h2>Inheritance:</h2>\n<ul>\n"
        for inh in inheritance:
            line = "  " + linkToPage(inh,inh) + " : "
            for parent in code.getClassParents(inh):
                line = line + linkToPage(parent,parent) + " "
            body = body + line + "<br>\n"
        body = body + "</ul>\n"
        for sclass in inheritance:
            methods = code.getClassMethods(sclass)[:]
            methods.sort()
            if (len(methods) > 0):
                body = body + "<h2>Methods of "+sclass+":</h2>\n<ul>\n"
                for method in methods:
                    fn = code.getFunctionName(method)
                    body = body + '<a href="#' + fn + '">' + fn + "</a><br>\n"
                body = body + "</ul>\n"
        body = body + "</ul>\n"
        for sclass in inheritance:
            methods = code.getClassMethods(sclass)[:]
            methods.sort()
            for method in methods:
                body = body + generateFunctionDocs(code, method)
        writeFile(dir + "/" + type + ".html", body)
        if (CLASS_RENAME_DICT.has_key(type)):
            modtype = CLASS_RENAME_DICT[type]
            writeFile(dir + "/" + modtype + ".html", body)
            xclasses.append(modtype)
    xclasses.sort()
    pversion = getPandaVersion()
    index = "<h1>List of Classes - Panda " + pversion + "</h1>\n<ul>\n"
    for type in xclasses:
        index = index + linkToPage(type,type) + "<br>\n"
    index = index + "</ul>\n"
    index = index + "<h1>List of Global Functions - Panda " + pversion + "</h1>\n<ul>\n"
    fnlist = code.getGlobalFunctionList()[:]
    fnlist.sort()
    for func in fnlist:
        fn = code.getFunctionName(func)
        index = index + '<a href="#' + fn + '">' + fn + "</a><br>\n"
    index = index + "</ul>\n"
    for func in fnlist:
        index = index + generateFunctionDocs(code, func)
    writeFile(dir + "/index.html", index)


generateDocs(getCodeDatabase())

