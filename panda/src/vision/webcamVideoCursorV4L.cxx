// Filename: webcamVideoCursorV4L.cxx
// Created by: rdb (11Jun2010)
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

#include "webcamVideoV4L.h"

#ifdef HAVE_VIDEO4LINUX

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#ifdef HAVE_JPEG
extern "C" {
  #include <jpeglib.h>
  #include <jerror.h>
}

#include <setjmp.h>
#endif

// This is supposed to be defined in jpegint.h,
// but not all implementations of JPEG provide that file.
#ifndef DSTATE_READY
#define DSTATE_READY 202
#endif

TypeHandle WebcamVideoCursorV4L::_type_handle;

#define clamp(x) min(max(x, 0.0), 255.0)

INLINE static void yuv_to_rgb(unsigned char *dest, const unsigned char *src) {
  double y1 = (255 / 219.0) * (src[0] - 16);
  double pb = (255 / 224.0) * (src[1] - 128);
  double pr = (255 / 224.0) * (src[2] - 128);
  dest[2] = clamp(1.0 * y1 + 0     * pb + 1.402 * pr);
  dest[1] = clamp(1.0 * y1 - 0.344 * pb - 0.714 * pr);
  dest[0] = clamp(1.0 * y1 + 1.772 * pb + 0     * pr);
}

INLINE static void yuyv_to_rgbrgb(unsigned char *dest, const unsigned char *src) {
  unsigned char yuv[] = {src[0], src[1], src[3]};
  yuv_to_rgb(dest, yuv);
  yuv[0] = src[2];
  yuv_to_rgb(dest + 3, yuv);
}

INLINE static void yuyv_to_rgbargba(unsigned char *dest, const unsigned char *src) {
  unsigned char yuv[] = {src[0], src[1], src[3]};
  yuv_to_rgb(dest, yuv);
  yuv[0] = src[2];
  yuv_to_rgb(dest + 4, yuv);
  dest[3] = (unsigned char) -1;
  dest[7] = (unsigned char) -1;
}

#if defined(HAVE_JPEG) && !defined(CPPPARSER)

struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr *my_error_ptr;

static void my_error_exit (j_common_ptr cinfo) {
  // Output the error message
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message) (cinfo, buffer);
  vision_cat.error() << buffer << "\n";
  // cinfo->err really points to a my_error_mgr struct, so coerce pointer
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  // Return control to the setjmp point
  longjmp(myerr->setjmp_buffer, 1);
}

static void my_output_message (j_common_ptr cinfo){
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message) (cinfo, buffer);
  vision_cat.warning() << buffer << "\n";
}

static void my_init_source(j_decompress_ptr cinfo) {
}

static boolean my_fill_input_buffer(j_decompress_ptr cinfo) {
  struct jpeg_source_mgr *src = cinfo->src;
  static JOCTET FakeEOI[] = {0xFF, JPEG_EOI};

  WARNMS(cinfo, JWRN_JPEG_EOF);

  src->next_input_byte = FakeEOI;
  src->bytes_in_buffer = 2;

  return TRUE;
}

static void my_skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
  struct jpeg_source_mgr *src = cinfo->src;

  if (num_bytes >= (long) src->bytes_in_buffer) {
    my_fill_input_buffer(cinfo);
    return;
  }

  src->bytes_in_buffer -= num_bytes;
  src->next_input_byte += num_bytes;
}

static void my_term_source(j_decompress_ptr cinfo) {
}

#endif

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoCursorV4L::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
WebcamVideoCursorV4L::
WebcamVideoCursorV4L(WebcamVideoV4L *src) : MovieVideoCursor(src) {
  _size_x = src->_size_x;
  _size_y = src->_size_y;
  _num_components = 3;
  _length = 1.0E10;
  _can_seek = false;
  _can_seek_fast = false;
  _aborted = false;
  _streaming = true;
  _ready = false;
  _format = (struct v4l2_format *) malloc(sizeof(struct v4l2_format));
  memset(_format, 0, sizeof(struct v4l2_format));
#ifdef HAVE_JPEG
  _cinfo = NULL;
#endif
  _buffers = NULL;
  _buflens = NULL;
  _fd = open(src->_device.c_str(), O_RDWR);
  if (-1 == _fd) {
    vision_cat.error() << "Failed to open " << src->_device.c_str() << "\n";
    return;
  }

  // Find the best format in our _pformats vector.
  // MJPEG is preferred over YUYV, as it's much smaller.
  _format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  pvector<uint32_t>::iterator it;
  for (it = src->_pformats.begin(); it != src->_pformats.end(); ++it) {
#ifdef HAVE_JPEG
    if (*it == V4L2_PIX_FMT_MJPEG) {
      _format->fmt.pix.pixelformat = *it;
      break;
    } else
#endif
    if (*it == V4L2_PIX_FMT_YUYV) {
      _format->fmt.pix.pixelformat = *it;
      break;
    }
  }
  if (it == src->_pformats.end()) {
    vision_cat.error() << "Failed to find a suitable pixel format!\n";
    _ready = false;
    close(_fd);
    _fd = -1;
    return;
  }

  // Request a format of this size, and no interlacing
  _format->fmt.pix.width = _size_x;
  _format->fmt.pix.height = _size_y;
  _format->fmt.pix.field = V4L2_FIELD_NONE;

  // Now politely ask the driver to switch to this format
  if (-1 == ioctl(_fd, VIDIOC_S_FMT, _format)) {
    vision_cat.error() << "Driver rejected format!\n";
    _ready = false;
    close(_fd);
    _fd = -1;
    return;
  }

  _size_x = _format->fmt.pix.width;
  _size_y = _format->fmt.pix.height;

  struct v4l2_streamparm streamparm;
  memset(&streamparm, 0, sizeof streamparm);
  streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  streamparm.parm.capture.timeperframe.numerator = 1;
  streamparm.parm.capture.timeperframe.denominator = src->_fps;
  if (ioctl(_fd, VIDIOC_S_PARM, &streamparm) < 0) {
    vision_cat.error() << "Driver rejected framerate!\n";
  }

  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof req);
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (-1 == ioctl (_fd, VIDIOC_REQBUFS, &req)) {
    vision_cat.error() << "Failed to request buffers from webcam!\n";
  }

  if (req.count < 2) {
    vision_cat.error() << "Insufficient buffer memory!\n";
  }

  _bufcount = req.count;
  _buffers = (void* *) calloc (req.count, sizeof (void*));
  _buflens = (size_t*) calloc (req.count, sizeof (size_t));

  if (!_buffers || !_buflens) {
    vision_cat.error() << "Not enough memory!\n";
  }

  // Set up the mmap buffers
  struct v4l2_buffer buf;
  for (int i = 0; i < _bufcount; ++i) {
    memset(&buf, 0, sizeof buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (-1 == ioctl(_fd, VIDIOC_QUERYBUF, &buf)) {
      vision_cat.error() << "Failed to query buffer!\n";
    }

    _buflens[i] = buf.length;
    _buffers[i] = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, buf.m.offset);

    if (_buffers[i] == MAP_FAILED) {
      vision_cat.error() << "Failed to map buffer!\n";
    }

    if (-1 == ioctl(_fd, VIDIOC_QBUF, &buf)) {
      vision_cat.error() << "Failed to exchange buffer with driver!\n";
    }
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ioctl(_fd, VIDIOC_STREAMON, &type)) {
    vision_cat.error() << "Failed to stream from buffer!\n";
  }

#ifdef HAVE_JPEG
  // Initialize the JPEG library, if necessary
  if (_format->fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) {
    _cinfo = (struct jpeg_decompress_struct *) malloc(sizeof(struct jpeg_decompress_struct));
    jpeg_create_decompress(_cinfo);

    _cinfo->src = (struct jpeg_source_mgr *)
      (*_cinfo->mem->alloc_small) ((j_common_ptr) _cinfo, JPOOL_PERMANENT,
                                          sizeof(struct jpeg_source_mgr));
    // Set up function pointers
    _cinfo->src->init_source = my_init_source;
    _cinfo->src->fill_input_buffer = my_fill_input_buffer;
    _cinfo->src->skip_input_data = my_skip_input_data;
    _cinfo->src->resync_to_restart = jpeg_resync_to_restart;
    _cinfo->src->term_source = my_term_source;
  }
#endif
  _ready = true;
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoCursorV4L::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
WebcamVideoCursorV4L::
~WebcamVideoCursorV4L() {
#ifdef HAVE_JPEG
  if (_cinfo != NULL) {
    jpeg_destroy_decompress(_cinfo);
    free(_cinfo);
  }
#endif
  if (_format != NULL) {
    free(_format);
  }
  if (-1 != _fd) {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(_fd, VIDIOC_STREAMOFF, &type);
    close(_fd);
  }
  if (_buffers) {
    for (int i = 0; i < _bufcount; ++i) {
      munmap(_buffers[i], _buflens[i]);
    }
    free(_buffers);
  }
  if (_buflens) {
    free(_buflens);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoCursorV4L::fetch_buffer
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor::Buffer) WebcamVideoCursorV4L::
fetch_buffer() {
  if (!_ready) {
    return NULL;
  }

  PT(Buffer) buffer = get_standard_buffer();
  unsigned char *block = buffer->_block;
  struct v4l2_buffer vbuf;
  memset(&vbuf, 0, sizeof vbuf);
  vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  vbuf.memory = V4L2_MEMORY_MMAP;
  if (-1 == ioctl(_fd, VIDIOC_DQBUF, &vbuf) && errno != EIO) {
    vision_cat.error() << "Failed to dequeue buffer!\n";
    return NULL;
  }
  nassertr(vbuf.index < _bufcount, NULL);
  size_t bufsize = _buflens[vbuf.index];
  size_t old_bpl = _format->fmt.pix.bytesperline;
  size_t new_bpl = _size_x * 3;
  unsigned char *buf = (unsigned char *) _buffers[vbuf.index];

  if (_format->fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) {
#ifdef HAVE_JPEG
    struct my_error_mgr jerr;
    _cinfo->err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    jerr.pub.output_message = my_output_message;

    unsigned char *newbuf = (unsigned char*) malloc(new_bpl * _size_y);

    // Establish the setjmp return context for my_error_exit to use
    if (setjmp(jerr.setjmp_buffer)) {
      if (_cinfo->global_state > DSTATE_READY) {
        jpeg_abort_decompress(_cinfo);
      }
    } else {
      // Set up data pointer
      _cinfo->src->bytes_in_buffer = bufsize;
      _cinfo->src->next_input_byte = buf;

      if (jpeg_read_header(_cinfo, TRUE) == JPEG_HEADER_OK) {
        _cinfo->scale_num = 1;
        _cinfo->scale_denom = 1;
        _cinfo->out_color_space = JCS_RGB;

        if (jpeg_start_decompress(_cinfo) && _cinfo->output_components == 3
          && _size_x == _cinfo->output_width && _size_y == _cinfo->output_height) {

          JSAMPLE *buffer_end = newbuf + new_bpl * _cinfo->output_height;
          JSAMPLE *rowptr = newbuf;
          while (_cinfo->output_scanline < _cinfo->output_height) {
            nassertd(rowptr + new_bpl <= buffer_end) break;
            jpeg_read_scanlines(_cinfo, &rowptr, 1);
            rowptr += new_bpl;
          }

          if (_cinfo->output_scanline < _cinfo->output_height) {
            jpeg_abort_decompress(_cinfo);
          } else {
            jpeg_finish_decompress(_cinfo);
          }
        }
      }
    }

    // Flip the image vertically
    for (size_t row = 0; row < _size_y; ++row) {
      memcpy(block + (_size_y - row - 1) * new_bpl, newbuf + row * new_bpl, new_bpl);
    }
    free(newbuf);

    // Swap red / blue
    unsigned char ex;
    for (size_t i = 0; i < new_bpl * _size_y; i += 3) {
      ex = block[i];
      block[i] = block[i + 2];
      block[i + 2] = ex;
    }
#else
    nassertr(false, NULL); // Not compiled with JPEG support
#endif
  } else {
    for (size_t row = 0; row < _size_y; ++row) {
      size_t c = 0;
      for (size_t i = 0; i < old_bpl; i += 4) {
        yuyv_to_rgbrgb(block + (_size_y - row - 1) * new_bpl + c, buf + row * old_bpl + i);
        c += 6;
      }
    }
  }

  if (-1 == ioctl(_fd, VIDIOC_QBUF, &vbuf)) {
    vision_cat.error() << "Failed to exchange buffer with driver!\n";
  }

  return buffer;
}

#endif
