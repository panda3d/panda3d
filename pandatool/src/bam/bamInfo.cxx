/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamInfo.cxx
 * @author drose
 * @date 2000-07-02
 */

#include "bamInfo.h"

#include "bamFile.h"
#include "pandaNode.h"
#include "geomNode.h"
#include "texture.h"
#include "recorderHeader.h"
#include "recorderFrame.h"
#include "recorderTable.h"
#include "dcast.h"
#include "pvector.h"
#include "bamCacheRecord.h"
#include "bamCacheIndex.h"

/**
 *
 */
BamInfo::
BamInfo() {
  set_program_brief("describe the contents of .bam files");
  set_program_description
    ("This program scans one or more Bam files--Panda's Binary Animation "
     "and Models native binary format--and describes their contents.");

  clear_runlines();
  add_runline("[opts] input.bam [input.bam ... ]");

  add_option
    ("ls", "", 0,
     "List the scene graph hierarchy in the bam file.",
     &BamInfo::dispatch_none, &_ls);

  add_option
    ("t", "", 0,
     "List explicitly each transition in the hierarchy.",
     &BamInfo::dispatch_none, &_verbose_transitions);

  add_option
    ("g", "", 0,
     "Output verbose information about the each Geom in the Bam file.",
     &BamInfo::dispatch_none, &_verbose_geoms);

  _num_scene_graphs = 0;
}


/**
 *
 */
void BamInfo::
run() {
  bool okflag = true;

  Filenames::const_iterator fi;
  for (fi = _filenames.begin(); fi != _filenames.end(); ++fi) {
    if (!get_info(*fi)) {
      okflag = false;
    }
  }

  if (_num_scene_graphs > 0) {
    nout << "\nScene graph statistics:\n";
    _analyzer.write(nout, 2);
  }
  nout << "\n";

  if (!okflag) {
    // Exit with an error if any of the files was unreadable.
    exit(1);
  }
}


/**
 *
 */
bool BamInfo::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the Bam file(s) to read on the command line.\n";
    return false;
  }

  ProgramBase::Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    _filenames.push_back(*ai);
  }

  return true;
}


/**
 * Reads a single Bam file and displays its contents.  Returns true if
 * successful, false on error.
 */
bool BamInfo::
get_info(const Filename &filename) {
  BamFile bam_file;

  if (!bam_file.open_read(filename)) {
    nout << "Unable to read.\n";
    return false;
  }

  const char *endian = "little-endian";
  if (bam_file.get_file_endian() == BamEnums::BE_bigendian) {
    endian = "big-endian";
  }
  int float_width = 32;
  if (bam_file.get_file_stdfloat_double()) {
    float_width = 64;
  }

  nout << filename << " : Bam version " << bam_file.get_file_major_ver()
       << "." << bam_file.get_file_minor_ver()
       << ", " << endian << ", " << float_width << "-bit floats.\n";

  Objects objects;
  TypedWritable *object = bam_file.read_object();

  if (object != nullptr &&
      object->is_exact_type(BamCacheRecord::get_class_type())) {
    // Here's a special case: if the first object in the file is a
    // BamCacheRecord, it's a cache data file; in this case, we output the
    // cache record, and then pretend it doesn't exist.
    DCAST(BamCacheRecord, object)->write(nout, 2);
    nout << "\n";
    object = bam_file.read_object();
  }

  while (object != nullptr || !bam_file.is_eof()) {
    if (object != nullptr) {
      objects.push_back(object);
    }
    object = bam_file.read_object();
  }
  if (!bam_file.resolve()) {
    nout << "Unable to fully resolve file.\n";
    return false;
  }

  // We can't close the bam file until we have examined the objects, since
  // closing it will decrement reference counts.

  if (objects.size() == 1 &&
      objects[0]->is_of_type(PandaNode::get_class_type())) {
    describe_scene_graph(DCAST(PandaNode, objects[0]));

  } else if (objects.size() == 1 &&
      objects[0]->is_of_type(Texture::get_class_type())) {
    describe_texture(DCAST(Texture, objects[0]));

  } else if (objects.size() == 1 &&
      objects[0]->is_of_type(BamCacheIndex::get_class_type())) {
    describe_cache_index(DCAST(BamCacheIndex, objects[0]));

  } else if (!objects.empty() && objects[0]->is_of_type(RecorderHeader::get_class_type())) {
    describe_session(DCAST(RecorderHeader, objects[0]), objects);

  } else {
    nout << "file contains " << objects.size() << " objects:\n";
    for (int i = 0; i < (int)objects.size(); i++) {
      describe_general_object(objects[i]);
    }
  }

  return true;
}


/**
 * Called for Bam files that contain a single scene graph and no other
 * objects.  This should describe that scene graph in some meaningful way.
 */
void BamInfo::
describe_scene_graph(PandaNode *node) {
  // Parent the node to our own scene graph root, so we can (a) guarantee it
  // won't accidentally be deleted before we're done, (b) easily determine the
  // bounding volume of the scene, and (c) report statistics on all the bam
  // file's scene graphs together when we've finished.

  PT(PandaNode) root = new PandaNode("root");
  root->add_child(node);
  _num_scene_graphs++;

  int num_nodes = _analyzer.get_num_nodes();
  _analyzer.add_node(node);
  num_nodes = _analyzer.get_num_nodes() - num_nodes;

  nout << "  " << num_nodes << " nodes, bounding volume is "
       << *root->get_bounds() << "\n";

  if (_ls || _verbose_geoms || _verbose_transitions) {
    list_hierarchy(node, 0);
  }
}

/**
 * Called for Bam files that contain a Texture object.
 */
void BamInfo::
describe_texture(Texture *tex) {
  tex->write(nout, 2);
}

/**
 * Called for Bam files that contain a BamCacheIndex object.
 */
void BamInfo::
describe_cache_index(BamCacheIndex *index) {
  index->write(nout, 2);
}

/**
 * Called for Bam files that contain a recorded session table.
 */
void BamInfo::
describe_session(RecorderHeader *header, const BamInfo::Objects &objects) {
  char time_buffer[1024];
  strftime(time_buffer, 1024, "%c",
           localtime(&header->_start_time));

  pset<std::string> recorders;
  double last_timestamp = 0.0;

  for (size_t i = 1; i < objects.size(); i++) {
    if (objects[i]->is_of_type(RecorderFrame::get_class_type())) {
      RecorderFrame *frame = DCAST(RecorderFrame, objects[i]);
      if (frame->_table_changed) {
        RecorderTable::Recorders::const_iterator ri;
        for (ri = frame->_table->_recorders.begin();
             ri != frame->_table->_recorders.end();
             ++ri) {
          recorders.insert((*ri).first);
        }
      }
      last_timestamp = frame->_timestamp;
    }
  }

  nout << "Session, " << last_timestamp
       << " secs, " << objects.size() - 1 << " frames, "
       << time_buffer << ".\n"
       << "Recorders:";
  for (pset<std::string>::iterator ni = recorders.begin();
       ni != recorders.end();
       ++ni) {
    nout << " " << (*ni);
  }
  nout << "\n";
}

/**
 * Called for Bam files that contain multiple objects which may or may not be
 * scene graph nodes.  This should describe each object in some meaningful
 * way.
 */
void BamInfo::
describe_general_object(TypedWritable *object) {
  nassertv(object != nullptr);
  nout << "  " << object->get_type() << "\n";
}

/**
 * Outputs the hierarchy and all of the verbose GeomNode information.
 */
void BamInfo::
list_hierarchy(PandaNode *node, int indent_level) {
  indent(nout, indent_level) << *node;

  if (_verbose_transitions) {
    nout << "\n";
    if (!node->get_transform()->is_identity()) {
      node->get_transform()->write(nout, indent_level);
    }
    if (!node->get_state()->is_empty()) {
      node->get_state()->write(nout, indent_level);
    }
    if (!node->get_effects()->is_empty()) {
      node->get_effects()->write(nout, indent_level);
    }

  } else {
    if (!node->get_transform()->is_identity()) {
      nout << " " << *node->get_transform();
    }
    if (!node->get_state()->is_empty()) {
      nout << " " << *node->get_state();
    }
    if (!node->get_effects()->is_empty()) {
      nout << " " << *node->get_effects();
    }
    nout << "\n";
  }

  if (_verbose_geoms && node->is_geom_node()) {
    GeomNode *geom_node;
    DCAST_INTO_V(geom_node, node);
    geom_node->write_verbose(nout, indent_level);
  }

  int num_children = node->get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);
    list_hierarchy(child, indent_level + 2);
  }
}

int main(int argc, char *argv[]) {
  BamInfo prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
