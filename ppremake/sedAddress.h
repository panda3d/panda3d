// Filename: sedAddress.h
// Created by:  drose (24Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SEDADDRESS_H
#define SEDADDRESS_H

#include "ppremake.h"

#include <sys/types.h>

#ifdef HAVE_REGEX_H
#include <regex.h>
#else
#include "gnu_regex.h"
#endif

class SedContext;

///////////////////////////////////////////////////////////////////
//       Class : SedAddress
// Description : This represents a single address in a sed command,
//               something like a line number or a regular expression.
////////////////////////////////////////////////////////////////////
class SedAddress {
public:
  SedAddress();
  ~SedAddress();

  bool parse_address(const string &line, size_t &p);

  bool matches(const SedContext &context) const;
  bool precedes(const SedContext &context) const;

private:
  enum AddressType {
    AT_invalid,
    AT_numeric,
    AT_last,
    AT_re,
  };
  AddressType _address_type;

  int _number;
  regex_t _re;
};

#endif
