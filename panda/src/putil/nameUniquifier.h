// Filename: nameUniquifier.h
// Created by:  drose (16Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef NAMEUNIQUIFIER_H
#define NAMEUNIQUIFIER_H

#include <pandabase.h>

#include <string>
#include <set>

////////////////////////////////////////////////////////////////////
// 	 Class : NameUniquifier
// Description : A handy class for converting a list of arbitrary
//               names (strings) so that each name is guaranteed to be
//               unique in the list.  Useful for writing egg files
//               with unique vertex pool names, or for file converters
//               to file formats that require unique node names, etc.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NameUniquifier {
public:
  INLINE NameUniquifier(const string &separator = string(),
			const string &empty = string());

  INLINE string add_name(const string &name);
  INLINE string add_name(const string &name, const string &prefix);

private:
  string add_name_body(const string &name, const string &prefix);

  typedef set<string> Names;
  Names _names;
  string _separator;
  string _empty;
  int _counter;
};

#include "nameUniquifier.I"

#endif
