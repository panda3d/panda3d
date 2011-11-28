// Filename: movieVideo.cxx
// Created by: jyelon (02Jul07)
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

#include "movieVideoCursor.h"
#include "config_movies.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle MovieVideoCursor::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::Default Constructor
//       Access: Protected
//  Description: This is a virtual base class and should not be
//               created directly.  Instead, create a more specialized
//               class.
////////////////////////////////////////////////////////////////////
MovieVideoCursor::
MovieVideoCursor(MovieVideo *src) :
  _source(src),
  _size_x(1),
  _size_y(1),
  _num_components(3),
  _length(1.0E10),
  _can_seek(true),
  _can_seek_fast(true),
  _aborted(false),
  _streaming(false),
  _ready(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MovieVideoCursor::
~MovieVideoCursor() {
}
  
////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::setup_texture
//       Access: Published
//  Description: Set up the specified Texture object to contain
//               content from this movie.  This should be called
//               once, not every frame.
////////////////////////////////////////////////////////////////////
void MovieVideoCursor::
setup_texture(Texture *tex) const {
  int fullx = size_x();
  int fully = size_y();
  tex->adjust_this_size(fullx, fully, tex->get_name(), true);
  Texture::Format fmt = (get_num_components() == 4) ? Texture::F_rgba : Texture::F_rgb;
  tex->setup_texture(Texture::TT_2d_texture, fullx, fully, 1, Texture::T_unsigned_byte, fmt);
  tex->set_pad_size(fullx - size_x(), fully - size_y());
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::set_time
//       Access: Published, Virtual
//  Description: Updates the cursor to the indicated time.  If
//               loop_count >= 1, the time is clamped to the movie's
//               length * loop_count.  If loop_count <= 0, the time is
//               understood to be modulo the movie's length.

//               Returns true if a new frame is now available, false
//               otherwise.  If this returns true, you should
//               immediately follow this with exactly *one* call to
//               one of the fetch_*() methods.
////////////////////////////////////////////////////////////////////
bool MovieVideoCursor::
set_time(double time, int loop_count) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::fetch_into_bitbucket
//       Access: Published, Virtual
//  Description: Discards the next video frame.
//
//               See fetch_buffer for more details.
////////////////////////////////////////////////////////////////////
void MovieVideoCursor::
fetch_into_bitbucket() {

  // This generic implementation is layered on fetch_buffer.
  // It will work for any derived class, so it is never necessary to
  // redefine this.  It is probably possible to make a faster
  // implementation, but since this function is rarely used, it
  // probably isn't worth the trouble.

  PT(Buffer) buffer = fetch_buffer();
  if (buffer != NULL) {
    release_buffer(buffer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::fetch_into_texture
//       Access: Published, Virtual
//  Description: Reads the specified video frame into 
//               the specified texture.
//
//               See fetch_buffer for more details.
////////////////////////////////////////////////////////////////////
void MovieVideoCursor::
fetch_into_texture(Texture *t, int page) {
  static PStatCollector fetch_into_texture_collector("*:Decode Video into Texture");
  PStatTimer timer(fetch_into_texture_collector);

  // This generic implementation is layered on fetch_buffer.
  // It will work for any derived class, so it is never necessary to
  // redefine this.  However, it may be possible to make a faster
  // implementation that uses fewer intermediate copies, depending
  // on the capabilities of the underlying codec software.

  nassertv(t->get_x_size() >= size_x());
  nassertv(t->get_y_size() >= size_y());
  nassertv((t->get_num_components() == 3) || (t->get_num_components() == 4));
  nassertv(t->get_component_width() == 1);
  nassertv(page < t->get_num_pages());
  
  t->set_keep_ram_image(true);
  PTA_uchar img = t->modify_ram_image();
  
  unsigned char *data = img.p() + page * t->get_expected_ram_page_size();

  PT(Buffer) buffer = fetch_buffer();
  if (buffer == NULL) {
    // No image available.
    return;
  }

  if (t->get_x_size() == size_x() && t->get_num_components() == get_num_components()) {
    memcpy(data, buffer->_block, size_x() * size_y() * get_num_components());

  } else {
    unsigned char *p = buffer->_block;
    if (t->get_num_components() == get_num_components()) {
      int src_stride = size_x() * get_num_components();
      int dst_stride = t->get_x_size() * t->get_num_components();
      for (int y=0; y<size_y(); y++) {
        memcpy(data, p, src_stride);
        data += dst_stride;
        p += src_stride;
      }
    } else {
      int src_width = get_num_components();
      int dst_width = t->get_num_components();
      for (int y = 0; y < size_y(); ++y) {
        for (int x = 0; x < size_x(); ++x) {
          data[0] = p[0];
          data[1] = p[1];
          data[2] = p[2];
          data += dst_width;
          p += src_width;
        }
      }
    }
  }
  release_buffer(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::fetch_into_texture_alpha
//       Access: Published, Virtual
//  Description: Reads the specified video frame into 
//               the alpha channel of the supplied texture.  The
//               RGB channels of the texture are not touched.
//
//               See fetch_buffer for more details.
////////////////////////////////////////////////////////////////////
void MovieVideoCursor::
fetch_into_texture_alpha(Texture *t, int page, int alpha_src) {

  // This generic implementation is layered on fetch_buffer.
  // It will work for any derived class, so it is never necessary to
  // redefine this.  However, it may be possible to make a faster
  // implementation that uses fewer intermediate copies, depending
  // on the capabilities of the underlying codec software.

  nassertv(t->get_x_size() >= size_x());
  nassertv(t->get_y_size() >= size_y());
  nassertv(t->get_num_components() == 4);
  nassertv(t->get_component_width() == 1);
  nassertv(page < t->get_z_size());
  nassertv((alpha_src >= 0) && (alpha_src <= get_num_components()));

  PT(Buffer) buffer = fetch_buffer();
  if (buffer == NULL) {
    // No image available.
    return;
  }

  t->set_keep_ram_image(true);
  PTA_uchar img = t->modify_ram_image();
  
  unsigned char *data = img.p() + page * t->get_expected_ram_page_size();
  
  int src_stride = size_x() * get_num_components();
  int dst_stride = t->get_x_size() * 4;
  if (alpha_src == 0) {
    unsigned char *p = buffer->_block;
    for (int y=0; y<size_y(); y++) {
      for (int x=0; x<size_x(); x++) {
        data[x*4+3] = (p[x*4+0] + p[x*4+1] + p[x*4+2]) / 3;
      }
      data += dst_stride;
      p += src_stride;
    }
  } else {
    alpha_src -= 1;
    unsigned char *p = buffer->_block;
    int src_width = get_num_components();
    for (int y=0; y<size_y(); y++) {
      for (int x=0; x<size_x(); x++) {
        data[x*4+3] = p[x *src_width + alpha_src];
      }
      data += dst_stride;
      p += src_stride;
    }
  }

  release_buffer(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::fetch_into_texture_rgb
//       Access: Published, Virtual
//  Description: Reads the specified video frame into
//               the RGB channels of the supplied texture.  The alpha
//               channel of the texture is not touched.
//
//               See fetch_buffer for more details.
////////////////////////////////////////////////////////////////////
void MovieVideoCursor::
fetch_into_texture_rgb(Texture *t, int page) {

  // This generic implementation is layered on fetch_buffer.
  // It will work for any derived class, so it is never necessary to
  // redefine this.  However, it may be possible to make a faster
  // implementation that uses fewer intermediate copies, depending
  // on the capabilities of the underlying codec software.

  nassertv(t->get_x_size() >= size_x());
  nassertv(t->get_y_size() >= size_y());
  nassertv(t->get_num_components() == 4);
  nassertv(t->get_component_width() == 1);
  nassertv(page < t->get_z_size());

  PT(Buffer) buffer = fetch_buffer();
  if (buffer == NULL) {
    // No image available.
    return;
  }

  t->set_keep_ram_image(true);
  PTA_uchar img = t->modify_ram_image();
  
  unsigned char *data = img.p() + page * t->get_expected_ram_page_size();
  
  int src_stride = size_x() * get_num_components();
  int src_width = get_num_components();
  int dst_stride = t->get_x_size() * 4;
  unsigned char *p = buffer->_block;
  for (int y=0; y<size_y(); y++) {
    for (int x=0; x<size_x(); x++) {
      data[x * 4 + 0] = p[x * src_width + 0];
      data[x * 4 + 1] = p[x * src_width + 1];
      data[x * 4 + 2] = p[x * src_width + 2];
    }
    data += dst_stride;
    p += src_stride;
  }

  release_buffer(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::fetch_buffer
//       Access: Published, Virtual
//  Description: Reads the specified video frame and returns it in a
//               pre-allocated buffer.  After you have copied the data
//               from the buffer, you should call release_buffer() to
//               make the space available again to populate the next
//               frame.  You may not call fetch_buffer() again until
//               you have called release_buffer().
//
//               If the movie reports that it can_seek, you may
//               also specify a timestamp less than next_start.
//               Otherwise, you may only specify a timestamp
//               greater than or equal to next_start.
//
//               If the movie reports that it can_seek, it doesn't
//               mean that it can do so quickly.  It may have to
//               rewind the movie and then fast forward to the
//               desired location.  Only if can_seek_fast returns
//               true can it seek rapidly.
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor::Buffer) MovieVideoCursor::
fetch_buffer() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::release_buffer
//       Access: Public, Virtual
//  Description: Should be called after processing the Buffer object
//               returned by fetch_buffer(), this releases the Buffer
//               for future use again.
////////////////////////////////////////////////////////////////////
void MovieVideoCursor::
release_buffer(Buffer *buffer) {
  nassertv(buffer == _standard_buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::get_standard_buffer
//       Access: Protected
//  Description: May be called by a derived class to return a single
//               standard Buffer object to easily implement
//               fetch_buffer().  The default release_buffer()
//               implementation assumes this method is used.
////////////////////////////////////////////////////////////////////
MovieVideoCursor::Buffer *MovieVideoCursor::
get_standard_buffer() {
  if (_standard_buffer == NULL) {
    _standard_buffer = make_new_buffer();
  }
  return _standard_buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::make_new_buffer
//       Access: Protected, Virtual
//  Description: May be called by a derived class to allocate a new
//               Buffer object.
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor::Buffer) MovieVideoCursor::
make_new_buffer() {
  return new Buffer(size_x() * size_y() * get_num_components());
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void MovieVideoCursor::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  manager->write_pointer(dg, _source);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int MovieVideoCursor::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  _source = DCAST(MovieVideo, p_list[pi++]);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideoCursor::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new MovieVideoCursor.
////////////////////////////////////////////////////////////////////
void MovieVideoCursor::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  manager->read_pointer(scan);  // _source
}
