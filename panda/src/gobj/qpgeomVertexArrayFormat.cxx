// Filename: qpgeomVertexArrayFormat.cxx
// Created by:  drose (06Mar05)
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

#include "qpgeomVertexFormat.h"
#include "qpgeomVertexDataType.h"
#include "qpgeomVertexData.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indirectLess.h"

TypeHandle qpGeomVertexArrayFormat::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayFormat::
qpGeomVertexArrayFormat() :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _data_types_unsorted(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayFormat::
qpGeomVertexArrayFormat(const InternalName *name0, int num_components0,
                        qpGeomVertexDataType::NumericType numeric_type0) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _data_types_unsorted(false)
{
  add_data_type(name0, num_components0, numeric_type0);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayFormat::
qpGeomVertexArrayFormat(const InternalName *name0, int num_components0,
                        qpGeomVertexDataType::NumericType numeric_type0,
                        const InternalName *name1, int num_components1,
                        qpGeomVertexDataType::NumericType numeric_type1) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _data_types_unsorted(false)
{
  add_data_type(name0, num_components0, numeric_type0);
  add_data_type(name1, num_components1, numeric_type1);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayFormat::
qpGeomVertexArrayFormat(const InternalName *name0, int num_components0,
                        qpGeomVertexDataType::NumericType numeric_type0,
                        const InternalName *name1, int num_components1,
                        qpGeomVertexDataType::NumericType numeric_type1,
                        const InternalName *name2, int num_components2,
                        qpGeomVertexDataType::NumericType numeric_type2) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _data_types_unsorted(false)
{
  add_data_type(name0, num_components0, numeric_type0);
  add_data_type(name1, num_components1, numeric_type1);
  add_data_type(name2, num_components2, numeric_type2);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayFormat::
qpGeomVertexArrayFormat(const InternalName *name0, int num_components0,
                        qpGeomVertexDataType::NumericType numeric_type0,
                        const InternalName *name1, int num_components1,
                        qpGeomVertexDataType::NumericType numeric_type1,
                        const InternalName *name2, int num_components2,
                        qpGeomVertexDataType::NumericType numeric_type2,
                        const InternalName *name3, int num_components3,
                        qpGeomVertexDataType::NumericType numeric_type3) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _data_types_unsorted(false)
{
  add_data_type(name0, num_components0, numeric_type0);
  add_data_type(name1, num_components1, numeric_type1);
  add_data_type(name2, num_components2, numeric_type2);
  add_data_type(name3, num_components3, numeric_type3);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayFormat::
qpGeomVertexArrayFormat(const qpGeomVertexArrayFormat &copy) :
  _is_registered(false),
  _stride(copy._stride),
  _total_bytes(copy._total_bytes),
  _pad_to(copy._pad_to),
  _data_types_unsorted(copy._data_types_unsorted)
{
  DataTypes::const_iterator dti;
  for (dti = copy._data_types.begin(); dti != copy._data_types.end(); ++dti) {
    add_data_type(*(*dti));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
operator = (const qpGeomVertexArrayFormat &copy) {
  nassertv(!_is_registered);
  _stride = copy._stride;
  _total_bytes = copy._total_bytes;
  _pad_to = copy._pad_to;

  _data_types.clear();
  _data_types_by_name.clear();
  _data_types_unsorted = false;
  DataTypes::const_iterator dti;
  for (dti = copy._data_types.begin(); dti != copy._data_types.end(); ++dti) {
    add_data_type(*(*dti));
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayFormat::
~qpGeomVertexArrayFormat() {
  // Once registered, these things should not be deallocated.
  nassertv(!_is_registered);

  DataTypes::iterator dti;
  for (dti = _data_types.begin(); dti != _data_types.end(); ++dti) {
    delete (*dti);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::add_data_type
//       Access: Published
//  Description: Adds a new data type to the specification.  This is a
//               table of per-vertex floating-point numbers such as
//               "vertex" or "normal"; you must specify where in each
//               record the table starts, and how many components
//               (dimensions) exist per vertex.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
add_data_type(const InternalName *name, int num_components, 
              qpGeomVertexDataType::NumericType numeric_type, int start) {
  if (start < 0) {
    start = _total_bytes;

    qpGeomVertexDataType temp_data_type(name, num_components, numeric_type, 0);
    int pad_to = temp_data_type.get_component_bytes();
    start = ((start + pad_to - 1) / pad_to) * pad_to;
  }

  add_data_type(qpGeomVertexDataType(name, num_components, 
                                     numeric_type, start));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::add_data_type
//       Access: Published
//  Description: Adds a new data type to the specification.  This is a
//               table of per-vertex floating-point numbers such as
//               "vertex" or "normal"; you must specify where in each
//               record the table starts, and how many components
//               (dimensions) exist per vertex.
//
//               Adding a data type with the same name as a previous
//               type, or that overlaps with one or more previous
//               types, quietly removes the previous type(s).
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
add_data_type(const qpGeomVertexDataType &data_type) {
  nassertv(!_is_registered);

  // Make sure there isn't already a data type with this name.
  remove_data_type(data_type.get_name());

  // Also make sure there aren't any data types that overlap with this
  // one.
  const qpGeomVertexDataType *orig_data_type = get_data_type(data_type.get_start(), data_type.get_total_bytes());
  while (orig_data_type != (const qpGeomVertexDataType *)NULL) {
    remove_data_type(orig_data_type->get_name());
    orig_data_type = get_data_type(data_type.get_start(), data_type.get_total_bytes());
  }

  _total_bytes = max(_total_bytes, data_type.get_start() + data_type.get_total_bytes());
  _pad_to = max(_pad_to, data_type.get_component_bytes());
  _stride = max(_stride, ((_total_bytes + _pad_to - 1) / _pad_to) * _pad_to);

  qpGeomVertexDataType *new_data_type = new qpGeomVertexDataType(data_type);

  if (!_data_types.empty() && *new_data_type < *_data_types.back()) {
    _data_types_unsorted = true;
  }

  _data_types.push_back(new_data_type);
  _data_types_by_name.insert(DataTypesByName::value_type(new_data_type->get_name(), new_data_type));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::remove_data_type
//       Access: Published
//  Description: Removes the data type with the indicated name, if
//               any.  This leaves a gap in the byte structure.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
remove_data_type(const InternalName *name) {
  nassertv(!_is_registered);
  DataTypesByName::iterator ni;
  ni = _data_types_by_name.find(name);
  if (ni != _data_types_by_name.end()) {
    qpGeomVertexDataType *data_type = (*ni).second;
    _data_types_by_name.erase(ni);

    DataTypes::iterator dti;
    dti = find(_data_types.begin(), _data_types.end(), data_type);
    nassertv(dti != _data_types.end());
    _data_types.erase(dti);

    delete data_type;

    // Maybe we just removed the tail data_type.  If that's so, we
    // should recompute _total_bytes to reflect the new tail.
    if (_data_types.empty()) {
      _total_bytes = 0;
    } else {
      consider_sort_data_types();
      qpGeomVertexDataType *last = _data_types.back();
      _total_bytes = last->get_start() + last->get_total_bytes();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::clear_data_types
//       Access: Published
//  Description: Removes all data types previously added, sets the
//               stride to zero, and prepares to start over.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
clear_data_types() {
  _data_types.clear();
  _data_types_by_name.clear();
  _data_types_unsorted = false;
  _stride = 0;
  _total_bytes = 0;
  _pad_to = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::get_data_type
//       Access: Published
//  Description: Returns the specification with the indicated name, or
//               NULL if the name is not used.
////////////////////////////////////////////////////////////////////
const qpGeomVertexDataType *qpGeomVertexArrayFormat::
get_data_type(const InternalName *name) const {
  DataTypesByName::const_iterator ni;
  ni = _data_types_by_name.find(name);
  if (ni != _data_types_by_name.end()) {
    return (*ni).second;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::get_data_type
//       Access: Published
//  Description: Returns the first specification that overlaps with
//               any of the indicated bytes in the range, or NULL if
//               none do.
////////////////////////////////////////////////////////////////////
const qpGeomVertexDataType *qpGeomVertexArrayFormat::
get_data_type(int start_byte, int num_bytes) const {
  consider_sort_data_types();
  DataTypes::const_iterator dti;
  for (dti = _data_types.begin(); dti != _data_types.end(); ++dti) {
    const qpGeomVertexDataType *data_type = (*dti);
    if (data_type->overlaps_with(start_byte, num_bytes)) {
      return data_type;
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::is_data_subset_of
//       Access: Published
//  Description: Returns true if all of the fields in this array
//               format are also present and equivalent in the other
//               array format, and in the same byte positions, and the
//               stride is the same.  That is, true if this format can
//               share the same data pointer as the other format (with
//               possibly some unused gaps).
////////////////////////////////////////////////////////////////////
bool qpGeomVertexArrayFormat::
is_data_subset_of(const qpGeomVertexArrayFormat &other) const {
  if (_data_types.size() > other._data_types.size() ||
      get_stride() != other.get_stride()) {
    return false;
  }

  consider_sort_data_types();
  other.consider_sort_data_types();
  size_t i = 0;
  size_t j = 0;
  while (i < _data_types.size() && j < other._data_types.size()) {
    if (*_data_types[i] == *other._data_types[j]) {
      ++i;
    }
    ++j;
  }

  // If we reached the end of our list, all fields matched.
  return (i == _data_types.size());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
output(ostream &out) const {
  DataTypes::const_iterator dti;
  out << "[";
  for (dti = _data_types.begin(); dti != _data_types.end(); ++dti) {
    const qpGeomVertexDataType *data_type = (*dti);
    out << " " << *data_type;
  }
  out << " ]";
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "Array format (stride = " << get_stride() << "):\n";
  consider_sort_data_types();
  DataTypes::const_iterator dti;
  for (dti = _data_types.begin(); dti != _data_types.end(); ++dti) {
    const qpGeomVertexDataType *data_type = (*dti);
    indent(out, indent_level + 2)
      << *data_type << " start at " << data_type->get_start() << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::write_with_data
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
write_with_data(ostream &out, int indent_level,
                const qpGeomVertexData *data, int array_index) const {
  consider_sort_data_types();
  int num_vertices = data->get_num_vertices();
  for (int i = 0; i < num_vertices; i++) {
    indent(out, indent_level)
      << "vertex index " << i << ":\n";
    DataTypes::const_iterator dti;
    for (dti = _data_types.begin(); dti != _data_types.end(); ++dti) {
      const qpGeomVertexDataType *data_type = (*dti);
      int num_values = min(data_type->get_num_values(), 4);
      float data_values[4]; 
      data->get_data(array_index, data_type, i, data_values, num_values);

      indent(out, indent_level + 2) 
        << *data_type->get_name();
      for (int v = 0; v < num_values; v++) {
        out << " " << data_values[v];
      }
      out << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int qpGeomVertexArrayFormat::
compare_to(const qpGeomVertexArrayFormat &other) const {
  if (_stride != other._stride) {
    return _stride - other._stride;
  }
  if (_total_bytes != other._total_bytes) {
    return _total_bytes - other._total_bytes;
  }
  if (_pad_to != other._pad_to) {
    return _pad_to - other._pad_to;
  }
  if (_data_types.size() != other._data_types.size()) {
    return (int)_data_types.size() - (int)other._data_types.size();
  }
  consider_sort_data_types();
  other.consider_sort_data_types();
  for (size_t i = 0; i < _data_types.size(); i++) {
    int compare = _data_types[i]->compare_to(*other._data_types[i]);
    if (compare != 0) {
      return compare;
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::sort_data_types
//       Access: Private
//  Description: Resorts the _data_types vector so that the data types
//               are listed in the same order they appear in the
//               record.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
sort_data_types() {
  sort(_data_types.begin(), _data_types.end(), IndirectLess<qpGeomVertexDataType>());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::do_register
//       Access: Private
//  Description: Called internally when the format is registered.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
do_register() {
  nassertv(!_is_registered);
  _is_registered = true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeomVertexArrayFormat.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int qpGeomVertexArrayFormat::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeomVertexArrayFormat is
//               encountered in the Bam file.  It should create the
//               qpGeomVertexArrayFormat and extract its information from
//               the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomVertexArrayFormat::
make_from_bam(const FactoryParams &params) {
  qpGeomVertexArrayFormat *object = new qpGeomVertexArrayFormat;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomVertexArrayFormat.
////////////////////////////////////////////////////////////////////
void qpGeomVertexArrayFormat::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);
}
