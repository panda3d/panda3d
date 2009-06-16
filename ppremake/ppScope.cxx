// Filename: ppScope.cxx
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "ppremake.h"
#include "ppScope.h"
#include "ppNamedScopes.h"
#include "ppFilenamePattern.h"
#include "ppDirectory.h"
#include "ppSubroutine.h"
#include "ppCommandFile.h"
#include "ppDependableFile.h"
#include "ppMain.h"
#include "tokenize.h"
#include "filename.h"
#include "dSearchPath.h"
#include "globPattern.h"
#include "md5.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <stdlib.h>
#include <algorithm>
#include <ctype.h>
#include <sys/stat.h>
#include <stdio.h>  // for perror() and sprintf().
#include <errno.h>
#include <signal.h>
#include <assert.h>

#ifdef WIN32_VC
#include <windows.h>  // for GetFileAttributes()
#endif  // WIN32_VC

static const string variable_patsubst(VARIABLE_PATSUBST);

PPScope::MapVariableDefinition PPScope::_null_map_def;

PPScope::ScopeStack PPScope::_scope_stack;

////////////////////////////////////////////////////////////////////
//     Function: PPScope::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPScope::
PPScope(PPNamedScopes *named_scopes) : 
  _named_scopes(named_scopes)
{
  _directory = (PPDirectory *)NULL;
  _parent_scope = (PPScope *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::get_named_scopes
//       Access: Public
//  Description: Returns a pointer to the PPNamedScopes collection
//               associated with this scope.  This pointer could be
//               NULL.
////////////////////////////////////////////////////////////////////
PPNamedScopes *PPScope::
get_named_scopes() {
  return _named_scopes;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::set_parent
//       Access: Public
//  Description: Sets a static parent scope to this scope.  When a
//               variable reference is undefined in this scope, it
//               will search first up the static parent chain before
//               it searches the dynamic scope stack.
////////////////////////////////////////////////////////////////////
void PPScope::
set_parent(PPScope *parent) {
  _parent_scope = parent;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::get_parent
//       Access: Public
//  Description: Returns the static parent scope to this scope, if
//               any, or NULL if the static parent has not been set.
//               See set_parent().
////////////////////////////////////////////////////////////////////
PPScope *PPScope::
get_parent() {
  return _parent_scope;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::define_variable
//       Access: Public
//  Description: Makes a new variable definition.  If the variable
//               does not already exist in this scope, a new variable
//               is created, possibly shadowing a variable declaration
//               in some parent scope.
////////////////////////////////////////////////////////////////////
void PPScope::
define_variable(const string &varname, const string &definition) {
  _variables[varname] = definition;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::set_variable
//       Access: Public
//  Description: Changes the definition of an already-existing
//               variable.  The variable is changed in whichever scope
//               it is defined.  Returns false if the variable has not
//               been defined.
////////////////////////////////////////////////////////////////////
bool PPScope::
set_variable(const string &varname, const string &definition) {
  if (p_set_variable(varname, definition)) {
    return true;
  }

  // Check the scopes on the stack for the variable definition.
  ScopeStack::reverse_iterator si;
  for (si = _scope_stack.rbegin(); si != _scope_stack.rend(); ++si) {
    if ((*si)->p_set_variable(varname, definition)) {
      return true;
    }
  }

  // If the variable isn't defined, we check the environment.
  const char *env = getenv(varname.c_str());
  if (env != (const char *)NULL) {
    // It is defined in the environment; thus, it is implicitly
    // defined here at the global scope: the bottom of the stack.
    PPScope *bottom = this;
    if (!_scope_stack.empty()) {
      bottom = _scope_stack.front();
    }
    bottom->define_variable(varname, definition);
    return true;
  }

  // The variable isn't defined anywhere.  Too bad.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::define_map_variable
//       Access: Public
//  Description: Makes a new map variable definition.  This defines a
//               new variable that can be used as a function to
//               retrieve variables from within a named scope, based
//               on a particular key variable.
//
//               In this variant of define_map_variable(), the
//               definition is a string of the form
//               key_varname(scope_names).
////////////////////////////////////////////////////////////////////
void PPScope::
define_map_variable(const string &varname, const string &definition) {
  size_t p = definition.find(VARIABLE_OPEN_NESTED);
  if (p != string::npos && definition[definition.length() - 1] == VARIABLE_CLOSE_NESTED) {
    size_t q = definition.length() - 1;
    string scope_names = definition.substr(p + 1, q - (p + 1));
    string key_varname = definition.substr(0, p);
    define_map_variable(varname, key_varname, scope_names);
  } else {
    // No scoping; not really a map variable.
    define_map_variable(varname, definition, "");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::define_map_variable
//       Access: Public
//  Description: Makes a new map variable definition.  This defines a
//               new variable that can be used as a function to
//               retrieve variables from within a named scope, based
//               on a particular key variable.
////////////////////////////////////////////////////////////////////
void PPScope::
define_map_variable(const string &varname, const string &key_varname,
                    const string &scope_names) {
  MapVariableDefinition &def = _map_variables[varname];
  def.clear();
  define_variable(varname, "");

  if (_named_scopes == (PPNamedScopes *)NULL) {
    return;
  }

  if (key_varname.empty()) {
    return;
  }

  vector<string> names;
  tokenize_whitespace(scope_names, names);

  // Get all of the named scopes.
  PPNamedScopes::Scopes scopes;
  
  vector<string>::const_iterator ni;
  for (ni = names.begin(); ni != names.end(); ++ni) {
    const string &name = (*ni);
    _named_scopes->get_scopes(name, scopes);
  }

  if (scopes.empty()) {
    return;
  }

  // Now go through the scopes and build up the results.
  vector<string> results;

  PPNamedScopes::Scopes::const_iterator si;
  for (si = scopes.begin(); si != scopes.end(); ++si) {
    PPScope *scope = (*si);
    string key_string = scope->expand_variable(key_varname);
    vector<string> keys;
    tokenize_whitespace(key_string, keys);

    if (!keys.empty()) {
      vector<string>::const_iterator ki;
      results.insert(results.end(), keys.begin(), keys.end());
      for (ki = keys.begin(); ki != keys.end(); ++ki) {
        def[*ki] = scope;
      }
    }
  }

  // Also define a traditional variable along with the map variable.
  define_variable(varname, repaste(results, " "));
}


////////////////////////////////////////////////////////////////////
//     Function: PPScope::add_to_map_variable
//       Access: Public
//  Description: Adds a new key/scope pair to a previous map variable
//               definition.
////////////////////////////////////////////////////////////////////
void PPScope::
add_to_map_variable(const string &varname, const string &key,
                    PPScope *scope) {
  MapVariableDefinition &def = find_map_variable(varname);
  if (&def == &_null_map_def) {
    cerr << "Warning:  undefined map variable: " << varname << "\n";
    return;
  }

  def[key] = scope;

  // We need to do all this work to define the traditional expansion.
  // Maybe not a great idea.
  vector<string> results;
  MapVariableDefinition::const_iterator di;
  for (di = def.begin(); di != def.end(); ++di) {
    results.push_back((*di).first);
  }

  set_variable(varname, repaste(results, " "));
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::define_formals
//       Access: Public
//  Description: Supplies values to a slew of variables at once,
//               typically to define actual values for a list of
//               formal parameters to a user-defined subroutine or
//               function.
//
//               Formals is a vector of variable names to be defined,
//               and actuals is a comma-separated list of expressions
//               to be substituted in, one-per-one.  The
//               subroutine_name is used only for error reporting.
////////////////////////////////////////////////////////////////////
void PPScope::
define_formals(const string &subroutine_name, 
               const vector<string> &formals, const string &actuals) {
  vector<string> actual_words;
  tokenize_params(actuals, actual_words, true);

  if (actual_words.size() < formals.size()) {
    cerr << "Warning: not all parameters defined for " << subroutine_name
         << ": " << actuals << "\n";
  } else if (actual_words.size() > formals.size()) {
    cerr << "Warning: more parameters defined for " << subroutine_name
         << " than actually exist: " << actuals << "\n";
  }

  for (int i = 0; i < (int)formals.size(); i++) {
    if (i < (int)actual_words.size()) {
      define_variable(formals[i], actual_words[i]);
    } else {
      define_variable(formals[i], string());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::get_variable
//       Access: Public
//  Description: Returns the variable definition associated with the
//               indicated variable name.
////////////////////////////////////////////////////////////////////
string PPScope::
get_variable(const string &varname) {
  // Is it a user-defined function?
  const PPSubroutine *sub = PPSubroutine::get_func(varname);
  if (sub != (const PPSubroutine *)NULL) {
    return expand_function(varname, sub, string());
  }      

  //  cerr << "getvar arg is: '" << varname << "'" << endl;

  string result;
  if (p_get_variable(varname, result)) {
    return result;
  }

  // Check the scopes on the stack for the variable definition.
  ScopeStack::reverse_iterator si;
  for (si = _scope_stack.rbegin(); si != _scope_stack.rend(); ++si) {
    if ((*si)->p_get_variable(varname, result)) {
      return result;
    }
  }

  // If the variable isn't defined, we check the environment.
  const char *env = getenv(varname.c_str());
  if (env != (const char *)NULL) {
    return env;
  }

  // It's not defined anywhere, so it's implicitly empty.
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_defined
//       Access: Private
//  Description: Expands the "defined" function variable. Code mimics get_variable()
////////////////////////////////////////////////////////////////////
string PPScope::
expand_defined(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 1) {
    cerr << "error: defined requires one parameter.\n";
    errors_occurred = true;
    return string();
  }

  string varname = tokens[0];
  string falsestr;
  string truestr = "1";

  // Is it a user-defined function?
  const PPSubroutine *sub = PPSubroutine::get_func(varname);

  string nullstr;

  if (sub != (const PPSubroutine *)NULL) {
    if(nullstr != expand_function(varname, sub, string())) {
      return truestr;
    }
  }      

  string result;

  if (p_get_variable(varname, result)) {
    return truestr;
  }

  // Check the scopes on the stack for the variable definition.
  ScopeStack::reverse_iterator si;
  for (si = _scope_stack.rbegin(); si != _scope_stack.rend(); ++si) {
    if ((*si)->p_get_variable(varname, result)) {
      return truestr;
    }
  }

  // If the variable isn't defined, we check the environment.
  const char *env = getenv(varname.c_str());
  if (env != (const char *)NULL) {
    return truestr;
  }

  // It's not defined anywhere, so it's implicitly empty.
  return falsestr;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_variable
//       Access: Public
//  Description: Similar to get_variable(), except the variable
//               definition is in turn expanded.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_variable(const string &varname) {
  return expand_string(get_variable(varname));
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::find_map_variable
//       Access: Public
//  Description: Looks for the map variable definition in this scope
//               or some ancestor scope.  Returns the map variable
//               definition if it is found, or _null_map_def if it is
//               not.
////////////////////////////////////////////////////////////////////
PPScope::MapVariableDefinition &PPScope::
find_map_variable(const string &varname) {
  MapVariableDefinition &def = p_find_map_variable(varname);
  if (&def != &_null_map_def) {
    return def;
  }

  // No such map variable.  Check the stack.
  ScopeStack::reverse_iterator si;
  for (si = _scope_stack.rbegin(); si != _scope_stack.rend(); ++si) {
    MapVariableDefinition &def = (*si)->p_find_map_variable(varname);
    if (&def != &_null_map_def) {
      return def;
    }
  }

  // Nada.
  return _null_map_def;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::get_directory
//       Access: Public
//  Description: Returns the directory level associated with this
//               scope, if any, or with the nearest parent to this
//               scope.
////////////////////////////////////////////////////////////////////
PPDirectory *PPScope::
get_directory() {
  if (_directory != (PPDirectory *)NULL) {
    return _directory;
  }

  // Check the stack.
  ScopeStack::reverse_iterator si;
  for (si = _scope_stack.rbegin(); si != _scope_stack.rend(); ++si) {
    if ((*si)->_directory != (PPDirectory *)NULL) {
      return (*si)->_directory;
    }
  }

  return (PPDirectory *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::set_directory
//       Access: Public
//  Description: Associates this scope with the indicated directory
//               level.  Typically this is done when definition a
//               scope for a particular source file which exists at a
//               known directory level.
////////////////////////////////////////////////////////////////////
void PPScope::
set_directory(PPDirectory *directory) {
  _directory = directory;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_string
//       Access: Public
//  Description: Expands out all the variable references in the given
//               string.  Variables are expanded recursively; that is,
//               if a variable expansion includes a reference to
//               another variable name, the second variable name is
//               expanded.  However, cyclical references are not
//               expanded.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_string(const string &str) {
  string result = r_expand_string(str, (ExpandedVariable *)NULL);

  if (debug_expansions > 0 && str != result) {
    // Look for the str in our table--how many times has this
    // particular string been expanded?
    ExpandResultCount &result_count = debug_expand[str];

    // Then, how many times has it expanded to this same result?
    // First, assuming this is the first time it has expanded to this
    // result, try to insert the result string with an initial count
    // of 1.
    pair<ExpandResultCount::iterator, bool> r = 
      result_count.insert(ExpandResultCount::value_type(result, 1));

    if (!r.second) {
      // If the result string was not successfully inserted into the
      // map, it was already there--so increment the count.
      ExpandResultCount::iterator rci = r.first;
      (*rci).second++;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_self_reference
//       Access: Public
//  Description: Similar to expand_string(), except that only simple
//               references to the named variable are expanded--other
//               variable references are left unchanged.  This allows
//               us to define a variable in terms of its previous
//               definition.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_self_reference(const string &str, const string &varname) {
  // Look for a simple reference to the named variable.  A more
  // complex reference, like a computed variable name or something
  // equally loopy, won't work with this simple test.  Too bad.
  string reference;
  reference += VARIABLE_PREFIX;
  reference += VARIABLE_OPEN_BRACE;
  reference += varname;
  reference += VARIABLE_CLOSE_BRACE;

  string result;

  size_t p = 0;
  size_t q = str.find(reference, p);
  while (q != string::npos) {
    result += str.substr(p, q - p);
    p = q;
    result += r_expand_variable(str, p, (ExpandedVariable *)NULL);
    q = str.find(reference, p);  
  }

  result += str.substr(p);
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: PPScope::push_scope
//       Access: Public, Static
//  Description: Pushes the indicated scope onto the top of the stack.
//               When a variable reference is unresolved in the
//               current scope, the scope stack is searched, in LIFO
//               order.
////////////////////////////////////////////////////////////////////
void PPScope::
push_scope(PPScope *scope) {
  _scope_stack.push_back(scope);
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::pop_scope
//       Access: Public, Static
//  Description: Pops another level off the top of the stack.  See
//               push_scope().
////////////////////////////////////////////////////////////////////
PPScope *PPScope::
pop_scope() {
  assert(!_scope_stack.empty());
  PPScope *back = _scope_stack.back();
  _scope_stack.pop_back();
  return back;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::get_bottom_scope
//       Access: Public, Static
//  Description: Returns the scope on the bottom of the stack.  This
//               was the very first scope ever pushed, e.g. the global
//               scope.
////////////////////////////////////////////////////////////////////
PPScope *PPScope::
get_bottom_scope() {
  assert(!_scope_stack.empty());
  return _scope_stack.front();
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::get_enclosing_scope
//       Access: Public, Static
//  Description: Returns the scope n below the top of the stack, or
//               the bottom scope if the stack has exactly n or fewer
//               scopes.
//
//               This will be the scope associated with the nth
//               enclosing syntax in the source file.
////////////////////////////////////////////////////////////////////
PPScope *PPScope::
get_enclosing_scope(int n) {
  assert(n >= 0);
  if (n >= _scope_stack.size()) {
    return get_bottom_scope();
  }
  return _scope_stack[_scope_stack.size() - 1 - n];
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::tokenize_params
//       Access: Public
//  Description: Separates a string into tokens based on comma
//               delimiters, e.g. for parameters to a function.
//               Nested variable references are skipped correctly,
//               even if they include commas.  Leading and trailing
//               whitespace in each token is automatically stripped.
//
//               If expand is true, the nested variables are
//               automatically expanded as the string is tokenized;
//               otherwise, they are left unexpanded.
////////////////////////////////////////////////////////////////////
void PPScope::
tokenize_params(const string &str, vector<string> &tokens,
                bool expand) {
  size_t p = 0;
  while (p < str.length()) {
    // Skip initial whitespace.
    while (p < str.length() && isspace(str[p])) {
      p++;
    }

    string token;
    while (p < str.length() && str[p] != FUNCTION_PARAMETER_SEPARATOR) {
      if (p + 1 < str.length() && str[p] == VARIABLE_PREFIX &&
          str[p + 1] == VARIABLE_OPEN_BRACE) {
        // Skip a nested variable reference.
        if (expand) {
          token += r_expand_variable(str, p, (ExpandedVariable *)NULL);
        } else {
          token += r_scan_variable(str, p);
        }
      } else {
        token += str[p];
        p++;
      }
    }

    // Back up past trailing whitespace.
    size_t q = token.length();
    while (q > 0 && isspace(token[q - 1])) {
      q--;
    }

    tokens.push_back(token.substr(0, q));
    p++;

    if (p == str.length()) {
      // In this case, we have just read past a trailing comma symbol
      // at the end of the string, so we have one more empty token.
      tokens.push_back(string());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::tokenize_numeric_pair
//       Access: Public
//  Description: This function is used by all the numeric comparision
//               functions, e.g. nne, nlt, etc.  It splits the string
//               up into two parameters based on commas, and evaluates
//               each parameter as a number, into a and b.  It returns
//               true if successful, or false if there was some user
//               error.
////////////////////////////////////////////////////////////////////
bool PPScope::
tokenize_numeric_pair(const string &str, double &a, double &b) {
  vector<string> words;
  tokenize_params(str, words, true);
  if (words.size() != 2) {
    cerr << words.size() << " parameters supplied when two were expected:\n"
         << str << "\n";
    errors_occurred = true;
    return false;
  }

  double results[2];

  for (int i = 0; i < 2; i++) {
    const char *param = words[i].c_str();
    char *n;
    results[i] = strtod(param, &n);
    if (*n != '\0') {
      // strtod failed--not a numeric representation.
      cerr << "Warning: " << words[i] << " is not a number.\n";
      if (n == param) {
        results[i] = 0.0;
      }
    }
  }

  a = results[0];
  b = results[1];
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::tokenize_ints
//       Access: Public
//  Description: This function is used by the arithmetic functions +,
//               -, etc.  It separates the string into parameters
//               based on the comma, interprets each parameter as an
//               integer, and fills up the indicated vector.
////////////////////////////////////////////////////////////////////
bool PPScope::
tokenize_ints(const string &str, vector<int> &tokens) {
  vector<string> words;
  tokenize_params(str, words, true);

  vector<string>::const_iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    const char *param = (*wi).c_str();
    char *n;
    int result = strtol(param, &n, 0);
    if (*n != '\0') {
      // strtol failed--not an integer.
      cerr << "Warning: " << param << " is not an integer.\n";
      if (n == param) {
        result = 0;
      }
    }

    tokens.push_back(result);
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::scan_to_whitespace
//       Access: Public
//  Description: Scans to the end of the first whitespace-delimited
//               word in the indicated string, even if it includes a
//               nested variable reference (which is itself allowed to
//               contain whitespace).
//
//               On input, str is a string, and start is the starting
//               position within the string of the scan; it should
//               point to a non-whitespace character.
//
//               The return value is the position within the string of
//               the first whitespace character encountered at its
//               original position or later, that is not part of a
//               variable reference.  All variable references are left
//               unexpanded.
////////////////////////////////////////////////////////////////////
size_t PPScope::
scan_to_whitespace(const string &str, size_t start) {
  size_t p = start;
  while (p < str.length() && !isspace(str[p])) {
    string token;
    if (p + 1 < str.length() && str[p] == VARIABLE_PREFIX &&
        str[p + 1] == VARIABLE_OPEN_BRACE) {
      // Skip a nested variable reference.
      r_scan_variable(str, p);

    } else {
      p++;
    }
  }

  return p;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::format_int
//       Access: Private, Static
//  Description: Formats the indicated integer as a string and returns
//               the string.
////////////////////////////////////////////////////////////////////
string PPScope::
format_int(int num) {
  char buffer[32];
  sprintf(buffer, "%d", num);
  return buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::p_set_variable
//       Access: Private
//  Description: The private implementation of p_set_variable.
//               Returns true if the variable's definition is found
//               and set, false otherwise.
////////////////////////////////////////////////////////////////////
bool PPScope::
p_set_variable(const string &varname, const string &definition) {
  Variables::iterator vi;
  vi = _variables.find(varname);
  if (vi != _variables.end()) {
    (*vi).second = definition;
    return true;
  }

  if (_parent_scope != (PPScope *)NULL) {
    return _parent_scope->p_set_variable(varname, definition);
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::p_get_variable
//       Access: Private
//  Description: The private implementation of get_variable().  This
//               checks the local scope only; it does not check the
//               stack.  It returns true if the variable is defined,
//               false otherwise..
////////////////////////////////////////////////////////////////////
bool PPScope::
p_get_variable(const string &varname, string &result) {
  Variables::const_iterator vi;
  vi = _variables.find(varname);
  if (vi != _variables.end()) {
    result = (*vi).second;
    return true;
  }

  if (varname == "RELDIR" && 
      _directory != (PPDirectory *)NULL &&
      current_output_directory != (PPDirectory *)NULL) {
    // $[RELDIR] is a special variable name that evaluates to the
    // relative directory of the current scope to the current output
    // directory.
    result = current_output_directory->get_rel_to(_directory);
    return true;
  }

  if (varname == "DEPENDS_INDEX" && 
      _directory != (PPDirectory *)NULL) {
    // $[DEPENDS_INDEX] is another special variable name that
    // evaluates to the numeric sorting index assigned to this
    // directory based on its dependency relationship with other
    // directories.  It's useful primarily for debugging.
    char buffer[32];
    sprintf(buffer, "%d", _directory->get_depends_index());
    result = buffer;
    return true;
  }

  if (_parent_scope != (PPScope *)NULL) {
    return _parent_scope->p_get_variable(varname, result);
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::r_expand_string
//       Access: Private
//  Description: The recursive implementation of expand_string().
//               This function detects cycles in the variable
//               expansion by storing the set of variable names that
//               have thus far been expanded in the linked list.
////////////////////////////////////////////////////////////////////
string PPScope::
r_expand_string(const string &str, PPScope::ExpandedVariable *expanded) {
  string result;

  // Search for a variable reference.
  size_t p = 0;
  while (p < str.length()) {
    if (p + 1 < str.length() && str[p] == VARIABLE_PREFIX &&
        str[p + 1] == VARIABLE_OPEN_BRACE) {
      // Here's a nested variable!  Expand it fully.
      result += r_expand_variable(str, p, expanded);

    } else {
      result += str[p];
      p++;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::r_scan_variable
//       Access: Private
//  Description: Scans past a single variable reference without
//               expanding it.  On input, str is a string containing a
//               variable reference (among other stuff), and vp is the
//               position within the string of the prefix character at
//               the beginning of the variable reference.
//
//               On output, vp is set to the position within the
//               string of the first character after the variable
//               reference's closing bracket.  The variable reference
//               itself is returned.
////////////////////////////////////////////////////////////////////
string PPScope::
r_scan_variable(const string &str, size_t &vp) {

  // Search for the end of the variable name: an unmatched square
  // bracket.
  size_t start = vp;
  size_t p = vp + 2;
  while (p < str.length() && str[p] != VARIABLE_CLOSE_BRACE) {
    if (p + 1 < str.length() && str[p] == VARIABLE_PREFIX && 
        str[p + 1] == VARIABLE_OPEN_BRACE) {
      // Here's a nested variable!  Scan past it, matching braces
      // properly.
      r_scan_variable(str, p);
    } else {
      p++;
    }
  }

  if (p < str.length()) {
    assert(str[p] == VARIABLE_CLOSE_BRACE);
    p++;
  } else {
    cerr << "Warning!  Unclosed variable reference:\n"
         << str.substr(vp) << "\n";
  }

  vp = p;
  return str.substr(start, vp - start);
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::r_expand_variable
//       Access: Private
//  Description: Expands a single variable reference.  On input, str
//               is a string containing a variable reference (among
//               other stuff), and vp is the position within the
//               string of the prefix character at the beginning of
//               the variable reference.
//
//               On output, vp is set to the position within the
//               string of the first character after the variable
//               reference's closing bracket, and the string expansion
//               of the variable reference is returned.
////////////////////////////////////////////////////////////////////
string PPScope::
r_expand_variable(const string &str, size_t &vp,
                  PPScope::ExpandedVariable *expanded) {
  string varname;

  size_t whitespace_at = 0;
  size_t open_nested_at = 0;

  // Search for the end of the variable name: an unmatched square
  // bracket.
  size_t p = vp + 2;
  while (p < str.length() && str[p] != VARIABLE_CLOSE_BRACE) {
    if (p + 1 < str.length() && str[p] == VARIABLE_PREFIX && 
        str[p + 1] == VARIABLE_OPEN_BRACE) {
      if (whitespace_at != 0) {
        // Once we have encountered whitespace, we don't expand
        // variables inline anymore.  These are now function
        // parameters, and might need to be expanded in some other
        // scope.
        varname += r_scan_variable(str, p);
      } else {
        varname += r_expand_variable(str, p, expanded);
      }

    } else {
      if (open_nested_at == 0 && str[p] == VARIABLE_OPEN_NESTED) {
        open_nested_at = p - (vp + 2);
      }
      if (open_nested_at == 0 && whitespace_at == 0 && isspace(str[p])) {
        whitespace_at = p - (vp + 2);
      }
      varname += str[p];
      p++;
    }
  }

  if (p < str.length()) {
    assert(str[p] == VARIABLE_CLOSE_BRACE);
    p++;
  } else {
    cerr << "Warning!  Unclosed variable reference:\n"
         << str.substr(vp) << "\n";
  }

  vp = p;

  // Check for a function expansion.
  if (whitespace_at != 0) {
    string funcname = varname.substr(0, whitespace_at);
    p = whitespace_at;
    while (p < varname.length() && isspace(varname[p])) {
      p++;
    }
    string params = varname.substr(p);

    // Is it a user-defined function?
    const PPSubroutine *sub = PPSubroutine::get_func(funcname);
    if (sub != (const PPSubroutine *)NULL) {
      return expand_function(funcname, sub, params);
    }      

    // Is it a built-in function?
    if (funcname == "isfullpath") {
      return expand_isfullpath(params);
    } else if (funcname == "osfilename") {
      return expand_osfilename(params);
    } else if (funcname == "unixfilename") {
      return expand_unixfilename(params);
    } else if (funcname == "unixshortname") {
      return expand_unixshortname(params);
    } else if (funcname == "cygpath_w") {
      // This maps to osfilename for historical reasons.
      return expand_osfilename(params);
    } else if (funcname == "cygpath_p") {
      // This maps to unixfilename for historical reasons.
      return expand_unixfilename(params);
    } else if (funcname == "wildcard") {
      return expand_wildcard(params);
    } else if (funcname == "isdir") {
      return expand_isdir(params);
    } else if (funcname == "isfile") {
      return expand_isfile(params);
    } else if (funcname == "libtest") {
      return expand_libtest(params);
    } else if (funcname == "bintest") {
      return expand_bintest(params);
    } else if (funcname == "shell") {
      return expand_shell(params);
    } else if (funcname == "standardize") {
      return expand_standardize(params);
    } else if (funcname == "canonical") {
      return expand_canonical(params);
    } else if (funcname == "length") {
      return expand_length(params);
    } else if (funcname == "substr") {
      return expand_substr(params);
    } else if (funcname == "findstring") {
      return expand_findstring(params);
    } else if (funcname == "dir") {
      return expand_dir(params);
    } else if (funcname == "notdir") {
      return expand_notdir(params);
    } else if (funcname == "suffix") {
      return expand_suffix(params);
    } else if (funcname == "basename") {
      return expand_basename(params);
    } else if (funcname == "makeguid") {
      return expand_makeguid(params);
    } else if (funcname == "word") {
      return expand_word(params);
    } else if (funcname == "wordlist") {
      return expand_wordlist(params);
    } else if (funcname == "words") {
      return expand_words(params);
    } else if (funcname == "firstword") {
      return expand_firstword(params);
    } else if (funcname == "patsubst") {
      return expand_patsubst(params, true);
    } else if (funcname == "patsubstw") {
      return expand_patsubst(params, false);
    } else if (funcname == "subst") {
      return expand_subst(params);
    } else if (funcname == "wordsubst") {
      return expand_wordsubst(params);
    } else if (funcname == "filter") {
      return expand_filter(params);
    } else if (funcname == "filter_out" || funcname == "filter-out") {
      return expand_filter_out(params);
    } else if (funcname == "join") {
      return expand_join(params);
    } else if (funcname == "sort") {
      return expand_sort(params);
    } else if (funcname == "unique") {
      return expand_unique(params);
    } else if (funcname == "matrix") {
      return expand_matrix(params);
    } else if (funcname == "if") {
      return expand_if(params);
    } else if (funcname == "eq") {
      return expand_eq(params);
    } else if (funcname == "defined") {
      return expand_defined(params);
    } else if (funcname == "ne") {
      return expand_ne(params);
    } else if (funcname == "=" || funcname == "==") {
      return expand_eqn(params);
    } else if (funcname == "!=") {
      return expand_nen(params);
    } else if (funcname == "<") {
      return expand_ltn(params);
    } else if (funcname == "<=") {
      return expand_len(params);
    } else if (funcname == ">") {
      return expand_gtn(params);
    } else if (funcname == ">=") {
      return expand_gen(params);
    } else if (funcname == "+") {
      return expand_plus(params);
    } else if (funcname == "-") {
      return expand_minus(params);
    } else if (funcname == "*") {
      return expand_times(params);
    } else if (funcname == "/") {
      return expand_divide(params);
    } else if (funcname == "%") {
      return expand_modulo(params);
    } else if (funcname == "not") {
      return expand_not(params);
    } else if (funcname == "or") {
      return expand_or(params);
    } else if (funcname == "and") {
      return expand_and(params);
    } else if (funcname == "upcase") {
      return expand_upcase(params);
    } else if (funcname == "downcase") {
      return expand_downcase(params);
    } else if (funcname == "cdefine") {
      return expand_cdefine(params);
    } else if (funcname == "closure") {
      return expand_closure(params);
    } else if (funcname == "unmapped") {
      return expand_unmapped(params);
    } else if (funcname == "dependencies") {
      return expand_dependencies(params);
    } else if (funcname == "foreach") {
      return expand_foreach(params);
    } else if (funcname == "forscopes") {
      return expand_forscopes(params);
    }

    // It must be a map variable.
    return expand_map_variable(funcname, params);
  }

  // Now we have the variable name; was it previously expanded?
  ExpandedVariable *ev;
  for (ev = expanded; ev != (ExpandedVariable *)NULL; ev = ev->_next) {
    if (ev->_varname == varname) {
      // Yes, this is a cyclical expansion.
      cerr << "Ignoring cyclical expansion of " << varname << "\n";
      return string();
    }
  }

  // And now expand the variable.

  string expansion;

  // Check for a special inline patsubst operation, like GNU make:
  // $[varname:%.c=%.o]
  string patsubst;
  bool got_patsubst = false;
  p = varname.find(variable_patsubst);
  if (p != string::npos) {
    got_patsubst = true;
    patsubst = varname.substr(p + variable_patsubst.length());
    varname = varname.substr(0, p);
  }

  // Check for special scoping operators in the variable name.
  p = varname.find(VARIABLE_OPEN_NESTED);
  if (p != string::npos && varname[varname.length() - 1] == VARIABLE_CLOSE_NESTED) {
    size_t q = varname.length() - 1;
    string scope_names = varname.substr(p + 1, q - (p + 1));
    varname = varname.substr(0, p);
    expansion = expand_variable_nested(varname, scope_names);

  } else {
    // No special scoping; just expand the variable name.
    expansion = get_variable(varname);
  }

  // Finally, recursively expand any variable references in the
  // variable's expansion.
  ExpandedVariable new_var;
  new_var._varname = varname;
  new_var._next = expanded;
  string result = r_expand_string(expansion, &new_var);

  // And *then* apply any inline patsubst.
  if (got_patsubst) {
    vector<string> tokens;
    tokenize(patsubst, tokens, VARIABLE_PATSUBST_DELIM);
    
    if (tokens.size() != 2) {
      cerr << "inline patsubst should be of the form "
           << VARIABLE_PREFIX << VARIABLE_OPEN_BRACE << "varname"
           << VARIABLE_PATSUBST << PATTERN_WILDCARD << ".c"
           << VARIABLE_PATSUBST_DELIM << PATTERN_WILDCARD << ".o"
           << VARIABLE_CLOSE_BRACE << ".\n";
      errors_occurred = true;
    } else {
      PPFilenamePattern from(tokens[0]);
      PPFilenamePattern to(tokens[1]);
    
      if (!from.has_wildcard() || !to.has_wildcard()) {
        cerr << "The two parameters of inline patsubst must both include "
             << PATTERN_WILDCARD << ".\n";
        errors_occurred = true;
        return string();
      }
    
      // Split the expansion into tokens based on the spaces.
      vector<string> words;
      tokenize_whitespace(result, words);
      
      vector<string>::iterator wi;
      for (wi = words.begin(); wi != words.end(); ++wi) {
        (*wi) = to.transform(*wi, from);
      }
    
      result = repaste(words, " ");
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_variable_nested
//       Access: Private
//  Description: Expands a variable reference of the form
//               $[varname(scope scope scope)].  This means to
//               concatenate the expansions of the variable in all of
//               the named scopes.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_variable_nested(const string &varname, 
                       const string &scope_names) {
  if (_named_scopes == (PPNamedScopes *)NULL) {
    return string();
  }

  vector<string> names;
  tokenize_whitespace(scope_names, names);

  // Get all of the named scopes.
  PPNamedScopes::Scopes scopes;
  
  vector<string>::const_iterator ni;
  for (ni = names.begin(); ni != names.end(); ++ni) {
    const string &name = (*ni);
    _named_scopes->get_scopes(name, scopes);
  }

  if (scopes.empty()) {
    return string();
  }

  // Now go through the scopes and build up the results.
  vector<string> results;

  PPNamedScopes::Scopes::const_iterator si;
  for (si = scopes.begin(); si != scopes.end(); ++si) {
    PPScope *scope = (*si);
    string nested = scope->expand_variable(varname);
    if (!nested.empty()) {
      results.push_back(nested);
    }
  }

  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_isfullpath
//       Access: Private
//  Description: Expands the "isfullpath" function variable.  This
//               returns true (actually, the same as its input) if the
//               input parameter is a fully-specified path name,
//               meaning it begins with a slash for unix_platform, and
//               it begins with a slash or backslash, with an optional
//               drive leterr, for windows_platform.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_isfullpath(const string &params) {
  Filename filename = trim_blanks(expand_string(params));

  string result;
  if (filename.is_fully_qualified()) {
    result = filename;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_osfilename
//       Access: Private
//  Description: Expands the "osfilename" function variable.  This
//               converts the filename from a Unix-style filename
//               (e.g. with slash separators) to a platform-specific
//               filename.
//
//               This follows the same rules of Panda filename
//               conversion; i.e. forward slashes become backslashes,
//               and $PANDA_ROOT prefixes full pathnames, unless the
//               topmost directory name is a single letter.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_osfilename(const string &params) {
  // Split the parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(params), words);

  vector<string>::iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    Filename filename = (*wi);
    (*wi) = filename.to_os_specific();
  }

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_unixfilename
//       Access: Private
//  Description: Expands the "unixfilename" function variable.  This
//               converts the filename from a platform-specific
//               filename to a Unix-style filename (e.g. with slash
//               separators).
//
//               This follows the rules of Panda filename conversion.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_unixfilename(const string &params) {
  // Split the parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(params), words);

  vector<string>::iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    Filename filename = Filename::from_os_specific(*wi);
    (*wi) = filename;
  }

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_unixshortname
//       Access: Private
//  Description: Expands the "unixshortname" function variable.  This
//               converts the filename from a platform-specific
//               filename to a Unix-style filename (e.g. with slash
//               separators), just like the unixfilename variable.
//
//               On Windows, this also specifically converts the
//               Windows-specific filename to 8.3 convention before
//               converting it to Unix style.  This can be a cheesy
//               way to work around embedded spaces in the filename.
//
//               Unlike unixfilename, this parameter accepts only one
//               filename.  However, the filename may contain embedded
//               spaces.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_unixshortname(const string &params) {
  Filename filename = Filename::from_os_specific(params);
  filename = Filename::from_os_specific(filename.to_os_short_name());

  return filename;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_wildcard
//       Access: Private
//  Description: Expands the "wildcard" function variable.  This
//               returns the set of files matched by the parameters
//               with shell matching characters.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_wildcard(const string &params) {
  vector<string> results;
  glob_string(expand_string(params), results);

  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_isdir
//       Access: Private
//  Description: Expands the "isdir" function variable.  This
//               returns true if the parameter exists and is a
//               directory, or false otherwise.  This actually expands
//               the parameter(s) with shell globbing characters,
//               similar to the "wildcard" function, and looks only at
//               the first expansion.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_isdir(const string &params) {
  vector<string> results;
  glob_string(expand_string(params), results);

  if (results.empty()) {
    // No matching file, too bad.
    return string();
  }

  Filename filename = results[0];
  if (filename.is_directory()) {
    return filename.get_fullpath();
  } else {
    return string();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_isfile
//       Access: Private
//  Description: Expands the "isfile" function variable.  This
//               returns true if the parameter exists and is a
//               regular file, or false otherwise.  This actually
//               expands the parameter(s) with shell globbing
//               characters, similar to the "wildcard" function, and
//               looks only at the first expansion.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_isfile(const string &params) {
  vector<string> results;
  glob_string(expand_string(params), results);

  if (results.empty()) {
    // No matching file, too bad.
    return string();
  }

  Filename filename = results[0];
  if (filename.is_regular_file()) {
    return filename.get_fullpath();
  } else {
    return string();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_libtest
//       Access: Private
//  Description: Expands the "libtest" function variable.  This
//               serves as a poor man's autoconf feature to check to
//               see if a library by the given name exists on the
//               indicated search path, or on the system search path.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_libtest(const string &params) {
  // Get the parameters out based on commas.  The first parameter is a
  // space-separated set of directories to search, the second
  // parameter is a space-separated set of library names.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 2) {
    cerr << "libtest requires two parameters.\n";
    errors_occurred = true;
    return string();
  }

  DSearchPath directories;
  directories.append_path(tokens[0], " \n\t");

  // Also add the system directories to the list, whatever we think
  // those should be.  Here we have to make a few assumptions.
#ifdef WIN32
  const char *windir = getenv("WINDIR");
  if (windir != (const char *)NULL) {
    Filename windir_filename = Filename::from_os_specific(windir);
    directories.append_directory(Filename(windir_filename, "System"));
    directories.append_directory(Filename(windir_filename, "System32"));
  }

  const char *lib = getenv("LIB");
  if (lib != (const char *)NULL) {
    vector<string> lib_dirs;
    tokenize(lib, lib_dirs, ";");
    vector<string>::const_iterator li;
    for (li = lib_dirs.begin(); li != lib_dirs.end(); ++li) {
      directories.append_directory(Filename::from_os_specific(*li));
    }
  }
#endif

  // We'll also check the Unix standard places, even if we're building
  // Windows, since we might be using Cygwin.

  // Check LD_LIBRARY_PATH.
  const char *ld_library_path = getenv("LD_LIBRARY_PATH");
  if (ld_library_path != (const char *)NULL) {
    directories.append_path(ld_library_path, ":");
  }

  directories.append_directory("/lib");
  directories.append_directory("/usr/lib");

  vector<string> libnames;
  tokenize_whitespace(tokens[1], libnames);

  if (libnames.empty()) {
    // No libraries is a default "false".
    return string();
  }

  // We only bother to search for the first library name in the list.
  Filename libname = libnames[0];

  bool found = false;

#ifdef WIN32
  if (libname.get_extension() != string("lib")) {
    libname = "lib" + libname.get_basename() + ".lib";
  }
  found = libname.resolve_filename(directories);
  if (!found) {
    libname.set_extension("dll");
    found = libname.resolve_filename(directories);
  }
  
#else  // WIN32
  libname = "lib" + libname.get_basename() + ".a";
  found = libname.resolve_filename(directories);
  if (!found) {
    libname.set_extension("so");
    found = libname.resolve_filename(directories);
  }
#ifdef HAVE_OSX
  if (!found) {
    libname.set_extension("dylib");
    found = libname.resolve_filename(directories);
  }
#endif  // HAVE_OSX
#endif  // WIN32

  if (found) {
    return libname.get_fullpath();
  } else {
    return string();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_bintest
//       Access: Private
//  Description: Expands the "bintest" function variable.  This
//               serves as a poor man's autoconf feature to check to
//               see if an executable program by the given name exists
//               on the indicated search path, or on the system search
//               path.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_bintest(const string &params) {
  // We only have one parameter: the filename of the executable.  We
  // always search for it on the path.
  Filename binname = Filename::from_os_specific(expand_string(params));

  if (binname.empty()) {
    // No binary, no exist.
    return string();
  }

  // An explicit path from the root does not require a search.
  if (binname.is_fully_qualified()) {
    if (binname.exists()) {
      return binname.get_fullpath();
    } else {
      return string();
    }
  }

  const char *path = getenv("PATH");
  if (path == (const char *)NULL) {
    // If the path is undefined, too bad.
    return string();
  }

  string pathvar(path);

  DSearchPath directories;

#ifdef WIN32
  if (pathvar.find(';') != string::npos) {
    // If the path contains semicolons, it's a native Windows-style
    // path: split it up based on semicolons, and convert each
    // directory from windows form.
    vector<string> path_dirs;
    tokenize(path, path_dirs, ";");
    vector<string>::const_iterator pi;
    for (pi = path_dirs.begin(); pi != path_dirs.end(); ++pi) {
      directories.append_directory(Filename::from_os_specific(*pi));
    }

  } else {
    // Otherwise, assume it's a Cygwin-style path: split it up based
    // on colons.
    directories.append_path(pathvar, ":");
  }
#else
  directories.append_path(pathvar, ":");
#endif

#ifdef WIN32
  bool found = binname.resolve_filename(directories, "exe");
#else
  bool found = binname.resolve_filename(directories);
#endif

  if (found) {
    return binname.get_fullpath();
  } else {
    return string();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_shell
//       Access: Private
//  Description: Expands the "shell" function variable.  This executes
//               the given command in a subprocess and returns the
//               standard output.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_shell(const string &params) {
#ifdef WIN32_VC
  cerr << "$[shell] is not presently supported on Win32 without Cygwin.\n";
  errors_occurred = true;
  string output;

#else  // WIN32_VC
  // We run $[shell] commands within the directory indicated by
  // $[THISDIRPREFIX].  This way, local filenames will be expanded the
  // way we expect.
  string dirname = trim_blanks(expand_variable("THISDIRPREFIX"));

  string command = expand_string(params);
  int pid, status;

  int pd[2];
  if (pipe(pd) < 0) {
    // pipe() failed.
    perror("pipe");
    return string();
  }

  pid = fork();
  if (pid < 0) {
    // fork() failed.
    perror("fork");
    return string();
  }
    
  if (pid == 0) {
    // Child.

    if (!dirname.empty()) {
      // We don't have to restore the directory after we're done,
      // because we're doing the chdir() call only within the child
      // process.
      if (chdir(dirname.c_str()) < 0) {
        perror("chdir");
      }
    }

    close(pd[0]);
    dup2(pd[1], STDOUT_FILENO);
    char *argv[4];
    argv[0] = (char *)"sh";
    argv[1] = (char *)"-c";
    argv[2] = (char *)command.c_str();
    argv[3] = (char *)NULL;
    execv("/bin/sh", argv);
    exit(127);
  }

  // Parent.  Wait for the child to terminate, and read from its
  // output while we're waiting.
  close(pd[1]);
  bool child_done = false;
  bool pipe_closed = false;
  string output;

  while (!child_done && !pipe_closed) {
    static const int buffer_size = 1024;
    char buffer[buffer_size];
    int read_bytes = (int)read(pd[0], buffer, buffer_size);
    if (read_bytes < 0) {
      perror("read");
    } else if (read_bytes == 0) {
      pipe_closed = true;
    } else {
      output += string(buffer, read_bytes);
    }

    if (!child_done) {
      int waitresult = waitpid(pid, &status, WNOHANG);
      if (waitresult < 0) {
        if (errno != EINTR) {
          perror("waitpid");
          return string();
        }
      } else if (waitresult > 0) {
        child_done = true;
      }
    }
  }
  close(pd[0]);
#endif  // WIN32_VC

  // Now get the output.  We split it into words and then reconnect
  // it, to simulate the shell's backpop operator.
  vector<string> results;
  tokenize_whitespace(output, results);

  string result = repaste(results, " ");

  return result;
}




////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_standardize
//       Access: Private
//  Description: Expands the "standardize" function variable.  This
//               converts the filename to standard form by removing
//               consecutive repeated slashes and collapsing /../
//               where possible.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_standardize(const string &params) {
  Filename filename = trim_blanks(expand_string(params));
  if (filename.empty()) {
    return string();
  }

  filename.standardize();
  return filename.get_fullpath();
}


////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_canonical
//       Access: Private
//  Description: Expands the "canonical" function variable.  This
//               converts this filename to a canonical name by
//               replacing the directory part with the fully-qualified
//               directory part.  This is done by changing to that
//               directory and calling getcwd().
//
//               See filename::make_canonical() for a complete
//               explanation of the implications of this and of the
//               difference between this and standardize, above.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_canonical(const string &params) {
  Filename filename = trim_blanks(expand_string(params));
  filename.make_canonical();
  return filename.get_fullpath();
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_length
//       Access: Private
//  Description: Expands the "length" function variable.  This returns
//               the length of the argument in characters.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_length(const string &params) {
  string word = trim_blanks(expand_string(params));

  char buffer[32];
  sprintf(buffer, "%d", (int) word.length());
  string result = buffer;
  return result;
}  

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_substr
//       Access: Private
//  Description: Expands the "substr" function variable.  $[substr
//               S,E,string] returns the substring of "string"
//               beginning at character S (1-based) and continuing to
//               character E, inclusive.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_substr(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 3) {
    cerr << "substr requires three parameters.\n";
    errors_occurred = true;
    return string();
  }

  int start = atoi(tokens[0].c_str());
  int end = atoi(tokens[1].c_str());

  if (end < start) {
    // Following GNU make's convention, we swap start and end if
    // they're out of order.
    int t = end;
    end = start;
    start = t;
  }

  const string &word = tokens[2];

  start = max(start, 1);
  end = min(end, (int)word.length());

  if (end < start) {
    return string();
  }

  string result = word.substr(start - 1, end - start + 1);
  return result;
}  

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_dir
//       Access: Private
//  Description: Expands the "dir" function variable.  This returns
//               the directory part of its filename argument(s), or ./
//               if the words contain no slash.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_dir(const string &params) {
  // Split the parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(params), words);

  vector<string>::iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    string &word = (*wi);

    size_t slash = word.rfind('/');
    if (slash != string::npos) {
      word = word.substr(0, slash + 1);
    } else {
      word = "./";
    }
  }

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_notdir
//       Access: Private
//  Description: Expands the "notdir" function variable.  This returns
//               everything following the rightmost slash, or the
//               string itself if there is no slash.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_notdir(const string &params) {
  // Split the parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(params), words);

  vector<string>::iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    string &word = (*wi);

    size_t slash = word.rfind('/');
    if (slash != string::npos) {
      word = word.substr(slash + 1);
    }
  }

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_suffix
//       Access: Private
//  Description: Expands the "suffix" function variable.  This returns
//               the filename extension, including a dot, if any.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_suffix(const string &params) {
  // Split the parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(params), words);

  vector<string>::iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    string &word = (*wi);

    size_t dot = word.rfind('.');
    if (dot != string::npos) {
      string ext = word.substr(dot);
      if (ext.find('/') == string::npos) {
        word = ext;
      } else {
        word = string();
      }
    } else {
      word = string();
    }
  }

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_basename
//       Access: Private
//  Description: Expands the "basename" function variable.  This returns
//               everything but the filename extension (including the
//               directory, if any).
////////////////////////////////////////////////////////////////////
string PPScope::
expand_basename(const string &params) {
  // Split the parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(params), words);

  vector<string>::iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    string &word = (*wi);

    size_t dot = word.rfind('.');
    if (dot != string::npos) {
      string ext = word.substr(dot);
      if (ext.find('/') == string::npos) {
        word = word.substr(0, dot);
      }
    }
  }

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_makeguid
//       Access: Private
//  Description: Expands the GUID (global unique identifier) of the
//               given name (generally a directory).  A GUID looks
//               like this: 398F2CC4-C683-26EB-3251-6FC996738F7F
////////////////////////////////////////////////////////////////////
string PPScope::
expand_makeguid(const string &params) {
  // Expand all of the parameters into a single string.
  string expansion = trim_blanks(expand_string(params));

  if (expansion.size() == 0) {
    cerr << "makeguid requires an argument.\n";
    errors_occurred = true;
    return string();
  }

  PP_MD5_CTX context;
  unsigned char digest[16];
  
  MD5Init(&context);
  MD5Update(&context, reinterpret_cast<const unsigned char *>(expansion.data()),
            expansion.size());
  MD5Final(digest, &context);

  string guid;
  char hex[2];
  int i;

  for (i = 0; i < 4; i++) {
    sprintf(hex, "%02x", digest[i]);
    guid.append(hex);
  }
  guid += "-";

  for (i = 4; i < 6; i++) {
    sprintf(hex, "%02x", digest[i]);
    guid.append(hex);
  }
  guid += "-";

  for (i = 6; i < 8; i++) {
    sprintf(hex, "%02x", digest[i]);
    guid.append(hex);
  }
  guid += "-";

  for (i = 8; i < 10; i++) {
    sprintf(hex, "%02x", digest[i]);
    guid.append(hex);
  }
  guid += "-";

  for (i = 10; i < 16; i++) {
    sprintf(hex, "%02x", digest[i]);
    guid.append(hex);
  }

  // Convert the entire GUID string to uppercased letters.
  string::iterator si;
  for (si = guid.begin(); si != guid.end(); ++si) {
    (*si) = toupper(*si);
  }

  return guid;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_word
//       Access: Private
//  Description: Expands the "word" function variable.  This returns
//               the nth word, 1-based, of the space-separated list of
//               words in the second parameter.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_word(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 2) {
    cerr << "word requires two parameters.\n";
    errors_occurred = true;
    return string();
  }

  int index = atoi(tokens[0].c_str());

  // Split the second parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(tokens[1]), words);

  if (index < 1 || index > (int)words.size()) {
    // Out of range.
    return string();
  }
  return words[index - 1];
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_wordlist
//       Access: Private
//  Description: Expands the "wordlist" function variable.  This
//               returns a range of words, 1-based, of the
//               space-separated list of words in the third parameter.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_wordlist(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 3) {
    cerr << "wordlist requires three parameters.\n";
    errors_occurred = true;
    return string();
  }

  int start = atoi(tokens[0].c_str());
  int end = atoi(tokens[1].c_str());

  if (end < start) {
    // Following GNU make's convention, we swap start and end if
    // they're out of order.
    int t = end;
    end = start;
    start = t;
  }

  // Split the third parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(tokens[2]), words);

  start = max(start, 1);
  end = min(end, (int)words.size() + 1);

  if (end < start) {
    return string();
  }

  vector<string> results;
  results.insert(results.end(), 
                 words.begin() + start - 1, 
                 words.begin() + end - 1);

  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_words
//       Access: Private
//  Description: Expands the "words" function variable.  This
//               returns the number of space-separated words in the
//               list.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_words(const string &params) {
  // Split the parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(params), words);

  char buffer[32];
  sprintf(buffer, "%d", (int) words.size());
  string result = buffer;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_firstword
//       Access: Private
//  Description: Expands the "firstword" function variable.  This
//               returns the first of several words separated by
//               whitespace.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_firstword(const string &params) {
  // Split the parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(params), words);

  if (!words.empty()) {
    return words[0];
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_patsubst
//       Access: Private
//  Description: Expands the "patsubst" function variable.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_patsubst(const string &params, bool separate_words) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, false);

  if (tokens.size() < 3) {
    cerr << "patsubst requires at least three parameters.\n";
    errors_occurred = true;
    return string();
  }

  if ((tokens.size() % 2) != 1) {
    cerr << "patsubst requires an odd number of parameters.\n";
    errors_occurred = true;
    return string();
  }

  // Split the last parameter into tokens based on the spaces--but
  // only if separate_words is true.
  vector<string> words;
  if (separate_words) {
    tokenize_whitespace(expand_string(tokens.back()), words);
  } else {
    words.push_back(expand_string(tokens.back()));
  }

  // Build up a vector of from/to patterns.
  typedef vector<PPFilenamePattern> Patterns;
  typedef vector<Patterns> FromPatterns;
  FromPatterns from;
  Patterns to;

  size_t i;
  for (i = 0; i < tokens.size() - 1; i += 2) {
    // Each "from" pattern might be a collection of patterns separated
    // by spaces, and it is expanded immediately.
    from.push_back(Patterns());
    vector<string> froms;
    tokenize_whitespace(expand_string(tokens[i]), froms);
    vector<string>::const_iterator fi;
    for (fi = froms.begin(); fi != froms.end(); ++fi) {
      PPFilenamePattern pattern(*fi);
      if (!pattern.has_wildcard()) {
        cerr << "All the \"from\" parameters of patsubst must include "
             << PATTERN_WILDCARD << ".\n";
        errors_occurred = true;
        return string();
      }
      from.back().push_back(pattern);
    }

    // However, the corresponding "to" pattern is just one pattern,
    // and it is expanded immediately only if it does not contain a
    // wildcard character.
    PPFilenamePattern to_pattern(tokens[i + 1]);
    if (!to_pattern.has_wildcard()) {
      to_pattern = PPFilenamePattern(expand_string(tokens[i + 1]));
    }
    to.push_back(to_pattern);
  }
  size_t num_patterns = from.size();
  assert(num_patterns == to.size());
  
  vector<string>::iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    bool matched = false;
    for (i = 0; i < num_patterns && !matched; i++) {
      Patterns::const_iterator pi;
      for (pi = from[i].begin(); pi != from[i].end() && !matched; ++pi) {
        if ((*pi).matches(*wi)) {
          matched = true;
          string transformed = to[i].transform(*wi, (*pi));
          (*wi) = expand_string(transformed);
        }
      }
    }
  }

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_filter
//       Access: Private
//  Description: Expands the "filter" function variable.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_filter(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 2) {
    cerr << "filter requires two parameters.\n";
    errors_occurred = true;
    return string();
  }

  // Split up the first parameter--the list of patterns to filter
  // by--into tokens based on the spaces.
  vector<string> pattern_strings;
  tokenize_whitespace(tokens[0], pattern_strings);

  vector<PPFilenamePattern> patterns;
  vector<string>::const_iterator psi;
  for (psi = pattern_strings.begin(); psi != pattern_strings.end(); ++psi) {
    patterns.push_back(PPFilenamePattern(*psi));
  }

  // Split up the second parameter--the list of words to filter--into
  // tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(tokens[1], words);

  vector<string>::iterator wi, wnext;
  wnext = words.begin();
  for (wi = words.begin(); wi != words.end(); ++wi) {
    const string &word = (*wi);

    bool matches_pattern = false;
    vector<PPFilenamePattern>::const_iterator pi;
    for (pi = patterns.begin(); 
         pi != patterns.end() && !matches_pattern; 
         ++pi) {
      matches_pattern = (*pi).matches(word);
    }

    if (matches_pattern) {
      *wnext++ = word;
    }
  }

  words.erase(wnext, words.end());

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_filter_out
//       Access: Private
//  Description: Expands the "filter_out" function variable.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_filter_out(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 2) {
    cerr << "filter-out requires two parameters.\n";
    errors_occurred = true;
    return string();
  }

  // Split up the first parameter--the list of patterns to filter
  // by--into tokens based on the spaces.
  vector<string> pattern_strings;
  tokenize_whitespace(tokens[0], pattern_strings);

  vector<PPFilenamePattern> patterns;
  vector<string>::const_iterator psi;
  for (psi = pattern_strings.begin(); psi != pattern_strings.end(); ++psi) {
    patterns.push_back(PPFilenamePattern(*psi));
  }

  // Split up the second parameter--the list of words to filter--into
  // tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(tokens[1], words);

  vector<string>::iterator wi, wnext;
  wnext = words.begin();
  for (wi = words.begin(); wi != words.end(); ++wi) {
    const string &word = (*wi);

    bool matches_pattern = false;
    vector<PPFilenamePattern>::const_iterator pi;
    for (pi = patterns.begin(); 
         pi != patterns.end() && !matches_pattern; 
         ++pi) {
      matches_pattern = (*pi).matches(word);
    }

    if (!matches_pattern) {
      *wnext++ = word;
    }
  }

  words.erase(wnext, words.end());

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_subst
//       Access: Private
//  Description: Expands the "subst" function variable.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_subst(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() < 3) {
    cerr << "subst requires at least three parameters.\n";
    errors_occurred = true;
    return string();
  }

  if ((tokens.size() % 2) != 1) {
    cerr << "subst requires an odd number of parameters.\n";
    errors_occurred = true;
    return string();
  }

  // Now substitute each of the substitute strings out for the
  // replacement strings.
  string str = tokens.back();
  for (size_t i = 0; i < tokens.size() - 1; i += 2) {
    string new_str;
    const string &subst = tokens[i];
    const string &repl = tokens[i + 1];
    size_t q = 0;
    size_t p = str.find(subst, q);
    while (p != string::npos) {
      new_str += str.substr(q, p - q) + repl;
      q = p + subst.length();
      p = str.find(subst, q);
    }
    str = new_str + str.substr(q);
  }
  return str;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_findstrnig
//       Access: Private
//  Description: Expands the "findstring" function variable.
//               $[findstring a,b] returns b if and only if it is a
//               substring of a; otherwise, it returns the empty
//               string.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_findstring(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 2) {
    cerr << "findstring requires two parameters.\n";
    errors_occurred = true;
    return string();
  }
  string str = tokens.back();
  const string &srchstr = tokens[0];
  size_t q = 0;
  size_t p = str.find(srchstr, q);
  if(p == string::npos)
    str = "";

  return str;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_wordsubst
//       Access: Private
//  Description: Expands the "wordsubst" function variable.  This is
//               like "subst" except it only replaces whole words.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_wordsubst(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() < 3) {
    cerr << "subst requires at least three parameters.\n";
    errors_occurred = true;
    return string();
  }

  if ((tokens.size() % 2) != 1) {
    cerr << "subst requires an odd number of parameters.\n";
    errors_occurred = true;
    return string();
  }

  // Split the last parameter into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(tokens.back(), words);
  
  for (size_t i = 0; i < tokens.size() - 1; i += 2) {
    const string &subst = tokens[i];
    const string &repl = tokens[i + 1];
    vector<string>::iterator wi;
    for (wi = words.begin(); wi != words.end(); ++wi) {
      if ((*wi) == subst) {
        (*wi) = repl;
      }
    }
  }

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_join
//       Access: Private
//  Description: Expands the "join" function variable: joins the list
//               of words using the specified separator.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_join(const string &params) {
  // Split the string up into tokens based on the spaces.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 2) {
    cerr << "join requires two parameters.\n";
    errors_occurred = true;
    return string();
  }

  const string &sep = tokens[0];
  vector<string> words;
  tokenize_whitespace(expand_string(tokens[1]), words);

  string result = repaste(words, sep);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_sort
//       Access: Private
//  Description: Expands the "sort" function variable: sort the words
//               into alphabetical order, and also remove duplicates.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_sort(const string &params) {
  // Split the string up into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(params), words);

  sort(words.begin(), words.end());
  words.erase(unique(words.begin(), words.end()), words.end());

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_unique
//       Access: Private
//  Description: Expands the "unique" function variable: remove
//               duplicates from the list of words without changing
//               the order.  The first appearance of each word
//               remains.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_unique(const string &params) {
  // Split the string up into tokens based on the spaces.
  vector<string> words;
  tokenize_whitespace(expand_string(params), words);

  vector<string>::iterator win, wout;
  set<string> included_words;

  win = words.begin();
  wout = words.begin();
  while (win != words.end()) {
    if (included_words.insert(*win).second) {
      // This is a unique word so far.
      *wout++ = *win;
    }
    ++win;
  }

  words.erase(wout, words.end());
  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_matrix
//       Access: Private
//  Description: Expands the "matrix" function variable.  This
//               combines the different words of the n parameters in
//               all possible ways, like the shell {a,b,c} expansion
//               characters.  For example, $[matrix a b,c,10 20 30]
//               expands to ac10 ac20 ac30 bc10 bc20 bc30.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_matrix(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  // Each token gets split up into words based on the spaces.
  vector<vector<string> > words;
  for (int i = 0; i < (int)tokens.size(); i++) {
    words.push_back(vector<string>());
    tokenize_whitespace(tokens[i], words.back());
  }

  // Now synthesize the results recursively.
  vector<string> results;
  r_expand_matrix(results, words, 0, "");

  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_if
//       Access: Private
//  Description: Expands the "if" function variable.  This evaluates
//               the first parameter and returns the second parameter
//               if the result is true (i.e. nonempty) and the third
//               parameter (if present) if the result is false
//               (i.e. empty).
////////////////////////////////////////////////////////////////////
string PPScope::
expand_if(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() == 2) {
    if (!tokens[0].empty()) {
      return tokens[1];
    } else {
      return "";
    }
  } else if (tokens.size() == 3) {
    if (!tokens[0].empty()) {
      return tokens[1];
    } else {
      return tokens[2];
    }
  }

  cerr << "if requires two or three parameters.\n";
  errors_occurred = true;
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_eq
//       Access: Private
//  Description: Expands the "eq" function variable.  This tests
//               string equivalence.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_eq(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 2) {
    cerr << "eq requires two parameters.\n";
    errors_occurred = true;
    return string();
  }

  string result;
  if (tokens[0] == tokens[1]) {
    result = "1";
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_ne
//       Access: Private
//  Description: Expands the "ne" function variable.  This tests
//               string equivalence.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_ne(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 2) {
    cerr << "ne requires two parameters.\n";
    errors_occurred = true;
    return string();
  }

  string result;
  if (!(tokens[0] == tokens[1])) {
    result = "1";
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_eqn
//       Access: Private
//  Description: Expands the "=" function variable.  This tests
//               numeric equivalence.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_eqn(const string &params) {
  double a, b;
  if (!tokenize_numeric_pair(params, a, b)) {
    return string();
  }

  string result;
  if (a == b) {
    result = "1";
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_nen
//       Access: Private
//  Description: Expands the "!=" function variable.  This tests
//               numeric equivalence.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_nen(const string &params) {
  double a, b;
  if (!tokenize_numeric_pair(params, a, b)) {
    return string();
  }

  string result;
  if (a != b) {
    result = "1";
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_ltn
//       Access: Private
//  Description: Expands the "<" function variable.  This tests
//               numeric relationships.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_ltn(const string &params) {
  double a, b;
  if (!tokenize_numeric_pair(params, a, b)) {
    return string();
  }

  string result;
  if (a < b) {
    result = "1";
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_len
//       Access: Private
//  Description: Expands the "<=" function variable.  This tests
//               numeric relationships.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_len(const string &params) {
  double a, b;
  if (!tokenize_numeric_pair(params, a, b)) {
    return string();
  }

  string result;
  if (a <= b) {
    result = "1";
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_gtn
//       Access: Private
//  Description: Expands the ">" function variable.  This tests
//               numeric relationships.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_gtn(const string &params) {
  double a, b;
  if (!tokenize_numeric_pair(params, a, b)) {
    return string();
  }

  string result;
  if (a > b) {
    result = "1";
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_gen
//       Access: Private
//  Description: Expands the ">=" function variable.  This tests
//               numeric relationships.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_gen(const string &params) {
  double a, b;
  if (!tokenize_numeric_pair(params, a, b)) {
    return string();
  }

  string result;
  if (a >= b) {
    result = "1";
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_plus
//       Access: Private
//  Description: Expands the "+" function variable.  This operates
//               on integer numbers.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_plus(const string &params) {
  vector<int> tokens;
  if (!tokenize_ints(params, tokens)) {
    return string();
  }

  int result = 0;
  vector<int>::const_iterator ti;
  for (ti = tokens.begin(); ti != tokens.end(); ++ti) {
    result += (*ti);
  }

  return format_int(result);
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_minus
//       Access: Private
//  Description: Expands the "-" function variable.  This operates
//               on integer numbers.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_minus(const string &params) {
  vector<int> tokens;
  if (!tokenize_ints(params, tokens)) {
    return string();
  }

  int result = 0;

  if (tokens.size() == 1) {
    // A special case: unary minus.
    result = -tokens[0];

  } else if (tokens.size() > 1) {
    result = tokens[0];
    for (int i = 1; i < (int)tokens.size(); i++) {
      result -= tokens[i];
    }
  }

  return format_int(result);
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_times
//       Access: Private
//  Description: Expands the "*" function variable.  This operates
//               on integer numbers.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_times(const string &params) {
  vector<int> tokens;
  if (!tokenize_ints(params, tokens)) {
    return string();
  }

  int result = 1;
  vector<int>::const_iterator ti;
  for (ti = tokens.begin(); ti != tokens.end(); ++ti) {
    result *= (*ti);
  }

  return format_int(result);
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_divide
//       Access: Private
//  Description: Expands the "/" function variable.  This operates
//               on integer numbers.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_divide(const string &params) {
  vector<int> tokens;
  if (!tokenize_ints(params, tokens)) {
    return string();
  }

  if (tokens.size() != 2) {
    cerr << tokens.size() << " parameters supplied when two were expected:\n"
         << params << "\n";
    errors_occurred = true;
    return string();
  }

  return format_int(tokens[0] / tokens[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_modulo
//       Access: Private
//  Description: Expands the "%" function variable.  This operates
//               on integer numbers.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_modulo(const string &params) {
  vector<int> tokens;
  if (!tokenize_ints(params, tokens)) {
    return string();
  }

  if (tokens.size() != 2) {
    cerr << tokens.size() << " parameters supplied when two were expected:\n"
         << params << "\n";
    errors_occurred = true;
    return string();
  }

  return format_int(tokens[0] % tokens[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_not
//       Access: Private
//  Description: Expands the "not" function variable.  This returns
//               nonempty if its argument is empty, empty if its
//               argument is nonempty.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_not(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  if (tokens.size() != 1) {
    cerr << "not requires one parameter.\n";
    errors_occurred = true;
    return string();
  }

  string result;
  if (tokens[0].empty()) {
    result = "1";
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_or
//       Access: Private
//  Description: Expands the "or" function variable.  This returns
//               nonempty if any of its arguments are nonempty.
//               Specifically, it returns the first nonempty argument.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_or(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  vector<string>::const_iterator ti;
  for (ti = tokens.begin(); ti != tokens.end(); ++ti) {
    if (!(*ti).empty()) {
      return (*ti);
    }
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_and
//       Access: Private
//  Description: Expands the "and" function variable.  This returns
//               nonempty if all of its arguments are nonempty.
//               Specifically, it returns the last argument.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_and(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, true);

  vector<string>::const_iterator ti;
  for (ti = tokens.begin(); ti != tokens.end(); ++ti) {
    if ((*ti).empty()) {
      return string();
    }
  }

  string result = "1";
  if (!tokens.empty()) {
    result = tokens.back();
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_upcase
//       Access: Private
//  Description: Expands the "upcase" function variable.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_upcase(const string &params) {
  string result = expand_string(params);
  string::iterator si;
  for (si = result.begin(); si != result.end(); ++si) {
    (*si) = toupper(*si);
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_downcase
//       Access: Private
//  Description: Expands the "downcase" function variable.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_downcase(const string &params) {
  string result = expand_string(params);
  string::iterator si;
  for (si = result.begin(); si != result.end(); ++si) {
    (*si) = tolower(*si);
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_cdefine
//       Access: Private
//  Description: Expands the "cdefine" function variable.  This is a
//               convenience function to output a C-style #define or
//               #undef statement based on the value of the named
//               variable.  If the named string is a variable whose
//               definition is nonempty, this returns "#define varname
//               definition".  Otherwise, it returns "#undef varname".
//               This is particularly useful for building up a
//               config.h file.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_cdefine(const string &params) {
  string varname = trim_blanks(params);
  string expansion = trim_blanks(expand_variable(varname));

  string result;
  if (expansion.empty()) {
    result = "#undef " + varname;
  } else {
    result = "#define " + varname + " " + expansion;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_closure
//       Access: Private
//  Description: Expands the "closure" function variable.  This is a
//               special function that recursively expands a map
//               variable with the given parameter string until all
//               definitions have been encountered.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_closure(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, false);

  if (tokens.size() != 2 && tokens.size() != 3) {
    cerr << "closure requires two or three parameters.\n";
    errors_occurred = true;
    return string();
  }

  // The first parameter is the map variable name, the second
  // parameter is the expression to evaluate, and the third parameter
  // (if present) is the expression that leads to the recursive
  // evaluation of the map variable.
  string varname = expand_string(tokens[0]);
  string expression = tokens[1];
  string close_on = expression;
  if (tokens.size() > 2) {
    close_on = tokens[2];
  }

  const MapVariableDefinition &def = find_map_variable(varname);
  if (&def == &_null_map_def) {
    cerr << "Warning:  undefined map variable: " << varname << "\n";
    return string();
  }

  // Now evaluate the expression within this scope, and then again
  // within each scope indicated by the result, and then within each
  // scope indicated by *that* result, and so on.  We need to keep
  // track of the words we have already evaluated (hence the set), and
  // we also need to keep track of all the partial results we have yet
  // to evaluate (hence the vector of strings).
  set<string> closure;
  vector<string> results;
  vector<string> next_pass;

  // Start off with the expression evaluated within the starting
  // scope.
  results.push_back(expand_string(expression));

  next_pass.push_back(expand_string(close_on));

  while (!next_pass.empty()) {
    // Pull off one of the partial results (it doesn't matter which
    // one), and chop it up into its constituent words.
    vector<string> pass;
    tokenize_whitespace(next_pass.back(), pass);
    next_pass.pop_back();

    // And then map each of those words into scopes.
    vector<string>::const_iterator wi;
    for (wi = pass.begin(); wi != pass.end(); ++wi) {
      const string &word = (*wi);
      bool inserted = closure.insert(word).second;
      if (inserted) {
        // This is a new word, which presumably maps to a scope.
        MapVariableDefinition::const_iterator di;
        di = def.find(word);
        if (di != def.end()) {
          PPScope *scope = (*di).second;
          // Evaluate the expression within this scope.
          results.push_back(scope->expand_string(expression));
      
          // What does close_on evaluate to within this scope?  That
          // points us to the next scope(s).
          next_pass.push_back(scope->expand_string(close_on));
        }
      }
    }
  }

  // Now we have the complete transitive closure of $[mapvar close_on].
  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_unmapped
//       Access: Private
//  Description: Expands the "closure" function variable.  This is a
//               special function that returns all the arguments to a
//               map variable, unchanged, that did *not* match any of
//               the keys in the map.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_unmapped(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, false);

  if (tokens.size() != 2) {
    cerr << "unmapped requires two parameters.\n";
    errors_occurred = true;
    return string();
  }

  // The first parameter is the map variable name, and the second
  // parameter is the space-separated list of arguments to the map.
  string varname = expand_string(tokens[0]);
  vector<string> keys;
  tokenize_whitespace(expand_string(tokens[1]), keys);

  const MapVariableDefinition &def = find_map_variable(varname);
  if (&def == &_null_map_def) {
    cerr << "Warning:  undefined map variable: " << varname << "\n";
    return string();
  }

  vector<string> results;
  vector<string>::const_iterator ki;
  for (ki = keys.begin(); ki != keys.end(); ++ki) {
    MapVariableDefinition::const_iterator di;
    di = def.find(*ki);
    if (di == def.end()) {
      // This key was undefined.
      results.push_back(*ki);
    }
  }

  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_dependencies
//       Access: Private
//  Description: Expands the "dependencies" function variable.  This
//               function returns all of the inter-file dependencies
//               that the named file(s) depend on, as defined by the
//               #include directives appearing within the files.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_dependencies(const string &params) {
  // Split the string up into filenames based on whitespace.
  vector<string> filenames;
  tokenize_whitespace(expand_string(params), filenames);

  PPDirectory *directory = get_directory();
  assert(directory != (PPDirectory *)NULL);

  vector<string> results;
  vector<string>::const_iterator fi;
  for (fi = filenames.begin(); fi != filenames.end(); ++fi) {
    PPDependableFile *file = directory->get_dependable_file(*fi, false);
    assert(file != (PPDependableFile *)NULL);

    vector<PPDependableFile *> files;
    file->get_complete_dependencies(files);

    vector<PPDependableFile *>::const_iterator dfi;
    for (dfi = files.begin(); dfi != files.end(); ++dfi) {
      PPDependableFile *df = (*dfi);
      string rel_filename =
        current_output_directory->get_rel_to(df->get_directory()) + "/" +
        df->get_filename();
      results.push_back(rel_filename);
    }
  }

  sort(results.begin(), results.end());
  results.erase(unique(results.begin(), results.end()), results.end());

  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_foreach
//       Access: Private
//  Description: Expands the "foreach" function variable.  This
//               evaluates an expression once for each word of a list.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_foreach(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, false);

  if (tokens.size() != 3) {
    cerr << "foreach requires three parameters.\n";
    errors_occurred = true;
    return string();
  }

  // The first parameter is the temporary variable name that holds
  // each word as it is expanded; the second parameter is the
  // space-separated list of words.  The third parameter is the
  // expression to evaluate.
  string varname = trim_blanks(expand_string(tokens[0]));
  vector<string> words;
  tokenize_whitespace(expand_string(tokens[1]), words);

  vector<string> results;
  vector<string>::const_iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    define_variable(varname, *wi);
    results.push_back(expand_string(tokens[2]));
  }

  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_forscopes
//       Access: Private
//  Description: Expands the "forscopes" function variable.  This
//               evaluates an expression once within each of a number
//               of named nested scopes.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_forscopes(const string &params) {
  // Split the string up into tokens based on the commas.
  vector<string> tokens;
  tokenize_params(params, tokens, false);

  if (tokens.size() != 2) {
    cerr << "forscopes requires two parameters.\n";
    errors_occurred = true;
    return string();
  }

  // The first parameter is the space-separated list of nested scope
  // names.  The second parameter is the expression to evaluate.
  vector<string> scope_names;
  tokenize_whitespace(expand_string(tokens[0]), scope_names);

  if (_named_scopes == (PPNamedScopes *)NULL) {
    return string();
  }

  // Now build up the list of scopes with these names.
  PPNamedScopes::Scopes scopes;
  vector<string>::const_iterator wi;
  for (wi = scope_names.begin(); wi != scope_names.end(); ++wi) {
    _named_scopes->get_scopes(*wi, scopes);
  }
  PPNamedScopes::sort_by_dependency(scopes);

  // Now evaluate the expression within each scope.

  vector<string> results;
  PPNamedScopes::Scopes::const_iterator si;
  for (si = scopes.begin(); si != scopes.end(); ++si) {
    PPScope *scope = *si;
    results.push_back(scope->expand_string(tokens[1]));
  }

  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_function
//       Access: Private
//  Description: Expands the user-defined function reference.  This
//               invokes the nested commands within the function body,
//               and returns all the output text as one line.  Quite a
//               job, really.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_function(const string &funcname, 
                const PPSubroutine *sub, const string &params) {
  PPScope::push_scope((PPScope *)this);
  PPScope nested_scope(_named_scopes);
  nested_scope.define_formals(funcname, sub->_formals, params);

#ifdef HAVE_SSTREAM
  ostringstream ostr;
#else
  ostrstream ostr;
#endif

  PPCommandFile command(&nested_scope);
  command.set_output(&ostr);

  command.begin_read();
  bool okflag = true;
  vector<string>::const_iterator li;
  for (li = sub->_lines.begin(); li != sub->_lines.end() && okflag; ++li) {
    okflag = command.read_line(*li);
  }
  if (okflag) {
    okflag = command.end_read();
  }
  // We don't do anything with okflag here.  What can we do?

  PPScope::pop_scope();

  // Now get the output.  We split it into words and then reconnect
  // it, to replace all whitespace with spaces.
#ifdef HAVE_SSTREAM
  string str = ostr.str();
#else
  ostr << ends;
  char *c_str = ostr.str();
  string str = c_str;
  delete[] c_str;
#endif

  vector<string> results;
  tokenize_whitespace(str, results);

  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_map_variable
//       Access: Private
//  Description: Expands a map variable function reference.  This
//               looks up the given keys in the map and expands the
//               first parameter for each corresponding scope.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_map_variable(const string &varname, const string &params) {
  // Split the string up into tokens based on the commas, but don't
  // expand the variables yet.
  vector<string> tokens;
  tokenize_params(params, tokens, false);

  if (tokens.size() != 2) {
    cerr << "map variable expansions require two parameters: $["
         << varname << " " << params << "]\n";
    errors_occurred = true;
    return string();
  }

  // Split the second parameter into tokens based on the spaces.  This
  // is the set of keys.
  vector<string> keys;
  tokenize_whitespace(expand_string(tokens[1]), keys);

  return expand_map_variable(varname, tokens[0], keys);
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::expand_map_variable
//       Access: Private
//  Description: Expands a map variable function reference.  This
//               looks up the given keys in the map and expands the
//               expression for each corresponding scope.
////////////////////////////////////////////////////////////////////
string PPScope::
expand_map_variable(const string &varname, const string &expression,
                    const vector<string> &keys) {
  const MapVariableDefinition &def = find_map_variable(varname);
  if (&def == &_null_map_def) {
    cerr << "Warning:  undefined map variable: " << varname << "\n";
    return string();
  }

  vector<string> results;

  // Now build up the set of expansions of the expression in the
  // various scopes indicated by the keys.
  vector<string>::const_iterator wi;
  for (wi = keys.begin(); wi != keys.end(); ++wi) {
    MapVariableDefinition::const_iterator di;
    di = def.find(*wi);
    if (di != def.end()) {
      PPScope *scope = (*di).second;
      string expansion = scope->expand_string(expression);
      if (!expansion.empty()) {
        results.push_back(expansion);
      }
    }
  }

  string result = repaste(results, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::r_expand_matrix
//       Access: Private
//  Description: The recursive implementation of expand_matrix().
//               This generates all of the combinations from the
//               indicated index into the words array, with the given
//               prefix.
////////////////////////////////////////////////////////////////////
void PPScope::
r_expand_matrix(vector<string> &results, const vector<vector<string> > &words,
                int index, const string &prefix) {
  if (index >= (int)words.size()) {
    // This is the terminal condition.
    results.push_back(prefix);

  } else {
    // Otherwise, tack on the next set of words, and recurse.
    const vector<string> &w = words[index];
    vector<string>::const_iterator wi;
    for (wi = w.begin(); wi != w.end(); ++wi) {
      r_expand_matrix(results, words, index + 1, prefix + (*wi));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::p_find_map_variable
//       Access: Private
//  Description: The implementation of find_map_variable() for a
//               particular static scope, without checking the stack.
////////////////////////////////////////////////////////////////////
PPScope::MapVariableDefinition &PPScope::
p_find_map_variable(const string &varname) {
  MapVariables::const_iterator mvi;
  mvi = _map_variables.find(varname);
  if (mvi != _map_variables.end()) {
    return (MapVariableDefinition &)(*mvi).second;
  }

  if (_parent_scope != (PPScope *)NULL) {
    return _parent_scope->p_find_map_variable(varname);
  }

  return _null_map_def;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::glob_string
//       Access: Private
//  Description: Expands the words in the string as if they were a set
//               of filenames using the shell globbing characters.
//               Fills up the results vector (which the user should
//               ensure is empty before calling) with the set of all
//               files that actually match the globbing characters.
////////////////////////////////////////////////////////////////////
void PPScope::
glob_string(const string &str, vector<string> &results) {
  // The globbing is relative to THISDIRPREFIX, not necessarily the
  // current directory.
  string dirname = trim_blanks(expand_variable("THISDIRPREFIX"));

  vector<string> words;
  tokenize_whitespace(str, words);

  vector<string>::const_iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    GlobPattern glob(*wi);
    glob.match_files(results, dirname);
  }

  // Sort the results into alphabetical order.
  sort(results.begin(), results.end());
}
