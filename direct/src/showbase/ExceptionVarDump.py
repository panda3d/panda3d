from pandac.PandaModules import ConfigConfigureGetConfigConfigShowbase as config
from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import fastRepr
from exceptions import Exception
import sys
import types
import traceback

notify = directNotify.newCategory("ExceptionVarDump")

reentry = 0

def _varDump__init__(self, *args, **kArgs):
    global reentry
    if reentry > 0:
        return
    reentry += 1
    # frame zero is this frame
    f = 1
    self._savedExcString = None
    self._savedStackFrames = []
    while True:
        try:
            frame = sys._getframe(f)
        except ValueError, e:
            break
        else:
            f += 1
            self._savedStackFrames.append(frame)
    self._moved__init__(*args, **kArgs)
    reentry -= 1

sReentry = 0

def _varDump__print(exc):
    global sReentry
    global notify
    if sReentry > 0:
        return
    sReentry += 1
    if not exc._savedExcString:
        s = ''
        foundRun = False
        for frame in reversed(exc._savedStackFrames):
            filename = frame.f_code.co_filename
            codename = frame.f_code.co_name
            if not foundRun and codename != 'run':
                # don't print stack frames before run(),
                # they contain builtins and are huge
                continue
            foundRun = True
            s += '\nlocals for %s:%s\n' % (filename, codename)
            locals = frame.f_locals
            for var in locals:
                obj = locals[var]
                rep = fastRepr(obj)
                s += '::%s = %s\n' % (var, rep)
        exc._savedExcString = s
        exc._savedStackFrames = None
    notify.info(exc._savedExcString)
    sReentry -= 1

oldExcepthook = None
# store these values here so that Task.py can always reliably access them
# from its main exception handler
wantVariableDump = False
dumpOnExceptionInit = False

class _AttrNotFound:
    pass

def _excepthookDumpVars(eType, eValue, tb):
    excStrs = traceback.format_exception(eType, eValue, tb)
    s = 'printing traceback in case variable repr crashes the process...\n'
    for excStr in excStrs:
        s += excStr
    notify.info(s)
    s = 'DUMPING STACK FRAME VARIABLES'
    origTb = tb
    #import pdb;pdb.set_trace()
    #foundRun = False
    foundRun = True
    while tb is not None:
        frame = tb.tb_frame
        code = frame.f_code
        # this is a list of every string identifier used in this stack frame's code
        codeNames = set(code.co_names)
        # skip everything before the 'run' method, those frames have lots of
        # not-useful information
        if not foundRun:
            if code.co_name == 'run':
                foundRun = True
            else:
                tb = tb.tb_next
                continue
        s += '\n  File "%s", line %s, in %s' % (
            code.co_filename, frame.f_lineno, code.co_name)
        stateStack = Stack()
        # prime the stack with the variables we should visit from the frame's data structures
        # grab all of the local, builtin and global variables that appear in the code's name list
        name2obj = {}
        for name, obj in frame.f_builtins.items():
            if name in codeNames:
                name2obj[name] = obj
        for name, obj in frame.f_globals.items():
            if name in codeNames:
                name2obj[name] = obj
        for name, obj in frame.f_locals.items():
            if name in codeNames:
                name2obj[name] = obj
        # show them in alphabetical order
        names = name2obj.keys()
        names.sort()
        # push them in reverse order so they'll be popped in the correct order
        names.reverse()

        traversedIds = set()

        for name in names:
            stateStack.push([name, name2obj[name], traversedIds])

        while len(stateStack) > 0:
            name, obj, traversedIds = stateStack.pop()
            #notify.info('%s, %s, %s' % (name, fastRepr(obj), traversedIds))
            r = fastRepr(obj, maxLen=10)
            if type(r) is types.StringType:
                r = r.replace('\n', '\\n')
            s += '\n    %s = %s' % (name, r)
            # if we've already traversed through this object, don't traverse through it again
            if id(obj) not in traversedIds:
                attrName2obj = {}
                for attrName in codeNames:
                    attr = getattr(obj, attrName, _AttrNotFound)
                    if (attr is not _AttrNotFound):
                        # prevent infinite recursion on method wrappers (__init__.__init__.__init__...)
                        try:
                            className = attr.__class__.__name__
                        except:
                            pass
                        else:
                            if className == 'method-wrapper':
                                continue
                        attrName2obj[attrName] = attr
                if len(attrName2obj):
                    # show them in alphabetical order
                    attrNames = attrName2obj.keys()
                    attrNames.sort()
                    # push them in reverse order so they'll be popped in the correct order
                    attrNames.reverse()
                    ids = set(traversedIds)
                    ids.add(id(obj))
                    for attrName in attrNames:
                        obj = attrName2obj[attrName]
                        stateStack.push(['%s.%s' % (name, attrName), obj, ids])
                
        tb = tb.tb_next

    if foundRun:
        s += '\n'
        notify.info(s)
    oldExcepthook(eType, eValue, origTb)

def install():
    global oldExcepthook
    global wantVariableDump
    global dumpOnExceptionInit

    wantVariableDump = True
    dumpOnExceptionInit = config.GetBool('variable-dump-on-exception-init', 0)
    if dumpOnExceptionInit:
        # this mode doesn't completely work because exception objects
        # thrown by the interpreter don't get created until the
        # stack has been unwound and an except block has been reached
        if not hasattr(Exception, '_moved__init__'):
            Exception._moved__init__ = Exception.__init__
            Exception.__init__ = _varDump__init__
    else:
        if sys.excepthook is not _excepthookDumpVars:
            oldExcepthook = sys.excepthook
            sys.excepthook = _excepthookDumpVars
