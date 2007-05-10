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
  GeomVertexArrayDataPipelineReader reader(this, Thread::get_current_thread());
  return reader.get_num_rows();
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
  GeomVertexArrayDataPipelineWriter writer(this, true, Thread::get_current_thread());
  return writer.set_num_rows(n);
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
  GeomVertexArrayDataPipelineWriter writer(this, true, Thread::get_current_thread());
  return writer.unclean_set_num_rows(n);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::clear_rows
//       Access: Published
//  Description: Removes all of the rows in the array.
//               Functionally equivalent to set_num_rows(0).
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexArrayData::
clear_rows() {
  set_data(PTA_uchar(GeomVertexArrayData::get_class_type()));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_data_size_bytes
//       Access: Published
//  Description: Returns the number of bytes stored in the array.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexArrayData::
get_data_size_bytes() const {
  CDReader cdata(_cycler);
  return cdata->_data.size();
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
//     Function: GeomVertexArrayData::get_data
//       Access: Published
//  Description: Returns a const pointer to the actual vertex data,
//               for application code to directly examine (but not
//               modify).
////////////////////////////////////////////////////////////////////
INLINE CPTA_uchar GeomVertexArrayData::
get_data() const {
  CDReader cdata(_cycler);
  return cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::modify_data
//       Access: Published
//  Description: Returns a modifiable pointer to the actual vertex
//               array, so that application code may directly
//               manipulate it.  Use with caution.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
INLINE PTA_uchar GeomVertexArrayData::
modify_data() {
  GeomVertexArrayDataPipelineWriter writer(this, true, Thread::get_current_thread());
  return writer.modify_data();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::set_data
//       Access: Published
//  Description: Replaces the vertex data array with a completely new
//               array.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexArrayData::
set_data(CPTA_uchar array) {
  GeomVertexArrayDataPipelineWriter writer(this, true, Thread::get_current_thread());
  writer.set_data(array);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData::CData::
CData() :
  _usage_hint(UH_unspecified),
  _data(GeomVertexArrayData::get_class_type())
{
  _data.node_ref();
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
  _modified(copy._modified)
{
  _data.node_ref();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineBase::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataPipelineBase::
GeomVertexArrayDataPipelineBase(GeomVertexArrayData *object, 
                                Thread *current_thread,
                                GeomVertexArrayData::CData *cdata) :
  _object(object),
  _current_thread(current_thread),
  _cdata(cdata)
{
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineBase::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataPipelineBase::
~GeomVertexArrayDataPipelineBase() {
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG

#ifdef DO_PIPELINING
  unref_delete((CycleData *)_cdata);
#endif  // DO_PIPELINING

#ifdef _DEBUG
  _object = NULL;
  _cdata = NULL;
#endif  // _DEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineBase::get_current_thread
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Thread *GeomVertexArrayDataPipelineBase::
get_current_thread() const {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineBase::get_array_format
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayFormat *GeomVertexArrayDataPipelineBase::
get_array_format() const {
  return _object->_array_format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineBase::get_usage_hint
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataPipelineBase::UsageHint GeomVertexArrayDataPipelineBase::
get_usage_hint() const {
  return _cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineBase::get_data
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CPTA_uchar GeomVertexArrayDataPipelineBase::
get_data() const {
  return _cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineBase::get_num_rows
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexArrayDataPipelineBase::
get_num_rows() const {
  return get_data_size_bytes() / _object->_array_format->get_stride();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineBase::get_data_size_bytes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexArrayDataPipelineBase::
get_data_size_bytes() const {
  return _cdata->_data.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineBase::get_modified
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq GeomVertexArrayDataPipelineBase::
get_modified() const {
  return _cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataPipelineReader::
GeomVertexArrayDataPipelineReader(const GeomVertexArrayData *object, 
                                  Thread *current_thread) :
  GeomVertexArrayDataPipelineBase((GeomVertexArrayData *)object,
                                  current_thread,
                                  (GeomVertexArrayData::CData *)object->_cycler.read_unlocked(current_thread))
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineReader::Copy Constructor
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataPipelineReader::
GeomVertexArrayDataPipelineReader(const GeomVertexArrayDataPipelineReader &copy) :
  GeomVertexArrayDataPipelineBase(copy)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineReader::Copy Assignment Operator
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexArrayDataPipelineReader::
operator = (const GeomVertexArrayDataPipelineReader &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineReader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataPipelineReader::
~GeomVertexArrayDataPipelineReader() {
  //  _object->_cycler.release_read(_cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineReader::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayData *GeomVertexArrayDataPipelineReader::
get_object() const {
  return _object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineWriter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataPipelineWriter::
GeomVertexArrayDataPipelineWriter(GeomVertexArrayData *object, bool force_to_0,
                                  Thread *current_thread) :
  GeomVertexArrayDataPipelineBase(object, current_thread,
                                  object->_cycler.write_upstream(force_to_0, current_thread))
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineWriter::Copy Constructor
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataPipelineWriter::
GeomVertexArrayDataPipelineWriter(const GeomVertexArrayDataPipelineWriter &copy) :
  GeomVertexArrayDataPipelineBase(copy) 
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineWriter::Copy Assignment Operator
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexArrayDataPipelineWriter::
operator = (const GeomVertexArrayDataPipelineWriter &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineWriter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataPipelineWriter::
~GeomVertexArrayDataPipelineWriter() {
  _object->_cycler.release_write(_cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayDataPipelineWriter::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData *GeomVertexArrayDataPipelineWriter::
get_object() const {
  return _object;
}

INLINE ostream &
operator << (ostream &out, const GeomVertexArrayData &obj) {
  obj.output(out);
  return out;
}
