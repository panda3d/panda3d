/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file openCVTexture.cxx
 * @author zacpavlov
 * @date 2005-08-19
 */

#include "pandabase.h"

#ifdef HAVE_OPENCV
#include "openCVTexture.h"
#include "clockObject.h"
#include "config_gobj.h"
#include "config_vision.h"
#include "bamReader.h"
#include "bamCacheRecord.h"

// This symbol is predefined by the Panda3D build system to select whether we
// are using the OpenCV 3.x or later interface.
#if defined(OPENCV_VER_3)

#include <opencv2/core.hpp>
#include <opencv2/videoio/videoio_c.h>

// This checks for 2.3 or later.
#elif defined(OPENCV_VER_23)

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// If neither of those are predefined, assume 1.x.
#else

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#endif  // OPENCV_VER_3

TypeHandle OpenCVTexture::_type_handle;

/**
 * Sets up the texture to read frames from a camera
 */
OpenCVTexture::
OpenCVTexture(const std::string &name) :
  VideoTexture(name)
{
}

/**
 *
 */
OpenCVTexture::
~OpenCVTexture() {
}

/**
 * Calls update_frame() if the current frame has changed.
 */
void OpenCVTexture::
consider_update() {
  int this_frame = ClockObject::get_global_clock()->get_frame_count();
  if (this_frame != _last_frame_update) {
    int frame = get_frame();
    if (_current_frame != frame) {
      Texture::CDWriter cdata(Texture::_cycler, false);
      do_update_frame(cdata, frame);
      _current_frame = frame;
    } else {
      // Loop through the pages to see if there's any camera stream to update.
      Texture::CDWriter cdata(Texture::_cycler, false);
      int max_z = std::max(cdata->_z_size, (int)_pages.size());
      for (int z = 0; z < max_z; ++z) {
        VideoPage &page = _pages[z];
        if (!page._color.is_from_file() || !page._alpha.is_from_file()) {
          do_update_frame(cdata, frame, z);
        }
      }
    }
    _last_frame_update = this_frame;
  }
}

/**
 * Returns a new copy of the same Texture.  This copy, if applied to geometry,
 * will be copied into texture as a separate texture from the original, so it
 * will be duplicated in texture memory (and may be independently modified if
 * desired).
 *
 * If the Texture is an OpenCVTexture, the resulting duplicate may be animated
 * independently of the original.
 */
PT(Texture) OpenCVTexture::
make_copy_impl() const {
  Texture::CDReader cdata_tex(Texture::_cycler);
  PT(OpenCVTexture) copy = new OpenCVTexture(get_name());
  Texture::CDWriter cdata_copy_tex(copy->Texture::_cycler, true);
  copy->do_assign(cdata_copy_tex, this, cdata_tex);

  return copy;
}

/**
 * Implements make_copy().
 */
void OpenCVTexture::
do_assign(Texture::CData *cdata_tex, const OpenCVTexture *copy,
          const Texture::CData *cdata_copy_tex) {
  VideoTexture::do_assign(cdata_tex, copy, cdata_copy_tex);
  _pages = copy->_pages;
}

/**
 * Sets up the OpenCVTexture (or the indicated page, if z is specified) to
 * accept its input from the camera with the given index number, or the
 * default camera if the index number is -1 or unspecified.
 *
 * If alpha_file_channel is 0, then the camera image becomes a normal RGB
 * texture.  If it is 1, 2, or 3, then the camera image becomes an alpha
 * texture, using the indicated channel of the source.
 */
bool OpenCVTexture::
from_camera(int camera_index, int z, int alpha_file_channel,
            const LoaderOptions &options) {
  Texture::CDWriter cdata(Texture::_cycler, true);
  if (!do_reconsider_z_size(cdata, z, options)) {
    return false;
  }
  nassertr(z >= 0 && z < cdata->_z_size, false);

  cdata->_alpha_file_channel = alpha_file_channel;

  VideoPage &page = do_modify_page(cdata, z);
  if (alpha_file_channel == 0) {
    // A normal RGB texture.
    page._alpha.clear();
    if (!page._color.from_camera(camera_index)) {
      return false;
    }

    if (!do_reconsider_video_properties(cdata, page._color, 3, z, options)) {
      page._color.clear();
      return false;
    }
  } else {
    // An alpha texture.
    page._color.clear();
    if (!page._alpha.from_camera(camera_index)) {
      return false;
    }

    if (!do_reconsider_video_properties(cdata, page._alpha, 1, z, options)) {
      page._alpha.clear();
      return false;
    }
    do_set_format(cdata, F_alpha);
  }

  cdata->_loaded_from_image = true;
  clear_current_frame();
  do_update_frame(cdata, 0);
  return true;
}

/**
 * Returns a reference to the zth VideoPage (level) of the texture.  In the
 * case of a 2-d texture, there is only one page, level 0; but cube maps and
 * 3-d textures have more.
 */
OpenCVTexture::VideoPage &OpenCVTexture::
do_modify_page(const Texture::CData *cdata, int z) {
  nassertr(z < cdata->_z_size, _pages[0]);
  while (z >= (int)_pages.size()) {
    _pages.push_back(VideoPage());
  }
  return _pages[z];
}

/**
 * Resets the internal Texture properties when a new video file is loaded.
 * Returns true if the new image is valid, false otherwise.
 */
bool OpenCVTexture::
do_reconsider_video_properties(Texture::CData *cdata,
                               const OpenCVTexture::VideoStream &stream,
                               int num_components, int z,
                               const LoaderOptions &options) {
  double frame_rate = 0.0f;
  int num_frames = 0;

  if (stream.is_from_file()) {
    frame_rate = cvGetCaptureProperty(stream._capture, CV_CAP_PROP_FPS);
    num_frames = (int)cvGetCaptureProperty(stream._capture, CV_CAP_PROP_FRAME_COUNT);
    if (vision_cat.is_debug()) {
      vision_cat.debug()
        << "Loaded " << stream._filename << ", " << num_frames << " frames at "
        << frame_rate << " fps\n";
    }
  } else {
    // In this case, we don't have a specific frame rate or number of frames.
    // Let both values remain at 0.
    if (vision_cat.is_debug()) {
      vision_cat.debug()
        << "Loaded camera stream\n";
    }
  }

  int width = (int)cvGetCaptureProperty(stream._capture, CV_CAP_PROP_FRAME_WIDTH);
  int height = (int)cvGetCaptureProperty(stream._capture, CV_CAP_PROP_FRAME_HEIGHT);

  int x_size = width;
  int y_size = height;
  do_adjust_this_size(cdata, x_size, y_size, get_name(), true);

  if (vision_cat.is_debug()) {
    vision_cat.debug()
      << "Video stream is " << width << " by " << height
      << " pixels; fitting in texture " << x_size << " by "
      << y_size << " texels.\n";
  }

  if (!do_reconsider_image_properties(cdata, x_size, y_size, num_components,
                                      T_unsigned_byte, z, options)) {
    return false;
  }

  if (cdata->_loaded_from_image &&
      (get_video_width() != width || get_video_height() != height ||
       get_num_frames() != num_frames || get_frame_rate() != frame_rate)) {
    vision_cat.error()
      << "Video properties have changed for texture " << get_name()
      << " level " << z << ".\n";
    return false;
  }

  set_frame_rate(frame_rate);
  set_num_frames(num_frames);
  set_video_size(width, height);

  // By default, the newly-loaded video stream will immediately start looping.
  loop(true);

  return true;
}

/**
 * A factory function to make a new OpenCVTexture, used to pass to the
 * TexturePool.
 */
PT(Texture) OpenCVTexture::
make_texture() {
  return new OpenCVTexture;
}

/**
 * Called once per frame, as needed, to load the new image contents.
 */
void OpenCVTexture::
do_update_frame(Texture::CData *cdata, int frame) {
  int max_z = std::max(cdata->_z_size, (int)_pages.size());
  for (int z = 0; z < max_z; ++z) {
    do_update_frame(cdata, frame, z);
  }
}

/**
 * This variant of update_frame updates the indicated page only.
 */
void OpenCVTexture::
do_update_frame(Texture::CData *cdata, int frame, int z) {
  if (vision_cat.is_spam()) {
    vision_cat.spam()
      << "Updating OpenCVTexture page " << z << "\n";
  }

  VideoPage &page = _pages[z];
  if (page._color.is_valid() || page._alpha.is_valid()) {
    do_modify_ram_image(cdata);
    ++(cdata->_image_modified);
  }
  int dest_x_pitch = cdata->_num_components * cdata->_component_width;
  int dest_y_pitch = cdata->_x_size * dest_x_pitch;

  if (page._color.is_valid()) {
    nassertv(get_num_components() >= 3 && get_component_width() == 1);

    const unsigned char *r, *g, *b;
    int x_pitch, y_pitch;
    if (page._color.get_frame_data(frame, r, g, b, x_pitch, y_pitch)) {
      nassertv(get_video_width() <= cdata->_x_size && get_video_height() <= cdata->_y_size);
      nassertv(!cdata->_ram_images.empty())
      unsigned char *dest = cdata->_ram_images[0]._image.p() + do_get_expected_ram_page_size(cdata) * z;

      if (cdata->_num_components == 3 && x_pitch == 3) {
        // The easy case--copy the whole thing in, row by row.
        int copy_bytes = get_video_width() * dest_x_pitch;
        nassertv(copy_bytes <= dest_y_pitch && copy_bytes <= abs(y_pitch));

        for (int y = 0; y < get_video_height(); ++y) {
          memcpy(dest, r, copy_bytes);
          dest += dest_y_pitch;
          r += y_pitch;
        }

      } else {
        // The harder case--interleave in the color channels, pixel by pixel,
        // possibly leaving room for alpha.

        for (int y = 0; y < get_video_height(); ++y) {
          int dx = 0;
          int sx = 0;
          for (int x = 0; x < get_video_width(); ++x) {
            dest[dx] = r[sx];
            dest[dx + 1] = g[sx];
            dest[dx + 2] = b[sx];
            dx += dest_x_pitch;
            sx += x_pitch;
          }
          dest += dest_y_pitch;
          r += y_pitch;
          g += y_pitch;
          b += y_pitch;
        }
      }
    }
  }
  if (page._alpha.is_valid()) {
    nassertv(get_component_width() == 1);

    const unsigned char *source[3];
    int x_pitch, y_pitch;
    if (page._alpha.get_frame_data(frame, source[0], source[1], source[2],
                                   x_pitch, y_pitch)) {
      nassertv(get_video_width() <= cdata->_x_size && get_video_height() <= cdata->_y_size);
      nassertv(!cdata->_ram_images.empty())
      unsigned char *dest = cdata->_ram_images[0]._image.p() + do_get_expected_ram_page_size(cdata) * z;

      // Interleave the alpha in with the color, pixel by pixel.  Even though
      // the alpha will probably be a grayscale video, the OpenCV library
      // presents it as RGB.
      const unsigned char *sch = source[0];
      if (cdata->_alpha_file_channel >= 1 && cdata->_alpha_file_channel <= 3) {
        sch = source[cdata->_alpha_file_channel - 1];
      }

      for (int y = 0; y < get_video_height(); ++y) {
        // Start dx at _num_components - 1, which writes to the last channel,
        // i.e.  the alpha channel.
        int dx = (cdata->_num_components - 1) * cdata->_component_width;
        int sx = 0;
        for (int x = 0; x < get_video_width(); ++x) {
          dest[dx] = sch[sx];
          dx += dest_x_pitch;
          sx += x_pitch;
        }
        dest += dest_y_pitch;
        sch += y_pitch;
      }
    }
  }
}

/**
 * Combines a color and alpha video image from the two indicated filenames.
 * Both must be the same kind of video with similar properties.
 */
bool OpenCVTexture::
do_read_one(Texture::CData *cdata,
            const Filename &fullpath, const Filename &alpha_fullpath,
            int z, int n, int primary_file_num_channels, int alpha_file_channel,
            const LoaderOptions &options,
            bool header_only, BamCacheRecord *record) {
  if (record != nullptr) {
    record->add_dependent_file(fullpath);
  }

  nassertr(n == 0, false);
  nassertr(z >= 0 && z < cdata->_z_size, false);

  VideoPage &page = do_modify_page(cdata, z);
  if (!page._color.read(fullpath)) {
    vision_cat.error()
      << "OpenCV couldn't read " << fullpath << " as video.\n";
    return false;
  }
  if (!alpha_fullpath.empty()) {
    if (!page._alpha.read(alpha_fullpath)) {
      vision_cat.error()
        << "OpenCV couldn't read " << alpha_fullpath << " as video.\n";
      page._color.clear();
      return false;
    }
  }

  if (z == 0) {
    if (!has_name()) {
      set_name(fullpath.get_basename_wo_extension());
    }
    // Don't use has_filename() here, it will cause a deadlock
    if (cdata->_filename.empty()) {
      cdata->_filename = fullpath;
      cdata->_alpha_filename = alpha_fullpath;
    }

    cdata->_fullpath = fullpath;
    cdata->_alpha_fullpath = alpha_fullpath;
  }

  cdata->_primary_file_num_channels = 3;
  cdata->_alpha_file_channel = 0;

  if (alpha_fullpath.empty()) {
    // Only one RGB movie.
    if (!do_reconsider_video_properties(cdata, page._color, 3, z, options)) {
      page._color.clear();
      return false;
    }

  } else {
    // An RGB movie combined with an alpha movie.
    cdata->_alpha_file_channel = alpha_file_channel;

    if (!do_reconsider_video_properties(cdata, page._color, 4, z, options)) {
      page._color.clear();
      page._alpha.clear();
      return false;
    }

    if (!do_reconsider_video_properties(cdata, page._alpha, 4, z, options)) {
      page._color.clear();
      page._alpha.clear();
      return false;
    }
  }

  set_loaded_from_image();
  clear_current_frame();
  do_update_frame(cdata, 0);
  return true;
}

/**
 * Resets the texture (or the particular level of the texture) to the
 * indicated static image.
 */
bool OpenCVTexture::
do_load_one(Texture::CData *cdata,
            const PNMImage &pnmimage, const std::string &name,
            int z, int n, const LoaderOptions &options) {
  if (z <= (int)_pages.size()) {
    VideoPage &page = do_modify_page(cdata, z);
    page._color.clear();
    page._alpha.clear();
  }

  return Texture::do_load_one(cdata, pnmimage, name, z, n, options);
}

/**
 * Factory method to generate a Texture object
 */
void OpenCVTexture::
register_with_read_factory() {
  // Since Texture is such a funny object that is reloaded from the
  // TexturePool each time, instead of actually being read fully from the bam
  // file, and since the VideoTexture and OpenCVTexture classes don't really
  // add any useful data to the bam record, we don't need to define
  // make_from_bam(), fillin(), or write_datagram() in this class--we just
  // inherit the same functions from Texture.

  // We do, however, have to register this class with the BamReader, to avoid
  // warnings about creating the wrong kind of object from the bam file.
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 *
 */
OpenCVTexture::VideoStream::
VideoStream() :
  _capture(nullptr),
  _camera_index(-1),
  _next_frame(0)
{
}

/**
 *
 */
OpenCVTexture::VideoStream::
VideoStream(const OpenCVTexture::VideoStream &copy) :
  _capture(nullptr),
  _camera_index(-1)
{
  // Rather than copying the _capture pointer, we must open a new stream that
  // references the same file.
  if (copy.is_valid()) {
    if (copy.is_from_file()) {
      read(copy._filename);
    } else {
      from_camera(copy._camera_index);
    }
  }
}

/**
 *
 */
OpenCVTexture::VideoStream::
~VideoStream() {
  clear();
}

/**
 * Gets the data needed to traverse through the decompressed buffer for the
 * indicated frame number.  It is most efficient to call this in increasing
 * order of frame number.  Returns true on success, false on failure.
 *
 * In the case of a success indication (true return value), the three pointers
 * r, g, b are loaded with the addresses of the three components of the
 * bottom-left pixel of the image.  (They will be adjacent in memory in the
 * case of an interleaved image, and separated in the case of a separate-
 * channel image.)  The x_pitch value is filled with the amount to add to each
 * pointer to advance to the pixel to the right; and the y_pitch value is
 * filled with the amount to add to each pointer to advance to the pixel
 * above.  Note that these values may be negative (particularly in the case of
 * a top-down image).
 */
bool OpenCVTexture::VideoStream::
get_frame_data(int frame,
               const unsigned char *&r,
               const unsigned char *&g,
               const unsigned char *&b,
               int &x_pitch, int &y_pitch) {
  nassertr(is_valid(), false);

  if (is_from_file() && _next_frame != frame) {
    cvSetCaptureProperty(_capture, CV_CAP_PROP_POS_FRAMES, frame);
  }

  _next_frame = frame + 1;
  IplImage *image = cvQueryFrame(_capture);
  if (image == nullptr) {
    return false;
  }

  r = (const unsigned char *)image->imageData;
  g = r + 1;
  b = g + 1;
  x_pitch = 3;
  y_pitch = image->widthStep;

  if (image->dataOrder == 1) {
    // Separate channel images.  That means a block of r, followed by a block
    // of g, followed by a block of b.
    x_pitch = 1;
    g = r + image->height * y_pitch;
    b = g + image->height * y_pitch;
  }

  if (image->origin == 0) {
    // The image data starts with the top row and ends with the bottom row--
    // the opposite of Texture::_ram_data's storage convention.  Therefore, we
    // must increment the initial pointers to the last row, and count
    // backwards.
    r += (image->height - 1) * y_pitch;
    g += (image->height - 1) * y_pitch;
    b += (image->height - 1) * y_pitch;
    y_pitch = -y_pitch;
  }

  return true;
}

/**
 * Sets up the stream to read the indicated file.  Returns true on success,
 * false on failure.
 */
bool OpenCVTexture::VideoStream::
read(const Filename &filename) {
  clear();

  std::string os_specific = filename.to_os_specific();
  _capture = cvCaptureFromFile(os_specific.c_str());
  if (_capture == nullptr) {
    return false;
  }
  _filename = filename;
  return true;
}

/**
 * Sets up the stream to display the indicated camera.  Returns true on
 * success, false on failure.
 */
bool OpenCVTexture::VideoStream::
from_camera(int camera_index) {
  clear();

  _capture = cvCaptureFromCAM(camera_index);
  if (_capture == nullptr) {
    return false;
  }
  _camera_index = camera_index;
  return true;
}

/**
 * Stops the video playback and frees the associated resources.
 */
void OpenCVTexture::VideoStream::
clear() {
  if (_capture != nullptr) {
    cvReleaseCapture(&_capture);
    _capture = nullptr;
  }
  _filename = Filename();
  _camera_index = -1;
  _next_frame = 0;
}

#endif  // HAVE_OPENCV
