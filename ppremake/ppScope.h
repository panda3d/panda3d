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
class PPDirectory;
class PPSubroutine;

///////////////////////////////////////////////////////////////////
// 	 Class : PPScope
// Description : Defines a (possibly nested) scope for variable
//               definitions.  Variables may be defined in a
//               system-wide variable file, in a template file, or in
//               an individual source file.
////////////////////////////////////////////////////////////////////
class PPScope {
public:
  typedef map<string, PPScope *> MapVariableDefinition;

  PPScope(PPNamedScopes *named_scopes);

  PPNamedScopes *get_named_scopes() const;

  void set_parent(PPScope *parent);
  PPScope *get_parent() const;

  void define_variable(const string &varname, const string &definition);
  bool set_variable(const string &varname, const string &definition);
  void define_map_variable(const string &varname, const string &definition);
  void define_map_variable(const string &varname, const string &key_varname,
			   const string &scope_names);
  void add_to_map_variable(const string &varname, const string &key,
			   PPScope *scope);
  void define_formals(const string &subroutine_name,
		      const vector<string> &formals, const string &actuals);

  string get_variable(const string &varname) const;
  string expand_variable(const string &varname) const;
  MapVariableDefinition &find_map_variable(const string &varname) const;

  PPDirectory *get_directory() const;
  void set_directory(PPDirectory *directory);

  string expand_string(const string &str) const;
  string expand_self_reference(const string &str, const string &varname) const;

  static void push_scope(PPScope *scope);
  static PPScope *pop_scope();
  static PPScope *get_bottom_scope();

  void tokenize_params(const string &str, vector<string> &tokens,
		       bool expand) const;
  bool tokenize_numeric_pair(const string &str, double &a, double &b) const;

  static MapVariableDefinition _null_map_def;

private:
  class ExpandedVariable {
  public:
    string _varname;
    ExpandedVariable *_next;
  };

  bool p_set_variable(const string &varname, const string &definition);
  bool p_get_variable(const string &varname, string &result) const;

  string r_expand_string(const string &str, ExpandedVariable *expanded) const;
  string r_scan_variable(const string &str, size_t &vp) const;
  string r_expand_variable(const string &str, size_t &vp,
			   PPScope::ExpandedVariable *expanded) const;
  string expand_variable_nested(const string &varname, 
				const string &scope_names) const;

  string expand_wildcard(const string &params) const;
  string expand_isdir(const string &params) const;
  string expand_isfile(const string &params) const;
  string expand_libtest(const string &params) const;
  string expand_bintest(const string &params) const;
  string expand_shell(const string &params) const;
  string expand_standardize(const string &params) const;
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
  string expand_eqn(const string &params) const;
  string expand_nen(const string &params) const;
  string expand_ltn(const string &params) const;
  string expand_len(const string &params) const;
  string expand_gtn(const string &params) const;
  string expand_gen(const string &params) const;
  string expand_not(const string &params) const;
  string expand_or(const string &params) const;
  string expand_and(const string &params) const;
  string expand_upcase(const string &params) const;
  string expand_downcase(const string &params) const;
  string expand_cdefine(const string &params) const;
  string expand_closure(const string &params) const;
  string expand_unmapped(const string &params) const;
  string expand_dependencies(const string &params) const;
  string expand_function(const string &funcname, const PPSubroutine *sub,
			 const string &params) const;
  string expand_map_variable(const string &varname, const string &params) const;
  string expand_map_variable(const string &varname, const string &expression,
			     const vector<string> &keys) const;

  MapVariableDefinition &
  p_find_map_variable(const string &varname) const;

  void glob_string(const string &str, vector<string> &results) const;

  PPNamedScopes *_named_scopes;

  PPDirectory *_directory;

  typedef map<string, string> Variables;
  Variables _variables;

  typedef map<string, MapVariableDefinition> MapVariables;
  MapVariables _map_variables;

  PPScope *_parent_scope;
  typedef vector<PPScope *> ScopeStack;
  static ScopeStack _scope_stack;
};


#endif
