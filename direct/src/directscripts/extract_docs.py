""" This script generates a pandadoc.hpp file representing the Python
wrappers that can be parsed by doxygen to generate the Python documentation.
You need to run this before invoking Doxyfile.python.

It requires a valid makepanda installation with interrogatedb .in
files in the lib/pandac/input directory. """

from __future__ import print_function

__all__ = []

import os, sys
from distutils import sysconfig
import panda3d, pandac
from panda3d.interrogatedb import *


if 'interrogate_element_is_sequence' not in globals():
    def interrogate_element_is_sequence(element):
        return False

if 'interrogate_element_is_mapping' not in globals():
    def interrogate_element_is_mapping(element):
        return False


LICENSE = """PANDA 3D SOFTWARE
Copyright (c) Carnegie Mellon University.  All rights reserved.
All use of this software is subject to the terms of the revised BSD
license.  You should have received a copy of this license along
with this source code in a file named \"LICENSE.\"""".split("\n")

MAINPAGE = """@mainpage Panda3D Python API Reference
Welcome to the Panda3D API reference.

Use the links at the top of this page to browse through the list of modules or
the list of classes.

This reference is automatically generated from comments in the source code.
"""


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
    code = code.strip()

    if not code.startswith('///<') and '@verbatim' not in code:
        code = code.replace('<', '\\<').replace('>', '\\>')

    if not code or code[0] != '/':
        # Not really a comment; get rid of it.
        return ""

    return code

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
    elif typename == "size_t":
        return "int"

    if interrogate_type_is_atomic(type):
        token = interrogate_type_atomic_token(type)
        if token == 7:
            return 'str'
        elif token == 8:
            return 'long'
        elif token == 9:
            return 'NoneType'
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
        print(comment(interrogate_element_comment(element)), file=handle)
    elif interrogate_element_has_getter(element):
        # If the property has no comment, use the comment of the getter.
        getter = interrogate_element_getter(element)
        if interrogate_function_has_comment(getter):
            print(block_comment(interrogate_function_comment(getter)), file=handle)

    if interrogate_element_is_mapping(element) or \
       interrogate_element_is_sequence(element):
        suffix = "[]"
    else:
        suffix = ""

    print(translated_type_name(interrogate_element_type(element)), end=' ', file=handle)
    print(interrogate_element_name(element) + suffix + ';', file=handle)


def processFunction(handle, function, isConstructor = False):
    for i_wrapper in range(interrogate_function_number_of_python_wrappers(function)):
        wrapper = interrogate_function_python_wrapper(function, i_wrapper)
        if interrogate_wrapper_has_comment(wrapper):
            print(block_comment(interrogate_wrapper_comment(wrapper)), file=handle)

        if not isConstructor:
            if interrogate_function_is_method(function):
                if not interrogate_wrapper_number_of_parameters(wrapper) > 0 or not interrogate_wrapper_parameter_is_this(wrapper, 0):
                    print("static", end=' ', file=handle)

            if interrogate_wrapper_has_return_value(wrapper):
                print(translated_type_name(interrogate_wrapper_return_type(wrapper)), end=' ', file=handle)
            else:
                pass#print >>handle, "void",

            print(translateFunctionName(interrogate_function_name(function)) + "(", end=' ', file=handle)
        else:
            print("__init__(", end=' ', file=handle)

        first = True
        for i_param in range(interrogate_wrapper_number_of_parameters(wrapper)):
            if not interrogate_wrapper_parameter_is_this(wrapper, i_param):
                if not first:
                    print(",", end=' ', file=handle)
                print(translated_type_name(interrogate_wrapper_parameter_type(wrapper, i_param)), end=' ', file=handle)
                if interrogate_wrapper_parameter_has_name(wrapper, i_param):
                    print(interrogate_wrapper_parameter_name(wrapper, i_param), end=' ', file=handle)
                first = False

        print(");", file=handle)


def processType(handle, type):
    typename = translated_type_name(type, scoped=False)
    derivations = [ translated_type_name(interrogate_type_get_derivation(type, n)) for n in range(interrogate_type_number_of_derivations(type)) ]

    if interrogate_type_has_comment(type):
        print(block_comment(interrogate_type_comment(type)), file=handle)

    if interrogate_type_is_enum(type):
        print("enum %s {" % typename, file=handle)
        for i_value in range(interrogate_type_number_of_enum_values(type)):
            docstring = comment(interrogate_type_enum_value_comment(type, i_value))
            if docstring:
                print(docstring, file=handle)
            print(interrogate_type_enum_value_name(type, i_value), "=", interrogate_type_enum_value(type, i_value), ",", file=handle)

    elif interrogate_type_is_typedef(type):
        wrapped_type = interrogate_type_wrapped_type(type)
        if interrogate_type_is_global(wrapped_type):
            wrapped_type_name = translated_type_name(wrapped_type)
            print("typedef %s %s;" % (wrapped_type_name, typename), file=handle)
        return
    else:
        if interrogate_type_is_struct(type):
            classtype = "struct"
        elif interrogate_type_is_class(type):
            classtype = "class"
        elif interrogate_type_is_union(type):
            classtype = "union"
        else:
            print("I don't know what type %s is" % interrogate_type_true_name(type))
            return

        if len(derivations) > 0:
            print("%s %s : public %s {" % (classtype, typename, ", public ".join(derivations)), file=handle)
        else:
            print("%s %s {" % (classtype, typename), file=handle)
        print("public:", file=handle)

    for i_ntype in range(interrogate_type_number_of_nested_types(type)):
        processType(handle, interrogate_type_get_nested_type(type, i_ntype))

    for i_method in range(interrogate_type_number_of_constructors(type)):
        processFunction(handle, interrogate_type_get_constructor(type, i_method), True)

    for i_method in range(interrogate_type_number_of_methods(type)):
        processFunction(handle, interrogate_type_get_method(type, i_method))

    for i_method in range(interrogate_type_number_of_make_seqs(type)):
        print("list", translateFunctionName(interrogate_make_seq_seq_name(interrogate_type_get_make_seq(type, i_method))), "();", file=handle)

    for i_element in range(interrogate_type_number_of_elements(type)):
        processElement(handle, interrogate_type_get_element(type, i_element))

    print("};", file=handle)

def processModule(handle, package):
    print("Processing module %s" % (package))
    print("namespace %s {" % package, file=handle)

    if package != "core":
        print("using namespace core;", file=handle)

    for i_type in range(interrogate_number_of_global_types()):
        type = interrogate_get_global_type(i_type)

        if interrogate_type_has_module_name(type):
            module_name = interrogate_type_module_name(type)
            if "panda3d." + package == module_name:
                processType(handle, type)
        else:
            typename = interrogate_type_name(type)
            print("Type %s has no module name" % typename)

    for i_func in range(interrogate_number_of_global_functions()):
        func = interrogate_get_global_function(i_func)

        if interrogate_function_has_module_name(func):
            module_name = interrogate_function_module_name(func)
            if "panda3d." + package == module_name:
                processFunction(handle, func)
        else:
            funcname = interrogate_function_name(func)
            print("Function %s has no module name" % funcname)

    print("}", file=handle)


if __name__ == "__main__":
    handle = open("pandadoc.hpp", "w")

    mainpage = MAINPAGE.strip()
    if mainpage:
        print("/**\n * " + mainpage.replace('\n', '\n * ') + '\n */', file=handle)

    print(comment("Panda3D modules that are implemented in C++."), file=handle)
    print("namespace panda3d {", file=handle)

    # Determine the path to the interrogatedb files
    pandac = os.path.dirname(pandac.__file__)
    interrogate_add_search_directory(os.path.join(pandac, "..", "..", "etc"))
    interrogate_add_search_directory(os.path.join(pandac, "input"))

    import panda3d.core
    processModule(handle, "core")

    # Determine the suffix for the extension modules.
    if sys.version_info >= (3, 0):
        import _imp
        ext_suffix = _imp.extension_suffixes()[0]
    elif sys.platform == "win32":
        ext_suffix = ".pyd"
    else:
        ext_suffix = ".so"

    for lib in os.listdir(os.path.dirname(panda3d.__file__)):
        if lib.endswith(ext_suffix) and not lib.startswith('core.'):
            module_name = lib[:-len(ext_suffix)]
            __import__("panda3d." + module_name)
            processModule(handle, module_name)

    print("}", file=handle)
    handle.close()

    print("Wrote output to pandadoc.hpp.  You can now run:")
    print()
    print("  doxygen built/direct/directscripts/Doxyfile.python")
