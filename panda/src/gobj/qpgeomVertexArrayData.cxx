// Filename: qpgeomVertexArrayData.cxx
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

#include "qpgeomVertexArrayData.h"
#include "qpgeom.h"
#include "preparedGraphicsObjects.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pset.h"

TypeHandle qpGeomVertexArrayData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::Default Constructor
//       Access: Private
//  Description: Constructs an invalid object.  This is only used when
//               reading from the bam file.
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayData::
qpGeomVertexArrayData() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayData::
qpGeomVertexArrayData(const qpGeomVertexArrayFormat *array_format,
                      qpGeomVertexArrayData::UsageHint usage_hint) :
  _array_format(array_format)
{
  set_usage_hint(usage_hint);
  nassertv(_array_format->is_registered());
}
  
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayData::
qpGeomVertexArrayData(const qpGeomVertexArrayData &copy) :
  TypedWritableReferenceCount(copy),
  _array_format(copy._array_format),
  _cycler(copy._cycler)
{
  nassertv(_array_format->is_registered());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
operator = (const qpGeomVertexArrayData &copy) {
  TypedWritableReferenceCount::operator = (copy);
  _array_format = copy._array_format;
  _cycler = copy._cycler;

  CDWriter cdata(_cycler);
  cdata->_modified = qpGeom::get_next_modified();

  nassertv(_array_format->is_registered());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayData::
~qpGeomVertexArrayData() {
  release_all();
}


////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::set_num_rows
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
bool qpGeomVertexArrayData::
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

    cdata->_modified = qpGeom::get_next_modified();

    return true;
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::set_usage_hint
//       Access: Published
//  Description: Changes the UsageHint hint for this array.  See
//               get_usage_hint().
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
set_usage_hint(qpGeomVertexArrayData::UsageHint usage_hint) {
  CDWriter cdata(_cycler);
  cdata->_usage_hint = usage_hint;
  cdata->_modified = qpGeom::get_next_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
output(ostream &out) const {
  out << get_num_rows() << " rows: " << *get_array_format();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
write(ostream &out, int indent_level) const {
  _array_format->write_with_data(out, indent_level, this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::modify_data
//       Access: Public
//  Description: Returns a modifiable pointer to the actual vertex
//               array, so that application code may directly
//               manipulate it.  Use with caution.
////////////////////////////////////////////////////////////////////
PTA_uchar qpGeomVertexArrayData::
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
  cdata->_modified = qpGeom::get_next_modified();

  return cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::set_data
//       Access: Public
//  Description: Replaces the vertex data array with a completely new
//               array.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
set_data(CPTA_uchar array) {
  CDWriter cdata(_cycler);
  cdata->_data = (PTA_uchar &)array;
  cdata->_modified = qpGeom::get_next_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::prepare
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
void qpGeomVertexArrayData::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_vertex_buffer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::prepare_now
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
VertexBufferContext *qpGeomVertexArrayData::
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
//     Function: qpGeomVertexArrayData::release
//       Access: Public
//  Description: Frees the data context only on the indicated object,
//               if it exists there.  Returns true if it was released,
//               false if it had not been prepared.
////////////////////////////////////////////////////////////////////
bool qpGeomVertexArrayData::
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
//     Function: qpGeomVertexArrayData::release_all
//       Access: Public
//  Description: Frees the context allocated on all objects for which
//               the data has been declared.  Returns the number of
//               contexts which have been freed.
////////////////////////////////////////////////////////////////////
int qpGeomVertexArrayData::
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
//     Function: qpGeomVertexArrayData::clear_prepared
//       Access: Private
//  Description: Removes the indicated PreparedGraphicsObjects table
//               from the data array's table, without actually
//               releasing the data array.  This is intended to be
//               called only from
//               PreparedGraphicsObjects::release_vertex_buffer(); it should
//               never be called by user code.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
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
//     Function: qpGeomVertexArrayData::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeomVertexArrayData.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  manager->write_pointer(dg, _array_format);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::write_raw_data
//       Access: Public, Static
//  Description: Called by CData::write_datagram to write the raw data
//               of the array to the indicated datagram.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
write_raw_data(Datagram &dg, const PTA_uchar &data) {
  // TODO: account for endianness of host.
  dg.add_uint32(data.size());
  dg.append_data(data, data.size());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::read_raw_data
//       Access: Public, Static
//  Description: Called by CData::fillin to read the raw data
//               of the array from the indicated datagram.
////////////////////////////////////////////////////////////////////
PTA_uchar qpGeomVertexArrayData::
read_raw_data(DatagramIterator &scan) {
  // TODO: account for endianness of host.

  size_t size = scan.get_uint32();
  PTA_uchar data = PTA_uchar::empty_array(size);
  const unsigned char *source_data = 
    (const unsigned char *)scan.get_datagram().get_data();
  memcpy(data, source_data + scan.get_current_index(), size);
  scan.skip_bytes(size);

  return data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int qpGeomVertexArrayData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  _array_format = DCAST(qpGeomVertexArrayFormat, p_list[pi++]);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
finalize(BamReader *manager) {
  // Now we need to register the format that we have read from the bam
  // file (since it doesn't come out of the bam file automatically
  // registered).  This may change the format's pointer, which we
  // should then update our own data to reflect.  But since this may
  // cause the unregistered object to destruct, we have to also tell
  // the BamReader to return the new object from now on.

  CDWriter cdata(_cycler);

  CPT(qpGeomVertexArrayFormat) new_array_format = 
    qpGeomVertexArrayFormat::register_format(_array_format);

  manager->change_pointer(_array_format, new_array_format);
  _array_format = new_array_format;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeomVertexArrayData is encountered
//               in the Bam file.  It should create the qpGeomVertexArrayData
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomVertexArrayData::
make_from_bam(const FactoryParams &params) {
  qpGeomVertexArrayData *object = new qpGeomVertexArrayData;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);
  manager->register_finalize(object);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomVertexArrayData.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  manager->read_pointer(scan);

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *qpGeomVertexArrayData::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_uint8(_usage_hint);
  WRITE_PTA(manager, dg, write_raw_data, _data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomVertexArrayData.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayData::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _usage_hint = (UsageHint)scan.get_uint8();
  READ_PTA(manager, scan, read_raw_data, _data);

  _modified = qpGeom::get_next_modified();
}
