// Filename: ppNamedScopes.h
// Created by:  drose (27Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PPNAMEDSCOPES_H
#define PPNAMEDSCOPES_H

#include "ppremake.h"

#include <map>
#include <vector>

class PPScope;

///////////////////////////////////////////////////////////////////
//       Class : PPNamedScopes
// Description : A collection of named scopes, as defined by #begin
//               .. #end sequences within a series of command files,
//               each associated with the directory name of the
//               command file in which it was read.
////////////////////////////////////////////////////////////////////
class PPNamedScopes {
public:
  PPNamedScopes();
  ~PPNamedScopes();

  typedef vector<PPScope *> Scopes;

  PPScope *make_scope(const string &name);
  void get_scopes(const string &name, Scopes &scopes) const;
  static void sort_by_dependency(Scopes &scopes);

  void set_current(const string &dirname);

private:  
  typedef map<string, Scopes> Named;

  void p_get_scopes(const Named &named, const string &name,
                    Scopes &scopes) const;

  typedef map<string, Named> Directories;
  Directories _directories;
  string _current;
};

#endif

