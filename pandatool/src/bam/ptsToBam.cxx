/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ptsToBam.cxx
 * @author drose
 * @date 2000-06-28
 */

#include "ptsToBam.h"

#include "config_putil.h"
#include "geomPoints.h"
#include "bamFile.h"
#include "pandaNode.h"
#include "geomNode.h"
#include "dcast.h"
#include "string_utils.h"
#include "config_egg2pg.h"

using std::string;

/**
 *
 */
PtsToBam::
PtsToBam() : WithOutputFile(true, false, true)
{
  set_program_brief("convert point cloud data into a .bam file");
  set_program_description
    ("This program reads a point clound in a pts file and outputs a bam files, "
     "suitable for viewing in Panda.");

  clear_runlines();
  add_runline("[opts] input.pts output.bam");
  add_runline("[opts] -o output.bam input.pts");

  add_option
    ("o", "filename", 0,
     "Specify the filename to which the resulting .bam file will be written.  "
     "If this option is omitted, the last parameter name is taken to be the "
     "name of the output file.",
     &PtsToBam::dispatch_filename, &_got_output_filename, &_output_filename);

  add_option
    ("d", "divisor", 0,
     "Decimates the point cloud by the indicated divisor.  The number of points\n"
     "added is 1/divisor; numbers larger than 1.0 mean correspondingly fewer\n"
     "points.",
     &PtsToBam::dispatch_double, nullptr, &_decimate_divisor);

  _decimate_divisor = 1.0;
}

/**
 *
 */
void PtsToBam::
run() {
  pifstream pts;
  _pts_filename.set_text();
  if (!_pts_filename.open_read(pts)) {
    nout << "Cannot open " << _pts_filename << "\n";
    exit(1);
  }

  _gnode = new GeomNode(_pts_filename.get_basename());

  _num_points_expected = 0;
  _num_points_found = 0;
  _num_points_added = 0;
  _decimate_factor = 1.0 / std::max(1.0, _decimate_divisor);
  _line_number = 0;
  _point_number = 0;
  _decimated_point_number = 0.0;
  _num_vdatas = 0;
  string line;
  while (std::getline(pts, line)) {
    process_line(line);
  }
  close_vertex_data();

  nout << "\nFound " << _num_points_found << " points of " << _num_points_expected << " expected.\n";
  nout << "Generated " << _num_points_added << " points to bam file.\n";

  // This should be guaranteed because we pass false to the constructor,
  // above.
  nassertv(has_output_filename());

  Filename filename = get_output_filename();
  filename.make_dir();
  nout << "Writing " << filename << "\n";
  BamFile bam_file;
  if (!bam_file.open_write(filename)) {
    nout << "Error in writing.\n";
    exit(1);
  }

  if (!bam_file.write_object(_gnode.p())) {
    nout << "Error in writing.\n";
    exit(1);
  }
}

/**
 *
 */
bool PtsToBam::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the pts file to read on the command line.\n";
    return false;
  }

  if (args.size() > 1) {
    nout << "Specify only one pts on the command line.\n";
    return false;
  }

  _pts_filename = Filename::from_os_specific(args[0]);

  return true;
}

/**
 * Reads a single line from the pts file.
 */
void PtsToBam::
process_line(const string &line) {
  _line_number++;

  if (_line_number % 1000000 == 0) {
    std::cerr << "." << std::flush;
  }

  if (line.empty() || !isdigit(line[0])) {
    return;
  }

  if (_line_number == 1) {
    // The first line might be just the number of points.
    vector_string words;
    tokenize(trim(line), words, " \t", true);
    if (words.size() == 1) {
      string tail;
      _num_points_expected = string_to_int(words[0], tail);
      nout << "Expecting " << _num_points_expected << " points, will generate "
           << (int)(_num_points_expected * _decimate_factor) << "\n";
      return;
    }
  }

  // Here we might have a point.
  _num_points_found++;
  _decimated_point_number += _decimate_factor;
  int point_number = int(_decimated_point_number);
  if (point_number > _point_number) {
    _point_number = point_number;

    vector_string words;
    tokenize(trim(line), words, " \t", true);
    if (words.size() >= 3) {
      add_point(words);
    }
  }
}

/**
 * Adds a point from the pts file.
 */
void PtsToBam::
add_point(const vector_string &words) {
  if (_data == nullptr || _data->get_num_rows() >= egg_max_vertices) {
    open_vertex_data();
  }

  string tail;
  double x, y, z;
  x = string_to_double(words[0], tail);
  y = string_to_double(words[1], tail);
  z = string_to_double(words[2], tail);
  _vertex.add_data3d(x, y, z);
  _num_points_added++;
}

/**
 * Creates a new GeomVertexData.
 */
void PtsToBam::
open_vertex_data() {
  if (_data != nullptr) {
    close_vertex_data();
  }
  CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3();
  _data = new GeomVertexData("pts", format, GeomEnums::UH_static);
  _vertex = GeomVertexWriter(_data, "vertex");
}

/**
 * Closes a previous GeomVertexData and adds it to the scene graph.
 */
void PtsToBam::
close_vertex_data() {
  if (_data == nullptr) {
    return;
  }

  _num_vdatas++;
  nout << "\nGenerating " << _num_points_added << " points in " << _num_vdatas << " GeomVertexDatas\n";

  PT(Geom) geom = new Geom(_data);

  int num_vertices = _data->get_num_rows();
  int vertices_so_far = 0;
  while (num_vertices > 0) {
    int this_num_vertices = std::min(num_vertices, (int)egg_max_indices);
    PT(GeomPrimitive) points = new GeomPoints(GeomEnums::UH_static);
    points->add_consecutive_vertices(vertices_so_far, this_num_vertices);
    geom->add_primitive(points);
    vertices_so_far += this_num_vertices;
    num_vertices -= this_num_vertices;
  }

  _gnode->add_geom(geom);

  _data = nullptr;
}

int main(int argc, char *argv[]) {
  PtsToBam prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
