// Filename: config_grutil.cxx
// Created by:  drose (24May00)
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
#include "frameRateMeter.h"
#include "meshDrawer.h"
#include "geoMipTerrain.h"
#include "openCVTexture.h"
#include "ffmpegTexture.h"
#include "movieTexture.h"
#include "pandaSystem.h"
#include "texturePool.h"
#include "nodeVertexTransform.h"
#include "rigidBodyCombiner.h"
#include "pipeOcclusionCullTraverser.h"

#include "dconfig.h"

Configure(config_grutil);
NotifyCategoryDef(grutil, "");

ConfigureFn(config_grutil) {
  init_libgrutil();
}

ConfigVariableDouble frame_rate_meter_update_interval
("frame-rate-meter-update-interval", 1.5);

ConfigVariableString frame_rate_meter_text_pattern
("frame-rate-meter-text-pattern", "%0.1f fps");

ConfigVariableInt frame_rate_meter_layer_sort
("frame-rate-meter-layer-sort", 1000);

ConfigVariableDouble frame_rate_meter_scale
("frame-rate-meter-scale", 0.05);

ConfigVariableDouble frame_rate_meter_side_margins
("frame-rate-meter-side-margins", 0.5);

////////////////////////////////////////////////////////////////////
//     Function: init_libgrutil
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libgrutil() {
  ConfigVariableBool use_movietexture
    ("use-movietexture", false,
     PRC_DESC("Panda contains a new animated texture class, MovieTexture. "
              "Because it is not yet fully tested, the texture loader "
              "will not use it unless this variable is set.  Eventually, "
              "this config variable will go away and the new code will "
              "be enabled all the time."));

  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  FrameRateMeter::init_type();
  MeshDrawer::init_type();
  GeoMipTerrain::init_type();
  NodeVertexTransform::init_type();
  RigidBodyCombiner::init_type();
  PipeOcclusionCullTraverser::init_type();

  MovieTexture::init_type();
  MovieTexture::register_with_read_factory();
#ifdef HAVE_OPENCV
  OpenCVTexture::init_type();
  OpenCVTexture::register_with_read_factory();
#endif
#ifdef HAVE_FFMPEG
  av_register_all();
  FFMpegTexture::init_type();
  FFMpegTexture::register_with_read_factory();
#endif

#ifdef HAVE_OPENCV
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("OpenCV");
#endif

  TexturePool *ts = TexturePool::get_global_ptr();
  if (use_movietexture) {
#if defined(HAVE_FFMPEG)
    ts->register_texture_type(MovieTexture::make_texture, "avi mov mpg wmv asf flv nut ogm");
#elif defined(HAVE_OPENCV)
    ts->register_texture_type(OpenCVTexture::make_texture, "avi");
#endif
  } else {
#if defined(HAVE_FFMPEG)
    ts->register_texture_type(FFMpegTexture::make_texture, "avi mov mpg wmv asf flv nut ogm");
#elif defined(HAVE_OPENCV)
    ts->register_texture_type(OpenCVTexture::make_texture, "avi");
#endif
  }
}

