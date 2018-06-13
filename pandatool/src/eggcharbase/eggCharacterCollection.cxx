/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCharacterCollection.cxx
 * @author drose
 * @date 2001-02-26
 */

#include "eggCharacterCollection.h"
#include "eggCharacterData.h"
#include "eggJointData.h"
#include "eggSliderData.h"

#include "dcast.h"
#include "eggGroup.h"
#include "eggTable.h"
#include "eggPrimitive.h"
#include "eggVertex.h"
#include "eggVertexUV.h"
#include "eggMorphList.h"
#include "eggSAnimData.h"
#include "indirectCompareNames.h"
#include "indent.h"

#include <algorithm>

using std::string;


/**
 *
 */
EggCharacterCollection::
EggCharacterCollection() {
  _next_model_index = 0;
}

/**
 *
 */
EggCharacterCollection::
~EggCharacterCollection() {
  Characters::iterator ci;

  for (ci = _characters.begin(); ci != _characters.end(); ++ci) {
    delete (*ci);
  }
}

/**
 * Adds a new egg file to the list of models and animation files for this
 * particular character.
 *
 * Returns the new egg_index if the file is successfully added, or -1 if there
 * is some problem (for instance, it does not contain a character model or
 * animation table).
 *
 * If the joint hierarchy does not match the existing joint hierarchy, a best
 * match is attempted.
 */
int EggCharacterCollection::
add_egg(EggData *egg) {
  _top_egg_nodes.clear();

  if (!scan_hierarchy(egg)) {
    return -1;
  }

  int egg_index = _eggs.size();
  _eggs.push_back(EggInfo());
  EggInfo &egg_info = _eggs.back();
  egg_info._egg = egg;
  egg_info._first_model_index = 0;

  // Now, for each model, add an entry in the egg_info and match the joint
  // hierarchy to the known joints.
  TopEggNodesByName::iterator tni;
  for (tni = _top_egg_nodes.begin(); tni != _top_egg_nodes.end(); ++tni) {
    string character_name = (*tni).first;
    TopEggNodes &top_nodes = (*tni).second;
    EggCharacterData *char_data = make_character(character_name);
    EggJointData *root_joint = char_data->get_root_joint();

    TopEggNodes::iterator ti;
    for (ti = top_nodes.begin(); ti != top_nodes.end(); ++ti) {
      EggNode *model_root = (*ti).first;
      ModelDescription &desc = (*ti).second;

      int model_index = _next_model_index++;
      if (egg_info._models.empty()) {
        egg_info._first_model_index = model_index;
      }
      egg_info._models.push_back(model_root);

      char_data->add_model(model_index, model_root, egg);
      nassertr(model_index == (int)_characters_by_model_index.size(), -1);
      _characters_by_model_index.push_back(char_data);
      root_joint->add_back_pointer(model_index, desc._root_node);

      match_egg_nodes(char_data, root_joint, desc._top_nodes,
                      egg_index, model_index);

      scan_for_morphs(model_root, model_index, char_data);
      scan_for_sliders(model_root, model_index, char_data);
    }
  }

  return egg_index;
}

/**
 * Returns the Character with the indicated name, if it exists in the
 * collection, or NULL if it does not.
 */
EggCharacterData *EggCharacterCollection::
get_character_by_name(const string &character_name) const {
  Characters::const_iterator ci;
  for (ci = _characters.begin(); ci != _characters.end(); ++ci) {
    EggCharacterData *char_data = (*ci);
    if (char_data->get_name() == character_name) {
      return char_data;
    }
  }

  return nullptr;
}


/**
 * Allocates and returns a new EggCharacterData structure.  This is primarily
 * intended as a hook so derived classes can customize the type of
 * EggCharacterData nodes used to represent the characters in this collection.
 */
EggCharacterData *EggCharacterCollection::
make_character_data() {
  return new EggCharacterData(this);
}

/**
 * Allocates and returns a new EggJointData structure for the given character.
 * This is primarily intended as a hook so derived classes can customize the
 * type of EggJointData nodes used to represent the joint hierarchy.
 */
EggJointData *EggCharacterCollection::
make_joint_data(EggCharacterData *char_data) {
  return new EggJointData(this, char_data);
}

/**
 * Allocates and returns a new EggSliderData structure for the given
 * character.  This is primarily intended as a hook so derived classes can
 * customize the type of EggSliderData nodes used to represent the slider
 * list.
 */
EggSliderData *EggCharacterCollection::
make_slider_data(EggCharacterData *char_data) {
  return new EggSliderData(this, char_data);
}

/**
 * Allocates and returns a new EggCharacterData object representing the named
 * character, if there is not already a character by that name.
 */
EggCharacterData *EggCharacterCollection::
make_character(const string &character_name) {
  // Does the named character exist yet?

  Characters::iterator ci;
  for (ci = _characters.begin(); ci != _characters.end(); ++ci) {
    EggCharacterData *char_data = (*ci);
    if (char_data->get_name() == character_name) {
      return char_data;
    }
  }

  // Define a new character.
  EggCharacterData *char_data = make_character_data();
  char_data->set_name(character_name);
  _characters.push_back(char_data);
  return char_data;
}

/**
 * Walks the given egg data's hierarchy, looking for either the start of an
 * animation channel or the start of a character model.  Returns true if
 * either (or both) is found, false if the model appears to have nothing to do
 * with characters.
 *
 * Fills up the _top_egg_nodes according to the nodes found.
 */
bool EggCharacterCollection::
scan_hierarchy(EggNode *egg_node) {
  if (egg_node->is_of_type(EggGroup::get_class_type())) {
    EggGroup *group = DCAST(EggGroup, egg_node);
    if (group->get_dart_type() != EggGroup::DT_none) {
      // A group with a <Dart> flag begins a character model.
      scan_for_top_joints(group, group, group->get_name());
      return true;
    }

  } else if (egg_node->is_of_type(EggTable::get_class_type())) {
    EggTable *table = DCAST(EggTable, egg_node);
    if (table->get_table_type() == EggTable::TT_bundle) {
      // A <Bundle> begins an animation table.
      scan_for_top_tables(table, table, table->get_name());
      return true;
    }
  }

  bool character_found = false;
  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, egg_node);
    EggGroupNode::iterator gi;
    for (gi = group->begin(); gi != group->end(); ++gi) {
      if (scan_hierarchy(*gi)) {
        character_found = true;
      }
    }
  }

  return character_found;
}

/**
 * Once a character model has been found, continue scanning the egg hierarchy
 * to look for the topmost <Joint> nodes encountered.
 */
void EggCharacterCollection::
scan_for_top_joints(EggNode *egg_node, EggNode *model_root,
                    const string &character_name) {
  if (egg_node->is_of_type(EggGroup::get_class_type())) {
    EggGroup *group = DCAST(EggGroup, egg_node);

    if (group->has_lod()) {
      // This group has an LOD specification; that indicates multiple skeleton
      // hierarchies for this character, one for each LOD. We call each of
      // these a separate model.
      model_root = group;
    }
    if (group->get_group_type() == EggGroup::GT_joint) {
      // A <Joint> node begins a model hierarchy.
      ModelDescription &desc = _top_egg_nodes[character_name][model_root];
      desc._root_node = model_root;
      desc._top_nodes.push_back(group);
      return;
    }
  }

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, egg_node);
    EggGroupNode::iterator gi;
    for (gi = group->begin(); gi != group->end(); ++gi) {
      scan_for_top_joints(*gi, model_root, character_name);
    }
  }
}

/**
 * Once an animation has been found, continue scanning the egg hierarchy to
 * look for the topmost <Table> nodes encountered.
 */
void EggCharacterCollection::
scan_for_top_tables(EggTable *bundle, EggNode *model_root,
                    const string &character_name) {
  // We really only need to check the immediate children of the bundle for a
  // table node called "<skeleton>".
  EggGroupNode::iterator gi;
  for (gi = bundle->begin(); gi != bundle->end(); ++gi) {
    EggNode *child = (*gi);
    if (child->is_of_type(EggTable::get_class_type())) {
      EggTable *table = DCAST(EggTable, child);
      if (table->get_name() == "<skeleton>") {
        // Here it is!  Now the immediate children of this node are the top
        // tables.
        ModelDescription &desc = _top_egg_nodes[character_name][model_root];
        desc._root_node = table;

        EggGroupNode::iterator cgi;
        for (cgi = table->begin(); cgi != table->end(); ++cgi) {
          EggNode *grandchild = (*cgi);
          if (grandchild->is_of_type(EggTable::get_class_type())) {
            desc._top_nodes.push_back(grandchild);
          }
        }
      }
    }
  }
}

/**
 * Go back through a model's hierarchy and look for morph targets on the
 * vertices and primitives.
 */
void EggCharacterCollection::
scan_for_morphs(EggNode *egg_node, int model_index,
                EggCharacterData *char_data) {
  if (egg_node->is_of_type(EggPrimitive::get_class_type())) {
    EggPrimitive *prim = DCAST(EggPrimitive, egg_node);
    // Check for morphs on the primitive.
    add_morph_back_pointers(prim, prim, model_index, char_data);

    // Also check for morphs on each of the prim's vertices.
    EggPrimitive::const_iterator vi;
    for (vi = prim->begin(); vi != prim->end(); ++vi) {
      EggVertex *vertex = (*vi);

      add_morph_back_pointers(vertex, vertex, model_index, char_data);
      add_morph_back_pointers_vertex(vertex, vertex, model_index, char_data);

      EggMorphVertexList::const_iterator mvi;
      for (mvi = vertex->_dxyzs.begin();
           mvi != vertex->_dxyzs.end();
           ++mvi) {
        const EggMorphVertex &morph = (*mvi);
        char_data->make_slider(morph.get_name())->add_back_pointer(model_index, vertex);
      }
    }
  }

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, egg_node);
    EggGroupNode::iterator gi;
    for (gi = group->begin(); gi != group->end(); ++gi) {
      scan_for_morphs(*gi, model_index, char_data);
    }
  }
}

/**
 * Go back to the animation tables and look for morph slider animation
 * channels.
 */
void EggCharacterCollection::
scan_for_sliders(EggNode *egg_node, int model_index,
                 EggCharacterData *char_data) {
  if (egg_node->is_of_type(EggTable::get_class_type())) {
    EggTable *bundle = DCAST(EggTable, egg_node);

    // We really only need to check the immediate children of the bundle for a
    // table node called "morph".  This is a sibling of "<skeleton>", which we
    // found a minute ago, but we weren't ready to scan for the morph sliders
    // at the time, so we have to look again now.

    EggGroupNode::iterator gi;
    for (gi = bundle->begin(); gi != bundle->end(); ++gi) {
      EggNode *child = (*gi);
      if (child->is_of_type(EggTable::get_class_type())) {
        EggTable *table = DCAST(EggTable, child);
        if (table->get_name() == "morph") {
          // Here it is!  Now the immediate children of this node are all the
          // slider channels.

          EggGroupNode::iterator cgi;
          for (cgi = table->begin(); cgi != table->end(); ++cgi) {
            EggNode *grandchild = (*cgi);
            if (grandchild->is_of_type(EggSAnimData::get_class_type())) {
              char_data->make_slider(grandchild->get_name())->add_back_pointer(model_index, grandchild);
            }
          }
        }
      }
    }
  }
}

/**
 * Adds the back pointers for the kinds of morphs we might find in an
 * EggAttributes object.
 */
void EggCharacterCollection::
add_morph_back_pointers(EggAttributes *attrib, EggObject *egg_object,
                        int model_index, EggCharacterData *char_data) {
  EggMorphNormalList::const_iterator mni;
  for (mni = attrib->_dnormals.begin();
       mni != attrib->_dnormals.end();
       ++mni) {
    const EggMorphNormal &morph = (*mni);
    char_data->make_slider(morph.get_name())->add_back_pointer(model_index, egg_object);
  }

  EggMorphColorList::const_iterator mci;
  for (mci = attrib->_drgbas.begin();
       mci != attrib->_drgbas.end();
       ++mci) {
    const EggMorphColor &morph = (*mci);
    char_data->make_slider(morph.get_name())->add_back_pointer(model_index, egg_object);
  }
}

/**
 * Adds the back pointers for the kinds of morphs we might find in an
 * EggVertex object.
 */
void EggCharacterCollection::
add_morph_back_pointers_vertex(EggVertex *vertex, EggObject *egg_object,
                               int model_index, EggCharacterData *char_data) {
  EggVertex::const_uv_iterator ui;
  for (ui = vertex->uv_begin(); ui != vertex->uv_end(); ++ui) {
    EggVertexUV *vert_uv = (*ui);
    EggMorphTexCoordList::const_iterator mti;
    for (mti = vert_uv->_duvs.begin();
         mti != vert_uv->_duvs.end();
         ++mti) {
      const EggMorphTexCoord &morph = (*mti);
      char_data->make_slider(morph.get_name())->add_back_pointer(model_index, egg_object);
    }
  }
}

/**
 * Attempts to match up the indicated list of egg_nodes with the children of
 * the given joint_data, by name if possible.
 *
 * Also recurses on each matched joint to build up the entire joint hierarchy.
 */
void EggCharacterCollection::
match_egg_nodes(EggCharacterData *char_data, EggJointData *joint_data,
                EggNodeList &egg_nodes, int egg_index, int model_index) {
  // Sort the list of egg_nodes in order by name.  This will make the matching
  // up by names easier and more reliable.
  sort(egg_nodes.begin(), egg_nodes.end(), IndirectCompareNames<Namable>());

  if (joint_data->_children.empty()) {
    // If the EggJointData has no children yet, we must be the first.
    // Gleefully define all the joints.
    EggNodeList::iterator ei;
    for (ei = egg_nodes.begin(); ei != egg_nodes.end(); ++ei) {
      EggNode *egg_node = (*ei);
      EggJointData *data = make_joint_data(char_data);
      joint_data->_children.push_back(data);
      char_data->_joints.push_back(data);
      char_data->_components.push_back(data);
      data->_parent = joint_data;
      data->_new_parent = joint_data;
      found_egg_match(char_data, data, egg_node, egg_index, model_index);
    }

  } else {
    // The EggJointData already has children; therefore, we have to match our
    // joints up with the already-existing ones.

    EggNodeList extra_egg_nodes;
    EggJointData::Children extra_data;

    EggNodeList::iterator ei;
    EggJointData::Children::iterator di;

    ei = egg_nodes.begin();
    di = joint_data->_children.begin();

    while (ei != egg_nodes.end() && di != joint_data->_children.end()) {
      EggNode *egg_node = (*ei);
      EggJointData *data = (*di);

      if (egg_node->get_name() < data->get_name()) {
        // Here's a joint in the egg file, unmatched in the data.
        extra_egg_nodes.push_back(egg_node);
        ++ei;

      } else if (data->get_name() < egg_node->get_name()) {
        // Here's a joint in the data, umatched by the egg file.
        extra_data.push_back(data);
        ++di;

      } else {
        // Hey, these two match!  Hooray!
        found_egg_match(char_data, data, egg_node, egg_index, model_index);
        ++ei;
        ++di;
      }
    }

    while (ei != egg_nodes.end()) {
      EggNode *egg_node = (*ei);

      // Here's a joint in the egg file, unmatched in the data.
      extra_egg_nodes.push_back(egg_node);
      ++ei;
    }

    while (di != joint_data->_children.end()) {
      EggJointData *data = (*di);

      // Here's a joint in the data, umatched by the egg file.
      extra_data.push_back(data);
      ++di;
    }

    if (!extra_egg_nodes.empty()) {
      // If we have some extra egg_nodes, we have to find a place to match
      // them.  (If we only had extra data, we don't care.)

      // First, check to see if any of the names match any past-used name.
      EggNodeList more_egg_nodes;

      for (ei = extra_egg_nodes.begin(); ei != extra_egg_nodes.end(); ++ei) {
        EggNode *egg_node = (*ei);
        bool matched = false;
        for (di = extra_data.begin(); di != extra_data.end(); ++di) {
          EggJointData *data = (*di);
          if (data->matches_name(egg_node->get_name())) {
            found_egg_match(char_data, data, egg_node, egg_index, model_index);
            extra_data.erase(di);
            matched = true;
            break;
          }
        }

        if (!matched) {
          // This joint name was never seen before.
          more_egg_nodes.push_back(egg_node);
        }
      }
      extra_egg_nodes.swap(more_egg_nodes);
    }

    if (!extra_egg_nodes.empty()) {
      // Ok, we've still got to find a home for these remaining egg_nodes.
      if (extra_egg_nodes.size() == extra_data.size()) {
        // Match 'em up one-for-one.
        size_t i;
        for (i = 0; i < extra_egg_nodes.size(); i++) {
          EggNode *egg_node = extra_egg_nodes[i];
          EggJointData *data = extra_data[i];
          found_egg_match(char_data, data, egg_node, egg_index, model_index);
        }

      } else {
        // Just tack 'em on the end.
        EggNodeList::iterator ei;
        for (ei = extra_egg_nodes.begin(); ei != extra_egg_nodes.end(); ++ei) {
          EggNode *egg_node = (*ei);
          EggJointData *data = make_joint_data(char_data);
          joint_data->_children.push_back(data);
          char_data->_joints.push_back(data);
          char_data->_components.push_back(data);
          data->_parent = joint_data;
          data->_new_parent = joint_data;
          found_egg_match(char_data, data, egg_node, egg_index, model_index);
        }
      }
    }
  }

  // Now sort the generated joint data hierarchy by name, just to be sure.
  sort(joint_data->_children.begin(), joint_data->_children.end(),
       IndirectCompareNames<Namable>());
}

/**
 * Marks a one-to-one association between the indicated EggJointData and the
 * indicated EggNode, and then recurses below.
 */
void EggCharacterCollection::
found_egg_match(EggCharacterData *char_data, EggJointData *joint_data,
                EggNode *egg_node, int egg_index, int model_index) {
  if (egg_node->has_name()) {
    joint_data->add_name(egg_node->get_name(), char_data->_component_names);
  }
  egg_node->set_name(joint_data->get_name());
  joint_data->add_back_pointer(model_index, egg_node);

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group_node = DCAST(EggGroupNode, egg_node);

    // Now consider all the children of egg_node that are themselves joints or
    // tables.
    EggNodeList egg_nodes;

    // Two approaches: either we are scanning a model with joints, or an
    // animation bundle with tables.

    if (egg_node->is_of_type(EggGroup::get_class_type())) {
      // A model with joints.
      EggGroupNode::iterator gi;
      for (gi = group_node->begin(); gi != group_node->end(); ++gi) {
        EggNode *child = (*gi);
        if (child->is_of_type(EggGroup::get_class_type())) {
          EggGroup *group = DCAST(EggGroup, child);
          if (group->get_group_type() == EggGroup::GT_joint) {
            egg_nodes.push_back(group);
          }
        }
      }

    } else {
      // An animation bundle with tables.
      EggGroupNode::iterator gi;
      for (gi = group_node->begin(); gi != group_node->end(); ++gi) {
        EggNode *child = (*gi);
        if (child->is_of_type(EggTable::get_class_type())) {
          EggTable *table = DCAST(EggTable, child);
          if (!(table->get_name() == "xform")) {
            egg_nodes.push_back(table);
          }
        }
      }
    }

    if (!egg_nodes.empty()) {
      match_egg_nodes(char_data, joint_data, egg_nodes,
                      egg_index, model_index);
    }
  }
}

/**
 * Renames the ith character to the indicated name.  This name must not
 * already be used by another character in the collection.
 */
void EggCharacterCollection::
rename_char(int i, const string &name) {
  nassertv(i >= 0 && i < (int)_characters.size());

  EggCharacterData *char_data = _characters[i];
  if (char_data->get_name() != name) {
    nassertv(get_character_by_name(name) == nullptr);
    char_data->rename_char(name);
  }
}

/**
 *
 */
void EggCharacterCollection::
write(std::ostream &out, int indent_level) const {
  Characters::const_iterator ci;

  for (ci = _characters.begin(); ci != _characters.end(); ++ci) {
    EggCharacterData *char_data = (*ci);
    char_data->write(out, indent_level);
  }
}

/**
 * Can be called after the collection has been completely filled up with egg
 * files to output any messages from warning conditions that have been
 * detected, such as inconsistent animation tables.
 *
 * In addition to reporting this errors, calling this function will also
 * ensure that they are all repaired.  Pass force_initial_rest_frame as true
 * to also force rest frames from different models to be the same if they are
 * initially different.
 */
void EggCharacterCollection::
check_errors(std::ostream &out, bool force_initial_rest_frame) {
  Characters::const_iterator ci;
  for (ci = _characters.begin(); ci != _characters.end(); ++ci) {
    EggCharacterData *char_data = (*ci);
    int num_joints = char_data->get_num_joints();
    for (int j = 0; j < num_joints; j++) {
      EggJointData *joint_data = char_data->get_joint(j);
      if (joint_data->rest_frames_differ()) {
        if (force_initial_rest_frame) {
          joint_data->force_initial_rest_frame();
          out << "Forced rest frames the same for " << joint_data->get_name()
              << ".\n";
        } else {
          out << "Warning: rest frames for " << joint_data->get_name()
              << " differ.\n";
        }
      }
    }

    int num_models = char_data->get_num_models();
    for (int mi = 0; mi < num_models; mi++) {
      int model_index = char_data->get_model_index(mi);
      if (!char_data->check_num_frames(model_index)) {
        out << "Warning: animation from "
            << char_data->get_egg_data(model_index)->get_egg_filename().get_basename()
            << " had an inconsistent number of frames.\n";
      }
    }
  }
}
