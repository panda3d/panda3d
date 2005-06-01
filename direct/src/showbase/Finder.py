
import types
import os
import new

def findClassInModule(module, className, visited):
    # Make sure you have not already visited this module
    # to prevent recursion
    if visited.has_key(module):
        return None
    # Ok, clear to proceed, add this module to the visited list
    visited[module] = 1
    # print ('visiting: ' + `module`)

    # First see if we are in the dict at this level
    classObj = module.__dict__.get(className)
    if classObj and ((type(classObj) == types.ClassType) or
                     (type(classObj) == types.TypeType)):
        return [classObj, module.__dict__]

    # Now filter out all the modules and iterate through them
    moduleList = filter(lambda value: type(value) == types.ModuleType, module.__dict__.values())
    for moduleObj in moduleList:
        ret =  findClassInModule(moduleObj, className, visited)
        # If that recursion found it, return the goodies
        if ret:
            return ret
            
    # Well, after all that we did not find anything
    return None


# Find a class named className somewhere in this namespace
def findClass(namespace, className):

    # First see if we are in the namespace
    classObj = namespace.get(className)
    # print classObj, type(classObj)
    if classObj and ((type(classObj) == types.ClassType) or
                     (type(classObj) == types.TypeType)):
        return [classObj, namespace]
    
    for key in namespace.keys():
        value = namespace[key]
        # If we found a class, see if it matches classname
        # Make sure we do not match "_"
        if ((key != "_") and ((type(value) == types.ClassType) or
                              (type(value) == types.TypeType))):
            if value.__name__ == className:
                # It does, that was easy!
                return [value, namespace]
        # Look in all the modules in this namespace
        elif (type(value) == types.ModuleType):
            ret = findClassInModule(value, className, {})
            # If we found it return the goodies
            if ret:
                return ret
            # Otherwise keep looking
    # Nope, not in there
    return None


def rebindClass(builtinGlobals, filename):
    file = open(filename, 'r')
    lines = file.readlines()
    for i in xrange(len(lines)):
        line = lines[i]
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
                    print 'error: className not found'
                    return
            print 'Rebinding class name: ' + className
            break

    # Store the original real class
    res = findClass(builtinGlobals, className)
    if res:
        realClass, realNameSpace = res
    else:
        # print ('Warning: could not find class, defining new class in builtins: ' + className)
        # Now execute that class def
        # execfile(filename, builtinGlobals)
        print ('Warning: Finder could not find class, try importing the file first')
        # Remove that temp file
        file.close()
        os.remove(filename)
        return

    # Now execute that class def
    execfile(filename, realNameSpace)
    # Remove that temp file
    file.close()
    os.remove(filename)

    tmpClass = realNameSpace[className]

    # Copy the functions that we just redefined into the real class
    copyFuncs(tmpClass, realClass)

    # Now make sure the original class is in that namespace, not our temp one
    realNameSpace[className] = realClass

    print ('    Finished rebind')


def copyFuncs(fromClass, toClass):
    # Copy the functions from fromClass into toClass dictionary
    for key in fromClass.__dict__.keys():
        value = fromClass.__dict__[key]
        if (type(value) == types.FunctionType):
            newFunc = value
            # See if we already have a function with this name
            if toClass.__dict__.has_key(key):
                # Look in the messenger and taskMgr to see if this
                # old function pointer is stored there,
                # and update it to the new function pointer
                oldFunc = toClass.__dict__[key]
                replaceMessengerFunc(oldFunc, newFunc)
                replaceTaskMgrFunc(oldFunc, newFunc)
                replaceStateFunc(oldFunc, newFunc)
                replaceTcrFunc(oldFunc, newFunc)
            # You cannot assign directly to the dict with new style classes
            # toClass.__dict__[key] = newFunc
            # Instead we will use setattr
            setattr(toClass, key, newFunc)

def replaceMessengerFunc(oldFunc, newFunc):
    try:
        messenger
    except:
        return
    res = messenger.replaceMethod(oldFunc, newFunc)
    if res:
        print 'replaced %d messenger functions: %s' % (res, newFunc.__name__)

def replaceTaskMgrFunc(oldFunc, newFunc):
    try:
        taskMgr
    except:
        return
    res = taskMgr.replaceMethod(oldFunc, newFunc)
    if res:
        print ('replaced taskMgr function: ' + newFunc.__name__)

def replaceStateFunc(oldFunc, newFunc):
    from direct.fsm import State
    res = State.redefineEnterFunc(oldFunc, newFunc)
    if res:
        print ('replaced state enter function: ' + newFunc.__name__)
    res = State.redefineExitFunc(oldFunc, newFunc)
    if res:
        print ('replaced state exit function: ' + newFunc.__name__)

def replaceTcrFunc(oldFunc, newFunc):
    try:
        res = base.cr.replaceMethod(oldFunc, newFunc)
    except:
        try:
            res = simbase.air.replaceMethod(oldFunc, newFunc)
        except:
            res = None
    if res:
        print ('replaced DistributedObject function: ' + newFunc.__name__)
