from direct.showbase.PythonUtil import *
from types import *
import string
import FFIConstants
import FFISpecs
import FFITypes

"""
Things that are not supported:
 - Overloading a function based on an enum being differentiated from an int
 - Type names from C++ cannot have __enum__ in their name
 - Overloading static and non-static methods with the same name
"""

AT_not_atomic = 0
AT_int = 1
AT_float = 2
AT_double = 3
AT_bool = 4
AT_char = 5
AT_void = 6
AT_string = 7
AT_longlong = 8

def cullOverloadedMethods(fullMethodDict):
    """
    Find all the entries that have multiple indexes for the same method name
    Get rid of all others.
    """
    tmpDict = {}
    # For each class
    for methodName in fullMethodDict.keys():
        methodList = fullMethodDict[methodName]
        # See if this method has more than one function index (overloaded)
        if (len(methodList) > 1):
            tmpDict[methodName] = methodList
            # Mark all the method specifications as overloaded
            for methodSpec in methodList:
                methodSpec.overloaded = 1

    return tmpDict


def getTypeName(classTypeDesc, typeDesc):
    """
    Map the interrogate primitive type names to python type names.
    We assume that the module using this has imported the types module.
    It is valid to pass in None for classTypeDesc if we are not in a class
    """

    typeName = typeDesc.getFullNestedName()

    # Atomic C++ types are type checked against the builtin
    # Python types. This code sorts out the mapping
    if typeDesc.isAtomic():

        # Ints, bools, and chars are treated as ints.
        # Enums are special and are not atomic, see below
        if ((typeDesc.atomicType == AT_int) or
            (typeDesc.atomicType == AT_bool) or
            (typeDesc.atomicType == AT_char)):
            return 'IntType'

        # Floats and doubles are both floats in Python
        elif ((typeDesc.atomicType == AT_float) or
            (typeDesc.atomicType == AT_double)):
            return 'FloatType'

        elif ((typeDesc.atomicType == AT_longlong)):
            return 'LongType'

        # Strings are treated as Python strings
        elif ((typeDesc.atomicType == AT_string)):
            return 'StringType'

        elif (typeDesc.atomicType == AT_void):
            # Convert the void type to None type... I guess...
            # So far we do not have any code that uses this
            return 'NoneType'

        else:
            FFIConstants.notify.error("Unknown atomicType: %s" % (typeDesc.atomicType))

    # If the type is an enum, we really want to treat it like an int
    # To handle this, the type will have __enum__ in the name
    # Usually it will start the typeName, but some typeNames have the
    # surrounding class as part of their name
    # like BoundedObject.__enum__BoundingVolumeType
    elif (typeName.find('__enum__') >= 0):
        return 'IntType'

    # If it was not atomic or enum, it must be a class which is a
    # bit trickier because we output different things depending on the
    # scoping of the type.
    else:

        #   classTypeDesc  typeDesc fullNestedName Resulting TypeName
        # 1   Outer         Other     Other          Other.Other
        # 2   Outer         Outer     Outer          Outer
        # 3   Outer         Inner     Outer.Inner    Outer.Inner
        # 4   Inner         Other     Other          Other.Other
        # 5   Inner         Outer     Outer          Outer
        # 6   Inner         Inner     Outer.Inner    Outer.Inner
        # 7   None          Other     Other          Other.Other

        # CASES 1, 4, and 7 are the only ones that are different from the full
        # nested name, returning Other.Other

        returnNestedTypeNames = string.split(typeName, '.')
        returnModuleName = returnNestedTypeNames[0]

        if classTypeDesc:
            classTypeName = classTypeDesc.getFullNestedName()
            classNestedTypeNames = string.split(classTypeName, '.')
            # If there is no nesting, return typeName.typeName
            if ((not (classTypeDesc.foreignTypeName in returnNestedTypeNames)) and
                (not (typeDesc.foreignTypeName in classNestedTypeNames))):
                return (returnModuleName + '.' + typeName)
            # All other cases, we just need typeName
            else:
                return typeName
        else:
            # If you had no class, you need to specify module plus typename
            return (returnModuleName + '.' + typeName)


def inheritsFrom(type1, type2):
    """
    Return true if type1 inherits from type2
    This works by recursively checking parentTypes for type1
    """
    if type1.parentTypes:
        if type2 in type1.parentTypes:
            return 1
        else:
            result = 0
            for type in type1.parentTypes:
                result = (result or inheritsFrom(type, type2))
            return result
    else:
        return 0

def getInheritanceLevel(type, checkNested = 1):
    if type.__class__ == FFITypes.PyObjectTypeDescriptor:
        # A special case: PyObject * is always the most general
        # object.  Everything is a PyObject.
        return -1

    # If this is a nested type, return the inheritance level of the outer type.
    if type.isNested:
        # Check the level of your outer class
        # pass the checkNested flag as 0 to prevent an infinite loop
        # between the parent and child
        level = getInheritanceLevel(type.outerType, 0)
    else:
        level = 0

    for parentType in type.parentTypes:
        # Add 1 because you are one level higher than your parent
        level = max(level, 1+getInheritanceLevel(parentType))

    if checkNested:
        for nestedType in type.nestedTypes:
            # Do not add 1 to your nested types
            level = max(level, getInheritanceLevel(nestedType))

    return level

def inheritanceLevelSort(type1, type2):
    level1 = getInheritanceLevel(type1)
    level2 = getInheritanceLevel(type2)
    if (level1 == level2):
        # If they are equal in the inheritance,
        # sort them alphabetically by their type name
        return cmp(type1.foreignTypeName, type2.foreignTypeName)
    elif (level1 < level2):
        return -1
    elif (level1 > level2):
        return 1


def subclass(type1, type2):
    """
    Helper funcion used in sorting classes by inheritance
    """
    # If the types are the same, return 0
    if type1 == type2:
        return 0
    # If you have no args, sort you first
    elif (type1 == 0):
        return 1
    elif (type2 == 0):
        return -1
    # If class1 inherits from class2 return -1
    elif inheritsFrom(type1, type2):
        return -1
    # If class2 inherits from class1 return 1
    elif inheritsFrom(type2, type1):
        return 1
    else:
        # This is the dont care case. We must specify a sorting
        # rule just so it is not arbitrary
        if (type1.foreignTypeName > type2.foreignTypeName):
            return -1
        else:
            return 1


class FFIMethodArgumentTreeCollection:
    def __init__(self, classTypeDesc, methodSpecList):
        self.classTypeDesc = classTypeDesc
        self.methodSpecList = methodSpecList
        self.methodDict = {}
        self.treeDict = {}

    def outputOverloadedMethodHeader(self, file, nesting):
        # If one is static, we assume they all are.
        # The current system does not support overloading static and non-static
        # methods with the same name
        # Constructors are not treated as static. They are special because
        # they are not really constructors, they are instance methods that fill
        # in the this pointer.
        # Global functions do not need static versions
        if (self.methodSpecList[0].isStatic() and
            (not self.methodSpecList[0].isConstructor())):
            indent(file, nesting, 'def ' +
                   self.methodSpecList[0].name + '(*_args):\n')
        else:
            indent(file, nesting, 'def ' +
                   self.methodSpecList[0].name + '(self, *_args):\n')
        self.methodSpecList[0].outputCFunctionComment(file, nesting+2)
        indent(file, nesting+2, 'numArgs = len(_args)\n')

    def outputOverloadedMethodFooter(self, file, nesting):
        # If this is a static method, we need to output a static version
        # If one is static, we assume they all are.
        # The current system does not support overloading static and non-static
        # methods with the same name
        # Constructors are not treated as static. They are special because
        # they are not really constructors, they are instance methods that fill
        # in the this pointer.
        methodName = self.methodSpecList[0].name

        if (self.methodSpecList[0].isStatic() and
            (not self.methodSpecList[0].isConstructor()) and
            (not isinstance(self.methodSpecList[0], FFISpecs.GlobalFunctionSpecification))):
                self.outputOverloadedStaticFooter(file, nesting)
        else:
            if self.classTypeDesc:
                indent(file, nesting,   "FFIExternalObject.funcToMethod("+methodName+','+ self.classTypeDesc.foreignTypeName+ ",'"+methodName+"')\n")
                indent(file, nesting,   'del '+methodName+'\n')
                indent(file, nesting, ' \n')

        indent(file, nesting+1, '\n')

    def outputOverloadedStaticFooter(self, file, nesting):
        # foo = staticmethod(foo)
        methodName = self.methodSpecList[0].name
        indent(file, nesting, self.classTypeDesc.foreignTypeName + '.' + methodName + ' = staticmethod(' + methodName + ')\n')
        indent(file, nesting,'del ' +methodName+' \n\n')

    def setup(self):
        for method in self.methodSpecList:
            numArgs = len(method.typeDescriptor.thislessArgTypes())
            numArgsList = self.methodDict.setdefault(numArgs, [])
            numArgsList.append(method)
        for numArgs in self.methodDict.keys():
            methodList = self.methodDict[numArgs]
            tree = FFIMethodArgumentTree(self.classTypeDesc, methodList)
            treeList = self.treeDict.setdefault(numArgs, [])
            treeList.append(tree)

    def generateCode(self, file, nesting):
        self.setup()
        self.outputOverloadedMethodHeader(file, nesting)
        numArgsKeys = self.treeDict.keys()
        numArgsKeys.sort()
        for i in range(len(numArgsKeys)):
            numArgs = numArgsKeys[i]
            trees = self.treeDict[numArgs]
            for tree in trees:
                # If this is the first case, output an if clause
                if (i == 0):
                    indent(file, nesting+2, 'if (numArgs == ' + `numArgs` + '):\n')
                # If this is a subsequent first case, output an elif clause
                else:
                    indent(file, nesting+2, 'elif (numArgs == ' + `numArgs` + '):\n')
                tree.setup()
                tree.traverse(file, nesting+1, 0)

        # If the overloaded function got all the way through the if statements
        # it must have had the wrong number or type of arguments
        indent(file, nesting+2, "else:\n")
        indent(file, nesting+3, "raise TypeError, 'Invalid number of arguments: ' + `numArgs` + ', expected one of: ")
        for numArgs in numArgsKeys:
            indent(file, 0, (`numArgs` + ' '))
        indent(file, 0, "'\n")

        self.outputOverloadedMethodFooter(file, nesting)



class FFIMethodArgumentTree:
    """
    Tree is made from nested dictionaries.
    The keys are methodNamed.
    The values are [tree, methodSpec]
    methodSpec may be None at any level
    If tree is None, it is a leaf node and methodSpec will be defined
    """
    def __init__(self, classTypeDesc, methodSpecList):
        self.argSpec = None
        self.classTypeDesc = classTypeDesc
        self.methodSpecList = methodSpecList
        # The actual tree is implemented as nested dictionaries
        self.tree = {}

    def setup(self):
        for methodSpec in self.methodSpecList:
            argTypes = methodSpec.typeDescriptor.thislessArgTypes()
            self.fillInArgTypes(argTypes, methodSpec)

    def fillInArgTypes(self, argTypes, methodSpec):
        # If the method takes no arguments, we will assign a type index of 0
        if (len(argTypes) == 0):
            self.tree[0] = [
                FFIMethodArgumentTree(self.classTypeDesc,
                                      self.methodSpecList),
                methodSpec]

        else:
            self.argSpec = argTypes[0]
            typeDesc = self.argSpec.typeDescriptor.recursiveTypeDescriptor()

            if (len(argTypes) == 1):
                # If this is the last parameter, we are a leaf node, so store the
                # methodSpec in this dictionary
                self.tree[typeDesc] = [None, methodSpec]
            else:
                if self.tree.has_key(typeDesc):
                    # If there already is a tree here, jump into and pass the
                    # cdr of the arg list
                    subTree = self.tree[typeDesc][0]
                    subTree.fillInArgTypes(argTypes[1:], methodSpec)
                else:
                    # Add a subtree for the rest of the arg list
                    subTree = FFIMethodArgumentTree(self.classTypeDesc,
                                                    self.methodSpecList)
                    subTree.fillInArgTypes(argTypes[1:], methodSpec)
                    # This subtree has no method spec
                    self.tree[typeDesc] = [subTree, None]

    def traverse(self, file, nesting, level):
        oneTreeHasArgs = 0
        typeNameList = []

        # First see if this tree branches at all. If it does not there are
        # drastic optimizations we can take because we can simply call the
        # bottom-most function. We are not checking the types of all the
        # arguments for the sake of type checking, we are simply trying to
        # figure out which overloaded function to call. If there is only
        # one overloaded function with this number of arguements at this
        # level, it must be the one. No need to continue checking all the
        # arguments.
        branches = 0
        subTree = self
        prevTree = subTree
        levelCopy = level

        while subTree:
            if (len(subTree.tree.keys()) == 0):
                # Dead end branch
                break
            if (len(subTree.tree.keys()) > 1):
                # Ok, we branch, it was worth a try though
                branches = 1
                break

            prevTree = subTree
            # Must only have one subtree, traverse it
            subTree = subTree.tree.values()[0][0]
            levelCopy += 1

        # If there were no branches, this is easy
        # Just output the function and return
        # Note this operates on prevTree because subTree went one too far
        if not branches:
            methodSpec = prevTree.tree.values()[0][1]
            indent(file, nesting+2, 'return ')
            methodSpec.outputOverloadedCall(file, prevTree.classTypeDesc, levelCopy)
            return

        # Ok, We must have a branch down here somewhere
        # Make a copy of the keys so we can sort them in place
        sortedKeys = self.tree.keys()
        # Sort the keys based on inheritance hierarchy, most specific classes first
        sortedKeys.sort(subclass)

        for i in range(len(sortedKeys)):
            typeDesc = sortedKeys[i]
            # See if this takes no arguments
            if (typeDesc == 0):
                # Output the function
                methodSpec = self.tree[0][1]
                indent(file, nesting+2, 'return ')
                methodSpec.outputOverloadedCall(file, self.classTypeDesc, 0)
            else:
                # This is handled at the top of the file now (?)
                # Import a file if we need to for this typeDesc
                # if ((typeDesc != 0) and
                #     (not typeDesc.isNested) and
                #     # Do not put our own module in the import list
                #     (self.classTypeDesc != typeDesc) and
                #     # If this is a class (not a primitive), put it on the list
                #     (typeDesc.__class__ == FFITypes.ClassTypeDescriptor)):
                #     indent(file, nesting+2, 'import ' + typeDesc.foreignTypeName + '\n')

                # Specify that at least one of these trees had arguments
                # so we know to output an else clause
                oneTreeHasArgs = 1
                typeName = getTypeName(self.classTypeDesc, typeDesc)
                typeNameList.append(typeName)
                if typeDesc.__class__ == FFITypes.PyObjectTypeDescriptor:
                    # A special case: if one of the parameters is
                    # PyObject *, that means anything is accepted.
                    condition = '1'

                else:
                    # Otherwise, we'll check the particular type of
                    # the object.
                    condition = '(isinstance(_args[' + `level` + '], ' + typeName + '))'
                    # Legal types for a float parameter include int and long.
                    if (typeName == 'FloatType'):
                        condition += (' or (isinstance(_args[' + `level` + '], IntType))')
                        condition += (' or (isinstance(_args[' + `level` + '], LongType))')
                    # Legal types for a long parameter include int.
                    elif (typeName == 'LongType'):
                        condition += (' or (isinstance(_args[' + `level` + '], IntType))')
                    # Legal types for an int parameter include long.
                    elif (typeName == 'IntType'):
                        condition += (' or (isinstance(_args[' + `level` + '], LongType))')

                indent(file, nesting+2, 'if ' + condition + ':\n')

                if (self.tree[typeDesc][0] is not None):
                    self.tree[typeDesc][0].traverse(file, nesting+1, level+1)
                else:
                    methodSpec = self.tree[typeDesc][1]
                    indent(file, nesting+3, 'return ')
                    numArgs = level+1
                    methodSpec.outputOverloadedCall(file, self.classTypeDesc, numArgs)

        # Output an else clause if one of the trees had arguments
        if oneTreeHasArgs:
            indent(file, nesting+2, "raise TypeError, 'Invalid argument " + `level` + ", expected one of: ")
            for name in typeNameList:
                indent(file, 0, ('<' + name + '> '))
            indent(file, 0, "'\n")

    def isSinglePath(self):
        if (len(self.tree.keys()) > 1):
            # More than one child, return false
            return 0
        else:
            # Only have one child, see if he only has one child
            key = self.tree.keys()[0]
            tree = self.tree[key][0]
            if tree:
                return tree.isSinglePath()
            else:
                return self.tree[key][1]

