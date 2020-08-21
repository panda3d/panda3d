"""Contains various utility functions."""

__all__ = ['findClass', 'rebindClass', 'copyFuncs', 'replaceMessengerFunc', 'replaceTaskMgrFunc', 'replaceStateFunc', 'replaceCRFunc', 'replaceAIRFunc', 'replaceIvalFunc']

import types
import os
import sys

def findClass(className):
    """
    Look in sys.modules dictionary for a module that defines a class
    with this className.
    """
    for moduleName, module in sys.modules.items():
        # Some modules are None for some reason
        if module:
            # print "Searching in ", moduleName
            classObj = module.__dict__.get(className)
            # If this modules defines some object called classname and the
            # object is a class or type definition and that class's module
            # is the same as the module we are looking in, then we found
            # the matching class and a good module namespace to redefine
            # our class in.
            if (classObj and
                ((type(classObj) == types.ClassType) or
                 (type(classObj) == types.TypeType)) and
                (classObj.__module__ == moduleName)):
                return [classObj, module.__dict__]
    return None

def rebindClass(filename):
    file = open(filename, 'r')
    lines = file.readlines()
    for line in lines:
        if (line[0:6] == 'class '):
            # Chop off the "class " syntax and strip extra whitespace
            classHeader = line[6:].strip()
            # Look for a open paren if it does inherit
            parenLoc = classHeader.find('(')
            if parenLoc > 0:
                className = classHeader[:parenLoc]
            else:
                # Look for a colon if it does not inherit
                colonLoc = classHeader.find(':')
                if colonLoc > 0:
                    className = classHeader[:colonLoc]
                else:
                    print('error: className not found')
                    # Remove that temp file
                    file.close()
                    os.remove(filename)
                    return
            print('Rebinding class name: ' + className)
            break

    # Try to find the original class with this class name
    res = findClass(className)

    if not res:
        print ('Warning: Finder could not find class')
        # Remove the temp file we made
        file.close()
        os.remove(filename)
        return

    # Store the original real class
    realClass, realNameSpace = res

    # Now execute that class def in this namespace
    exec(compile(open(filename).read(), filename, 'exec'), realNameSpace)

    # That execfile should have created a new class obj in that namespace
    tmpClass = realNameSpace[className]

    # Copy the functions that we just redefined into the real class
    copyFuncs(tmpClass, realClass)

    # Now make sure the original class is in that namespace,
    # not our temp one from the execfile. This will help us preserve
    # class variables and other state on the original class.
    realNameSpace[className] = realClass

    # Remove the temp file we made
    file.close()
    os.remove(filename)

    print ('    Finished rebind')


def copyFuncs(fromClass, toClass):
    replaceFuncList = []
    newFuncList = []

    # Copy the functions from fromClass into toClass dictionary
    for funcName, newFunc in fromClass.__dict__.items():
        # Filter out for functions
        if (type(newFunc) == types.FunctionType):
            # See if we already have a function with this name
            oldFunc = toClass.__dict__.get(funcName)
            if oldFunc:

                """
                # This code is nifty, but with nested functions, give an error:
                #   SystemError: cellobject.c:22: bad argument to internal function
                # Give the new function code the same filename as the old function
                # Perhaps there is a cleaner way to do this? This was my best idea.
                newCode = types.CodeType(newFunc.func_code.co_argcount,
                                         newFunc.func_code.co_nlocals,
                                         newFunc.func_code.co_stacksize,
                                         newFunc.func_code.co_flags,
                                         newFunc.func_code.co_code,
                                         newFunc.func_code.co_consts,
                                         newFunc.func_code.co_names,
                                         newFunc.func_code.co_varnames,
                                         # Use the oldFunc's filename here. Tricky!
                                         oldFunc.func_code.co_filename,
                                         newFunc.func_code.co_name,
                                         newFunc.func_code.co_firstlineno,
                                         newFunc.func_code.co_lnotab)
                newFunc = types.FunctionType(newCode,
                                             newFunc.func_globals,
                                             newFunc.func_name,
                                             newFunc.func_defaults,
                                             newFunc.func_closure)
                """
                replaceFuncList.append((oldFunc, funcName, newFunc))
            else:
                # TODO: give these new functions a proper code filename
                newFuncList.append((funcName, newFunc))

    # Look in the messenger, taskMgr, and other globals that store func
    # pointers to see if this old function pointer is stored there, and
    # update it to the new function pointer.
    replaceMessengerFunc(replaceFuncList)
    replaceTaskMgrFunc(replaceFuncList)
    replaceStateFunc(replaceFuncList)
    replaceCRFunc(replaceFuncList)
    replaceAIRFunc(replaceFuncList)
    replaceIvalFunc(replaceFuncList)

    # Now that we've the globals funcs, actually swap the pointers in
    # the new class to the new functions
    for oldFunc, funcName, newFunc in replaceFuncList:
        # print "replacing old func: ", oldFunc, funcName, newFunc
        setattr(toClass, funcName, newFunc)
    # Add the brand new functions too
    for funcName, newFunc in newFuncList:
        # print "adding new func: ", oldFunc, funcName, newFunc
        setattr(toClass, funcName, newFunc)

def replaceMessengerFunc(replaceFuncList):
    try:
        messenger
    except:
        return
    for oldFunc, funcName, newFunc in replaceFuncList:
        res = messenger.replaceMethod(oldFunc, newFunc)
        if res:
            print('replaced %s messenger function(s): %s' % (res, funcName))

def replaceTaskMgrFunc(replaceFuncList):
    try:
        taskMgr
    except:
        return
    for oldFunc, funcName, newFunc in replaceFuncList:
        if taskMgr.replaceMethod(oldFunc, newFunc):
            print('replaced taskMgr function: %s' % funcName)

def replaceStateFunc(replaceFuncList):
    if not sys.modules.get('base.direct.fsm.State'):
        return
    from direct.fsm.State import State
    for oldFunc, funcName, newFunc in replaceFuncList:
        res = State.replaceMethod(oldFunc, newFunc)
        if res:
            print('replaced %s FSM transition function(s): %s' % (res, funcName))

def replaceCRFunc(replaceFuncList):
    try:
        base.cr
    except:
        return
    # masad: Gyedo's fake cr causes a crash in followingreplaceMethod on rebinding, so
    # I throw in the isFake check. I still think the fake cr should be eliminated.
    if hasattr(base.cr,'isFake'):
        return
    for oldFunc, funcName, newFunc in replaceFuncList:
        if base.cr.replaceMethod(oldFunc, newFunc):
            print('replaced DistributedObject function: %s' % funcName)

def replaceAIRFunc(replaceFuncList):
    try:
        simbase.air
    except:
        return
    for oldFunc, funcName, newFunc in replaceFuncList:
        if simbase.air.replaceMethod(oldFunc, newFunc):
            print('replaced DistributedObject function: %s' % funcName)

def replaceIvalFunc(replaceFuncList):
    # Make sure we have imported IntervalManager and thus created
    # a global ivalMgr.
    if not sys.modules.get('base.direct.interval.IntervalManager'):
        return
    from direct.interval.FunctionInterval import FunctionInterval
    for oldFunc, funcName, newFunc in replaceFuncList:
        res = FunctionInterval.replaceMethod(oldFunc, newFunc)
        if res:
            print('replaced %s interval function(s): %s' % (res, funcName))
