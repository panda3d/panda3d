from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import fastRepr
from exceptions import Exception
import sys
import traceback

reentry = 0

def _varDump__init__(self, *args, **kArgs):
    global reentry
    if reentry > 0:
        return
    reentry += 1
    f = 1
    self._savedExcString = None
    self._savedStackFrames = []
    while True:
        try:
            frame = sys._getframe(f)
            f += 1
            self._savedStackFrames.append(frame)
        except:
            break
    self._moved__init__(*args, **kArgs)
    reentry -= 1

sReentry = 0

def _varDump__str__(self, *args, **kArgs):
    global sReentry
    if sReentry > 0:
        return
    sReentry += 1
    if not self._savedExcString:
        s = ''
        foundRun = False
        for frame in reversed(self._savedStackFrames):
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
        self._savedExcString = s
        self._savedStackFrames = None
    notify = directNotify.newCategory("ExceptionVarDump")
    notify.info(self._savedExcString)
    str = self._moved__str__(*args, **kArgs)
    sReentry -= 1
    return str

def install():
    if not hasattr(Exception, '_moved__init__'):
        Exception._moved__init__ = Exception.__init__
        Exception.__init__ = _varDump__init__
        Exception._moved__str__ = Exception.__str__
        Exception.__str__ = _varDump__str__
