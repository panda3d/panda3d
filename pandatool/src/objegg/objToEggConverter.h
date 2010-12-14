// Filename: ObjToEggConverter.h
// Created by:  drose (07Dec10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef ObjTOEGGCONVERTER_H
#define ObjTOEGGCONVERTER_H

#include "pandatoolbase.h"

#include "somethingToEggConverter.h"
#include "eggVertexPool.h"
#include "eggGroup.h"

////////////////////////////////////////////////////////////////////
//       Class : ObjToEggConverter
// Description : Convert an Obj file to egg data.
////////////////////////////////////////////////////////////////////
class ObjToEggConverter : public SomethingToEggConverter {
public:
  ObjToEggConverter();
  ObjToEggConverter(const ObjToEggConverter &copy);
  ~ObjToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual string get_name() const;
  virtual string get_extension() const;
  virtual bool supports_compressed() const;

  virtual bool convert_file(const Filename &filename);

protected:
  bool process(const Filename &filename);
  bool process_line(const string &line);

  bool process_v(vector_string &words);
  bool process_vt(vector_string &words);
  bool process_vn(vector_string &words);
  bool process_f(vector_string &words);
  bool process_g(vector_string &words);

  EggVertex *get_vertex(int n);
  EggVertex *get_face_vertex(const string &face_reference);

  int _line_number;
  int _vi, _vti, _vni;
  PT(EggVertexPool) _vpool;
  PT(EggGroup) _root_group;
  EggGroup *_current_group;

  pset<string> _ignored_tags;
};

#endif
