/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamToEgg.cxx
 * @author drose
 * @date 2001-06-25
 */

#include "bamToEgg.h"
#include "save_egg_file.h"
#include "string_utils.h"
#include "bamFile.h"
#include "bamCacheRecord.h"

/**
 *
 */
BamToEgg::
BamToEgg() :
  SomethingToEgg("bam", ".bam")
{
  set_program_brief("convert a native Panda .bam file to an .egg file");
  set_program_description
    ("This program converts native Panda bam files to egg.  The conversion "
     "is somewhat incomplete; running egg2bam followed by bam2egg should not "
     "be expected to yield the same egg file you started with.");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  By default, this is taken from the Config.prc file, which "
     "is currently " + format_string(get_default_coordinate_system()) + ".");

  _coordinate_system = get_default_coordinate_system();
}

/**
 *
 */
void BamToEgg::
run() {
  BamFile bam_file;

  if (!bam_file.open_read(_input_filename)) {
    nout << "Unable to read " << _input_filename << "\n";
    exit(1);
  }

  nout << _input_filename << " : Bam version "
       << bam_file.get_file_major_ver() << "."
       << bam_file.get_file_minor_ver() << "\n";

  typedef pvector<TypedWritable *> Objects;
  Objects objects;
  TypedWritable *object = bam_file.read_object();

  if (object != nullptr &&
      object->is_exact_type(BamCacheRecord::get_class_type())) {
    // Here's a special case: if the first object in the file is a
    // BamCacheRecord, it's really a cache data file and not a true bam file;
    // but skip over the cache data record and let the user treat it like an
    // ordinary bam file.
    object = bam_file.read_object();
  }

  while (object != nullptr || !bam_file.is_eof()) {
    if (object != nullptr) {
      ReferenceCount *ref_ptr = object->as_reference_count();
      if (ref_ptr != nullptr) {
        ref_ptr->ref();
      }
      objects.push_back(object);
    }
    object = bam_file.read_object();
  }
  bam_file.resolve();
  bam_file.close();

  _data->set_coordinate_system(_coordinate_system);

  if (objects.size() == 1 &&
      objects[0]->is_of_type(PandaNode::get_class_type())) {
    PandaNode *node = DCAST(PandaNode, objects[0]);
    save_egg_data(_data, node);

  } else {
    nout << "File does not contain a scene graph.\n";
    exit(1);
  }

  write_egg_file();
}

int main(int argc, char *argv[]) {
  BamToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
