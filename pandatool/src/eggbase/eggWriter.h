// Filename: eggWriter.h
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGWRITER_H
#define EGGWRITER_H

#include <pandatoolbase.h>

#include "eggBase.h"

#include <filename.h>
#include <luse.h>

////////////////////////////////////////////////////////////////////
// 	 Class : EggWriter
// Description : This is the base class for a program that generates
//               an egg file output, but doesn't read any for input.
////////////////////////////////////////////////////////////////////
class EggWriter : virtual public EggBase {
public:
  EggWriter(bool allow_last_param = false, bool allow_stdout = true);

  void add_normals_options();
  void add_transform_options();

  virtual EggWriter *as_writer();

  ostream &get_output();
  bool has_output_filename() const;
  Filename get_output_filename() const;

  virtual void post_process_egg_file();
  void write_egg_file();

protected:
  virtual bool handle_args(Args &args);
  bool check_last_arg(Args &args);
  virtual bool post_command_line();

  static bool dispatch_normals(ProgramBase *self, const string &opt, const string &arg, void *mode);
  bool ns_dispatch_normals(const string &opt, const string &arg, void *mode);

  static bool dispatch_scale(const string &opt, const string &arg, void *var);
  static bool dispatch_rotate_xyz(ProgramBase *self, const string &opt, const string &arg, void *var);
  bool ns_dispatch_rotate_xyz(const string &opt, const string &arg, void *var);
  static bool dispatch_rotate_axis(ProgramBase *self, const string &opt, const string &arg, void *var);
  bool ns_dispatch_rotate_axis(const string &opt, const string &arg, void *var);
  static bool dispatch_translate(const string &opt, const string &arg, void *var);

  bool verify_output_file_safe() const;

protected:
  bool _allow_last_param;
  bool _allow_stdout;
  bool _got_output_filename;
  Filename _output_filename;

  enum NormalsMode {
    NM_strip,
    NM_polygon,
    NM_vertex,
    NM_preserve
  };
  NormalsMode _normals_mode;
  double _normals_threshold;

  bool _got_transform;
  LMatrix4d _transform;

private:
  ofstream _output_stream;
  ostream *_output_ptr;
};

#endif


