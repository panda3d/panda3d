/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texturePosition.cxx
 * @author drose
 * @date 2000-12-04
 */

#include "texturePosition.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle TexturePosition::_type_handle;

/**
 *
 */
TexturePosition::
TexturePosition() {
  _margin = 0;
  _x = 0;
  _y = 0;
  _x_size = 0;
  _y_size = 0;
  _min_uv.set(0.0, 0.0);
  _max_uv.set(0.0, 0.0);
  _wrap_u = EggTexture::WM_unspecified;
  _wrap_v = EggTexture::WM_unspecified;
}

/**
 *
 */
TexturePosition::
TexturePosition(const TexturePosition &copy) :
  _margin(copy._margin),
  _x(copy._x),
  _y(copy._y),
  _x_size(copy._x_size),
  _y_size(copy._y_size),
  _min_uv(copy._min_uv),
  _max_uv(copy._max_uv),
  _wrap_u(copy._wrap_u),
  _wrap_v(copy._wrap_v)
{
}

/**
 *
 */
void TexturePosition::
operator = (const TexturePosition &copy) {
  _margin = copy._margin;
  _x = copy._x;
  _y = copy._y;
  _x_size = copy._x_size;
  _y_size = copy._y_size;
  _min_uv = copy._min_uv;
  _max_uv = copy._max_uv;
  _wrap_u = copy._wrap_u;
  _wrap_v = copy._wrap_v;
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void TexturePosition::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_from_bam);
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void TexturePosition::
write_datagram(BamWriter *writer, Datagram &datagram) {
  TypedWritable::write_datagram(writer, datagram);
  datagram.add_int32(_margin);
  datagram.add_int32(_x);
  datagram.add_int32(_y);
  datagram.add_int32(_x_size);
  datagram.add_int32(_y_size);
  datagram.add_float64(_min_uv[0]);
  datagram.add_float64(_min_uv[1]);
  datagram.add_float64(_max_uv[0]);
  datagram.add_float64(_max_uv[1]);
  datagram.add_int32((int)_wrap_u);
  datagram.add_int32((int)_wrap_v);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 */
TypedWritable *TexturePosition::
make_from_bam(const FactoryParams &params) {
  TexturePosition *me = new TexturePosition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Reads the binary data from the given datagram iterator, which was written
 * by a previous call to write_datagram().
 */
void TexturePosition::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  _margin = scan.get_int32();
  _x = scan.get_int32();
  _y = scan.get_int32();
  _x_size = scan.get_int32();
  _y_size = scan.get_int32();
  _min_uv[0] = scan.get_float64();
  _min_uv[1] = scan.get_float64();
  _max_uv[0] = scan.get_float64();
  _max_uv[1] = scan.get_float64();
  _wrap_u = (EggTexture::WrapMode)scan.get_int32();
  _wrap_v = (EggTexture::WrapMode)scan.get_int32();
}
