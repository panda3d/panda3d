/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webcamVideoCursorV4L.cxx
 * @author rdb
 * @date 2010-06-11
 */

#include "webcamVideoCursorV4L.h"

#include "config_vision.h"
#include "webcamVideoV4L.h"

#include "movieVideoCursor.h"

#if defined(HAVE_VIDEO4LINUX) && !defined(CPPPARSER)

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#ifdef HAVE_JPEG
extern "C" {
  #include <jpeglib.h>
  #include <jerror.h>
}

#include <setjmp.h>
#endif

TypeHandle WebcamVideoCursorV4L::_type_handle;

#define clamp(x) std::min(std::max(x, 0.0), 255.0)

INLINE static void yuv_to_bgr(unsigned char *dest, const unsigned char *src) {
  double y1 = (255 / 219.0) * (src[0] - 16);
  double pb = (255 / 224.0) * (src[1] - 128);
  double pr = (255 / 224.0) * (src[2] - 128);
  dest[2] = clamp(1.0 * y1 + 0     * pb + 1.402 * pr);
  dest[1] = clamp(1.0 * y1 - 0.344 * pb - 0.714 * pr);
  dest[0] = clamp(1.0 * y1 + 1.772 * pb + 0     * pr);
}

INLINE static void yuyv_to_bgrbgr(unsigned char *dest, const unsigned char *src) {
  unsigned char yuv[] = {src[0], src[1], src[3]};
  yuv_to_bgr(dest, yuv);
  yuv[0] = src[2];
  yuv_to_bgr(dest + 3, yuv);
}

INLINE static void yuyv_to_bgrabgra(unsigned char *dest, const unsigned char *src) {
  unsigned char yuv[] = {src[0], src[1], src[3]};
  yuv_to_bgr(dest, yuv);
  yuv[0] = src[2];
  yuv_to_bgr(dest + 4, yuv);
  dest[3] = 0xff;
  dest[7] = 0xff;
}

INLINE static void rgb_to_bgr(unsigned char *dest, const unsigned char *src) {
  dest[0] = src[2];
  dest[1] = src[1];
  dest[2] = src[0];
}

INLINE static void rgb_to_bgra(unsigned char *dest, const unsigned char *src) {
  dest[0] = src[2];
  dest[1] = src[1];
  dest[2] = src[0];
  dest[3] = 0xff;
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

// Huffman tables used for MJPEG streams that omit them.
static JHUFF_TBL dc_luminance_tbl = {
  {0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
  FALSE
};

static JHUFF_TBL dc_chrominance_tbl = {
  {0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
  FALSE
};

static JHUFF_TBL ac_luminance_tbl = {
  {0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d},
  {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21,
    0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71,
    0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1,
    0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72,
    0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83,
    0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93,
    0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3,
    0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3,
    0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1,
    0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa
  },
  FALSE
};

static JHUFF_TBL ac_chrominance_tbl = {
  {0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77},
  {
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31,
    0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22,
    0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1,
    0x09, 0x23, 0x33, 0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1,
    0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18,
    0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
    0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,
    0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,
    0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
    0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
    0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,
    0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa
  },
  FALSE
};

#endif

/**
 *
 */
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
  memset(&_format, 0, sizeof(struct v4l2_format));

  _buffers = nullptr;
  _buflens = nullptr;

  int mode = O_RDWR;
  if (!v4l_blocking) {
    mode = O_NONBLOCK;
  }

  _fd = open(src->_device.c_str(), mode);
  if (-1 == _fd) {
    vision_cat.error() << "Failed to open " << src->_device.c_str() << "\n";
    return;
  }

  // Find the best format in our _pformats vector.  MJPEG is preferred over
  // YUYV, as it's much smaller.
  _format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  _format.fmt.pix.pixelformat = src->_pformat;

  switch (_format.fmt.pix.pixelformat) {
#ifdef HAVE_JPEG
  case V4L2_PIX_FMT_MJPEG:
    _num_components = 3;
    break;
#endif

  case V4L2_PIX_FMT_YUYV:
    _num_components = 3;
    break;

  case V4L2_PIX_FMT_BGR24:
    _num_components = 3;
    break;

  case V4L2_PIX_FMT_BGR32:
    _num_components = 4;
    break;

  case V4L2_PIX_FMT_RGB24:
    _num_components = 3;
    break;

  case V4L2_PIX_FMT_RGB32:
    _num_components = 4;
    break;

  case V4L2_PIX_FMT_GREY:
    _num_components = 1;
    break;

  default:
    vision_cat.error() << "Unsupported pixel format " << src->get_pixel_format() << "!\n";
    _ready = false;
    close(_fd);
    _fd = -1;
    return;
  }

  // Request a format of this size, and no interlacing
  _format.fmt.pix.width = _size_x;
  _format.fmt.pix.height = _size_y;
  _format.fmt.pix.field = V4L2_FIELD_NONE;

  // Now politely ask the driver to switch to this format
  if (-1 == ioctl(_fd, VIDIOC_S_FMT, &_format)) {
    vision_cat.error() << "Driver rejected format!\n";
    _ready = false;
    close(_fd);
    _fd = -1;
    return;
  }

  _size_x = _format.fmt.pix.width;
  _size_y = _format.fmt.pix.height;

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
  _buffers = (void **) calloc (req.count, sizeof (void*));
  _buflens = (size_t*) calloc (req.count, sizeof (size_t));

  if (!_buffers || !_buflens) {
    vision_cat.error() << "Not enough memory!\n";
  }

  // Set up the mmap buffers
  struct v4l2_buffer buf;
  for (unsigned int i = 0; i < (unsigned int)_bufcount; ++i) {
    memset(&buf, 0, sizeof buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (-1 == ioctl(_fd, VIDIOC_QUERYBUF, &buf)) {
      vision_cat.error() << "Failed to query buffer!\n";
    }

    _buflens[i] = buf.length;
    _buffers[i] = mmap (nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, buf.m.offset);

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
  if (_format.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) {
    jpeg_create_decompress(&_cinfo);

    _cinfo.src = (struct jpeg_source_mgr *)
      (*_cinfo.mem->alloc_small) ((j_common_ptr) &_cinfo, JPOOL_PERMANENT,
                                          sizeof(struct jpeg_source_mgr));
    // Set up function pointers
    _cinfo.src->init_source = my_init_source;
    _cinfo.src->fill_input_buffer = my_fill_input_buffer;
    _cinfo.src->skip_input_data = my_skip_input_data;
    _cinfo.src->resync_to_restart = jpeg_resync_to_restart;
    _cinfo.src->term_source = my_term_source;
  }
#endif
  _ready = true;
}

/**
 *
 */
WebcamVideoCursorV4L::
~WebcamVideoCursorV4L() {
#ifdef HAVE_JPEG
  if (_format.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) {
    jpeg_destroy_decompress(&_cinfo);
  }
#endif
  if (-1 != _fd) {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(_fd, VIDIOC_STREAMOFF, &type);
    close(_fd);
  }
  if (_buffers) {
    for (unsigned int i = 0; i < (unsigned int)_bufcount; ++i) {
      munmap(_buffers[i], _buflens[i]);
    }
    free(_buffers);
  }
  if (_buflens) {
    free(_buflens);
  }
}

/**
 *
 */
PT(MovieVideoCursor::Buffer) WebcamVideoCursorV4L::
fetch_buffer() {
  if (!_ready) {
    return nullptr;
  }

  PT(Buffer) buffer = get_standard_buffer();
  unsigned char *block = buffer->_block;
  struct v4l2_buffer vbuf;
  memset(&vbuf, 0, sizeof vbuf);
  vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  vbuf.memory = V4L2_MEMORY_MMAP;
  if (-1 == ioctl(_fd, VIDIOC_DQBUF, &vbuf) && errno != EIO) {
    if (errno == EAGAIN) {
      // Simply nothing is available yet.
      return nullptr;
    }
    vision_cat.error() << "Failed to dequeue buffer!\n";
    return nullptr;
  }
  nassertr(vbuf.index < _bufcount, nullptr);
  size_t bufsize = _buflens[vbuf.index];
  size_t old_bpl = _format.fmt.pix.bytesperline;
  size_t new_bpl = _size_x * _num_components;
  unsigned char *buf = (unsigned char *) _buffers[vbuf.index];

  switch (_format.fmt.pix.pixelformat) {
  case V4L2_PIX_FMT_MJPEG: {
#ifdef HAVE_JPEG
    struct my_error_mgr jerr;
    _cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    jerr.pub.output_message = my_output_message;

    unsigned char *newbuf = (unsigned char*) malloc(new_bpl * _size_y);

    // Establish the setjmp return context for my_error_exit to use
    if (setjmp(jerr.setjmp_buffer)) {
      jpeg_abort_decompress(&_cinfo);
    } else {
      // Set up data pointer
      _cinfo.src->bytes_in_buffer = bufsize;
      _cinfo.src->next_input_byte = buf;

      if (jpeg_read_header(&_cinfo, TRUE) == JPEG_HEADER_OK) {
        if (_cinfo.dc_huff_tbl_ptrs[0] == nullptr) {
          // Many MJPEG streams do not include huffman tables.  Remedy this.
          _cinfo.dc_huff_tbl_ptrs[0] = &dc_luminance_tbl;
          _cinfo.dc_huff_tbl_ptrs[1] = &dc_chrominance_tbl;
          _cinfo.ac_huff_tbl_ptrs[0] = &ac_luminance_tbl;
          _cinfo.ac_huff_tbl_ptrs[1] = &ac_chrominance_tbl;
        }

        _cinfo.scale_num = 1;
        _cinfo.scale_denom = 1;
        _cinfo.out_color_space = JCS_RGB;
        _cinfo.dct_method = JDCT_IFAST;

        if (jpeg_start_decompress(&_cinfo) && _cinfo.output_components == 3
          && _size_x == _cinfo.output_width && _size_y == _cinfo.output_height) {

          JSAMPLE *buffer_end = newbuf + new_bpl * _cinfo.output_height;
          JSAMPLE *rowptr = newbuf;
          while (_cinfo.output_scanline < _cinfo.output_height) {
            nassertd(rowptr + new_bpl <= buffer_end) break;
            jpeg_read_scanlines(&_cinfo, &rowptr, _cinfo.output_height);
            rowptr += new_bpl;
          }

          if (_cinfo.output_scanline < _cinfo.output_height) {
            jpeg_abort_decompress(&_cinfo);
          } else {
            jpeg_finish_decompress(&_cinfo);
          }
        }
      }
    }

    // Flip the image vertically
    for (int row = 0; row < _size_y; ++row) {
      memcpy(block + (_size_y - row - 1) * new_bpl, newbuf + row * new_bpl, new_bpl);
    }
    free(newbuf);

    // Swap red  blue
    unsigned char ex;
    for (size_t i = 0; i < new_bpl * _size_y; i += 3) {
      ex = block[i];
      block[i] = block[i + 2];
      block[i + 2] = ex;
    }
#else
    nassert_raise("JPEG support not compiled-in");
    return nullptr;
#endif
    break;
  }
  case V4L2_PIX_FMT_YUYV:
    for (size_t row = 0; row < _size_y; ++row) {
      size_t c = 0;
      for (size_t i = 0; i < old_bpl; i += 4) {
        yuyv_to_bgrbgr(block + (_size_y - row - 1) * new_bpl + c, buf + row * old_bpl + i);
        c += 6;
      }
    }
    break;

  case V4L2_PIX_FMT_BGR24:
  case V4L2_PIX_FMT_BGR32:
  case V4L2_PIX_FMT_GREY:
    // Simplest case: copying every row verbatim.
    nassertr(old_bpl == new_bpl, nullptr);

    for (size_t row = 0; row < _size_y; ++row) {
      memcpy(block + (_size_y - row - 1) * new_bpl, buf + row * old_bpl, new_bpl);
    }
    break;

  case V4L2_PIX_FMT_RGB24:
    // Swap components.
    nassertr(old_bpl == new_bpl, nullptr);

    for (size_t row = 0; row < _size_y; ++row) {
      for (size_t i = 0; i < old_bpl; i += 3) {
        rgb_to_bgr(block + (_size_y - row - 1) * old_bpl + i, buf + row * old_bpl + i);
      }
    }
    break;

  case V4L2_PIX_FMT_RGB32:
    // Swap components.
    nassertr(old_bpl == new_bpl, nullptr);

    for (size_t row = 0; row < _size_y; ++row) {
      for (size_t i = 0; i < old_bpl; i += 4) {
        rgb_to_bgra(block + (_size_y - row - 1) * old_bpl + i, buf + row * old_bpl + i + 1);
      }
    }
    break;
  }

  if (-1 == ioctl(_fd, VIDIOC_QBUF, &vbuf)) {
    vision_cat.error() << "Failed to exchange buffer with driver!\n";
  }

  return buffer;
}

#endif
