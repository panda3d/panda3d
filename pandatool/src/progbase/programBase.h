/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file programBase.h
 * @author drose
 * @date 2000-02-13
 */

#ifndef PROGRAMBASE_H
#define PROGRAMBASE_H

#include "pandatoolbase.h"

#include "distanceUnit.h"
#include "pathReplace.h"
#include "pathStore.h"
#include "filename.h"
#include "pointerTo.h"
#include "vector_string.h"
#include "pvector.h"
#include "pdeque.h"
#include "pmap.h"

/**
 * This is intended to be the base class for most general-purpose utility
 * programs in the PANDATOOL tree.  It automatically handles things like
 * command-line arguments in a portable way.
 */
class ProgramBase {
public:
  ProgramBase(const std::string &name = std::string());
  virtual ~ProgramBase();

  void show_description();
  void show_usage();
  void show_options();

  INLINE void show_text(const std::string &text);
  void show_text(const std::string &prefix, int indent_width, std::string text);

  void write_man_page(std::ostream &out);

  virtual void parse_command_line(int argc, char **argv);

  std::string get_exec_command() const;

  typedef pdeque<std::string> Args;
  Filename _program_name;
  Args _program_args;

protected:
  typedef bool (*OptionDispatchFunction)(const std::string &opt, const std::string &parm, void *data);
  typedef bool (*OptionDispatchMethod)(ProgramBase *self, const std::string &opt, const std::string &parm, void *data);

  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

  void set_program_brief(const std::string &brief);
  void set_program_description(const std::string &description);
  void clear_runlines();
  void add_runline(const std::string &runline);
  void clear_options();
  void add_option(const std::string &option, const std::string &parm_name,
                  int index_group, const std::string &description,
                  OptionDispatchFunction option_function,
                  bool *bool_var = nullptr,
                  void *option_data = nullptr);
  void add_option(const std::string &option, const std::string &parm_name,
                  int index_group, const std::string &description,
                  OptionDispatchMethod option_method,
                  bool *bool_var = nullptr,
                  void *option_data = nullptr);
  bool redescribe_option(const std::string &option, const std::string &description);
  bool remove_option(const std::string &option);

  void add_path_replace_options();
  void add_path_store_options();

  static bool dispatch_none(const std::string &opt, const std::string &arg, void *);
  static bool dispatch_true(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_false(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_count(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_int(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_int_pair(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_int_quad(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_double(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_double_pair(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_double_triple(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_double_quad(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_color(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_string(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_vector_string(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_vector_string_comma(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_filename(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_search_path(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_coordinate_system(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_units(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_image_type(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_path_replace(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_path_store(const std::string &opt, const std::string &arg, void *var);

  static bool handle_help_option(const std::string &opt, const std::string &arg, void *);

  static void format_text(std::ostream &out, bool &last_newline,
                          const std::string &prefix, int indent_width,
                          const std::string &text, int line_width);

  PT(PathReplace) _path_replace;
  bool _got_path_store;
  bool _got_path_directory;


private:
  void sort_options();
  void get_terminal_width();

  class Option {
  public:
    std::string _option;
    std::string _parm_name;
    int _index_group;
    int _sequence;
    std::string _description;
    OptionDispatchFunction _option_function;
    OptionDispatchMethod _option_method;
    bool *_bool_var;
    void *_option_data;
  };

  class SortOptionsByIndex {
  public:
    bool operator () (const Option *a, const Option *b) const;
  };

  std::string _name;
  std::string _brief;
  std::string _description;
  typedef vector_string Runlines;
  Runlines _runlines;

  typedef pmap<std::string, Option> OptionsByName;
  typedef pvector<const Option *> OptionsByIndex;
  OptionsByName _options_by_name;
  OptionsByIndex _options_by_index;
  int _next_sequence;
  bool _sorted_options;

  typedef pmap<std::string, std::string> GotOptions;
  GotOptions _got_options;

  bool _last_newline;
  int _terminal_width;
  bool _got_terminal_width;
  int _option_indent;
  bool _got_option_indent;
};

#include "programBase.I"

#endif
