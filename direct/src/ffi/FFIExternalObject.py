from new import instance
import FFIConstants

WrapperClassMap = {}

DowncastMap = {}

# For testing, you can turn verbose and debug on
# FFIConstants.notify.setInfo(1)
# FFIConstants.notify.setDebug(1)

# Uncomment the notify statements if you need to debug,
# otherwise leave them commented out to prevent runtime
# overhead of calling them



# Register a python class in the type map if it is a typed object
# The type map is used for upcasting and downcasting through
# the panda inheritance chain
def registerInTypeMap(pythonClass):
    from pandac import TypedObject
    if issubclass(pythonClass, TypedObject.TypedObject):
        typeIndex = pythonClass.getClassType().getIndex()
        WrapperClassMap[typeIndex] = pythonClass


def funcToMethod(func, clas, method_name=None):
    """Adds func to class so it is an accessible method; use method_name to specify the name to be used for calling the method.
    The new method is accessible to any instance immediately."""
    func.im_class=clas
    func.im_func=func
    func.im_self=None
    if not method_name:
        clas.__dict__[method_name]=func
    else:
        clas.__dict__[func.__name__]=func


def FFIInstance(classdef, this = 0, userManagesMemory = 0):
    answer = instance(classdef)
    answer.this = this
    answer.userManagesMemory = userManagesMemory
    return answer

class FFIExternalObject:
    def __init__(self, *_args):
        # By default, we do not manage our own memory
        self.userManagesMemory = 0
        # Start with a null this pointer
        self.this = 0

    def destructor(self):
        # Base destructor in case you do not define one
        pass

    def getLineage(self, thisClass, targetBaseClass):
        # Recursively determine the path in the heirarchy tree from thisClass
        # to the targetBaseClass
        return self.getLineageInternal(thisClass, targetBaseClass, [thisClass])

    def getLineageInternal(self, thisClass, targetBaseClass, chain):
        # Recursively determine the path in the heirarchy tree from thisClass
        # to the targetBaseClass
        #FFIConstants.notify.debug('getLineageInternal: checking %s to %s'
        #                          % (thisClass.__name__, targetBaseClass.__name__))
        if (targetBaseClass in thisClass.__bases__):
            # Found a link
            return chain + [targetBaseClass]
        elif (len(thisClass.__bases__) == 0):
            # No possible links
            return 0
        else:
            # recurse
            for base in thisClass.__bases__:
                res = self.getLineageInternal(base, targetBaseClass, chain+[base])
                if res:
                    # FFIConstants.notify.debug('getLineageInternal: found path: ' + `res`)
                    return res
            # Not found anywhere
            return 0

    def getDowncastFunctions(self, thisClass, baseClass):
        #FFIConstants.notify.debug(
        #    'getDowncastFunctions: Looking for downcast function from %s to %s'
        #    % (baseClass.__name__, thisClass.__name__))
        lineage = self.getLineage(thisClass, baseClass)
        # Start with an empty list of downcast functions
        downcastFunctionList = []

        # If it could not find the baseClass anywhere in the lineage,
        # return empty
        if not lineage:
            return []

        # Walk along the lineage looking for downcast functions from
        # class to class+1.  Start at the top and work downwards.
        top = len(lineage) - 1
        for i in range(top):
            toClass = lineage[top - i - 1]
            fromClass = lineage[top - i]
            downcastFuncName = ('downcastTo' + toClass.__name__
                                + 'From' + fromClass.__name__)
            # Look over this classes global modules dictionaries
            # for the downcast function name
            for globmod in toClass.__CModuleDowncasts__:
                func = globmod.__dict__.get(downcastFuncName)
                if func:
                    #FFIConstants.notify.debug(
                    #    'getDowncastFunctions: Found downcast function %s in %s'
                    #    % (downcastFuncName, globmod.__name__))
                    downcastFunctionList.append(func)
        return downcastFunctionList

    def lookUpNewType(self, typeHandle, rootType):
        # We tried to downcast to an unknown type.  Try to figure out
        # the lowest type we *do* know, so we can downcast to that
        # type instead.
        if typeHandle.getNumParentClasses() == 0:
            # This type has no parents!  That shouldn't happen.
            FFIConstants.notify.warning("Unknown class type: %s has no parents!" % (typeHandle.getName()))
            return None

        parentType = typeHandle.getParentTowards(rootType, self)
        parentIndex = parentType.getIndex()
        parentWrapperClass = WrapperClassMap.get(parentIndex)
        if parentWrapperClass == None:
            parentWrapperClass = self.lookUpNewType(parentType, rootType)

        if parentWrapperClass != None:
            # If the parent class is known, then record that this
            # class is a derivation of that parent class.
            WrapperClassMap[typeHandle.getIndex()] = parentWrapperClass

        return parentWrapperClass

    def setPointer(self):
        # See what type it really is and downcast to that type (if necessary)
        # Look up the TypeHandle in the dict. get() returns None if it is not there
        index = self.getTypeIndex()
        exactWrapperClass = WrapperClassMap.get(index)
        if exactWrapperClass == None:
            # This is an unknown class type.  Perhaps it derives from
            # a class type we know.
            exactWrapperClass = self.lookUpNewType(self.getType(), self.getClassType())

        # We do not need to downcast if we already have the same class
        if (exactWrapperClass and (exactWrapperClass != self.__class__)):
            # Create a new wrapper class instance
            #exactObject = exactWrapperClass(None)
            exactObject = FFIInstance(exactWrapperClass)
            # Get the downcast pointer that has had all the downcast
            # funcs called
            downcastObject = self.downcast(exactWrapperClass)
            exactObject.this = downcastObject.this
            exactObject.userManagesMemory = downcastObject.userManagesMemory
            # Make sure the original downcast object does not get
            # garbage collected so that the exactObject will not get
            # gc'd thereby transferring ownership of the object to
            # this new exactObject
            downcastObject.userManagesMemory = 0
            return exactObject
        else:
            return self

    def downcast(self, toClass):
        fromClass = self.__class__
        #FFIConstants.notify.debug('downcast: downcasting from %s to %s' % \
        #    (fromClass.__name__, toClass.__name__))

        # Check the cache to see if we have looked this up before
        downcastChain = DowncastMap.get((fromClass, toClass))
        if downcastChain == None:
            downcastChain = self.getDowncastFunctions(toClass, fromClass)
            #FFIConstants.notify.debug('downcast: computed downcast chain: ' + `downcastChain`)
            # Store it for next time
            DowncastMap[(fromClass, toClass)] = downcastChain
        newObject = self
        for downcastFunc in downcastChain:
            #FFIConstants.notify.debug('downcast: downcasting %s using %s' % \
            #                         (newObject.__class__.__name__, downcastFunc))
            newObject = downcastFunc(newObject)
        return newObject

    def compareTo(self, other):
        # By default, we compare the C++ pointers
        # Some classes will override the compareTo operator with their own
        # logic in C++ (like vectors and matrices for instance)
        try:
            if self.this < other.this:
                return -1
            if self.this > other.this:
                return 1
            else:
                return 0
        except:
            return 1

    def __cmp__(self, other):
        # Only use the C++ compareTo if they are the same class
        if isinstance(other, self.__class__):
            return self.compareTo(other)
        # Otherwise, they must not be the same
        # Just do a basic python id compare
        else:
            return cmp(id(self), id(other))

    def __repr__(self):
        # Lots of Panda classes have an output function defined that takes an Ostream
        # We create a LineStream for the output function to write to, then we extract
        # the string out of it and return it as our str
        try:
            from pandac import LineStream
            lineStream = LineStream.LineStream()
            self.output(lineStream)
            baseRepr = lineStream.getLine()
        except AssertionError, e:
            raise AssertionError, e
        except:
            baseRepr = ('[' + self.__class__.__name__ + ' at: ' + `self.this` + ']')
        # In any case, return the baseRepr
        return baseRepr

    def __str__(self):
        # This is a more complete version of printing which shows the object type
        # and pointer, plus the output from write() or output() whichever is defined
        # Print this info for all objects
        baseRepr = ('[' + self.__class__.__name__ + ' at: ' + `self.this` + ']')
        # Lots of Panda classes have an write or output function defined that takes an Ostream
        # We create a LineStream for the write or output function to write to, then we extract
        # the string out of it and return it as our repr
        from pandac import LineStream
        lineStream = LineStream.LineStream()
        try:
            # First try the write function, that is the better one
            self.write(lineStream)
            while lineStream.isTextAvailable():
                baseRepr = baseRepr + '\n' + lineStream.getLine()
        except AssertionError, e:
            raise AssertionError, e
        except:
            try:
                # Sometimes write insists on a seconds parameter.
                self.write(lineStream, 0)
                while lineStream.isTextAvailable():
                    baseRepr = baseRepr + '\n' + lineStream.getLine()
            except AssertionError, e:
                raise AssertionError, e
            except:
                try:
                    # Ok, no write function, lets try output then
                    self.output(lineStream)
                    while lineStream.isTextAvailable():
                        baseRepr = baseRepr + '\n' + lineStream.getLine()
                except AssertionError, e:
                    raise AssertionError, e
                except:
                    pass
        # In any case, return the baseRepr
        return baseRepr

    def __hash__(self):
        return self.this





