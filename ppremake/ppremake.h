/*
// Filename: ppremake.h
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////
*/

#ifndef PPREMAKE_H
#define PPREMAKE_H

#ifdef _MSC_VER
  /* For Visual C, include the special config.h file. */
  #include "config_msvc.h"
#else
  /* Otherwise, include the normal automatically-generated file. */
  #include "config.h"
#endif

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

#ifndef HAVE_ALLOCA_H
  /* If we don't have alloca.h, use malloc() to implement gnu_regex. */
  #define REGEX_MALLOC 1
#endif

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

/* These are set from the similarly-named variables defined in
   System.pp. */
#ifdef __cplusplus
extern bool unix_platform;
extern bool windows_platform;
#endif

#endif
