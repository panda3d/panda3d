// Filename: pal_string_utils.h
// Created by:  drose (30Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PAL_STRING_UTILS_H
#define PAL_STRING_UTILS_H

#include <pandatoolbase.h>
#include <vector_string.h>

class PNMFileType;

string trim_left(const string &str);
string trim_right(const string &str);

int string_to_int(const string &str, string &tail);
bool string_to_int(const string &str, int &result);
double string_to_double(const string &str, string &tail);
bool string_to_double(const string &str, double &result);

void extract_words(const string &str, vector_string &words);
void extract_param_value(const string &str, string &param, string &value);

bool parse_image_type_request(const string &word, PNMFileType *&color_type,
			      PNMFileType *&alpha_type);

#endif

