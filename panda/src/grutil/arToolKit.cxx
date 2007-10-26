// Filename: arToolKit.cxx
// Created by: jyelon (01Nov2007)
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

#ifdef HAVE_ARTOOLKIT

#include "arToolKit.h"
#include "pandaNode.h"
#include "camera.h"
#include "config_grutil.h"
extern "C" {
  #include "artools.h"
};

ARToolKit::PatternTable ARToolKit::_pattern_table;

////////////////////////////////////////////////////////////////////
//     Function: ARToolKit::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
ARToolKit *ARToolKit::
make(NodePath camera, const Filename &paramfile) {
  if (camera.is_empty()) {
    grutil_cat.error() << "ARToolKit: invalid camera nodepath\n";
    return 0;
  }
  PandaNode *node = camera.node();
  if ((node == 0) || (node->get_type() != Camera::get_class_type())) {
    grutil_cat.error() << "ARToolKit: invalid camera nodepath\n";
    return 0;
  }

  ARParam wparam;
  string fn = paramfile.to_os_specific();
  if( arParamLoad(fn.c_str(), 1, &wparam) < 0 ) {
    grutil_cat.error() << "Cannot load ARToolKit camera config\n";
    return 0;
  }
  
  ARToolKit *result = new ARToolKit();
  result->_camera = camera;
  result->_camera_param = new ARParam;
  memcpy(result->_camera_param, &wparam, sizeof(wparam));
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: ARToolKit::cleanup
//       Access: private
//  Description: Pre-destructor deallocation and cleanup.
////////////////////////////////////////////////////////////////////
void ARToolKit::
cleanup() {
  if (_camera_param) {
    ARParam *param = (ARParam *)_camera_param;
    delete param;
    _camera_param = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ARToolKit::Constructor
//       Access: Private
//  Description: Use ARToolKit::make to create an ARToolKit.
////////////////////////////////////////////////////////////////////
ARToolKit::
ARToolKit() {
}

////////////////////////////////////////////////////////////////////
//     Function: ARToolKit::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
ARToolKit::
~ARToolKit() {
  cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: ARToolKit::get_pattern
//       Access: Private
//  Description: Load the specified pattern into the toolkit, and
//               return the pattern index.  Initially, the pattern
//               is inactive.
////////////////////////////////////////////////////////////////////
int ARToolKit::
get_pattern(const Filename &filename) {
  PatternTable::iterator ptf = _pattern_table.find(filename);
  if (ptf != _pattern_table.end()) {
    return (*ptf).second;
  }
  
  string fn = filename.to_os_specific();
  int id = arLoadPatt(fn.c_str());
  if (id < 0) {
    grutil_cat.error() << "Could not load AR ToolKit Pattern: " << fn << "\n";
    return -1;
  }
  arDeactivatePatt(id);
  _pattern_table[filename] = id;
  return id;
}

////////////////////////////////////////////////////////////////////
//     Function: ARToolKit::attach_pattern
//       Access: Public
//  Description: Associates the specified glyph with the specified
//               NodePath.  Each time you call analyze, ARToolKit
//               will update the NodePath's transform.  If the node
//               is not visible, its scale will be set to zero.
////////////////////////////////////////////////////////////////////
void ARToolKit::
attach_pattern(const Filename &filename, NodePath path) {
  int patt = get_pattern(filename);
  if (patt < 0) return;
  _controls[patt] = path;
}

////////////////////////////////////////////////////////////////////
//     Function: ARToolKit::detach_patterns
//       Access: Public
//  Description: Dissociates all patterns from all NodePaths.
////////////////////////////////////////////////////////////////////
void ARToolKit::
detach_patterns() {
  _controls.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ARToolKit::analyze
//       Access: Public
//  Description: Analyzes the non-pad region of the specified texture.
////////////////////////////////////////////////////////////////////
void ARToolKit::
analyze(Texture *tex, double thresh) {
  nassertv(tex->get_texture_type() == Texture::TT_2d_texture);
  nassertv(tex->get_component_type() == Texture::T_unsigned_byte);
  nassertv(tex->get_num_components() >= 3);
  
  int xsize = tex->get_x_size() - tex->get_pad_x_size();
  int ysize = tex->get_y_size() - tex->get_pad_y_size();
  nassertv((xsize > 0) && (ysize > 0));
  
  ARParam cparam;
  arParamChangeSize( (ARParam*)_camera_param, xsize, ysize, &cparam );
  arInitCparam( &cparam );
  
  cerr << "Analyze video " << xsize << " x " << ysize << "\n";
}


#endif // HAVE_ARTOOLKIT
