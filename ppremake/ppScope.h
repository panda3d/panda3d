// Filename: ppScope.h
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PPSCOPE_H
#define PPSCOPE_H

#include "ppremake.h"

#include <map>
#include <vector>

class PPNamedScopes;
class PPDirectoryTree;

///////////////////////////////////////////////////////////////////
// 	 Class : PPScope
// Description : Defines a (possibly nested) scope for variable
//               definitions.  Variables may be defined in a
//               system-wide variable file, in a template file, or in
//               an individual source file.
////////////////////////////////////////////////////////////////////
class PPScope {
public:
  PPScope(PPNamedScopes *named_scopes);

  PPNamedScopes *get_named_scopes() const;

  void set_parent(PPScope *parent);
  PPScope *get_parent() const;

  void define_variable(const string &varname, const string &definition);
  bool set_variable(const string &varname, const string &definition);
  void define_map_variable(const string &varname, const string &definition);
  void define_map_variable(const string &varname, const string &key_varname,
			   const string &scope_names);

  string get_variable(const string &varname) const;
  string expand_variable(const string &varname) const;

  PPDirectoryTree *get_directory() const;
  void set_directory(PPDirectoryTree *directory);

  string expand_string(const string &str) const;
  string expand_self_reference(const string &str, const string &varname) const;

  static void push_scope(PPScope *scope);
  static PPScope *pop_scope();
  static PPScope *get_bottom_scope();

private:
  class ExpandedVariable {
  public:
    string _varname;
    ExpandedVariable *_next;
  };

  typedef map<string, PPScope *> MapVariableDefinition;

  bool p_set_variable(const string &varname, const string &definition);
  bool p_get_variable(const string &varname, string &result) const;

  void tokenize_params(const string &str, vector<string> &tokens,
		       bool expand) const;

  string r_expand_string(const string &str, ExpandedVariable *expanded) const;
  string r_scan_variable(const string &str, size_t &vp) const;
  string r_expand_variable(const string &str, size_t &vp,
			   PPScope::ExpandedVariable *expanded) const;
  string expand_variable_nested(const string &varname, 
				const string &scope_names) const;

  string expand_wildcard(const string &params) const;
  string expand_isdir(const string &params) const;
  string expand_libtest(const string &params) const;
  string expand_bintest(const string &params) const;
  string expand_shell(const string &params) const;
  string expand_firstword(const string &params) const;
  string expand_patsubst(const string &params) const;
  string expand_filter(const string &params) const;
  string expand_filter_out(const string &params) const;
  string expand_subst(const string &params) const;
  string expand_sort(const string &params) const;
  string expand_unique(const string &params) const;
  string expand_if(const string &params) const;
  string expand_eq(const string &params) const;
  string expand_ne(const string &params) const;
  string expand_not(const string &params) const;
  string expand_or(const string &params) const;
  string expand_and(const string &params) const;
  string expand_upcase(const string &params) const;
  string expand_downcase(const string &params) const;
  string expand_closure(const string &params) const;
  string expand_map_variable(const string &varname, const string &params) const;
  string expand_map_variable(const string &varname, const string &expression,
			     const vector<string> &keys) const;

  const MapVariableDefinition &
  find_map_variable(const string &varname) const;
  const MapVariableDefinition &
  p_find_map_variable(const string &varname) const;

  void glob_string(const string &str, vector<string> &results) const;

  PPNamedScopes *_named_scopes;

  PPDirectoryTree *_directory;

  typedef map<string, string> Variables;
  Variables _variables;

  typedef map<string, MapVariableDefinition> MapVariables;
  MapVariables _map_variables;
  static MapVariableDefinition _null_map_def;

  PPScope *_parent_scope;
  typedef vector<PPScope *> ScopeStack;
  static ScopeStack _scope_stack;
};


#endif
