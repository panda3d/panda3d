// Filename: pal_string_utils.cxx
// Created by:  drose (30Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "pal_string_utils.h"

#include <pnmFileType.h>
#include <pnmFileTypeRegistry.h>


string 
trim_left(const string &str) {
  size_t begin = 0;
  while (begin < str.size() && isspace(str[begin])) {
    begin++;
  }

  return str.substr(begin);
}

string 
trim_right(const string &str) {
  size_t begin = 0;
  size_t end = str.size();
  while (end > begin && isspace(str[end - 1])) {
    end--;
  }

  return str.substr(begin, end - begin);
}

////////////////////////////////////////////////////////////////////
//     Function: string_to_int
//  Description: A string-interface wrapper around the C library
//               strtol().  This parses the ASCII representation of an
//               integer, and then sets tail to everything that
//               follows the first valid integer read.  If, on exit,
//               str == tail, there was no valid integer in the
//               source string; if !tail.empty(), there was garbage
//               after the integer.
//
//               It is legal if str and tail refer to the same string.
////////////////////////////////////////////////////////////////////
int
string_to_int(const string &str, string &tail) {
  const char *nptr = str.c_str();
  char *endptr;
  int result = strtol(nptr, &endptr, 10);
  tail = endptr;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: string_to_int
//  Description: Another flavor of string_to_int(), this one returns
//               true if the string is a perfectly valid integer (and
//               sets result to that value), or false otherwise.
////////////////////////////////////////////////////////////////////
bool
string_to_int(const string &str, int &result) {
  string tail;
  result = string_to_int(str, tail);
  return tail.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: string_to_double
//  Description: A string-interface wrapper around the C library
//               strtol().  This parses the ASCII representation of an
//               floating-point number, and then sets tail to
//               everything that follows the first valid integer read.
//               If, on exit, str == tail, there was no valid integer
//               in the source string; if !tail.empty(), there was
//               garbage after the number.
//
//               It is legal if str and tail refer to the same string.
////////////////////////////////////////////////////////////////////
double
string_to_double(const string &str, string &tail) {
  const char *nptr = str.c_str();
  char *endptr;
  double result = strtod(nptr, &endptr);
  tail = endptr;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: string_to_double
//  Description: Another flavor of string_to_double(), this one
//               returns true if the string is a perfectly valid
//               number (and sets result to that value), or false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool
string_to_double(const string &str, double &result) {
  string tail;
  result = string_to_double(str, tail);
  return tail.empty();
}

void
extract_words(const string &str, vector_string &words) {
  size_t pos = 0;
  while (pos < str.length() && isspace(str[pos])) {
    pos++;
  }
  while (pos < str.length()) {
    size_t word_start = pos;
    while (pos < str.length() && !isspace(str[pos])) {
      pos++;
    }
    words.push_back(str.substr(word_start, pos - word_start));

    while (pos < str.length() && isspace(str[pos])) {
      pos++;
    }
  }
}

// Extracts the first word of the string into param, and the remainder
// of the line into value.
void 
extract_param_value(const string &str, string &param, string &value) {
  size_t i = 0;

  // First, skip all whitespace at the beginning.
  while (i < str.length() && isspace(str[i])) {
    i++;
  }

  size_t start = i;

  // Now skip to the end of the whitespace.
  while (i < str.length() && !isspace(str[i])) {
    i++;
  }

  size_t end = i;

  param = str.substr(start, end - start);

  // Skip a little bit further to the start of the value.
  while (i < str.length() && isspace(str[i])) {
    i++;
  }
  value = trim_right(str.substr(i));
}


bool
parse_image_type_request(const string &word, PNMFileType *&color_type,
			 PNMFileType *&alpha_type) {
  PNMFileTypeRegistry *registry = PNMFileTypeRegistry::get_ptr();
  color_type = (PNMFileType *)NULL;
  alpha_type = (PNMFileType *)NULL;

  string color_name = word;
  string alpha_name;
  size_t comma = word.find(',');
  if (comma != string::npos) {
    // If we have a comma in the image_type, it's two types: a color
    // type and an alpha type.
    color_name = word.substr(0, comma);
    alpha_name = word.substr(comma + 1);
  }
  
  if (!color_name.empty()) {
    color_type = registry->get_type_from_extension(color_name);
    if (color_type == (PNMFileType *)NULL) {
      nout << "Image file type '" << color_name << "' is unknown.\n";
      return false;
    }
  }
  
  if (!alpha_name.empty()) {
    alpha_type = registry->get_type_from_extension(alpha_name);
    if (alpha_type == (PNMFileType *)NULL) {
      nout << "Image file type '" << alpha_name << "' is unknown.\n";
      return false;
    }
  }

  return true;
}


