// Filename: eggToBam.cxx
// Created by:  drose (28Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "eggToBam.h"

#include <config_util.h>
#include <bamFile.h>
#include <load_egg_file.h>
#include <config_egg2sg.h>
#include <config_gobj.h>

////////////////////////////////////////////////////////////////////
//     Function: EggToBam::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggToBam::
EggToBam() :
  EggToSomething("Bam", ".bam", true, false)
{
  set_program_description
    ("This program reads Egg files and outputs Bam files, the binary format "
     "suitable for direct loading of animation and models into Panda.");

  add_option
    ("tp", "path", 0, 
     "Add the indicated colon-delimited paths to the texture-path.  This "
     "option may also be repeated to add multiple paths.",
     &EggToBam::dispatch_search_path, NULL, &get_texture_path());

  add_option
    ("kp", "", 0, 
     "Keep the texture paths exactly as they are specified in the egg file, "
     "as relative paths, rather than storing them as full paths or as "
     "whatever is specified by the bam-texture-mode Configrc variable.",
     &EggToBam::dispatch_none, &_keep_paths);

  redescribe_option
    ("cs",
     "Specify the coordinate system of the resulting " + _format_name +
     " file.  This may be "
     "one of 'y-up', 'z-up', 'y-up-left', or 'z-up-left'.  The default "
     "is z-up.");
}

////////////////////////////////////////////////////////////////////
//     Function: EggToBam::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggToBam::
run() {
  if (_keep_paths) {
    // If the user specified -kp, we need to set a couple of Configrc
    // variables directly to achieve this.
    egg_keep_texture_pathnames = true;
    bam_texture_mode = BTM_fullpath;
  }

  if (!_got_coordinate_system) {
    // If the user didn't specify otherwise, ensure the coordinate
    // system is Z-up.
    _data.set_coordinate_system(CS_zup_right);
  }

  PT_NamedNode root = load_egg_data(_data);
  if (root == (NamedNode *)NULL) {
    nout << "Unable to build scene graph from egg file.\n";
    exit(1);
  }

  // This should be guaranteed because we pass false to the
  // constructor, above.
  nassertv(has_output_filename());

  Filename filename = get_output_filename();
  filename.make_dir();
  nout << "Writing " << filename << "\n";
  BamFile bam_file;
  if (!bam_file.open_write(filename)) {
    nout << "Error in writing.\n";
    exit(1);
  }

  if (!bam_file.write_object(root)) {
    nout << "Error in writing.\n";
    exit(1);
  }
}


int main(int argc, char *argv[]) {
  EggToBam prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
