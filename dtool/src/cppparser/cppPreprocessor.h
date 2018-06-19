/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppPreprocessor.h
 * @author drose
 * @date 1999-10-22
 */

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

// #define CPP_VERBOSE_LEX

/**
 *
 */
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
  CPPToken peek_next_token();
#ifdef CPP_VERBOSE_LEX
  CPPToken get_next_token0();
  int _token_index;
#endif

  void warning(const std::string &message);
  void warning(const std::string &message, const YYLTYPE &loc);
  void error(const std::string &message);
  void error(const std::string &message, const YYLTYPE &loc);
  void show_line(const YYLTYPE &loc);

  CPPCommentBlock *get_comment_before(int line, CPPFile file);
  CPPCommentBlock *get_comment_on(int line, CPPFile file);

  int get_warning_count() const;
  int get_error_count() const;

  typedef std::map<std::string, CPPManifest *> Manifests;
  Manifests _manifests;

  typedef pvector<CPPManifest *> ManifestStack;
  std::map<std::string, ManifestStack> _manifest_stack;

  pvector<CPPFile::Source> _quote_include_kind;
  DSearchPath _quote_include_path;
  DSearchPath _angle_include_path;
  bool _noangles;

  CPPComments _comments;

  typedef std::set<CPPFile> ParsedFiles;
  ParsedFiles _parsed_files;

  typedef std::set<std::string> Includes;
  Includes _quote_includes;
  Includes _angle_includes;

  std::set<Filename> _explicit_files;

  // This is normally true, to indicate that the preprocessor should decode
  // identifiers like foo::bar<snarf> into a single IDENTIFIER,
  // TYPENAME_IDENTIFIER, or SCOPING token for yacc's convenience.  When
  // false, it leaves them alone and returns a sequence of SIMPLE_IDENTIFIER
  // and SCOPE tokens instead.
  bool _resolve_identifiers;

  // The default _verbose level is 1, which will output normal error and
  // warning messages but nothing else.  Set this to 0 to make the warning
  // messages go away (although the counts will still be incremented), or set
  // it higher to get more debugging information.
  int _verbose;

  // The location of the last token.
  cppyyltype _last_token_loc;

protected:
  bool init_cpp(const CPPFile &file);
  bool init_const_expr(const std::string &expr);
  bool init_type(const std::string &type);
  bool push_file(const CPPFile &file);
  bool push_string(const std::string &input, bool lock_position);

  std::string expand_manifests(const std::string &input_expr, bool expand_undefined,
                          const YYLTYPE &loc);
  CPPExpression *parse_expr(const std::string &expr, CPPScope *current_scope,
                            CPPScope *global_scope, const YYLTYPE &loc);

private:
  CPPToken internal_get_next_token();
  int check_digraph(int c);
  int check_trigraph(int c);
  int skip_whitespace(int c);
  int skip_comment(int c);
  int skip_c_comment(int c);
  int skip_cpp_comment(int c);
  int skip_digit_separator(int c);
  int process_directive(int c);

  int get_preprocessor_command(int c, std::string &command);
  int get_preprocessor_args(int c, std::string &args);

  void handle_define_directive(const std::string &args, const YYLTYPE &loc);
  void handle_undef_directive(const std::string &args, const YYLTYPE &loc);
  void handle_ifdef_directive(const std::string &args, const YYLTYPE &loc);
  void handle_ifndef_directive(const std::string &args, const YYLTYPE &loc);
  void handle_if_directive(const std::string &args, const YYLTYPE &loc);
  void handle_include_directive(const std::string &args, const YYLTYPE &loc);
  void handle_pragma_directive(const std::string &args, const YYLTYPE &loc);
  void handle_error_directive(const std::string &args, const YYLTYPE &loc);

  void skip_false_if_block(bool consider_elifs);
  bool is_manifest_defined(const std::string &manifest_name);
  bool find_include(Filename &filename, bool angle_quotes, CPPFile::Source &source);

  CPPToken get_quoted_char(int c);
  CPPToken get_quoted_string(int c);
  CPPToken get_identifier(int c);
  CPPToken get_literal(int token, YYLTYPE loc, const std::string &str,
                       const YYSTYPE &result = YYSTYPE());
  CPPToken expand_manifest(const CPPManifest *manifest);
  void extract_manifest_args(const std::string &name, int num_args,
                             int va_arg, vector_string &args);
  void expand_defined_function(std::string &expr, size_t q, size_t &p);
  void expand_has_include_function(std::string &expr, size_t q, size_t &p, YYLTYPE loc);
  void expand_manifest_inline(std::string &expr, size_t q, size_t &p,
                              const CPPManifest *manifest);
  void extract_manifest_args_inline(const std::string &name, int num_args,
                                    int va_arg, vector_string &args,
                                    const std::string &expr, size_t &p);

  CPPToken get_number(int c);
  static int check_keyword(const std::string &name);
  int scan_escape_sequence(int c);
  std::string scan_quoted(int c);
  std::string scan_raw(int c);

  bool should_ignore_manifest(const CPPManifest *manifest) const;
  bool should_ignore_preprocessor() const;

  int get();
  int peek();
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
    bool connect_input(const std::string &input);
    int get();
    int peek();

    const CPPManifest *_ignore_manifest;
    CPPFile _file;
    std::string _input;
    std::istream *_in;
    int _line_number;
    int _col_number;
    int _next_line_number;
    int _next_col_number;
    bool _lock_position;
    int _prev_last_c;
  };

  // This must be a list and not a vector because we don't have a good copy
  // constructor defined for InputFile.
  typedef std::list<InputFile> Files;
  Files _files;

  enum State {
    S_normal, S_eof, S_nested, S_end_nested
  };
  State _state;
  int _paren_nesting;
  bool _parsing_template_params;
  bool _parsing_attribute;

  bool _start_of_line;
  int _unget;

  int _last_c;
  bool _last_cpp_comment;
  bool _save_comments;

  std::vector<CPPToken> _saved_tokens;

  int _warning_count;
  int _error_count;
  bool _error_abort;
};

#endif
