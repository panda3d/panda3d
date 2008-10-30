// Filename: geoMipTerrain.I
// Created by:  pro-rsoft (29jun07)
// Last updated by: pro-rsoft (03mar08)
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

#include "config_grutil.h"

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeoMipTerrain::
GeoMipTerrain(const string &name) {
  _root = NodePath(name);
  _root_flattened = false;
  _xsize = 0;
  _ysize = 0;
  _min_level = 0;
  _block_size = 16;
  _factor = 100.0;
  _has_color_map = false;
  PT(PandaNode) tmpnode = new PandaNode("tmp_focal");
  _auto_flatten = AFM_off;
  _focal_point = NodePath(tmpnode);
  _focal_is_temporary = true;
  _is_dirty = true;
  _bruteforce = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::Destructor
//       Access: Published
//  Description: This will not remove the terrain node itself.
//               To have the terrain itself also deleted, please
//               call remove_node() prior to destruction.
////////////////////////////////////////////////////////////////////
INLINE GeoMipTerrain::
~GeoMipTerrain() { 
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::heightfield
//       Access: Published
//  Description: Returns a reference to the heightfield (a PNMImage)
//               contained inside GeoMipTerrain.  You can use
//               the reference to alter the heightfield.
////////////////////////////////////////////////////////////////////
INLINE PNMImage &GeoMipTerrain::
heightfield() {
  return _heightfield;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::color_map
//       Access: Published
//  Description: Returns a reference to the color map (a PNMImage)
//               contained inside GeoMipTerrain.  You can use
//               the reference to alter the color map.
////////////////////////////////////////////////////////////////////
INLINE PNMImage &GeoMipTerrain::
color_map() {
  return _color_map;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::set_bruteforce
//       Access: Published
//  Description: Sets a boolean specifying whether the terrain will
//               be rendered bruteforce. If the terrain is rendered
//               bruteforce, there will be no Level of Detail, and
//               the update() call will only update the
//               terrain if it is marked dirty.
////////////////////////////////////////////////////////////////////
INLINE void GeoMipTerrain::
set_bruteforce(bool bf) {
  if (bf == true && _bruteforce == false) {
    _is_dirty = true;
  }
  _bruteforce = bf;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_bruteforce
//       Access: Published
//  Description: Returns a boolean whether the terrain is rendered
//               bruteforce or not. See set_bruteforce for more
//               information.
////////////////////////////////////////////////////////////////////
INLINE bool GeoMipTerrain::
get_bruteforce() {
  return _bruteforce;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::set_auto_flatten
//       Access: Private
//  Description: The terrain can be automatically flattened (using
//               flatten_light, flatten_medium, or flatten_strong)
//               after each update.  This only affects future
//               updates, it doesn't flatten the current terrain.
//                
////////////////////////////////////////////////////////////////////
INLINE void GeoMipTerrain::
set_auto_flatten(int mode) {
  _auto_flatten = mode;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::set_focal_point
//       Access: Published
//  Description: Sets the focal point.  GeoMipTerrain generates
//               high-resolution terrain around the focal point, and
//               progressively lower and lower resolution terrain
//               as you get farther away. If a point is supplied
//               and not a NodePath, make sure it's relative to
//               the terrain. Only the x and y coordinates of
//               the focal point are taken in respect.
////////////////////////////////////////////////////////////////////
INLINE void GeoMipTerrain::
set_focal_point(double x, double y) {
  if (!_focal_is_temporary) {
    PT(PandaNode) tmpnode = new PandaNode("tmp_focal");
    _focal_point = NodePath(tmpnode);
  }
  _focal_point.set_pos(_root, x, y, 0);
  _focal_is_temporary = true;
}
INLINE void GeoMipTerrain::
set_focal_point(LPoint2d fp) {
  set_focal_point(fp.get_x(), fp.get_y());
}
INLINE void GeoMipTerrain::
set_focal_point(LPoint2f fp) {
  set_focal_point(double(fp.get_x()), double(fp.get_y()));
}
INLINE void GeoMipTerrain::
set_focal_point(LPoint3d fp) {
  set_focal_point(fp.get_x(), fp.get_y());
}
INLINE void GeoMipTerrain::
set_focal_point(LPoint3f fp) {
  set_focal_point(double(fp.get_x()), double(fp.get_y()));
}
INLINE void GeoMipTerrain::
set_focal_point(NodePath fp) {
  if (_focal_is_temporary) {
    _focal_point.remove_node();
  }
  _focal_point = fp;
  _focal_is_temporary = false;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_focal_point
//       Access: Published
//  Description: Returns the focal point, as a NodePath.
//               If you have set it to be just a point, it will
//               return an empty node at the focal position.
////////////////////////////////////////////////////////////////////
INLINE NodePath GeoMipTerrain::
get_focal_point() const {
  return _focal_point;
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_root
//       Access: Published
//  Description: Returns the root of the terrain.  This is a
//               single PandaNode to which all the rest of the
//               terrain is parented.  The generate and update
//               operations replace the nodes which are parented
//               to this root, but they don't replace this root
//               itself.
////////////////////////////////////////////////////////////////////
INLINE NodePath GeoMipTerrain::
get_root() const {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::set_min_level
//       Access: Published
//  Description: Sets the minimum level of detail at which blocks
//               may be generated by generate() or update().
//               The default value is 0, which is the highest
//               quality. This value is also taken in respect when
//               generating the terrain bruteforce.
////////////////////////////////////////////////////////////////////
INLINE void GeoMipTerrain::
set_min_level(unsigned short minlevel) {
  _min_level = minlevel;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_min_level
//       Access: Published
//  Description: Gets the minimum level of detail at which blocks
//               may be generated by generate() or update().
//               The default value is 0, which is the highest
//               quality.
////////////////////////////////////////////////////////////////////
INLINE unsigned short GeoMipTerrain::
get_min_level() {
  return _min_level;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_block_size
//       Access: Published
//  Description: Gets the block size.
////////////////////////////////////////////////////////////////////
INLINE unsigned short GeoMipTerrain::
get_block_size() {
  return _block_size;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::set_block_size
//       Access: Published
//  Description: Sets the block size. If it is not a power of two,
//               the closest power of two is used.
////////////////////////////////////////////////////////////////////
INLINE void GeoMipTerrain::
set_block_size(unsigned short newbs) {
  if (is_power_of_two(newbs)) {
    _block_size = newbs;
  } else {
    if (is_power_of_two(newbs - 1)) {
      _block_size = newbs - 1;
    } else {
      if (is_power_of_two(newbs + 1)) {
        _block_size = newbs + 1;
      } else {
        _block_size = (unsigned short) pow(2.0,
                            floor(log((double) newbs) / log(2.0) + 0.5));
      }
    }
  }
  _is_dirty = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::is_dirty
//       Access: Published
//  Description: Returns a bool indicating whether the terrain is
//               marked 'dirty', that means the terrain has to be
//               regenerated on the next update() call, because
//               for instance the heightfield has changed.
//               Once the terrain has been regenerated, the dirty
//               flag automatically gets reset internally.
////////////////////////////////////////////////////////////////////
INLINE bool GeoMipTerrain::
is_dirty() {
  return _is_dirty;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::set_factor
//       Access: Published
//  Description: Sets the quality factor at which blocks must be
//               generated. The higher this level, the better
//               quality the terrain will be, but more expensive
//               to render. A value of 0 makes the terrain the
//               lowest quality possible, depending on blocksize.
//               The default value is 100.
////////////////////////////////////////////////////////////////////
INLINE void GeoMipTerrain::
set_factor(float factor) {
  _factor = factor;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_factor
//       Access: Published
//  Description: Gets the quality factor at which blocks must be
//               generated. The higher this level, the better
//               quality the terrain will be, but more expensive
//               to render. A value of 0 makes the terrain the
//               lowest quality possible, depending on blocksize.
//               The default value is 100.
////////////////////////////////////////////////////////////////////
INLINE float GeoMipTerrain::
get_factor() {
  return _factor;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_block_node_path
//       Access: Published
//  Description: Returns the NodePath of the specified block.
//               If auto-flatten is enabled and the node is
//               getting removed during the flattening process,
//               it will still return a NodePath with the
//               appropriate terrain chunk, but it will be in
//               a temporary scenegraph.
//               Please note that this returns a const object and
//               you can not modify the node. Modify the heightfield
//               instead.
////////////////////////////////////////////////////////////////////
INLINE const NodePath GeoMipTerrain::
get_block_node_path(unsigned short mx, unsigned short my) {
  return _blocks[mx][my];
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_block_from_pos
//       Access: Published
//  Description: Gets the coordinates of the block at the specified
//               position. This position must be relative to the
//               terrain, not to render. Returns an array containing
//               two values: the block x and the block y coords.
//               If the positions are out of range, the closest
//               block is taken.
//               Note that the VecBase returned does not represent
//               a vector, position, or rotation, but it contains
//               the block index of the block which you can use
//               in GeoMipTerrain::get_block_node_path.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f GeoMipTerrain::
get_block_from_pos(double x, double y) {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x > _xsize - 1) x = _xsize - 1;
  if (y > _ysize - 1) y = _ysize - 1;
  x = floor(x / _block_size);
  y = floor(y / _block_size);
  return LVecBase2f(x, y);
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::lod_decide
//       Access: Private
//  Description: Calculates the level for the given mipmap.
////////////////////////////////////////////////////////////////////
INLINE unsigned short GeoMipTerrain::
lod_decide(unsigned short mx, unsigned short my) {
  float cx = mx; 
  float cy = my;
  cx = (cx * _block_size + _block_size / 2) * _root.get_sx();
  cy = (cy * _block_size + _block_size / 2) * _root.get_sy();
  float d;
  if (_factor > 0.0) {
    d = sqrt(pow(_focal_point.get_x(_root) - cx, 2) +
             pow(_focal_point.get_y(_root) - cy, 2)) / _factor;
  } else {
    d = log((double) _block_size) / log(2.0);
  }
  return short(floor(d));
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::set_heightfield
//       Access: Published
//  Description: Loads the specified heightmap image file into
//               the heightfield. Returns true if succeeded, or
//               false if an error has occured.
//               If the heightmap is not a power of two plus one,
//               it is scaled up using a gaussian filter.
////////////////////////////////////////////////////////////////////
INLINE bool GeoMipTerrain::
set_heightfield(const Filename &filename, PNMFileType *ftype) {
  // First, we need to load the header to determine the size and format.
  PNMImageHeader imgheader;
  if (imgheader.read_header(filename, ftype)) {
    // Copy over the header to the heightfield image.
    _heightfield.copy_header_from(imgheader);
    
    if(!is_power_of_two(imgheader.get_x_size() - 1) || !is_power_of_two(imgheader.get_y_size() - 1)) {
      // Calculate the nearest power-of-two-plus-one size.
      unsigned int reqx, reqy;
      reqx = max(3, (int) pow(2.0, ceil(log((double) max(2, imgheader.get_x_size() - 1)) / log(2.0))) + 1);
      reqy = max(3, (int) pow(2.0, ceil(log((double) max(2, imgheader.get_y_size() - 1)) / log(2.0))) + 1);
      
      // If it's not a valid size, tell PNMImage to resize it.
      if (reqx != imgheader.get_x_size() || reqy != imgheader.get_y_size()) {
        grutil_cat.warning() << "Rescaling heightfield image " << filename << " to "
                                             << reqx << " by " << reqy << " pixels\n";
        _heightfield.set_read_size(reqx, reqy);
      }
    }
    
    // Read the real image now
    if (!_heightfield.read(filename, ftype)) {
      _heightfield.clear_read_size();
      grutil_cat.error() << "Failed to read heightfield image " << filename << "!\n";
      return false;
    }
    
    _is_dirty = true;
    _xsize = _heightfield.get_x_size();
    _ysize = _heightfield.get_y_size();
    return true;
  } else {
    grutil_cat.error() << "Failed to load heightfield image " << filename << "!\n";
  }
  return false;
}

INLINE bool GeoMipTerrain::
set_heightfield(const PNMImage &image) {
  // Before we apply anything, validate the size.
  if(is_power_of_two(image.get_x_size() - 1) && is_power_of_two(image.get_y_size() - 1)) {
    _heightfield = image;
    _is_dirty = true;
    _xsize = _heightfield.get_x_size();
    _ysize = _heightfield.get_y_size();
    return true;
  } else {
    grutil_cat.error() << "Specified image does not have a power-of-two-plus-one size!\n";
  }
  return false;
}

INLINE bool GeoMipTerrain::
set_heightfield(const Texture *tex) {
  grutil_cat.warning() << "Setting a texture as heightfield image is deprecated!\n";
  PNMImage image;
  tex->store(image);
  return set_heightfield(image);
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::set_color_map
//       Access: Published
//  Description: Loads the specified image as color map. The next
//               time generate() is called, the terrain is painted
//               with this color map using the vertex color column.
//               Returns a boolean indicating whether the operation
//               has succeeded.
////////////////////////////////////////////////////////////////////
INLINE bool GeoMipTerrain::
set_color_map(const Filename &filename, PNMFileType *ftype) {
  if (_color_map.read(filename, ftype)) {
    _is_dirty = true;
    _has_color_map = true;
    return true;
  }
  return false;
}
INLINE bool GeoMipTerrain::
set_color_map(const PNMImage &image) {
  _color_map.copy_from(image);
  _is_dirty = true;
  _has_color_map = true;
  return true;
}
INLINE bool GeoMipTerrain::
set_color_map(const Texture *tex) {
  tex->store(_color_map);
  _is_dirty = true;
  return true;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::has_color_map
//       Access: Published
//  Description: Returns whether a color map has been set.
////////////////////////////////////////////////////////////////////
INLINE bool GeoMipTerrain::
has_color_map() {
  return _has_color_map;
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::clear_color_map
//       Access: Published
//  Description: Clears the color map.
////////////////////////////////////////////////////////////////////
INLINE void GeoMipTerrain::
clear_color_map() {
  if (_has_color_map) {
    _color_map.clear();
    _has_color_map = false;
  }
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_pixel_value
//       Access: Private
//  Description: Get the elevation at a certain pixel of the image.
//               This function does NOT linearly interpolate.
//               For that, use GeoMipTerrain::get_elevation() instead.
////////////////////////////////////////////////////////////////////
INLINE double GeoMipTerrain::
get_pixel_value(int x, int y) {
  x = max(min(x,int(_xsize-1)),0);
  y = max(min(y,int(_ysize-1)),0);
  return double(_heightfield.get_bright(int(x),int(y)));
/*  return double(_heightfield.get_red_val(int(x),int(y))
              + _heightfield.get_green_val(int(x),int(y)) * 256
              + _heightfield.get_blue_val(int(x),int(y)) * 65536) / 16777215.0;
*/
}
INLINE double GeoMipTerrain::
get_pixel_value(unsigned short mx, unsigned short my, int x, int y) {
  nassertr_always(mx < (_xsize - 1) / _block_size, false);
  nassertr_always(my < (_ysize - 1) / _block_size, false);
  return get_pixel_value(mx * _block_size + x, (_ysize - 1) -
                         (my * _block_size + y));
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::get_normal
//       Access: Published
//  Description: Fetches the terrain normal at (x,y), where the input
//               coordinate is specified in pixels. This ignores the
//               current LOD level and instead provides an
//               accurate number.
//               Terrain scale is NOT taken into account! To get
//               accurate normals, please divide it by the
//               terrain scale and normalize it again!
////////////////////////////////////////////////////////////////////
INLINE LVector3f GeoMipTerrain::
get_normal(unsigned short mx, unsigned short my, int x, int y) {
  nassertr_always(mx < (_xsize - 1) / _block_size, false);
  nassertr_always(my < (_ysize - 1) / _block_size, false);
  return get_normal(mx * _block_size + x, (_ysize - 1) -
                    (my * _block_size + y));
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::int2str
//       Access: Private
//  Description: Converts the given int to a std::string.
////////////////////////////////////////////////////////////////////
INLINE std::string GeoMipTerrain::
int_to_str(int i) {
  std::stringstream ss;
  std::string str;
  ss << i;
  ss >> str;
  return str;
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::str2int
//       Access: Private
//  Description: Converts the given std::string to an int.
////////////////////////////////////////////////////////////////////
INLINE int GeoMipTerrain::
str_to_int(std::string str) {
  std::istringstream strin(str);
  int i;
  strin >> i;
  return i;
}

////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::is_power_of_two
//       Access: Private
//  Description: Returns a bool whether the given int i is a
//               power of two or not.
////////////////////////////////////////////////////////////////////
INLINE bool GeoMipTerrain::
is_power_of_two(unsigned int i) {
  return !((i - 1) & i);
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::f_part
//       Access: Private
//  Description: Returns the part of the number right of the
//               floating-point.
////////////////////////////////////////////////////////////////////
INLINE float GeoMipTerrain::
f_part(float i) {
  return i - floor(i);
}
INLINE double GeoMipTerrain::
f_part(double i) {
  return i - floor(i);
}
////////////////////////////////////////////////////////////////////
//     Function: GeoMipTerrain::sfav
//       Access: Private
//  Description: Used to calculate vertex numbers. Only to
//               be used internally.
////////////////////////////////////////////////////////////////////
INLINE int GeoMipTerrain::
sfav(int n, int powlevel, int mypowlevel) {
  double t = n - 1;
  t /= pow(2.0, powlevel - mypowlevel);
  t = double(int(t > 0.0 ? t + 0.5 : t - 0.5));
  t *= pow(2.0, powlevel - mypowlevel);
  return int(t);
}

