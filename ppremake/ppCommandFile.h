// Filename: ppCommandFile.h
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PPCOMMANDFILE_H
#define PPCOMMANDFILE_H

#include "ppremake.h"

#include <map>
#include <vector>

class PPScope;

///////////////////////////////////////////////////////////////////
// 	 Class : PPCommandFile
// Description : This encapsulates a file that contains #commands to
//               execute (like #define, #if, #begin .. #end),
//               $[variables] to expand, and plain text to output.
////////////////////////////////////////////////////////////////////
class PPCommandFile {
public:
  PPCommandFile(PPScope *scope);
  ~PPCommandFile();

  void set_output(ostream *out);

  void set_scope(PPScope *scope);
  PPScope *get_scope() const;

  bool read_file(const string &filename);
  bool read_stream(istream &in);
  void begin_read();
  bool read_line(string line);
  bool end_read();

protected:
  bool handle_command(const string &line);
  bool handle_if_command();
  bool handle_elif_command();
  bool handle_else_command();
  bool handle_endif_command();

  bool handle_begin_command();
  bool handle_forscopes_command();
  bool handle_foreach_command();
  bool handle_formap_command();
  bool handle_format_command();
  bool handle_output_command();
  bool handle_print_command();
  bool handle_defsub_command(bool is_defsub);
  bool handle_end_command();

  bool handle_include_command();
  bool handle_sinclude_command();
  bool handle_call_command();
  bool handle_error_command();

  bool handle_defer_command();
  bool handle_define_command();
  bool handle_set_command();
  bool handle_map_command();
  bool handle_addmap_command();

  bool include_file(const string &filename);
  bool replay_forscopes(const string &name);
  bool replay_foreach(const string &varname, const vector<string> &words);
  bool replay_formap(const string &varname, const string &mapvar);
  bool compare_output(const string &temp_name, const string &true_name);
  bool failed_if() const;

  bool is_valid_formal(const string &formal_parameter_name) const;

private:
  class PushFilename {
  public:
    PushFilename(PPScope *scope, const string &filename);
    ~PushFilename();

    PPScope *_scope;
    string _old_thisdirprefix;
    string _old_thisfilename;
  };

private:
  PPScope *_native_scope;
  PPScope *_scope;

  enum IfState {
    IS_on,   // e.g. a passed #if
    IS_else, // after matching an #else
    IS_off,  // e.g. a failed #if
    IS_done  // e.g. after reaching an #else or #elif for a passed #if.
  };

  class IfNesting {
  public:
    IfState _state;
    IfNesting *_next;
  };

  enum BlockState {
    BS_begin,
    BS_forscopes,
    BS_nested_forscopes,
    BS_foreach,
    BS_nested_foreach,
    BS_formap,
    BS_nested_formap,
    BS_defsub,
    BS_defun,
    BS_output
  };

  enum WriteFormat {
    WF_straight,   // write lines directly as they come in
    WF_collapse,   // collapse out consecutive blank lines
    WF_makefile    // fancy makefile formatting
  };

  class WriteState {
  public:
    WriteState();
    WriteState(const WriteState &copy);
    bool write_line(const string &line);
    bool write_collapse_line(const string &line);
    bool write_makefile_line(const string &line);

    ostream *_out;
    WriteFormat _format;
    bool _last_blank;
  };
  
  class BlockNesting {
  public:
    BlockState _state;
    string _name;
    WriteState *_write_state;
    PPScope *_scope;
    string _true_name;
    char *_tempnam;
    ofstream _output;
    vector<string> _words;
    BlockNesting *_next;
  };

  bool _got_command;
  bool _in_for;
  IfNesting *_if_nesting;
  BlockNesting *_block_nesting;
  string _command;
  string _params;
  WriteState *_write_state;

  vector<string> _saved_lines;

  friend PPCommandFile::IfNesting;
  friend PPCommandFile::WriteState;
  friend PPCommandFile::BlockNesting;
};


#endif
