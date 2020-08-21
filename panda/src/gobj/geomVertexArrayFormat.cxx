/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexArrayFormat.cxx
 * @author drose
 * @date 2005-03-06
 */

#include "geomVertexFormat.h"
#include "geomVertexColumn.h"
#include "geomVertexData.h"
#include "geomVertexReader.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indirectLess.h"
#include "lightMutexHolder.h"

using std::max;
using std::min;
using std::move;

GeomVertexArrayFormat::Registry *GeomVertexArrayFormat::_registry = nullptr;
TypeHandle GeomVertexArrayFormat::_type_handle;

/**
 *
 */
GeomVertexArrayFormat::
GeomVertexArrayFormat() :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _divisor(0),
  _columns_unsorted(false)
{
}

/**
 *
 */
GeomVertexArrayFormat::
GeomVertexArrayFormat(CPT_InternalName name0, int num_components0,
                      GeomVertexArrayFormat::NumericType numeric_type0,
                      GeomVertexArrayFormat::Contents contents0) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _divisor(0),
  _columns_unsorted(false)
{
  add_column(move(name0), num_components0, numeric_type0, contents0);
}

/**
 *
 */
GeomVertexArrayFormat::
GeomVertexArrayFormat(CPT_InternalName name0, int num_components0,
                      GeomVertexArrayFormat::NumericType numeric_type0,
                      GeomVertexArrayFormat::Contents contents0,
                      CPT_InternalName name1, int num_components1,
                      GeomVertexArrayFormat::NumericType numeric_type1,
                      GeomVertexArrayFormat::Contents contents1) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _divisor(0),
  _columns_unsorted(false)
{
  add_column(move(name0), num_components0, numeric_type0, contents0);
  add_column(move(name1), num_components1, numeric_type1, contents1);
}

/**
 *
 */
GeomVertexArrayFormat::
GeomVertexArrayFormat(CPT_InternalName name0, int num_components0,
                      GeomVertexArrayFormat::NumericType numeric_type0,
                      GeomVertexArrayFormat::Contents contents0,
                      CPT_InternalName name1, int num_components1,
                      GeomVertexArrayFormat::NumericType numeric_type1,
                      GeomVertexArrayFormat::Contents contents1,
                      CPT_InternalName name2, int num_components2,
                      GeomVertexArrayFormat::NumericType numeric_type2,
                      GeomVertexArrayFormat::Contents contents2) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _divisor(0),
  _columns_unsorted(false)
{
  add_column(move(name0), num_components0, numeric_type0, contents0);
  add_column(move(name1), num_components1, numeric_type1, contents1);
  add_column(move(name2), num_components2, numeric_type2, contents2);
}

/**
 *
 */
GeomVertexArrayFormat::
GeomVertexArrayFormat(CPT_InternalName name0, int num_components0,
                      GeomVertexArrayFormat::NumericType numeric_type0,
                      GeomVertexArrayFormat::Contents contents0,
                      CPT_InternalName name1, int num_components1,
                      GeomVertexArrayFormat::NumericType numeric_type1,
                      GeomVertexArrayFormat::Contents contents1,
                      CPT_InternalName name2, int num_components2,
                      GeomVertexArrayFormat::NumericType numeric_type2,
                      GeomVertexArrayFormat::Contents contents2,
                      CPT_InternalName name3, int num_components3,
                      GeomVertexArrayFormat::NumericType numeric_type3,
                      GeomVertexArrayFormat::Contents contents3) :
  _is_registered(false),
  _stride(0),
  _total_bytes(0),
  _pad_to(1),
  _divisor(0),
  _columns_unsorted(false)
{
  add_column(move(name0), num_components0, numeric_type0, contents0);
  add_column(move(name1), num_components1, numeric_type1, contents1);
  add_column(move(name2), num_components2, numeric_type2, contents2);
  add_column(move(name3), num_components3, numeric_type3, contents3);
}

/**
 *
 */
GeomVertexArrayFormat::
GeomVertexArrayFormat(const GeomVertexArrayFormat &copy) :
  _is_registered(false),
  _stride(copy._stride),
  _total_bytes(copy._total_bytes),
  _pad_to(copy._pad_to),
  _divisor(copy._divisor),
  _columns_unsorted(copy._columns_unsorted)
{
  Columns::const_iterator dti;
  for (dti = copy._columns.begin(); dti != copy._columns.end(); ++dti) {
    add_column(*(*dti));
  }
}

/**
 *
 */
void GeomVertexArrayFormat::
operator = (const GeomVertexArrayFormat &copy) {
  nassertv(!_is_registered);
  _stride = copy._stride;
  _total_bytes = copy._total_bytes;
  _pad_to = copy._pad_to;
  _divisor = copy._divisor;

  _columns.clear();
  _columns_by_name.clear();
  _columns_unsorted = false;
  Columns::const_iterator dti;
  for (dti = copy._columns.begin(); dti != copy._columns.end(); ++dti) {
    add_column(*(*dti));
  }
}

/**
 *
 */
GeomVertexArrayFormat::
~GeomVertexArrayFormat() {
  // unref() should have unregistered us.
  nassertv(!is_registered());

  Columns::iterator ci;
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    delete (*ci);
  }
}

/**
 * This method overrides ReferenceCount::unref() to unregister the object when
 * its reference count goes to zero.
 */
bool GeomVertexArrayFormat::
unref() const {
  Registry *registry = get_registry();
  LightMutexHolder holder(registry->_lock);

  if (ReferenceCount::unref()) {
    return true;
  }

  if (is_registered()) {
    registry->unregister_format((GeomVertexArrayFormat *)this);
  }

  return false;
}

/**
 * Adds a new column to the specification.  This is a table of per-vertex
 * floating-point numbers such as "vertex" or "normal"; you must specify where
 * in each record the table starts, and how many components (dimensions) exist
 * per vertex.
 *
 * The return value is the index number of the new data type.
 */
int GeomVertexArrayFormat::
add_column(CPT_InternalName name, int num_components,
           GeomVertexArrayFormat::NumericType numeric_type,
           GeomVertexArrayFormat::Contents contents, int start,
           int column_alignment) {
  if (start < 0) {
    start = _total_bytes;
  }

  return add_column(GeomVertexColumn(move(name), num_components, numeric_type, contents,
                                     start, column_alignment));
}

/**
 * Adds a new column to the specification.  This is a table of per-vertex
 * floating-point numbers such as "vertex" or "normal"; you must specify where
 * in each record the table starts, and how many components (dimensions) exist
 * per vertex.
 *
 * Adding a column with the same name as a previous type, or that overlaps
 * with one or more previous types, quietly removes the previous type(s).
 *
 * The return value is the index number of the new data type.
 */
int GeomVertexArrayFormat::
add_column(const GeomVertexColumn &column) {
  nassertr(!_is_registered, -1);

  // Make sure there isn't already a column with this name.
  remove_column(column.get_name());

  // Also make sure there aren't any columns that overlap with this one.
  const GeomVertexColumn *orig_column = get_column(column.get_start(), column.get_total_bytes());
  while (orig_column != nullptr) {
    remove_column(orig_column->get_name());
    orig_column = get_column(column.get_start(), column.get_total_bytes());
  }

  _total_bytes = max(_total_bytes, column.get_start() + column.get_total_bytes());
  _pad_to = max(_pad_to, column.get_column_alignment());
  _stride = max(_stride, _total_bytes);
  if (_pad_to > 1) {
    _stride = ((_stride + _pad_to - 1) / _pad_to) * _pad_to;
  }

  GeomVertexColumn *new_column = new GeomVertexColumn(column);

  if (!_columns.empty() && *new_column < *_columns.back()) {
    _columns_unsorted = true;
  }

  int new_index = (int)_columns.size();
  _columns.push_back(new_column);
  _columns_by_name.insert(ColumnsByName::value_type(new_column->get_name(), new_column));

  return new_index;
}

/**
 * Removes the column with the indicated name, if any.  This leaves a gap in
 * the byte structure.
 */
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

    // Maybe we just removed the tail column.  If that's so, we should
    // recompute _total_bytes to reflect the new tail.
    if (_columns.empty()) {
      _total_bytes = 0;
    } else {
      consider_sort_columns();
      GeomVertexColumn *last = _columns.back();
      _total_bytes = last->get_start() + last->get_total_bytes();
    }
  }
}

/**
 * Removes all columns previously added, sets the stride to zero, and prepares
 * to start over.
 */
void GeomVertexArrayFormat::
clear_columns() {
  nassertv(!_is_registered);

  _columns.clear();
  _columns_by_name.clear();
  _columns_unsorted = false;
  _stride = 0;
  _total_bytes = 0;
  _pad_to = 1;
}

/**
 * Removes wasted space between columns.
 */
void GeomVertexArrayFormat::
pack_columns() {
  nassertv(!_is_registered);

  Columns orig_columns;
  orig_columns.swap(_columns);
  clear_columns();

  Columns::const_iterator ci;
  for (ci = orig_columns.begin(); ci != orig_columns.end(); ++ci) {
    GeomVertexColumn *column = (*ci);
    add_column(column->get_name(), column->get_num_components(),
               column->get_numeric_type(), column->get_contents());
  }
}

/**
 * Reprocesses the columns in the format to align the C_point and C_vector
 * columns to 16-byte boundaries to allow for the more efficient SSE2
 * operations (assuming SSE2 is enabled in the build).
 *
 * The caller is responsible for testing vertex_animation_align_16 to decide
 * whether to call this method.
 */
void GeomVertexArrayFormat::
align_columns_for_animation() {
  nassertv(!_is_registered);

  Columns orig_columns;
  orig_columns.swap(_columns);
  clear_columns();

  Columns::const_iterator ci;
  for (ci = orig_columns.begin(); ci != orig_columns.end(); ++ci) {
    GeomVertexColumn *column = (*ci);
    if ((column->get_contents() == C_point ||
         column->get_contents() == C_vector ||
         column->get_contents() == C_normal) &&
        (column->get_numeric_type() == NT_float32 ||
         column->get_numeric_type() == NT_float64) &&
        column->get_num_components() >= 3) {
      add_column(column->get_name(), 4, column->get_numeric_type(), column->get_contents(), -1, 16);
    } else {
      add_column(column->get_name(), column->get_num_components(),
                 column->get_numeric_type(), column->get_contents(),
                 -1, column->get_column_alignment());
    }
  }
}


/**
 * Returns the specification with the indicated name, or NULL if the name is
 * not used.
 */
const GeomVertexColumn *GeomVertexArrayFormat::
get_column(const InternalName *name) const {
  ColumnsByName::const_iterator ni;
  ni = _columns_by_name.find(name);
  if (ni != _columns_by_name.end()) {
    return (*ni).second;
  }
  return nullptr;
}

/**
 * Returns the first specification that overlaps with any of the indicated
 * bytes in the range, or NULL if none do.
 */
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

  return nullptr;
}

/**
 * Returns true if all of the fields in this array format are also present and
 * equivalent in the other array format, and in the same byte positions, and
 * the stride is the same.  That is, true if this format can share the same
 * data pointer as the other format (with possibly some unused gaps).
 */
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

/**
 * Returns the number of bytes per row that are not assigned to any column.
 */
int GeomVertexArrayFormat::
count_unused_space() const {
  consider_sort_columns();

  int unused_space = 0;
  int last_pos = 0;

  Columns::const_iterator ci;
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    const GeomVertexColumn *column = (*ci);
    if (column->get_start() > last_pos) {
      unused_space += (column->get_start() - last_pos);
    }
    last_pos = column->get_start() + column->get_total_bytes();
  }

  if (_stride > last_pos) {
    unused_space += (_stride - last_pos);
  }

  return unused_space;
}

/**
 *
 */
void GeomVertexArrayFormat::
output(std::ostream &out) const {
  Columns::const_iterator ci;
  int last_pos = 0;
  out << "[";
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    const GeomVertexColumn *column = (*ci);
    if (column->get_start() > last_pos) {
      out << " (..." << (column->get_start() - last_pos) << "...)";
    }
    out << " " << *column;
    last_pos = column->get_start() + column->get_total_bytes();
  }

  if (_stride > last_pos) {
    out << " ..." << (_stride - last_pos) << "...";
  }

  out << " ]";
}

/**
 *
 */
void GeomVertexArrayFormat::
write(std::ostream &out, int indent_level) const {
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

/**
 *
 */
void GeomVertexArrayFormat::
write_with_data(std::ostream &out, int indent_level,
                const GeomVertexArrayData *array_data) const {
  consider_sort_columns();
  int num_rows = array_data->get_num_rows();

  GeomVertexReader reader(array_data);

  for (int i = 0; i < num_rows; i++) {
    indent(out, indent_level)
      << "row " << i << ":\n";
    reader.set_row_unsafe(i);
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

/**
 * Returns a string with format codes representing the exact memory layout of
 * the columns in memory, as understood by Python's struct module.  If pad is
 * true, extra padding bytes are added to the end as 'x' characters as needed.
 */
std::string GeomVertexArrayFormat::
get_format_string(bool pad) const {
  consider_sort_columns();

  int row_size;
  if (pad) {
    row_size = get_stride();
  } else {
    row_size = get_total_bytes();
  }

  // Synthesize the format string.
  char *fmt = (char*) malloc(row_size + 1);
  memset((void*) fmt, 0, row_size + 1);
  int fi = 0;
  int offset = 0;

  for (const GeomVertexColumn *column : _columns) {
    if (offset < column->get_start()) {
      // Add padding bytes to fill the gap.
      int pad = column->get_start() - offset;
      memset((void*) (fmt + fi), 'x', pad);
      fi += pad;
      offset += pad;
    }

    char fmt_code = 'x';
    switch (column->get_numeric_type()) {
    case NT_uint8:
      fmt_code = 'B';
      break;

    case NT_uint16:
      fmt_code = 'H';
      break;

    case NT_uint32:
    case NT_packed_dcba:
    case NT_packed_dabc:
      fmt_code = 'I';
      break;

    case NT_float32:
      fmt_code = 'f';
      break;

    case NT_float64:
      fmt_code = 'd';
      break;

    case NT_int8:
      fmt_code = 'b';
      break;

    case NT_int16:
      fmt_code = 'h';
      break;

    case NT_int32:
      fmt_code = 'i';
      break;

    default:
      gobj_cat.error()
        << "Unknown numeric type " << column->get_numeric_type() << "!\n";
      return nullptr;
    }
    memset((void*) (fmt + fi), fmt_code, column->get_num_components());
    offset += column->get_total_bytes();
    fi += column->get_num_components();
  }

  if (offset < row_size) {
    // Add padding bytes.
    int pad = row_size - offset;
    memset((void*) (fmt + fi), 'x', pad);
  }

  std::string fmt_string (fmt);
  free(fmt);
  return fmt_string;
}

/**
 *
 */
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
  if (_divisor != other._divisor) {
    return _divisor - other._divisor;
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

/**
 * Resorts the _columns vector so that the columns are listed in the same
 * order they appear in the record.
 */
void GeomVertexArrayFormat::
sort_columns() {
  sort(_columns.begin(), _columns.end(), IndirectLess<GeomVertexColumn>());
}

/**
 * Returns the global registry object.
 */
void GeomVertexArrayFormat::
make_registry() {
  if (_registry == nullptr) {
    _registry = new Registry;
  }
}

/**
 * Called internally when the format is registered.
 */
void GeomVertexArrayFormat::
do_register() {
  nassertv(!_is_registered);
  _is_registered = true;
}

/**
 * Called internally when the format is unregistered.
 */
void GeomVertexArrayFormat::
do_unregister() {
  nassertv(_is_registered);
  _is_registered = false;
}

/**
 * Tells the BamReader how to create objects of type GeomVertexArrayFormat.
 */
void GeomVertexArrayFormat::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void GeomVertexArrayFormat::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  dg.add_uint16(_stride);
  dg.add_uint16(_total_bytes);
  dg.add_uint8(_pad_to);

  if (manager->get_file_minor_ver() > 36) {
    dg.add_uint16(_divisor);
  }

  consider_sort_columns();

  dg.add_uint16(_columns.size());
  Columns::iterator ci;
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    GeomVertexColumn *column = (*ci);
    column->write_datagram(manager, dg);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
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

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void GeomVertexArrayFormat::
finalize(BamReader *manager) {
  // Now we can build up the _columns_by_name index.  We have to wait until
  // finalize(), since the index is based on the nested name pointer within
  // each column, which might not be available at the time complete_pointers()
  // is called.
  _columns_by_name.clear();
  Columns::iterator ci;
  for (ci = _columns.begin(); ci != _columns.end(); ++ci) {
    GeomVertexColumn *column = (*ci);
    _columns_by_name.insert(ColumnsByName::value_type(column->get_name(), column));
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type GeomVertexArrayFormat is encountered in the Bam file.  It should
 * create the GeomVertexArrayFormat and extract its information from the file.
 */
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

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new GeomVertexArrayFormat.
 */
void GeomVertexArrayFormat::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);
  nassertv(!_is_registered);

  _stride = scan.get_uint16();
  _total_bytes = scan.get_uint16();
  _pad_to = scan.get_uint8();
  if (manager->get_file_minor_ver() > 36) {
    _divisor = scan.get_uint16();
  } else {
    _divisor = 0;
  }

  int num_columns = scan.get_uint16();
  _columns.reserve(num_columns);
  for (int i = 0; i < num_columns; ++i) {
    GeomVertexColumn *column = new GeomVertexColumn;
    column->fillin(scan, manager);
    _columns.push_back(column);
  }
  _columns_unsorted = false;
}

/**
 *
 */
GeomVertexArrayFormat::Registry::
Registry() {
}

/**
 * Adds the indicated format to the registry, if there is not an equivalent
 * format already there; in either case, returns the pointer to the equivalent
 * format now in the registry.
 *
 * This must be called before a format may be used in a Geom.  After this
 * call, you should discard the original pointer you passed in (which may or
 * may not now be invalid) and let its reference count decrement normally; you
 * should use only the returned value from this point on.
 */
CPT(GeomVertexArrayFormat) GeomVertexArrayFormat::Registry::
register_format(GeomVertexArrayFormat *format) {
  if (format->is_registered()) {
    return format;
  }

  // Save the incoming pointer in a local PointerTo, so that if it has a zero
  // reference count and is not added into the map below, it will be
  // automatically deleted when this function returns.
  PT(GeomVertexArrayFormat) pt_format = format;

  GeomVertexArrayFormat *new_format;
  {
    LightMutexHolder holder(_lock);
    ArrayFormats::iterator fi = _formats.insert(format).first;
    new_format = (*fi);
    if (!new_format->is_registered()) {
      new_format->do_register();
    }
  }

  return new_format;
}

/**
 * Removes the indicated format from the registry.  Normally this should not
 * be done until the format is destructing.
 *
 * The lock should be held prior to calling this method.
 */
void GeomVertexArrayFormat::Registry::
unregister_format(GeomVertexArrayFormat *format) {
  nassertv(format->is_registered());
  ArrayFormats::iterator fi = _formats.find(format);
  nassertv(fi != _formats.end());
  _formats.erase(fi);
  format->do_unregister();
}
