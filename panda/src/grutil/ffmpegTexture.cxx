// Filename: ffmpegTexture.cxx
// Created by:  zacpavlov (05May06)
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

#include "pandabase.h"

#ifdef HAVE_FFMPEG
#include "ffmpegTexture.h"
#include "clockObject.h"
#include "config_gobj.h"
#include "config_grutil.h"
#include "bamCacheRecord.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle FFMpegTexture::_type_handle;

#if LIBAVFORMAT_VERSION_MAJOR < 53
  #define AVMEDIA_TYPE_VIDEO CODEC_TYPE_VIDEO
#endif

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
FFMpegTexture::
FFMpegTexture(const string &name) : 
  VideoTexture(name) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::Copy Constructor
//       Access: Protected
//  Description: Use FFmpegTexture::make_copy() to make a duplicate copy of
//               an existing FFMpegTexture.
////////////////////////////////////////////////////////////////////
FFMpegTexture::
FFMpegTexture(const FFMpegTexture &copy) : 
  VideoTexture(copy),
  _pages(copy._pages)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::Destructor
//       Access: Published, Virtual
//  Description: I'm betting that texture takes care of the, so we'll just do a clear.
////////////////////////////////////////////////////////////////////
FFMpegTexture::
~FFMpegTexture() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::do_make_copy
//       Access: Protected, Virtual
//  Description: Returns a new copy of the same Texture.  This copy,
//               if applied to geometry, will be copied into texture
//               as a separate texture from the original, so it will
//               be duplicated in texture memory (and may be
//               independently modified if desired).
//               
//               If the Texture is an FFMpegTexture, the resulting
//               duplicate may be animated independently of the
//               original.
////////////////////////////////////////////////////////////////////
PT(Texture) FFMpegTexture::
do_make_copy() {
  PT(FFMpegTexture) tex = new FFMpegTexture(get_name());
  tex->do_assign(*this);

  return tex.p();
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::do_assign
//       Access: Protected
//  Description: Implements make_copy().
////////////////////////////////////////////////////////////////////
void FFMpegTexture::
do_assign(const FFMpegTexture &copy) {
  VideoTexture::do_assign(copy);
  _pages = copy._pages;
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::modify_page
//       Access: Private
//  Description: Returns a reference to the zth VideoPage (level) of
//               the texture.  In the case of a 2-d texture, there is
//               only one page, level 0; but cube maps and 3-d
//               textures have more.
////////////////////////////////////////////////////////////////////
FFMpegTexture::VideoPage &FFMpegTexture::
modify_page(int z) {
  nassertr(z < _z_size, _pages[0]);
  while (z >= (int)_pages.size()) {
    _pages.push_back(VideoPage());
  }
  return _pages[z];
}

////////////////////////////////////////////////////////////////////
//     Function: FFmpegTexture::do_reconsider_video_properties
//       Access: Private
//  Description: Resets the internal Texture properties when a new
//               video file is loaded.  Returns true if the new image
//               is valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool FFMpegTexture::
do_reconsider_video_properties(const FFMpegTexture::VideoStream &stream, 
                               int num_components, int z, 
                               const LoaderOptions &options) {
  double frame_rate = 0.0f;
  int num_frames = 0;
  if (!stream._codec_context) {
    // printf("not valid yet\n");
    return true;
  }
  
  AVStream *vstream = stream._format_context->streams[stream._stream_number];
  
  if (stream.is_from_file()) {
    // frame rate comes from ffmpeg as an avRational. 
    frame_rate = vstream->r_frame_rate.num/(float)vstream->r_frame_rate.den;
    
    // Number of frames is a little questionable if we've got variable 
    // frame rate. Duration comes in as a generic timestamp, 
    // and is therefore multiplied by AV_TIME_BASE.
    num_frames = (int)((stream._format_context->duration*frame_rate)/AV_TIME_BASE);
    if (grutil_cat.is_debug()) {
      grutil_cat.debug()
        << "Loaded " << stream._filename << ", "
	<< num_frames << " frames at " << frame_rate << " fps\n";
    }
  }
  
  int width = stream._codec_context->width;
  int height = stream._codec_context->height;

  int x_size = width;
  int y_size = height;

  if (Texture::get_textures_power_2() != ATS_none) {
    x_size = up_to_power_2(width);
    y_size = up_to_power_2(height);
  }

  if (grutil_cat.is_debug()) {
    grutil_cat.debug()
      << "Video stream is " << width << " by " << height 
      << " pixels; fitting in texture " << x_size << " by "
      << y_size << " texels.\n";
  }

  if (!do_reconsider_image_properties(x_size, y_size, num_components,
                                      T_unsigned_byte, z, options)) {
    return false;
  }

  if (_loaded_from_image && 
      (get_video_width() != width || get_video_height() != height ||
       get_num_frames() != num_frames || get_frame_rate() != frame_rate)) {
    grutil_cat.error()
      << "Video properties have changed for texture " << get_name()
      << " level " << z << ".\n";
    return false;
  }

  set_frame_rate(frame_rate);
  set_num_frames(num_frames);
  set_video_size(width, height);

  // By default, the newly-loaded video stream will immediately start
  // looping.
  loop(true);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::make_texture
//       Access: Public, Static
//  Description: A factory function to make a new FFMpegTexture, used
//               to pass to the TexturePool.
////////////////////////////////////////////////////////////////////
PT(Texture) FFMpegTexture::
make_texture() {
  return new FFMpegTexture;
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::update_frame
//       Access: Protected, Virtual
//  Description: Called once per frame, as needed, to load the new
//               image contents.
////////////////////////////////////////////////////////////////////
void FFMpegTexture::
update_frame(int frame) {
  int max_z = min(_z_size, (int)_pages.size());
  for (int z = 0; z < max_z; ++z) {
    VideoPage &page = _pages.at(z);
    if (page._color.is_valid() || page._alpha.is_valid()) {
      do_modify_ram_image();
    }
    if (page._color.is_valid()) {
      nassertv(_num_components >= 3 && _component_width == 1);

      // A little different from the opencv implementation
      // The frame is kept on the stream itself. This is partially 
      // because there is a conversion step that must be done for 
      // every video (I've gotten very odd results with any video
      // that I don't convert, even if the IO formats are the same!)  
      if (page._color.get_frame_data(frame)) {
        nassertv(get_video_width() <= _x_size && get_video_height() <= _y_size);
        unsigned char *dest = _ram_images[0]._image.p() + do_get_expected_ram_page_size() * z;
        int dest_row_width = (_x_size * _num_components * _component_width);
        
        // Simplest case, where we deal with an rgb texture
        if (_num_components == 3) {
          int source_row_width=3*page._color._codec_context->width;
          unsigned char * source=(unsigned char *)page._color._frame_out->data[0]
            +source_row_width*(get_video_height()-1);         

          // row by row copy.        
          for (int y = 0; y < get_video_height(); ++y) {
            memcpy(dest, source, source_row_width);
            dest += dest_row_width;
            source -= source_row_width;
          }
          // Next best option, we're a 4 component alpha video on one stream 
        } else if (page._color._codec_context->pix_fmt==PIX_FMT_RGB32) {
          int source_row_width= page._color._codec_context->width * 4;
          unsigned char * source=(unsigned char *)page._color._frame_out->data[0]
            +source_row_width*(get_video_height()-1);
          
          // row by row copy.        
          for (int y = 0; y < get_video_height(); ++y) {
            memcpy(dest,source,source_row_width);
            dest += dest_row_width;
            source -= source_row_width;
          } 
          // Otherwise, we've got to be tricky
        } else {
          int source_row_width= page._color._codec_context->width * 3;
          unsigned char * source=(unsigned char *)page._color._frame_out->data[0]
            +source_row_width*(get_video_height()-1);
          
          // The harder case--interleave the color in with the alpha,
          // pixel by pixel.
          nassertv(_num_components == 4);
          for (int y = 0; y < get_video_height(); ++y) {
            int dx = 0;
            int sx = 0;
            for (int x = 0; x < get_video_width(); ++x) {
              dest[dx] = source[sx];
              dest[dx + 1] = source[sx + 1];
              dest[dx + 2] = source[sx + 2];
              dx += 4;
              sx += 3;
            }
            dest += dest_row_width;
            source -= source_row_width;
          }
        }
      }
      ++_image_modified;
    }
    
    if (page._alpha.is_valid()) {
      nassertv(_num_components == 4 && _component_width == 1);
  
      if (page._alpha.get_frame_data(frame)) {
        nassertv(get_video_width() <= _x_size && get_video_height() <= _y_size);
        
        // Currently, we assume the alpha has been converted to an rgb format
        // There is no reason it can't be a 256 color grayscale though.
        unsigned char *dest = _ram_images[0]._image.p() + do_get_expected_ram_page_size() * z;
        int dest_row_width = (_x_size * _num_components * _component_width);
        
        int source_row_width= page._alpha._codec_context->width * 3;
        unsigned char * source=(unsigned char *)page._alpha._frame_out->data[0]
          +source_row_width*(get_video_height()-1);
        for (int y = 0; y < get_video_height(); ++y) {
          int dx = 3;
          int sx = 0;
          for (int x = 0; x < get_video_width(); ++x) {
            dest[dx] = source[sx];
            dx += 4;
            sx += 3;
          }
          dest += dest_row_width;
          source -= source_row_width;
        }
      }
      ++_image_modified;
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::do_read_one
//       Access: Protected, Virtual
//  Description: Combines a color and alpha video image from the two
//               indicated filenames.  Both must be the same kind of
//               video with similar properties.
////////////////////////////////////////////////////////////////////
bool FFMpegTexture::
do_read_one(const Filename &fullpath, const Filename &alpha_fullpath,
            int z, int n, int primary_file_num_channels, int alpha_file_channel,
            const LoaderOptions &options,
            bool header_only, BamCacheRecord *record) {
  if (record != (BamCacheRecord *)NULL) {
    record->add_dependent_file(fullpath);
  }

  nassertr(n == 0, false);
  nassertr(z >= 0 && z < get_z_size(), false);

  VideoPage &page = modify_page(z);
  if (!page._color.read(fullpath)) {
    grutil_cat.error()
      << "FFMpeg couldn't read " << fullpath << " as video.\n";
    return false;
  }

  if (!alpha_fullpath.empty()) {
    if (!page._alpha.read(alpha_fullpath)) {
      grutil_cat.error()
        << "FFMPEG couldn't read " << alpha_fullpath << " as video.\n";
      page._color.clear();
      return false;
    }
  }


  if (z == 0) {
    if (!has_name()) {
      set_name(fullpath.get_basename_wo_extension());
    }
    if (!_filename.empty()) {
      _filename = fullpath;
      _alpha_filename = alpha_fullpath;
    }

    _fullpath = fullpath;
    _alpha_fullpath = alpha_fullpath;
  }
  if (page._color._codec_context->pix_fmt==PIX_FMT_RGB32) {
    // There had better not be an alpha interleave here. 
    nassertr(alpha_fullpath.empty(), false);
    
    _primary_file_num_channels = 4;
    _alpha_file_channel = 0;
    if (!do_reconsider_video_properties(page._color, 4, z, options)) {
      page._color.clear();
      return false;
    }
     
  } else {
    _primary_file_num_channels = 3;
    _alpha_file_channel = alpha_file_channel;

    if (page._alpha.is_valid()) {
      if (!do_reconsider_video_properties(page._color, 4, z, options)) {
        page._color.clear();
        page._alpha.clear();
        return false;
      }
      if (!do_reconsider_video_properties(page._alpha, 4, z, options)) {
        page._color.clear();
        page._alpha.clear();
        return false;
      }
    } else {
      if (!do_reconsider_video_properties(page._color, 3, z, options)) {
        page._color.clear();
        page._alpha.clear();
        return false;
      }
      
    }
    
  }
  set_loaded_from_image();
  clear_current_frame();
  update_frame(0);
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::do_load_one
//       Access: Protected, Virtual
//  Description: Resets the texture (or the particular level of the
//               texture) to the indicated static image.
////////////////////////////////////////////////////////////////////
bool FFMpegTexture::
do_load_one(const PNMImage &pnmimage, const string &name, int z, int n,
            const LoaderOptions &options) {
  if (z <= (int)_pages.size()) {
    VideoPage &page = modify_page(z);
    page._color.clear();
  }

  return Texture::do_load_one(pnmimage, name, z, n, options);
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::do_has_bam_rawdata
//       Access: Protected, Virtual
//  Description: Returns true if there is a rawdata image that we have
//               available to write to the bam stream.  For a normal
//               Texture, this is the same thing as
//               do_has_ram_image(), but a movie texture might define
//               it differently.
////////////////////////////////////////////////////////////////////
bool FFMpegTexture::
do_has_bam_rawdata() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::do_get_bam_rawdata
//       Access: Protected, Virtual
//  Description: If do_has_bam_rawdata() returned false, this attempts
//               to reload the rawdata image if possible.
////////////////////////////////////////////////////////////////////
void FFMpegTexture::
do_get_bam_rawdata() {
}




////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a Texture object
////////////////////////////////////////////////////////////////////
void FFMpegTexture::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::make_from_bam
//       Access: Protected, Static
//  Description: Factory method to generate an FFMpegTexture object
////////////////////////////////////////////////////////////////////
TypedWritable *FFMpegTexture::
make_from_bam(const FactoryParams &params) {
  PT(FFMpegTexture) dummy = new FFMpegTexture;
  return dummy->make_this_from_bam(params);
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::do_write_datagram_rawdata
//       Access: Protected, Virtual
//  Description: Writes the rawdata part of the texture to the
//               Datagram.
////////////////////////////////////////////////////////////////////
void FFMpegTexture::
do_write_datagram_rawdata(BamWriter *manager, Datagram &me) {
  me.add_uint32(_x_size);
  me.add_uint32(_y_size);
  me.add_uint32(_z_size);
  me.add_uint8(_component_type);
  me.add_uint8(_component_width);
  me.add_uint8(_ram_image_compression);

  me.add_uint16(_pages.size());
  for (size_t n = 0; n < _pages.size(); ++n) {
    VideoPage &page = _pages[n];
    page.write_datagram_rawdata(manager, me);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::do_fillin_rawdata
//       Access: Protected, Virtual
//  Description: Reads in the part of the Texture that was written
//               with do_write_datagram_rawdata().
////////////////////////////////////////////////////////////////////
void FFMpegTexture::
do_fillin_rawdata(DatagramIterator &scan, BamReader *manager) {
  _x_size = scan.get_uint32();
  _y_size = scan.get_uint32();
  _z_size = scan.get_uint32();
  _component_type = (ComponentType)scan.get_uint8();
  _component_width = scan.get_uint8();
  _ram_image_compression = CM_off;
  _ram_image_compression = (CompressionMode)scan.get_uint8();
  
  int num_pages = scan.get_uint16();
  _pages.clear();
  for (int n = 0; n < num_pages; ++n) {
    _pages.push_back(VideoPage());
    VideoPage &page = _pages.back();
    page.fillin_rawdata(scan, manager);

    LoaderOptions options;
    do_reconsider_video_properties(page._color, _num_components, n, options);
    if (page._alpha.is_valid()) {
      do_reconsider_video_properties(page._alpha, _num_components, n, options);
    }
  }

  _loaded_from_image = true;
  do_set_pad_size(0, 0, 0);
  ++_image_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FFMpegTexture::VideoStream::
VideoStream() :
  _codec_context(NULL),
  _format_context(NULL),
  _frame(NULL),
  _frame_out(NULL),
  _next_frame_number(0)
{
  // printf("creating video stream\n");
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FFMpegTexture::VideoStream::
VideoStream(const FFMpegTexture::VideoStream &copy) :
  _codec_context(NULL),
  _format_context(NULL),
  _frame(NULL),
  _frame_out(NULL),
  _next_frame_number(0)
{
  // Rather than copying the _capture pointer, we must open a new
  // stream that references the same file.
  if (copy.is_valid() && copy.is_from_file()) {
    if (copy._file_info.is_empty()) {
      read(copy._filename);
    } else {
      read(copy._file_info);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FFMpegTexture::VideoStream::
~VideoStream() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FFMpegTexture::VideoStream::
operator = (const FFMpegTexture::VideoStream &copy) {
  clear();

  // Rather than copying the _capture pointer, we must open a new
  // stream that references the same file.
  if (copy.is_valid() && copy.is_from_file()) {
    if (copy._file_info.is_empty()) {
      read(copy._filename);
    } else {
      read(copy._file_info);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::get_frame_data
//       Access: Public
//  Description: Returns the pointer to the beginning of the
//               decompressed buffer for the indicated frame number.
//               It is most efficient to call this in increasing order
//               of frame number.
////////////////////////////////////////////////////////////////////
bool FFMpegTexture::VideoStream::
get_frame_data(int frame_number) {
  nassertr(is_valid(), false);
  int coming_from = _next_frame_number;

  _next_frame_number = frame_number + 1;
  AVPacket packet;
  AVStream *vstream = _format_context->streams[_stream_number];
  
  int got_frame;
                  
  // Can we get to our target frame just by skipping forward a few
  // frames?  We arbitrarily draw the line at 50 frames for now.
  if (frame_number >= coming_from && frame_number - coming_from < 50) { 

    if (frame_number > coming_from) {
      // Ok, we do have to skip a few frames.
      _codec_context->skip_frame = AVDISCARD_BIDIR;

      while (frame_number > coming_from) {
        int err = read_video_frame(&packet);
        if (err < 0) {
          return false;
        }
#if LIBAVCODEC_VERSION_INT < 3414272
        avcodec_decode_video(_codec_context, _frame, &got_frame, packet.data, packet.size);
#else
        avcodec_decode_video2(_codec_context, _frame, &got_frame, &packet);
#endif
        av_free_packet(&packet);
        ++coming_from;
      }
      _codec_context->skip_frame = AVDISCARD_DEFAULT;
    }

    // Now we're ready to read a frame.
    int err = read_video_frame(&packet);
    if (err < 0) {
      return false;
    }

  } else {
    // We have to skip backward, or maybe forward a whole bunch of
    // frames.  Better off seeking through the stream.

    double time_stamp = ((double)AV_TIME_BASE * frame_number * vstream->r_frame_rate.den) / vstream->r_frame_rate.num;
    double curr_time_stamp;

    // find point in time
    av_seek_frame(_format_context, -1, (long long)time_stamp,
                  AVSEEK_FLAG_BACKWARD);
    
    // Okay, now we're at the nearest keyframe behind our timestamp.
    // Hurry up and move through frames until we find a frame just after it.
    _codec_context->skip_frame = AVDISCARD_BIDIR;
    do {
      int err = read_video_frame(&packet);
      if (err < 0) {
        return false;
      }

      curr_time_stamp = (((double)AV_TIME_BASE * packet.pts) / 
                         ((double)packet.duration * av_q2d(vstream->r_frame_rate)));
      if (curr_time_stamp > time_stamp) {
        break;
      }

#if LIBAVCODEC_VERSION_INT < 3414272
      avcodec_decode_video(_codec_context, _frame, &got_frame, packet.data, packet.size);
#else
      avcodec_decode_video2(_codec_context, _frame, &got_frame, &packet);
#endif

      av_free_packet(&packet);
    } while (true);
    
    _codec_context->skip_frame = AVDISCARD_DEFAULT;
    // Now near frame with Packet ready for decode (and free)
  }
  
  // Now we have a packet from someone. Lets get this in a frame
  
  int frame_finished;

  // Is this a packet from the video stream?
  if (packet.stream_index == _stream_number) {
    // Decode video frame
#if LIBAVCODEC_VERSION_INT < 3414272
    avcodec_decode_video(_codec_context, _frame, &frame_finished, packet.data, packet.size);
#else
    avcodec_decode_video2(_codec_context, _frame, &frame_finished, &packet);
#endif

    // Did we get a video frame?
    if (frame_finished) {
      // Convert the image from its native format to RGB
#ifdef HAVE_SWSCALE
      // Note from pro-rsoft: ffmpeg removed img_convert and told
      // everyone to use sws_scale instead - that's why I wrote
      // this code. I have no idea if it works well or not, but
      // it seems to compile and run without crashing.
      PixelFormat dst_format;
      if (_codec_context->pix_fmt != PIX_FMT_RGB32) {
        dst_format = PIX_FMT_BGR24;
      } else {
        dst_format = PIX_FMT_RGB32;
      }
      struct SwsContext *convert_ctx = 
        sws_getContext(_codec_context->width, _codec_context->height,
                       _codec_context->pix_fmt, _codec_context->width, _codec_context->height,
                       dst_format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
      nassertr(convert_ctx != NULL, false);
      sws_scale(convert_ctx, _frame->data, _frame->linesize,
                0, _codec_context->height, _frame_out->data, _frame_out->linesize);
      sws_freeContext(convert_ctx);
#else
      if (_codec_context->pix_fmt != PIX_FMT_RGB32) {
        img_convert((AVPicture *)_frame_out, PIX_FMT_BGR24, 
                    (AVPicture *)_frame, _codec_context->pix_fmt, 
                    _codec_context->width, _codec_context->height);

      } else { // _codec_context->pix_fmt == PIX_FMT_RGB32
        img_convert((AVPicture *)_frame_out, PIX_FMT_RGB32, 
                    (AVPicture *)_frame, _codec_context->pix_fmt, 
                    _codec_context->width, _codec_context->height);
      }
#endif
    }
  }

  // Free the packet that was allocated by av_read_frame
  av_free_packet(&packet);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::read
//       Access: Public
//  Description: Sets up the stream to read the indicated file from
//               the VFS.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool FFMpegTexture::VideoStream::
read(const Filename &filename) {
  // Clear out the last stream
  clear();
  
  // Open video file
  if (!_ffvfile.open_vfs(filename)) {
    grutil_cat.error()
      << "couldn't open " << filename << "\n";
    // Don't call clear(), because nothing happened yet
    return false;
  }

  _filename = filename;
  return continue_read();
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::read
//       Access: Public
//  Description: Sets up the stream to read the indicated file,
//               avoiding the VFS.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool FFMpegTexture::VideoStream::
read(const SubfileInfo &info) {
  // Clear out the last stream
  clear();
  
  // Open video file
  if (!_ffvfile.open_subfile(info)) {
    grutil_cat.error()
      << "couldn't open " << info << "\n";
    // Don't call clear(), because nothing happened yet
    return false;
  }

  _file_info = info;
  _filename = _file_info.get_filename();
  return continue_read();
}


////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::clear
//       Access: Public
//  Description: Stops the video playback and frees the associated
//               resources.
////////////////////////////////////////////////////////////////////
void FFMpegTexture::VideoStream::
clear() {
  if (_codec_context) {
    avcodec_close(_codec_context);
    _codec_context = NULL;
  }

  if (_frame) {
    av_free(_frame);
    _frame = NULL;
  }
  if (_frame_out) {
    av_free(_frame_out);
    _frame_out = NULL;
  }

  // We cannot close the format_context, since we didn't open it--the
  // FfmpegVirtualFile will do that.
  _format_context = NULL;

  _next_frame_number = 0;
  _filename = Filename();
  _file_info = SubfileInfo();
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::write_datagram_rawdata
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FFMpegTexture::VideoStream::
write_datagram_rawdata(BamWriter *manager, Datagram &me) {
  SubfileInfo result;
  if (!_file_info.is_empty()) {
    me.add_bool(true);
    manager->write_file_data(result, _file_info);
  } else if (!_filename.empty()) {
    me.add_bool(true);
    manager->write_file_data(result, _filename);
  } else {
    me.add_bool(false);
  }

  /* Not sure yet if this is a good idea.
  if (!result.is_empty()) {
    // If we've just copied the data to a local file, read it from
    // there in the future.
    _file_info = result;
  }
  */
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::fillin_rawdata
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FFMpegTexture::VideoStream::
fillin_rawdata(DatagramIterator &scan, BamReader *manager) {
  bool got_info = scan.get_bool();
  if (got_info) {
    SubfileInfo info;
    manager->read_file_data(info);
    read(info);
  } else {
    clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::continue_read
//       Access: Private
//  Description: Once the FfmpegVirtualFile has been opened, continues
//               to make the appropriate ffmpeg calls to open the
//               stream.
////////////////////////////////////////////////////////////////////
bool FFMpegTexture::VideoStream::
continue_read() {
  _format_context = _ffvfile.get_format_context();
  nassertr(_format_context != NULL, false);

  // Retrieve stream information
  int result = av_find_stream_info(_format_context);
  if (result < 0) {
    grutil_cat.error() << "ffmpeg AVERROR: " << result << endl;
    clear();
    return false;
  }
  dump_format(_format_context, 0, _filename.c_str(), false);
  
  _stream_number = -1;
  for(size_t i = 0; i < _format_context->nb_streams; i++) {
    if ((*_format_context->streams[i]->codec).codec_type == AVMEDIA_TYPE_VIDEO) {
      _stream_number = i;
      break;
    }
  }

  if (_stream_number == -1) {
    grutil_cat.error()
      << "ffmpeg: no stream found with codec of type AVMEDIA_TYPE_VIDEO" << endl;
    clear();
    return false;
  }
  
  // Get a pointer to the codec context for the video stream
  AVCodecContext *codec_context = _format_context->streams[_stream_number]->codec;
  
  if (grutil_cat.is_debug()) {
    grutil_cat.debug()
      << "ffmpeg: codec id is " << codec_context->codec_id << endl;
  }

  // Find the decoder for the video stream
  _codec = avcodec_find_decoder(codec_context->codec_id);
  if (_codec == NULL) {
    grutil_cat.error() << "ffmpeg: no appropriate decoder found" << endl;
    clear();
    return false;
  }

  if (_codec->capabilities & CODEC_CAP_TRUNCATED) {
    codec_context->flags |= CODEC_FLAG_TRUNCATED;
  }

  // Open codec
  _codec_context = codec_context;
  result = avcodec_open(_codec_context, _codec);
  if (result < 0) {
    grutil_cat.error() << "ffmpeg AVERROR: " << result << endl;
    _codec_context = NULL;
    clear();
    return false;
  }
  
  _frame = avcodec_alloc_frame();
  
  if (_codec_context->pix_fmt != PIX_FMT_RGB32) {
    _frame_out = avcodec_alloc_frame();
    if (_frame_out == NULL) {
      grutil_cat.error()
        << "ffmpeg: unable to allocate AVPFrame (BGR24)" << endl;
      clear();
      return false;
    }

    // Determine required buffer size and allocate buffer
    _image_size_bytes = avpicture_get_size(PIX_FMT_BGR24, _codec_context->width,
                                           _codec_context->height);
            
    _raw_data = new uint8_t[_image_size_bytes];

    // Assign appropriate parts of buffer to image planes in _frameRGB
    avpicture_fill((AVPicture *)_frame_out, _raw_data, PIX_FMT_BGR24,
                   _codec_context->width, _codec_context->height);

  } else {
    _frame_out = avcodec_alloc_frame();
    if (_frame_out == NULL) {
      grutil_cat.error()
        << "ffmpeg: unable to allocate AVPFrame (RGBA32)" << endl;
      clear();
      return false;
    }
    
    // Determine required buffer size and allocate buffer
    _image_size_bytes = avpicture_get_size(PIX_FMT_RGB32, _codec_context->width,
                                           _codec_context->height);
            
    _raw_data = new uint8_t[_image_size_bytes];
    // Assign appropriate parts of buffer to image planes in _frameRGB
    avpicture_fill((AVPicture *)_frame_out, _raw_data, PIX_FMT_RGB32,
                   _codec_context->width, _codec_context->height);
  } 
  // We could put an option here for single channel frames.
  
  _next_frame_number = 0;

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::read_video_frame
//       Access: Private
//  Description: Fills packet with the next sequential video frame in
//               the stream, skipping over all non-video frames.
//               packet must later be deallocated with
//               av_free_packet().
//
//               Returns nonnegative on success, or negative on error.
////////////////////////////////////////////////////////////////////
int FFMpegTexture::VideoStream::
read_video_frame(AVPacket *packet) {
  int err = av_read_frame(_format_context, packet);
  if (grutil_cat.is_spam()) {
    grutil_cat.spam()
      << "av_read_frame() = " << err << "\n";
  }
  if (err < 0) {
    return err;
  }

  while (packet->stream_index != _stream_number) {
    // It's not a video packet; free it and get another.
    av_free_packet(packet);

    err = av_read_frame(_format_context, packet);
    if (grutil_cat.is_spam()) {
      grutil_cat.spam()
        << "av_read_frame() = " << err << "\n";
    }
    if (err < 0) {
      if (grutil_cat.is_debug()) {
        grutil_cat.debug()
          << "Got error " << err << " reading frame.\n";
      }
      return err;
    }
  }

  // This is a video packet, return it.
  return err;
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoPage::write_datagram_rawdata
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FFMpegTexture::VideoPage::
write_datagram_rawdata(BamWriter *manager, Datagram &me) {
  _color.write_datagram_rawdata(manager, me);
  _alpha.write_datagram_rawdata(manager, me);
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoPage::fillin_rawdata
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FFMpegTexture::VideoPage::
fillin_rawdata(DatagramIterator &scan, BamReader *manager) {
  _color.fillin_rawdata(scan, manager);
  _alpha.fillin_rawdata(scan, manager);
}


#endif  // HAVE_FFMpeg

