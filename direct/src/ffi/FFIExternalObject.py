
import FFIConstants
import TypedObject

WrapperClassMap = {}

DowncastMap = {}

# For testing, you can turn verbose and debug on
# FFIConstants.notify.setVerbose(1)
# FFIConstants.notify.setDebug(1)



# Register a python class in the type map if it is a typed object
# The type map is used for upcasting and downcasting through
# the panda inheritance chain
def registerInTypeMap(pythonClass):
    if issubclass(pythonClass, TypedObject.TypedObject):
        typeIndex = pythonClass.getClassType().getIndex()
        WrapperClassMap[typeIndex] = pythonClass



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
        FFIConstants.notify.debug('getLineageInternal: checking %s to %s'
                                  % (thisClass.__name__, targetBaseClass.__name__))
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
                    # Actually, we want them in the reverse order
                    res.reverse()
                    FFIConstants.notify.debug('getLineageInternal: found path: ' + `res`)
                    return res
            # Not found anywhere
            return 0

    def getDowncastFunctions(self, thisClass, baseClass):
        FFIConstants.notify.debug(
            'getDowncastFunctions: Looking for downcast function from %s to %s'
            % (baseClass.__name__, thisClass.__name__))
        lineage = self.getLineage(thisClass, baseClass)
        # Start with an empty list of downcast functions
        downcastFunctionList = []

        # If it could not find the baseClass anywhere in the lineage, return empty
        if not lineage:
            return []

        # Walk along the lineage looking for downcast functions from class to class+1
        for i in range((len(lineage) - 1)):
            fromClass = lineage[i]
            toClass = lineage[i+1]
            downcastFuncName = ('downcastTo' + toClass.__name__
                                + 'From' + fromClass.__name__)
            # Look over this classes global modules dictionaries
            # for the downcast function name
            for globmod in fromClass.__CModuleDowncasts__:
                if globmod.__dict__.has_key(downcastFuncName):
                    func = globmod.__dict__[downcastFuncName]
                    FFIConstants.notify.debug(
                        'getDowncastFunctions: Found downcast function %s in %s'
                        % (downcastFuncName, globmod.__name__))
                    downcastFunctionList.append(func)
                else:
                    FFIConstants.notify.debug(
                        'getDowncastFunctions: Did not find downcast function %s in %s'
                        % (downcastFuncName, globmod.__name__))
        return downcastFunctionList
        
    def setPointer(self):
        if (self.this == 0):
            # Null pointer, return None
            return None
        # If it is not a typed object, our work is done, just return the object
        if (not isinstance(self, TypedObject.TypedObject)): 
            return self
        # Ok, it is a typed object. See what type it really is and downcast
        # to that type (if necessary)
        exactWrapperClass = self.wrapperClassForTypeHandle(self.getType())
        # We do not need to downcast if we already have the same class
        if (exactWrapperClass and (exactWrapperClass != self.__class__)):
            # Create a new wrapper class instance
            exactObject = exactWrapperClass(None)
            # Get the downcast pointer that has had all the downcast funcs called
            downcastObject = self.downcast(exactWrapperClass)
            exactObject.this = downcastObject.this
            exactObject.userManagesMemory = downcastObject.userManagesMemory
            # Make sure the original downcast object does not get garbage collected
            # so that the exactObject will not get gc'd thereby transferring ownership
            # of the object to this new exactObject
            downcastObject.userManagesMemory = 0
            return exactObject
        else:
            return self
 
    def wrapperClassForTypeHandle(self, aTypeHandle):
        if WrapperClassMap.has_key(aTypeHandle.getIndex()):
            return WrapperClassMap[aTypeHandle.getIndex()]
        else:
            return None
        
    def downcast(self, toClass):
        fromClass = self.__class__
        FFIConstants.notify.debug('downcast: downcasting from %s to %s' % \
            (fromClass.__name__, toClass.__name__))

        # Check the cache to see if we have looked this up before
        if DowncastMap.has_key((fromClass, toClass)):
            downcastChain = DowncastMap[(fromClass, toClass)]
            FFIConstants.notify.debug('downcast: found cached downcast chain: ' + `downcastChain`)            
        else:
            downcastChain = self.getDowncastFunctions(toClass, fromClass)
            FFIConstants.notify.debug('downcast: computed downcast chain: ' + `downcastChain`)
            # Store it for next time
            DowncastMap[(fromClass, toClass)] = downcastChain
            
        newObject = self
        for downcastFunc in downcastChain:
            FFIConstants.notify.debug('downcast: downcasting %s using %s' % \
                                      (newObject.__class__.__name__, downcastFunc))
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
        else:
            return 1

    def __repr__(self):
        # Lots of Panda classes have an output function defined that takes an Ostream
        # We create a LineStream for the output function to write to, then we extract
        # the string out of it and return it as our str
        baseRepr = ''
        import LineStream
        lineStream = LineStream.LineStream()
        try:
            self.output(lineStream)
            baseRepr = baseRepr + lineStream.getLine()
        except:
            pass
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
        import LineStream
        lineStream = LineStream.LineStream()
        try:
            # First try the write function, that is the better one
            self.write(lineStream)
            while lineStream.isTextAvailable():
                baseRepr = baseRepr + '\n' + lineStream.getLine()
        except:
            try:
                # Ok, no write function, lets try output then
                self.output(lineStream)
                while lineStream.isTextAvailable():
                    baseRepr = baseRepr + '\n' + lineStream.getLine()
            except:
                pass
        # In any case, return the baseRepr
        return baseRepr

    def __hash__(self):
        return self.this




    
