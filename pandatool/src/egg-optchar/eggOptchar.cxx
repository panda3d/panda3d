// Filename: eggOptchar.cxx
// Created by:  drose (18Jul03)
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

#include "eggOptchar.h"
#include "eggOptcharUserData.h"
#include "vertexMembership.h"

#include "eggJointData.h"
#include "eggSliderData.h"
#include "eggCharacterCollection.h"
#include "eggCharacterData.h"
#include "eggBackPointer.h"
#include "eggGroupNode.h"
#include "eggPrimitive.h"
#include "eggVertexPool.h"
#include "string_utils.h"
#include "dcast.h"
#include "pset.h"
#include "compose_matrix.h"
#include "fftCompressor.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggOptchar::
EggOptchar() {
  add_path_replace_options();
  add_path_store_options();
  add_normals_options();
  add_transform_options();
  add_fixrest_option();

  set_program_description
    ("egg-optchar performs basic optimizations of a character model "
     "and its associated animations, primarily by analyzing the "
     "animation tables and removing unneeded joints and/or morphs.  "
     "It can also perform basic restructuring operations on the "
     "character hierarchy.");

  add_option
    ("ls", "", 0,
     "List the joint hierarchy instead of performing any operations.",
     &EggOptchar::dispatch_none, &_list_hierarchy);

  add_option
    ("lsv", "", 0,
     "List the joint hierarchy along with an indication of the properties "
     "each joint.",
     &EggOptchar::dispatch_none, &_list_hierarchy_v);

  add_option
    ("lsp", "", 0,
     "List the existing joint hierarchy as a series of -p joint,parent "
     "commands, suitable for pasting into an egg-optchar command line.",
     &EggOptchar::dispatch_none, &_list_hierarchy_p);

  add_option
    ("keep", "joint[,joint...]", 0,
     "Keep the named joints (or sliders) in the character, even if they do "
     "not appear to be needed by the animation.",
     &EggOptchar::dispatch_vector_string_comma, NULL, &_keep_components);

  add_option
    ("drop", "joint[,joint...]", 0,
     "Removes the named joints or sliders, even if they appear to be needed.",
     &EggOptchar::dispatch_vector_string_comma, NULL, &_drop_components);

  add_option
    ("expose", "joint[,joint...]", 0,
     "Expose the named joints by flagging them with a DCS attribute, so "
     "each one can be found in the scene graph when the character is loaded, "
     "and objects can be parented to it.  This implies -keep.",
     &EggOptchar::dispatch_vector_string_comma, NULL, &_expose_components);

  add_option
    ("flag", "node[,node...][=name]", 0,
     "Assign the indicated name to the geometry within the given nodes.  "
     "This will make the geometry visible as a node in the resulting "
     "character model when it is loaded in the scene graph (normally, "
     "the node hierarchy is suppressed when loading characters).  This "
     "is different from -expose in that it reveals geometry rather than "
     "joints; the revealed node can be hidden or its attributes changed "
     "at runtime, but it will be animated by its vertices, not the node, so "
     "objects parented to this node will not inherit its animation.",
     &EggOptchar::dispatch_flag_groups, NULL, &_flag_groups);

  add_option
    ("zero", "joint[,hprxyzijkabc]", 0,
     "Zeroes out the animation channels for the named joint.  If "
     "a subset of the component letters hprxyzijkabc is included, the "
     "operation is restricted to just those components; otherwise the "
     "entire transform is cleared.",
     &EggOptchar::dispatch_name_components, NULL, &_zero_channels);

  add_option
    ("keepall", "", 0,
     "Keep all joints and sliders in the character, except those named "
     "explicitly by -drop.",
     &EggOptchar::dispatch_none, &_keep_all);
  
  add_option
    ("p", "joint,parent", 0,
     "Moves the named joint under the named parent joint.  Use "
     "\"-p joint,\" to reparent a joint to the root.  The joint transform "
     "is recomputed appropriately under its new parent so that the animation "
     "is not affected (the effect is similar to NodePath::wrt_reparent_to).",
     &EggOptchar::dispatch_vector_string_pair, NULL, &_reparent_joints);
  
  add_option
    ("new", "joint,source", 0,
     "Creates a new joint under the named parent joint.  The new "
     "joint will inherit the same net transform as its parent.",
     &EggOptchar::dispatch_vector_string_pair, NULL, &_new_joints);

  if (FFTCompressor::is_compression_available()) {
    add_option
      ("optimal", "", 0,
       "Computes the optimal joint hierarchy for the character by analyzing "
       "all of the joint animation and reparenting joints to minimize "
       "transformations.  This can repair skeletons that have been flattened "
       "or whose hierarchy was otherwise damaged in conversion; it can also "
       "detect joints that are constrained to follow other joints and should "
       "therefore be parented to the master joints.  The result is a file "
       "from which more joints may be successfully removed, that generally "
       "compresses better and with fewer artifacts.  However, this is a "
       "fairly expensive operation.",
       &EggOptchar::dispatch_none, &_optimal_hierarchy);
  }

  add_option
    ("q", "quantum", 0,
     "Quantize joint membership values to the given unit.  This is "
     "the smallest significant change in joint membership.  The "
     "default is 0.01; specifying 0 means to preserve the original "
     "values.",
     &EggOptchar::dispatch_double, NULL, &_vref_quantum);

  _optimal_hierarchy = false;
  _vref_quantum = 0.01;
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggOptchar::
run() {
  // We have to apply the user-specified reparent requests first,
  // before we even analyze the joints.  This is because reparenting
  // the joints may change their properties.
  if (apply_user_reparents()) {
    nout << "Reparenting hierarchy.\n";
    // So we'll have to call do_reparent() twice.  It seems wasteful,
    // but it really is necessary, and it's not that bad.
    do_reparent();
  }

  if (!_zero_channels.empty()) {
    zero_channels();
  }

  int num_characters = _collection->get_num_characters();
  int ci;

  // Now we can analyze the joints for their properties.
  for (ci = 0; ci < num_characters; ci++) {
    EggCharacterData *char_data = _collection->get_character(ci);
    analyze_joints(char_data->get_root_joint());
    analyze_sliders(char_data);
  }

  if (_list_hierarchy || _list_hierarchy_v) {
    for (ci = 0; ci < num_characters; ci++) {
      EggCharacterData *char_data = _collection->get_character(ci);
      nout << "Character: " << char_data->get_name() << "\n";
      list_joints(char_data->get_root_joint(), 0, _list_hierarchy_v);
      list_scalars(char_data, _list_hierarchy_v);
      nout << char_data->get_num_joints() << " joints.\n";
    }

  } else if (_list_hierarchy_p) {
    for (ci = 0; ci < num_characters; ci++) {
      EggCharacterData *char_data = _collection->get_character(ci);
      nout << "Character: " << char_data->get_name() << "\n";
      int col = 0;
      list_joints_p(char_data->get_root_joint(), col);
      // A newline to cout is needed after the above call.
      cout << "\n";
      nout << char_data->get_num_joints() << " joints.\n";
    }

  } else {
    // The meat of the program: determine which joints are to be
    // removed, and then actually remove them.
    determine_removed_components();
    move_vertices();
    if (process_joints()) {
      do_reparent();
    }

    // Quantize the vertex memberships.  We call this even if
    // _vref_quantum is 0, because this also normalizes the vertex
    // memberships.
    quantize_vertices();

    // We currently do not implement optimizing morph sliders.  Need
    // to add this at some point; it's quite easy.  Identity and empty
    // morph sliders can simply be removed, while static sliders need
    // to be applied to the vertices and then removed.

    // Finally, flag all the groups as the user requested.
    if (!_flag_groups.empty()) {
      Eggs::iterator ei;
      for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
        do_flag_groups(*ei);
      }
    }

    write_eggs();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggOptchar::
handle_args(ProgramBase::Args &args) {
  if (_list_hierarchy || _list_hierarchy_v || _list_hierarchy_p) {
    _read_only = true;
  }

  return EggCharacterFilter::handle_args(args);
}

////////////////////////////////////////////////////////////////////
//     Function: ProgramBase::dispatch_vector_string_pair
//       Access: Protected, Static
//  Description: Standard dispatch function for an option that takes
//               a pair of string parameters.  The data pointer is to
//               StringPairs vector; the pair will be pushed onto the
//               end of the vector.
////////////////////////////////////////////////////////////////////
bool EggOptchar::
dispatch_vector_string_pair(const string &opt, const string &arg, void *var) {
  StringPairs *ip = (StringPairs *)var;

  vector_string words;
  tokenize(arg, words, ",");

  if (words.size() == 2) {
    StringPair sp;
    sp._a = words[0];
    sp._b = words[1];
    ip->push_back(sp);

  } else {
    nout << "-" << opt
         << " requires a pair of strings separated by a comma.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ProgramBase::dispatch_name_components
//       Access: Protected, Static
//  Description: Accepts a name optionally followed by a comma and some
//               of the nine standard component letters,
//
//               The data pointer is to StringPairs vector; the pair
//               will be pushed onto the end of the vector.
////////////////////////////////////////////////////////////////////
bool EggOptchar::
dispatch_name_components(const string &opt, const string &arg, void *var) {
  StringPairs *ip = (StringPairs *)var;

  vector_string words;
  tokenize(arg, words, ",");

  StringPair sp;
  if (words.size() == 1) {
    sp._a = words[0];

  } else if (words.size() == 2) {
    sp._a = words[0];
    sp._b = words[1];

  } else {
    nout << "-" << opt
         << " requires a pair of strings separated by a comma.\n";
    return false;
  }

  if (sp._b.empty()) {
    sp._b = matrix_component_letters;
  } else {
    for (string::const_iterator si = sp._b.begin(); si != sp._b.end(); ++si) {
      if (strchr(matrix_component_letters, *si) == NULL) {
        nout << "Not a standard matrix component: \"" << *si << "\"\n"
             << "-" << opt << " requires a joint name followed by a set "
             << "of component names.  The standard component names are \"" 
             << matrix_component_letters << "\".\n";
        return false;
      }
    }
  }

  ip->push_back(sp);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ProgramBase::dispatch_flag_groups
//       Access: Protected, Static
//  Description: Accepts a set of comma-delimited group names followed
//               by an optional name separated with an equal sign.
//
//               The data pointer is to a FlagGroups object.
////////////////////////////////////////////////////////////////////
bool EggOptchar::
dispatch_flag_groups(const string &opt, const string &arg, void *var) {
  FlagGroups *ip = (FlagGroups *)var;

  vector_string words;

  tokenize(arg, words, ",");

  if (words.empty()) {
    nout << "-" << opt
         << " requires a series of words separated by a comma.\n";
    return false;
  }

  FlagGroupsEntry entry;

  // Check for an equal sign in the last word.  This marks the name to
  // assign.
  string &last_word = words.back();
  size_t equals = last_word.rfind('=');
  if (equals != string::npos) {
    entry._name = last_word.substr(equals + 1);
    last_word = last_word.substr(0, equals);

  } else {
    // If there's no equal sign, the default is to name all groups
    // after the last word.
    entry._name = last_word;
  }

  // Convert the words to GlobPatterns.
  vector_string::const_iterator si;
  for (si = words.begin(); si != words.end(); ++si) {
    const string &word = (*si);
    entry._groups.push_back(GlobPattern(word));
  }

  ip->push_back(entry);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::determine_removed_components
//       Access: Private
//  Description: Flag all joints and sliders that should be removed
//               for optimization purposes.
////////////////////////////////////////////////////////////////////
void EggOptchar::
determine_removed_components() {
  typedef pset<string> Names;
  Names keep_names;
  Names drop_names;
  Names expose_names;
  Names names_used;

  vector_string::const_iterator si;
  for (si = _keep_components.begin(); si != _keep_components.end(); ++si) {
    keep_names.insert(*si);
  }
  for (si = _drop_components.begin(); si != _drop_components.end(); ++si) {
    drop_names.insert(*si);
  }
  for (si = _expose_components.begin(); si != _expose_components.end(); ++si) {
    keep_names.insert(*si);
    expose_names.insert(*si);
  }

  // We always keep the root joint, which has no name.
  keep_names.insert("");

  int num_characters = _collection->get_num_characters();
  for (int ci = 0; ci < num_characters; ci++) {
    EggCharacterData *char_data = _collection->get_character(ci);
    int num_components = char_data->get_num_components();
    cerr << char_data->get_name() << " has " << num_components << " components.\n";
    for (int i = 0; i < num_components; i++) {
      EggComponentData *comp_data = char_data->get_component(i);
      nassertv(comp_data != (EggComponentData *)NULL);

      EggOptcharUserData *user_data = 
        DCAST(EggOptcharUserData, comp_data->get_user_data());
      nassertv(user_data != (EggOptcharUserData *)NULL);

      const string &name = comp_data->get_name();
      if (drop_names.find(name) != drop_names.end()) {
        // Remove this component by user request.
        names_used.insert(name);
        user_data->_flags |= EggOptcharUserData::F_remove;

      } else if (_keep_all || keep_names.find(name) != keep_names.end()) {
        // Keep this component.
        names_used.insert(name);

        if (expose_names.find(name) != expose_names.end()) {
          // In fact, expose it.
          user_data->_flags |= EggOptcharUserData::F_expose;
        }

      } else {
        // Remove this component if it's unanimated or empty.
        if ((user_data->_flags & (EggOptcharUserData::F_static | EggOptcharUserData::F_empty)) != 0) {
          user_data->_flags |= EggOptcharUserData::F_remove;
        }
      }
    }
  }

  // Go back and tell the user about component names we didn't use,
  // just to be helpful.
  for (si = _keep_components.begin(); si != _keep_components.end(); ++si) {
    const string &name = (*si);
    if (names_used.find(name) == names_used.end()) {
      nout << "No such component: " << name << "\n";
    }
  }
  for (si = _drop_components.begin(); si != _drop_components.end(); ++si) {
    const string &name = (*si);
    if (names_used.find(name) == names_used.end()) {
      nout << "No such component: " << name << "\n";
    }
  }
  for (si = _expose_components.begin(); si != _expose_components.end(); ++si) {
    const string &name = (*si);
    if (names_used.find(name) == names_used.end()) {
      nout << "No such component: " << name << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::move_vertices
//       Access: Private
//  Description: Moves the vertices from joints that are about to be
//               removed into the first suitable parent.  This might
//               result in fewer joints being removed (because
//               the parent might suddenly no longer be empty).
////////////////////////////////////////////////////////////////////
void EggOptchar::
move_vertices() {
  int num_characters = _collection->get_num_characters();
  for (int ci = 0; ci < num_characters; ci++) {
    EggCharacterData *char_data = _collection->get_character(ci);
    int num_joints = char_data->get_num_joints();
    
    for (int i = 0; i < num_joints; i++) {
      EggJointData *joint_data = char_data->get_joint(i);
      EggOptcharUserData *user_data = 
        DCAST(EggOptcharUserData, joint_data->get_user_data());

      if ((user_data->_flags & EggOptcharUserData::F_empty) == 0 &&
          (user_data->_flags & EggOptcharUserData::F_remove) != 0) {
        // This joint has vertices, but is scheduled to be removed;
        // find a suitable home for its vertices.
        EggJointData *best_joint = find_best_vertex_joint(joint_data->get_parent());
        joint_data->move_vertices_to(best_joint);

        // Now we can't remove the joint.
        if (best_joint != (EggJointData *)NULL) {
          EggOptcharUserData *best_user_data = 
            DCAST(EggOptcharUserData, best_joint->get_user_data());
          best_user_data->_flags &= ~(EggOptcharUserData::F_empty | EggOptcharUserData::F_remove);
        }
      }
    }
  }
}
      

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::process_joints
//       Access: Private
//  Description: Effects the actual removal of joints flagged for
//               removal by reparenting the hierarchy appropriately.
//               Returns true if any joints are removed, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool EggOptchar::
process_joints() {
  cerr << "process_joints\n";
  bool removed_any = false;
  int num_characters = _collection->get_num_characters();
  for (int ci = 0; ci < num_characters; ci++) {
    EggCharacterData *char_data = _collection->get_character(ci);
    int num_joints = char_data->get_num_joints();
    
    int num_static = 0;
    int num_empty = 0;
    int num_identity = 0;
    int num_other = 0;
    int num_kept = 0;
    
    for (int i = 0; i < num_joints; i++) {
      EggJointData *joint_data = char_data->get_joint(i);
      EggOptcharUserData *user_data = 
        DCAST(EggOptcharUserData, joint_data->get_user_data());
      
      if ((user_data->_flags & EggOptcharUserData::F_remove) != 0) {
        // This joint will be removed, so reparent it to nothing.
        joint_data->reparent_to((EggJointData *)NULL);
        
        // Determine what kind of node it is we're removing, for the
        // user's information.
        if ((user_data->_flags & EggOptcharUserData::F_identity) != 0) {
          num_identity++;
        } else if ((user_data->_flags & EggOptcharUserData::F_static) != 0) {
          num_static++;
        } else if ((user_data->_flags & EggOptcharUserData::F_empty) != 0) {
          num_empty++;
        } else {
          num_other++;
        }
        removed_any = true;

      } else {
        // This joint will be preserved, but maybe its parent will
        // change.
        EggJointData *best_parent = find_best_parent(joint_data->get_parent());
        joint_data->reparent_to(best_parent);
        if ((user_data->_flags & EggOptcharUserData::F_expose) != 0) {
          joint_data->expose();
        }
        num_kept++;
      }
    }

    if (num_joints == num_kept) {
      nout << char_data->get_name() << ": keeping " << num_joints
           << " joints.\n";
    } else {
      nout << setw(5) << num_joints
           << " original joints in " << char_data->get_name()
           << "\n";
      if (num_identity != 0) {
        nout << setw(5) << num_identity << " identity joints\n";
      }
      if (num_static != 0) {
        nout << setw(5) << num_static << " unanimated joints\n";
      }
      if (num_empty != 0) {
        nout << setw(5) << num_empty << " empty joints\n";
      }
      if (num_other != 0) {
        nout << setw(5) << num_other << " other joints\n";
      }
      nout << " ----\n" 
           << setw(5) << num_kept << " joints remaining\n\n";
    }
  }

  return removed_any;
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::find_best_parent
//       Access: Private
//  Description: Searches for the first joint at this level or above
//               that is not scheduled to be removed.  This is the
//               joint that the first child of this joint should be
//               reparented to.
////////////////////////////////////////////////////////////////////
EggJointData *EggOptchar::
find_best_parent(EggJointData *joint_data) const {
  EggOptcharUserData *user_data = 
    DCAST(EggOptcharUserData, joint_data->get_user_data());

  if ((user_data->_flags & EggOptcharUserData::F_remove) != 0) {
    // Keep going.
    if (joint_data->get_parent() != (EggJointData *)NULL) {
      return find_best_parent(joint_data->get_parent());
    }
  }

  // This is the one!
  return joint_data;
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::find_best_vertex_joint
//       Access: Private
//  Description: Searches for the first joint at this level or above
//               that is not static.  This is the joint that the
//               vertices of this joint should be moved into.
////////////////////////////////////////////////////////////////////
EggJointData *EggOptchar::
find_best_vertex_joint(EggJointData *joint_data) const {
  if (joint_data == (EggJointData *)NULL) {
    return NULL;
  }

  EggOptcharUserData *user_data = 
    DCAST(EggOptcharUserData, joint_data->get_user_data());

  if ((user_data->_flags & EggOptcharUserData::F_static) != 0) {
    // Keep going.
    return find_best_vertex_joint(joint_data->get_parent());
  }

  // This is the one!
  return joint_data;
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::apply_user_reparents
//       Access: Private
//  Description: Reparents all the joints that the user suggested on
//               the command line.  Returns true if any operations
//               were performed, false otherwise.
////////////////////////////////////////////////////////////////////
bool EggOptchar::
apply_user_reparents() {
  bool did_anything = false;

  int num_characters = _collection->get_num_characters();

  // First, get the new joints.
  StringPairs::const_iterator spi;
  for (spi = _new_joints.begin(); spi != _new_joints.end(); ++spi) {
    const StringPair &p = (*spi);

    for (int ci = 0; ci < num_characters; ci++) {
      EggCharacterData *char_data = _collection->get_character(ci);
      EggJointData *node_a = char_data->find_joint(p._a);
      EggJointData *node_b = char_data->get_root_joint();
      if (!p._b.empty()) {
        node_b = char_data->find_joint(p._b);
      }

      if (node_b == (EggJointData *)NULL) {
        nout << "No joint named " << p._b << " in " << char_data->get_name()
             << ".\n";

      } else if (node_a != (EggJointData *)NULL) {
        nout << "Joint " << p._a << " already exists in " 
             << char_data->get_name() << ".\n";
        
      } else {
        nout << "Creating new joint " << p._a << " in "
             << char_data->get_name() << ".\n";
        node_a = char_data->make_new_joint(p._a, node_b);
        did_anything = true;
      }
    }
  }

  // Now get the user reparents.
  for (spi = _reparent_joints.begin(); spi != _reparent_joints.end(); ++spi) {
    const StringPair &p = (*spi);

    for (int ci = 0; ci < num_characters; ci++) {
      EggCharacterData *char_data = _collection->get_character(ci);
      EggJointData *node_a = char_data->find_joint(p._a);
      EggJointData *node_b = char_data->get_root_joint();
      if (!p._b.empty()) {
        node_b = char_data->find_joint(p._b);
      }

      if (node_b == (EggJointData *)NULL) {
        nout << "No joint named " << p._b << " in " << char_data->get_name()
             << ".\n";
      } else if (node_a == (EggJointData *)NULL) {
        nout << "No joint named " << p._a << " in " << char_data->get_name()
             << ".\n";
      } else {
        node_a->reparent_to(node_b);
        did_anything = true;
      }
    }
  }

  if (_optimal_hierarchy) {
    did_anything = true;
    for (int ci = 0; ci < num_characters; ci++) {
      EggCharacterData *char_data = _collection->get_character(ci);
      nout << "Computing optimal hierarchy for "
           << char_data->get_name() << ".\n";
      char_data->choose_optimal_hierarchy();
      nout << "Done computing optimal hierarchy for "
           << char_data->get_name() << ".\n";
    }
  }

  return did_anything;
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::zero_channels
//       Access: Private
//  Description: Zeroes out the channels specified by the user on the
//               command line.
//
//               Returns true if any operation was performed, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool EggOptchar::
zero_channels() {
  bool did_anything = false;
  int num_characters = _collection->get_num_characters();

  StringPairs::const_iterator spi;
  for (spi = _zero_channels.begin(); spi != _zero_channels.end(); ++spi) {
    const StringPair &p = (*spi);

    for (int ci = 0; ci < num_characters; ci++) {
      EggCharacterData *char_data = _collection->get_character(ci);
      EggJointData *joint_data = char_data->find_joint(p._a);

      if (joint_data == (EggJointData *)NULL) {
        nout << "No joint named " << p._a << " in " << char_data->get_name()
             << ".\n";
      } else {
        joint_data->zero_channels(p._b);
        did_anything = true;
      }
    }
  }

  return did_anything;
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::analyze_joints
//       Access: Private
//  Description: Recursively walks the joint hierarchy for a
//               particular character, indentifying properties of each
//               joint.
////////////////////////////////////////////////////////////////////
void EggOptchar::
analyze_joints(EggJointData *joint_data) {
  PT(EggOptcharUserData) user_data = new EggOptcharUserData;
  joint_data->set_user_data(user_data);

  // Analyze the table of matrices for this joint, checking to see if
  // they're all the same across all frames, or if any of them are
  // different; also look for empty joints (that control no vertices).
  int num_mats = 0;
  bool different_mat = false;
  bool has_vertices = false;

  int num_models = joint_data->get_num_models();
  int i;
  for (i = 0; i < num_models; i++) {
    if (joint_data->has_model(i)) {
      EggBackPointer *model = joint_data->get_model(i);
      if (model->has_vertices()) {
        has_vertices = true;
      }

      int num_frames = joint_data->get_num_frames(i);

      int f;
      for (f = 0; f < num_frames && !different_mat; f++) {
        LMatrix4d mat = joint_data->get_frame(i, f);
        num_mats++;
        if (num_mats == 1) {
          // This is the first matrix.
          user_data->_static_mat = mat;

        } else {
          // This is a second or later matrix.
          if (!mat.almost_equal(user_data->_static_mat, 0.0001)) {
            // It's different than the first one.
            different_mat = true;
          }
        }
      }
    }
  }

  if (!different_mat) {
    // All the mats are the same for this joint.
    user_data->_flags |= EggOptcharUserData::F_static;

    if (num_mats == 0 || 
        user_data->_static_mat.almost_equal(LMatrix4d::ident_mat(), 0.0001)) {
      // It's not only static, but it's the identity matrix.
      user_data->_flags |= EggOptcharUserData::F_identity;
    }
  }

  if (!has_vertices) {
    // There are no vertices in this joint.
    user_data->_flags |= EggOptcharUserData::F_empty;
  }

  int num_children = joint_data->get_num_children();
  for (i = 0; i < num_children; i++) {
    analyze_joints(joint_data->get_child(i));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::analyze_sliders
//       Access: Private
//  Description: Linearly walks the slider list for a particular
//               character, indentifying properties of each slider.
////////////////////////////////////////////////////////////////////
void EggOptchar::
analyze_sliders(EggCharacterData *char_data) {
  int num_sliders = char_data->get_num_sliders();
  for (int si = 0; si < num_sliders; si++) {
    EggSliderData *slider_data = char_data->get_slider(si);

    PT(EggOptcharUserData) user_data = new EggOptcharUserData;
    slider_data->set_user_data(user_data);

    // Analyze the table of values for this slider, checking to see if
    // they're all the same across all frames, or if any of them are
    // different; also look for empty sliders (that control no vertices).
    int num_values = 0;
    bool different_value = false;
    bool has_vertices = false;

    int num_models = slider_data->get_num_models();
    for (int i = 0; i < num_models; i++) {
      if (slider_data->has_model(i)) {
        EggBackPointer *model = slider_data->get_model(i);
        if (model->has_vertices()) {
          has_vertices = true;
        }
        
        int num_frames = slider_data->get_num_frames(i);
        
        int f;
        for (f = 0; f < num_frames && !different_value; f++) {
          double value = slider_data->get_frame(i, f);
          num_values++;
          if (num_values == 1) {
            // This is the first value.
            user_data->_static_value = value;
            
          } else {
            // This is a second or later value.
            if (!IS_THRESHOLD_EQUAL(value, user_data->_static_value, 0.0001)) {
              // It's different than the first one.
              different_value = true;
            }
          }
        }
      }
    }
    
    if (!different_value) {
      // All the values are the same for this slider.
      user_data->_flags |= EggOptcharUserData::F_static;
      
      if (num_values == 0 || IS_THRESHOLD_ZERO(user_data->_static_value, 0.0001)) {
        // It's not only static, but it's the identity value.
        user_data->_flags |= EggOptcharUserData::F_identity;
      }
    }
    
    if (!has_vertices) {
      // There are no vertices in this slider.
      user_data->_flags |= EggOptcharUserData::F_empty;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::list_joints
//       Access: Private
//  Description: Outputs a list of the joint hierarchy.
////////////////////////////////////////////////////////////////////
void EggOptchar::
list_joints(EggJointData *joint_data, int indent_level, bool verbose) {
  // Don't list the root joint, which is artificially created when the
  // character is loaded.  Instead, list each child as it is
  // encountered.

  int num_children = joint_data->get_num_children();
  for (int i = 0; i < num_children; i++) {
    EggJointData *child_data = joint_data->get_child(i);
    describe_component(child_data, indent_level, verbose);

    list_joints(child_data, indent_level + 2, verbose);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::list_joints_p
//       Access: Private
//  Description: Outputs a list of the joint hierarchy as a series of
//               -p joint,parent commands.
////////////////////////////////////////////////////////////////////
void EggOptchar::
list_joints_p(EggJointData *joint_data, int &col) {
  // As above, don't list the root joint.

  int num_children = joint_data->get_num_children();
  static const int max_col = 72;

  for (int i = 0; i < num_children; i++) {
    EggJointData *child_data = joint_data->get_child(i);
    // We send output to cout instead of nout to avoid the
    // word-wrapping, and also to allow the user to redirect this
    // easily to a file.

    string text = string(" -p ") + child_data->get_name() + 
      string(",") + joint_data->get_name();
    if (col == 0) {
      cout << "    " << text;
      col = 4 + text.length();
    } else {
      col += text.length();
      if (col >= max_col) {
        cout << " \\\n    " << text;
        col = 4 + text.length();
      } else {
        cout << text;
      }
    }

    list_joints_p(child_data, col);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::list_scalars
//       Access: Private
//  Description: Outputs a list of the scalars.
////////////////////////////////////////////////////////////////////
void EggOptchar::
list_scalars(EggCharacterData *char_data, bool verbose) {
  int num_sliders = char_data->get_num_sliders();
  for (int si = 0; si < num_sliders; si++) {
    EggSliderData *slider_data = char_data->get_slider(si);
    describe_component(slider_data, 0, verbose);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::describe_component
//       Access: Private
//  Description: Describes one particular slider or joint.
////////////////////////////////////////////////////////////////////
void EggOptchar::
describe_component(EggComponentData *comp_data, int indent_level,
                   bool verbose) {
  // We use cout instead of nout so the user can easily redirect this
  // to a file.
  indent(cout, indent_level)
    << comp_data->get_name();

  if (verbose) {
    EggOptcharUserData *user_data = 
      DCAST(EggOptcharUserData, comp_data->get_user_data());
    if (user_data->is_identity()) {
      cout << " (identity)";
    } else if (user_data->is_static()) {
      cout << " (static)";
    }
    if (user_data->is_empty()) {
      cout << " (empty)";
    }
  }
  cout << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::do_reparent
//       Access: Private
//  Description: Performs all of the queued up reparenting operations.
////////////////////////////////////////////////////////////////////
void EggOptchar::
do_reparent() {
  bool all_ok = true;

  int num_characters = _collection->get_num_characters();
  for (int ci = 0; ci < num_characters; ci++) {
    EggCharacterData *char_data = _collection->get_character(ci);
    if (!char_data->do_reparent()) {
      all_ok = false;
    }
  }

  if (!all_ok) {
    exit(1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::quantize_vertices
//       Access: Private
//  Description: Walks through all of the loaded egg files, looking
//               for vertices whose joint memberships are then
//               quantized according to _vref_quantum.
////////////////////////////////////////////////////////////////////
void EggOptchar::
quantize_vertices() {
  Eggs::iterator ei;
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    quantize_vertices(*ei);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::quantize_vertices
//       Access: Private
//  Description: Recursively walks through the indicated egg
//               hierarchy, looking for vertices whose joint
//               memberships are then quantized according to
//               _vref_quantum.
////////////////////////////////////////////////////////////////////
void EggOptchar::
quantize_vertices(EggNode *egg_node) {
  if (egg_node->is_of_type(EggVertexPool::get_class_type())) {
    EggVertexPool *vpool = DCAST(EggVertexPool, egg_node);
    EggVertexPool::iterator vi;
    for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
      quantize_vertex(*vi);
    }
    
  } else if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, egg_node);
    EggGroupNode::iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      quantize_vertices(*ci);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::quantize_vertex
//       Access: Private
//  Description: Quantizes the indicated vertex's joint membership.
////////////////////////////////////////////////////////////////////
void EggOptchar::
quantize_vertex(EggVertex *egg_vertex) {
  if (egg_vertex->gref_size() == 0) {
    // Never mind on this vertex.
    return;
  }

  // First, get a copy of the existing membership.
  VertexMemberships memberships;
  EggVertex::GroupRef::const_iterator gi;
  double net_membership = 0.0;
  for (gi = egg_vertex->gref_begin(); gi != egg_vertex->gref_end(); ++gi) {
    EggGroup *group = (*gi);
    double membership = group->get_vertex_membership(egg_vertex);
    memberships.push_back(VertexMembership(group, membership));
    net_membership += membership;
  }
  nassertv(net_membership != 0.0);

  // Now normalize all the memberships so the net membership is 1.0,
  // and then quantize the result (if the user so requested).
  double factor = 1.0 / net_membership;
  net_membership = 0.0;
  VertexMemberships::iterator mi;
  VertexMemberships::iterator largest = memberships.begin();

  for (mi = memberships.begin(); mi != memberships.end(); ++mi) {
    if ((*largest) < (*mi)) {
      // Remember the largest membership value, so we can readjust it
      // at the end.
      largest = mi;
    }

    double value = (*mi)._membership * factor;
    if (_vref_quantum != 0.0) {
      value = floor(value / _vref_quantum + 0.5) * _vref_quantum;
    }
    (*mi)._membership = value;

    net_membership += value;
  }

  // The the largest membership value gets corrected again by the
  // roundoff error.
  (*largest)._membership += 1.0 - net_membership;

  // Finally, walk back through and apply these computed values to the
  // vertex.
  for (mi = memberships.begin(); mi != memberships.end(); ++mi) {
    (*mi)._group->set_vertex_membership(egg_vertex, (*mi)._membership);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::do_flag_groups
//       Access: Private
//  Description: Recursively walks the indicated egg hierarchy,
//               looking for groups that match one of the group names
//               in _flag_groups, and renaming geometry appropriately.
////////////////////////////////////////////////////////////////////
void EggOptchar::
do_flag_groups(EggGroupNode *egg_group) {
  bool matched = false;
  string name;
  FlagGroups::const_iterator fi;
  for (fi = _flag_groups.begin(); 
       fi != _flag_groups.end() && !matched; 
       ++fi) {
    const FlagGroupsEntry &entry = (*fi);
    Globs::const_iterator si;
    for (si = entry._groups.begin(); 
         si != entry._groups.end() && !matched; 
         ++si) {
      if ((*si).matches(egg_group->get_name())) {
        matched = true;
        name = entry._name;
      }
    }
  }

  if (matched) {
    // Ok, this group matched one of the user's command-line renames.
    // Rename all the primitives in this group and below to the
    // indicated name; this will expose the primitives through the
    // character loader.
    rename_primitives(egg_group, name);
  }

  // Now recurse on children.
  EggGroupNode::iterator gi;
  for (gi = egg_group->begin(); gi != egg_group->end(); ++gi) {
    EggNode *child = (*gi);
    if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group = DCAST(EggGroupNode, child);
      do_flag_groups(group);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::rename_primitives
//       Access: Private
//  Description: Recursively walks the indicated egg hierarchy,
//               renaming geometry to the indicated name.
////////////////////////////////////////////////////////////////////
void EggOptchar::
rename_primitives(EggGroupNode *egg_group, const string &name) {
  EggGroupNode::iterator gi;
  for (gi = egg_group->begin(); gi != egg_group->end(); ++gi) {
    EggNode *child = (*gi);

    if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group = DCAST(EggGroupNode, child);
      rename_primitives(group, name);

    } else if (child->is_of_type(EggPrimitive::get_class_type())) {
      child->set_name(name);
    }
  }
}


int main(int argc, char *argv[]) {
  EggOptchar prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
