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

#include "arToolKit.h"

#ifdef HAVE_ARTOOLKIT

#include "pandaNode.h"
#include "camera.h"
#include "perspectiveLens.h"
#include "config_grutil.h"
extern "C" {
  #include "artools.h"
};

ARToolKit::PatternTable ARToolKit::_pattern_table;

static void argConvGLcpara2( double cparam[3][4], int width, int height, double gnear, double gfar, double m[16] )
{
    double   icpara[3][4];
    double   trans[3][4];
    double   p[3][3], q[4][4];
    int      i, j;

    if( arParamDecompMat(cparam, icpara, trans) < 0 ) {
        printf("gConvGLcpara: Parameter error!!\n");
        exit(0);
    }

    for( i = 0; i < 3; i++ ) {
        for( j = 0; j < 3; j++ ) {
            p[i][j] = icpara[i][j] / icpara[2][2];
        }
    }
    q[0][0] = (2.0 * p[0][0] / width);
    q[0][1] = (2.0 * p[0][1] / width);
    q[0][2] = ((2.0 * p[0][2] / width)  - 1.0);
    q[0][3] = 0.0;

    q[1][0] = 0.0;
    q[1][1] = (2.0 * p[1][1] / height);
    q[1][2] = ((2.0 * p[1][2] / height) - 1.0);
    q[1][3] = 0.0;

    q[2][0] = 0.0;
    q[2][1] = 0.0;
    q[2][2] = (gfar + gnear)/(gfar - gnear);
    q[2][3] = -2.0 * gfar * gnear / (gfar - gnear);

    q[3][0] = 0.0;
    q[3][1] = 0.0;
    q[3][2] = 1.0;
    q[3][3] = 0.0;

    for( i = 0; i < 4; i++ ) {
        for( j = 0; j < 3; j++ ) {
            m[i+j*4] = q[i][0] * trans[0][j]
                     + q[i][1] * trans[1][j]
                     + q[i][2] * trans[2][j];
        }
        m[i+3*4] = q[i][0] * trans[0][3]
                 + q[i][1] * trans[1][3]
                 + q[i][2] * trans[2][3]
                 + q[i][3];
    }
}

////////////////////////////////////////////////////////////////////
//     Function: ARToolKit::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
ARToolKit *ARToolKit::
make(NodePath camera, const Filename &paramfile) {
  
  if (AR_DEFAULT_PIXEL_FORMAT != AR_PIXEL_FORMAT_BGRA) {
    grutil_cat.error() <<
      "The copy of ARToolKit that you are using is not compiled "
      "for BGRA input.  Panda3D cannot use this copy of ARToolKit. "
      "Modify the ARToolKit's config file and compile it again.\n";
    return 0;
  }
  
  if (camera.is_empty()) {
    grutil_cat.error() << "ARToolKit: invalid camera nodepath\n";
    return 0;
  }
  PandaNode *node = camera.node();
  if ((node == 0) || (node->get_type() != Camera::get_class_type())) {
    grutil_cat.error() << "ARToolKit: invalid camera nodepath\n";
    return 0;
  }
  Camera *cam = DCAST(Camera, node);
  Lens *lens = cam->get_lens();
  if (lens->get_type() != PerspectiveLens::get_class_type()) {
    grutil_cat.error() << "ARToolKit: supplied camera node must be perspective.\n";
    return 0;
  }

  ARParam wparam;
  string fn = paramfile.to_os_specific();
  if( arParamLoad(fn.c_str(), 1, &wparam) < 0 ) {
    grutil_cat.error() << "Cannot load ARToolKit camera config\n";
    return 0;
  }
  
  arParamDisp(&wparam);
  double m[16];
  argConvGLcpara2(wparam.mat, 640, 480, 10, 1000, m);
  cerr << "Panda3D projection mat: " << lens->get_projection_mat() << "\n";
  cerr << "Artoolkit mat: "
       << m[ 0] << " " << m[ 1] << " " << m[ 2] << " " << m[ 3] << "\n"
       << m[ 4] << " " << m[ 5] << " " << m[ 6] << " " << m[ 7] << "\n"
       << m[ 8] << " " << m[ 9] << " " << m[10] << " " << m[11] << "\n"
       << m[12] << " " << m[13] << " " << m[14] << " " << m[15] << "\n";
  
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
  nassertv(tex->has_ram_image());
  nassertv(tex->get_ram_image_compression() == Texture::CM_off);
  nassertv(tex->get_component_type() == Texture::T_unsigned_byte);
  nassertv(tex->get_texture_type() == Texture::TT_2d_texture);

  if (tex->get_num_components() != 4) {
    grutil_cat.error() << "ARToolKit can only analyze RGBA textures.\n";
    return;
  }
  
  int padx = tex->get_pad_x_size();
  int pady = tex->get_pad_y_size();
  int xsize = tex->get_x_size() - padx;
  int ysize = tex->get_y_size() - pady;
  int pagesize = xsize * ysize * 4;
  nassertv((xsize > 0) && (ysize > 0));
  
  ARParam cparam;
  arParamChangeSize( (ARParam*)_camera_param, xsize, ysize, &cparam );
  arInitCparam( &cparam );

  // Pack the data into a buffer with no padding and invert the video
  // vertically (panda's representation is upside down from ARToolKit)

  CPTA_uchar ri = tex->get_ram_image();
  const unsigned char *ram = ri.p();
  unsigned char *data = new unsigned char[xsize * ysize * 4];
  int dstlen = xsize * 4;
  int srclen = (xsize + padx) * 4;
  for (int y=0; y<ysize; y++) {
    int invy = (ysize - y - 1);
    memcpy(data + invy * dstlen, ram + y * srclen, dstlen);
  }
  
  Controls::const_iterator ctrli;
  for (ctrli=_controls.begin(); ctrli!=_controls.end(); ctrli++) {
    arActivatePatt((*ctrli).first);
  }

  ARMarkerInfo *marker_info;
  int marker_num;

  cerr << "Analyze video " << xsize << " x " << ysize << "\n";
  if (arDetectMarker(data, thresh, &marker_info, &marker_num) < 0) {
    grutil_cat.error() << "ARToolKit detection error.\n";
    return;
  }

  for (ctrli=_controls.begin(); ctrli!=_controls.end(); ctrli++) {
    int pattern = (*ctrli).first;
    arDeactivatePatt(pattern);
    double conf = -1;
    int best = -1;
    for (int i=0; i<marker_num; i++) {
      if (marker_info[i].id == pattern) {
        if (marker_info[i].cf >= conf) {
          conf = marker_info[i].cf;
          best = i;
        }
      }
    }
    if (conf > 0.0) {
      cerr << "Marker " << pattern 
           << " " << int(marker_info[best].vertex[0][0]) << "," << int(marker_info[best].vertex[0][1])
           << " " << int(marker_info[best].vertex[1][0]) << "," << int(marker_info[best].vertex[1][1])
           << " " << int(marker_info[best].vertex[2][0]) << "," << int(marker_info[best].vertex[2][1])
           << " " << int(marker_info[best].vertex[3][0]) << "," << int(marker_info[best].vertex[3][1])
           << "\n";
    }
  }
  
  delete data;
}


#endif // HAVE_ARTOOLKIT
