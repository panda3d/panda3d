// Filename: string_utils.cxx
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "string_utils.h"


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

vector<string>
extract_words(const string &str) {
  vector<string> result;

  size_t pos = 0;
  while (pos < str.length() && isspace(str[pos])) {
    pos++;
  }
  while (pos < str.length()) {
    size_t word_start = pos;
    while (pos < str.length() && !isspace(str[pos])) {
      pos++;
    }
    result.push_back(str.substr(word_start, pos - word_start));

    while (pos < str.length() && isspace(str[pos])) {
      pos++;
    }
  }

  return result;
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
