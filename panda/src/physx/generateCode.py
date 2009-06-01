
import os.path
import re
import xmltramp

docDir = "C:\\pratt\\schellGames\\physxDoxy\\2.7.3\\doc\\xml"

membersToOmit = (
    "user_data",
    "get_type",
    "get_internal",
    "load",
    "create_fluid_hardware_triangle_mesh",
    "get_center",
    "get_extents",
    "get_rot",
    "physx_ray_capsule_intersect",
    "is_point_in_plane_joint",
    "is_point_on_line_joint",
    "get_actors",
    "get_actor_group_pair_array",
    "get_filter_ops",
    "get_island_array_from_actor",
    "get_material_array",
    "physx_compute_sphere",
    "l_vec_base3f_obb_sqr_dist",
)

stringConversions = [
    ("NX_INLINE", "INLINE"),
    ("NX_BOOL", "bool"),
    ("NxI64", "PN_int64"),
    ("NxI32", "int"),
    ("NxI16", "short"),
    ("NxI8", "char"),
    ("NxU64", "PN_uint64"),
    ("NxU32", "unsigned int"),
    ("NxU16", "unsigned short"),
    ("NxU8", "unsigned char"),
    ("NxF32", "float"),
    ("NxF64", "double"),
    ("NxReal", "float"),
    ("NxActorGroup", "unsigned short"),
    ("NxCollisionGroup", "unsigned short"),
    ("NxMaterialIndex", "unsigned short"),
    ("NxSubmeshIndex", "unsigned int"),
    ("NxTriangleID", "unsigned int"),
    ("NxVec3", "LVecBase3f"),
    ("NxPoint", "LVecBase3f"),
    ("NxMat33", "LMatrix3f"),
    ("NxMat34", "LMatrix4f"),
    ("NxQuat", "LQuaternionf"),
    ("Nx", "Physx"),
]
reConversions = []
for pair in stringConversions:
    reConversions.append( (re.compile( pair[0] ), pair[1]) )

convertFromFunctions = {
    "LVecBase3f" : "PhysxManager::lVecBase3_to_nxVec3",
    "LMatrix3f" : "PhysxManager::lMatrix3_to_nxMat33",
    "LMatrix4f" : "PhysxManager::lMatrix4_to_nxMat34",
    "LQuaternionf" : "PhysxManager::lQuaternion_to_nxQuat",
}

convertToFunctions = {
    "LVecBase3f" : "PhysxManager::nxVec3_to_lVecBase3",
    "LMatrix3f" : "PhysxManager::nxMat33_to_lMatrix3",
    "LMatrix4f" : "PhysxManager::nxMat34_to_lMatrix4",
    "LQuaternionf" : "PhysxManager::nxQuat_to_lQuaternion",
}

builtinTypes = (
    "void",
    "bool",
    "int",
    "float",
    "double",
    "char",
    "unsigned char",
    "signed char",
    "unsigned int",
    "signed int",
    "short int",
    "unsigned short int",
    "signed short int",
    "long int",
    "signed long int",
    "unsigned long int",
    "long double",
)

#subClasses = {
#   "PhysxBoxShape" : "PhysxShape",
#   "PhysxCapsuleShape" : "PhysxShape",
#   "PhysxPlaneShape" : "PhysxShape",
#   "PhysxSphereShape" : "PhysxShape",
#   "PhysxD6Joint" : "PhysxJoint",
#   "PhysxD6JointDesc" : "PhysxJointDesc",
#   "PhysxActorNode" : "PandaNode",
#}

typedWritableReferenceCountClasses = (
    "BoxShape",
    "CapsuleShape",
    "D6Joint",
    "Joint",
    "PlaneShape",
    "Shape",
    "SphereShape",
)

classNameConversions = {
    "Actor" : "ActorNode",
}

def generateClassCode( name, isStruct=False ):
    underscoredName = getUnderscoredFromMixedCase( name )
    if isStruct:
        xmlfile = file( os.path.join( docDir, "struct_nx_%s.xml" % underscoredName ) )
    else:
        xmlfile = file( os.path.join( docDir, "class_nx_%s.xml" % underscoredName ) )
    doc = xmltramp.seed( xmlfile )
    xmlfile.close()

    if hasattr( doc[0], "basecompoundref" ):
        className = str( doc[0].basecompoundref ).replace( "Nx", "Physx" )
        prot = doc[0].basecompoundref( "prot" )
        virt = doc[0].basecompoundref( "virt" )
        if virt == "virtual":
            baseClassRef = "%s virtual %s" % (prot, className)
        else:
            baseClassRef = "%s %s" % (prot, className)
    else:
        baseClassRef = None

    structors = {}
    methods = {}
    attributes = {}
    for section in doc[0]["sectiondef":]:
        if (section( "kind" ) == "public-func") or (section( "kind" ) == "user-defined"):
            for member in section["memberdef":]:
                memberName = str( member.name )
                method = {}
                method["type"] = str( member.type )
                method["argsstring"] = str( member.argsstring )
                method["const"] = member( "const" ) == "yes"
                method["params"] = []
                for param in member["param":]:
                    if hasattr( param, "type" ) and hasattr( param, "declname" ):
                        method["params"].append( [str( param.type ), str( param.declname )] )
                if (memberName == "Nx" + name) or (memberName == "~Nx" + name):
                    structors[memberName] = method
                else:
                    methods[memberName] = method
        elif section( "kind" ) == "public-attrib":
            for member in section["memberdef":]:
                memberName = str( member.name )
                variable = {}
                variable["type"] = "INLINE " + str( member.type )
                attributes[memberName] = variable

    convertNames( structors, underscoreNames=False )
    convertNames( methods )
    convertNames( attributes )

    filterMembers( structors )
    filterMembers( methods )
    filterMembers( attributes )

    createHeader( name, structors, methods, attributes, baseClassRef )
    createCxx( name, structors, methods, attributes )
    createInline( name, structors, methods, attributes )

def convertNames( memberDict, underscoreNames=True ):
    for memberName in memberDict.keys():
        info = memberDict[memberName]
        for key in info.keys():
            if key == "type":
                info[key] = convertString( info[key] )
                rawType = getRawType( info[key] )
                if (rawType not in builtinTypes) and ("INLINE" in info[key]):
                    info[key] = info[key].replace( "INLINE", "" ).strip()
                if rawType.startswith( "Physx" ) and (rawType[5:] in classNameConversions):
                    info[key] = info[key].replace( "Physx" + rawType[5:], "Physx" + classNameConversions[rawType[5:]] ).strip()
            elif key == "argsstring":
                info[key] = convertString( info[key] )
            elif key == "params":
                for param in info[key]:
                    param[0] = convertString( param[0] )
                    param[1] = getUnderscoredFromMixedCase( convertString( param[1] ) )
                    rawType = getRawType( param[0] )
                    if (rawType not in builtinTypes) and ("INLINE" in info["type"]):
                        param[0] = param[0].replace( "INLINE", "" ).strip()
                    if rawType.startswith( "Physx" ) and (rawType[5:] in classNameConversions):
                        param[0] = param[0].replace( "Physx" + rawType[5:], "Physx" + classNameConversions[rawType[5:]] ).strip()
            elif key == "const":
                pass
            else:
                raise

        memberDict[memberName]["originalName"] = memberName
        if underscoreNames:
            newMemberName = getUnderscoredFromMixedCase( convertString( memberName ) )
        else:
            newMemberName = convertString( memberName )

        if newMemberName != memberName:
            del memberDict[memberName]
            memberDict[newMemberName] = info

def filterMembers( memberDict ):
    for memberName in memberDict.keys():
        info = memberDict[memberName]
        rawType = getRawType( info["type"] )
        if memberName in membersToOmit:
            del memberDict[memberName]
        elif rawType.startswith( "Physx" ) and (not isNameKnown( rawType[5:] )):
            del memberDict[memberName]
        elif info["type"].count( "*" ) > 1:
            del memberDict[memberName]
        elif memberDict[memberName].has_key( "params" ):
            for param in memberDict[memberName]["params"]:
                rawType = getRawType( param[0] )
                if rawType.startswith( "Physx" ) and (not isNameKnown( rawType[5:] )):
                    del memberDict[memberName]
                    break

def convertString( s ):
    for pattern, replacement in reConversions:
        s = pattern.sub( replacement, s )
    return s

def createHeader( name, structors, methods, attributes, baseClassRef ):
    templateFile = file( "physxTemplate.h" )
    templateString = templateFile.read()
    templateFile.close()

    lowerPattern = re.compile( "template" )
    upperPattern = re.compile( "TEMPLATE" )
    capsPattern = re.compile( "Template" )
    typeHeaderPattern = re.compile( "//typedWritableReferenceCountHeader" )
    superHeaderPattern = re.compile( "//superHeader\n" )
    forwardsPattern = re.compile( "//forwardDeclarations\n" )
    baseClassRefPattern = re.compile( "/\*baseclassref\*/" )
    structorsPattern = re.compile( "//publicstructors" )
    methodsPattern = re.compile( "//publicmethods" )
    attributesPattern = re.compile( "//publicattributes" )
    nxReferencePattern = re.compile( "//nxreference" )
    typeInfoPattern = re.compile( "//typeinfo\n" )

    nName = name
    if name in classNameConversions:
        name = classNameConversions[name]

    if name in typedWritableReferenceCountClasses:
        if baseClassRef is None:
            baseClassRef = "public TypedWritableReferenceCount"
        templateString = typeInfoPattern.sub( getTypeInfo( name, baseClassRef ), templateString )
    else:
        templateString = typeInfoPattern.sub( "", templateString )

    if baseClassRef is not None:
        templateString = superHeaderPattern.sub( '#include "%s.h"\n\n' % lowerFirst( baseClassRef.split()[1] ), templateString )
        templateString = baseClassRefPattern.sub( ": %s " % baseClassRef, templateString )
    else:
        templateString = superHeaderPattern.sub( "", templateString )
        templateString = baseClassRefPattern.sub( "", templateString )

    templateString = lowerPattern.sub( lowerFirst( name ), templateString )
    templateString = upperPattern.sub( name.upper(), templateString )
    templateString = capsPattern.sub( name, templateString )
    templateString = forwardsPattern.sub( getForwardDeclarations( name, structors, methods, attributes, baseClassRef ), templateString )
    templateString = structorsPattern.sub( getMethodHeaders( structors ), templateString )
    templateString = methodsPattern.sub( getMethodHeaders( methods ), templateString )
    templateString = attributesPattern.sub( getAttributeHeaders( attributes ), templateString )

    if name in avoidPointerNames:
        templateString = nxReferencePattern.sub( "  Nx%s %s;" % (nName, "n" + nName), templateString )
    else:
        templateString = nxReferencePattern.sub( "  Nx%s *%s;" % (nName, "n" + nName), templateString )

    outputFile = file( "generated/physx" + name + ".h", "w" )
    outputFile.write( templateString )
    outputFile.close()

def getIncludes( name, structors, methods, attributes, baseClassRef=None ):
    rawTypes = []
    for method in structors.values() + methods.values():
        for param in method["params"]:
            rawTypes.append( getRawType( param[0] ) )
    for member in methods.values() + attributes.values():
        rawTypes.append( getRawType( member["type"] ) )
    if baseClassRef is not None:
        for part in baseClassRef.split():
            rawTypes.append( part )

    includeDict = {}
    for rawType in rawTypes:
        if rawType.startswith( "Physx" ) and isBuiltName( rawType[5:] ) and (rawType[5:] != name):
            includeDict['#include "%s.h"\n' % (rawType[0].lower() + rawType[1:])] = True
        elif rawType.startswith( "L" ):
            includeDict['#include "luse.h"\n'] = True

    includes = includeDict.keys()
    includes.sort()
    return "".join( includes )

def getForwardDeclarations( name, structors, methods, attributes, baseClassRef=None ):
    rawTypes = []
    for method in structors.values() + methods.values():
        for param in method["params"]:
            rawTypes.append( getRawType( param[0] ) )
    for member in methods.values() + attributes.values():
        rawTypes.append( getRawType( member["type"] ) )

    if baseClassRef is not None:
        baseClass = baseClassRef.split()[1]
    else:
        baseClass = None
    forwardDict = {}  # use a dictionary to avoid duplicates
    for rawType in rawTypes:
        if rawType.startswith( "Physx" ) and isBuiltName( rawType[5:] ) and (rawType[5:] != name) and (rawType != baseClass):
            forwardDict["class %s;\n" % rawType] = True

    forwards = forwardDict.keys()
    forwards.sort()

    if len( forwards ) > 0:
        return "".join( forwards ) + "\n"
    else:
        return ""

def getMethodHeaders( methodDict ):
    s = ""
    names = methodDict.keys()
    names.sort()
    for name in names:
        method = methodDict[name]
        s += "  "
        if method["type"] != "":
            s += "%s " % method["type"]
        s += "%s(%s)" % (name, getParamString( method["params"] ))
        if method["const"]:
            s += " const"
        s += ";\n"

    return s

def getAttributeHeaders( attributeDict ):
    getters = ""
    setters = ""
    names = attributeDict.keys()
    names.sort()
    for name in names:
        attribute = attributeDict[name]
        type = attribute["type"]
        rawType = getRawType( type )
        if (rawType not in builtinTypes) and (not type.endswith( "*" )) and (not convertFromFunctions.has_key( rawType )) and (rawType not in enumerations.keys()):
            type += " &"
        getters += "  %s get_%s() const;\n" % (type, name)
        if "INLINE" in type:
            setters += "  INLINE void set_%s(%s value);\n" % (name, type.replace( "INLINE", "" ).strip())
        else:
            setters += "  void set_%s(%s value);\n" % (name, type.replace( "INLINE", "" ).strip())
    if getters != "":
        return "%s\n%s" % (getters, setters)
    else:
        return ""

def getTypeInfo( className, baseClassRef ):
    typeInfoString = """
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    %(superName)s::init_type();
    register_type(_type_handle, "Physx%(className)s",
                  %(superName)s::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
"""
    content = {}
    content["className"] = className
    content["superName"] = baseClassRef.split()[1]
    return typeInfoString % content

def createCxx( name, structors, methods, attributes ):
    templateFile = file( "physxTemplate.cxx" )
    templateString = templateFile.read()
    templateFile.close()

    lowerPattern = re.compile( "template" )
    upperPattern = re.compile( "TEMPLATE" )
    capsPattern = re.compile( "Template" )
    includesPattern = re.compile( "//includes" )
    typeHandlePattern = re.compile( "//typehandle" )
    structorsPattern = re.compile( "//publicstructors\n" )
    methodsPattern = re.compile( "//publicmethods\n" )
    attributesPattern = re.compile( "//publicattributes\n" )

    pName = name
    if name in classNameConversions:
        pName = classNameConversions[name]

    if pName in typedWritableReferenceCountClasses:
        templateString = typeHandlePattern.sub( "TypeHandle Physx%s::_type_handle;\n" % pName, templateString )
    else:
        templateString = typeHandlePattern.sub( "", templateString )

    templateString = lowerPattern.sub( lowerFirst( pName ), templateString )
    templateString = upperPattern.sub( pName.upper(), templateString )
    templateString = capsPattern.sub( pName, templateString )
    templateString = includesPattern.sub( getIncludes( name, structors, methods, attributes ), templateString )
    templateString = structorsPattern.sub( getMethodSource( name, structors, doInline=False, doCode=False ), templateString )
    templateString = methodsPattern.sub( getMethodSource( name, methods, doInline=False, doCode=True ), templateString )
    templateString = attributesPattern.sub( getAttributeSource( name, attributes, doInline=False ), templateString )

    outputFile = file( "generated/physx" + pName + ".cxx", "w" )
    outputFile.write( templateString )
    outputFile.close()

def createInline( name, structors, methods, attributes ):
    templateFile = file( "physxTemplate.I" )
    templateString = templateFile.read()
    templateFile.close()

    lowerPattern = re.compile( "template" )
    upperPattern = re.compile( "TEMPLATE" )
    capsPattern = re.compile( "Template" )
    structorsPattern = re.compile( "//publicstructors\n" )
    methodsPattern = re.compile( "//publicmethods\n" )
    attributesPattern = re.compile( "//publicattributes\n" )

    pName = name
    if name in classNameConversions:
        pName = classNameConversions[name]

    templateString = lowerPattern.sub( lowerFirst( pName ), templateString )
    templateString = upperPattern.sub( pName.upper(), templateString )
    templateString = capsPattern.sub( pName, templateString )
    templateString = structorsPattern.sub( getMethodSource( name, structors, doInline=True, doCode=False ), templateString )
    templateString = methodsPattern.sub( getMethodSource( name, methods, doInline=True, doCode=True ), templateString )
    templateString = attributesPattern.sub( getAttributeSource( name, attributes, doInline=True ), templateString )

    outputFile = file( "generated/physx" + pName + ".I", "w" )
    outputFile.write( templateString )
    outputFile.close()

def getMethodSource( className, methodDict, doInline=False, doCode=True ):
    if className in avoidPointerNames:
        attribOperator = "."
    else:
        attribOperator = "->"

    pClassName = className
    if className in classNameConversions:
        pClassName = classNameConversions[className]

    result = ""

    methodString = """////////////////////////////////////////////////////////////////////
//     Function : %(name)s
//       Access : Published
//  Description : 
////////////////////////////////////////////////////////////////////
%(type)s%(class)s::
%(name)s%(params)s%(const)s {
%(code)s
}

"""
    names = methodDict.keys()
    names.sort()
    for name in names:
        method = methodDict[name]
        if (("INLINE" in method["type"]) and doInline) or (("INLINE" not in method["type"]) and (not doInline)):
            content = {}
            content["name"] = name
            if method["type"] != "":
                content["type"] = "%s " % method["type"]
            else:
                content["type"] = ""
            content["class"] = "Physx" + pClassName
            content["params"] = "(%s)" % getParamString( method["params"] )
            if method["const"]:
                content["const"] = " const"
            else:
                content["const"] = ""

            nxObject = "n" + className
            nxMethodName = method["originalName"]
            callerParamString = getCallerParamString( method["params"] )

            if name.startswith( "create" ):
                content["code"] = '  throw "Not Implemented";'
            elif doCode:
                if method.has_key( "code" ):
                    content["code"] = method["code"] % nxObject
                elif ("void" in method["type"]) and ("*" not in method["type"]):
                    content["code"] = "  %s%s%s(%s);" % (nxObject, attribOperator, nxMethodName, callerParamString)
                else:
                    inner = "%s%s%s(%s)" % (nxObject, attribOperator, nxMethodName, callerParamString)
                    content["code"] = "  return %s;" % wrapValue( method["type"], inner, isReturnValue=True )

                if className not in avoidPointerNames:
                    if ("void" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertv(%s != NULL);\n\n" % nxObject) + content["code"]
                    elif ("bool" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertr(%s != NULL, false);\n\n" % nxObject) + content["code"]
                    elif ("float" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertr(%s != NULL, -1.0f);\n\n" % nxObject) + content["code"]
                    elif ("unsigned int" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertr(%s != NULL, -1);\n\n" % nxObject) + content["code"]
                    elif ("unsigned short" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertr(%s != NULL, -1);\n\n" % nxObject) + content["code"]
                    elif ("LVecBase3f" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertr(%s != NULL, *((LVecBase3f *)NULL));\n\n" % nxObject) + content["code"]
                    elif ("LMatrix3f" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertr(%s != NULL, *((LMatrix3f *)NULL));\n\n" % nxObject) + content["code"]
                    elif ("LMatrix4f" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertr(%s != NULL, *((LMatrix4f *)NULL));\n\n" % nxObject) + content["code"]
                    elif ("LQuaternionf" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertr(%s != NULL, *((LQuaternionf *)NULL));\n\n" % nxObject) + content["code"]
                    elif ("PhysxScene" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertr(%s != NULL, *((PhysxScene *)NULL));\n\n" % nxObject) + content["code"]
                    elif ("PhysxJointState" in method["type"]) and ("*" not in method["type"]):
                        content["code"] = ("  nassertr(%s != NULL, physx_js_unbound);\n\n" % nxObject) + content["code"]
                    elif "*" in method["type"]:
                        content["code"] = ("  nassertr(%s != NULL, NULL);\n\n" % nxObject) + content["code"]

            else:
                content["code"] = ""

            result += methodString % content

    return result

def getAttributeSource( className, attributeDict, doInline=False ):
    if className in avoidPointerNames:
        attribOperator = "."
    else:
        attribOperator = "->"

    methodDict = {}

    for name in attributeDict.keys():
        type = attributeDict[name]["type"]
        rawType = getRawType( type )
        if (rawType not in builtinTypes) and (not type.endswith( "*" )) and (not convertFromFunctions.has_key( rawType )) and (rawType not in enumerations.keys()):
            type += " &"
        setterValue = wrapValue( type, "value", isReturnValue=False )
        getter = {
            "type" : type,
            "const" : True,
            "params" : [],
            "code" : "  return %%s%s%s;" % (attribOperator, attributeDict[name]["originalName"]),
            "originalName" : getMixedCaseFromUnderscored( "get_" + name ),
        }
        setter = {
            "type" : "void",
            "const" : False,
            "params" : [[type.replace( "INLINE", "" ).strip(), "value"]],
            "code" : "  %%s%s%s = %s;" % (attribOperator, attributeDict[name]["originalName"], setterValue),
            "originalName" : getMixedCaseFromUnderscored( "set_" + name ),
        }
        methodDict["get_" + name] = getter
        methodDict["set_" + name] = setter

        if "INLINE" in type:
            setter["type"] = "INLINE void"

        if convertFromFunctions.has_key( rawType ):
            valueString = "%%s%s%s" % (attribOperator, attributeDict[name]["originalName"])
            getter["code"] = "  return %s;" % getConvertedValue( type, valueString, False )
            getter["type"] = type
        elif rawType in enumerations.keys():
            valueString = "%%s%s%s" % (attribOperator, attributeDict[name]["originalName"])
            getter["code"] = "  return %s;" % wrapValue( rawType, valueString, isReturnValue=True )
        elif rawType.startswith( "Physx" ) and isBuiltName( rawType[5:] ):
            getter["code"] = '  throw "Not Implemented"; // return %%s%s%s;' % (attribOperator, attributeDict[name]["originalName"])

    return getMethodSource( className, methodDict, doInline=doInline, doCode=True )

def getParamString( params ):
    items = ["%s %s" % (param[0], param[1]) for param in params]
    return ", ".join( items )

def getCallerParamString( params ):
    items = []
    for param in params:
        items.append( wrapValue( param[0], param[1] ) )
    return ", ".join( items )

def wrapValue( type, valueString, isReturnValue=False ):
    rawType = getRawType( type )
    if rawType in builtinTypes:
        return valueString
    elif (not isReturnValue) and convertFromFunctions.has_key( rawType ):
        return getConvertedValue( type, valueString, not isReturnValue )
    elif isReturnValue and convertToFunctions.has_key( rawType ):
        return getConvertedValue( type, valueString, not isReturnValue )
    elif rawType.startswith( "Physx" ) and isBuiltName( rawType[5:] ):
        if isReturnValue:
            if "*" in type:
                return "(%s *)(%s->userData)" % (rawType, valueString)
            else:
                return "*((%s *)(%s.userData))" % (rawType, valueString)
        else:
            if "*" in type:
                if rawType[5:] in avoidPointerNames:
                    return "&(%s->%s)" % (valueString, "n" + rawType[5:])
                else:
                    return "%s->%s" % (valueString, "n" + rawType[5:])
            else:
                if rawType[5:] in avoidPointerNames:
                    return "%s.%s" % (valueString, "n" + rawType[5:])
                else:
                    return "*(%s.%s)" % (valueString, "n" + rawType[5:])
    elif rawType in enumerations.keys():
        if isReturnValue:
            return "(%s)%s" % (rawType, valueString)
        else:
            return "(%s)%s" % ("Nx" + rawType[5:], valueString)
    else:
        return valueString

def isNameKnown( name ):
    if name in allBuiltNames:
        return True
    elif name in classNameConversions.values():
        return True
    elif enumerations.has_key( "Physx" + name ):
        return True
    else:
        return False

def isBuiltName( name ):
    return (name in allBuiltNames) or (name in classNameConversions.values())

def getUnderscoredFromMixedCase( name ):
    return re.sub( r"(?<=[a-z])[A-Z]|(?<!^)[A-Z](?=[a-z])", r"_\g<0>", name ).lower()

def getMixedCaseFromUnderscored( name ):
    return re.sub( r"_([a-z])", lambda m: (m.group( 1 ).upper()), name )

def lowerFirst( name ):
    return name[0].lower() + name[1:]

def getRawType( type ):
    rawType = type.replace( "const", "" )
    rawType = rawType.replace( "INLINE", "" )
    rawType = rawType.strip( " *&" )
    return rawType

def getConvertedValue( type, valueString, convertFrom=True ):
    rawType = getRawType( type )
    if convertFrom:
        func = convertFromFunctions[rawType]
    else:
        func = convertToFunctions[rawType]
    if "*" in type:
        return "&%s(*%s)" % (func, valueString)
    else:
        return "%s(%s)" % (func, valueString)

def generateEnumerationFile():
    global enumerations
    enumerations = {}
    locations = {}
    for group in ("cloth", "fluids", "foundation", "physics", "softbody"):
        getEnumerations( group, enumerations, locations )

    enumerationContent = []
    enumerationContentForInterrogate = []
    names = enumerations.keys()
    names.sort()
    for name in names:
        if name not in ["PhysxMemoryType"]:
            enumerationContent.append( "enum %s {\n" % name )
            enumerationContentForInterrogate.append( "enum %s {\n" % name )
            nonInitializedItems, namesToInitializers = enumerations[name]
            for nxValue in nonInitializedItems:
                parts = nxValue.split( "_" )[1:]
                parts = ["physx"] + [part.lower() for part in parts]
                physxValue = "_".join( parts )
                enumerationContent.append( "  %s = %s,\n" % (physxValue, nxValue) )
                enumerationContentForInterrogate.append( "  %s,\n" % physxValue )
            for nxValue in namesToInitializers.keys():
                parts = nxValue.split( "_" )[1:]
                parts = ["physx"] + [part.lower() for part in parts]
                physxValue = "_".join( parts )
                enumerationContent.append( "  %s = %s,\n" % (physxValue, nxValue) )

                initializer = namesToInitializers[nxValue]
                enumerationContentForInterrogate.append( "  %s = %s,\n" % (physxValue, initializer) )

            enumerationContent.append( "};\n\n" )
            enumerationContentForInterrogate.append( "};\n\n" )

    enumerationTemplate = """// Filename: physx_enumerations.h
// Created by: pratt (Apr 20, 2006)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2006, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PHYSX_ENUMERATIONS_H
#define PHYSX_ENUMERATIONS_H

#ifdef HAVE_PHYSX

#include "NoMinMax.h"
#include "NxPhysics.h"

%s

#endif // HAVE_PHYSX

#endif // PHYSX_ENUMERATIONS_H
"""

    outputFile = file( "generated/physx_enumerations.h", "w" )
    outputFile.write( enumerationTemplate % "".join( enumerationContent ) )
    outputFile.close()

    outputFile = file( "generated/physx_enumerations_for_interrogate.h", "w" )
    outputFile.write( enumerationTemplate % "".join( enumerationContentForInterrogate ) )
    outputFile.close()

def getEnumerations( group, enumerations, locations ):
    xmlfile = file( os.path.join( docDir, "group__%s.xml" % group ) )
    doc = xmltramp.seed( xmlfile )
    xmlfile.close()

    for section in doc.compounddef["sectiondef":]:
        if section( "kind" ) == "enum":
            for enumeration in section["memberdef":]:
                name = str( enumeration.name ).replace( "Nx", "Physx" )
                nonInitializedItems = []
                namesToInitializers = {}
                for value in enumeration["enumvalue":]:
                    try:
                        initializer = str( value.initializer )
                        try:
                            eval( initializer )
                            namesToInitializers[str( value.name )] = initializer
                        except:
                            # Avoid creating enum values that we know will
                            # not work, i.e. bitwise ORs of previous values.
                            pass
                    except:
                        nonInitializedItems.append( str( value.name ) )
                enumerations[name] = (nonInitializedItems, namesToInitializers)
                location = os.path.basename( str( enumeration["location"]( "file" ) ) )
                if group == "cloth":
                    location = "cloth/" + location
                elif group == "fluids":
                    location = "fluids/" + location
                elif group == "softbody":
                    location = "softbody/" + location
                locations[location] = True

def generateSourcesPP( names ):
    templateFile = file( "SourcesTemplate.pp" )
    templateString = templateFile.read()
    templateFile.close()

    headerLines = []
    sourceLines = []

    for name in names:
        if name == "Actor":
            name = "ActorNode"
        headerLines.append( "    physx%s.I physx%s.h \\\n" % (name, name) )
        sourceLines.append( "    physx%s.cxx \\\n" % name )

    headersPattern = re.compile( "//headers" )
    cxxPattern = re.compile( "//cxx" )

    templateString = headersPattern.sub( "".join( headerLines ), templateString )
    templateString = cxxPattern.sub( "".join( sourceLines ), templateString )

    outputFile = file( "generated/Sources.pp", "w" )
    outputFile.write( templateString )
    outputFile.close()

buildNowNames = [
    "Actor",
    "ActorDesc",
    "BodyDesc",
    "Bounds3",
    "Box",
    "Joint",
    "D6Joint",
    "JointDesc",
    "D6JointDesc",
    "JointDriveDesc",
    "JointLimitSoftDesc",
    "JointLimitSoftPairDesc",
    "Material",
    "MaterialDesc",
    "Plane",
    "Ray",
    "Scene",
    "SceneDesc",
    "SceneStats2",
    "Segment",
    "Capsule",
    "Shape",
    "BoxShape",
    "CapsuleShape",
    "PlaneShape",
    "SphereShape",
    "ShapeDesc",
    "BoxShapeDesc",
    "CapsuleShapeDesc",
    "PlaneShapeDesc",
    "SphereShapeDesc",
    "Sphere",
    "UtilLib",
]
buildNowStructs = [
]

allBuiltNames = [
    "Actor",
    "ActorDesc",
    "BodyDesc",
    "Bounds3",
    "Box",
    "Joint",
    "D6Joint",
    "JointDesc",
    "D6JointDesc",
    "JointDriveDesc",
    "JointLimitSoftDesc",
    "JointLimitSoftPairDesc",
    "Material",
    "MaterialDesc",
    "Plane",
    "Ray",
    "Scene",
    "SceneDesc",
    "SceneStats2",
    "Segment",
    "Capsule",
    "Shape",
    "BoxShape",
    "CapsuleShape",
    "PlaneShape",
    "SphereShape",
    "ShapeDesc",
    "BoxShapeDesc",
    "CapsuleShapeDesc",
    "PlaneShapeDesc",
    "SphereShapeDesc",
    "Sphere",
    "UtilLib",
]

expectedNames = [
    "Actor",
    "ActorDesc",
    "BodyDesc",
    "Bounds3",
    "Box",
    "Cloth",
    "ClothDesc",
    "ClothMesh",
    "ContactPair",
    "ContactStreamIterator",
    "ConvexMesh",
    "ConvexMeshDesc",
    "Effector",
    "SpringAndDamperEffector",
    "Fluid",
    "FluidDesc",
    "FluidEmitter",
    "FluidEmitterDesc",
    "GroupsMask",
    "HeightField",
    "HeightFieldDesc",
    "Joint",
    "CylindricalJoint",
    "D6Joint",
    "DistanceJoint",
    "FixedJoint",
    "PointInPlaneJoint",
    "PointOnLineJoint",
    "PrismaticJoint",
    "PulleyJoint",
    "RevoluteJoint",
    "SphericalJoint",
    "JointDesc",
    "CylindricalJointDesc",
    "D6JointDesc",
    "DistanceJointDesc",
    "FixedJointDesc",
    "PointInPlaneJointDesc",
    "PointOnLineJointDesc",
    "PrismaticJointDesc",
    "PulleyJointDesc",
    "RevoluteJointDesc",
    "SphericalJointDesc",
    "JointDriveDesc",
    "JointLimitDesc",
    "JointLimitPairDesc",
    "JointLimitSoftDesc",
    "JointLimitSoftPairDesc",
    "Material",
    "MaterialDesc",
    "MeshData",
    "MotorDesc",
    "PairFlag",
    "ParticleData",
    "Plane",
    "PMap",
    "Ray",
    "Scene",
    "SceneDesc",
    "SceneStats2",
    "Segment",
    "Capsule",
    "Shape",
    "BoxShape",
    "CapsuleShape",
    "ConvexShape",
    "PlaneShape",
    "SphereShape",
    "TriangleMeshShape",
    "WheelShape",
    "ShapeDesc",
    "BoxShapeDesc",
    "CapsuleShapeDesc",
    "ConvexShapeDesc",
    "PlaneShapeDesc",
    "SphereShapeDesc",
    "TriangleMeshShapeDesc",
    "WheelShapeDesc",
    "SimpleTriangleMesh",
    "ClothMeshDesc",
    "TriangleMeshDesc",
    "Sphere",
    "SpringAndDamperEffectorDesc",
    "SpringDesc",
    "TireFunctionDesc",
    "UtilLib",
    "WheelContactData",
]

avoidPointerNames = [
    "ActorDesc",
    "ActorDescBase",
    "BodyDesc",
    "ClothDesc",
    "ConvexMeshDesc",
    "FluidDesc",
    "FluidEmitterDesc",
    "HeightFieldDesc",
    "CylindricalJointDesc",
    "D6JointDesc",
    "DistanceJointDesc",
    "FixedJointDesc",
    "PointInPlaneJointDesc",
    "PointOnLineJointDesc",
    "PrismaticJointDesc",
    "PulleyJointDesc",
    "RevoluteJointDesc",
    "SphericalJointDesc",
    "JointDriveDesc",
    "JointLimitDesc",
    "JointLimitPairDesc",
    "JointLimitSoftDesc",
    "JointLimitSoftPairDesc",
    "MaterialDesc",
    "MotorDesc",
    "SceneDesc",
    "ShapeDesc",
    "BoxShapeDesc",
    "CapsuleShapeDesc",
    "ConvexShapeDesc",
    "PlaneShapeDesc",
    "SphereShapeDesc",
    "TriangleMeshShapeDesc",
    "WheelShapeDesc",
    "ClothMeshDesc",
    "TriangleMeshDesc",
    "SpringAndDamperEffectorDesc",
    "SpringDesc",
    "TireFunctionDesc",
]

if __name__ == "__main__":
    if not os.path.exists( "generated" ):
        os.mkdir( "generated" )
    generateEnumerationFile()
    generateSourcesPP( allBuiltNames )
    for name in buildNowNames:
        print name
        generateClassCode( name )
    for name in buildNowStructs:
        print name
        generateClassCode( name, isStruct=True )

