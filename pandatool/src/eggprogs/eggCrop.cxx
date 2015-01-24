// Filename: eggCrop.cxx
// Created by:  drose (10Jun02)
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

#include "eggCrop.h"

#include "eggGroupNode.h"
#include "eggPrimitive.h"
#include "eggVertex.h"
#include "dcast.h"
#include "pystub.h"

////////////////////////////////////////////////////////////////////
//     Function: EggCrop::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggCrop::
EggCrop() {
  set_program_brief("crop geometry in an .egg file");
  set_program_description
    ("egg-crop strips out all parts of an egg file that fall outside of an "
     "arbitrary bounding volume, specified with a minimum and maximum point "
     "in world coordinates.");

  add_option
    ("min", "x,y,z", 0,
     "Specify the minimum point.",
     &EggCrop::dispatch_double_triple, &_got_min, &_min[0]);

  add_option
    ("max", "x,y,z", 0,
     "Specify the maximum point.",
     &EggCrop::dispatch_double_triple, &_got_max, &_max[0]);
}

////////////////////////////////////////////////////////////////////
//     Function: EggCrop::post_command_line
//       Access: Public, Virtual
//  Description: This is called after the command line has been
//               completely processed, and it gives the program a
//               chance to do some last-minute processing and
//               validation of the options and arguments.  It should
//               return true if everything is fine, false if there is
//               an error.
////////////////////////////////////////////////////////////////////
bool EggCrop::
post_command_line() {
  if (!_got_min || !_got_max) {
    nout << "You must specify both a minimum and a maximum bounds.\n";
    return false;
  }
    
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCrop::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggCrop::
run() {
  int num_removed = strip_prims(_data);
  nout << "Removed " << num_removed << " primitives.\n";

  _data->remove_unused_vertices(true);
  write_egg_file();
}


////////////////////////////////////////////////////////////////////
//     Function: EggCrop::strip_prims
//       Access: Private
//  Description: Recursively walks the scene graph, looking for
//               primitives that exceed the specified bounding volume,
//               and removes them.  Returns the number of primitives
//               removed.
////////////////////////////////////////////////////////////////////
int EggCrop::
strip_prims(EggGroupNode *group) {
  int num_removed = 0;

  EggGroupNode::iterator ci;
  ci = group->begin();
  while (ci != group->end()) {
    EggNode *child = (*ci);
    bool all_in = true;

    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *prim = DCAST(EggPrimitive, child);
      EggPrimitive::iterator vi;
      for (vi = prim->begin(); vi != prim->end() && all_in; ++vi) {
        EggVertex *vert = (*vi);
        LPoint3d pos = vert->get_pos3();

        all_in = (pos[0] >= _min[0] && pos[0] <= _max[0] &&
                  pos[1] >= _min[1] && pos[1] <= _max[1] &&
                  pos[2] >= _min[2] && pos[2] <= _max[2]);
          
      }
    }

    if (!all_in) {
      // Reject this primitive.
      ci = group->erase(ci);
      num_removed++;
    } else {
      // Keep this primitive.
      if (child->is_of_type(EggGroupNode::get_class_type())) {
        EggGroupNode *group_child = DCAST(EggGroupNode, child);
        num_removed += strip_prims(group_child);
      }
      ++ci;
    }
  }

  return num_removed;
}


int main(int argc, char *argv[]) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  EggCrop prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
