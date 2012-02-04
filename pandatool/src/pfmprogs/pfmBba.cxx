// Filename: pfmBba.cxx
// Created by:  drose (02Mar11)
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

#include "pfmBba.h"
#include "config_pfm.h"
#include "pfmFile.h"
#include "pystub.h"

////////////////////////////////////////////////////////////////////
//     Function: PfmBba::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PfmBba::
PfmBba() {
  set_program_description
    ("pfm-bba generates a .bba file from a .pfm file that lists the "
     "planar bounding volume of the pfm's internal data.");

  add_option
    ("z", "", 0,
     "Treats (0,0,0) in the pfm file as a special don't-touch value.",
     &PfmBba::dispatch_none, &_got_zero_special);

  add_option
    ("r", "index", 0,
     "Selects a reorder index.",
     &PfmBba::dispatch_int, NULL, &_reorder_index);

  add_option
    ("o", "filename", 50,
     "Specify the filename to which the resulting bba file will be written.",
     &PfmBba::dispatch_filename, &_got_output_filename, &_output_filename);

  _reorder_index = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: PfmBba::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PfmBba::
run() {
  Filenames::const_iterator fi;
  for (fi = _input_filenames.begin(); fi != _input_filenames.end(); ++fi) {
    PfmFile file;
    if (!file.read(*fi)) {
      nout << "Cannot read " << *fi << "\n";
      exit(1);
    }
    if (!process_pfm(*fi, file)) {
      exit(1);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmBba::process_pfm
//       Access: Public
//  Description: Handles a single pfm file.
////////////////////////////////////////////////////////////////////
bool PfmBba::
process_pfm(const Filename &input_filename, PfmFile &file) {
  file.set_zero_special(_got_zero_special);

  Filename bba_filename;
  if (_got_output_filename) {
    bba_filename = _output_filename;
  } else {
    bba_filename = input_filename;
    bba_filename.set_extension("bba");
  }

  if (!bba_filename.empty()) {
    bba_filename.set_text();
    PT(BoundingHexahedron) bounds = file.compute_planar_bounds(pfm_bba_dist[0], pfm_bba_dist[1]);
    nassertr(bounds != (BoundingHexahedron *)NULL, false);
    
    pofstream out;
    if (!bba_filename.open_write(out)) {
      pfm_cat.error()
        << "Unable to open " << bba_filename << "\n";
      return false;
    }
    
    // This is the order expected by our existing bba system.
    static const int num_reorder_points = 4;
    static const int reorder_points[num_reorder_points][8] = {
      { 0, 1, 2, 3, 4, 5, 6, 7 },  // unfiltered
      { 7, 5, 1, 3, 6, 4, 0, 2 },  // front, floor
      { 4, 6, 2, 0, 5, 7, 3, 1 },  // left
      { 7, 5, 1, 3, 2, 0, 4, 6 },  // right
    };
    int ri = max(_reorder_index, 0);
    ri = min(ri, num_reorder_points - 1);
    
    for (int i = 0; i < bounds->get_num_points(); ++i) {
      LPoint3 p = bounds->get_point(reorder_points[ri][i]);
      out << p[0] << "," << p[1] << "," << p[2] << "\n";
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmBba::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PfmBba::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the pfm file(s) to read on the command line.\n";
    return false;
  }

  if (args.size() > 1 && _got_output_filename) {
    nout << "Cannot use -o when multiple pfm files are specified.\n";
    return false;
  }

  Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    _input_filenames.push_back(Filename::from_os_specific(*ai));
  }

  return true;
}


int main(int argc, char *argv[]) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  PfmBba prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
