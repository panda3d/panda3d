""" This script generates a pandadoc.hpp file representing the Python
wrappers that can be parsed by doxygen to generate the Python documentation.
You need to run this before invoking Doxyfile.python.

It requires a valid makepanda installation with interrogatedb .in
files in the lib/pandac/input directory. """

__all__ = []

import os
import panda3d, pandac
from panda3d.dtoolconfig import *

LICENSE = """PANDA 3D SOFTWARE
Copyright (c) Carnegie Mellon University.  All rights reserved.
All use of this software is subject to the terms of the revised BSD
license.  You should have received a copy of this license along
with this source code in a file named \"LICENSE.\"""".split("\n")

def comment(code):
    if not code:
        return ""

    comment = ''

    empty_line = False
    for line in code.splitlines(False):
        line = line.strip('\t\n /')
        if line:
            if empty_line:
                # New paragraph.
                comment += '\n\n'
                empty_line = False
            elif comment:
                comment += '\n'
            comment += '/// ' + line
        else:
            empty_line = True

    if comment:
        return comment
    else:
        return ''

def block_comment(code):
    if not code:
        return ""

    lines = code.split("\n")
    newlines = []
    indent = 0
    reading_desc = False

    for line in lines:
        if line.startswith("////"):
            continue

        line = line.rstrip()
        strline = line.lstrip('/ \t')

        if ':' in strline:
            pre, post = strline.split(':', 1)
            pre = pre.rstrip()
            if pre == "Description":
                strline = post.lstrip()
            elif pre in ("Class", "Access", "Function", "Created by", "Enum"):
                continue

        if strline or len(newlines) > 0:
            newlines.append('/// ' + strline)

        #if reading_desc:
        #    newlines.append('/// ' + line[min(indent, len(line) - len(strline)):])
        #else:
        #    # A "Description:" text starts the description.
        #    if strline.startswith("Description"):
        #        strline = strline[11:].lstrip(': \t')
        #        indent = len(line) - len(strline)
        #        reading_desc = True
        #        newlines.append('/// ' + strline)
        #    else:
        #        print line

    newcode = '\n'.join(newlines)
    if len(newcode) > 0:
        return newcode
    else:
        return ""

def translateFunctionName(name):
    if name.startswith("__"):
        return name

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

def translateTypeName(name, mangle=True):
    # Equivalent to C++ classNameFromCppName
    class_name = ""
    bad_chars = "!@#$%^&*()<>,.-=+~{}? "
    next_cap = False
    first_char = mangle

    for chr in name:
        if (chr == '_' or chr == ' ') and mangle:
            next_cap = True
        elif chr in bad_chars:
            if not mangle:
                class_name += '_'
        elif next_cap or first_char:
            class_name += chr.upper()
            next_cap = False
            first_char = False
        else:
            class_name += chr

    return class_name

def translated_type_name(type, scoped=True):
    while interrogate_type_is_wrapped(type):
        if interrogate_type_is_const(type):
            return 'const ' + translated_type_name(interrogate_type_wrapped_type(type))
        else:
            type = interrogate_type_wrapped_type(type)

    typename = interrogate_type_name(type)
    if typename in ("PyObject", "_object"):
        return "object"
    elif typename == "PN_stdfloat":
        return "float"

    if interrogate_type_is_atomic(type):
        token = interrogate_type_atomic_token(type)
        if token == 7:
            return 'str'
        else:
            return typename

    if not typename.endswith('_t'):
        # Hack: don't mangle size_t etc.
        typename = translateTypeName(typename)

    if scoped and interrogate_type_is_nested(type):
        return translated_type_name(interrogate_type_outer_class(type)) + '::' + typename
    else:
        return typename

def processElement(handle, element):
    if interrogate_element_has_comment(element):
        print >>handle, comment(interrogate_element_comment(element))

    print >>handle, translated_type_name(interrogate_element_type(element)),
    print >>handle, interrogate_element_name(element) + ';'

def processFunction(handle, function, isConstructor = False):
    for i_wrapper in xrange(interrogate_function_number_of_python_wrappers(function)):
        wrapper = interrogate_function_python_wrapper(function, i_wrapper)
        if interrogate_wrapper_has_comment(wrapper):
            print >>handle, block_comment(interrogate_wrapper_comment(wrapper))
        
        if not isConstructor:
            if interrogate_function_is_method(function):
                if not interrogate_wrapper_number_of_parameters(wrapper) > 0 or not interrogate_wrapper_parameter_is_this(wrapper, 0):
                    print >>handle, "static",
            
            if interrogate_wrapper_has_return_value(wrapper):
                print >>handle, translated_type_name(interrogate_wrapper_return_type(wrapper)),
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
                print >>handle, translated_type_name(interrogate_wrapper_parameter_type(wrapper, i_param)),
                if interrogate_wrapper_parameter_has_name(wrapper, i_param):
                    print >>handle, interrogate_wrapper_parameter_name(wrapper, i_param),
                first = False
        
        print >>handle, ");"

def processType(handle, type):
    typename = translated_type_name(type, scoped=False)
    derivations = [ translated_type_name(interrogate_type_get_derivation(type, n)) for n in range(interrogate_type_number_of_derivations(type)) ]
    
    if interrogate_type_has_comment(type):
        print >>handle, block_comment(interrogate_type_comment(type))
    
    if interrogate_type_is_enum(type):
        print >>handle, "enum %s {" % typename
        for i_value in range(interrogate_type_number_of_enum_values(type)):
            docstring = comment(interrogate_type_enum_value_comment(type, i_value))
            if docstring:
                print >>handle, docstring
            print >>handle, interrogate_type_enum_value_name(type, i_value), "=", interrogate_type_enum_value(type, i_value), ","

    elif interrogate_type_is_typedef(type):
        wrapped_type = translated_type_name(interrogate_type_wrapped_type(type))
        print >>handle, "typedef %s %s;" % (wrapped_type, typename)
        return
    else:
        if interrogate_type_is_struct(type):
            classtype = "struct"
        elif interrogate_type_is_class(type):
            classtype = "class"
        elif interrogate_type_is_union(type):
            classtype = "union"
        else:
            print "I don't know what type %s is" % interrogate_type_true_name(type)
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

    for i_element in xrange(interrogate_type_number_of_elements(type)):
        processElement(handle, interrogate_type_get_element(type, i_element))
    
    print >>handle, "};"

def processModule(handle, package):
    print >>handle, "namespace %s {" % package

    if package != "core":
        print >>handle, "using namespace core;"

    for i_type in xrange(interrogate_number_of_global_types()):
        type = interrogate_get_global_type(i_type)

        if interrogate_type_has_module_name(type):
            module_name = interrogate_type_module_name(type)
            if "panda3d." + package == module_name:
                processType(handle, type)
        else:
            print "Type %s has no module name" % typename

    for i_func in xrange(interrogate_number_of_global_functions()):
        func = interrogate_get_global_function(i_func)

        if interrogate_function_has_module_name(func):
            module_name = interrogate_function_module_name(func)
            if "panda3d." + package == module_name:
                processFunction(handle, func)
        else:
            print "Type %s has no module name" % typename

    print >>handle, "}"


if __name__ == "__main__":
    handle = open("pandadoc.hpp", "w")
    
    print >>handle, comment("Panda3D modules that are implemented in C++.")
    print >>handle, "namespace panda3d {"
    
    # Determine the path to the interrogatedb files
    interrogate_add_search_directory(os.path.join(os.path.dirname(pandac.__file__), "..", "..", "etc"))
    interrogate_add_search_directory(os.path.join(os.path.dirname(pandac.__file__), "input"))

    import panda3d.core
    processModule(handle, "core")

    for lib in os.listdir(os.path.dirname(panda3d.__file__)):
        if lib.endswith(('.pyd', '.so')) and not lib.startswith('core.'):
            module_name = os.path.splitext(lib)[0]
            __import__("panda3d." + module_name)
            processModule(handle, module_name)


    print >>handle, "}"
    handle.close()
