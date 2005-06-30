// Filename: geomVertexArrayData.cxx
// Created by:  drose (17Mar05)
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

#include "geomVertexArrayData.h"
#include "geom.h"
#include "preparedGraphicsObjects.h"
#include "reversedNumericData.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pset.h"

TypeHandle GeomVertexArrayData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::Default Constructor
//       Access: Private
//  Description: Constructs an invalid object.  This is only used when
//               reading from the bam file.
////////////////////////////////////////////////////////////////////
GeomVertexArrayData::
GeomVertexArrayData() {
  _endian_reversed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayData::
GeomVertexArrayData(const GeomVertexArrayFormat *array_format,
                    GeomVertexArrayData::UsageHint usage_hint) :
  _array_format(array_format)
{
  set_usage_hint(usage_hint);
  _endian_reversed = false;
  nassertv(_array_format->is_registered());
}
  
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayData::
GeomVertexArrayData(const GeomVertexArrayData &copy) :
  TypedWritableReferenceCount(copy),
  _array_format(copy._array_format),
  _cycler(copy._cycler)
{
  _endian_reversed = false;
  nassertv(_array_format->is_registered());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
operator = (const GeomVertexArrayData &copy) {
  TypedWritableReferenceCount::operator = (copy);
  _array_format = copy._array_format;
  _cycler = copy._cycler;

  CDWriter cdata(_cycler);
  cdata->_modified = Geom::get_next_modified();

  nassertv(_array_format->is_registered());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayData::
~GeomVertexArrayData() {
  release_all();
}


////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::set_num_rows
//       Access: Published
//  Description: Sets the length of the array to n rows.
//               Normally, you would not call this directly, since all
//               of the arrays in a particular GeomVertexData must
//               have the same number of rows; instead, call
//               GeomVertexData::set_num_rows().
//
//               The return value is true if the number of rows
//               was changed, false if the object already contained n
//               rows (or if there was some error).
//
//               The new vertex data is initialized to 0, including
//               the "color" column (but see
//               GeomVertexData::set_num_rows()).
////////////////////////////////////////////////////////////////////
bool GeomVertexArrayData::
set_num_rows(int n) {
  CDWriter cdata(_cycler);

  int stride = _array_format->get_stride();
  int delta = n - (cdata->_data.size() / stride);
  
  if (delta != 0) {
    if (cdata->_data.get_ref_count() > 1) {
      // Copy-on-write: the data is already reffed somewhere else,
      // so we're just going to make a copy.
      PTA_uchar new_data;
      new_data.reserve(n * stride);
      new_data.insert(new_data.end(), n * stride, 0);
      memcpy(new_data, cdata->_data, 
             min((size_t)(n * stride), cdata->_data.size()));
      cdata->_data = new_data;
      
    } else {
      // We've got the only reference to the data, so we can change
      // it directly.
      if (delta > 0) {
        cdata->_data.insert(cdata->_data.end(), delta * stride, 0);
        
      } else {
        cdata->_data.erase(cdata->_data.begin() + n * stride, 
                           cdata->_data.end());
      }
    }

    cdata->_modified = Geom::get_next_modified();

    return true;
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::set_usage_hint
//       Access: Published
//  Description: Changes the UsageHint hint for this array.  See
//               get_usage_hint().
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
set_usage_hint(GeomVertexArrayData::UsageHint usage_hint) {
  CDWriter cdata(_cycler);
  cdata->_usage_hint = usage_hint;
  cdata->_modified = Geom::get_next_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
output(ostream &out) const {
  out << get_num_rows() << " rows: " << *get_array_format();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
write(ostream &out, int indent_level) const {
  _array_format->write_with_data(out, indent_level, this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::modify_data
//       Access: Public
//  Description: Returns a modifiable pointer to the actual vertex
//               array, so that application code may directly
//               manipulate it.  Use with caution.
////////////////////////////////////////////////////////////////////
PTA_uchar GeomVertexArrayData::
modify_data() {
  // Perform copy-on-write: if the reference count on the vertex data
  // is greater than 1, assume some other GeomVertexData has the same
  // pointer, so make a copy of it first.
  CDWriter cdata(_cycler);

  if (cdata->_data.get_ref_count() > 1) {
    PTA_uchar orig_data = cdata->_data;
    cdata->_data = PTA_uchar();
    cdata->_data.v() = orig_data.v();
  }
  cdata->_modified = Geom::get_next_modified();

  return cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::set_data
//       Access: Public
//  Description: Replaces the vertex data array with a completely new
//               array.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
set_data(CPTA_uchar array) {
  CDWriter cdata(_cycler);
  cdata->_data = (PTA_uchar &)array;
  cdata->_modified = Geom::get_next_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::prepare
//       Access: Public
//  Description: Indicates that the data should be enqueued to be
//               prepared in the indicated prepared_objects at the
//               beginning of the next frame.  This will ensure the
//               data is already loaded into the GSG if it is expected
//               to be rendered soon.
//
//               Use this function instead of prepare_now() to preload
//               datas from a user interface standpoint.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_vertex_buffer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::prepare_now
//       Access: Public
//  Description: Creates a context for the data on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) VertexBufferContext.  This assumes that the
//               GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               datas.  If this is not necessarily the case, you
//               should use prepare() instead.
//
//               Normally, this is not called directly except by the
//               GraphicsStateGuardian; a data does not need to be
//               explicitly prepared by the user before it may be
//               rendered.
////////////////////////////////////////////////////////////////////
VertexBufferContext *GeomVertexArrayData::
prepare_now(PreparedGraphicsObjects *prepared_objects, 
            GraphicsStateGuardianBase *gsg) {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  VertexBufferContext *vbc = prepared_objects->prepare_vertex_buffer_now(this, gsg);
  if (vbc != (VertexBufferContext *)NULL) {
    _contexts[prepared_objects] = vbc;
  }
  return vbc;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::release
//       Access: Public
//  Description: Frees the data context only on the indicated object,
//               if it exists there.  Returns true if it was released,
//               false if it had not been prepared.
////////////////////////////////////////////////////////////////////
bool GeomVertexArrayData::
release(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    VertexBufferContext *vbc = (*ci).second;
    prepared_objects->release_vertex_buffer(vbc);
    return true;
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_vertex_buffer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::release_all
//       Access: Public
//  Description: Frees the context allocated on all objects for which
//               the data has been declared.  Returns the number of
//               contexts which have been freed.
////////////////////////////////////////////////////////////////////
int GeomVertexArrayData::
release_all() {
  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response
  // to each release_vertex_buffer(), and we don't want to be modifying the
  // _contexts list while we're traversing it.
  Contexts temp = _contexts;
  int num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    VertexBufferContext *vbc = (*ci).second;
    prepared_objects->release_vertex_buffer(vbc);
  }

  // Now that we've called release_vertex_buffer() on every known context,
  // the _contexts list should have completely emptied itself.
  nassertr(_contexts.empty(), num_freed);

  return num_freed;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::clear_prepared
//       Access: Private
//  Description: Removes the indicated PreparedGraphicsObjects table
//               from the data array's table, without actually
//               releasing the data array.  This is intended to be
//               called only from
//               PreparedGraphicsObjects::release_vertex_buffer(); it should
//               never be called by user code.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_prepared() was given a
    // prepared_objects which the data array didn't know about.
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::reverse_data_endianness
//       Access: Private
//  Description: Returns a new data array with all numeric values
//               expressed in the indicated array reversed,
//               byte-for-byte, to convert littleendian to bigendian
//               and vice-versa.
////////////////////////////////////////////////////////////////////
PTA_uchar GeomVertexArrayData::
reverse_data_endianness(const PTA_uchar &data) {
  // First, make a complete copy of the data.
  PTA_uchar new_data;
  new_data.v() = data.v();

  int num_columns = _array_format->get_num_columns();

  // Now, walk through each row of the data.
  unsigned char *begin = new_data;
  unsigned char *end = new_data + new_data.size();
  for (unsigned char *row_data = begin; 
       row_data < end; 
       row_data += _array_format->get_stride()) {
    nassertr(row_data + _array_format->get_stride() <= end, new_data);

    // For each row, visit all of the columns; and for each column,
    // visit all of the components of that column.
    for (int ci = 0; ci < num_columns; ++ci) {
      const GeomVertexColumn *col = _array_format->get_column(ci);
      int component_bytes = col->get_component_bytes();
      if (component_bytes > 1) {
        unsigned char *col_data = row_data + col->get_start();
        int num_components = col->get_num_components();
        for (int cj = 0; cj < num_components; ++cj) {
          // Reverse the bytes of each component.
          ReversedNumericData nd(col_data, component_bytes);
          nd.store_value(col_data, component_bytes);
          col_data += component_bytes;
        }
      }
    }
  }
  
  return new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               GeomVertexArrayData.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  manager->write_pointer(dg, _array_format);
  manager->write_cdata(dg, _cycler, this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::write_raw_data
//       Access: Public
//  Description: Called by CData::write_datagram to write the raw data
//               of the array to the indicated datagram.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
write_raw_data(BamWriter *manager, Datagram &dg, const PTA_uchar &data) {
  dg.add_uint32(data.size());

  if (manager->get_file_endian() == BE_native) {
    // For native endianness, we only have to write the data directly.
    dg.append_data(data, data.size());

  } else {
    // Otherwise, we have to convert it.
    PTA_uchar new_data = reverse_data_endianness(data);
    dg.append_data(new_data, new_data.size());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::read_raw_data
//       Access: Public
//  Description: Called by CData::fillin to read the raw data
//               of the array from the indicated datagram.
////////////////////////////////////////////////////////////////////
PTA_uchar GeomVertexArrayData::
read_raw_data(BamReader *manager, DatagramIterator &scan) {
  size_t size = scan.get_uint32();
  PTA_uchar data = PTA_uchar::empty_array(size);
  const unsigned char *source_data = 
    (const unsigned char *)scan.get_datagram().get_data();
  memcpy(data, source_data + scan.get_current_index(), size);
  scan.skip_bytes(size);

  if (manager->get_file_endian() != BE_native) {
    // For non-native endian files, we have to convert the data.  
    if (_array_format == (GeomVertexArrayFormat *)NULL) {
      // But we can't do that until we've completed the _array_format
      // pointer, which tells us how to convert it.
      _endian_reversed = true;
    } else {
      // Since we have the _array_format pointer now, we can reverse
      // it immediately (and we should, to support threaded CData
      // updates).
      data = reverse_data_endianness(data);
    }
  }

  return data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int GeomVertexArrayData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  _array_format = DCAST(GeomVertexArrayFormat, p_list[pi++]);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
finalize(BamReader *manager) {
  // Now we need to register the format that we have read from the bam
  // file (since it doesn't come out of the bam file automatically
  // registered).  This may change the format's pointer, which we
  // should then update our own data to reflect.  But since this may
  // cause the unregistered object to destruct, we have to also tell
  // the BamReader to return the new object from now on.

  CDWriter cdata(_cycler);

  CPT(GeomVertexArrayFormat) new_array_format = 
    GeomVertexArrayFormat::register_format(_array_format);

  manager->change_pointer(_array_format, new_array_format);
  _array_format = new_array_format;

  if (_endian_reversed) {
    // Now is the time to endian-reverse the data.
    cdata->_data = reverse_data_endianness(cdata->_data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type GeomVertexArrayData is encountered
//               in the Bam file.  It should create the GeomVertexArrayData
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomVertexArrayData::
make_from_bam(const FactoryParams &params) {
  GeomVertexArrayData *object = new GeomVertexArrayData;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);
  manager->register_finalize(object);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomVertexArrayData.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  manager->read_pointer(scan);
  manager->read_cdata(scan, _cycler, this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *GeomVertexArrayData::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::CData::
write_datagram(BamWriter *manager, Datagram &dg, void *extra_data) const {
  GeomVertexArrayData *array_data = (GeomVertexArrayData *)extra_data;
  dg.add_uint8(_usage_hint);
  WRITE_PTA(manager, dg, array_data->write_raw_data, _data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomVertexArrayData.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayData::CData::
fillin(DatagramIterator &scan, BamReader *manager, void *extra_data) {
  GeomVertexArrayData *array_data = (GeomVertexArrayData *)extra_data;
  _usage_hint = (UsageHint)scan.get_uint8();
  READ_PTA(manager, scan, array_data->read_raw_data, _data);

  _modified = Geom::get_next_modified();
}
