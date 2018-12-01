/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieTexture.cxx
 * @author jyelon
 * @date 2007-08-01
 */

#include "pandabase.h"

#ifdef HAVE_AUDIO

#include "movieVideo.h"
#include "movieVideoCursor.h"
#include "movieTypeRegistry.h"
#include "movieTexture.h"
#include "clockObject.h"
#include "config_gobj.h"
#include "config_grutil.h"
#include "bamCacheRecord.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "audioSound.h"

#include <math.h>

TypeHandle MovieTexture::_type_handle;

/**
 * Creates a blank movie texture.  Movies must be added using do_read_one or
 * do_load_one.
 */
MovieTexture::
MovieTexture(const std::string &name) :
  Texture(name)
{
}

/**
 * Creates a texture playing the specified movie.
 */
MovieTexture::
MovieTexture(MovieVideo *video) :
  Texture(video->get_name())
{
  Texture::CDWriter cdata_tex(Texture::_cycler, true);
  do_load_one(cdata_tex, video->open(), nullptr, 0, LoaderOptions());
}

/**
 * xxx
 */
MovieTexture::CData::
CData() :
  _video_width(1),
  _video_height(1),
  _video_length(1.0),
  _clock(0.0),
  _playing(false),
  _loop_count(1),
  _play_rate(1.0),
  _has_offset(false)
{
}

/**
 * xxx
 */
MovieTexture::CData::
CData(const CData &copy) :
  _pages(copy._pages),
  _video_width(copy._video_width),
  _video_height(copy._video_height),
  _video_length(copy._video_length),
  _clock(0.0),
  _playing(false),
  _loop_count(1),
  _play_rate(1.0),
  _has_offset(false)
{
}

/**
 * xxx
 */
CycleData *MovieTexture::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * xxx
 */
MovieTexture::
~MovieTexture() {
  clear();
}

/**
 * May be called prior to calling read_txo() or any bam-related Texture-
 * creating callback, to ensure that the proper dynamic libraries for a
 * Texture of the current class type, and the indicated filename, have been
 * already loaded.
 *
 * This is a low-level function that should not normally need to be called
 * directly by the user.
 *
 * Note that for best results you must first create a Texture object of the
 * appropriate class type for your filename, for instance with
 * TexturePool::make_texture().
 */
void MovieTexture::
ensure_loader_type(const Filename &filename) {
  // Creating a MovieVideo of the appropriate type is a slightly hacky way to
  // ensure the appropriate libraries are loaded.  We can let the MovieVideo
  // we create immediately destruct.
  MovieTypeRegistry *reg = MovieTypeRegistry::get_global_ptr();
  PT(MovieVideo) video = reg->make_video(filename);
}

/**
 * A factory function to make a new MovieTexture, used to pass to the
 * TexturePool.
 */
PT(Texture) MovieTexture::
make_texture() {
  return new MovieTexture("");
}

/**
 * Resizes the texture, and adjusts the format, based on the source movies.
 * The resulting texture will be large enough to hold all the videos.
 *
 * Assumes the lock is already held.
 */
void MovieTexture::
do_recalculate_image_properties(CData *cdata, Texture::CData *cdata_tex, const LoaderOptions &options) {
  int x_max = 1;
  int y_max = 1;
  bool rgb = false;
  bool alpha = false;
  double len = 0.0;

  for (size_t i = 0; i < cdata->_pages.size(); ++i) {
    MovieVideoCursor *t = cdata->_pages[i]._color;
    if (t) {
      if (t->size_x() > x_max) x_max = t->size_x();
      if (t->size_y() > y_max) y_max = t->size_y();
      if (t->length() > len) len = t->length();
      if (t->get_num_components() >= 3) rgb=true;
      if (t->get_num_components() == 4 || t->get_num_components() == 2) alpha=true;
    }
    t = cdata->_pages[i]._alpha;
    if (t) {
      if (t->size_x() > x_max) x_max = t->size_x();
      if (t->size_y() > y_max) y_max = t->size_y();
      if (t->length() > len) len = t->length();
      alpha = true;
    }
  }

  cdata->_video_width  = x_max;
  cdata->_video_height = y_max;
  cdata->_video_length = len;

  do_adjust_this_size(cdata_tex, x_max, y_max, get_name(), true);

  int num_components = (rgb ? 3 : 1) + alpha;
  do_reconsider_image_properties(cdata_tex, x_max, y_max, num_components,
                                 T_unsigned_byte, cdata->_pages.size(),
                                 options);
  cdata_tex->_orig_file_x_size = cdata->_video_width;
  cdata_tex->_orig_file_y_size = cdata->_video_height;

  do_set_pad_size(cdata_tex,
                  std::max(cdata_tex->_x_size - cdata_tex->_orig_file_x_size, 0),
                  std::max(cdata_tex->_y_size - cdata_tex->_orig_file_y_size, 0),
                  0);
}

/**
 * Works like adjust_size, but also considers the texture class.  Movie
 * textures, for instance, always pad outwards, never scale down.
 */
bool MovieTexture::
do_adjust_this_size(const Texture::CData *cdata_tex,
                    int &x_size, int &y_size, const std::string &name,
                    bool for_padding) const {
  AutoTextureScale ats = do_get_auto_texture_scale(cdata_tex);
  if (ats != ATS_none) {
    ats = ATS_pad;
  }

  return adjust_size(x_size, y_size, name, for_padding, ats);
}

/**
 * Combines a color and alpha video image from the two indicated filenames.
 * Both must be the same kind of video with similar properties.
 */
bool MovieTexture::
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

  if (record != nullptr) {
    record->add_dependent_file(fullpath);
  }

  PT(MovieVideoCursor) color;
  PT(MovieVideoCursor) alpha;

  color = MovieVideo::get(fullpath)->open();
  if (color == nullptr) {
    return false;
  }
  if (!alpha_fullpath.empty()) {
    alpha = MovieVideo::get(alpha_fullpath)->open();
    if (alpha == nullptr) {
      return false;
    }
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

  if (!do_load_one(cdata_tex, color, alpha, z, options)) {
    return false;
  }

  cdata_tex->_loaded_from_image = true;
  set_loop(true);
  play();
  return true;
}

/**
 * Loads movie objects into the texture.
 */
bool MovieTexture::
do_load_one(Texture::CData *cdata_tex,
            PT(MovieVideoCursor) color, PT(MovieVideoCursor) alpha, int z,
            const LoaderOptions &options) {
  CDWriter cdata(_cycler);
  cdata->_pages.resize(z + 1);
  cdata->_pages[z]._color = color;
  cdata->_pages[z]._alpha = alpha;
  do_recalculate_image_properties(cdata, cdata_tex, options);

  // Make sure the image data is initially black, which is nice for padded
  // textures.
  PTA_uchar image = make_ram_image();
  memset(image.p(), 0, image.size());

  return true;
}

/**
 * Loading a static image into a MovieTexture is an error.
 */
bool MovieTexture::
do_load_one(Texture::CData *cdata_tex,
            const PNMImage &pnmimage, const std::string &name, int z, int n,
            const LoaderOptions &options) {
  grutil_cat.error() << "You cannot load a static image into a MovieTexture\n";
  return false;
}

/**
 * Loading a static image into a MovieTexture is an error.
 */
bool MovieTexture::
do_load_one(Texture::CData *cdata_tex,
            const PfmFile &pfm, const std::string &name, int z, int n,
            const LoaderOptions &options) {
  grutil_cat.error() << "You cannot load a static image into a MovieTexture\n";
  return false;
}

/**
 * Called internally by do_reconsider_z_size() to allocate new memory in
 * _ram_images[0] for the new number of pages.
 *
 * Assumes the lock is already held.
 */
void MovieTexture::
do_allocate_pages(Texture::CData *cdata_tex) {
  // We don't actually do anything here; the allocation is made in
  // do_load_one(), above.
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this node during the cull traversal.
 */
bool MovieTexture::
has_cull_callback() const {
  return true;
}

/**
 * This function will be called during the cull traversal to update the
 * MovieTexture.  This update consists of fetching the next video frame from
 * the underlying MovieVideo sources.  The MovieVideo object belongs to the
 * cull thread.
 */
bool MovieTexture::
cull_callback(CullTraverser *, const CullTraverserData &) const {
  Texture::CDReader cdata_tex(Texture::_cycler);
  CDReader cdata(_cycler);

  if (!cdata->_has_offset) {
    // If we don't have a previously-computed timestamp (offset) cached, then
    // compute a new one.
    double offset;
    int true_loop_count = 1;
    if (cdata->_synchronize != nullptr) {
      offset = cdata->_synchronize->get_time();
    } else {
      // Calculate the cursor position modulo the length of the movie.
      double now = ClockObject::get_global_clock()->get_frame_time();
      offset = cdata->_clock;
      if (cdata->_playing) {
        offset += now * cdata->_play_rate;
      }
      true_loop_count = cdata->_loop_count;
    }
    ((CData *)cdata.p())->_offset = offset;
    ((CData *)cdata.p())->_true_loop_count = true_loop_count;
    ((CData *)cdata.p())->_has_offset = true;
  }

  bool in_sync = do_update_frames(cdata);
  if (!in_sync) {
    // If it didn't successfully sync, try again--once.  The second time it
    // might be able to fill in some more recent frames.
    in_sync = do_update_frames(cdata);
  }

  if (in_sync) {
    // Now go back through and apply all the frames to the texture.
    Pages::const_iterator pi;
    for (pi = cdata->_pages.begin(); pi != cdata->_pages.end(); ++pi) {
      const VideoPage &page = (*pi);
      MovieVideoCursor *color = page._color;
      MovieVideoCursor *alpha = page._alpha;
      size_t i = pi - cdata->_pages.begin();

      if (color != nullptr && alpha != nullptr) {
        color->apply_to_texture_rgb(page._cbuffer, (MovieTexture*)this, i);
        alpha->apply_to_texture_alpha(page._abuffer, (MovieTexture*)this, i, cdata_tex->_alpha_file_channel);

      } else if (color != nullptr) {
        color->apply_to_texture(page._cbuffer, (MovieTexture*)this, i);
      }

      ((VideoPage &)page)._cbuffer.clear();
      ((VideoPage &)page)._abuffer.clear();
    }

    // Clear the cached offset so we can update the frame next time.
    ((CData *)cdata.p())->_has_offset = false;
  }

  return true;
}

/**
 * Returns a new copy of the same Texture.  This copy, if applied to geometry,
 * will be copied into texture as a separate texture from the original, so it
 * will be duplicated in texture memory (and may be independently modified if
 * desired).
 *
 * If the Texture is a MovieTexture, the resulting duplicate may be animated
 * independently of the original.
 */
PT(Texture) MovieTexture::
make_copy_impl() const {
  Texture::CDReader cdata_tex(Texture::_cycler);
  CDReader cdata(_cycler);
  PT(MovieTexture) copy = new MovieTexture(get_name());
  Texture::CDWriter cdata_copy_tex(copy->Texture::_cycler, true);
  CDWriter cdata_copy(copy->_cycler, true);
  copy->do_assign(cdata_copy, cdata_copy_tex, this, cdata, cdata_tex);

  return copy;
}

/**
 * Implements make_copy().
 */
void MovieTexture::
do_assign(CData *cdata, Texture::CData *cdata_tex, const MovieTexture *copy,
          const CData *cdata_copy, const Texture::CData *cdata_copy_tex) {
  Texture::do_assign(cdata_tex, copy, cdata_copy_tex);

  pvector<MovieVideoCursor *> color;
  pvector<MovieVideoCursor *> alpha;
  color.resize(cdata_copy->_pages.size());
  alpha.resize(cdata_copy->_pages.size());
  for (int i=0; i<(int)(color.size()); i++) {
    color[i] = cdata_copy->_pages[i]._color;
    alpha[i] = cdata_copy->_pages[i]._alpha;
  }

  cdata->_pages.resize(color.size());
  for (int i=0; i<(int)(color.size()); i++) {
    if (color[i]) {
      cdata->_pages[i]._color = color[i]->get_source()->open();
    }
    if (alpha[i]) {
      cdata->_pages[i]._alpha = alpha[i]->get_source()->open();
    }
  }
  do_recalculate_image_properties(cdata, cdata_tex, LoaderOptions());
}

/**
 * A MovieTexture must always keep its ram image, since there is no way to
 * reload it from the source MovieVideo.
 */
void MovieTexture::
do_reload_ram_image(Texture::CData *cdata, bool allow_compression) {
  // A MovieTexture should never dump its RAM image.  Therefore, this is not
  // needed.
}

/**
 * A MovieTexture must always keep its ram image, since there is no way to
 * reload it from the source MovieVideo.
 */
bool MovieTexture::
get_keep_ram_image() const {
  // A MovieTexture should never dump its RAM image.
  return true;
}

/**
 * Returns true if there is a rawdata image that we have available to write to
 * the bam stream.  For a normal Texture, this is the same thing as
 * do_has_ram_image(), but a movie texture might define it differently.
 */
bool MovieTexture::
do_has_bam_rawdata(const Texture::CData *cdata) const {
  return true;
}

/**
 * If do_has_bam_rawdata() returned false, this attempts to reload the rawdata
 * image if possible.
 */
void MovieTexture::
do_get_bam_rawdata(Texture::CData *cdata) {
}

/**
 * Returns true if we can safely call do_unlock_and_reload_ram_image() in
 * order to make the image available, or false if we shouldn't do this
 * (because we know from a priori knowledge that it wouldn't work anyway).
 */
bool MovieTexture::
do_can_reload(const Texture::CData *cdata) const {
  return false;
}

/**
 * Start playing the movie from where it was last paused.  Has no effect if
 * the movie is not paused, or if the movie's cursor is already at the end.
 */
void MovieTexture::
restart() {
  CDWriter cdata(_cycler);
  if (!cdata->_playing) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    cdata->_clock = cdata->_clock - (now * cdata->_play_rate);
    cdata->_playing = true;
  }
}

/**
 * Stops a currently playing or looping movie right where it is.  The movie's
 * cursor remains frozen at the point where it was stopped.
 */
void MovieTexture::
stop() {
  CDWriter cdata(_cycler);
  if (cdata->_playing) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    cdata->_clock = cdata->_clock + (now * cdata->_play_rate);
    cdata->_playing = false;
  }
}

/**
 * Plays the movie from the beginning.
 */
void MovieTexture::
play() {
  CDWriter cdata(_cycler);
  double now = ClockObject::get_global_clock()->get_frame_time();
  cdata->_clock = 0.0 - (now * cdata->_play_rate);
  cdata->_playing = true;
}

/**
 * Sets the movie's cursor.
 */
void MovieTexture::
set_time(double t) {
  CDWriter cdata(_cycler);
  t = std::min(cdata->_video_length, std::max(0.0, t));
  if (cdata->_playing) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    cdata->_clock = t - (now * cdata->_play_rate);
  } else {
    cdata->_clock = t;
  }
}

/**
 * Returns the current value of the movie's cursor.  If the movie's loop count
 * is greater than one, then its length is effectively multiplied for the
 * purposes of this function.  In other words, the return value will be in the
 * range 0.0 to (length * loopcount).
 */
double MovieTexture::
get_time() const {
  CDReader cdata(_cycler);
  double clock = cdata->_clock;
  if (cdata->_playing) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    clock += (now * cdata->_play_rate);
  }
  return clock;
}

/**
 * If true, sets the movie's loop count to 1 billion.  If false, sets the
 * movie's loop count to one.
 */
void MovieTexture::
set_loop(bool loop) {
  set_loop_count(loop ? 0:1);
}

/**
 * Returns true if the movie's loop count is not equal to one.
 */
bool MovieTexture::
get_loop() const {
  CDReader cdata(_cycler);
  return (cdata->_loop_count == 0);
}

/**
 * Sets the movie's loop count to the desired value.
 */
void MovieTexture::
set_loop_count(int n) {
  CDWriter cdata(_cycler);
  cdata->_loop_count = n;
}

/**
 * Returns the movie's loop count.
 */
int MovieTexture::
get_loop_count() const {
  CDReader cdata(_cycler);
  return cdata->_loop_count;
}

/**
 * Sets the movie's play-rate.  This is the speed at which the movie's cursor
 * advances.  The default is to advance 1.0 movie-seconds per real-time
 * second.
 */
void MovieTexture::
set_play_rate(double rate) {
  CDWriter cdata(_cycler);
  if (cdata->_playing) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    cdata->_clock += (now * cdata->_play_rate);
    cdata->_play_rate = rate;
    cdata->_clock -= (now * cdata->_play_rate);
  } else {
    cdata->_play_rate = rate;
  }
}

/**
 * Gets the movie's play-rate.
 */
double MovieTexture::
get_play_rate() const {
  CDReader cdata(_cycler);
  return cdata->_play_rate;
}

/**
 * Returns true if the movie's cursor is advancing.
 */
bool MovieTexture::
is_playing() const {
  CDReader cdata(_cycler);
  return cdata->_playing;
}

/**
 * Synchronize this texture to a sound.  Typically, you would load the texture
 * and the sound from the same AVI file.
 */
void MovieTexture::
synchronize_to(AudioSound *s) {
  CDWriter cdata(_cycler);
  cdata->_synchronize = s;
}

/**
 * Stop synchronizing with a sound.
 */
void MovieTexture::
unsynchronize() {
  CDWriter cdata(_cycler);
  cdata->_synchronize = nullptr;
}


/**
 * Called internally to sync all of the frames to the current time.  Returns
 * true if successful, or false of some of the frames are out-of-date with
 * each other.
 */
bool MovieTexture::
do_update_frames(const CData *cdata) const {
  // Throughout this method, we cast the VideoPage to non-const to update the
  // _cbuffer or _abuffer member.  We can do this safely because this is only
  // a transparent cache value.
  nassertr(cdata->_has_offset, false);

  // First, go through and get all of the current frames.
  Pages::const_iterator pi;
  for (pi = cdata->_pages.begin(); pi != cdata->_pages.end(); ++pi) {
    const VideoPage &page = (*pi);
    MovieVideoCursor *color = page._color;
    MovieVideoCursor *alpha = page._alpha;

    if (color != nullptr && page._cbuffer == nullptr) {
      if (color->set_time(cdata->_offset, cdata->_true_loop_count)) {
        ((VideoPage &)page)._cbuffer = color->fetch_buffer();
      }
    }
    if (alpha != nullptr && page._abuffer == nullptr) {
      if (alpha->set_time(cdata->_offset, cdata->_true_loop_count)) {
        ((VideoPage &)page)._abuffer = alpha->fetch_buffer();
      }
    }
  }

  if (!movies_sync_pages) {
    // If movies-sync-pages is configured off, we don't care about syncing the
    // pages, and we always return true here to render the pages we've got.
    return true;
  }

  // Now make sure all of the frames are in sync with each other.
  bool in_sync = true;
  bool any_frames = false;
  bool any_dropped = false;
  PT(MovieVideoCursor::Buffer) newest;
  for (pi = cdata->_pages.begin(); pi != cdata->_pages.end(); ++pi) {
    const VideoPage &page = (*pi);
    if (page._cbuffer == nullptr) {
      if (page._color != nullptr) {
        // This page isn't ready at all.
        in_sync = false;
      }
    } else {
      nassertr(page._color != nullptr, true);
      any_frames = true;
      if (newest == nullptr) {
        newest = page._cbuffer;
      } else {
        int ref = newest->compare_timestamp(page._cbuffer);
        if (ref != 0) {
          // This page is ready, but out-of-date.
          in_sync = false;
          any_dropped = true;
          if (ref < 0) {
            newest = page._cbuffer;
          }
        }
      }
    }
    if (page._abuffer == nullptr) {
      if (page._alpha != nullptr) {
        in_sync = false;
      }
    } else {
      nassertr(page._alpha != nullptr, true);
      any_frames = true;
      if (newest == nullptr) {
        newest = page._abuffer;
      } else {
        int ref = newest->compare_timestamp(page._abuffer);
        if (ref != 0) {
          in_sync = false;
          any_dropped = true;
          if (ref < 0) {
            newest = page._abuffer;
          }
        }
      }
    }
  }

  if (!any_frames) {
    // If no frames at all are ready yet, just carry on.
    return true;
  }

  if (!in_sync) {
    // If we're not in sync, throw away pages that are older than the newest
    // available frame.
    if (newest != nullptr) {
      Pages::const_iterator pi;
      for (pi = cdata->_pages.begin(); pi != cdata->_pages.end(); ++pi) {
        const VideoPage &page = (*pi);
        if (page._cbuffer != nullptr && newest->compare_timestamp(page._cbuffer) > 0) {
          ((VideoPage &)page)._cbuffer.clear();
          any_dropped = true;
        }
        if (page._abuffer != nullptr && newest->compare_timestamp(page._abuffer) > 0) {
          ((VideoPage &)page)._abuffer.clear();
          any_dropped = true;
        }
      }

      if (any_dropped) {
        // If we dropped one or more frames for being out-of-sync, implying
        // that compare_timestamp() is implemented, then we also want to
        // update our internal offset value so that future frames will get the
        // same value.
        ((CData *)cdata)->_offset = newest->get_timestamp();
      }
    }
  }

  return in_sync;
}

/**
 * Factory method to generate a Texture object
 */
void MovieTexture::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Factory method to generate a MovieTexture object
 */
TypedWritable *MovieTexture::
make_from_bam(const FactoryParams &params) {
  PT(MovieTexture) dummy = new MovieTexture("");
  return dummy->make_this_from_bam(params);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int MovieTexture::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = Texture::complete_pointers(p_list, manager);

  CDWriter cdata(_cycler);
  size_t num_pages = cdata->_pages.size();
  for (size_t n = 0; n < num_pages; ++n) {
    VideoPage &page = cdata->_pages[n];
    page._color = DCAST(MovieVideoCursor, p_list[pi++]);
    page._alpha = DCAST(MovieVideoCursor, p_list[pi++]);
  }

  return pi;
}

/**
 * Writes the rawdata part of the texture to the Datagram.
 */
void MovieTexture::
do_write_datagram_rawdata(Texture::CData *cdata_tex, BamWriter *manager, Datagram &dg) {
  CDReader cdata(_cycler);

  dg.add_uint16(cdata_tex->_z_size);
  if (manager->get_file_minor_ver() >= 26) {
    dg.add_uint16(cdata_tex->_num_views);
  }

  nassertv(cdata->_pages.size() == (size_t)(cdata_tex->_z_size * cdata_tex->_num_views));
  for (size_t n = 0; n < cdata->_pages.size(); ++n) {
    const VideoPage &page = cdata->_pages[n];
    manager->write_pointer(dg, page._color);
    manager->write_pointer(dg, page._alpha);
  }
}

/**
 * Reads in the part of the Texture that was written with
 * do_write_datagram_rawdata().
 */
void MovieTexture::
do_fillin_rawdata(Texture::CData *cdata_tex, DatagramIterator &scan, BamReader *manager) {
  CDWriter cdata(_cycler);

  cdata_tex->_z_size = scan.get_uint16();
  cdata_tex->_num_views = 1;
  if (manager->get_file_minor_ver() >= 26) {
    cdata_tex->_num_views = scan.get_uint16();
  }

  size_t num_pages = (size_t)(cdata_tex->_z_size * cdata_tex->_num_views);
  cdata->_pages.reserve(num_pages);
  for (size_t n = 0; n < num_pages; ++n) {
    cdata->_pages.push_back(VideoPage());
    manager->read_pointer(scan);  // page._color
    manager->read_pointer(scan);  // page._alpha
  }

  // We load one or more MovieVideoCursors during the above loop.  We need a
  // finalize callback so we can initialize ourselves once those cursors have
  // been read completely.
  manager->register_finalize(this);
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void MovieTexture::
finalize(BamReader *manager) {
  Texture::CDWriter cdata_tex(Texture::_cycler);
  CDWriter cdata(_cycler);

  // Insist that each of our video pages gets finalized before we do.
  size_t num_pages = cdata->_pages.size();
  for (size_t n = 0; n < num_pages; ++n) {
    VideoPage &page = cdata->_pages[n];
    manager->finalize_now(page._color);
    manager->finalize_now(page._alpha);
  }

  do_recalculate_image_properties(cdata, cdata_tex, LoaderOptions());

  set_loaded_from_image();
  set_loop(true);
  play();
}

#endif  // HAVE_AUDIO
