/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoToEggConverter.h
 * @author drose
 * @date 2001-04-17
 */

#ifndef LWOTOEGGCONVERTER_H
#define LWOTOEGGCONVERTER_H

#include "pandatoolbase.h"

#include "somethingToEggConverter.h"
#include "lwoHeader.h"
#include "pointerTo.h"

#include "pvector.h"
#include "pmap.h"

class CLwoLayer;
class CLwoClip;
class CLwoPoints;
class CLwoPolygons;
class CLwoSurface;
class LwoClip;

/**
 * This class supervises the construction of an EggData structure from the
 * data represented by the LwoHeader.  Reading and writing the egg and lwo
 * structures is left to the user.
 */
class LwoToEggConverter : public SomethingToEggConverter {
public:
  LwoToEggConverter();
  LwoToEggConverter(const LwoToEggConverter &copy);
  virtual ~LwoToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual std::string get_name() const;
  virtual std::string get_extension() const;

  virtual bool convert_file(const Filename &filename);
  bool convert_lwo(const LwoHeader *lwo_header);
  virtual bool supports_compressed() const;

  CLwoLayer *get_layer(int number) const;
  CLwoClip *get_clip(int number) const;

  CLwoSurface *get_surface(const std::string &name) const;

  bool _make_materials;

private:
  void cleanup();

  void collect_lwo();
  void make_egg();
  void connect_egg();

  void slot_layer(int number);
  void slot_clip(int number);
  CLwoLayer *make_generic_layer();

  CPT(LwoHeader) _lwo_header;

  CLwoLayer *_generic_layer;
  typedef pvector<CLwoLayer *> Layers;
  Layers _layers;

  typedef pvector<CLwoClip *> Clips;
  Clips _clips;

  typedef pvector<CLwoPoints *> Points;
  Points _points;

  typedef pvector<CLwoPolygons *> Polygons;
  Polygons _polygons;

  typedef pmap<std::string, CLwoSurface *> Surfaces;
  Surfaces _surfaces;
};

#include "lwoToEggConverter.I"

#endif
