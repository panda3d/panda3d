// Filename: tokenize.h
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "ppremake.h"

#include <vector>

// A couple of handy functions for breaking up a string into tokens,
// and repasting the tokens back into a string.

void tokenize(const string &source, vector<string> &tokens,
              const string &delimiters);

void tokenize_whitespace(const string &source, vector<string> &tokens);

string repaste(const vector<string> &tokens, const string &separator);

// And this is just handy to have.
string trim_blanks(const string &str);

// So is this.
bool contains_whitespace(const string &str);

#endif

