// Filename: lwoToEggConverter.h
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

////////////////////////////////////////////////////////////////////
//       Class : LwoToEggConverter
// Description : This class supervises the construction of an EggData
//               structure from the data represented by the LwoHeader.
//               Reading and writing the egg and lwo structures is
//               left to the user.
////////////////////////////////////////////////////////////////////
class LwoToEggConverter : public SomethingToEggConverter {
public:
  LwoToEggConverter();
  LwoToEggConverter(const LwoToEggConverter &copy);
  virtual ~LwoToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual string get_name() const;
  virtual string get_extension() const;

  virtual bool convert_file(const Filename &filename);
  bool convert_lwo(const LwoHeader *lwo_header);

  CLwoLayer *get_layer(int number) const;
  CLwoClip *get_clip(int number) const;

  CLwoSurface *get_surface(const string &name) const;

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

  typedef pmap<string, CLwoSurface *> Surfaces;
  Surfaces _surfaces;
};

#include "lwoToEggConverter.I"

#endif


