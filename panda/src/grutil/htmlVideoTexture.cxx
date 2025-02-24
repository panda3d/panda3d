/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file htmlVideoTexture.cxx
 * @author rdb
 * @date 2025-02-24
 */

#include "htmlVideoTexture.h"

#ifdef __EMSCRIPTEN__

#include "config_grutil.h"
#include "virtualFileSystem.h"
#include "virtualFileHTTP.h"

#include <emscripten/emscripten.h>

#ifndef CPPPARSER
// If the browser supports video frame callback, we use that instead of relying
// on cull callbacks.
extern "C" void EMSCRIPTEN_KEEPALIVE
_video_frame_callback(HTMLVideoTexture *texture, int width, int height, double time) {
  texture->video_frame_callback(width, height, time);
}

static bool get_supports_video_frame_callback() {
  static bool supported = EM_ASM_INT({
    return 'requestVideoFrameCallback' in HTMLVideoElement.prototype;
  });
  return supported;
}
#endif

TypeHandle HTMLVideoTexture::_type_handle;

/**
 * Creates a blank movie texture.  Movies must be added using do_read_one.
 */
HTMLVideoTexture::
HTMLVideoTexture(const std::string &name) :
  Texture(name)
{
  EM_ASM_INT({
    if (!window._htmlVideoData) {
      window._htmlVideoData = {};
    }
    var video = document.createElement("video");
    video.style.display = 'none';
    video.defaultMuted = true;
    document.body.appendChild(video);
    window._htmlVideoData[$0] = {video: video};
  }, this);
}

/**
 * xxx
 */
HTMLVideoTexture::
~HTMLVideoTexture() {
  clear();

  EM_ASM_INT({
    var data = window._htmlVideoData[$0];
    if (data) {
      if (data.video) {
        if (data.callback) {
          video.cancelVideoFrameCallback(data.callback);
          delete data.callback;
        }
        data.video.remove();
        delete data.video;
      }
      if (data.objectURL) {
        URL.revokeObjectURL(data.objectURL);
        delete data.objectURL;
      }
      delete window._htmlVideoData[$0];
    }
  }, this);
}

/**
 * Returns the length of the video.
 */
double HTMLVideoTexture::
get_video_length() const {
  return EM_ASM_DOUBLE({
    return window._htmlVideoData[$0].video.duration;
  }, this);
}

/**
 * Returns the width in texels of the source video stream.  This is not
 * necessarily the width of the actual texture, since the texture may have
 * been expanded to raise it to a power of 2.
 */
int HTMLVideoTexture::
get_video_width() const {
  return EM_ASM_DOUBLE({
    return window._htmlVideoData[$0].video.videoWidth;
  }, this);
}

/**
 * Returns the height in texels of the source video stream.  This is not
 * necessarily the height of the actual texture, since the texture may have
 * been expanded to raise it to a power of 2.
 */
int HTMLVideoTexture::
get_video_height() const {
  return EM_ASM_DOUBLE({
    return window._htmlVideoData[$0].video.videoHeight;
  }, this);
}

/**
 * Start playing the video from where it was last paused.  Has no effect if
 * the video is not paused, or if the video's cursor is already at the end.
 */
void HTMLVideoTexture::
restart() {
  EM_ASM({
    window._htmlVideoData[$0].video.play();
  }, this);
}

/**
 * Stops a currently playing or looping movie right where it is.  the video's
 * cursor remains frozen at the point where it was stopped.
 */
void HTMLVideoTexture::
stop() {
  EM_ASM({
    window._htmlVideoData[$0].video.pause();
  }, this);
}

/**
 * Plays the video from the beginning.
 */
void HTMLVideoTexture::
play() {
  EM_ASM({
    var video = window._htmlVideoData[$0].video;
    if (video.playing) {
      video.pause();
    }
    video.currentTime = 0.0;
    video.play();
  }, this);
}

/**
 * Sets the video's cursor.
 */
void HTMLVideoTexture::
set_time(double t) {
  EM_ASM({
    window._htmlVideoData[$0].video.currentTime = $1;
  }, this, t);
}

/**
 * Returns the current value of the video's cursor.
 */
double HTMLVideoTexture::
get_time() const {
  return EM_ASM_DOUBLE({
    return window._htmlVideoData[$0].video.currentTime;
  }, this);
}

/**
 * If true, sets the video's loop count to 1 billion.  If false, sets the
 * movie's loop count to one.
 */
void HTMLVideoTexture::
set_loop(bool loop) {
  EM_ASM({
    window._htmlVideoData[$0].video.loop = $1;
  }, this, loop);
}

/**
 * Returns true if the video's is looping.
 */
bool HTMLVideoTexture::
get_loop() const {
  return EM_ASM_INT({
    return window._htmlVideoData[$0].video.loop;
  }, this);
}

/**
 * Sets the video's play-rate.  This is the speed at which the video's cursor
 * advances.  The default is to advance 1.0 movie-seconds per real-time
 * second.
 */
void HTMLVideoTexture::
set_play_rate(double rate) {
  EM_ASM({
    window._htmlVideoData[$0].video.playbackRate = $1;
  }, this, rate);
}

/**
 * Gets the video's play-rate.
 */
double HTMLVideoTexture::
get_play_rate() const {
  return EM_ASM_DOUBLE({
    return window._htmlVideoData[$0].video.playbackRate;
  }, this);
}

/**
 * Returns true if the video's cursor is advancing.
 */
bool HTMLVideoTexture::
is_playing() const {
  return EM_ASM_INT({
    return !window._htmlVideoData[$0].video.paused;
  }, this);
}

/**
 * A factory function to make a new HTMLVideoTexture, used to pass to the
 * TexturePool.
 */
PT(Texture) HTMLVideoTexture::
make_texture() {
  return new HTMLVideoTexture("");
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this node during the cull traversal.
 */
bool HTMLVideoTexture::
has_cull_callback() const {
  return !get_supports_video_frame_callback();
}

/**
 * This function will be called during the cull traversal to update the
 * HTMLVideoTexture.  This will just check whether the video time changed since
 * the last time it was checked.
 */
bool HTMLVideoTexture::
cull_callback(CullTraverser *, const CullTraverserData &) const {
  ((HTMLVideoTexture *)this)->check_update();
  return true;
}

/**
 *
 */
void HTMLVideoTexture::
video_frame_callback(int width, int height, double time) {
  CDWriter cdata(_cycler, true);
  if (width != cdata->_x_size || height != cdata->_y_size) {
    cdata->_x_size = width;
    cdata->_y_size = height;
    cdata->inc_properties_modified();
  }
  cdata->inc_image_modified();
  _last_time = time;
}

/**
 *
 */
bool HTMLVideoTexture::
check_update() {
  double current_time = get_time();
  if (current_time != _last_time) {
    int width = get_video_width();
    int height = get_video_height();
    if (width == 0 && height == 0) {
      return false;
    }

    video_frame_callback(width, height, current_time);
    return true;
  }
  return false;
}

/**
 * The protected implementation of clear().  Assumes the lock is already held.
 */
void HTMLVideoTexture::
do_clear(Texture::CData *cdata) {
  EM_ASM_INT({
    var data = window._htmlVideoData[$0];
    data.video.pause();
    data.video.src = "";
    if (data.callback) {
      data.video.cancelVideoFrameCallback(data.callback);
      delete data.callback;
    }
    if (data.objectURL) {
      URL.revokeObjectURL(data.objectURL);
      delete data.objectURL;
    }
  }, this);

  Texture::do_clear(cdata);
}

/**
 * Returns true if we can safely call do_unlock_and_reload_ram_image() in
 * order to make the image available, or false if we shouldn't do this
 * (because we know from a priori knowledge that it wouldn't work anyway).
 */
bool HTMLVideoTexture::
do_can_reload(const Texture::CData *cdata) const {
  return false;
}

/**
 * Works like adjust_size, but also considers the texture class.  Movie
 * textures, for instance, always pad outwards, never scale down.
 */
bool HTMLVideoTexture::
do_adjust_this_size(const Texture::CData *cdata_tex,
                    int &x_size, int &y_size, const std::string &name,
                    bool for_padding) const {
  // We always scale, for now.  May change in the future.
  AutoTextureScale ats = do_get_auto_texture_scale(cdata_tex);
  if (ats == ATS_pad) {
    ats = ATS_up;
  }

  return adjust_size(x_size, y_size, name, for_padding, ats);
}

/**
 * Combines a color and alpha video image from the two indicated filenames.
 * Both must be the same kind of video with similar properties.
 */
bool HTMLVideoTexture::
do_read_one(Texture::CData *cdata_tex,
            const Filename &fullpath, const Filename &alpha_fullpath,
            int z, int n, int primary_file_num_channels, int alpha_file_channel,
            const LoaderOptions &options,
            bool header_only, BamCacheRecord *record) {
  nassertr(n == 0, false);
  if (!do_reconsider_z_size(cdata_tex, z, options)) {
    return false;
  }
  nassertr(z >= 0 && z < cdata_tex->_z_size * cdata_tex->_num_views, false);

  if (!alpha_fullpath.empty()) {
    grutil_cat.error()
      << "HTMLVideoTexture does not support loading separate alpha video.\n";
    return false;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vfile = vfs->get_file(fullpath, true);
  if (vfile == nullptr) {
    return false;
  }

  clear();

  if (record != nullptr) {
    record->add_dependent_file(vfile);
  }

  // Pass in the URL directly if it's an HTTP file.
  if (vfile->is_of_type(VirtualFileHTTP::get_class_type())) {
    URLSpec url = ((VirtualFileHTTP *)vfile.p())->get_url();

    _last_time = EM_ASM_DOUBLE({
      var data = window._htmlVideoData[$0];
      var video = data.video;
      video.src = UTF8ToString($1);
      video.loop = true;
      video.muted = true;

      video.load();
      return video.currentTime;
    }, this, url.c_str());
  }
  else {
    vector_uchar data;
    if (!vfile->read_file(data, true)) {
      return false;
    }

    _last_time = EM_ASM_DOUBLE({
      var data = window._htmlVideoData[$0];

      var blob = new Blob([HEAPU8.subarray($1, $1 + $2)]);
      data.objectURL = URL.createObjectURL(blob);

      var video = data.video;
      video.src = data.objectURL;
      video.loop = true;
      video.muted = true;

      video.load();
      return video.currentTime;
    }, this, (void *)data.data(), data.size());
  }

  // If the browser supports requestVideoframeCallback, we set up a callback to
  // know when the next video frame is available instead of having to poll in
  // cull_callback().
  if (get_supports_video_frame_callback()) {
    EM_ASM({
      var data = window._htmlVideoData[$0];

      function callback(now, metadata) {
        __video_frame_callback($0, metadata.width, metadata.height, metadata.mediaTime);
        data.callback = data.video.requestVideoFrameCallback(callback);
      }
      data.callback = data.video.requestVideoFrameCallback(callback);
    }, this);
  }

  int width = get_video_width();
  int height = get_video_height();
  if (width == 0 || height == 0) {
    // Not yet known.  Don't set the size to 0 since that will cause
    // CardMaker::set_uv_range() to generate invalid coordinates.
    width = 1;
    height = 1;
  }

  if (!do_reconsider_image_properties(cdata_tex, width, height, 3, T_unsigned_byte, z, options)) {
    return false;
  }

  if (z == 0) {
    if (!has_name()) {
      set_name(fullpath.get_basename_wo_extension());
    }
    // Don't use has_filename() here, it will cause a deadlock
    if (cdata_tex->_filename.empty()) {
      cdata_tex->_filename = fullpath;
      cdata_tex->_alpha_filename = alpha_fullpath;
    }

    cdata_tex->_fullpath = fullpath;
    cdata_tex->_alpha_fullpath = alpha_fullpath;
  }

  cdata_tex->_primary_file_num_channels = primary_file_num_channels;
  cdata_tex->_alpha_file_channel = alpha_file_channel;

  cdata_tex->_loaded_from_image = true;
  play();
  check_update();
  return true;
}

/**
 * Loading a static image into an HTMLVideoTexture is an error.
 */
bool HTMLVideoTexture::
do_load_one(Texture::CData *cdata_tex,
            const PNMImage &pnmimage, const std::string &name, int z, int n,
            const LoaderOptions &options) {
  grutil_cat.error() << "You cannot load a static image into an HTMLVideoTexture\n";
  return false;
}

/**
 * Loading a static image into an HTMLVideoTexture is an error.
 */
bool HTMLVideoTexture::
do_load_one(Texture::CData *cdata_tex,
            const PfmFile &pfm, const std::string &name, int z, int n,
            const LoaderOptions &options) {
  grutil_cat.error() << "You cannot load a static image into an HTMLVideoTexture\n";
  return false;
}

#endif  // __EMSCRIPTEN__
