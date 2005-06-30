// Filename: geomVertexArrayFormat.cxx
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

#include "geomVertexFormat.h"
#include "geomVertexColumn.h"
#include "geomVertexData.h"
#include "geomVertexReader.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indirectLess.h"

GeomVertexArrayFormat::Registry *GeomVertexArrayFormat::_registry = NULL;
TypeHandle GeomVertexArrayFormat::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayFormat::
GeomVertexArrayFormat() :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _columns_unsorted(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayFormat::
GeomVertexArrayFormat(const InternalName *name0, int num_components0,
                      GeomVertexArrayFormat::NumericType numeric_type0,
                      GeomVertexArrayFormat::Contents contents0) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _columns_unsorted(false)
{
  add_column(name0, num_components0, numeric_type0, contents0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayFormat::
GeomVertexArrayFormat(const InternalName *name0, int num_components0,
                      GeomVertexArrayFormat::NumericType numeric_type0,
                      GeomVertexArrayFormat::Contents contents0,
                      const InternalName *name1, int num_components1,
                      GeomVertexArrayFormat::NumericType numeric_type1,
                      GeomVertexArrayFormat::Contents contents1) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _columns_unsorted(false)
{
  add_column(name0, num_components0, numeric_type0, contents0);
  add_column(name1, num_components1, numeric_type1, contents1);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayFormat::
GeomVertexArrayFormat(const InternalName *name0, int num_components0,
                      GeomVertexArrayFormat::NumericType numeric_type0,
                      GeomVertexArrayFormat::Contents contents0,
                      const InternalName *name1, int num_components1,
                      GeomVertexArrayFormat::NumericType numeric_type1,
                      GeomVertexArrayFormat::Contents contents1,
                      const InternalName *name2, int num_components2,
                      GeomVertexArrayFormat::NumericType numeric_type2,
                      GeomVertexArrayFormat::Contents contents2) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _columns_unsorted(false)
{
  add_column(name0, num_components0, numeric_type0, contents0);
  add_column(name1, num_components1, numeric_type1, contents1);
  add_column(name2, num_components2, numeric_type2, contents2);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayFormat::
GeomVertexArrayFormat(const InternalName *name0, int num_components0,
                      GeomVertexArrayFormat::NumericType numeric_type0,
                      GeomVertexArrayFormat::Contents contents0,
                      const InternalName *name1, int num_components1,
                      GeomVertexArrayFormat::NumericType numeric_type1,
                      GeomVertexArrayFormat::Contents contents1,
                      const InternalName *name2, int num_components2,
                      GeomVertexArrayFormat::NumericType numeric_type2,
                      GeomVertexArrayFormat::Contents contents2,
                      const InternalName *name3, int num_components3,
                      GeomVertexArrayFormat::NumericType numeric_type3,
                      GeomVertexArrayFormat::Contents contents3) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _columns_unsorted(false)
{
  add_column(name0, num_components0, numeric_type0, contents0);
  add_column(name1, num_components1, numeric_type1, contents1);
  add_column(name2, num_components2, numeric_type2, contents2);
  add_column(name3, num_components3, numeric_type3, contents3);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayFormat::
GeomVertexArrayFormat(const GeomVertexArrayFormat &copy) :
  _is_registered(false),
  _stride(copy._stride),
  _total_bytes(copy._total_bytes),
  _pad_to(copy._pad_to),
  _columns_unsorted(copy._columns_unsorted)
{
  Columns::const_iterator dti;
  for (dti = copy._columns.begin(); dti != copy._columns.end(); ++dti) {
    add_column(*(*dti));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
operator = (const GeomVertexArrayFormat &copy) {
  nassertv(!_is_registered);
  _stride = copy._stride;
  _total_bytes = copy._total_bytes;
  _pad_to = copy._pad_to;

  _columns.clear();
  _columns_by_name.clear();
  _columns_unsorted = false;
  Columns::const_iterator dti;
  for (dti = copy._columns.begin(); dti != copy._columns.end(); ++dti) {
    add_column(*(*dti));
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayFormat::
~GeomVertexArrayFormat() {
  if (is_registered()) {
    get_registry()->unregister_format(this);
  }
  Columns::iterator ci;
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    delete (*ci);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::add_column
//       Access: Published
//  Description: Adds a new column to the specification.  This is a
//               table of per-vertex floating-point numbers such as
//               "vertex" or "normal"; you must specify where in each
//               record the table starts, and how many components
//               (dimensions) exist per vertex.
//
//               The return value is the index number of the new data
//               type.
////////////////////////////////////////////////////////////////////
int GeomVertexArrayFormat::
add_column(const InternalName *name, int num_components, 
           GeomVertexArrayFormat::NumericType numeric_type, 
           GeomVertexArrayFormat::Contents contents, int start) {
  if (start < 0) {
    start = _total_bytes;

    GeomVertexColumn temp_column
      (name, num_components, numeric_type, contents, 0);
    int pad_to = temp_column.get_component_bytes();
    start = ((start + pad_to - 1) / pad_to) * pad_to;
  }

  return add_column(GeomVertexColumn(name, num_components, 
                                       numeric_type, contents, start));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::add_column
//       Access: Published
//  Description: Adds a new column to the specification.  This is a
//               table of per-vertex floating-point numbers such as
//               "vertex" or "normal"; you must specify where in each
//               record the table starts, and how many components
//               (dimensions) exist per vertex.
//
//               Adding a column with the same name as a previous
//               type, or that overlaps with one or more previous
//               types, quietly removes the previous type(s).
//
//               The return value is the index number of the new data
//               type.
////////////////////////////////////////////////////////////////////
int GeomVertexArrayFormat::
add_column(const GeomVertexColumn &column) {
  nassertr(!_is_registered, -1);

  // Make sure there isn't already a column with this name.
  remove_column(column.get_name());

  // Also make sure there aren't any columns that overlap with this
  // one.
  const GeomVertexColumn *orig_column = get_column(column.get_start(), column.get_total_bytes());
  while (orig_column != (const GeomVertexColumn *)NULL) {
    remove_column(orig_column->get_name());
    orig_column = get_column(column.get_start(), column.get_total_bytes());
  }

  _total_bytes = max(_total_bytes, column.get_start() + column.get_total_bytes());
  _pad_to = max(_pad_to, column.get_component_bytes());
  _stride = max(_stride, ((_total_bytes + _pad_to - 1) / _pad_to) * _pad_to);

  GeomVertexColumn *new_column = new GeomVertexColumn(column);

  if (!_columns.empty() && *new_column < *_columns.back()) {
    _columns_unsorted = true;
  }

  int new_index = (int)_columns.size();
  _columns.push_back(new_column);
  _columns_by_name.insert(ColumnsByName::value_type(new_column->get_name(), new_column));

  return new_index;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::remove_column
//       Access: Published
//  Description: Removes the column with the indicated name, if
//               any.  This leaves a gap in the byte structure.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
remove_column(const InternalName *name) {
  nassertv(!_is_registered);
  ColumnsByName::iterator ni;
  ni = _columns_by_name.find(name);
  if (ni != _columns_by_name.end()) {
    GeomVertexColumn *column = (*ni).second;
    _columns_by_name.erase(ni);

    Columns::iterator ci;
    ci = find(_columns.begin(), _columns.end(), column);
    nassertv(ci != _columns.end());
    _columns.erase(ci);

    delete column;

    // Maybe we just removed the tail column.  If that's so, we
    // should recompute _total_bytes to reflect the new tail.
    if (_columns.empty()) {
      _total_bytes = 0;
    } else {
      consider_sort_columns();
      GeomVertexColumn *last = _columns.back();
      _total_bytes = last->get_start() + last->get_total_bytes();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::clear_columns
//       Access: Published
//  Description: Removes all columns previously added, sets the
//               stride to zero, and prepares to start over.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
clear_columns() {
  _columns.clear();
  _columns_by_name.clear();
  _columns_unsorted = false;
  _stride = 0;
  _total_bytes = 0;
  _pad_to = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::get_column
//       Access: Published
//  Description: Returns the specification with the indicated name, or
//               NULL if the name is not used.
////////////////////////////////////////////////////////////////////
const GeomVertexColumn *GeomVertexArrayFormat::
get_column(const InternalName *name) const {
  ColumnsByName::const_iterator ni;
  ni = _columns_by_name.find(name);
  if (ni != _columns_by_name.end()) {
    return (*ni).second;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::get_column
//       Access: Published
//  Description: Returns the first specification that overlaps with
//               any of the indicated bytes in the range, or NULL if
//               none do.
////////////////////////////////////////////////////////////////////
const GeomVertexColumn *GeomVertexArrayFormat::
get_column(int start_byte, int num_bytes) const {
  consider_sort_columns();
  Columns::const_iterator ci;
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    const GeomVertexColumn *column = (*ci);
    if (column->overlaps_with(start_byte, num_bytes)) {
      return column;
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::is_data_subset_of
//       Access: Published
//  Description: Returns true if all of the fields in this array
//               format are also present and equivalent in the other
//               array format, and in the same byte positions, and the
//               stride is the same.  That is, true if this format can
//               share the same data pointer as the other format (with
//               possibly some unused gaps).
////////////////////////////////////////////////////////////////////
bool GeomVertexArrayFormat::
is_data_subset_of(const GeomVertexArrayFormat &other) const {
  if (_columns.size() > other._columns.size() ||
      get_stride() != other.get_stride()) {
    return false;
  }

  consider_sort_columns();
  other.consider_sort_columns();
  size_t i = 0;
  size_t j = 0;
  while (i < _columns.size() && j < other._columns.size()) {
    if (*_columns[i] == *other._columns[j]) {
      ++i;
    }
    ++j;
  }

  // If we reached the end of our list, all fields matched.
  return (i == _columns.size());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
output(ostream &out) const {
  Columns::const_iterator ci;
  out << "[";
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    const GeomVertexColumn *column = (*ci);
    out << " " << *column;
  }
  out << " ]";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "Array format (stride = " << get_stride() << "):\n";
  consider_sort_columns();
  Columns::const_iterator ci;
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    const GeomVertexColumn *column = (*ci);
    indent(out, indent_level + 2)
      << *column 
      << " " << column->get_numeric_type()
      << " " << column->get_contents()
      << " start at " << column->get_start() << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::write_with_data
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
write_with_data(ostream &out, int indent_level,
                const GeomVertexArrayData *array_data) const {
  consider_sort_columns();
  int num_rows = array_data->get_num_rows();

  GeomVertexReader reader(array_data);

  for (int i = 0; i < num_rows; i++) {
    indent(out, indent_level)
      << "row " << i << ":\n";
    reader.set_row(i);
    Columns::const_iterator ci;
    for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
      const GeomVertexColumn *column = (*ci);
      int num_values = min(column->get_num_values(), 4);
      reader.set_column(0, column);
      const LVecBase4f &d = reader.get_data4f();

      indent(out, indent_level + 2) 
        << *column->get_name();
      for (int v = 0; v < num_values; v++) {
        out << " " << d[v];
      }
      out << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int GeomVertexArrayFormat::
compare_to(const GeomVertexArrayFormat &other) const {
  if (_stride != other._stride) {
    return _stride - other._stride;
  }
  if (_total_bytes != other._total_bytes) {
    return _total_bytes - other._total_bytes;
  }
  if (_pad_to != other._pad_to) {
    return _pad_to - other._pad_to;
  }
  if (_columns.size() != other._columns.size()) {
    return (int)_columns.size() - (int)other._columns.size();
  }
  consider_sort_columns();
  other.consider_sort_columns();
  for (size_t i = 0; i < _columns.size(); i++) {
    int compare = _columns[i]->compare_to(*other._columns[i]);
    if (compare != 0) {
      return compare;
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::sort_columns
//       Access: Private
//  Description: Resorts the _columns vector so that the columns
//               are listed in the same order they appear in the
//               record.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
sort_columns() {
  sort(_columns.begin(), _columns.end(), IndirectLess<GeomVertexColumn>());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::make_registry
//       Access: Private
//  Description: Returns the global registry object.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
make_registry() {
  if (_registry == (Registry *)NULL) {
    _registry = new Registry;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::do_register
//       Access: Private
//  Description: Called internally when the format is registered.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
do_register() {
  nassertv(!_is_registered);
  _is_registered = true;
}
 
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::do_unregister
//       Access: Private
//  Description: Called internally when the format is unregistered.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
do_unregister() {
  nassertv(_is_registered);
  _is_registered = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               GeomVertexArrayFormat.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  dg.add_uint16(_stride);
  dg.add_uint16(_total_bytes);
  dg.add_uint8(_pad_to);

  consider_sort_columns();

  dg.add_uint16(_columns.size());
  Columns::iterator ci;
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    GeomVertexColumn *column = (*ci);
    column->write_datagram(manager, dg);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int GeomVertexArrayFormat::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  Columns::iterator ci;
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    GeomVertexColumn *column = (*ci);
    pi += column->complete_pointers(p_list + pi, manager);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
finalize(BamReader *manager) {
  // Now we can build up the _columns_by_name index.  We have to wait
  // until finalize(), since the index is based on the nested name
  // pointer within each column, which might not be available at the
  // time complete_pointers() is called.
  _columns_by_name.clear();
  Columns::iterator ci;
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    GeomVertexColumn *column = (*ci);
    _columns_by_name.insert(ColumnsByName::value_type(column->get_name(), column));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type GeomVertexArrayFormat is
//               encountered in the Bam file.  It should create the
//               GeomVertexArrayFormat and extract its information
//               from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomVertexArrayFormat::
make_from_bam(const FactoryParams &params) {
  GeomVertexArrayFormat *object = new GeomVertexArrayFormat;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);
  manager->register_finalize(object);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomVertexArrayFormat.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);
  nassertv(!_is_registered);

  _stride = scan.get_uint16();
  _total_bytes = scan.get_uint16();
  _pad_to = scan.get_uint8();

  int num_columns = scan.get_uint16();
  _columns.reserve(num_columns);
  for (int i = 0; i < num_columns; ++i) {
    GeomVertexColumn *column = new GeomVertexColumn;
    column->fillin(scan, manager);
    _columns.push_back(column);
  }
  _columns_unsorted = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Registry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexArrayFormat::Registry::
Registry() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Registry::register_format
//       Access: Public
//  Description: Adds the indicated format to the registry, if there
//               is not an equivalent format already there; in either
//               case, returns the pointer to the equivalent format
//               now in the registry.
//
//               This must be called before a format may be used in a
//               Geom.  After this call, you should discard the
//               original pointer you passed in (which may or may not
//               now be invalid) and let its reference count decrement
//               normally; you should use only the returned value from
//               this point on.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexArrayFormat) GeomVertexArrayFormat::Registry::
register_format(GeomVertexArrayFormat *format) {
  if (format->is_registered()) {
    return format;
  }

  // Save the incoming pointer in a local PointerTo, so that if it has
  // a zero reference count and is not added into the map below, it
  // will be automatically deleted when this function returns.
  PT(GeomVertexArrayFormat) pt_format = format;

  ArrayFormats::iterator fi = _formats.insert(format).first;

  GeomVertexArrayFormat *new_format = (*fi);
  if (!new_format->is_registered()) {
    new_format->do_register();
  }

  return new_format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayFormat::Registry::unregister_format
//       Access: Public
//  Description: Removes the indicated format from the registry.
//               Normally this should not be done until the format is
//               destructing.
////////////////////////////////////////////////////////////////////
void GeomVertexArrayFormat::Registry::
unregister_format(GeomVertexArrayFormat *format) {
  nassertv(format->is_registered());
  ArrayFormats::iterator fi = _formats.find(format);
  nassertv(fi != _formats.end());
  _formats.erase(fi);
  format->do_unregister();
}
