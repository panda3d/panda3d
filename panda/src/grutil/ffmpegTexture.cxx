// Filename: ffmpegTexture.cxx
// Created by:  zacpavlov (05May06)
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

#include "pandabase.h"

#ifdef HAVE_FFMPEG
#include "ffmpegTexture.h"
#include "clockObject.h"
#include "config_gobj.h"
#include "config_grutil.h"

TypeHandle FFMpegTexture::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::Constructor
//       Access: Published
//  Description: Sets up the texture to read frames from a camera
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
//     Function: FFMpegTexture::make_copy
//       Access: Published, Virtual
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
make_copy() {
  return new FFMpegTexture(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: FFMPegTexture::modify_page
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
//     Function: FFmpegTexture::reconsider_video_properties
//       Access: Private
//  Description: Resets the internal Texture properties when a new
//               video file is loaded.  Returns true if the new image
//               is valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool FFMpegTexture::
reconsider_video_properties(const FFMpegTexture::VideoStream &stream, 
                            int num_components, int z) {

  double frame_rate = 0.0f;
  int num_frames = 0;
  if(!stream.pCodecCtx)
  {
    //printf("not valid yet\n");
      return true;
  }
  
        
  AVStream* vstream=stream.pFormatCtx->streams[stream.streamNumber];
  
  if (stream.is_from_file() ) {
      //frame rate comes from ffmpeg as an avRational. 
      frame_rate = vstream->r_frame_rate.num/(float)vstream->r_frame_rate.den;
      
      //Number of frames is a little questionable if we've got variable 
      //frame rate. Duration comes in as a generic timestamp, 
      //and is therefore multiplied by AV_TIME_BASE.
      num_frames = (int(stream.pFormatCtx->duration*frame_rate))/AV_TIME_BASE;
      if (grutil_cat.is_debug()) {
        grutil_cat.debug()
          << "Loaded " << stream._filename << ", " << num_frames << " frames at "
          << frame_rate << " fps\n";
      }
    
  }
  
  int width = stream.pCodecCtx->width;
  int height = stream.pCodecCtx->height;

  int x_size = width;
  int y_size = height;

  if (textures_power_2 != ATS_none) {
    x_size = up_to_power_2(width);
    y_size = up_to_power_2(height);
  }

  if (grutil_cat.is_debug()) {
    grutil_cat.debug()
      << "Video stream is " << width << " by " << height 
      << " pixels; fitting in texture " << x_size << " by "
      << y_size << " texels.\n";
  }

  if (!reconsider_image_properties(x_size, y_size, num_components,
                                   T_unsigned_byte, z)) {
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
//     Function: FFMPegTexture::update_frame
//       Access: Protected, Virtual
//  Description: Called once per frame, as needed, to load the new
//               image contents.
////////////////////////////////////////////////////////////////////
void FFMpegTexture::update_frame(int frame) {
  int max_z = max(_z_size, (int)_pages.size());
  for (int z = 0; z < max_z; ++z) {
    VideoPage &page = _pages[z];
    if (page._color.is_valid() || page._alpha.is_valid()) {
      modify_ram_image();
    }
    if (page._color.is_valid()) {
      nassertv(get_num_components() >= 3 && get_component_width() == 1);

      //A little different from the opencv implementation
      //The frame is kept on the stream itself. This is partially 
      //because there is a conversion step that must be done for 
      //every video (I've gotten very odd results with any video
      //that I don't convert, even if the IO formats are the same!)  
      if(page._color.get_frame_data(frame)) {
        nassertv(get_video_width() <= _x_size && get_video_height() <= _y_size);
        unsigned char *dest = _ram_images[0]._image.p() + get_expected_ram_page_size() * z;
        int dest_row_width = (_x_size * _num_components * _component_width);
        
        //Simplest case, where we deal with an rgb texture
        if (get_num_components() == 3) {
          int source_row_width=3*page._color.pCodecCtx->width;
          unsigned char * source=(unsigned char *)page._color.pFrameOut->data[0]
                                  +source_row_width*(get_video_height()-1);         

          //row by row copy.        
          for (int y = 0; y < get_video_height(); ++y) {
            memcpy(dest, source, source_row_width);
            dest += dest_row_width;
            source -= source_row_width;
          }
        //Next best option, we're a 4 component alpha video on one stream 
        } else if(page._color.pCodecCtx->pix_fmt==PIX_FMT_RGBA32) {
          int source_row_width= page._color.pCodecCtx->width * 4;
          unsigned char * source=(unsigned char *)page._color.pFrameOut->data[0]
                                  +source_row_width*(get_video_height()-1);
          
          //row by row copy.        
          for (int y = 0; y < get_video_height(); ++y) {
            memcpy(dest,source,source_row_width);
            dest += dest_row_width;
            source -= source_row_width;
          } 
        //Otherwise, we've got to be tricky
        } else {
          int source_row_width= page._color.pCodecCtx->width * 3;
          unsigned char * source=(unsigned char *)page._color.pFrameOut->data[0]
                                  +source_row_width*(get_video_height()-1);
          
          // The harder case--interleave the color in with the alpha,
          // pixel by pixel.
          nassertv(get_num_components() == 4);
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
    }
    
    if (page._alpha.is_valid()) {
      nassertv(get_num_components() == 4 && get_component_width() == 1);
  
      if(page._alpha.get_frame_data(frame)) {
        nassertv(get_video_width() <= _x_size && get_video_height() <= _y_size);
        
        //Currently, we assume the alpha has been converted to an rgb format
        //There is no reason it can't be a 256 color grayscale though.
        unsigned char *dest = _ram_images[0]._image.p() + get_expected_ram_page_size() * z;
        int dest_row_width = (_x_size * _num_components * _component_width);
        
        int source_row_width= page._alpha.pCodecCtx->width * 3;
        unsigned char * source=(unsigned char *)page._alpha.pFrameOut->data[0]
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
      int z, int n, int primary_file_num_channels, int alpha_file_channel) {
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
    if (!has_filename()) {
      set_filename(fullpath);
      set_alpha_filename(alpha_fullpath);
    }

    set_fullpath(fullpath);
    set_alpha_fullpath(alpha_fullpath);
  }
  if(page._color.pCodecCtx->pix_fmt==PIX_FMT_RGBA32) {
    //There had better not be an alpha interleave here. 
    nassertr(alpha_fullpath.empty(), false);
    
    _primary_file_num_channels = 4;
    _alpha_file_channel = 0;
    if (!reconsider_video_properties(page._color, 4, z)) {
        page._color.clear();
        return false;
    }
     
  } else {
    _primary_file_num_channels = 3;
    _alpha_file_channel = alpha_file_channel;

    if(page._alpha.is_valid()) {
      if (!reconsider_video_properties(page._color, 4, z)) {
        page._color.clear();
        page._alpha.clear();
        return false;
      }
      if (!reconsider_video_properties(page._alpha, 4, z)) {
        page._color.clear();
        page._alpha.clear();
        return false;
      }
    } else {
      if (!reconsider_video_properties(page._color, 3, z)) {
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
do_load_one(const PNMImage &pnmimage, int z, int n) {
  if (z <= (int)_pages.size()) {
    VideoPage &page = modify_page(z);
    page._color.clear();
  }

  return Texture::do_load_one(pnmimage, z, n);
}




////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a Texture object
////////////////////////////////////////////////////////////////////
void FFMpegTexture::
register_with_read_factory() {
  // Since Texture is such a funny object that is reloaded from the
  // TexturePool each time, instead of actually being read fully from
  // the bam file, and since the VideoTexture and FFMpegTexture
  // classes don't really add any useful data to the bam record, we
  // don't need to define make_from_bam(), fillin(), or
  // write_datagram() in this class--we just inherit the same
  // functions from Texture.

  // We do, however, have to register this class with the BamReader,
  // to avoid warnings about creating the wrong kind of object from
  // the bam file.
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FFMpegTexture::VideoStream::
VideoStream() :
  pCodecCtx(NULL),
  pFormatCtx(NULL),
  pFrame(NULL),
  pFrameOut(NULL),
  _next_frame(0)
{
  //printf("creating video stream\n");
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FFMpegTexture::VideoStream::
VideoStream(const FFMpegTexture::VideoStream &copy) :
  pCodecCtx(NULL),
  pFormatCtx(NULL),
  pFrame(NULL),
  pFrameOut(NULL),
  _next_frame(0)
{
  // Rather than copying the _capture pointer, we must open a new
  // stream that references the same file.
  if (copy.is_valid()) {
    if (copy.is_from_file()) {
      read(copy._filename);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FFMpegTexture::VideoStream::
~VideoStream() {
 clear();
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
get_frame_data(int frame) {
  nassertr(is_valid(), false);
  int comingFrom=_next_frame;

  _next_frame = frame + 1;
  AVPacket packet;
  AVStream* vstream=pFormatCtx->streams[streamNumber];
  
  int gotFrame;
  
  long long timeStamp=(AV_TIME_BASE/vstream->r_frame_rate.num)*(frame *vstream->r_frame_rate.den);
  long long currTimeStamp;
                  
  //first find out where to go
  if(frame==comingFrom)
  { 
    int err=av_read_frame(pFormatCtx, &packet);
    if(err<0)
    {
      return 0;
    }
  }
  else
  {
    //find point in time
    int res=av_seek_frame( pFormatCtx,-1, timeStamp,AVSEEK_FLAG_BACKWARD );
    
    //Okay, now we're at the nearest keyframe behind our timestamp.
    //Hurry up and move through frames until we find a frame just after it.
    pCodecCtx->hurry_up = 1;
    do {
      av_read_frame( pFormatCtx, &packet );
      
      // should really be checking that this is a video packet
      currTimeStamp = (long long)(packet.pts / packet.duration *
                                  AV_TIME_BASE / av_q2d( vstream->r_frame_rate));
      
      if( currTimeStamp > timeStamp )
          break;
      
      avcodec_decode_video( pCodecCtx, pFrame, &gotFrame, packet.data,
          packet.size );
      av_free_packet( &packet );
    } while(1);
    pCodecCtx->hurry_up = 0;
    // Now near frame with Packet ready for decode (and free)
  }
  
  //Now we have a packet from someone. Lets get this in a frame
  
  int frameFinished;
  // Is this a packet from the video stream?
  if(packet.stream_index==streamNumber)
  {
    // Decode video frame
    avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, 
        packet.data, packet.size);

    // Did we get a video frame?
    if(frameFinished)
    {
      // Convert the image from its native format to RGB
      if(pCodecCtx->pix_fmt!=PIX_FMT_RGBA32)
      {
        img_convert((AVPicture *)pFrameOut, PIX_FMT_BGR24, 
          (AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, 
          pCodecCtx->height);
      }
      else if(pCodecCtx->pix_fmt==PIX_FMT_RGBA32)
      {
        img_convert((AVPicture *)pFrameOut, PIX_FMT_RGBA32, 
          (AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, 
          pCodecCtx->height);
      }
    }
  }

  // Free the packet that was allocated by av_read_frame
  av_free_packet(&packet);

  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::read
//       Access: Public
//  Description: Sets up the stream to read the indicated file.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool FFMpegTexture::VideoStream::
read(const Filename &filename) {
  
  //Clear out the last stream
  clear();
  
  string os_specific = filename.to_os_specific();
  // Open video file
  if(av_open_input_file(&pFormatCtx, os_specific.c_str(), NULL, 0, NULL)!=0) {
    //Don't do anything, because nothing happened yet
    return 0;
  }

  // Retrieve stream information
  if(av_find_stream_info(pFormatCtx)<0) {
    clear();
    return 0;
  }
  dump_format(pFormatCtx, 0, os_specific.c_str(), false);
  
  streamNumber=-1;
  for(int i=0; i<pFormatCtx->nb_streams; i++) {
    if((*pFormatCtx->streams[i]->codec).codec_type==CODEC_TYPE_VIDEO) {
      streamNumber=i;
      break;
    }
  }
  if(streamNumber==-1) {
    clear();
    return 0;
  }
  
  //Get a pointer to the codec context for the video stream
  pCodecCtx=pFormatCtx->streams[streamNumber]->codec;
  
  //Find the decoder for the video stream
  //printf("codec id is %d\n",pCodecCtx->codec_id);
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    clear();
    return 0;
  }
  
  if(pCodec->capabilities & CODEC_CAP_TRUNCATED)
    pCodecCtx->flags|=CODEC_FLAG_TRUNCATED;

  // Open codec
  if(avcodec_open(pCodecCtx, pCodec)<0) {
    clear();
    return 0;
  }
  
  pFrame=avcodec_alloc_frame();
  
  if(pCodecCtx->pix_fmt!=PIX_FMT_RGBA32) {
    pFrameOut=avcodec_alloc_frame();
    if(pFrameOut==NULL) {
      clear();
      return 0;
    }
    
    // Determine required buffer size and allocate buffer
    imageSizeBytes=avpicture_get_size(PIX_FMT_BGR24, pCodecCtx->width,
            pCodecCtx->height);
            
    rawData=new uint8_t[imageSizeBytes];
    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)pFrameOut, rawData, PIX_FMT_BGR24,
              pCodecCtx->width, pCodecCtx->height);
  } else {
    pFrameOut=avcodec_alloc_frame();
    if(pFrameOut==NULL) {
      clear();
      return 0;
    }
    
    // Determine required buffer size and allocate buffer
    imageSizeBytes=avpicture_get_size(PIX_FMT_RGBA32, pCodecCtx->width,
            pCodecCtx->height);
            
    rawData=new uint8_t[imageSizeBytes];
    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)pFrameOut, rawData, PIX_FMT_RGBA32,
              pCodecCtx->width, pCodecCtx->height);
  } 
  //We could put an option here for single channel frames.
  
    
  _next_frame=0;
  
  _filename = filename;
  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::clear
//       Access: Public
//  Description: Stops the video playback and frees the associated
//               resources.
////////////////////////////////////////////////////////////////////
void FFMpegTexture::VideoStream::
clear() {
  if(pCodecCtx)
  {
    avcodec_close(pCodecCtx);
    pCodecCtx=0;  
  }
  if(pFormatCtx)
  {
    av_close_input_file(pFormatCtx);
    pFormatCtx=0;
  }
  if(pFrame)
  {
    av_free(pFrame);
    pFrame=0;
  }
  if(pFrameOut)
  {
    av_free(pFrameOut);
    pFrameOut=0;
  }
  
  _next_frame=0;
  
}


#endif  // HAVE_FFMpeg

