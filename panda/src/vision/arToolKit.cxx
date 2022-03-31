/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file arToolKit.cxx
 * @author jyelon
 * @date 2007-11-01
 */

#include "arToolKit.h"

#ifdef HAVE_ARTOOLKIT

#include "pandaNode.h"
#include "camera.h"
#include "perspectiveLens.h"
#include "lvecBase3.h"
#include "compose_matrix.h"
#include "config_vision.h"
extern "C" {
  #include <AR/ar.h>
};

ARToolKit::PatternTable ARToolKit::_pattern_table;

static void change_size( ARParam *source, int xsize, int ysize, ARParam *newparam )
{
    int     i;

    newparam->xsize = xsize;
    newparam->ysize = ysize;

    double xscale = (double)xsize / (double)(source->xsize);
    double yscale = (double)ysize / (double)(source->ysize);
    for( i = 0; i < 4; i++ ) {
      newparam->mat[0][i] = source->mat[0][i] * xscale;
      newparam->mat[1][i] = source->mat[1][i] * yscale;
      newparam->mat[2][i] = source->mat[2][i];
    }

    newparam->dist_factor[0] = source->dist_factor[0] * xscale;
    newparam->dist_factor[1] = source->dist_factor[1] * yscale;
    newparam->dist_factor[2] = source->dist_factor[2] / (xscale*yscale);
    newparam->dist_factor[3] = source->dist_factor[3];
}

static void analyze_fov(double cparam[3][4], int width, int height, double &xfov, double &yfov)
{
  double   gnear = 10.0;
  double gfar = 1000.0;
  double   icpara[3][4];
  double   trans[3][4];
  double   p[3][3], q[4][4];
  double   xval, yval;
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

  xval =
    q[0][0] * trans[0][0] +
    q[0][1] * trans[1][0] +
    q[0][2] * trans[2][0];
  yval =
    q[1][0] * trans[0][1] +
    q[1][1] * trans[1][1] +
    q[1][2] * trans[2][1];

  xfov = 2.0 * atan(1.0/xval) * (180.0/3.141592654);
  yfov = 2.0 * atan(1.0/yval) * (180.0/3.141592654);
}

/**
 * Create a new ARToolKit instance.
 *
 * Camera must be the nodepath of a panda camera object.  The panda camera's
 * field of view is initialized to match the field of view of the physical
 * webcam.  Each time you call analyze, all marker nodepaths will be moved
 * into a position which is relative to this camera.  The marker_size
 * parameter indicates how large you printed the physical markers.  You should
 * use the same size units that you wish to use in the panda code.
 */
ARToolKit *ARToolKit::
make(NodePath camera, const Filename &paramfile, double marker_size) {

  if (AR_DEFAULT_PIXEL_FORMAT != AR_PIXEL_FORMAT_BGRA &&
      AR_DEFAULT_PIXEL_FORMAT != AR_PIXEL_FORMAT_RGBA &&
      AR_DEFAULT_PIXEL_FORMAT != AR_PIXEL_FORMAT_ARGB &&
      AR_DEFAULT_PIXEL_FORMAT != AR_PIXEL_FORMAT_ABGR &&
      AR_DEFAULT_PIXEL_FORMAT != AR_PIXEL_FORMAT_RGB &&
      AR_DEFAULT_PIXEL_FORMAT != AR_PIXEL_FORMAT_BGR) {
    vision_cat.error() <<
      "The copy of ARToolKit that you are using is not compiled "
      "for RGB, BGR, RGBA, BGRA, ARGB or ABGR input.  Panda3D cannot "
      "use this copy of ARToolKit. Please modify the ARToolKit's "
      "config file and compile it again.\n";
    return 0;
  }

  if (camera.is_empty()) {
    vision_cat.error() << "ARToolKit: invalid camera nodepath\n";
    return 0;
  }
  PandaNode *node = camera.node();
  if ((node == 0) || (node->get_type() != Camera::get_class_type())) {
    vision_cat.error() << "ARToolKit: invalid camera nodepath\n";
    return 0;
  }
  Camera *cam = DCAST(Camera, node);
  Lens *lens = cam->get_lens();
  if (lens->get_type() != PerspectiveLens::get_class_type()) {
    vision_cat.error() << "ARToolKit: supplied camera node must be perspective.\n";
    return 0;
  }

  ARParam wparam;
  std::string fn = paramfile.to_os_specific();
  if( arParamLoad(fn.c_str(), 1, &wparam) < 0 ) {
    vision_cat.error() << "Cannot load ARToolKit camera config\n";
    return 0;
  }

  arParamDisp(&wparam);
  double xfov, yfov;
  analyze_fov(wparam.mat, 640, 480, xfov, yfov);

  lens->set_fov(xfov, yfov);

  ARToolKit *result = new ARToolKit();
  result->_camera = camera;
  result->_camera_param = new ARParam;
  result->_threshold = 0.5;
  result->_marker_size = marker_size;
  result->_have_prev_conv = false;
  memcpy(result->_camera_param, &wparam, sizeof(wparam));
  return result;
}


/**
 * Pre-destructor deallocation and cleanup.
 */
void ARToolKit::
cleanup() {
  if (_camera_param) {
    ARParam *param = (ARParam *)_camera_param;
    delete param;
    _camera_param = 0;
  }
}

/**
 * Use ARToolKit::make to create an ARToolKit.
 */
ARToolKit::
ARToolKit() : _have_prev_conv(false) {
}

/**
 *
 */
ARToolKit::
~ARToolKit() {
  cleanup();
}

/**
 * Load the specified pattern into the toolkit, and return the pattern index.
 * Initially, the pattern is inactive.
 */
int ARToolKit::
get_pattern(const Filename &filename) {
  PatternTable::iterator ptf = _pattern_table.find(filename);
  if (ptf != _pattern_table.end()) {
    return (*ptf).second;
  }

  std::string fn = filename.to_os_specific();
  int id = arLoadPatt(fn.c_str());
  if (id < 0) {
    vision_cat.error() << "Could not load AR ToolKit Pattern: " << fn << "\n";
    return -1;
  }
  arDeactivatePatt(id);
  _pattern_table[filename] = id;
  return id;
}

/**
 * Associates the specified glyph with the specified NodePath.  Each time you
 * call analyze, ARToolKit will update the NodePath's transform.  If the node
 * is not visible, its scale will be set to zero.
 */
void ARToolKit::
attach_pattern(const Filename &filename, NodePath path) {
  int patt = get_pattern(filename);
  if (patt < 0) return;
  _controls[patt] = path;
}

/**
 * Dissociates all patterns from all NodePaths.
 */
void ARToolKit::
detach_patterns() {
  _controls.clear();
}

/**
 * Analyzes the non-pad region of the specified texture.  This causes all
 * attached nodepaths to move.  The parameter do_flip_texture is true by
 * default, because Panda's representation of textures is upside down from
 * ARToolKit.  If you already have a texture that's upside-down, however, you
 * should set it to false.
 */
void ARToolKit::
analyze(Texture *tex, bool do_flip_texture) {
  // We shouldn't assert on has_ram_image since it also returns false when
  // there is a ram image but it's not updated for this frame.
  // nassertv(tex->has_ram_image());
  nassertv(tex->get_ram_image_compression() == Texture::CM_off);
  nassertv(tex->get_component_type() == Texture::T_unsigned_byte);
  nassertv(tex->get_texture_type() == Texture::TT_2d_texture);

  if (tex->get_num_components() != 3 && tex->get_num_components() != 4) {
    vision_cat.error() << "ARToolKit can only analyze RGB and RGBA textures.\n";
    return;
  }

  int padx = tex->get_pad_x_size();
  int pady = tex->get_pad_y_size();
  int xsize = tex->get_x_size() - padx;
  int ysize = tex->get_y_size() - pady;
  // int pagesize = xsize * ysize * 4;
  nassertv((xsize > 0) && (ysize > 0));

  // row length in bytes
  int srclen = tex->get_x_size() * tex->get_num_components();

  ARParam cparam;
  change_size((ARParam*)_camera_param, xsize, ysize, &cparam);
  arInitCparam(&cparam);

  // Pack the data into a buffer with no padding and invert the video
  // vertically (panda's representation is upside down from ARToolKit) Note:
  // ARToolKit treats the images as grayscale, so the order of the individual
  // R, G and B components does not matter.

  CPTA_uchar ri = tex->get_ram_image();
  const unsigned char *ram = ri.p();

  if (ram == nullptr) {
    vision_cat.warning() << "No data in texture!\n";
    return;
  }

  unsigned char *data;
  unsigned char *dstrow;
  const unsigned char *srcrow;

  if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB ||
      AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR) {
    data = new unsigned char[xsize * ysize * 3];
    int dstlen = xsize * 3;
    if (tex->get_num_components() == 3) {
      if (do_flip_texture) {
        for (int y = 0; y < ysize; ++y) {
          int invy = (ysize - y - 1);
          memcpy(data + invy * dstlen, ram + y * srclen, dstlen);
        }
      } else if (dstlen == srclen) {
        memcpy(data, ram, ysize * srclen);
      } else {
        for (int y = 0; y < ysize; ++y) {
          memcpy(data + y * dstlen, ram + y * srclen, dstlen);
        }
      }
    } else {
      // Chop off the alpha component.
      if (do_flip_texture) {
        for (int y = 0; y < ysize; ++y) {
          dstrow = data + dstlen * (ysize - y - 1);
          srcrow = ram + srclen * y;
          for (int x = 0; x < xsize; ++x) {
            memcpy(dstrow + x * 3, srcrow + x * 4, 3);
          }
        }
      } else {
        for (int y = 0; y < ysize; y++) {
          dstrow = data + dstlen * y;
          srcrow = ram + srclen * y;
          for (int x = 0; x < xsize; x++) {
            memcpy(dstrow + x * 3, srcrow + x * 4, 3);
          }
        }
      }
    }
  } else if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA ||
             AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA) {
    data = new unsigned char[xsize * ysize * 4];
    int dstlen = xsize * 4;
    if (tex->get_num_components() == 3) {
      // We'll have to add an alpha component.
      if (do_flip_texture) {
        for (int y = 0; y < ysize; ++y) {
          dstrow = data + dstlen * (ysize - y - 1);
          srcrow = ram + srclen * y;
          for (int x = 0; x < xsize; ++x) {
            memcpy(dstrow + x * 4, srcrow + x * 3, 3);
            dstrow[x * 4 + 3] = 255;
          }
        }
      } else {
        for (int y = 0; y < ysize; ++y) {
          dstrow = data + dstlen * y;
          srcrow = ram + srclen * y;
          for (int x = 0; x < xsize; ++x) {
            memcpy(dstrow + x * 4, srcrow + x * 3, 3);
            dstrow[x * 4 + 3] = 255;
          }
        }
      }
    } else {
      if (do_flip_texture) {
        for (int y = 0; y < ysize; ++y) {
          int invy = (ysize - y - 1);
          memcpy(data + invy * dstlen, ram + y * srclen, dstlen);
        }
      } else if (dstlen == srclen) {
        memcpy(data, ram, ysize * srclen);
      } else {
        for (int y = 0; y < ysize; ++y) {
          memcpy(data + y * dstlen, ram + y * srclen, dstlen);
        }
      }
    }
  } else { // ARToolKit wants ARGB / ABGR.
    data = new unsigned char[xsize * ysize * 4];
    int dstlen = xsize * 4;
    if (tex->get_num_components() == 3) {
      // We'll have to add an alpha component.
      if (do_flip_texture) {
        for (int y = 0; y < ysize; ++y) {
          dstrow = data + dstlen * (ysize - y - 1);
          srcrow = ram + srclen * y;
          for (int x = 0; x < xsize; ++x) {
            memcpy(dstrow + x * 4 + 1, srcrow + x * 3, 3);
            dstrow[x * 4] = 255;
          }
        }
      } else {
        for (int y = 0; y < ysize; ++y) {
          dstrow = data + dstlen * y;
          srcrow = ram + srclen * y;
          for (int x = 0; x < xsize; ++x) {
            memcpy(dstrow + x * 4 + 1, srcrow + x * 3, 3);
            dstrow[x * 4] = 255;
          }
        }
      }
    } else {
      if (do_flip_texture) {
        for (int y = 0; y < ysize; ++y) {
          dstrow = data + dstlen * (ysize - y - 1);
          srcrow = ram + srclen * y;
          for (int x = 0; x < xsize; ++x) {
            memcpy(dstrow + x * 4 + 1, srcrow + x * 4, 3);
            dstrow[x * 4] = srcrow[x * 4 + 3];
          }
        }
      } else {
        for (int y = 0; y < ysize; ++y) {
          dstrow = data + dstlen * y;
          srcrow = ram + srclen * y;
          for (int x = 0; x < xsize; ++x) {
            memcpy(dstrow + x * 4 + 1, srcrow + x * 4, 3);
            dstrow[x * 4] = srcrow[x * 4 + 3];
          }
        }
      }
    }
  }

  // Activate the patterns.
  Controls::const_iterator ctrli;
  for (ctrli = _controls.begin(); ctrli != _controls.end(); ++ctrli) {
    arActivatePatt((*ctrli).first);
  }

  ARMarkerInfo *marker_info;
  int marker_num;

  if (arDetectMarker(data, _threshold * 256, &marker_info, &marker_num) < 0) {
    vision_cat.error() << "ARToolKit detection error.\n";
    delete data;
    return;
  }

  for (ctrli = _controls.begin(); ctrli != _controls.end(); ++ctrli) {
    NodePath np = (*ctrli).second;
    int pattern = (*ctrli).first;
    arDeactivatePatt(pattern);
    double conf = -1;
    int best = -1;
    for (int i = 0; i < marker_num; ++i) {
      if (marker_info[i].id == pattern) {
        if (marker_info[i].cf >= conf) {
          conf = marker_info[i].cf;
          best = i;
        }
      }
    }
    if (conf > 0.0) {
      ARMarkerInfo *inf = &marker_info[best];
      double center[2];
      center[0] = 0.0;
      center[1] = 0.0;
      if (_have_prev_conv) {
        arGetTransMatCont(inf, _prev_conv, center, _marker_size, _prev_conv);
      } else {
        arGetTransMat(inf, center, _marker_size, _prev_conv);
        _have_prev_conv = true;
      }
      LMatrix4 mat;
      for (int i = 0; i < 4; ++i) {
        mat(i, 0) =  _prev_conv[0][i];
        mat(i, 1) =  _prev_conv[2][i];
        mat(i, 2) = -_prev_conv[1][i];
        mat(i, 3) = 0.0;
      }
      mat(3,3) = 1.0;
      LVecBase3 scale, shear, hpr, pos;
      decompose_matrix(mat, scale, shear, hpr, pos);

      if (np.get_parent().is_empty()) {
        vision_cat.error() << "NodePath must have a parent.\n";
      } else {
        np.set_pos_hpr(_camera, pos, hpr);
      }

      np.show();
    } else {
      np.hide();
    }
  }

  delete data;
}

#endif // HAVE_ARTOOLKIT
