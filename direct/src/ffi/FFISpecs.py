
import FFIConstants
import FFITypes
import FFIOverload
import string

from direct.showbase.PythonUtil import *

augmentedAssignments = ['__iadd__', '__isub__', '__imul__', '__idiv__',
                        '__ior__', '__iand__', '__ixor__',
                        '__ilshift__', '__irshift__']

class FunctionSpecification:
    def __init__(self):
        self.name = ''
        self.typeDescriptor = None
        self.index = 0
        self.overloaded = 0
        # Is this function a constructor
        self.constructor = 0

    def isConstructor(self):
        return self.constructor

    def isStatic(self):
        for arg in self.typeDescriptor.argumentTypes:
            if arg.isThis:
                return 0
        # No args were this pointers, must be static
        return 1

    def outputTypeChecking(self, methodClass, args, file, nesting):
        """
        Output an assert statement to check the type of each arg in this method
        This can be turned off with a command line parameter in generatePythonCode
        It is valid to pass in None for methodClass if you are not in any methodClass
        """
        if FFIConstants.wantTypeChecking:
            for i in range(len(args)):
                methodArgSpec = args[i]
                typeDesc = methodArgSpec.typeDescriptor.recursiveTypeDescriptor()
                typeName = FFIOverload.getTypeName(methodClass, typeDesc)

                # We only do type checking on class types.  C++ can do
                # type checking on the primitive types, and will do a
                # better job anyway.
                if typeDesc.__class__ == FFITypes.ClassTypeDescriptor:
                    # Get the real return type (not derived)
                    if ((not typeDesc.isNested) and
                        # Do not put our own module in the import list
                        (methodClass != typeDesc)):
                        indent(file, nesting, 'import ' + typeDesc.foreignTypeName + '\n')
                    indent(file, nesting, 'if not isinstance(' +
                           methodArgSpec.name + ', ' + typeName + '):\n')
                    indent(file, nesting + 1,
                           'raise TypeError, "Invalid argument %s, expected <%s>"\n' % (i, typeDesc.foreignTypeName))

    def outputCFunctionComment(self, file, nesting):
        """
        Output a docstring to the file describing the C++ call with type info
        Also output the C++ comment from interrogate.
        """
        if FFIConstants.wantComments:
            indent(file, nesting, '"""\n')

            # Output the function prototype
            if self.typeDescriptor.prototype:
                indent(file, nesting, self.typeDescriptor.prototype + '\n')

            # Output the function comment
            if self.typeDescriptor.comment:
                # To insert tabs into the comment, replace all newlines with a newline+tabs
                comment = string.replace(self.typeDescriptor.comment,
                                         '\n', ('\n' + ('    ' * nesting)))
                indent(file, nesting, comment)

            indent(file, 0, '\n')
            indent(file, nesting, '"""\n')

    def getFinalName(self):
        """
        Return the name of the function given that it might be overloaded
        If it is overloaded, prepend "overloaded", then append the types of
        each argument to make it unique.

        So "getChild(int)" becomes "overloaded_getChild_int(int)"
        """
        if self.overloaded:
            name = 'private__overloaded_' + self.name
            for methodArgSpec in self.typeDescriptor.argumentTypes:
                name = name + '_' + methodArgSpec.typeDescriptor.foreignTypeName
            return name
        else:
            return self.name

    def outputOverloadedCall(self, file, classTypeDesc, numArgs):
        """
        Write the function call to call this overloaded method
        For example:
          self.overloaded_setPos_ptrNodePath_float_float_float(*_args)
        If it is a class (static) method, call the class method
          Class.overloaded_setPos_ptrNodePath_float_float_float(*_args)

        Constructors are not treated as static. They are special because
        they are not really constructors, they are instance methods that fill
        in the this pointer.

        These do not get indented because they are not the beginning of the line

        If classTypeDesc is None, then this is a global function and should
        output code as such

        """
        if classTypeDesc:
            if (self.isStatic() and not self.isConstructor()):
                if numArgs:
                    indent(file, 0, classTypeDesc.foreignTypeName + '.' + self.getFinalName() + '(*_args)\n')
                else:
                    indent(file, 0, classTypeDesc.foreignTypeName + '.' + self.getFinalName() + '()\n')
            else:
                if numArgs:
                    indent(file, 0, 'self.' + self.getFinalName() + '(*_args)\n')
                else:
                    indent(file, 0, 'self.' + self.getFinalName() + '()\n')
        else:
            if numArgs:
                indent(file, 0, self.getFinalName() + '(*_args)\n')
            else:
                indent(file, 0, self.getFinalName() + '()\n')


class GlobalFunctionSpecification(FunctionSpecification):
    def __init__(self):
        FunctionSpecification.__init__(self)

    # Use generateCode when creating a global (non-class) function
    def generateGlobalCode(self, file):
        self.outputHeader(file)
        self.outputBody(file)
        self.outputFooter(file)

    # Use generateCode when creating a global (non-class) function
    def generateGlobalDowncastCode(self, file):
        self.outputHeader(file)
        self.outputBody(file, 0, 0) # no downcast, no type checking
        self.outputFooter(file)

    # Use generateMethodCode when creating a global->class function
    def generateMethodCode(self, methodClass, file, nesting):
        self.outputMethodHeader(methodClass, file, nesting)
        self.outputMethodBody(methodClass, file, nesting)
        self.outputMethodFooter(methodClass, file, nesting)

    ##################################################
    ## Global Function Code Generation
    ##################################################
    def outputHeader(self, file):
        argTypes = self.typeDescriptor.argumentTypes
        indent(file, 0, 'def ' + self.getFinalName() + '(')
        for i in range(len(argTypes)):
            file.write(argTypes[i].name)
            if (i < (len(argTypes)-1)):
                file.write(', ')
        file.write('):\n')

    def outputBody(self, file, needsDowncast=1, typeChecking=1):
        # The method body will look something like
        #     returnValue = PandaGlobal.method(arg)
        #     returnObject = NodePath()
        #     returnObject.this = returnValue
        #     returnObject.userManagesMemory = 1  (optional)
        #     return returnObject
        self.outputCFunctionComment(file, 1)
        argTypes = self.typeDescriptor.argumentTypes
        # The global downcast functions do not need type checking, and
        # in fact have a problem where they assert you are downcasting
        # from the immediate superclass when in fact you may be downcasting
        # from a class way up the chain as long as their is single
        # inheritance up to it
        if typeChecking:
            self.outputTypeChecking(None, argTypes, file, 1)
        indent(file, 1, 'returnValue = ' + self.typeDescriptor.moduleName
                   + '.' + self.typeDescriptor.wrapperName + '(')
        for i in range(len(argTypes)):
            file.write(argTypes[i].passName())
            if (i < (len(argTypes)-1)):
                file.write(', ')
        file.write(')\n')
        #indent(file, 1, 'if returnValue is None:\n')
        #indent(file, 2, 'return None\n')
        returnType = self.typeDescriptor.returnType.recursiveTypeDescriptor()
        returnType.generateReturnValueWrapper(None, file,
                                              self.typeDescriptor.userManagesMemory,
                                              needsDowncast, 1)

    def outputFooter(self, file):
        indent(file, 0, '\n')

    ##################################################
    ## Class Method Code Generation
    ##################################################
    def outputMethodHeader(self, methodClass, file, nesting):
        argTypes = self.typeDescriptor.argumentTypes
        indent(file, nesting, 'def ' + self.getFinalName() + '(')
        for i in range(len(argTypes)):
            # Instead of the first argument, put self
            if (i == 0):
                file.write('self')
            else:
                file.write(argTypes[i].name)
            if (i < (len(argTypes)-1)):
                file.write(', ')
        file.write('):\n')

    def outputMethodBody(self, methodClass, file, nesting):
        # The method body will look something like
        #     returnValue = PandaGlobal.method(self.this, arg)
        #     returnValue.userManagesMemory = 1  (optional)
        #     return returnValue
        self.outputCFunctionComment(file, nesting+2)
        argTypes = self.typeDescriptor.argumentTypes
        self.outputTypeChecking(methodClass, argTypes[1:], file, nesting+2)
        indent(file, nesting+2, 'returnValue = ' + self.typeDescriptor.moduleName
                   + '.' + self.typeDescriptor.wrapperName + '(')
        for i in range(len(argTypes)):
            # Instead of the first argument, put self.this
            if (i == 0):
                file.write('self.this')
            else:
                file.write(argTypes[i].passName())
            if (i < (len(argTypes)-1)):
                file.write(', ')
        file.write(')\n')
        indent(file, 1, 'if returnValue is None:\n')
        indent(file, 2, 'return None\n')

        returnType = self.typeDescriptor.returnType.recursiveTypeDescriptor()
        returnType.generateReturnValueWrapper(methodClass, file,
                                              self.typeDescriptor.userManagesMemory,
                                              1, nesting+2)

    def outputMethodFooter(self, methodClass, file, nesting):
        indent(file, nesting+1, '\n')


class MethodSpecification(FunctionSpecification):
    def __init__(self):
        FunctionSpecification.__init__(self)

    def generateConstructorCode(self, methodClass, file, nesting):
        self.outputConstructorHeader(methodClass, file, nesting)
        self.outputConstructorBody(methodClass, file, nesting)
        self.outputConstructorFooter(methodClass, file, nesting)

    def generateDestructorCode(self, methodClass, file, nesting):
        self.outputDestructorHeader(methodClass, file, nesting)
        self.outputDestructorBody(methodClass, file, nesting)
        self.outputDestructorFooter(methodClass, file, nesting)

    def generateMethodCode(self, methodClass, file, nesting):
        self.outputMethodHeader(methodClass, file, nesting)
        self.outputMethodBody(methodClass, file, nesting)
        self.outputMethodFooter(methodClass, file, nesting)

    def generateStaticCode(self, methodClass, file, nesting):
        self.outputStaticHeader(methodClass, file, nesting)
        self.outputStaticBody(methodClass, file, nesting)
        self.outputStaticFooter(methodClass, file, nesting)

    def generateInheritedMethodCode(self, methodClass, parentList, file, nesting, needsDowncast):
        self.outputInheritedMethodHeader(methodClass, parentList, file, nesting, needsDowncast)
        self.outputInheritedMethodBody(methodClass, parentList, file, nesting, needsDowncast)
        self.outputInheritedMethodFooter(methodClass, parentList, file, nesting, needsDowncast)

    def generateUpcastMethodCode(self, methodClass, file, nesting):
        # The upcast method code is just like regular code, but the
        # return value wrapper does not have downcasting instructions
        self.outputMethodHeader(methodClass, file, nesting)
        self.outputMethodBody(methodClass, file, nesting, 0) # no downcast
        self.outputMethodFooter(methodClass, file, nesting)

    ##################################################
    ## Constructor Code Generation
    ##################################################
    def outputConstructorHeader(self, methodClass, file, nesting):
        argTypes = self.typeDescriptor.argumentTypes
        thislessArgTypes = self.typeDescriptor.thislessArgTypes()
        indent(file, nesting+1, 'def ' + self.getFinalName() + '(self')
        if (len(thislessArgTypes) > 0):
            file.write(', ')
            for i in range(len(thislessArgTypes)):
                file.write(thislessArgTypes[i].name)
                if (i < (len(thislessArgTypes)-1)):
                    file.write(', ')
        file.write('):\n')
        self.outputCFunctionComment(file, nesting+2)


    def outputConstructorBody(self, methodClass, file, nesting):
        # The method body will look something like
        #     self.this = panda.Class_constructor(arg)
        #     self.userManagesMemory = 1  (optional)
        argTypes = self.typeDescriptor.argumentTypes
        thislessArgTypes = self.typeDescriptor.thislessArgTypes()
        self.outputTypeChecking(methodClass, thislessArgTypes, file, nesting+2)
        indent(file, nesting+2, 'self.this = ' + self.typeDescriptor.moduleName + '.'
                   + self.typeDescriptor.wrapperName + '(')
        # Do not pass self into the constructor
        for i in range(len(thislessArgTypes)):
            file.write(thislessArgTypes[i].passName())
            if (i < (len(thislessArgTypes)-1)):
                file.write(', ')
        file.write(')\n')
        indent(file, nesting+2, 'assert self.this != 0\n')
        if self.typeDescriptor.userManagesMemory:
            indent(file, nesting+2, 'self.userManagesMemory = 1\n')

    def outputConstructorFooter(self, methodClass, file, nesting):
        indent(file, nesting+1, '\n')


    ##################################################
    ## Destructor Code Generation
    ##################################################
    def outputDestructorHeader(self, methodClass, file, nesting):
        argTypes = self.typeDescriptor.argumentTypes
        thislessArgTypes = self.typeDescriptor.thislessArgTypes()
        indent(file, nesting+1, 'def ' + self.getFinalName() + '(self')
        if (len(thislessArgTypes) > 0):
            file.write(', ')
            for i in range(len(thislessArgTypes)):
                file.write(thislessArgTypes[i].name)
                if (i < (len(thislessArgTypes)-1)):
                    file.write(', ')
        file.write('):\n')
        self.outputCFunctionComment(file, nesting+2)

    def outputDestructorBody(self, methodClass, file, nesting):
        # The method body will look something like
        #     panda.Class_destructor(self.this)
        functionName = (self.typeDescriptor.moduleName + '.'
                        + self.typeDescriptor.wrapperName)
        # Make sure the module and function have not been deleted first
        # This only happens during shutdown
        indent(file, nesting+2, 'if (' + self.typeDescriptor.moduleName + ' and ' +
               functionName + '):\n')
        indent(file, nesting+3, functionName + '(self.this)\n')

    def outputDestructorFooter(self, methodClass, file, nesting):
        indent(file, nesting+1, '\n')

    ##################################################
    ## Method Code Generation
    ##################################################
    def outputMethodHeader(self, methodClass, file, nesting):
        argTypes = self.typeDescriptor.argumentTypes
        thislessArgTypes = self.typeDescriptor.thislessArgTypes()
        indent(file, nesting, 'def ' + self.getFinalName() + '(self')
        if (len(thislessArgTypes) > 0):
            file.write(', ')
            for i in range(len(thislessArgTypes)):
                file.write(thislessArgTypes[i].name)
                if (i < (len(thislessArgTypes)-1)):
                    file.write(', ')
        file.write('):\n')

    def outputMethodBody(self, methodClass, file, nesting, needsDowncast=1):
        # The method body will look something like
        #     returnValue = panda.Class_method(self.this, arg)
        #     returnValue.userManagesMemory = 1  (optional)
        #     return returnValue
        self.outputCFunctionComment(file, nesting+2)
        argTypes = self.typeDescriptor.argumentTypes
        thislessArgTypes = self.typeDescriptor.thislessArgTypes()
        self.outputTypeChecking(methodClass, thislessArgTypes, file, nesting+2)
        indent(file, nesting+2, 'returnValue = ' + self.typeDescriptor.moduleName + '.'
                   + self.typeDescriptor.wrapperName + '(')
        file.write('self.this')
        if (len(thislessArgTypes) > 0):
            file.write(', ')
            for i in range(len(thislessArgTypes)):
                file.write(thislessArgTypes[i].passName())
                if (i < (len(thislessArgTypes)-1)):
                    file.write(', ')
        file.write(')\n')
        # If this is an augmented assignment operator like +=, we have special rules
        # In this case we simply call the C++ function, make sure we got the same
        # return value back, then return self. Otherwise if you let it go through the
        # normal system, it actually deletes the old Python object causing the C++ memory
        # to be deleted then returns a new Python shadow object with the old C++ pointer... BAD!
        if self.getFinalName() in augmentedAssignments:
            indent(file, nesting+2, 'assert self.this == returnValue\n')
            indent(file, nesting+2, 'return self\n')
        else:
            returnType = self.typeDescriptor.returnType.recursiveTypeDescriptor()
            returnType.generateReturnValueWrapper(methodClass, file,
                                                  self.typeDescriptor.userManagesMemory,
                                                  needsDowncast, nesting+2)

    def outputMethodFooter(self, methodClass, file, nesting):
        indent(file, nesting,  'FFIExternalObject.funcToMethod(' +self.getFinalName()+ ',' + methodClass.foreignTypeName + ",'" +self.getFinalName() +"')\n")
        indent(file, nesting,  'del ' + self.getFinalName()+' \n')
        indent(file, nesting+1,'\n')
        #indent(file, nesting, methodClass.foreignTypeName +'.'+  self.getFinalName() + ' = staticmethod(' + self.getFinalName() + ')\n')
        #indent(file, nesting,'del ' + self.getFinalName()+' \n')
        #indent(file, nesting+1, '\n')
        indent(file, nesting+1, '\n')


    ##################################################
    ## Static Method Code Generation
    ##################################################
    def outputStaticHeader(self, methodClass, file, nesting):
        argTypes = self.typeDescriptor.argumentTypes
        indent(file, nesting, 'def ' + self.getFinalName() + '(')
        for i in range(len(argTypes)):
            file.write(argTypes[i].name)
            if (i < (len(argTypes)-1)):
                    file.write(', ')
        file.write('):\n')


    def outputStaticBody(self, methodClass, file, nesting):
        # The method body will look something like
        #     returnValue = panda.class_method(self.this, arg)
        #     returnValue.userManagesMemory = 1  (optional)
        #     return returnValue
        self.outputCFunctionComment(file, nesting+2)
        argTypes = self.typeDescriptor.argumentTypes
        thislessArgTypes = self.typeDescriptor.thislessArgTypes()
        self.outputTypeChecking(methodClass, thislessArgTypes, file, nesting+2)
        indent(file, nesting+2, 'returnValue = ' + self.typeDescriptor.moduleName + '.'
                   + self.typeDescriptor.wrapperName + '(')
        # Static methods do not take the this parameter
        if (len(thislessArgTypes) > 0):
            for i in range(len(thislessArgTypes)):
                file.write(thislessArgTypes[i].passName())
                if (i < (len(thislessArgTypes)-1)):
                    file.write(', ')
        file.write(')\n')
        returnType = self.typeDescriptor.returnType.recursiveTypeDescriptor()
        returnType.generateReturnValueWrapper(methodClass, file,
                                              self.typeDescriptor.userManagesMemory,
                                              1, nesting+2)

    def outputStaticFooter(self, methodClass, file, nesting):
        indent(file, nesting, methodClass.foreignTypeName +'.'+  self.getFinalName() + ' = staticmethod(' + self.getFinalName() + ')\n')
        indent(file, nesting,'del ' + self.getFinalName()+' \n')
        indent(file, nesting+1, '\n')

    ##################################################
    ## Upcast Method Code Generation
    ##################################################
    def outputInheritedMethodHeader(self, methodClass, parentList, file, nesting, needsDowncast):
        argTypes = self.typeDescriptor.argumentTypes
        thislessArgTypes = self.typeDescriptor.thislessArgTypes()
        indent(file, nesting, 'def ' + self.getFinalName() + '(self')
        if (len(thislessArgTypes) > 0):
            file.write(', ')
            for i in range(len(thislessArgTypes)):
                file.write(thislessArgTypes[i].name)
                if (i < (len(thislessArgTypes)-1)):
                    file.write(', ')
        file.write('):\n')

    def outputInheritedMethodBody(self, methodClass, parentList, file, nesting, needsDowncast):
        # The method body will look something like
        #     upcastSelf = self.upcastToParentClass()
        #     returnValue = libpanda.method(upcastSelf.this, arg)
        #     returnValue.userManagesMemory = 1  (optional)
        #     return returnValue
        self.outputCFunctionComment(file, nesting+2)
        argTypes = self.typeDescriptor.argumentTypes
        thislessArgTypes = self.typeDescriptor.thislessArgTypes()
        self.outputTypeChecking(methodClass, thislessArgTypes, file, nesting+2)
        indent(file, nesting+2, 'upcastSelf = self\n')
        for i in range(len(parentList)):
            # Only output the upcast call if that parent class defines it
            parentClass = parentList[i]
            methodName = 'upcastTo' + parentClass.foreignTypeName
            if (i != 0):
                childClass = parentList[i-1]
                if childClass.hasMethodNamed(methodName):
                    indent(file, nesting+2, 'upcastSelf = upcastSelf.' + methodName + '()\n')
                else:
                    indent(file, nesting+2, '# upcastSelf = upcastSelf.' + methodName + '()\n')
            else:
                if methodClass.hasMethodNamed(methodName):
                    indent(file, nesting+2, 'upcastSelf = upcastSelf.' + methodName + '()\n')
                else:
                    indent(file, nesting+2, '# upcastSelf = upcastSelf.' + methodName + '()\n')

        indent(file, nesting+2, 'returnValue = ' + self.typeDescriptor.moduleName
               + '.' + self.typeDescriptor.wrapperName + '(upcastSelf.this')
        if (len(thislessArgTypes) > 0):
            file.write(', ')
            for i in range(len(thislessArgTypes)):
                file.write(thislessArgTypes[i].passName())
                if (i < (len(thislessArgTypes)-1)):
                    file.write(', ')
        file.write(')\n')
        returnType = self.typeDescriptor.returnType.recursiveTypeDescriptor()
        # Generate the return value code with no downcast instructions
        returnType.generateReturnValueWrapper(methodClass, file,
                                              self.typeDescriptor.userManagesMemory,
                                              needsDowncast, nesting+2)

    def outputInheritedMethodFooter(self, methodClass, parentList, file, nesting, needsDowncast):
        indent(file, nesting,  'FFIExternalObject.funcToMethod(' +self.getFinalName()+ ',' + methodClass.foreignTypeName + ",'" +self.getFinalName() +"')\n")
        indent(file, nesting,  'del ' + self.getFinalName()+' \n')
        indent(file, nesting+1,'\n')


class GlobalValueSpecification:
    def __init__(self):
        self.name = ''
        # We really do not need the descriptor for the value, just
        # the getter and setter
        # self.typeDescriptor = None
        # To be filled in with a GlobalFunctionSpecification
        self.getter = None
        # To be filled in with a GlobalFunctionSpecification
        self.setter = None

    def generateGlobalCode(self, file):
        indent(file, 0, '# Global value: ' + self.name + '\n')
        if self.getter:
            self.getter.generateGlobalCode(file)
        if self.setter:
            self.setter.generateGlobalCode(file)
        indent(file, 0, '\n')


# Manifest symbols
class ManifestSpecification:
    def __init__(self):
        self.name = ''

        # We are not currently using the type descriptor
        self.typeDescriptor = None

        # To be filled in with a GlobalFunctionSpecification
        # if this manifest has one
        self.getter = None

        # Manifests that have int values have their int value defined
        # instead of having to call a getter (because there are so many of them)
        self.intValue = None

        # The string definition of this manifest
        self.definition = None

    def generateGlobalCode(self, file):
        # Note, if the manifest has no value and no getter we do not output anything
        # even though they may be defined in the C++ sense. Without any values
        # they are pretty useless in Python

        # If it has an int value, just output that instead of bothering
        # with a getter
        if (self.intValue != None):
            indent(file, 0, '# Manifest: ' + self.name + '\n')
            indent(file, 0, (self.name + ' = ' + `self.intValue` + '\n'))
            indent(file, 0, '\n')

        elif self.definition:
            indent(file, 0, ('# Manifest: ' + self.name + ' definition: ' +
                             self.definition + '\n'))
            # Out put the getter
            if self.getter:
                self.getter.generateGlobalCode(file)
            indent(file, 0, '\n')


class MethodArgumentSpecification:
    def __init__(self):
        self.name = ''
        self.typeDescriptor = None
        # By default it is not the this pointer
        self.isThis = 0

    def passName(self):
        if (self.typeDescriptor.recursiveTypeDescriptor().__class__ == \
            FFITypes.ClassTypeDescriptor):
            return self.name + '.this'
        else:
            return self.name
