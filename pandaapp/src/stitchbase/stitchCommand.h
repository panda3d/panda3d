// Filename: stitchCommand.h
// Created by:  drose (08Nov99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef STITCHCOMMAND_H
#define STITCHCOMMAND_H

#include <pandatoolbase.h>

#include <luse.h>

#include <vector>
#include <map>

class StitchLens;
class StitchImage;
class StitchImageOutputter;
class StitchFile;
class Stitcher;

class StitchCommand {
public:
  enum Command {
    C_global,
    C_define,
    C_lens,
    C_input_image,
    C_output_image,
    C_perspective,
    C_fisheye,
    C_cylindrical,
    C_psphere,
    C_focal_length,
    C_fov,
    C_singularity_tolerance,
    C_resolution,
    C_filename,
    C_point2d,
    C_point3d,
    C_show_points,
    C_image_size,
    C_film_size,
    C_film_offset,
    C_grid,
    C_untextured_color,
    C_hpr,
    C_layers,
    C_stitch,
    C_using,
    C_user_command,
  };

  StitchCommand(StitchCommand *parent = NULL,
                Command command = C_global);
  ~StitchCommand();

  void clear();

  void set_name(const string &name);
  void set_length(double number);
  void set_resolution(double number);
  void set_number(double number);
  void set_point2d(const LVecBase2d &point);
  void set_point3d(const LVecBase3d &point);
  void set_length_pair(const LVecBase2d &point);
  void set_color(const Colord &color);
  void set_str(const string &str);
  bool add_using(const string &name);
  void add_nested(StitchCommand *nested);

  string get_name() const;
  double get_number() const;
  LVecBase2d get_point2d() const;
  LVecBase3d get_point3d() const;
  LVector3d get_vector3d() const;
  Colord get_color() const;
  string get_str() const;

  StitchCommand *find_definition(const string &name);

  void process(StitchImageOutputter &outputter, Stitcher *stitcher,
               StitchFile &file);

  void write(ostream &out, int indent) const;


private:
  StitchLens *find_using_lens();
  StitchLens *find_lens();
  StitchLens *make_lens();

  StitchCommand *find_using_command(Command command);
  StitchCommand *find_command(Command command);
  string find_parameter(Command command, const string &dflt);
  double find_parameter(Command command, double dflt);
  LVecBase2d find_parameter(Command command, const LVecBase2d &dflt);

  StitchImage *create_image();


  StitchCommand *_parent;
  Command _command;

  enum Parameters {
    P_name        = 0x001,
    P_length      = 0x002,
    P_resolution  = 0x004,
    P_number      = 0x008,
    P_point2d     = 0x010,
    P_point3d     = 0x020,
    P_length_pair = 0x040,
    P_color       = 0x080,
    P_str         = 0x100,
    P_using       = 0x200,
    P_nested      = 0x400
  };

  int _params;

  string _name;
  double _number;
  double _n[4];
  string _str;

  // This will only get filled in by make_lens().
  StitchLens *_lens;

  typedef vector<StitchCommand *> Commands;
  Commands _using;
  Commands _nested;
};

inline ostream &operator << (ostream &out, const StitchCommand &c) {
  c.write(out, 0);
  return out;
}

ostream &operator << (ostream &out, StitchCommand::Command c);


#endif
