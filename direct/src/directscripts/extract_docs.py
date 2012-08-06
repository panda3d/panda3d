""" This script generates a pandadoc.hpp file representing the Python
wrappers that can be parsed by doxygen to generate the Python documentation.
You need to run this before invoking Doxyfile.python.

It requires a valid makepanda installation with interrogatedb .in
files in the lib/pandac/input directory. """

__all__ = []

import os, re
import panda3d, pandac
from panda3d.dtoolconfig import *

LICENSE = """PANDA 3D SOFTWARE
Copyright (c) Carnegie Mellon University.  All rights reserved.
All use of this software is subject to the terms of the revised BSD
license.  You should have received a copy of this license along
with this source code in a file named \"LICENSE.\"""".split("\n")

libraries = {}
for m, lib in panda3d.modules.items():
    if not isinstance(lib, str):
        for l in lib:
            libraries[l.replace("lib", "")] = m
    else:
        libraries[lib.replace("lib", "")] = m

def comment(code):
    lines = code.split("\n")
    newlines = []
    indent = 0
    reading_desc = False

    for line in lines:
        if line.startswith("////"):
            continue

        line = line.rstrip()
        strline = line.lstrip('/ \t')
        if reading_desc:
            newlines.append(line[min(indent, len(line) - len(strline)):])
        else:
            # A "Description:" text starts the description.
            if strline.startswith("Description"):
                strline = strline[11:].lstrip(': \t')
                indent = len(line) - len(strline)
                reading_desc = True
                newlines.append(strline)

    newcode = "\n".join(newlines)
    if len(newcode) > 0:
        return "/** " + newcode + " */"
    else:
        return ""

def translateFunctionName(name):
    new = ""
    for i in name.split("_"):
        if new == "":
            new += i
        elif i == "":
            pass
        elif len(i) == 1:
            new += i[0].upper()
        else:
            new += i[0].upper() + i[1:]
    return new

def translated_type_name(type):
    typename = interrogate_type_name(type)
    typename = typename.replace("< ", "").replace(" >", "")
    return typename

def translateTypeSpec(name):
    name = name.strip("* ")
    name = name.replace("BitMask< unsigned int, 32 >", "BitMask32")
    name = name.replace("atomic ", "")
    name = name.replace("< ", "").replace(" >", "")
    return name

def processFunction(handle, function, isConstructor = False):
    for i_wrapper in xrange(interrogate_function_number_of_python_wrappers(function)):
        wrapper = interrogate_function_python_wrapper(function, i_wrapper)
        if interrogate_wrapper_has_comment(wrapper):
            print >>handle, comment(interrogate_wrapper_comment(wrapper))
        
        if not isConstructor:
            if not interrogate_wrapper_number_of_parameters(wrapper) > 0 or not interrogate_wrapper_parameter_is_this(wrapper, 0):
                print >>handle, "static",
            
            if interrogate_wrapper_has_return_value(wrapper):
                print >>handle, translateTypeSpec(translated_type_name(interrogate_wrapper_return_type(wrapper))),
            else:
                pass#print >>handle, "void",

            print >>handle, translateFunctionName(interrogate_function_name(function)) + "(",
        else:
            print >>handle, "__init__(",
        
        first = True
        for i_param in range(interrogate_wrapper_number_of_parameters(wrapper)):
            if not interrogate_wrapper_parameter_is_this(wrapper, i_param):
                if not first:
                    print >>handle, ",",
                print >>handle, translateTypeSpec(translated_type_name(interrogate_wrapper_parameter_type(wrapper, i_param))),
                if interrogate_wrapper_parameter_has_name(wrapper, i_param):
                    print >>handle, interrogate_wrapper_parameter_name(wrapper, i_param),
                first = False
        
        print >>handle, ");"

def processType(handle, type):
    typename = translated_type_name(type)
    derivations = [ translated_type_name(interrogate_type_get_derivation(type, n)) for n in range(interrogate_type_number_of_derivations(type)) ]
    
    if interrogate_type_has_comment(type):
        print >>handle, comment(interrogate_type_comment(type))
    
    if interrogate_type_is_enum(type):
        print >>handle, "enum %s {" % typename
        for i_value in range(interrogate_type_number_of_enum_values(type)):
            print >>handle, translateFunctionName(interrogate_type_enum_value_name(type, i_value)), "=", interrogate_type_enum_value(type, i_value), ","
    else:
        if interrogate_type_is_struct(type):
            classtype = "struct"
        elif interrogate_type_is_class(type):
            classtype = "class"
        elif interrogate_type_is_union(type):
            classtype = "union"
        else:
            print "I don't know what type %s is" % typename
            return
        
        if len(derivations) > 0:
            print >>handle, "%s %s : public %s {" % (classtype, typename, ", public ".join(derivations))
        else:
            print >>handle, "%s %s {" % (classtype, typename)
        print >>handle, "public:"
    
    for i_ntype in xrange(interrogate_type_number_of_nested_types(type)):
        processType(handle, interrogate_type_get_nested_type(type, i_ntype))
    
    for i_method in xrange(interrogate_type_number_of_constructors(type)):
        processFunction(handle, interrogate_type_get_constructor(type, i_method), True)
    
    for i_method in xrange(interrogate_type_number_of_methods(type)):
        processFunction(handle, interrogate_type_get_method(type, i_method))
    
    for i_method in xrange(interrogate_type_number_of_make_seqs(type)):
        print >>handle, "list", translateFunctionName(interrogate_make_seq_seq_name(interrogate_type_get_make_seq(type, i_method))), "();"
    
    print >>handle, "};"

if __name__ == "__main__":
    handle = open("pandadoc.hpp", "w")
    
    print >>handle, comment("Panda3D modules that are implemented in C++.")
    print >>handle, "namespace panda3d {}"
    
    # Determine the path to the interrogatedb files
    interrogate_add_search_directory(os.path.join(os.path.dirname(pandac.__file__), "..", "..", "etc"))
    interrogate_add_search_directory(os.path.join(os.path.dirname(pandac.__file__), "input"))

    try:
        panda3d.__load__()
    except ImportError, msg:
        print msg

    lastpkg = None
    for i_type in xrange(interrogate_number_of_global_types()):
        type = interrogate_get_global_type(i_type)

        if interrogate_type_has_module_name(type):
            package = libraries[interrogate_type_module_name(type)]
            if lastpkg != package:
                print >>handle, "}"
                print >>handle, "namespace panda3d.%s {" % package
                lastpkg = package

            processType(handle, type)
        else:
            print "Type %s has no module name" % typename
    
    if lastpkg is not None:
        print >>handle, "}"
    handle.close()

