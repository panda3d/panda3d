// Filename: string_utils.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <pandatoolbase.h>
#include <vector_string.h>

#include <vector>

string trim_left(const string &str);
string trim_right(const string &str);

vector_string extract_words(const string &str);
void extract_param_value(const string &str, string &param, string &value);

#endif

