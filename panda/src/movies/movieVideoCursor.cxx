/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieVideoCursor.cxx
 * @author jyelon
 * @date 2007-07-02
 */

#include "movieVideoCursor.h"
#include "config_movies.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"

PStatCollector MovieVideoCursor::_copy_pcollector("*:Copy Video into Texture");
PStatCollector MovieVideoCursor::_copy_pcollector_ram("*:Copy Video into Texture:modify_ram_image");
PStatCollector MovieVideoCursor::_copy_pcollector_copy("*:Copy Video into Texture:copy");

TypeHandle MovieVideoCursor::_type_handle;
TypeHandle MovieVideoCursor::Buffer::_type_handle;

/**
 * This is a virtual base class and should not be created directly.  Instead,
 * create a more specialized class.
 */
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

/**
 *
 */
MovieVideoCursor::
~MovieVideoCursor() {
}

/**
 * Set up the specified Texture object to contain content from this movie.
 * This should be called once, not every frame.
 */
void MovieVideoCursor::
setup_texture(Texture *tex) const {
  int fullx = size_x();
  int fully = size_y();
  tex->adjust_this_size(fullx, fully, tex->get_name(), true);
  Texture::Format fmt;
  switch (get_num_components()) {
  case 1:
    fmt = Texture::F_luminance;
    break;
  case 2:
    fmt = Texture::F_luminance_alpha;
    break;
  default:
    fmt = Texture::F_rgb;
    break;
  case 4:
    fmt = Texture::F_rgba;
    break;
  }
  tex->setup_texture(Texture::TT_2d_texture, fullx, fully, 1, Texture::T_unsigned_byte, fmt);
  tex->set_pad_size(fullx - size_x(), fully - size_y());
}

/**
 * Updates the cursor to the indicated time.  If loop_count >= 1, the time is
 * clamped to the movie's length * loop_count.  If loop_count <= 0, the time
 * is understood to be modulo the movie's length.
 *
 * Returns true if a new frame is now available, false otherwise.  If this
 * returns true, you should immediately follow this with exactly *one* call to
 * fetch_buffer().
 *
 * If the movie reports that it can_seek, you may also specify a time value
 * less than the previous value you passed to set_time().  Otherwise, you may
 * only specify a time value greater than or equal to the previous value.
 *
 * If the movie reports that it can_seek, it doesn't mean that it can do so
 * quickly.  It may have to rewind the movie and then fast forward to the
 * desired location.  Only if can_seek_fast returns true can it seek rapidly.
 */
bool MovieVideoCursor::
set_time(double timestamp, int loop_count) {
  return true;
}

/**
 * Gets the current video frame (as specified by set_time()) from the movie
 * and returns it in a pre-allocated buffer.  You may simply let the buffer
 * dereference and delete itself when you are done with it.
 *
 * This may return NULL (even if set_time() returned true) if the frame is not
 * available for some reason.
 */
PT(MovieVideoCursor::Buffer) MovieVideoCursor::
fetch_buffer() {
  return nullptr;
}

/**
 * Stores this buffer's contents in the indicated texture.
 */
void MovieVideoCursor::
apply_to_texture(const Buffer *buffer, Texture *t, int page) {
  if (buffer == nullptr) {
    return;
  }

  PStatTimer timer(_copy_pcollector);

  nassertv(t->get_x_size() >= size_x());
  nassertv(t->get_y_size() >= size_y());
  nassertv((t->get_num_components() == 3) || (t->get_num_components() == 4) ||
           (t->get_num_components() == 1 && get_num_components() == 1) ||
           (t->get_num_components() == 2 && get_num_components() == 2));
  nassertv(t->get_component_width() == 1);
  nassertv(page < t->get_num_pages());

  PTA_uchar img;
  {
    PStatTimer timer2(_copy_pcollector_ram);
    t->set_keep_ram_image(true);
    img = t->modify_ram_image();
  }

  unsigned char *data = img.p() + page * t->get_expected_ram_page_size();

  PStatTimer timer2(_copy_pcollector_copy);
  if (t->get_x_size() == size_x() && t->get_num_components() == get_num_components()) {
    memcpy(data, buffer->_block, size_x() * size_y() * get_num_components());

  } else {
    unsigned char *p = buffer->_block;
    int src_width = get_num_components();
    int dst_width = t->get_num_components();
    if (src_width == dst_width) {
      int src_stride = src_width * size_x();
      int dst_stride = dst_width * t->get_x_size();
      for (int y=0; y<size_y(); y++) {
        memcpy(data, p, src_stride);
        data += dst_stride;
        p += src_stride;
      }
    } else {
      nassertv(src_width >= 3);
      nassertv(dst_width >= 3);
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
}

/**
 * Copies this buffer's contents into the alpha channel of the supplied
 * texture.  The RGB channels of the texture are not touched.
 */
void MovieVideoCursor::
apply_to_texture_alpha(const Buffer *buffer, Texture *t, int page, int alpha_src) {
  if (buffer == nullptr) {
    return;
  }

  PStatTimer timer(_copy_pcollector);

  // Is this a grayscale texture?
  if (get_num_components() < 3) {
    if (get_num_components() == 1 || alpha_src < 2 || alpha_src == 3) {
      // There's only one "RGB" channel to take from.
      alpha_src = 1;
    } else {
      // Alpha is actually in the second channel for grayscale-alpha.
      alpha_src = 2;
    }
  }

  nassertv(t->get_x_size() >= size_x());
  nassertv(t->get_y_size() >= size_y());
  nassertv(t->get_num_components() == 4 || t->get_num_components() == 2);
  nassertv(t->get_component_width() == 1);
  nassertv(page < t->get_z_size());
  nassertv((alpha_src >= 0) && (alpha_src <= get_num_components()));

  PTA_uchar img;
  {
    PStatTimer timer2(_copy_pcollector_ram);
    t->set_keep_ram_image(true);
    img = t->modify_ram_image();
  }

  unsigned char *data = img.p() + page * t->get_expected_ram_page_size();

  PStatTimer timer2(_copy_pcollector_copy);
  int src_width = get_num_components();
  int dst_width = t->get_num_components();
  int src_stride = size_x() * src_width;
  int dst_stride = t->get_x_size() * dst_width;
  unsigned char *p = buffer->_block;
  if (alpha_src == 0) {
    nassertv(src_width >= 3);
    for (int y=0; y<size_y(); y++) {
      for (int x=0; x<size_x(); x++) {
        unsigned char *pp = &p[x * src_width];
        data[(x + 1) * dst_width - 1] = (pp[0] + pp[1] + pp[2]) / 3;
      }
      data += dst_stride;
      p += src_stride;
    }
  } else {
    alpha_src -= 1;
    for (int y=0; y<size_y(); y++) {
      for (int x=0; x<size_x(); x++) {
        data[(x + 1) * dst_width - 1] = p[x * src_width + alpha_src];
      }
      data += dst_stride;
      p += src_stride;
    }
  }
}

/**
 * Copies this buffer's contents into the RGB channels of the supplied
 * texture.  The alpha channel of the texture is not touched.
 */
void MovieVideoCursor::
apply_to_texture_rgb(const Buffer *buffer, Texture *t, int page) {
  if (buffer == nullptr) {
    return;
  }

  PStatTimer timer(_copy_pcollector);

  nassertv(t->get_x_size() >= size_x());
  nassertv(t->get_y_size() >= size_y());
  nassertv(t->get_num_components() == 4 || t->get_num_components() == 2);
  nassertv(t->get_component_width() == 1);
  nassertv(page < t->get_z_size());

  PTA_uchar img;
  {
    PStatTimer timer2(_copy_pcollector_ram);
    t->set_keep_ram_image(true);
    img = t->modify_ram_image();
  }

  unsigned char *data = img.p() + page * t->get_expected_ram_page_size();

  PStatTimer timer2(_copy_pcollector_copy);
  int src_width = get_num_components();
  int dst_width = t->get_num_components();
  int src_stride = size_x() * src_width;
  int dst_stride = t->get_x_size() * dst_width;
  unsigned char *p = buffer->_block;
  if (src_width >= 3) {
    // It has RGB values.
    nassertv(dst_width >= 3);
    for (int y = 0; y < size_y(); ++y) {
      for (int x = 0; x < size_x(); ++x) {
        data[x * dst_width + 0] = p[x * src_width + 0];
        data[x * dst_width + 1] = p[x * src_width + 1];
        data[x * dst_width + 2] = p[x * src_width + 2];
      }
      data += dst_stride;
      p += src_stride;
    }
  } else if (dst_width == 4) {
    // It has only grayscale.
    for (int y = 0; y < size_y(); ++y) {
      for (int x = 0; x < size_x(); ++x) {
        unsigned char gray = p[x * src_width];
        data[x * dst_width + 0] = gray;
        data[x * dst_width + 1] = gray;
        data[x * dst_width + 2] = gray;
      }
      data += dst_stride;
      p += src_stride;
    }
  }
}

/**
 * May be called by a derived class to return a single standard Buffer object
 * to easily implement fetch_buffer().
 */
MovieVideoCursor::Buffer *MovieVideoCursor::
get_standard_buffer() {
  if (_standard_buffer == nullptr) {
    _standard_buffer = make_new_buffer();
  }
  return _standard_buffer;
}

/**
 * May be called by a derived class to allocate a new Buffer object.
 */
PT(MovieVideoCursor::Buffer) MovieVideoCursor::
make_new_buffer() {
  return new Buffer(size_x() * size_y() * get_num_components());
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void MovieVideoCursor::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  manager->write_pointer(dg, _source);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int MovieVideoCursor::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  _source = DCAST(MovieVideo, p_list[pi++]);

  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new MovieVideoCursor.
 */
void MovieVideoCursor::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  manager->read_pointer(scan);  // _source
}

/**
 *
 */
MovieVideoCursor::Buffer::
Buffer(size_t block_size) :
  _block_size(block_size)
{
  _deleted_chain = memory_hook->get_deleted_chain(_block_size);
  _block = (unsigned char *)_deleted_chain->allocate(_block_size, get_class_type());
}

/**
 *
 */
MovieVideoCursor::Buffer::
~Buffer() {
  _deleted_chain->deallocate(_block, get_class_type());
}

/**
 * Used to sort different buffers to ensure they correspond to the same source
 * frame, particularly important when synchronizing the different pages of a
 * multi-page texture.
 *
 * Returns 0 if the two buffers are of the same frame, <0 if this one comes
 * earlier than the other one, and >0 if the other one comes earlier.
 */
int MovieVideoCursor::Buffer::
compare_timestamp(const Buffer *other) const {
  return 0;
}

/**
 * Returns the nearest timestamp value of this particular buffer.  Ideally,
 * MovieVideoCursor::set_time() for this timestamp would return this buffer
 * again.  This need be defined only if compare_timestamp() is also defined.
 */
double MovieVideoCursor::Buffer::
get_timestamp() const {
  return 0.0;
}
