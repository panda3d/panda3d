// Filename: eggCharacterData.cxx
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggCharacterData.h"
#include "eggJointData.h"
#include "eggGroup.h"
#include "eggTable.h"

#include <indirectCompareNames.h>
#include <indent.h>

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggCharacterData::
EggCharacterData() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
EggCharacterData::
~EggCharacterData() {
  Characters::iterator ci;

  for (ci = _characters.begin(); ci != _characters.end(); ++ci) {
    CharacterInfo &character_info = (*ci);
    delete character_info._root_joint;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::add_egg
//       Access: Public
//  Description: Adds a new egg file to the list of models and
//               animation files for this particular character.
//               Returns true if the file is successfully added, or
//               false if there is some problem (for instance, it does
//               not contain a character model or animation table).
//
//               If the joint hierarchy does not match the existing
//               joint hierarchy, a best match is attempted.
////////////////////////////////////////////////////////////////////
bool EggCharacterData::
add_egg(EggData *egg) {
  _top_egg_nodes.clear();

  if (!scan_hierarchy(egg)) {
    return false;
  }

  int egg_index = _eggs.size();
  _eggs.push_back(EggInfo());
  EggInfo &egg_info = _eggs.back();
  egg_info._egg = egg;

  // Now, for each model, add an entry in the egg_info and match the
  // joint hierarchy to the known joints.
  TopEggNodesByName::iterator tni;
  for (tni = _top_egg_nodes.begin(); tni != _top_egg_nodes.end(); ++tni) {
    string character_name = (*tni).first;
    TopEggNodes &top_nodes = (*tni).second;
    
    TopEggNodes::iterator ti;
    for (ti = top_nodes.begin(); ti != top_nodes.end(); ++ti) {
      EggNode *model = (*ti).first;
      EggNodeList &egg_nodes = (*ti).second;
      
      int model_index = egg_info._models.size();
      egg_info._models.push_back(model);
      match_egg_nodes(get_root_joint(character_name), 
		      egg_nodes, egg_index, model_index);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::get_root_joint
//       Access: Protected
//  Description: Returns the root of the joint hierarchy for the named
//               character.  This is actually a fictitious node that
//               does not correspond to any particular nodes in the
//               character hierarchy; it exists to hold all of the top
//               joints (if any) read from the character hierarchy.
////////////////////////////////////////////////////////////////////
EggJointData *EggCharacterData::
get_root_joint(const string &character_name) {
  // Does the named character exist yet?

  Characters::iterator ci;
  for (ci = _characters.begin(); ci != _characters.end(); ++ci) {
    CharacterInfo &character_info = (*ci);
    if (character_info._name == character_name) {
      return character_info._root_joint;
    }
  }

  // Define a new character.
  _characters.push_back(CharacterInfo());
  CharacterInfo &character_info = _characters.back();
  character_info._name = character_name;
  character_info._root_joint = make_joint_data();
  return character_info._root_joint;
}


////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::make_joint_data
//       Access: Protected, Virtual
//  Description: Allocates and returns a new EggJointData structure.
//               This is primarily intended as a hook so derived
//               classes can customize the type of EggJointData nodes
//               used to represent the joint hierarchy.
////////////////////////////////////////////////////////////////////
EggJointData *EggCharacterData::
make_joint_data() {
  return new EggJointData;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::scan_hierarchy
//       Access: Private
//  Description: Walks the given egg data's hierarchy, looking for
//               either the start of an animation channel or the start
//               of a character model.  Returns true if either (or
//               both) is found, false if the model appears to have
//               nothing to do with characters.
//
//               Fills up the _top_egg_nodes according to the nodes
//               found.
////////////////////////////////////////////////////////////////////
bool EggCharacterData::
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

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::scan_for_top_joints
//       Access: Private
//  Description: Once a character model has been found, continue
//               scanning the egg hierarchy to look for the topmost
//               <Joint> nodes encountered.
////////////////////////////////////////////////////////////////////
void EggCharacterData::
scan_for_top_joints(EggNode *egg_node, EggNode *model_root, 
		    const string &character_name) {
  if (egg_node->is_of_type(EggGroup::get_class_type())) {
    EggGroup *group = DCAST(EggGroup, egg_node);

    if (group->has_lod()) {
      // This flag has an LOD specification. 
      model_root = group;
    }
    if (group->get_group_type() == EggGroup::GT_joint) {
      // A <Joint> node begins a model hierarchy.
      _top_egg_nodes[character_name][model_root].push_back(group);
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

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::scan_for_top_tables
//       Access: Private
//  Description: Once an animation has been found, continue scanning
//               the egg hierarchy to look for the topmost <Table>
//               nodes encountered.
////////////////////////////////////////////////////////////////////
void EggCharacterData::
scan_for_top_tables(EggTable *bundle, EggNode *model_root,
		    const string &character_name) {
  // We really only need to check the immediate children of the bundle
  // for a table node called "<skeleton>".
  EggGroupNode::iterator gi;
  for (gi = bundle->begin(); gi != bundle->end(); ++gi) {
    EggNode *child = (*gi);
    if (child->is_of_type(EggTable::get_class_type())) {
      EggTable *table = DCAST(EggTable, child);
      if (table->get_name() == "<skeleton>") {
	// Here it is!  Now the immediate children of this node are
	// the top tables.

	EggGroupNode::iterator cgi;
	for (cgi = table->begin(); cgi != table->end(); ++cgi) {
	  EggNode *grandchild = (*cgi);
	  if (grandchild->is_of_type(EggTable::get_class_type())) {
	    _top_egg_nodes[character_name][model_root].push_back(grandchild);
	  }
	}
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::match_egg_nodes
//       Access: Private
//  Description: Attempts to match up the indicated list of egg_nodes
//               with the children of the given joint_data, by name if
//               possible.
//
//               Also recurses on each matched joint to build up the
//               entire joint hierarchy.
////////////////////////////////////////////////////////////////////
void EggCharacterData::
match_egg_nodes(EggJointData *joint_data, EggNodeList &egg_nodes,
		int egg_index, int model_index) {
  // Sort the list of egg_nodes in order by name.  This will make the
  // matching up by names easier and more reliable.
  sort(egg_nodes.begin(), egg_nodes.end(), IndirectCompareNames<Namable>());
  
  if (joint_data->_children.empty()) {
    // If the EggJointData has no children yet, we must be the first.
    // Gleefully define all the joints.
    EggNodeList::iterator ei;
    for (ei = egg_nodes.begin(); ei != egg_nodes.end(); ++ei) {
      EggNode *egg_node = (*ei);
      EggJointData *data = make_joint_data();
      joint_data->_children.push_back(data);
      found_egg_node_match(data, egg_node, egg_index, model_index);
    }

  } else {
    // The EggJointData already has children; therefore, we have to
    // match our joints up with the already-existing ones.

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
	found_egg_node_match(data, egg_node, egg_index, model_index);
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
      // If we have some extra egg_nodes, we have to find a place to
      // match them.  (If we only had extra data, we don't care.)
      
      // First, check to see if any of the names match any past-used
      // name.

      EggNodeList more_egg_nodes;

      for (ei = extra_egg_nodes.begin(); ei != extra_egg_nodes.end(); ++ei) {
	EggNode *egg_node = (*ei);
	bool matched = false;
	for (di = extra_data.begin(); 
	     di != extra_data.end() && !matched; 
	     ++di) {
	  EggJointData *data = (*di);
	  if (data->matches_name(egg_node->get_name())) {
	    matched = true;
	    found_egg_node_match(data, egg_node, egg_index, model_index);
	    extra_data.erase(di);
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
      // Ok, we've still got to find a home for these remaining
      // egg_nodes.
      if (extra_egg_nodes.size() == extra_data.size()) {
	// Match 'em up one-for-one.
	size_t i;
	for (i = 0; i < extra_egg_nodes.size(); i++) {
	  EggNode *egg_node = extra_egg_nodes[i];
	  EggJointData *data = extra_data[i];
	  found_egg_node_match(data, egg_node, egg_index, model_index);
	}

      } else {
	// Just tack 'em on the end.
	EggNodeList::iterator ei;
	for (ei = extra_egg_nodes.begin(); ei != extra_egg_nodes.end(); ++ei) {
	  EggNode *egg_node = (*ei);
	  EggJointData *data = make_joint_data();
	  joint_data->_children.push_back(data);
	  found_egg_node_match(data, egg_node, egg_index, model_index);
	}
      }
    }
  }

  // Now sort the generated joint data hierarchy by name, just to be
  // sure.
  sort(joint_data->_children.begin(), joint_data->_children.end(), 
       IndirectCompareNames<Namable>());
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::found_egg_node_match
//       Access: Private
//  Description: Marks a one-to-one association between the indicated
//               EggJointData and the indicated EggNode, and then
//               recurses below.
////////////////////////////////////////////////////////////////////
void EggCharacterData::
found_egg_node_match(EggJointData *data, EggNode *egg_node,
		     int egg_index, int model_index) {
  data->add_egg_node(egg_index, model_index, egg_node);

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group_node = DCAST(EggGroupNode, egg_node);
    
    // Now consider all the children of egg_node that are themselves
    // joints or tables.
    EggNodeList egg_nodes;

    // Two approaches: either we are scanning a model with joints, or
    // an animation bundle with tables.

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
      match_egg_nodes(data, egg_nodes, egg_index, model_index);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void EggCharacterData::
write(ostream &out, int indent_level) const {
  Characters::const_iterator ci;

  for (ci = _characters.begin(); ci != _characters.end(); ++ci) {
    const CharacterInfo &character_info = (*ci);
    indent(out, indent_level)
      << "Character " << character_info._name << ":\n";
    character_info._root_joint->write(out, indent_level + 2);
  }
}
