
import types
import os

def findClassInModule(module, className, visited):
    # Make sure you have not already visited this module
    # to prevent recursion
    if module in visited:
        return None
    # Ok, clear to proceed, add this module to the visited list
    visited.append(module)
    # print ('visiting: ' + `module`)
    # Look in this module for classes and other modules
    for key in module.__dict__.keys():
        value = module.__dict__[key]
        # If this is a class
        if ((key != "_") and (type(value) == types.ClassType)):
            # See if the name matches
            if value.__name__ == className:
                # It does! We found our class
                return [value, module.__dict__]
        # Its a module, recursively look into its namespace
        elif (type(value) == types.ModuleType):
            ret =  findClassInModule(value, className, visited)
            # If that recursion found it, return the goodies
            if ret:
                return ret
            # Otherwise keep looking
    # Well, after all that we did not find anything
    return None


# Find a class named className somewhere in this namespace
def findClass(namespace, className):
    for key in namespace.keys():
        value = namespace[key]
        # If we found a class, see if it matches classname
        # Make sure we do not match "_"
        if ((key != "_") and (type(value) == types.ClassType)):
            if value.__name__ == className:
                # It does, that was easy!
                return [value, namespace]
        # Look in all the modules in this namespace
        elif (type(value) == types.ModuleType):
            ret = findClassInModule(value, className, [])
            # If we found it return the goodies
            if ret:
                return ret
            # Otherwise keep looking
    # Nope, not in there
    return None


def rebindClass(builtins, filename):
    file = open(filename, 'r')
    lines = file.readlines()
    curLine = 0
    found = 0
    foundLine = -1
    foundChar = -1
    for i in range(len(lines)):
        line = lines[i]
        if (line[0:6] == 'class '):
            classHeader = line[6:]
            # Look for a open paren if it does inherit
            parenLoc = classHeader.find('(')
            if parenLoc > 0:
                className = classHeader[:parenLoc]
                print 'Rebinding class name: ' + className
                found = 1
                foundLine = i
                foundChar = parenLoc
            else:
                # Look for a colon if it does not inherit
                colonLoc = classHeader.find(':')
                if colonLoc > 0:
                    className = classHeader[:colonLoc]
                    # print 'found className: ' + className
                    found = 1
                    foundLine = i
                    foundChar = colonLoc

    if not found:
        print 'error: className not found'
        return

    # Store the original real class
    res = findClass(builtins, className)
    if res:
        realClass, realNameSpace = res
    else:
        # print ('Warning: could not find class, defining new class in builtins: ' + className)
        # Now execute that class def
        # execfile(filename, builtins)
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

    res = findClass(realNameSpace, className)
    if res:
        tmpClass, tmpNameSpace = res
    else:
        print ('Internal error redefining class: could not find temp class')
        return

    # Copy the functions that we just redefined into the real class
    copyFuncs(tmpClass, realClass)

    # Now make sure the original class is in that namespace, not our temp one
    realNameSpace[className] = realClass


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
            toClass.__dict__[key] = newFunc

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
    import State
    res = State.redefineEnterFunc(oldFunc, newFunc)
    if res:
        print ('replaced state enter function: ' + newFunc.__name__)
    res = State.redefineExitFunc(oldFunc, newFunc)
    if res:
        print ('replaced state exit function: ' + newFunc.__name__)

def replaceTcrFunc(oldFunc, newFunc):
    try:
        res = toonbase.tcr.replaceMethod(oldFunc, newFunc)
    except:
        try:
            res = simbase.air.replaceMethod(oldFunc, newFunc)
        except:
            res = None
    if res:
        print ('replaced DistributedObject function: ' + newFunc.__name__)
