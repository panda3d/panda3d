/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToSomethingConverter.h
 * @author drose
 * @date 2012-09-26
 */

#ifndef EGGTOSOMETHINGCONVERTER_H
#define EGGTOSOMETHINGCONVERTER_H

#include "pandatoolbase.h"

#include "filename.h"
#include "pointerTo.h"
#include "distanceUnit.h"
#include "coordinateSystem.h"

class EggData;
class EggGroupNode;

/**
 * This is a base class for a family of converter classes that manage a
 * conversion from egg format to some other file type.
 *
 * Classes of this type can be used to implement egg2xxx converter programs,
 * as well as LoaderFileTypeXXX run-time savers.
 */
class EggToSomethingConverter {
public:
  EggToSomethingConverter();
  EggToSomethingConverter(const EggToSomethingConverter &copy);
  virtual ~EggToSomethingConverter();

  virtual EggToSomethingConverter *make_copy()=0;

  INLINE void clear_error();
  INLINE bool had_error() const;

  void set_egg_data(EggData *egg_data);
  INLINE void clear_egg_data();
  INLINE EggData *get_egg_data();

  INLINE void set_output_units(DistanceUnit output_units);
  INLINE DistanceUnit get_output_units() const;
  INLINE void set_output_coordinate_system(CoordinateSystem output_coordinate_system) const;
  INLINE CoordinateSystem get_output_coordinate_system() const;

  virtual std::string get_name() const=0;
  virtual std::string get_extension() const=0;
  virtual std::string get_additional_extensions() const;
  virtual bool supports_compressed() const;

  virtual bool write_file(const Filename &filename)=0;

protected:
  PT(EggData) _egg_data;
  DistanceUnit _output_units;
  CoordinateSystem _output_coordinate_system;

  bool _error;
};

#include "eggToSomethingConverter.I"

#endif
