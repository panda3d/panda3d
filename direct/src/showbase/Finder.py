
import types

def findClassInModule(module, className, nesting, visited=[]):
    # Make sure you have not already visited this module
    # to prevent recursion
    if module in visited:
        return None
    # Ok, clear to proceed, add this module to the visited list
    visited.append(module)
    print ('visiting: ' + `module`)
    for key in module.__dict__.keys():
        value = module.__dict__[key]
        if (type(value) == types.ClassType):
            if value.__name__ == className:
                return value
        elif (type(value) == types.ModuleType):
            ret =  findClassInModule(value, className, nesting+1, visited)
            if ret:
                return ret
            # otherwise keep looking
    return None


def findClass(builtins, className):
    for key in builtins.keys():
        value = builtins[key]
        if (type(value) == types.ClassType):
            if value.__name__ == className:
                return value
        elif (type(value) == types.ModuleType):
            ret = findClassInModule(value, className, 0)
            if ret:
                return ret
            # otherwise keep looking
    return None



def rebindClass(builtins, filename):
    tempClassName = 'py_temp_class'
    file = open(filename)
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
                print 'found className: ' + className
                found = 1
                foundLine = i
                foundChar = parenLoc
            else:
                # Look for a colon if it does not inherit
                colonLoc = classHeader.find(':')
                if colonLoc > 0:
                    className = classHeader[:colonLoc]
                    print 'found className: ' + className
                    found = 1
                    foundLine = i
                    foundChar = colonLoc

    if not found:
        print 'error: className not found'
        return None

    # Store the original real class
    realClass = findClass(builtins, className)

    tmpfilename = '/usr/local/tmp_py_file'
    tmpfile = open(tmpfilename, 'w')
    # newline = 'class ' + tempClassName + lines[foundLine][(6+foundChar):]
    newline = 'class ' + tempClassName + ':\012'
    
    # now write the class back to the file with the new class name
    for i in range(len(lines)):
        if (i == foundLine):
            tmpfile.write(newline)
        else:
            tmpfile.write(lines[i])
            
    file.close()
    tmpfile.close()

    # Now execute that class def
    execfile(tmpfilename, builtins)

    tmpClass = findClass(builtins, tempClassName)

    print 'realClass: ' + `realClass`
    print 'tmpClass: ' + `tmpClass`

    # Reassign the new dict
    #copyFuncs(tmpClass, realClass)
    copyDict(tmpClass, realClass)


def copyFuncs(fromClass, toClass):
    for key in fromClass.__dict__.keys():
        value = fromClass.__dict__[key]
        if (type(value) == types.FunctionType):
            toClass.__dict__[key] = value


def copyDict(fromClass, toClass):
    oldModule = toClass.__module__
    toClass.__dict__ = fromClass.__dict__
    toClass.__module__ = oldModule

class Finder:
    def __init__(self):
        pass
    def tester(self):
        pass

    
