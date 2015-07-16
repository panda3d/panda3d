// Filename: cppPreprocessor.h
// Created by:  drose (22Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CPPPREPROCESSOR_H
#define CPPPREPROCESSOR_H

#include "dtoolbase.h"

#include "cppManifest.h"
#include "cppToken.h"
#include "cppFile.h"
#include "cppCommentBlock.h"

#include "dSearchPath.h"
#include "vector_string.h"

#include <map>
#include <list>
#include <vector>

class CPPScope;
class CPPTemplateParameterList;
class CPPExpression;

//#define CPP_VERBOSE_LEX

///////////////////////////////////////////////////////////////////
//       Class : CPPPreprocessor
// Description :
////////////////////////////////////////////////////////////////////
class CPPPreprocessor {
public:
  CPPPreprocessor();

  void set_verbose(int verbose);
  int get_verbose() const;

  void copy_filepos(const CPPPreprocessor &other);

  CPPFile get_file() const;
  int get_line_number() const;
  int get_col_number() const;

  CPPToken get_next_token();
#ifdef CPP_VERBOSE_LEX
  CPPToken get_next_token0();
  int _token_index;
#endif

  void warning(const string &message, int line = 0, int col = 0,
               CPPFile file = CPPFile());
  void error(const string &message, int line = 0, int col = 0,
             CPPFile file = CPPFile());

  CPPCommentBlock *get_comment_before(int line, CPPFile file);
  CPPCommentBlock *get_comment_on(int line, CPPFile file);

  int get_warning_count() const;
  int get_error_count() const;

  typedef map<string, CPPManifest *> Manifests;
  Manifests _manifests;

  pvector<CPPFile::Source> _quote_include_kind;
  DSearchPath _quote_include_path;
  DSearchPath _angle_include_path;
  bool _noangles;

  CPPComments _comments;

  typedef set<CPPFile> ParsedFiles;
  ParsedFiles _parsed_files;

  typedef set<string> Includes;
  Includes _quote_includes;
  Includes _angle_includes;

  // This is normally true, to indicate that the preprocessor should
  // decode identifiers like foo::bar<snarf> into a single IDENTIFIER,
  // TYPENAME_IDENTIFIER, or SCOPING token for yacc's convenience.
  // When false, it leaves them alone and returns a sequence of
  // SIMPLE_IDENTIFIER and SCOPE tokens instead.
  bool _resolve_identifiers;

  // The default _verbose level is 1, which will output normal error
  // and warning messages but nothing else.  Set this to 0 to make the
  // warning messages go away (although the counts will still be
  // incremented), or set it higher to get more debugging information.
  int _verbose;

protected:
  bool init_cpp(const CPPFile &file);
  bool init_const_expr(const string &expr);
  bool init_type(const string &type);
  bool push_file(const CPPFile &file);
  bool push_string(const string &input, bool lock_position);

  string expand_manifests(const string &input_expr, bool expand_undefined);
  CPPExpression *parse_expr(const string &expr, CPPScope *current_scope,
                            CPPScope *global_scope);

private:
  CPPToken internal_get_next_token();
  int skip_whitespace(int c);
  int skip_comment(int c);
  int skip_c_comment(int c);
  int skip_cpp_comment(int c);
  int process_directive(int c);

  int get_preprocessor_command(int c, string &command);
  int get_preprocessor_args(int c, string &args);

  void handle_define_directive(const string &args, int first_line,
                               int first_col, const CPPFile &first_file);
  void handle_undef_directive(const string &args, int first_line,
                              int first_col, const CPPFile &first_file);
  void handle_ifdef_directive(const string &args, int first_line,
                              int first_col, const CPPFile &first_file);
  void handle_ifndef_directive(const string &args, int first_line,
                               int first_col, const CPPFile &first_file);
  void handle_if_directive(const string &args, int first_line,
                           int first_col, const CPPFile &first_file);
  void handle_include_directive(const string &args, int first_line,
                                int first_col, const CPPFile &first_file);
  void handle_pragma_directive(const string &args, int first_line,
                               int first_col, const CPPFile &first_file);
  void handle_error_directive(const string &args, int first_line,
                              int first_col, const CPPFile &first_file);

  void skip_false_if_block(bool consider_elifs);

  CPPToken get_quoted_char(int c);
  CPPToken get_quoted_string(int c);
  CPPToken get_identifier(int c);
  CPPToken expand_manifest(const CPPManifest *manifest);
  void extract_manifest_args(const string &name, int num_args,
                             int va_arg, vector_string &args);
  void expand_defined_function(string &expr, size_t q, size_t &p);
  void expand_manifest_inline(string &expr, size_t q, size_t &p,
                              const CPPManifest *manifest);
  void extract_manifest_args_inline(const string &name, int num_args,
                                    int va_arg, vector_string &args,
                                    const string &expr, size_t &p);

  CPPToken get_number(int c, int c2 = 0);
  static int check_keyword(const string &name);
  int scan_escape_sequence(int c);
  string scan_quoted(int c);

  bool should_ignore_manifest(const CPPManifest *manifest) const;
  bool should_ignore_preprocessor() const;

  int get();
  void unget(int c);

  CPPTemplateParameterList *
  nested_parse_template_instantiation(CPPTemplateScope *scope);
  void skip_to_end_nested();
  void skip_to_angle_bracket();

  class InputFile {
  public:
    InputFile();
    ~InputFile();

    bool open(const CPPFile &file);
    bool connect_input(const string &input);
    int get();

    const CPPManifest *_ignore_manifest;
    CPPFile _file;
    string _input;
    istream *_in;
    int _line_number;
    int _col_number;
    bool _lock_position;
    int _prev_last_c;
  };

  // This must be a list and not a vector because we don't have a good
  // copy constructor defined for InputFile.
  typedef list<InputFile> Files;
  Files _files;

  enum State {
    S_normal, S_eof, S_nested, S_end_nested
  };
  State _state;
  int _paren_nesting;
  bool _angle_bracket_found;

  bool _start_of_line;
  int _unget;

  int _last_c;
  bool _last_cpp_comment;
  bool _save_comments;

  vector<CPPToken> _saved_tokens;

  int _warning_count;
  int _error_count;
  bool _error_abort;
};

#endif
