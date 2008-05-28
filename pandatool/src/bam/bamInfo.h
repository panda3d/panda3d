// Filename: bamInfo.h
// Created by:  drose (02Jul00)
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

#ifndef BAMINFO_H
#define BAMINFO_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "filename.h"
#include "sceneGraphAnalyzer.h"

#include "pvector.h"

class TypedWritable;
class PandaNode;
class Texture;
class BamCacheIndex;
class RecorderHeader;

////////////////////////////////////////////////////////////////////
//       Class : BamInfo
// Description :
////////////////////////////////////////////////////////////////////
class BamInfo : public ProgramBase {
public:
  BamInfo();

  void run();

protected:
  virtual bool handle_args(Args &args);

private:
  typedef pvector<TypedWritable *> Objects;

  bool get_info(const Filename &filename);
  void describe_scene_graph(PandaNode *node);
  void describe_texture(Texture *tex);
  void describe_cache_index(BamCacheIndex *index);
  void describe_session(RecorderHeader *header, const Objects &objects);
  void describe_general_object(TypedWritable *object);
  void list_hierarchy(PandaNode *node, int indent_level);

  typedef pvector<Filename> Filenames;
  Filenames _filenames;

  bool _ls;
  bool _verbose_transitions;
  bool _verbose_geoms;

  int _num_scene_graphs;
  SceneGraphAnalyzer _analyzer;
};

#endif
