/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrmlToEggConverter.h
 * @author drose
 * @date 2004-10-01
 */

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

/**
 * This class supervises the construction of an EggData structure from a VRML
 * file.
 */
class VRMLToEggConverter : public SomethingToEggConverter {
public:
  VRMLToEggConverter();
  VRMLToEggConverter(const VRMLToEggConverter &copy);
  ~VRMLToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual std::string get_name() const;
  virtual std::string get_extension() const;
  virtual bool supports_compressed() const;

  virtual bool convert_file(const Filename &filename);

private:
  typedef pmap<std::string, VrmlNode *> Nodes;

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
