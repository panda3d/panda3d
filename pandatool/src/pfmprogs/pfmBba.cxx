/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfmBba.cxx
 * @author drose
 * @date 2011-03-02
 */

#include "pfmBba.h"
#include "config_pfmprogs.h"
#include "pfmFile.h"

/**
 *
 */
PfmBba::
PfmBba() {
  set_program_brief("generate .bba files from .pfm files");
  set_program_description
    ("pfm-bba generates a .bba file from a .pfm file that lists the "
     "planar bounding volume of the pfm's internal data.");

  add_option
    ("z", "", 0,
     "Treats (0,0,0) in the pfm file as a special don't-touch value.",
     &PfmBba::dispatch_none, &_got_zero_special);

  add_option
    ("o", "filename", 50,
     "Specify the filename to which the resulting bba file will be written.",
     &PfmBba::dispatch_filename, &_got_output_filename, &_output_filename);
}


/**
 *
 */
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

/**
 * Handles a single pfm file.
 */
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
    PT(BoundingHexahedron) bounds = file.compute_planar_bounds(LPoint2f(0.5, 0.5), pfm_bba_dist[0], pfm_bba_dist[1], false);
    nassertr(bounds != nullptr, false);

    pofstream out;
    if (!bba_filename.open_write(out)) {
      std::cerr << "Unable to open " << bba_filename << "\n";
      return false;
    }

    LPoint3 points[8];
    for (int i = 0; i < 8; ++i) {
      points[i] = bounds->get_point(i);
    }

    // Experiment with expanding the back wall backwards.
    /*
    LPlane plane(points[0], points[1], points[2]);
    LVector3 normal = plane.get_normal();

    static const PN_stdfloat scale = 20.0f;
    normal *= scale;
    points[0] += normal;
    points[1] += normal;
    points[2] += normal;
    points[3] += normal;
    */

    for (int i = 0; i < 8; ++i) {
      const LPoint3 &p = points[i];
      out << p[0] << "," << p[1] << "," << p[2] << "\n";
    }
  }

  return true;
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
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
  PfmBba prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
