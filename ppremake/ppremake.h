/*
// Filename: ppremake.h
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////
*/

#ifndef PPREMAKE_H
#define PPREMAKE_H

#include "config.h"

#ifdef __cplusplus
#ifdef HAVE_IOSTREAM
#include <iostream>
#include <fstream>
#include <strstream>
#else
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#endif

#include <string>

#ifdef HAVE_NAMESPACE
using namespace std;
#endif
#endif /* __cplusplus */

#define PACKAGE_FILENAME "Package.pp"
#define SOURCE_FILENAME "Sources.pp"

#define COMMAND_PREFIX '#'
#define VARIABLE_PREFIX '$'
#define VARIABLE_OPEN_BRACE '['
#define VARIABLE_CLOSE_BRACE ']'
#define PATTERN_WILDCARD '%'
#define BEGIN_COMMENT "//"

#define FUNCTION_PARAMETER_SEPARATOR ','

#define VARIABLE_OPEN_NESTED '('
#define VARIABLE_CLOSE_NESTED ')'
#define VARIABLE_PATSUBST ":"
#define VARIABLE_PATSUBST_DELIM "="

#define SCOPE_DIRNAME_SEPARATOR '/'
#define SCOPE_DIRNAME_WILDCARD "*"
#define SCOPE_DIRNAME_CURRENT "."

#endif
