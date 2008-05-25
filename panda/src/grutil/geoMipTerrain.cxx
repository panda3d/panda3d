// Filename: geoMipTerrain.cxx
// Created by:  pro-rsoft (29jun07)
// Last updated by: pro-rsoft (08mar08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "geoMipTerrain.h"

#include "geomVertexFormat.h"
#include "geomVertexArrayFormat.h"
#include "internalName.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomTristrips.h"
#include "geomTriangles.h"
#include "geom.h"
#include "geomNode.h"
#include "config_grutil.h"

#include "sceneGraphReducer.h"

#include "collideMask.h"

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::generate_block
//       Access: Private
//  Description: Generates a chunk of terrain based on the level
//               specified. As arguments it takes the x and y coords
//               of the mipmap to be generated, and the level of
//               detail. T-Junctions for neighbor-mipmaps with
//               different levels are also taken into account.
////////////////////////////////////////////////////////////////////
NodePath GeoMipTerrain::
generate_block(unsigned short mx,
               unsigned short my,
               unsigned short level) {
  
  nassertr(mx < (_xsize - 1) / _block_size, NodePath::fail());
  nassertr(my < (_ysize - 1) / _block_size, NodePath::fail());

  unsigned short center = _block_size / 2;
  unsigned int vcounter = 0;
  
  // Create the format
  PT(GeomVertexArrayFormat) array = new GeomVertexArrayFormat();
  if (_has_color_map) {
    array->add_column(InternalName::make("color"), 4,
                                            Geom::NT_float32, Geom::C_color);
  }
  array->add_column(InternalName::make("vertex"), 3,
                                            Geom::NT_float32, Geom::C_point);
  array->add_column(InternalName::make("texcoord"), 2,
                                            Geom::NT_float32, Geom::C_texcoord);
  array->add_column(InternalName::make("normal"), 3,
                                            Geom::NT_float32, Geom::C_vector);
  PT(GeomVertexFormat) format = new GeomVertexFormat();
  format->add_array(array);

  // Create vertex data and writers
  PT(GeomVertexData) vdata = new GeomVertexData(_root.get_name(),
                   GeomVertexFormat::register_format(format), Geom::UH_static);
  GeomVertexWriter cwriter;
  if (_has_color_map) {
    cwriter=GeomVertexWriter(vdata, "color"  );
  }
  GeomVertexWriter vwriter (vdata, "vertex"  );
  GeomVertexWriter twriter (vdata, "texcoord");
  GeomVertexWriter nwriter (vdata, "normal"  );
  PT(GeomTriangles) prim = new GeomTriangles(Geom::UH_static);

  if (_bruteforce) {
    // LOD Level when rendering bruteforce is always 0 (no lod)
    level = 0;
  }

  // Do some calculations with the level
  level = min(short(max(_min_level, level)), short(log(float(_block_size))
                                                                  / log(2.0)));
  unsigned short reallevel = level;
  level = int(pow(2.0, int(level)));
  
  // Confusing note:
  // the variable level contains not the actual level as described
  // in the GeoMipMapping paper. That is stored in reallevel,
  // while the variable level contains 2^reallevel.

  // This is the number of vertices at the certain level.
  unsigned short lowblocksize = _block_size / level + 1;
  
  for (int x = 0; x <= _block_size; x++) {
    for (int y = 0; y <= _block_size; y++) {
      if ((x % level) == 0 && (y % level) == 0) {
        LVector3f normal (get_normal(mx, my, x, y));
        normal.set(normal.get_x() / _root.get_sx(),
                   normal.get_y() / _root.get_sy(),
                   normal.get_z() / _root.get_sz());
        normal.normalize();
        if (_has_color_map) {
          LVecBase4d color = _color_map.get_xel_a(int((mx * _block_size + x)
                                  / double(_xsize) * _color_map.get_x_size()),
                                                      int((my * _block_size + y)
                                  / double(_ysize) * _color_map.get_y_size()));
          cwriter.add_data4f(color.get_x(), color.get_y(),
                             color.get_z(), color.get_w());
        }
        vwriter.add_data3f(x - 0.5 * _block_size, y - 0.5 * _block_size,
                                                get_pixel_value(mx, my, x, y));
        twriter.add_data2f((mx * _block_size + x) / double(_xsize - 1),
                           (my * _block_size + y) / double(_ysize - 1));
        nwriter.add_data3f(normal);
        if (x > 0 && y > 0) {
          //left border
          if (!_bruteforce && x == level && mx > 0 && _levels[mx - 1][my] > reallevel) {
            if (y > level && y < _block_size) {
              prim->add_vertex(min(max(sfav(y / level, _levels[mx - 1][my], reallevel), 0), lowblocksize - 1));
              prim->add_vertex(vcounter - 1);
              prim->add_vertex(vcounter);
              prim->close_primitive();
            }
            if (f_part((y / level) / float(pow(2.0, int(_levels[mx - 1][my] - reallevel)))) == 0.5) {
              prim->add_vertex(min(max(sfav(y / level + 1, _levels[mx - 1][my], reallevel), 0), lowblocksize - 1));
              prim->add_vertex(min(max(sfav(y / level - 1, _levels[mx - 1][my], reallevel), 0), lowblocksize - 1));
              prim->add_vertex(vcounter);
              prim->close_primitive();
            }
          } else if (_bruteforce ||
                      (!(y == level && x > level && x < _block_size && my > 0
                                          && _levels[mx][my - 1] > reallevel) &&
                       !(x == _block_size && mx < (_xsize - 1) / (_block_size) - 1
                                          && _levels[mx + 1][my] > reallevel) &&
                       !(x == _block_size && y > level && y < _block_size && mx < (_xsize - 1) / (_block_size) - 1
                                          && _levels[mx + 1][my] > reallevel) &&
                       !(y == _block_size && x > level && x < _block_size && my < (_ysize - 1) / (_block_size) - 1
                                          && _levels[mx][my + 1] > reallevel))) {
            if ((x <= center && y <= center) || (x > center && y > center)) {
              if (x > center) {
                prim->add_vertex(vcounter - lowblocksize - 1);
                prim->add_vertex(vcounter - 1);
                prim->add_vertex(vcounter);
              } else {
                prim->add_vertex(vcounter);
                prim->add_vertex(vcounter - lowblocksize);
                prim->add_vertex(vcounter - lowblocksize - 1);
              }
            } else {
              if (x > center) {
                prim->add_vertex(vcounter);
                prim->add_vertex(vcounter - lowblocksize);
                prim->add_vertex(vcounter - 1);
              } else {
                prim->add_vertex(vcounter - 1);
                prim->add_vertex(vcounter - lowblocksize);
                prim->add_vertex(vcounter - lowblocksize - 1);
              }
            }
            prim->close_primitive();
          }
          //right border
          if (!_bruteforce && x == _block_size - level && mx < (_xsize - 1) / (_block_size) - 1 && _levels[mx + 1][my] > reallevel) {
            if (y > level && y < _block_size - level + 1) {
              prim->add_vertex(lowblocksize * (lowblocksize - 1) + min(max(sfav(y / level, _levels[mx + 1][my], reallevel), 0), lowblocksize - 1));
              prim->add_vertex(vcounter);
              prim->add_vertex(vcounter - 1);
              prim->close_primitive();
            }
            if (f_part((y / level)/float(pow(2.0, int(_levels[mx + 1][my]-reallevel)))) == 0.5) {
              prim->add_vertex(lowblocksize * (lowblocksize - 1) + min(max(sfav(y / level - 1, _levels[mx + 1][my], reallevel), 0), lowblocksize - 1));
              prim->add_vertex(lowblocksize * (lowblocksize - 1) + min(max(sfav(y / level + 1, _levels[mx + 1][my], reallevel), 0), lowblocksize - 1));
              prim->add_vertex(vcounter);
              prim->close_primitive();
            }
          }
          //bottom border
          if (!_bruteforce && y == level && my > 0 && _levels[mx][my - 1] > reallevel) {
            if (x > level && x < _block_size) {
              prim->add_vertex(vcounter);
              prim->add_vertex(vcounter - lowblocksize);
              prim->add_vertex(min(max(sfav(x / level, _levels[mx][my - 1], reallevel), 0), lowblocksize - 1) * lowblocksize);
              prim->close_primitive();
            }
            if (f_part((x / level)/float(pow(2.0, int(_levels[mx][my - 1]-reallevel)))) == 0.5) {
              prim->add_vertex(min(max(sfav(x / level - 1, _levels[mx][my - 1], reallevel), 0), lowblocksize - 1) * lowblocksize);
              prim->add_vertex(min(max(sfav(x / level + 1, _levels[mx][my - 1], reallevel), 0), lowblocksize - 1) * lowblocksize);
              prim->add_vertex(vcounter);
              prim->close_primitive();
            }
          } else if (_bruteforce || (!(x == level && y > level && y < _block_size && mx > 0 && _levels[mx - 1][my] > reallevel) && !(x == _block_size && y > level && y < _block_size && mx < (_xsize - 1) / (_block_size) - 1 && _levels[mx + 1][my] > reallevel) && !(x == _block_size && y > level && y < _block_size && mx < (_xsize - 1) / (_block_size) - 1 && _levels[mx + 1][my] > reallevel) && !(y == _block_size && my < (_ysize - 1) / (_block_size) - 1 && _levels[mx][my + 1] > reallevel))) {
            if ((x <= center && y <= center) || (x > center && y > center)) {
              if (y > center) {
                prim->add_vertex(vcounter);
                prim->add_vertex(vcounter - lowblocksize);//
                prim->add_vertex(vcounter - lowblocksize - 1);
              } else {
                prim->add_vertex(vcounter - lowblocksize - 1);
                prim->add_vertex(vcounter - 1);//
                prim->add_vertex(vcounter);
              }
            } else {
              if (y > center) {
                prim->add_vertex(vcounter);//
                prim->add_vertex(vcounter - lowblocksize);
                prim->add_vertex(vcounter - 1);
              } else {
                prim->add_vertex(vcounter - 1);
                prim->add_vertex(vcounter - lowblocksize);
                prim->add_vertex(vcounter - lowblocksize - 1);//
              }
            }
            prim->close_primitive();
          }
          //top border
          if (!_bruteforce && y == _block_size - level && my < (_xsize - 1) / (_block_size) - 1 && _levels[mx][my + 1] > reallevel) {
            if (x > level && x < _block_size - level + 1) {
              prim->add_vertex(min(max(sfav(x / level, _levels[mx][my + 1], reallevel), 0), lowblocksize - 1) * lowblocksize + lowblocksize - 1);
              prim->add_vertex(vcounter - lowblocksize);
              prim->add_vertex(vcounter);
              prim->close_primitive();
            }
            if (f_part((x / level)/float(pow(2.0, int(_levels[mx][my + 1]-reallevel)))) == 0.5) {
              prim->add_vertex(min(max(sfav(x / level + 1, _levels[mx][my + 1], reallevel), 0), lowblocksize - 1) * lowblocksize + lowblocksize - 1);
              prim->add_vertex(min(max(sfav(x / level - 1, _levels[mx][my + 1], reallevel), 0), lowblocksize - 1) * lowblocksize + lowblocksize - 1);
              prim->add_vertex(vcounter);
              prim->close_primitive();
            }
          }
        }
        vcounter++;
      }
    }
  }

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(prim);

  PT(GeomNode) node = new GeomNode("gmm" + int_to_str(mx) + "x" + int_to_str(my));
  node->add_geom(geom);
  _old_levels.at(mx).at(my) = reallevel;
  return NodePath(node);
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_elevation
//       Access: Published
//  Description: Fetches the elevation at (x, y), where the input
//               coordinate is specified in pixels. This ignores
//               the current LOD level and instead provides an
//               accurate number. Linear blending is used for 
//               non-integral coordinates.
//               Terrain scale is NOT taken into account! To get
//               accurate normals, please multiply this with the
//               terrain Z scale!
//
//               trueElev = terr.get_elevation(x,y) * terr.get_sz();
////////////////////////////////////////////////////////////////////
double GeoMipTerrain::
get_elevation(double x, double y) {
  y = (_ysize - 1) - y;
  unsigned int xlo = (unsigned int) x;
  unsigned int ylo = (unsigned int) y;
  if (xlo < 0) xlo = 0;
  if (ylo < 0) ylo = 0;
  if (xlo > _xsize - 2)
    xlo = _xsize - 2;
  if (ylo > _ysize - 2)
    ylo = _ysize - 2;
  unsigned int xhi = xlo + 1;
  unsigned int yhi = ylo + 1;
  double xoffs = x - xlo;
  double yoffs = y - ylo;
  double grayxlyl = get_pixel_value(xlo, ylo);
  double grayxhyl = get_pixel_value(xhi, ylo);
  double grayxlyh = get_pixel_value(xlo, yhi);
  double grayxhyh = get_pixel_value(xhi, yhi);
  double lerpyl = grayxhyl * xoffs + grayxlyl * (1.0 - xoffs);
  double lerpyh = grayxhyh * xoffs + grayxlyh * (1.0 - xoffs);
  return lerpyh * yoffs + lerpyl * (1.0 - yoffs);
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_normal
//       Access: Published
//  Description: Fetches the terrain normal at (x, y), where the
//               input coordinate is specified in pixels. This
//               ignores the current LOD level and instead provides
//               an accurate number.
//               Terrain scale is NOT taken into account! To get
//               accurate normals, please divide it by the
//               terrain scale and normalize it again, like this:
//
//               LVector3f normal (terr.get_normal(mx, my, x, y));
//               normal.set(normal.get_x() / terr.get_sx(),
//                          normal.get_y() / terr.get_sy(),
//                          normal.get_z() / terr.get_sz());
//               normal.normalize();
////////////////////////////////////////////////////////////////////
LVector3f GeoMipTerrain::
get_normal(int x, int y) {
  int nx = x - 1;
  int px = x + 1;
  int ny = y - 1;
  int py = y + 1;
  if (nx < 0) nx++;
  if (ny < 0) ny++;
  if (px >= int(_xsize)) px--;
  if (py >= int(_ysize)) py--;
  double drx = get_pixel_value(px, y) - get_pixel_value(nx, y);
  double dry = get_pixel_value(x, py) - get_pixel_value(x, ny);
  LVector3f normal(drx * 0.5, dry * 0.5, 1);
  normal.normalize();

  return normal;
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::generate
//       Access: Published
//  Description: (Re)generates the entire terrain, erasing the
//               current.
//               This call un-flattens the terrain, so make sure
//               you have set auto-flatten if you want to keep
//               your terrain flattened.
////////////////////////////////////////////////////////////////////
void GeoMipTerrain::
generate() {
  calc_levels();
  _root.node()->remove_all_children();
  _blocks.clear();
  _old_levels.clear();
  _old_levels.resize(int((_xsize - 1) / _block_size));
  _root_flattened = false;
  for (unsigned int mx = 0; mx < (_xsize - 1) / _block_size; mx++) {
    _old_levels[mx].resize(int((_ysize - 1) / _block_size));
    pvector<NodePath> tvector; //create temporary row
    for (unsigned int my = 0; my < (_ysize - 1) / _block_size; my++) {
      if (_bruteforce) {
        tvector.push_back(generate_block(mx, my, 0));
      } else {
        tvector.push_back(generate_block(mx, my, _levels[mx][my]));
      }
      tvector[my].reparent_to(_root);
      tvector[my].set_pos((mx + 0.5) * _block_size, (my + 0.5) * _block_size, 0);
    }
    _blocks.push_back(tvector); //push the new row of NodePaths into the 2d vect
    tvector.clear();
  }
  auto_flatten();
  _is_dirty = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::update
//       Access: Published
//  Description: Loops through all of the terrain blocks, and
//               checks whether they need to be updated.
//               If that is indeed the case, it regenerates the
//               mipmap. Returns a true when the terrain has
//               changed. Returns false when the terrain isn't
//               updated at all. If there is no terrain yet,
//               it generates the entire terrain.
//               This call un-flattens the terrain, so make sure
//               you have set auto-flatten if you want to keep
//               your terrain flattened.
////////////////////////////////////////////////////////////////////
bool GeoMipTerrain::
update() {
  if (_is_dirty) {
    generate();
    return true;
  } else if (!_bruteforce) {
    calc_levels();
    if (root_flattened()) {
      _root.node()->remove_all_children();
      unsigned int xsize = _blocks.size();
      for (unsigned int tx = 0; tx < xsize; tx++) {
        unsigned int ysize = _blocks[tx].size();
        for (unsigned int ty = 0;ty < ysize; ty++) {
          _blocks[tx][ty].reparent_to(_root);
        }
      }
      _root_flattened = false;
    }
    bool returnVal = false;
    for (unsigned int mx = 0; mx < (_xsize - 1) / _block_size; mx++) {
      for (unsigned int my = 0; my < (_ysize - 1) / _block_size; my++) {
        bool isUpd (update_block(mx, my));
        if (isUpd && mx > 0 && _old_levels[mx - 1][my] == _levels[mx - 1][my]) {
          if (update_block(mx - 1, my, -1, true)) {
            returnVal = true;
          }
        }
        if (isUpd && mx < (_ysize - 1)/_block_size - 1
                  && _old_levels[mx + 1][my] == _levels[mx + 1][my]) {
          if (update_block(mx + 1, my, -1, true)) {
            returnVal = true;
          }
        }
        if (isUpd && my > 0 && _old_levels[mx][my - 1] == _levels[mx][my - 1]) {
          if (update_block(mx, my - 1, -1, true)) {
            returnVal = true;
          }
        }
        if (isUpd && my < (_ysize - 1)/_block_size - 1
                  && _old_levels[mx][my + 1] == _levels[mx][my + 1]) {
          if (update_block(mx, my + 1, -1, true)) {
            returnVal = true;
          }
        }
        if (isUpd) {
          returnVal = true;
        }
      }
    }
    auto_flatten();
    return returnVal;
  }
  return false;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::root_flattened
//       Access: Private
//  Description: Normally, the root's children are the terrain blocks.
//               However, if we call flatten_strong on the root,
//               then the root will contain unpredictable stuff.
//               This function returns true if the root has been
//               flattened, and therefore, does not contain the 
//               terrain blocks.
////////////////////////////////////////////////////////////////////
bool GeoMipTerrain::
root_flattened() {
  if (_root_flattened) {
    return true;
  }
  
  // The following code is error-checking code.  It actually verifies
  // that the terrain blocks are underneath the root, and that nothing
  // else is underneath the root.  It is not very efficient, and should
  // eventually be removed once we're sure everything works.
  
  int total = 0;
  unsigned int xsize = _blocks.size();
  for (unsigned int tx = 0; tx < xsize; tx++) {
    unsigned int ysize = _blocks[tx].size();
    for (unsigned int ty = 0;ty < ysize; ty++) {
      if (_blocks[tx][ty].get_node(1) != _root.node()) {
        grutil_cat.error() << "GeoMipTerrain: root node unexpectedly mangled!\n";
        return true;
      }
      total += 1;
    }
  }
  if (total != _root.node()->get_num_children()) {
    grutil_cat.error() << "GeoMipTerrain: root node unexpectedly mangled: " << total << " vs " << (_root.node()->get_num_children()) << "\n";
    return true;
  }    
  
  // The default.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::auto_flatten
//       Access: Private
//  Description: Flattens the geometry under the root.
////////////////////////////////////////////////////////////////////
void GeoMipTerrain::
auto_flatten() {
  if (_auto_flatten == AFM_off) {
    return;
  }
  
  // Creating a backup node causes the SceneGraphReducer
  // to operate in a nondestructive manner.  This protects
  // the terrain blocks themselves from the flattener.

  NodePath np("Backup Node");
  np.node()->copy_children(_root.node());
  
  // Check if the root's children have changed unexpectedly.
  switch(_auto_flatten) {
  case AFM_light:  _root.flatten_light();  break;
  case AFM_medium: _root.flatten_medium(); break;
  case AFM_strong: _root.flatten_strong(); break;
  }
  
  _root_flattened = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::calc_levels
//       Access: Private
//  Description: Loops through all of the terrain blocks, and
//               calculates on what level they should be generated.
////////////////////////////////////////////////////////////////////
void GeoMipTerrain::
calc_levels() {
  _levels.clear();
  for (unsigned int mx = 0; mx < (_xsize - 1) / _block_size; mx++) {
    pvector<unsigned short> tvector; //create temporary row
    for (unsigned int my = 0; my < (_ysize - 1) / _block_size; my++) {
      if(_bruteforce) {
        tvector.push_back(0);
      } else {
        tvector.push_back(min(short(max(_min_level, lod_decide(mx, my))),
                              short(log(float(_block_size)) / log(2.0))));
      }
    }
    _levels.push_back(tvector); //push the new row of levels into the 2d vector
    tvector.clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::update_block
//       Access: Private
//  Description: Checks whether the specified mipmap at (mx,my)
//               needs to be updated, if so, it regenerates the
//               mipmap. Returns a true when it has generated
//               a mipmap. Returns false when the mipmap is already
//               at the desired level, or when there is no terrain
//               to update. Note: This does not affect neighboring
//               blocks, so does NOT fix t-junctions. You will have
//               to fix that by forced updating the neighboring
//               chunks as well, with the same levels.
//               NOTE: do NOT call this when the terrain is marked
//               dirty. If the terrain is dirty, you will need to
//               call update() or generate() first.
//               You can check this by calling GeoMipTerrain::is_dirty().
////////////////////////////////////////////////////////////////////
bool GeoMipTerrain::
update_block(unsigned short mx, unsigned short my,
                                  signed short level, bool forced) {
  nassertr_always(!_is_dirty, false);
  nassertr_always(mx < (_xsize - 1) / _block_size, false);
  nassertr_always(my < (_ysize - 1) / _block_size, false);
  if (level == -1) {
    level = _levels[mx][my];
  }
  if (forced || _old_levels[mx][my] != level) { // if the level has changed...
    // this code copies the collision mask, removes the chunk and
    // replaces it with a regenerated one.
    CollideMask mask = _blocks[mx][my].get_collide_mask();
    _blocks[mx][my].remove_node();
    _blocks[mx][my] = generate_block(mx, my, level);
    _blocks[mx][my].set_collide_mask(mask);
    _blocks[mx][my].reparent_to(_root);
    _blocks[mx][my].set_pos((mx + 0.5) * _block_size,
                            (my + 0.5) * _block_size, 0);
    return true;
  }
  return false;
}

