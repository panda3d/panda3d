// Filename: somethingToEggConverter.h
// Created by:  drose (17Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef SOMETHINGTOEGGCONVERTER_H
#define SOMETHINGTOEGGCONVERTER_H

#include "pandatoolbase.h"

#include "filename.h"
#include "config_util.h"  // for get_texture_path() and get_model_path()
#include "animationConvert.h"
#include "pathReplace.h"
#include "pointerTo.h"
#include "distanceUnit.h"

class EggData;
class EggGroupNode;

////////////////////////////////////////////////////////////////////
//       Class : SomethingToEggConverter
// Description : This is a base class for a family of converter
//               classes that manage a conversion from some file type
//               to egg format.
//
//               Classes of this type can be used to implement xxx2egg
//               converter programs, as well as LoaderFileTypeXXX
//               run-time loaders.
////////////////////////////////////////////////////////////////////
class SomethingToEggConverter {
public:
  SomethingToEggConverter();
  SomethingToEggConverter(const SomethingToEggConverter &copy);
  virtual ~SomethingToEggConverter();

  virtual SomethingToEggConverter *make_copy()=0;

  INLINE void set_path_replace(PathReplace *path_replace);
  INLINE PathReplace *get_path_replace();
  INLINE const PathReplace *get_path_replace() const;

  // These methods dealing with animation and frame rate are only
  // relevant to converter types that understand animation.
  INLINE void set_animation_convert(AnimationConvert animation_convert);
  INLINE AnimationConvert get_animation_convert() const;

  INLINE void set_character_name(const string &character_name);
  INLINE const string &get_character_name() const;

  INLINE void set_start_frame(double start_frame);
  INLINE bool has_start_frame() const;
  INLINE double get_start_frame() const;
  INLINE void clear_start_frame();

  INLINE void set_end_frame(double end_frame);
  INLINE bool has_end_frame() const;
  INLINE double get_end_frame() const;
  INLINE void clear_end_frame();

  INLINE void set_frame_inc(double frame_inc);
  INLINE bool has_frame_inc() const;
  INLINE double get_frame_inc() const;
  INLINE void clear_frame_inc();

  INLINE void set_neutral_frame(double neutral_frame);
  INLINE bool has_neutral_frame() const;
  INLINE double get_neutral_frame() const;
  INLINE void clear_neutral_frame();

  INLINE void set_input_frame_rate(double input_frame_rate);
  INLINE bool has_input_frame_rate() const;
  INLINE double get_input_frame_rate() const;
  INLINE void clear_input_frame_rate();

  INLINE void set_output_frame_rate(double output_frame_rate);
  INLINE bool has_output_frame_rate() const;
  INLINE double get_output_frame_rate() const;
  INLINE void clear_output_frame_rate();

  INLINE static double get_default_frame_rate();
  
  INLINE void set_merge_externals(bool merge_externals);
  INLINE bool get_merge_externals() const;

  void set_egg_data(EggData *egg_data, bool owns_egg_data);
  INLINE void clear_egg_data();
  INLINE EggData &get_egg_data();

  virtual string get_name() const=0;
  virtual string get_extension() const=0;
  virtual string get_additional_extensions() const;

  virtual bool convert_file(const Filename &filename)=0;
  virtual DistanceUnit get_input_units();

  bool handle_external_reference(EggGroupNode *egg_parent,
                                 const Filename &ref_filename);

  INLINE Filename convert_texture_path(const Filename &orig_filename);
  INLINE Filename convert_model_path(const Filename &orig_filename);

  // Set this true to treat errors as warnings and generate output
  // anyway.
  bool _allow_errors;

protected:
  PT(PathReplace) _path_replace;

  AnimationConvert _animation_convert;
  string _character_name;
  double _start_frame;
  double _end_frame;
  double _frame_inc;
  double _neutral_frame;
  double _input_frame_rate;   // frames per second
  double _output_frame_rate;  // frames per second
  enum ControlFlags {
    CF_start_frame        = 0x0001,
    CF_end_frame          = 0x0002,
    CF_frame_inc          = 0x0004,
    CF_neutral_frame      = 0x0008,
    CF_input_frame_rate   = 0x0010,
    CF_output_frame_rate  = 0x0020,
  };
  int _control_flags;

  bool _merge_externals;

  EggData *_egg_data;
  bool _owns_egg_data;

  bool _error;
};

#include "somethingToEggConverter.I"

#endif


