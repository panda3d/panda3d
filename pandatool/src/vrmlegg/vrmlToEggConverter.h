// Filename: vrmlToEggConverter.h
// Created by:  drose (01Oct04)
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

#ifndef VRMLTOEGGCONVERTER_H
#define VRMLTOEGGCONVERTER_H

#include "pandatoolbase.h"

#include "somethingToEggConverter.h"
#include "pmap.h"

class VrmlNode;
struct SFNodeRef;
class EggGroupNode;
class EggGroup;
class LMatrix4d;

////////////////////////////////////////////////////////////////////
//       Class : VRMLToEggConverter
// Description : This class supervises the construction of an EggData
//               structure from a VRML file.
////////////////////////////////////////////////////////////////////
class VRMLToEggConverter : public SomethingToEggConverter {
public:
  VRMLToEggConverter();
  VRMLToEggConverter(const VRMLToEggConverter &copy);
  ~VRMLToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual string get_name() const;
  virtual string get_extension() const;

  virtual bool convert_file(const Filename &filename);

private:
  typedef pmap<string, VrmlNode *> Nodes;

  void get_all_defs(SFNodeRef &vrml, Nodes &nodes);
  void vrml_node(const SFNodeRef &vrml, EggGroupNode *egg, 
                 const LMatrix4d &net_transform);

  void vrml_grouping_node(const SFNodeRef &vrml, EggGroupNode *egg,
                          const LMatrix4d &net_transform,
                          void (VRMLToEggConverter::*process_func)
                          (const VrmlNode *node, EggGroup *group,
                           const LMatrix4d &net_transform));
  void vrml_group(const VrmlNode *node, EggGroup *group,
                  const LMatrix4d &net_transform);
  void vrml_transform(const VrmlNode *node, EggGroup *group,
                      const LMatrix4d &net_transform);
  void vrml_shape(const VrmlNode *node, EggGroup *group,
                  const LMatrix4d &net_transform);
};

#endif


