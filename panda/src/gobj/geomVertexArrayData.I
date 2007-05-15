// Filename: geomVertexArrayData.I
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


////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_array_format
//       Access: Published
//  Description: Returns the format object that describes this array.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayFormat *GeomVertexArrayData::
get_array_format() const {
  return _array_format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_usage_hint
//       Access: Published
//  Description: Returns the usage hint that describes to the
//               rendering backend how often the vertex data will be
//               modified and/or rendered.  See geomEnums.h.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData::UsageHint GeomVertexArrayData::
get_usage_hint() const {
  CDReader cdata(_cycler);
  return cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::has_column
//       Access: Published
//  Description: Returns true if the array has the named column,
//               false otherwise.  This is really just a shortcut for
//               asking the same thing from the format.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexArrayData::
has_column(const InternalName *name) const {
  return _array_format->has_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_num_rows
//       Access: Published
//  Description: Returns the number of rows stored in the array,
//               based on the number of bytes and the stride.  This
//               should be the same for all arrays within a given
//               GeomVertexData object.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexArrayData::
get_num_rows() const {
  return get_handle()->get_num_rows();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::set_num_rows
//       Access: Published
//  Description: Sets the length of the array to n rows.
//
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
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexArrayData::
set_num_rows(int n) {
  return modify_handle()->set_num_rows(n);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::unclean_set_num_rows
//       Access: Published
//  Description: This method behaves like set_num_rows(), except the
//               new data is not initialized.  Furthermore, after this
//               call, *any* of the data in the GeomVertexArrayData
//               may be uninitialized, including the earlier rows.
//
//               Normally, you would not call this directly, since all
//               of the arrays in a particular GeomVertexData must
//               have the same number of rows; instead, call
//               GeomVertexData::unclean_set_num_rows().
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexArrayData::
unclean_set_num_rows(int n) {
  return modify_handle()->unclean_set_num_rows(n);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::clear_rows
//       Access: Published
//  Description: Removes all of the rows in the array.
//               Functionally equivalent to set_num_rows(0).
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexArrayData::
clear_rows() {
  return modify_handle()->clear_rows();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_data_size_bytes
//       Access: Published
//  Description: Returns the number of bytes stored in the array.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexArrayData::
get_data_size_bytes() const {
  CDReader cdata(_cycler);
  return cdata->_data_full_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the array vertex data is
//               modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq GeomVertexArrayData::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_handle
//       Access: Published
//  Description: Returns an object that can be used to read the actual
//               data bytes stored in the array.  Calling this method
//               locks the data, and will block any other threads
//               attempting to read or write the data, until the
//               returned object destructs.
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexArrayDataHandle) GeomVertexArrayData::
get_handle(Thread *current_thread) const {
  return new GeomVertexArrayDataHandle(this, current_thread, 
                                       _cycler.read_unlocked(current_thread),
                                       false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::modify_handle
//       Access: Published
//  Description: Returns an object that can be used to read or write
//               the actual data bytes stored in the array.  Calling
//               this method locks the data, and will block any other
//               threads attempting to read or write the data, until
//               the returned object destructs.
////////////////////////////////////////////////////////////////////
INLINE PT(GeomVertexArrayDataHandle) GeomVertexArrayData::
modify_handle(Thread *current_thread) {
  return new GeomVertexArrayDataHandle(this, current_thread, 
                                       _cycler.write_upstream(true, current_thread),
                                       true);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_ram_class
//       Access: Published
//  Description: Returns the current ram class of the array.  If this
//               is other than RC_resident, the array data is not
//               resident in memory.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData::RamClass GeomVertexArrayData::
get_ram_class() const {
  return _ram_class;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_global_lru
//       Access: Published, Static
//  Description: Returns a pointer to the global LRU object that
//               manages the GeomVertexArrayData's with the indicated
//               RamClass.
////////////////////////////////////////////////////////////////////
INLINE SimpleLru *GeomVertexArrayData::
get_global_lru(RamClass rclass) {
  nassertr(rclass >= 0 && rclass < RC_end_of_list, NULL);
  return _global_lru[rclass];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_save_file
//       Access: Published, Static
//  Description: Returns the global VertexDataSaveFile that will be
//               used to save vertex data buffers to disk when
//               necessary.
////////////////////////////////////////////////////////////////////
INLINE VertexDataSaveFile *GeomVertexArrayData::
get_save_file() {
  if (_save_file == (VertexDataSaveFile *)NULL) {
    make_save_file();
  }
  return _save_file;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::set_ram_class
//       Access: Private
//  Description: Puts the data in a new ram class.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexArrayData::
set_ram_class(RamClass rclass) {
  _ram_class = rclass;
  mark_used_lru(_global_lru[rclass]);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData::CData::
CData() :
  _usage_hint(UH_unspecified),
  _data(GeomVertexArrayData::get_class_type()),
  _data_full_size(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData::CData::
CData(const GeomVertexArrayData::CData &copy) :
  _usage_hint(copy._usage_hint),
  _data(copy._data),
  _data_full_size(copy._data_full_size),
  _modified(copy._modified)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::CData::Copy Assignment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexArrayData::CData::
operator = (const GeomVertexArrayData::CData &copy) {
  _usage_hint = copy._usage_hint;
  _modified = copy._modified;
  _data = copy._data;
  _data_full_size = copy._data_full_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataHandle::
GeomVertexArrayDataHandle(const GeomVertexArrayData *object, 
                          Thread *current_thread,
                          const GeomVertexArrayData::CData *cdata,
                          bool writable) :
  _holder(((GeomVertexArrayData::CData *)cdata)->_rw_lock),
  _object((GeomVertexArrayData *)object),
  _current_thread(current_thread),
  _cdata((GeomVertexArrayData::CData *)cdata),
  _writable(writable)
{
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::Copy Constructor
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataHandle::
GeomVertexArrayDataHandle(const GeomVertexArrayDataHandle &copy) :
  _holder(copy._cdata->_rw_lock)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::Copy Assignment Operator
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexArrayDataHandle::
operator = (const GeomVertexArrayDataHandle &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataHandle::
~GeomVertexArrayDataHandle() {
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG

  if (_writable) {
    _object->_cycler.release_write(_cdata);
  }

#ifdef DO_PIPELINING
  unref_delete((CycleData *)_cdata);
#endif  // DO_PIPELINING

#ifdef _DEBUG
  _object = NULL;
  _cdata = NULL;
#endif  // _DEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::get_current_thread
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Thread *GeomVertexArrayDataHandle::
get_current_thread() const {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayData *GeomVertexArrayDataHandle::
get_object() const {
  return _object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData *GeomVertexArrayDataHandle::
get_object() {
  return _object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::get_pointer
//       Access: Public
//  Description: Returns a pointer to the beginning of the actual data
//               stream.
////////////////////////////////////////////////////////////////////
INLINE const unsigned char *GeomVertexArrayDataHandle::
get_pointer() const {
  check_resident();
  return &_cdata->_data[0];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::get_pointer
//       Access: Public
//  Description: Returns a pointer to the beginning of the actual data
//               stream.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *GeomVertexArrayDataHandle::
get_pointer() {
  nassertr(_writable, NULL);
  check_resident();
  if (_cdata->_data.size() == 0) {
    return NULL;
  }
  return &_cdata->_data[0];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::get_array_format
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayFormat *GeomVertexArrayDataHandle::
get_array_format() const {
  return _object->_array_format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::get_usage_hint
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataHandle::UsageHint GeomVertexArrayDataHandle::
get_usage_hint() const {
  return _cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::get_num_rows
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexArrayDataHandle::
get_num_rows() const {
  return get_data_size_bytes() / _object->_array_format->get_stride();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::clear_rows
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexArrayDataHandle::
clear_rows() {
  set_num_rows(0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::get_data_size_bytes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexArrayDataHandle::
get_data_size_bytes() const {
  return _cdata->_data_full_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::get_modified
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq GeomVertexArrayDataHandle::
get_modified() const {
  return _cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataHandle::check_resident
//       Access: Published
//  Description: Forces the vertex data into system RAM, if it is not
//               already there; also, marks it recently-used.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayDataHandle::
check_resident() const {
  if (_object->get_ram_class() != RC_resident) {
    _object->make_resident();
  } else {
    _object->mark_used_lru();
  }
  nassertv(_cdata->_data.size() == _cdata->_data_full_size);
}

INLINE ostream &
operator << (ostream &out, const GeomVertexArrayData &obj) {
  obj.output(out);
  return out;
}
