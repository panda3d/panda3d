// Filename: ppCommandFile.h
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PPCOMMANDFILE_H
#define PPCOMMANDFILE_H

#include "ppremake.h"
#include "filename.h"

#include <map>
#include <vector>

class PPScope;

///////////////////////////////////////////////////////////////////
//       Class : PPCommandFile
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

  bool read_file(Filename filename);
  bool read_stream(istream &in, const string &filename);
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
  bool handle_while_command();
  bool handle_for_command();
  bool handle_forscopes_command();
  bool handle_foreach_command();
  bool handle_formap_command();
  bool handle_defsub_command(bool is_defsub);
  bool handle_output_command();
  bool handle_end_command();

  bool handle_format_command();
  bool handle_print_command();
  bool handle_printvar_command();
  bool handle_include_command();
  bool handle_sinclude_command();
  bool handle_copy_command();
  bool handle_call_command();
  bool handle_error_command();
  bool handle_mkdir_command();

  bool handle_defer_command();
  bool handle_define_command();
  bool handle_set_command();
  bool handle_map_command();
  bool handle_addmap_command();
  bool handle_push_command();

  bool include_file(Filename filename);
  bool replay_while(const string &name);
  bool replay_for(const string &name, const vector<string> &words);
  bool replay_forscopes(const string &name);
  bool replay_foreach(const string &varname, const vector<string> &words);
  bool replay_formap(const string &varname, const string &mapvar);
  bool compare_output(const string &new_contents, Filename filename,
                      bool notouch, bool binary);
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
  class BlockNesting;

  enum IfState {
    IS_on,   // e.g. a passed #if
    IS_else, // after matching an #else
    IS_off,  // e.g. a failed #if
    IS_done  // e.g. after reaching an #else or #elif for a passed #if.
  };

  class IfNesting {
  public:
    IfNesting(IfState state);
    void push(PPCommandFile *file);
    void pop(PPCommandFile *file);

    IfState _state;
    BlockNesting *_block;
    IfNesting *_next;
  };

  enum BlockState {
    BS_begin,
    BS_while,
    BS_nested_while,
    BS_for,
    BS_nested_for,
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
    WF_error,      // anything other than whitespace is an error.
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

  enum OutputFlags {
    OF_notouch  = 0x001,
    OF_binary   = 0x002,
  };
  
  class BlockNesting {
  public:
    BlockNesting(BlockState state, const string &name);
    void push(PPCommandFile *file);
    void pop(PPCommandFile *file);

    BlockState _state;
    string _name;
    IfNesting *_if;
    WriteState *_write_state;
    PPScope *_scope;
    string _params;
#ifdef HAVE_SSTREAM
    ostringstream _output;
#else
    ostrstream _output;
#endif
    vector<string> _words;
    int _flags;
    BlockNesting *_next;
  };

  PPScope *_native_scope;
  PPScope *_scope;
  bool _got_command;
  bool _in_for;
  IfNesting *_if_nesting;
  BlockNesting *_block_nesting;
  string _command;
  string _params;
  WriteState *_write_state;

  vector<string> _saved_lines;

  friend class PPCommandFile::IfNesting;
  friend class PPCommandFile::WriteState;
  friend class PPCommandFile::BlockNesting;
};


#endif
