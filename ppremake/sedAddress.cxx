// Filename: sedAddress.cxx
// Created by:  drose (24Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "sedAddress.h"
#include "sedContext.h"

#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

////////////////////////////////////////////////////////////////////
//     Function: SedAddress::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SedAddress::
SedAddress() {
  _address_type = AT_invalid;
}

////////////////////////////////////////////////////////////////////
//     Function: SedAddress::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SedAddress::
~SedAddress() {
  if (_address_type == AT_re) {
    regfree(&_re);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SedAddress::parse_address
//       Access: Public
//  Description: Scans the indicated string beginning at the indicated
//               character position for an address specification,
//               e.g. a number, $, or a regular expression.  If a
//               correct address is found, increments p to the first
//               non-whitespace character past it and returns true;
//               otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool SedAddress::
parse_address(const string &line, size_t &p) {
  assert(p < line.length());
  if (line[p] == '$') {
    p++;
    _address_type = AT_last;

  } else if (isdigit(line[p])) {
    const char *str = line.c_str() + p;
    char *end;
    _number = strtol(str, &end, 10);
    _address_type = AT_numeric;
    p += (end - str);

  } else {
    // It must be a regular expression.
    size_t p0 = p;
    char delimiter = line[p];
    p++;
    if (p < line.length() && delimiter == '\\') {
      // A backslash might escape the opening character.
      delimiter = line[p];
      p++;
    }

    size_t begin = p;
    while (p < line.length() && line[p] != delimiter) {
      if (line[p] == '\\') {
        p++;
        // A backslash could escape the closing character.
      }
      p++;
    }

    if (p >= line.length()) {
      cerr << "Could not find terminating character '" << delimiter
           << "' in regular expression: " << line.substr(p0) << "\n";
      return false;
    }

    string re = line.substr(begin, p - begin);
    p++;

    int error = regcomp(&_re, re.c_str(), REG_NOSUB);
    if (error != 0) {
      static const int errbuf_size = 512;
      char errbuf[errbuf_size];
      regerror(error, &_re, errbuf, errbuf_size);

      cerr << "Invalid regular expression: " << re << "\n"
           << errbuf << "\n";
      return false;
    }

    _address_type = AT_re;
  }

  // Skip whitespace following the address.
  while (p < line.length() && isspace(line[p])) {
    p++;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SedAddress::matches
//       Access: Public
//  Description: Returns true if this address exactly matches the
//               current pattern space.
////////////////////////////////////////////////////////////////////
bool SedAddress::
matches(const SedContext &context) const {
  switch (_address_type) {
  case AT_invalid:
    cerr << "Internal error!\n";
    assert(false);
    return false;

  case AT_numeric:
    return (_number == context._line_number);

  case AT_last:
    return context._is_last_line;

  case AT_re:
    return (regexec(&_re, context._pattern_space.c_str(), 0, (regmatch_t *)NULL, 0) == 0);
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: SedAddress::precedes
//       Access: Public
//  Description: Returns true if this address exactly matches the
//               current line or refers to a previous line.  This
//               never returns true if the address is a regular
//               expression type.
////////////////////////////////////////////////////////////////////
bool SedAddress::
precedes(const SedContext &context) const {
  if (_address_type == AT_numeric) {
    return (_number <= context._line_number);
  }

  return false;
}

