
# Note: do not import this file directly, it is meant to be used as part of
# a Python script (generatePythonCode) that sets up variables that this
# module depends on

import string
import os
import compileall

import FFIEnvironment
import FFITypes
import FFISpecs
import FFIRename
import FFIConstants
import FFIOverload


# FFIConstants.notify.setDebug(1)
FFIConstants.notify.info('Importing interrogate library: ' + FFIConstants.InterrogateModuleName)
# Note: we do a from lib import * here because we do not want
# to be dependent on the name of the interrogate library in this code
exec('from ' + FFIConstants.InterrogateModuleName + ' import *')

# Import all the C++ modules
for CModuleName in FFIConstants.CodeModuleNameList:
    FFIConstants.notify.info('Importing code library: ' + CModuleName)
    exec('import ' + CModuleName)

def constructGlobalFile(codeDir):
    """
    Open a file that will hold the global values and functions code
    """
    file = open(os.path.join(codeDir, FFIConstants.globalModuleName + '.py'), 'w')
    return file

def constructImportFile(codeDir):
    """
    Open a file that will hold the global values and functions code
    """
    file = open(os.path.join(codeDir, FFIConstants.importModuleName + '.py'), 'w')
    return file

def outputGlobalFileImports(file, methodList):
    # Print the standard header
    file.write(FFIConstants.generatedHeader)

    # Import Python's builtin types
    file.write('import types\n')

    # Import the C modules
    for CModuleName in FFIConstants.CodeModuleNameList:
        file.write('import ' + CModuleName + '\n')

    moduleList = []
    for method in methodList:
        returnType = method.typeDescriptor.returnType.recursiveTypeDescriptor()
        if (not (returnType.foreignTypeName in moduleList)):
            if (returnType.__class__ == FFITypes.ClassTypeDescriptor):
                moduleList.append(returnType.foreignTypeName)
    
    for moduleName in moduleList:
        file.write('import ' + moduleName + '\n')
    
    file.write('\n')


def outputImportFileImports(file, typeList):
    """
    This is the file that we will import to get all the panda modules
    """
    
    # Print the standard header
    file.write(FFIConstants.generatedHeader)
    
    file.write('# Import the interrogate module\n')
    file.write('import ' + FFIConstants.InterrogateModuleName + '\n')
    file.write('\n')
    
    file.write('# Import the C modules\n')
    for CModuleName in FFIConstants.CodeModuleNameList:
        file.write('import ' + CModuleName + '\n')
    file.write('\n')

    # Filter out only the class and enum type descriptors (not const, pointers, etc)
    classTypeList = []
    enumTypeList = []
    for type in typeList:
        if (type.__class__ == FFITypes.ClassTypeDescriptor):
            if (not type.isNested):
                classTypeList.append(type)
        elif (type.__class__ == FFITypes.EnumTypeDescriptor):
            if (not type.isNested):
                enumTypeList.append(type)
            
    # Sort the types based on inheritance, most generic first
    classTypeList.sort(FFIOverload.inheritanceLevelSort)

    moduleList = []
    for type in classTypeList:    
        moduleList.append(type.foreignTypeName)

    file.write('import FFIExternalObject\n')
    file.write('\n')

    file.write('# Import enums into the global name space\n')
    for type in enumTypeList:
        file.write('from ' + type.enumName + ' import *\n')
    file.write('\n')

    file.write('# Import classes\n')
    for moduleName in moduleList:
        file.write('import ' + moduleName + '\n')    
    file.write('\n')

    file.write('# Import the global module file into our name space\n')
    file.write('from ' + FFIConstants.globalModuleName + ' import *\n')
    file.write('\n')

    file.write('# Now generate the classes\n')
    for moduleName in moduleList:
        file.write(moduleName + '.generateClass_' + moduleName + '()\n')
    file.write('\n')
        
    file.write('# Now put the classes in the wrapper class map\n')
    for moduleName in moduleList:
        file.write('obj = ' + moduleName + '.' + moduleName + '(None)\n')
        file.write('obj.registerInTypeMap()\n')
    file.write('\n')

    file.write('# Now copy the classes into our own namespace\n')
    for moduleName in moduleList:
        file.write(moduleName + ' = ' + moduleName + '.' + moduleName + '\n')
    file.write('\n')

def generateStaticClass(codeDir):
    """
    Create a file that will hold the static class definition
    """
    file = open(os.path.join(codeDir, FFIConstants.staticModuleName + '.py'), 'w')
    # Print the standard header
    file.write(FFIConstants.generatedHeader)
    file.write('class ' + FFIConstants.staticModuleName + ':\n')
    file.write('    def __init__(self, function):\n')
    file.write('        self.__call__ = function\n')
    file.close()
    return file

def getTypeName(typeIndex, scoped=0):
    """
    Return a fully specified type name for this type index
    Return the scoped name if asked for it
    """
    nameComponents = []
    name = ''

    
    if scoped:
        typeName = interrogate_type_scoped_name(typeIndex)
    else:        
        typeName = interrogate_type_name(typeIndex)

    if typeIndex == 0:
        FFIConstants.notify.debug('typeIndex 0: ' + typeName)
        
    if interrogate_type_is_wrapped(typeIndex):
        typeName = getTypeName(interrogate_type_wrapped_type(typeIndex))
    if interrogate_type_is_const(typeIndex):
        nameComponents.append('const')
    if interrogate_type_is_pointer(typeIndex):
        nameComponents.append('ptr')
    if interrogate_type_is_signed(typeIndex):
        # signed is now built into the type name
        #nameComponents.append('signed')
        pass
    if interrogate_type_is_unsigned(typeIndex):
        # unsigned is now built into the type name
        #nameComponents.append('unsigned')
        pass
    if interrogate_type_is_long(typeIndex):
        nameComponents.append('long')
    if interrogate_type_is_longlong(typeIndex):
        nameComponents.append('longLong')
    if interrogate_type_is_short(typeIndex):
        nameComponents.append('short')
    if (len(nameComponents) > 0):
        typeName = string.capitalize(typeName[0]) + typeName[1:]
    nameComponents.append(typeName)
    for i in range(len(nameComponents)):
        if (i == 0):
            name = name + nameComponents[i]
        else:
            name = name + string.capitalize(nameComponents[i][0]) + nameComponents[i][1:]

    FFIConstants.notify.debug('typeIndex: ' + `typeIndex` + ' typeName: ' + typeName + ' has name: ' + name)

    if not name:
        FFIConstants.notify.warning('typeIndex: ' + `typeIndex` + ' typeName: ' + typeName + ' has no name')

    return name


class FFIInterrogateDatabase:

    def __init__(self):
        self.typeIndexMap = {}
        self.environment = FFIEnvironment.FFIEnvironment()

    def isDefinedType(self, typeIndex):
        return self.typeIndexMap.has_key(typeIndex)
    
    def constructDescriptor(self, typeIndex):
        if interrogate_type_is_atomic(typeIndex):
            return self.constructPrimitiveTypeDescriptor(typeIndex)
        
        elif interrogate_type_is_enum(typeIndex):
            return self.constructEnumTypeDescriptor(typeIndex)
        
        elif interrogate_type_is_wrapped(typeIndex):
            if interrogate_type_is_pointer(typeIndex):
                return self.constructPointerTypeDescriptor(typeIndex)
            elif interrogate_type_is_const(typeIndex):
                return self.constructConstTypeDescriptor(typeIndex)
        
        elif (interrogate_type_is_class(typeIndex) or
              interrogate_type_is_struct(typeIndex) or
              interrogate_type_is_union(typeIndex)):
            return self.constructClassTypeDescriptor(typeIndex)

        elif (not interrogate_type_is_fully_defined(typeIndex)):
            return  self.constructClassTypeDescriptor(typeIndex)
        
        else:
            raise 'A type in the interrogate database was not recognized: '+ `typeIndex`
    
    def constructPrimitiveTypeDescriptor(self, typeIndex):
        if self.isDefinedType(typeIndex):
            return self.typeIndexMap[typeIndex]
        else:
            descriptor = FFITypes.PrimitiveTypeDescriptor()
            #descriptor.environment = self.environment
            descriptor.atomicType = interrogate_type_atomic_token(typeIndex)
            descriptor.foreignTypeName = \
                FFIRename.nonClassNameFromCppName(getTypeName(typeIndex))
            descriptor.typeIndex = typeIndex
            self.typeIndexMap[typeIndex] = descriptor
            return descriptor
    
    def constructEnumTypeDescriptor(self, typeIndex):
        if self.isDefinedType(typeIndex):
            return self.typeIndexMap[typeIndex]
        else:
            descriptor = FFITypes.EnumTypeDescriptor()
            #descriptor.environment = self.environment
            descriptor.isNested = interrogate_type_is_nested(typeIndex)
            if descriptor.isNested:
                outerTypeIndex = interrogate_type_outer_class(typeIndex)
                descriptor.outerType = self.constructDescriptor(outerTypeIndex)
            # Enums are ints in C++ but we do not want to redefine the int type
            # So we will just call them enums
            descriptor.enumName = FFIRename.classNameFromCppName(getTypeName(typeIndex))
            descriptor.foreignTypeName = '__enum__' + descriptor.enumName
            numValues = interrogate_type_number_of_enum_values(typeIndex)

            # Store the names and values of the enum in a dictionary
            for i in range(numValues):
                value = interrogate_type_enum_value(typeIndex, i)
                name = FFIRename.classNameFromCppName(
                    interrogate_type_enum_value_name(typeIndex, i))
                scopedName = FFIRename.classNameFromCppName(
                    interrogate_type_enum_value_scoped_name(typeIndex, i))
                descriptor.values[name] = value
            
            descriptor.typeIndex = typeIndex
            self.typeIndexMap[typeIndex] = descriptor
            return descriptor

    def constructPointerTypeDescriptor(self, typeIndex):
        if self.isDefinedType(typeIndex):
            return self.typeIndexMap[typeIndex]
        descriptor = FFITypes.PointerTypeDescriptor()
        #descriptor.environment = self.environment
        descriptor.isNested = interrogate_type_is_nested(typeIndex)
        if descriptor.isNested:
            outerTypeIndex = interrogate_type_outer_class(typeIndex)
            descriptor.outerType = self.constructDescriptor(outerTypeIndex)
        descriptor.foreignTypeName = \
             FFIRename.nonClassNameFromCppName(getTypeName(typeIndex))
        descriptor.typeIndex = typeIndex
        wrappedTypeIndex = interrogate_type_wrapped_type(typeIndex)
        wrappedTypeDescriptor = self.constructDescriptor(wrappedTypeIndex)
        descriptor.typeDescriptor = wrappedTypeDescriptor
        self.typeIndexMap[typeIndex] = descriptor
        return descriptor
    
    def constructConstTypeDescriptor(self, typeIndex):
        if self.isDefinedType(typeIndex):
            return self.typeIndexMap[typeIndex]
        descriptor = FFITypes.ConstTypeDescriptor()
        #descriptor.environment = self.environment
        descriptor.isNested = interrogate_type_is_nested(typeIndex)
        if descriptor.isNested:
            outerTypeIndex = interrogate_type_outer_class(typeIndex)
            descriptor.outerType = self.constructDescriptor(outerTypeIndex)
        descriptor.foreignTypeName = \
             FFIRename.nonClassNameFromCppName(getTypeName(typeIndex))
        descriptor.typeIndex = typeIndex
        wrappedTypeIndex = interrogate_type_wrapped_type(typeIndex)
        wrappedTypeDescriptor = self.constructDescriptor(wrappedTypeIndex)
        descriptor.typeDescriptor = wrappedTypeDescriptor
        self.typeIndexMap[typeIndex] = descriptor
        return descriptor

    def constructParentTypeDescriptors(self, typeIndex):
        numParents = interrogate_type_number_of_derivations(typeIndex)
        descriptors = []
        for i in range(numParents):
            parentTypeIndex = interrogate_type_get_derivation(typeIndex, i)
            if self.isDefinedType(parentTypeIndex):
                parentTypeDescriptor = self.typeIndexMap[parentTypeIndex]
            else:
                parentTypeDescriptor = self.constructDescriptor(parentTypeIndex)
            descriptors.append(parentTypeDescriptor)
        return descriptors

    def constructNestedTypeDescriptors(self, typeIndex):
        nestedTypes = []
        numNestedTypes = interrogate_type_number_of_nested_types(typeIndex)
        for i in range(numNestedTypes):
            nestedTypeIndex = interrogate_type_get_nested_type(typeIndex, i)
            descriptor = self.constructDescriptor(nestedTypeIndex)
            nestedTypes.append(descriptor)
        return nestedTypes
    
    def constructClassTypeDescriptor(self, typeIndex):
        if self.isDefinedType(typeIndex):
            return self.typeIndexMap[typeIndex]
        descriptor = FFITypes.ClassTypeDescriptor()
        self.typeIndexMap[typeIndex] = descriptor
        #descriptor.environment = self.environment
        descriptor.isNested = interrogate_type_is_nested(typeIndex)
        if descriptor.isNested:
            outerTypeIndex = interrogate_type_outer_class(typeIndex)
            descriptor.outerType = self.constructDescriptor(outerTypeIndex)
        descriptor.foreignTypeName = FFIRename.classNameFromCppName(getTypeName(typeIndex))
        if FFIConstants.wantComments:
            if interrogate_type_has_comment(typeIndex):
                descriptor.comment = interrogate_type_comment(typeIndex)
        descriptor.typeIndex = typeIndex
        descriptor.instanceMethods = self.constructMemberFunctionSpecifications(typeIndex)
        descriptor.upcastMethods = self.constructUpcastFunctionSpecifications(typeIndex)
        # Constructing downcasts does not return the functions, it just puts them in the class
        # See the comment in that function
        self.constructDowncastFunctionSpecifications(typeIndex)
        descriptor.filterOutStaticMethods()
        descriptor.constructors = self.constructConstructorSpecifications(typeIndex)
        descriptor.destructor = self.constructDestructorSpecification(typeIndex)
        descriptor.parentTypes = self.constructParentTypeDescriptors(typeIndex)
        descriptor.nestedTypes = self.constructNestedTypeDescriptors(typeIndex)
        return descriptor

    def constructFunctionTypeDescriptors(self, functionIndex):

        # Store these values because they will be the same for all the wrappers
        isVirtual = interrogate_function_is_virtual(functionIndex)
        #environment = self.environment
        foreignTypeName = interrogate_function_name(functionIndex)
        if FFIConstants.wantComments:
            prototype = interrogate_function_prototype(functionIndex)
            if interrogate_function_has_comment(functionIndex):
                comment = interrogate_function_comment(functionIndex)
            else:
                comment = ''
        # Prepend lib to the module name it reports because that will be the name of
        # the Python module we import. This is apparently stems from a makefile
        # discrepency in the way we build the libraries
        if interrogate_function_has_module_name(functionIndex):
            moduleName = 'lib' + interrogate_function_module_name(functionIndex)
        else:
            moduleName = None
        typeIndex = functionIndex

        # Look at the Python wrappers for this function
        numPythonWrappers = interrogate_function_number_of_python_wrappers(functionIndex)
        
        if numPythonWrappers == 0:
            # If there are no Python wrappers, it is because interrogate could not handle
            # something about the function. Just return an empty list
            return []

        wrapperDescriptors = []

        # Iterate over the wrappers constructing a FunctionTypeDescriptor for each
        for i in range(numPythonWrappers):
            descriptor = FFITypes.FunctionTypeDescriptor()
            descriptor.isVirtual = isVirtual
            #descriptor.environment = environment
            descriptor.foreignTypeName = foreignTypeName
            if FFIConstants.wantComments:
                descriptor.comment = comment
                descriptor.prototype = prototype
            descriptor.moduleName = moduleName
            descriptor.typeIndex = typeIndex
            pythonFunctionIndex = interrogate_function_python_wrapper(functionIndex, i)
            descriptor.wrapperName = interrogate_wrapper_name(pythonFunctionIndex)
            # Even if it does not have a return value, it reports void which is better
            # for generating code, so I will not even ask here
            # if interrogate_wrapper_has_return_value(pythonFunctionIndex):
            returnType = interrogate_wrapper_return_type(pythonFunctionIndex)
            descriptor.returnType = self.constructDescriptor(returnType)
            descriptor.argumentTypes = self.constructFunctionArgumentTypes(pythonFunctionIndex)
            descriptor.userManagesMemory = interrogate_wrapper_caller_manages_return_value(pythonFunctionIndex)
            descriptor.returnValueDestructor = interrogate_wrapper_return_value_destructor(pythonFunctionIndex)
            wrapperDescriptors.append(descriptor)
            
        return wrapperDescriptors
    
    def constructFunctionArgumentTypes(self, functionIndex):
        numArgs = interrogate_wrapper_number_of_parameters(functionIndex)
        arguments = []
        for argIndex in range(numArgs):
            if interrogate_wrapper_parameter_has_name(functionIndex, argIndex):
                name =  FFIRename.nonClassNameFromCppName(
                    interrogate_wrapper_parameter_name(functionIndex, argIndex))
            else:
                name = ('parameter' + `argIndex`)
            descriptor = self.constructDescriptor(
                interrogate_wrapper_parameter_type(functionIndex, argIndex))
            
            argSpec = FFISpecs.MethodArgumentSpecification()
            if interrogate_wrapper_parameter_is_this(functionIndex, argIndex):
                argSpec.isThis = 1
            argSpec.name = name
            argSpec.typeDescriptor = descriptor
            arguments.append(argSpec)
        return arguments
        
    def constructMemberFunctionSpecifications(self, typeIndex):
        funcSpecs = []
        numFuncs = interrogate_type_number_of_methods(typeIndex)
        for i in range(numFuncs):
            funcIndex = interrogate_type_get_method(typeIndex, i)
            typeDescs = self.constructFunctionTypeDescriptors(funcIndex)
            for typeDesc in typeDescs:
                funcSpec = FFISpecs.MethodSpecification()
                funcSpec.name = FFIRename.methodNameFromCppName(
                    interrogate_function_name(funcIndex))
                funcSpec.typeDescriptor = typeDesc
                funcSpec.index = funcIndex
                funcSpecs.append(funcSpec)
        return funcSpecs

    def constructUpcastFunctionSpecifications(self, typeIndex):
        funcSpecs = []
        numFuncs = interrogate_type_number_of_derivations(typeIndex)
        for i in range(numFuncs):
            if interrogate_type_derivation_has_upcast(typeIndex, i):
                funcIndex = interrogate_type_get_upcast(typeIndex, i)
                typeDescs = self.constructFunctionTypeDescriptors(funcIndex)
                for typeDesc in typeDescs:
                    funcSpec = FFISpecs.MethodSpecification()
                    funcSpec.name = FFIRename.methodNameFromCppName(
                        interrogate_function_name(funcIndex))
                    funcSpec.typeDescriptor = typeDesc
                    funcSpec.index = funcIndex
                    funcSpecs.append(funcSpec)
        return funcSpecs
    
    def constructDowncastFunctionSpecifications(self, typeIndex):
        """
        The strange thing about downcast functions is that they appear in the
        class they are being downcast TO, not downcast FROM. But they should be
        built into the class they are being downcast from. For instance, a method
        downcastToNode(ptrBoundedObject) will appear in Node's list of methods
        but should be compiled into BoundedObject's class
        """
        numFuncs = interrogate_type_number_of_derivations(typeIndex)
        for i in range(numFuncs):
            # Make sure this downcast is possible
            if (not interrogate_type_derivation_downcast_is_impossible(typeIndex, i)):
                if interrogate_type_derivation_has_downcast(typeIndex, i):
                    funcIndex = interrogate_type_get_downcast(typeIndex, i)
                    typeDescs = self.constructFunctionTypeDescriptors(funcIndex)
                    for typeDesc in typeDescs:
                        funcSpec = FFISpecs.MethodSpecification()
                        funcSpec.name = FFIRename.methodNameFromCppName(
                            interrogate_function_name(funcIndex))
                        funcSpec.typeDescriptor = typeDesc
                        funcSpec.index = funcIndex
                        # Here we look for the class in the first argument
                        fromClass = typeDesc.argumentTypes[0].typeDescriptor.recursiveTypeDescriptor()
                        # Append this funcSpec to that class's downcast methods
                        fromClass.downcastMethods.append(funcSpec)
    
    def constructConstructorSpecifications(self, typeIndex):
        funcSpecs = []
        numFuncs = interrogate_type_number_of_constructors(typeIndex)
        for i in range(numFuncs):
            funcIndex = interrogate_type_get_constructor(typeIndex, i)
            typeDescs = self.constructFunctionTypeDescriptors(funcIndex)
            for typeDesc in typeDescs:
                funcSpec = FFISpecs.MethodSpecification()
                funcSpec.name = 'constructor'
                # funcSpec.name = FFIRename.methodNameFromCppName(
                #    interrogate_function_name(funcIndex))
                funcSpec.typeDescriptor = typeDesc
                # Flag this function as being a constructor
                funcSpec.constructor = 1
                funcSpec.index = funcIndex            
                funcSpecs.append(funcSpec)
        return funcSpecs
    
    def constructDestructorSpecification(self, typeIndex):
        if (not interrogate_type_has_destructor(typeIndex)):
            return None
        funcIndex = interrogate_type_get_destructor(typeIndex)
        typeDescs = self.constructFunctionTypeDescriptors(funcIndex)
        if (len(typeDescs) == 0):
            return None
        for typeDesc in typeDescs:
            funcSpec = FFISpecs.MethodSpecification()
            funcSpec.name = 'destructor'
            # funcSpec.name = FFIRename.methodNameFromCppName(
            #    interrogate_function_name(funcIndex))
            funcSpec.typeDescriptor = typeDesc
            funcSpec.index = funcIndex
            return funcSpec
    
    def addTypes(self):
        for i in range(interrogate_number_of_global_types()):
            self.constructDescriptor(interrogate_get_global_type(i))

    def addEnvironmentTypes(self):
        for descriptor in self.typeIndexMap.values():
            self.environment.addType(descriptor, descriptor.foreignTypeName)
    
    def constructGlobal(self, globalIndex):
        # We really do not need the descriptor for the value, just
        # the getter and setter
        # typeIndex = interrogate_element_type(globalIndex)
        # descriptor = self.constructDescriptor(typeIndex)
        
        if interrogate_element_has_getter(globalIndex):
            getterIndex = interrogate_element_getter(globalIndex)
            getter = self.constructGlobalFunction(getterIndex)
        else:
            getter = None

        if interrogate_element_has_setter(globalIndex):
            setterIndex = interrogate_element_setter(globalIndex)
            setter = self.constructGlobalFunction(setterIndex)
        else:
            setter = None
        globalSpec = FFISpecs.GlobalValueSpecification()
        globalSpec.getter = getter
        globalSpec.setter = setter
        # globalSpec.typeDescriptor = descriptor
        cppName = interrogate_element_name(globalIndex)
        globalSpec.name = FFIRename.classNameFromCppName(cppName)
        return globalSpec

    def constructGlobalFunction(self, globalIndex):
        descriptors = self.constructFunctionTypeDescriptors(globalIndex)
        if (len(descriptors) == 0):
            return None
        for descriptor in descriptors:
            funcSpec = FFISpecs.GlobalFunctionSpecification()
            funcSpec.typeDescriptor = descriptor
            funcSpec.name = FFIRename.methodNameFromCppName(
                funcSpec.typeDescriptor.foreignTypeName)
            funcSpec.index = globalIndex
            return funcSpec
        
    def addGlobalFunctions(self):
        numGlobals = interrogate_number_of_global_functions()
        for i in range(numGlobals):
            funcIndex = interrogate_get_global_function(i)
            newGlob = self.constructGlobalFunction(funcIndex)
            if newGlob:
                self.environment.addGlobalFunction(newGlob)

        # Take all the global functions that have a Panda Class as their
        # first argument and make them class methods on that class
        # For example the global function
        #    get_distance(node1, node2)
        # becomes:
        #    node1.getDistance(node2)
       
        # Functions that do not get moved will be stored here temporarily
        tempGlobalFunctions = []
        for funcSpec in self.environment.globalFunctions:
            # If there are any arguments
            if (len(funcSpec.typeDescriptor.argumentTypes) > 0):
                # If the first argument is a class type descriptor
                methodArgSpec = funcSpec.typeDescriptor.argumentTypes[0]
                argBaseType = methodArgSpec.typeDescriptor.recursiveTypeDescriptor()
                if isinstance(argBaseType, FFITypes.ClassTypeDescriptor):
                    # Move this global function into the class
                    argBaseType.globalMethods.append(funcSpec)
                else:
                    # Copy this function into the temp list
                    tempGlobalFunctions.append(funcSpec)
            else:
                # Copy this function into the temp list
                tempGlobalFunctions.append(funcSpec)
        # Now copy the temp list back over the real list
        self.environment.globalFunctions = tempGlobalFunctions
                    
    def addGlobalValues(self):
        numGlobals = interrogate_number_of_globals()
        for i in range(numGlobals):
            globalIndex = interrogate_get_global(i)
            newGlob = self.constructGlobal(globalIndex)
            self.environment.addGlobalValue(newGlob)


    def constructManifest(self, manifestIndex):
        descriptor = None
        intValue = None
        getter = None

        if interrogate_manifest_has_type(manifestIndex):
            typeIndex = interrogate_manifest_get_type(manifestIndex)
            descriptor = self.constructDescriptor(typeIndex)

        definition = interrogate_manifest_definition(manifestIndex)

        # See if this manifest is an int. There are shortcuts if it is.
        # If it does have an int value, there will be no getter, we will
        # just output the value in the generated code
        if interrogate_manifest_has_int_value(manifestIndex):
            intValue = interrogate_manifest_get_int_value(manifestIndex)
        else:
            # See if this manifest has a getter
            if interrogate_manifest_has_getter(manifestIndex):
                getterIndex = interrogate_manifest_getter(manifestIndex)
                getter = self.constructGlobalFunction(getterIndex)

        manifestSpec = FFISpecs.ManifestSpecification()
        manifestSpec.typeDescriptor = descriptor
        manifestSpec.definition = definition
        manifestSpec.intValue = intValue
        manifestSpec.getter = getter
        cppName = interrogate_manifest_name(manifestIndex)
        manifestSpec.name = FFIRename.classNameFromCppName(cppName)
        return manifestSpec

    def addManifestSymbols(self):
        numManifests = interrogate_number_of_manifests()
        for i in range(numManifests):
            manifestIndex = interrogate_get_manifest(i)
            newManifest = self.constructManifest(manifestIndex)
            self.environment.addManifest(newManifest)


    def generateCode(self, codeDir, extensionsDir):
        FFIConstants.notify.info( 'Generating static class...')
        generateStaticClass(codeDir)

        FFIConstants.notify.info( 'Generating type code...')
        for type in self.environment.types.values():
            # Do not generate code for nested types at the top level
            if (not type.isNested):
                type.generateGlobalCode(codeDir, extensionsDir)
            
        FFIConstants.notify.info( 'Generating global value code...')
        globalFile = constructGlobalFile(codeDir)

        # Make a list of all the global functions. This includes the normal
        # global functions as well as the getters and setters on all the
        # global values. This list is used to figure out what files to import
        globalFunctions = self.environment.globalFunctions
        for globalValue in self.environment.globalValues:
            if globalValue.getter:
                globalFunctions.append(globalValue.getter)
            if globalValue.setter:
                globalFunctions.append(globalValue.setter)
        # Output all the imports based on this list of functions
        outputGlobalFileImports(globalFile, globalFunctions)

        FFIConstants.notify.info( 'Generating global value code...')
        for type in self.environment.globalValues:
            type.generateGlobalCode(globalFile)
            
        FFIConstants.notify.info( 'Generating global function code...')
        for type in self.environment.globalFunctions:
            type.generateGlobalCode(globalFile)

        FFIConstants.notify.info( 'Generating manifest code...')
        for type in self.environment.manifests:
            type.generateGlobalCode(globalFile)

        globalFile.close()

        FFIConstants.notify.info( 'Generating import code...')
        importFile = constructImportFile(codeDir)
        outputImportFileImports(importFile, self.environment.types.values())

        FFIConstants.notify.info( 'Compiling code...')
        compileall.compile_dir(codeDir)
        

    def updateBindings(self):
        FFIConstants.notify.info( 'Updating Bindings')
        FFIConstants.notify.info( 'Adding Types...')
        self.addTypes()
        FFIConstants.notify.info( 'Adding global values...')
        self.addGlobalValues()
        FFIConstants.notify.info( 'Adding global functions...')
        self.addGlobalFunctions()
        FFIConstants.notify.info( 'Adding manifests symbols...')
        self.addManifestSymbols()
        FFIConstants.notify.info( 'Adding environment types...')
        self.addEnvironmentTypes()


