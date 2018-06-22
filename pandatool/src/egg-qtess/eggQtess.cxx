/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggQtess.cxx
 * @author drose
 * @date 2003-10-13
 */

#include "eggQtess.h"
#include "qtessGlobals.h"
#include "dcast.h"

/**
 *
 */
EggQtess::
EggQtess() {
  add_normals_options();

  set_program_brief("tesselate NURBS surfaces in .egg files");
  set_program_description
    ("egg-qtess reads an egg file, tessellates all of its NURBS surfaces "
     "using a simple uniform tessellation, and outputs a polygonal "
     "egg file.\n\n"

     "Characters are supported, soft-skinned and otherwise; joint "
     "ownership is computed correctly for each new polygon vertex.  "
     "Primitives other than NURBS surfaces appearing in the egg file "
     "are unaffected.");

  add_option
    ("f", "filename", 0,
     "Read the indicated parameter file.  Type egg-qtess -H "
     "to print a description of the parameter file format.",
     &EggQtess::dispatch_filename, nullptr, &_qtess_filename);

  add_option
    ("up", "subdiv", 0,
     "Specify a uniform subdivision per patch (isoparam).  Each NURBS "
     "surface is made up of N x M patches, each of which is divided "
     "into subdiv x subdiv quads.  A fractional number is allowed.",
     &EggQtess::dispatch_double, nullptr, &_uniform_per_isoparam);

  add_option
    ("us", "subdiv", 0,
     "Specify a uniform subdivision per surface.  Each NURBS "
     "surface is subdivided into subdiv x subdiv quads, regardless "
     "of the number of isoparams it has.  A fractional number is "
     "meaningless.",
     &EggQtess::dispatch_int, nullptr, &_uniform_per_surface);

  add_option
    ("t", "tris", 0,
     "Specify an approximate number of triangles to produce.  This "
     "is the total number of triangles for the entire egg file, "
     "including those surfaces that have already been given an "
     "explicit tessellation by a parameter file.",
     &EggQtess::dispatch_int, nullptr, &_total_tris);

  add_option
    ("ap", "", 0,
     "Attempt to automatically place tessellation lines where they'll "
     "do the most good on each surface (once the number of polygons "
     "for the surface has already been determined).",
     &EggQtess::dispatch_none, &QtessGlobals::_auto_place);

  add_option
    ("ad", "", 0,
     "Attempt to automatically distribute polygons among the surfaces "
     "where they are most needed according to curvature and size, "
     "instead of according to the number of isoparams.  This only has "
     "meaning when used in conjunction with -t.",
     &EggQtess::dispatch_none, &QtessGlobals::_auto_distribute);

  add_option
    ("ar", "ratio", 0,
     "Specify the ratio of dominance of size to curvature for -ap and "
     "-ad.  A value of 0 forces placement by curvature only; a very "
     "large value (like 1000) forces placement by size only.  The "
     "default is 5.0.",
     &EggQtess::dispatch_double, nullptr, &QtessGlobals::_curvature_ratio);

  add_option
    ("e", "", 0,
     "Respect subdivision parameters given in the egg file.  If this "
     "is specified, the egg file may define the effective number of "
     "patches of each NURBS entry.  This can be used alone or in "
     "conjunction with -u or -t to fine-tune the uniform tessellation "
     "on a per-surface basis.  (This is ignored if -ad is in effect.)",
     &EggQtess::dispatch_none, &QtessGlobals::_respect_egg);

  add_option
    ("q", "", 0,
     "Instead of writing an egg file, generate a parameter file "
     "for output.",
     &EggQtess::dispatch_none, &_qtess_output);

  add_option
    ("H", "", 0,
     "Describe the format of the parameter file specified with -f.",
     &EggQtess::dispatch_none, &_describe_qtess);

  _uniform_per_isoparam = 0.0;
  _uniform_per_surface = 0;
  _total_tris = 0;
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool EggQtess::
handle_args(ProgramBase::Args &args) {
  if (_describe_qtess) {
    describe_qtess_format();
    exit(0);
  }

  return EggFilter::handle_args(args);
}

/**
 *
 */
void EggQtess::
run() {
  bool read_qtess = false;
  if (!_qtess_filename.empty()) {
    if (!_qtess_file.read(_qtess_filename)) {
      exit(1);
    }
    read_qtess = true;
  }

  find_surfaces(_data);

  QtessInputEntry &default_entry = _qtess_file.get_default_entry();
  if (!read_qtess || default_entry.get_num_surfaces() == 0) {
    nout << _surfaces.size() << " NURBS surfaces found.\n";

  } else {
    nout << _surfaces.size() << " NURBS surfaces found; "
         << default_entry.get_num_surfaces()
         << " unaccounted for by input file.\n";
  }

  int num_tris = _qtess_file.count_tris();

  if (_total_tris != 0) {
    // Whatever number of triangles we have unaccounted for, assign to the
    // default bucket.
    int extra_tris = std::max(0, _total_tris - num_tris);
    if (read_qtess && default_entry.get_num_surfaces() != 0) {
      std::cerr << extra_tris << " triangles unaccounted for.\n";
    }

    default_entry.set_num_tris(extra_tris);

  } else if (_uniform_per_isoparam!=0.0) {
    default_entry.set_per_isoparam(_uniform_per_isoparam);

  } else if (_uniform_per_surface!=0.0) {
    default_entry.set_uv(_uniform_per_surface, _uniform_per_surface);

  } else {
    default_entry.set_per_isoparam(1.0);
  }

  default_entry.count_tris();

  if (_qtess_output) {
    // Sort the names into alphabetical order for aesthetics.
    // sort(_surfaces.begin(), _surfaces.end(), compare_surfaces());

    int tris = 0;

    std::ostream &out = get_output();
    Surfaces::const_iterator si;
    for (si = _surfaces.begin(); si != _surfaces.end(); ++si) {
      tris += (*si)->write_qtess_parameter(out);
    }

    std::cerr << tris << " tris generated.\n";

  } else {

    int tris = 0;

    Surfaces::const_iterator si;
    for (si = _surfaces.begin(); si != _surfaces.end(); ++si) {
      tris += (*si)->tesselate();
    }

    std::cerr << tris << " tris generated.\n";

    // Clear out the surfaces list before removing the vertices, since each
    // surface is holding reference counts to the previously-used vertices.
    _surfaces.clear();

    _data->remove_unused_vertices(true);
    write_egg_file();
  }
}

/**
 *
 */
void EggQtess::
describe_qtess_format() {
  nout <<
    "An egg-qtess parameter file consists of lines of the form:\n\n"

    "name [name...] : parameters\n\n"

    "Where name is a string (possibly including wildcard characters "
    "such as * and ?) that matches one or more surface "
    "names, and parameters is a tesselation specification, described below. "
    "The colon must be followed by at least one space to differentiate it "
    "from a colon character in the name(s).  Multiple names "
    "may be combined on one line.\n\n\n"


    "The parameters may be any of the following.  Lowercase letters are "
    "literal.  NUM is any number.\n\n";

  show_text("  omit", 10,
            "Remove the surface from the output.\n\n");

  show_text("  NUM", 10,
            "Try to achieve the indicated number of triangles over all the "
            "surfaces matched by this line.\n\n");

  show_text("  NUM NUM [[!]u# [!]u# ...] [[!]v# [!]v# ...]", 10,
            "Tesselate to NUM x NUM quads.  If u# or v# appear, they indicate "
            "additional isoparams to insert (or remove if preceded by an "
            "exclamation point).  The range is [0, 1].\n\n");

  show_text("  iNUM", 10,
            "Subdivision amount per isoparam.  Equivalent to the command-line "
            "option -u NUM.\n\n");

  show_text("  NUM%", 10,
            "This is a special parameter.  This does not request any specific "
            "tesselation for the named surfaces, but instead gives a relative "
            "importance for them when they appear with other surfaces in a "
            "later entry (or are tesselated via -t on the command line).  In "
            "general, a surface with a weight of 25% will be given a quarter "
            "of the share of the polygons it otherwise would have received; "
            "a weight of 150% will give the surface 50% more than its fair "
            "share.\n\n");

  show_text("  matchvu", 10,
            "This is a special parameter that indicates that two or more "
            "surfaces share a common edge, and must be tesselated the "
            "same way "
            "along that edge.  Specifically, matchvu means that the V "
            "tesselation of the first named surface will be applied to the U "
            "tesselation of the second (and later) named surface(s).  Similar "
            "definitions exist for matchuv, matchuu, and matchvv.\n\n");

  show_text("  minu NUM", 10,
            "This is another special parameter that specifies a "
            "minimum tesselation for all these surfaces in "
            "the U direction.  This is "
            "the number of quads across the dimension the surface will be "
            "broken into.  The default is 1 for an open surface, and 3 for "
            "a closed surface.\n\n");

  show_text("  minv NUM", 10,
            "Similar to minv, in the V direction.\n\n");

  nout <<
    "In addition, the following optional parameters may appear.  If they appear, "
    "they override similar parameters given on the command line; if they do not "
    "appear, the defaults are taken from the command line:\n\n";

  show_text("  ap", 10,
            "Automatically place tesselation lines on each surface where they "
            "seem to be needed most.\n\n");

  show_text("  !ap", 10,
            "Do not move lines automatically; use a strict uniform "
            "tesselation.\n\n");

  show_text("  ad", 10,
            "Automatically distribute polygons to the surfaces that seem to "
            "need them the most.\n\n");

  show_text("  !ad", 10,
            "Do not automatically distribute polygons; distribute "
            "them according to the number of isoparams of each surface.\n\n");

  show_text("  arNUM", 10,
            "Specify the ratio of dominance of size to curvature.\n\n");

  nout <<
    "The hash symbol '#' begins a comment if it is preceded by whitespace or at the "
    "beginning of a line.  The backslash character at the end of a line can be used "
    "to indicate a continuation.\n\n";
}

/**
 * Recursively walks the egg graph, collecting all the NURBS surfaces found.
 */
void EggQtess::
find_surfaces(EggNode *egg_node) {
  if (egg_node->is_of_type(EggNurbsSurface::get_class_type())) {
    PT(QtessSurface) surface =
      new QtessSurface(DCAST(EggNurbsSurface, egg_node));
    if (surface->is_valid()) {
      _surfaces.push_back(surface);
      QtessInputEntry::Type match_type = _qtess_file.match(surface);
      nassertv(match_type != QtessInputEntry::T_undefined);
    }
  }

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *egg_group = DCAST(EggGroupNode, egg_node);
    EggGroupNode::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      find_surfaces(*ci);
    }
  }
}

int main(int argc, char *argv[]) {
  EggQtess prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
