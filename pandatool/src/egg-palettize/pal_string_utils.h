// Filename: pal_string_utils.h
// Created by:  drose (30Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PAL_STRING_UTILS_H
#define PAL_STRING_UTILS_H

#include <pandatoolbase.h>
#include <string_utils.h>

class PNMFileType;

void extract_param_value(const string &str, string &param, string &value);

bool parse_image_type_request(const string &word, PNMFileType *&color_type,
                              PNMFileType *&alpha_type);

#endif

