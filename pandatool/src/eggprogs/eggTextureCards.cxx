// Filename: eggTextureCards.cxx
// Created by:  drose (21Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggTextureCards.h"

#include <eggGroup.h>
#include <eggVertexPool.h>
#include <eggVertex.h>
#include <eggTexture.h>
#include <eggPolygon.h>

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggTextureCards::
EggTextureCards() : EggWriter(true, true) {
  set_program_description
    ("egg-texture-cards generates an egg file consisting of several "
     "square polygons, one for each texture name that appears on the "
     "command line.\n\n"

     "This is a handy thing to have for importing texture images through "
     "egg-palettize, even when those textures do not appear on any real "
     "geometry; it can also be used for creating a lot of simple polygons "
     "for rendering click buttons and similar interfaces.");

  clear_runlines();
  add_runline("[opts] texture [texture ...] output.egg");
  add_runline("[opts] -o output.egg texture [texture ...]");
  add_runline("[opts] texture [texture ...] >output.egg");

  add_option
    ("g", "left,right,bottom,top", 0, 
     "Specifies the geometry of each polygon.  The default is a unit polygon "
     "centered on the origin: -0.5,0.5,-0.5,0.5.  Polygons are always created "
     "on the X-Y plane.",
     &EggTextureCards::dispatch_double_quad, NULL, &_polygon_geometry[0]);

  add_option
    ("c", "r,g,b[,a]", 0, 
     "Specifies the color of each polygon.  The default is white: 1,1,1,1.",
     &EggTextureCards::dispatch_color, NULL, &_polygon_color[0]);

  add_option
    ("wm", "[repeat | clamp]", 0,
     "Indicates the wrap mode of the texture: either \"repeat\" or \"clamp\" "
     "(or \"r\" or \"c\").  The default is to leave this unspecified.",
     &EggTextureCards::dispatch_wrap_mode, NULL, &_wrap_mode);

  _polygon_geometry.set(-0.5, 0.5, -0.5, 0.5);
  _polygon_color.set(1.0, 1.0, 1.0, 1.0);
  _wrap_mode = EggTexture::WM_unspecified;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggTextureCards::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 0)) {
    return false;
  }

  if (args.empty()) {
    nout << "No texture names specified on the command line.\n";
    return false;
  }

  copy(args.begin(), args.end(),
       back_inserter(_texture_names));

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::dispatch_wrap_mode
//       Access: Protected, Static
//  Description: Standard dispatch function for an option that takes
//               one parameter, which is to be interpreted as a
//               WrapMode string.  The data pointer is to a WrapMode
//               enum variable.
////////////////////////////////////////////////////////////////////
bool EggTextureCards::
dispatch_wrap_mode(const string &opt, const string &arg, void *var) {
  EggTexture::WrapMode *wmp = (EggTexture::WrapMode *)var;

  *wmp = EggTexture::string_wrap_mode(arg);
  if (*wmp == EggTexture::WM_unspecified) {
    // An unknown string.  Let's check for our special cases.
    if (arg == "r") {
      *wmp = EggTexture::WM_repeat;
    } else if (arg == "c") {
      *wmp = EggTexture::WM_clamp;
    } else {
      nout << "Invalid wrap mode parameter for -" << opt << ": " 
           << arg << "\n";
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggTextureCards::
run() {
  // First, create an enclosing group and a vertex pool with four
  // vertices.  We can use the same four vertices on all polygons.

  EggGroup *group = new EggGroup();
  _data.add_child(group);

  // Make the group a sequence, as a convenience.  If we view the
  // egg file directly we can see all the tiles one at a time.
  group->set_switch_flag(true);
  group->set_switch_fps(2.0);

  EggVertexPool *vpool = new EggVertexPool("vpool");
  group->add_child(vpool);

  //
  //   1     4
  //
  //
  //   2     3
  //

  EggVertex *v1 = vpool->make_new_vertex
    (LPoint3d(_polygon_geometry[0], _polygon_geometry[3], 0.0));
  EggVertex *v2 = vpool->make_new_vertex
    (LPoint3d(_polygon_geometry[0], _polygon_geometry[2], 0.0));
  EggVertex *v3 = vpool->make_new_vertex
    (LPoint3d(_polygon_geometry[1], _polygon_geometry[2], 0.0));
  EggVertex *v4 = vpool->make_new_vertex
    (LPoint3d(_polygon_geometry[1], _polygon_geometry[3], 0.0));

  v1->set_uv(TexCoordd(0.0, 1.0));
  v2->set_uv(TexCoordd(0.0, 0.0));
  v3->set_uv(TexCoordd(1.0, 0.0));
  v4->set_uv(TexCoordd(1.0, 1.0));

  // Now, create a texture reference and a polygon for each texture.
  // We don't really care whether the texture files actually exist at
  // this point yet.

  vector_string::const_iterator ti;
  for (ti = _texture_names.begin(); ti != _texture_names.end(); ++ti) {
    Filename filename = (*ti);
    string name = filename.get_basename_wo_extension();

    EggTexture *tref = new EggTexture(name, filename);
    tref->set_wrap_mode(_wrap_mode);
    group->add_child(tref);

    // Each polygon gets placed in its own sub-group.  This will make
    // pulling them out by name at runtime possible.
    EggGroup *sub_group = new EggGroup(name);
    group->add_child(sub_group);
    EggPolygon *poly = new EggPolygon();
    sub_group->add_child(poly);
    poly->set_texture(tref);
    poly->set_color(_polygon_color);
    
    poly->add_vertex(v1);
    poly->add_vertex(v2);
    poly->add_vertex(v3);
    poly->add_vertex(v4);
  }

  // Done!
  write_egg_file();
}


int main(int argc, char *argv[]) {
  EggTextureCards prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
