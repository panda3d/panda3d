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
//   Class : PPScope
// Description : Defines a (possibly nested) scope for variable
//               definitions.  Variables may be defined in a
//               system-wide variable file, in a template file, or in
//               an individual source file.
////////////////////////////////////////////////////////////////////
class PPScope {
public:
  typedef map<string, PPScope *> MapVariableDefinition;

  PPScope(PPNamedScopes *named_scopes);

  PPNamedScopes *get_named_scopes();

  void set_parent(PPScope *parent);
  PPScope *get_parent();

  void define_variable(const string &varname, const string &definition);
  bool set_variable(const string &varname, const string &definition);
  void define_map_variable(const string &varname, const string &definition);
  void define_map_variable(const string &varname, const string &key_varname,
               const string &scope_names);
  void add_to_map_variable(const string &varname, const string &key,
               PPScope *scope);
  void define_formals(const string &subroutine_name,
              const vector<string> &formals, const string &actuals);

  string get_variable(const string &varname);
  string expand_variable(const string &varname);
  MapVariableDefinition &find_map_variable(const string &varname);

  PPDirectory *get_directory();
  void set_directory(PPDirectory *directory);

  string expand_string(const string &str);
  string expand_self_reference(const string &str, const string &varname);

  static void push_scope(PPScope *scope);
  static PPScope *pop_scope();
  static PPScope *get_bottom_scope();
  static PPScope *get_enclosing_scope(int n);

  void tokenize_params(const string &str, vector<string> &tokens,
                       bool expand);
  bool tokenize_numeric_pair(const string &str, double &a, double &b);
  bool tokenize_ints(const string &str, vector<int> &tokens);
  size_t scan_to_whitespace(const string &str, size_t start = 0);
  static string format_int(int num);

  static MapVariableDefinition _null_map_def;

private:
  class ExpandedVariable {
  public:
    string _varname;
    ExpandedVariable *_next;
  };

  bool p_set_variable(const string &varname, const string &definition);
  bool p_get_variable(const string &varname, string &result);

  string r_expand_string(const string &str, ExpandedVariable *expanded);
  string r_scan_variable(const string &str, size_t &vp);
  string r_expand_variable(const string &str, size_t &vp,
               PPScope::ExpandedVariable *expanded);
  string expand_variable_nested(const string &varname, 
                const string &scope_names);

  string expand_isfullpath(const string &params);
  string expand_osfilename(const string &params);
  string expand_unixfilename(const string &params);
  string expand_unixshortname(const string &params);
  string expand_cygpath_w(const string &params);
  string expand_cygpath_p(const string &params);
  string expand_wildcard(const string &params);
  string expand_isdir(const string &params);
  string expand_isfile(const string &params);
  string expand_libtest(const string &params);
  string expand_bintest(const string &params);
  string expand_shell(const string &params);
  string expand_standardize(const string &params);
  string expand_canonical(const string &params);
  string expand_length(const string &params);
  string expand_substr(const string &params);
  string expand_findstring(const string &params);
  string expand_dir(const string &params);
  string expand_notdir(const string &params);
  string expand_suffix(const string &params);
  string expand_basename(const string &params);
  string expand_makeguid(const string &params);
  string expand_word(const string &params);
  string expand_wordlist(const string &params);
  string expand_words(const string &params);
  string expand_firstword(const string &params);
  string expand_patsubst(const string &params, bool separate_words);
  string expand_filter(const string &params);
  string expand_filter_out(const string &params);
  string expand_wordsubst(const string &params);
  string expand_subst(const string &params);
  string expand_join(const string &params);
  string expand_sort(const string &params);
  string expand_unique(const string &params);
  string expand_matrix(const string &params);
  string expand_if(const string &params);
  string expand_defined(const string &params);
  string expand_eq(const string &params);
  string expand_ne(const string &params);
  string expand_eqn(const string &params);
  string expand_nen(const string &params);
  string expand_ltn(const string &params);
  string expand_len(const string &params);
  string expand_gtn(const string &params);
  string expand_gen(const string &params);
  string expand_plus(const string &params);
  string expand_minus(const string &params);
  string expand_times(const string &params);
  string expand_divide(const string &params);
  string expand_modulo(const string &params);
  string expand_not(const string &params);
  string expand_or(const string &params);
  string expand_and(const string &params);
  string expand_upcase(const string &params);
  string expand_downcase(const string &params);
  string expand_cdefine(const string &params);
  string expand_closure(const string &params);
  string expand_unmapped(const string &params);
  string expand_dependencies(const string &params);
  string expand_foreach(const string &params);
  string expand_forscopes(const string &params);
  string expand_function(const string &funcname, const PPSubroutine *sub,
             const string &params);
  string expand_map_variable(const string &varname, const string &params);
  string expand_map_variable(const string &varname, const string &expression,
                 const vector<string> &keys);
  
  void
  r_expand_matrix(vector<string> &results,
          const vector<vector<string> > &words,
          int index, const string &prefix);

  MapVariableDefinition &
  p_find_map_variable(const string &varname);

  void glob_string(const string &str, vector<string> &results);

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
