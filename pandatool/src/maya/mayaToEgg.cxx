// Filename: mayaToEgg.cxx
// Created by:  drose (15Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "mayaToEgg.h"
#include "global_parameters.h"

#include <maya/MGlobal.h>

////////////////////////////////////////////////////////////////////
//     Function: MayaToEgg::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaToEgg::
MayaToEgg() :
  SomethingToEgg("Maya", ".mb")
{
  add_normals_options();

  set_program_description
    ("This program converts Maya model files to egg.  Nothing fancy yet.");

  add_option
    ("p", "", 0,
     "Generate polygon output only.  Tesselate all NURBS surfaces to "
     "polygons via the built-in Maya tesselator.  The tesselation will "
     "be based on the tolerance factor given by -ptol.",
     &MayaToEgg::dispatch_none, &polygon_output);

  add_option
    ("ptol", "tolerance", 0,
     "Specify the fit tolerance for Maya polygon tesselation.  The smaller "
     "the number, the more polygons will be generated.  The default is "
     "0.01.",
     &MayaToEgg::dispatch_double, NULL, &polygon_tolerance);

  add_option
    ("notrans", "", 0,
     "Don't convert explicit DAG transformations given in the Maya file.  "
     "Instead, convert all vertices to world space and write the file as "
     "one big transform space.  Using this option doesn't change the "
     "position of objects in the scene, just the number of explicit "
     "transforms appearing in the resulting egg file.",
     &MayaToEgg::dispatch_none, &ignore_transforms);

  add_option
    ("v", "", 0, 
     "Increase verbosity.  More v's means more verbose.", 
     &MayaToEgg::dispatch_count, NULL, &verbose);
  verbose = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEgg::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MayaToEgg::
run() {
  nout << "Initializing Maya.\n";
  if (!_maya.init(_program_name)) {
    nout << "Unable to initialize Maya.\n";
    exit(1);
  }
  
  if (!_maya.read(_input_filename.c_str())) {
    nout << "Error reading " << _input_filename << ".\n";
    exit(1);
  }

  // First, we build the egg file in the same coordinate system as
  // Maya.
  if (MGlobal::isYAxisUp()) {
    _data.set_coordinate_system(CS_yup_right);
  } else {
    _data.set_coordinate_system(CS_zup_right);
  }

  _maya.make_egg(_data);

  // Then, if the user so requested, we convert the egg file to the
  // desired output coordinate system.
  if (_got_coordinate_system) {
    _data.set_coordinate_system(_coordinate_system);
  }

  write_egg_file();
}


int main(int argc, char *argv[]) {
  MayaToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
