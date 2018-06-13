/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexFormat.cxx
 * @author drose
 * @date 2005-03-07
 */

#include "geomVertexFormat.h"
#include "geomVertexData.h"
#include "geomMunger.h"
#include "lightReMutexHolder.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"

GeomVertexFormat::Registry *GeomVertexFormat::_registry = nullptr;
TypeHandle GeomVertexFormat::_type_handle;

/**
 *
 */
GeomVertexFormat::
GeomVertexFormat() :
  _is_registered(false),
  _post_animated_format(nullptr)
{
}

/**
 *
 */
GeomVertexFormat::
GeomVertexFormat(const GeomVertexArrayFormat *array_format) :
  _is_registered(false),
  _post_animated_format(nullptr)
{
  add_array(array_format);
}

/**
 *
 */
GeomVertexFormat::
GeomVertexFormat(const GeomVertexFormat &copy) :
  _is_registered(false),
  _animation(copy._animation),
  _arrays(copy._arrays),
  _post_animated_format(nullptr)
{
}

/**
 *
 */
void GeomVertexFormat::
operator = (const GeomVertexFormat &copy) {
  nassertv(!is_registered());

  _animation = copy._animation;
  _arrays = copy._arrays;
}

/**
 *
 */
GeomVertexFormat::
~GeomVertexFormat() {
  // unref() should have unregistered us.
  nassertv(!is_registered());
}

/**
 * This method overrides ReferenceCount::unref() to unregister the object when
 * its reference count goes to zero.
 */
bool GeomVertexFormat::
unref() const {
  Registry *registry = get_registry();
  LightReMutexHolder holder(registry->_lock);

  if (ReferenceCount::unref()) {
    return true;
  }

  if (is_registered()) {
    registry->unregister_format((GeomVertexFormat *)this);
  }

  return false;
}

/**
 * Returns a suitable vertex format for sending the animated vertices to the
 * graphics backend.  This is the same format as the source format, with the
 * CPU-animation data elements removed.
 *
 * This may only be called after the format has been registered.  The return
 * value will have been already registered.
 */
CPT(GeomVertexFormat) GeomVertexFormat::
get_post_animated_format() const {
  nassertr(is_registered(), nullptr);

  if (_post_animated_format == nullptr) {
    PT(GeomVertexFormat) new_format = new GeomVertexFormat(*this);
    new_format->remove_column(InternalName::get_transform_blend());

    int num_morphs = get_num_morphs();
    for (int mi = 0; mi < num_morphs; mi++) {
      CPT(InternalName) delta_name = get_morph_delta(mi);
      new_format->remove_column(delta_name);
    }

    new_format->_animation.set_none();

    CPT(GeomVertexFormat) registered =
      GeomVertexFormat::register_format(new_format);
    ((GeomVertexFormat *)this)->_post_animated_format = registered;
    if (_post_animated_format != this) {
      // We only keep the reference count if the new pointer is not the same
      // as this, to avoid a circular dependency.
      _post_animated_format->ref();
    }
  }

  _post_animated_format->test_ref_count_integrity();

  return _post_animated_format;
}

/**
 * Returns a new GeomVertexFormat that includes all of the columns defined in
 * either this GeomVertexFormat or the other one.  If any column is defined in
 * both formats with different sizes (for instance, texcoord2 vs.  texcoord3),
 * the new format will include the larger of the two definitions.
 *
 * This may only be called after both source formats have been registered.
 * The return value will also have been already registered.
 */
CPT(GeomVertexFormat) GeomVertexFormat::
get_union_format(const GeomVertexFormat *other) const {
  nassertr(is_registered() && other->is_registered(), nullptr);

  PT(GeomVertexFormat) new_format = new GeomVertexFormat;

  // Preserve whichever animation type is not AT_None.  (If both animation
  // types are not AT_None, but they are different, this whole operation is
  // questionable.)
  if (_animation.get_animation_type() != AT_none) {
    new_format->set_animation(_animation);
  } else {
    new_format->set_animation(other->get_animation());
  }

  // Keep track of the columns we have already added.
  typedef pset< CPT(InternalName) > ColumnNames;
  ColumnNames column_names;

  // We go through all the (0)-level arrays first, then all the (1)-level
  // arrays, and so on.  We do this to ensure that the new format gets written
  // out with all the (0)-level columns appearing before all the (1)-level
  // columns, which might lead to a small optimization at render time.

  // We also try to keep the structure as similar as possible.  If both source
  // formats have columns (A, B) in array 0, and columns (C, D, E) in array 1,
  // then the resulting union format will also have (A, B) in array 0 and (C,
  // D, E) in array 1.  In general, a column will appear in the result in the
  // first array it appears in either of the inputs.

  size_t num_arrays = std::max(_arrays.size(), other->_arrays.size());
  for (size_t ai = 0; ai < num_arrays; ++ai) {
    PT(GeomVertexArrayFormat) new_array = new GeomVertexArrayFormat;

    // Add the columns from the first format.
    if (ai < _arrays.size()) {
      GeomVertexArrayFormat *array_format = _arrays[ai];
      size_t num_columns = array_format->get_num_columns();
      for (size_t i = 0; i < num_columns; ++i) {
        const GeomVertexColumn *column_a = array_format->get_column(i);
        bool inserted = column_names.insert(column_a->get_name()).second;
        if (inserted) {
          const GeomVertexColumn *column_b = other->get_column(column_a->get_name());
          if (column_b != nullptr &&
              column_b->get_total_bytes() > column_a->get_total_bytes()) {
            // Column b is larger.  Keep it.
            new_array->add_column(column_b->get_name(),
                                  column_b->get_num_components(),
                                  column_b->get_numeric_type(),
                                  column_b->get_contents());
          } else {
            // Column a is larger.  Keep it.
            new_array->add_column(column_a->get_name(),
                                  column_a->get_num_components(),
                                  column_a->get_numeric_type(),
                                  column_a->get_contents());
          }
        }
      }
    }

    // Add the columns from the second format.
    if (ai < other->_arrays.size()) {
      GeomVertexArrayFormat *array_format = other->_arrays[ai];
      size_t num_columns = array_format->get_num_columns();
      for (size_t i = 0; i < num_columns; ++i) {
        const GeomVertexColumn *column_a = array_format->get_column(i);
        bool inserted = column_names.insert(column_a->get_name()).second;
        if (inserted) {
          const GeomVertexColumn *column_b = get_column(column_a->get_name());
          if (column_b != nullptr &&
              column_b->get_total_bytes() > column_a->get_total_bytes()) {
            // Column b is larger.  Keep it.
            new_array->add_column(column_b->get_name(),
                                  column_b->get_num_components(),
                                  column_b->get_numeric_type(),
                                  column_b->get_contents());
          } else {
            // Column a is larger.  Keep it.
            new_array->add_column(column_a->get_name(),
                                  column_a->get_num_components(),
                                  column_a->get_numeric_type(),
                                  column_a->get_contents());
          }
        }
      }
    }

    if (new_array->get_num_columns() != 0) {
      new_format->add_array(new_array);
    }
  }

  // Finally, register the format for the thing.
  return GeomVertexFormat::register_format(new_format);
}

/**
 * Returns a modifiable pointer to the indicated array.  This means
 * duplicating it if it is shared or registered.
 *
 * This may not be called once the format has been registered.
 */
GeomVertexArrayFormat *GeomVertexFormat::
modify_array(size_t array) {
  nassertr(!is_registered(), nullptr);
  nassertr(array < _arrays.size(), nullptr);

  if (_arrays[array]->is_registered() ||
      _arrays[array]->get_ref_count() > 1) {
    _arrays[array] = new GeomVertexArrayFormat(*_arrays[array]);
  }

  return _arrays[array];
}

/**
 * Replaces the definition of the indicated array.
 *
 * This may not be called once the format has been registered.
 */
void GeomVertexFormat::
set_array(size_t array, const GeomVertexArrayFormat *format) {
  nassertv(!is_registered());
  nassertv(array < _arrays.size());
  _arrays[array] = (GeomVertexArrayFormat *)format;
}

/**
 * Removes the nth array from the format.
 *
 * This may not be called once the format has been registered.
 */
void GeomVertexFormat::
remove_array(size_t array) {
  nassertv(!is_registered());

  nassertv(array < _arrays.size());
  _arrays.erase(_arrays.begin() + array);
}

/**
 * Adds the indicated array definition to the list of arrays included within
 * this vertex format definition.  The return value is the index number of the
 * new array.
 *
 * This may not be called once the format has been registered.
 */
size_t GeomVertexFormat::
add_array(const GeomVertexArrayFormat *array_format) {
  nassertr(!is_registered(), 0);

  size_t new_array = _arrays.size();
  _arrays.push_back((GeomVertexArrayFormat *)array_format);
  return new_array;
}

/**
 * Adds the indicated array definition to the list of arrays at the indicated
 * position.  This works just like add_array(), except that you can specify
 * which array index the new array should have.
 *
 * This may not be called once the format has been registered.
 */
void GeomVertexFormat::
insert_array(size_t array, const GeomVertexArrayFormat *array_format) {
  nassertv(!is_registered());
  if (array > _arrays.size()) {
    array = _arrays.size();
  }

  _arrays.insert(_arrays.begin() + array, (GeomVertexArrayFormat *)array_format);
}

/**
 * Removes all of the array definitions from the format and starts over.
 *
 * This may not be called once the format has been registered.
 */
void GeomVertexFormat::
clear_arrays() {
  nassertv(!is_registered());

  _arrays.clear();
}

/**
 * Removes the arrays that define no columns.
 *
 * This may not be called once the format has been registered.
 */
void GeomVertexFormat::
remove_empty_arrays() {
  nassertv(!is_registered());

  Arrays orig_arrays;
  orig_arrays.swap(_arrays);
  Arrays::const_iterator ai;
  for (ai = orig_arrays.begin(); ai != orig_arrays.end(); ++ai) {
    GeomVertexArrayFormat *array_format = (*ai);
    if (array_format->get_num_columns() != 0) {
      _arrays.push_back(array_format);
    }
  }
}

/**
 * Returns the total number of different columns in the specification, across
 * all arrays.
 */
size_t GeomVertexFormat::
get_num_columns() const {
  size_t num_columns = 0;
  Arrays::const_iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    num_columns += (*ai)->get_num_columns();
  }
  return num_columns;
}

/**
 * Returns the ith column of the specification, across all arrays.
 */
const GeomVertexColumn *GeomVertexFormat::
get_column(size_t i) const {
  Arrays::const_iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    if (i < (size_t)(*ai)->get_num_columns()) {
      return (*ai)->get_column(i);
    }
    i -= (*ai)->get_num_columns();
  }

  return nullptr;
}

/**
 * Returns the name of the ith column, across all arrays.
 */
const InternalName *GeomVertexFormat::
get_column_name(size_t i) const {
  Arrays::const_iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    if (i < (size_t)(*ai)->get_num_columns()) {
      return (*ai)->get_column(i)->get_name();
    }
    i -= (*ai)->get_num_columns();
  }

  return nullptr;
}

/**
 * Returns the index number of the array with the ith column.
 *
 * The return value can be passed to get_array_format() to get the format of
 * the array.  It may also be passed to GeomVertexData::get_array_data() or
 * get_data() or set_data() to manipulate the actual array data.
 */
int GeomVertexFormat::
get_array_with(size_t i) const {
  int array_index = 0;
  for (array_index = 0; array_index < (int)_arrays.size(); array_index++) {
    if (i < (size_t)_arrays[array_index]->get_num_columns()) {
      return array_index;
    }
    i -= _arrays[array_index]->get_num_columns();
  }

  return -1;
}

/**
 * Returns the index number of the array with the indicated column, or -1 if
 * no arrays contained that name.
 *
 * The return value can be passed to get_array_format() to get the format of
 * the array.  It may also be passed to GeomVertexData::get_array_data() or
 * get_data() or set_data() to manipulate the actual array data.
 *
 * This may only be called after the format has been registered.
 */
int GeomVertexFormat::
get_array_with(const InternalName *name) const {
  nassertr(_is_registered, -1);

  DataTypesByName::const_iterator ai;
  ai = _columns_by_name.find(name);
  if (ai != _columns_by_name.end()) {
    return (*ai).second._array_index;
  }
  return -1;
}

/**
 * Returns the specification with the indicated name, or NULL if the name is
 * not used.  Use get_array_with() to determine which array this column is
 * associated with.
 */
const GeomVertexColumn *GeomVertexFormat::
get_column(const InternalName *name) const {
  if (!_is_registered) {
    // If the format hasn't yet been registered, we have to search for the
    // column the hard way.
    Arrays::const_iterator ai;
    for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
      const GeomVertexColumn *column = (*ai)->get_column(name);
      if (column != nullptr) {
        return column;
      }
    }
    return nullptr;

  } else {
    // If the format has been registered, we can just check the toplevel
    // index.

    DataTypesByName::const_iterator ai;
    ai = _columns_by_name.find(name);
    if (ai != _columns_by_name.end()) {
      int array_index = (*ai).second._array_index;
      int column_index = (*ai).second._column_index;

      nassertr(array_index >= 0 && array_index < (int)_arrays.size(), nullptr);
      return _arrays[array_index]->get_column(column_index);
    }
    return nullptr;
  }
}

/**
 * Removes the named column from the format, from whichever array it exists
 * in.  If there are other columns remaining in the array, the array is left
 * with a gap where the column used to be; if this was the only column in the
 * array, the array is removed (unless keep_empty_array is true).
 *
 * This may not be called once the format has been registered.
 */
void GeomVertexFormat::
remove_column(const InternalName *name, bool keep_empty_array) {
  nassertv(!_is_registered);

  // Since the format's not registered, it doesn't yet have an index of
  // columns--so we have to search all of the arrays, one at a time, until we
  // find it.
  for (int array = 0; array < (int)_arrays.size(); ++array) {
    GeomVertexArrayFormat *array_format = _arrays[array];

    if (array_format->get_column(name) != nullptr) {
      // Here's the array with the named column!
      if (array_format->is_registered() ||
          array_format->get_ref_count() > 1) {
        // Get a safe-to-modify copy of the array format.
        _arrays[array] = new GeomVertexArrayFormat(*array_format);
        array_format = _arrays[array];
      }

      array_format->remove_column(name);

      // Are there any columns remaining in the array?
      if (!keep_empty_array && array_format->get_num_columns() == 0) {
        // Remove the whole array.
        remove_array(array);
      }
      return;
    }
  }

  // It appears that column wasn't part of the format anyway.  No problem;
  // quietly return.
}

/**
 * Removes wasted space between columns.
 */
void GeomVertexFormat::
pack_columns() {
  nassertv(!_is_registered);
  Arrays::iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    if ((*ai)->is_registered()) {
      (*ai) = new GeomVertexArrayFormat(*(*ai));
    }
    (*ai)->pack_columns();
  }
}

/**
 * Reprocesses the columns in the format to align the C_point and C_vector
 * columns to 16-byte boundaries to allow for the more efficient SSE2
 * operations (assuming SSE2 is enabled in the build).
 *
 * Also see maybe_align_columns_for_animation().
 */
void GeomVertexFormat::
align_columns_for_animation() {
  nassertv(!_is_registered);
  Arrays::iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    if ((*ai)->is_registered()) {
      (*ai) = new GeomVertexArrayFormat(*(*ai));
    }
    (*ai)->align_columns_for_animation();
  }
}

/**
 * Calls align_columns_for_animation() if this format's AnimationSpec
 * indicates that it contains animated vertices, and if vertex-animation-
 * align-16 is true.
 */
void GeomVertexFormat::
maybe_align_columns_for_animation() {
  if (_animation.get_animation_type() == AT_panda && vertex_animation_align_16) {
    align_columns_for_animation();
  }
}

/**
 *
 */
void GeomVertexFormat::
output(std::ostream &out) const {
  if (_arrays.empty()) {
    out << "(empty)";

  } else {
    Arrays::const_iterator ai;
    ai = _arrays.begin();
    out << *(*ai);
    ++ai;
    while (ai != _arrays.end()) {
      out << ", " << *(*ai);
      ++ai;
    }
  }

  if (_animation.get_animation_type() != AT_none) {
    out << ", anim " << _animation;
  }
}

/**
 *
 */
void GeomVertexFormat::
write(std::ostream &out, int indent_level) const {
  for (size_t i = 0; i < _arrays.size(); i++) {
    indent(out, indent_level)
      << "Array " << i << ":\n";
    _arrays[i]->write(out, indent_level + 2);
  }

  if (_animation.get_animation_type() != AT_none) {
    indent(out, indent_level)
      << "anim " << _animation << "\n";
  }
}

/**
 *
 */
void GeomVertexFormat::
write_with_data(std::ostream &out, int indent_level,
                const GeomVertexData *data) const {
  indent(out, indent_level)
    << data->get_num_rows() << " rows.\n";
  for (size_t i = 0; i < _arrays.size(); i++) {
    CPT(GeomVertexArrayDataHandle) handle = data->get_array(i)->get_handle();
    const unsigned char *array_data = handle->get_read_pointer(true);
    indent(out, indent_level)
      << "Array " << i << " (" << (void *)array_data << ", "
      << *_arrays[i] << "):\n";
    _arrays[i]->write_with_data(out, indent_level + 2, data->get_array(i));
  }
}

/**
 * Quickly looks up the indicated column within all of the nested arrays and
 * sets array_index and column appropriately.  Returns true if the data type
 * exists in this format, false if it does not.  If it returns false,
 * array_index is set to -1, and column is set to NULL.
 *
 * This may only be called after the format has been registered.
 */
bool GeomVertexFormat::
get_array_info(const InternalName *name, int &array_index,
               const GeomVertexColumn *&column) const {
  nassertr(_is_registered, false);

  DataTypesByName::const_iterator ai;
  ai = _columns_by_name.find(name);
  if (ai != _columns_by_name.end()) {
    array_index = (*ai).second._array_index;
    column = _arrays[array_index]->get_column((*ai).second._column_index);
    return true;
  }

  array_index = -1;
  column = nullptr;

  return false;
}

/**
 *
 */
int GeomVertexFormat::
compare_to(const GeomVertexFormat &other) const {
  int compare = _animation.compare_to(other._animation);
  if (compare != 0) {
    return compare;
  }

  if (_arrays.size() != other._arrays.size()) {
    return (int)_arrays.size() - (int)other._arrays.size();
  }

  for (size_t i = 0; i < _arrays.size(); i++) {
    int compare = _arrays[i]->compare_to(*other._arrays[i]);
    if (compare != 0) {
      return compare;
    }
  }

  return 0;
}

/**
 * Returns the global registry object.
 */
void GeomVertexFormat::
make_registry() {
  if (_registry == nullptr) {
    _registry = new Registry;
    _registry->make_standard_formats();
  }
}

/**
 * Called internally when the format is registered.
 */
void GeomVertexFormat::
do_register() {
  nassertv(!is_registered());
  nassertv(_columns_by_name.empty());

  Arrays orig_arrays;
  orig_arrays.swap(_arrays);
  Arrays::const_iterator ai;
  for (ai = orig_arrays.begin(); ai != orig_arrays.end(); ++ai) {
    CPT(GeomVertexArrayFormat) array_format = (*ai);
    if (!array_format->is_registered()) {
      array_format = GeomVertexArrayFormat::register_format(array_format);
    }

    // Let's keep arrays with nonzero stride but no used columns; they're
    // needed to preserve the isomorphic nature of matching formats.  But
    // we'll toss arrays with 0 stride, which add nothing of value and only
    // cause problems later.
    if (array_format->get_stride() == 0) {
      gobj_cat.warning()
        << "Dropping empty array from GeomVertexFormat.\n";
      continue;
    }

    int array = (int)_arrays.size();
    _arrays.push_back((GeomVertexArrayFormat *)array_format.p());

    // Now add the names to the index.
    int num_columns = array_format->get_num_columns();
    for (int i = 0; i < num_columns; i++) {
      const GeomVertexColumn *column = array_format->get_column(i);
      std::pair<DataTypesByName::iterator, bool> result;
      result = _columns_by_name.insert(DataTypesByName::value_type(column->get_name(), DataTypeRecord()));
      if (!result.second) {
        gobj_cat.warning()
          << "Column " << *column->get_name() << " repeated in format.\n";
      } else {
        DataTypeRecord &record = (*result.first).second;
        record._array_index = array;
        record._column_index = i;
      }
    }
  }

  // Go back through the index now and identify the points, vectors, and morph
  // descriptions, so we can quickly look these up later.
  DataTypesByName::iterator ni;
  for (ni = _columns_by_name.begin();
       ni != _columns_by_name.end();
       ++ni) {
    const DataTypeRecord &record = (*ni).second;
    const GeomVertexColumn *column = _arrays[record._array_index]->get_column(record._column_index);

    switch (column->get_contents()) {
    case C_point:
      // It's a point.
      _points.push_back(column->get_name());
      break;

    case C_vector:
    case C_normal:
      // It's a vector.
      _vectors.push_back(column->get_name());
      break;

    case C_texcoord:
      // It's a texcoord.
      _texcoords.push_back(column->get_name());
      break;

    case C_morph_delta:
      {
        // It's a morph description.
        MorphRecord morph;
        morph._delta = column->get_name();

        // The delta name must be of the form "basename.morph.slidername".
        int n = morph._delta->find_ancestor("morph");
        if (n < 0) {
          gobj_cat.warning()
            << "vertex format defines " << *column->get_name()
            << ", which is stored as a C_morph_delta, but its name does not include \"morph\".\n";
        } else {
          morph._slider = InternalName::make(morph._delta->get_net_basename(n - 1));
          morph._base = morph._delta->get_ancestor(n + 1);

          if (_columns_by_name.find(morph._base) == _columns_by_name.end()) {
            gobj_cat.warning()
              << "vertex format defines "
              << *column->get_name() << " but does not define "
              << *morph._base << "\n";
          } else {
            _morphs.push_back(morph);
          }
        }
      }
      break;

    default:
      // Some other type of value we don't care about caching a pointer to.
      break;
    }
  }

  _is_registered = true;

  get_array_info(InternalName::get_vertex(), _vertex_array_index,
                 _vertex_column);
  get_array_info(InternalName::get_normal(), _normal_array_index,
                 _normal_column);
  get_array_info(InternalName::get_color(), _color_array_index,
                 _color_column);
}

/**
 * Called internally when the format is unregistered.
 */
void GeomVertexFormat::
do_unregister() {
  nassertv(_is_registered);
  _is_registered = false;

  _columns_by_name.clear();
  _points.clear();
  _vectors.clear();
  _texcoords.clear();
  _morphs.clear();

  if (_post_animated_format != nullptr &&
      _post_animated_format != this) {
    unref_delete(_post_animated_format);
  }
  _post_animated_format = nullptr;
}

/**
 * Tells the BamReader how to create objects of type GeomVertexFormat.
 */
void GeomVertexFormat::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void GeomVertexFormat::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  _animation.write_datagram(manager, dg);

  dg.add_uint16(_arrays.size());
  Arrays::const_iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    manager->write_pointer(dg, *ai);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int GeomVertexFormat::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  Arrays::iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    (*ai) = DCAST(GeomVertexArrayFormat, p_list[pi++]);
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type GeomVertexFormat is encountered in the Bam file.  It should create the
 * GeomVertexFormat and extract its information from the file.
 */
TypedWritable *GeomVertexFormat::
make_from_bam(const FactoryParams &params) {
  GeomVertexFormat *object = new GeomVertexFormat;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new GeomVertexFormat.
 */
void GeomVertexFormat::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  _animation.fillin(scan, manager);

  int num_arrays = scan.get_uint16();
  _arrays.reserve(num_arrays);
  for (int i = 0; i < num_arrays; i++) {
    manager->read_pointer(scan);
    _arrays.push_back(nullptr);
  }
}

/**
 *
 */
GeomVertexFormat::Registry::
Registry() {
}

/**
 *
 */
void GeomVertexFormat::Registry::
make_standard_formats() {
  _empty = register_format(new GeomVertexFormat);

  _v3 = register_format(new GeomVertexArrayFormat
                        (InternalName::get_vertex(), 3,
                         NT_stdfloat, C_point));

  _v3n3 = register_format(new GeomVertexArrayFormat
                          (InternalName::get_vertex(), 3,
                           NT_stdfloat, C_point,
                           InternalName::get_normal(), 3,
                           NT_stdfloat, C_normal));

  _v3t2 = register_format(new GeomVertexArrayFormat
                          (InternalName::get_vertex(), 3,
                           NT_stdfloat, C_point,
                           InternalName::get_texcoord(), 2,
                           NT_stdfloat, C_texcoord));

  _v3n3t2 = register_format(new GeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_stdfloat, C_point,
                             InternalName::get_normal(), 3,
                             NT_stdfloat, C_normal,
                             InternalName::get_texcoord(), 2,
                             NT_stdfloat, C_texcoord));

  // Define the DirectX-style packed color formats
  _v3cp = register_format(new GeomVertexArrayFormat
                          (InternalName::get_vertex(), 3,
                           NT_stdfloat, C_point,
                           InternalName::get_color(), 1,
                           NT_packed_dabc, C_color));

  _v3n3cp = register_format(new GeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_stdfloat, C_point,
                             InternalName::get_normal(), 3,
                             NT_stdfloat, C_normal,
                             InternalName::get_color(), 1,
                             NT_packed_dabc, C_color));

  _v3cpt2 = register_format(new GeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_stdfloat, C_point,
                             InternalName::get_color(), 1,
                             NT_packed_dabc, C_color,
                             InternalName::get_texcoord(), 2,
                             NT_stdfloat, C_texcoord));

  _v3n3cpt2 = register_format(new GeomVertexArrayFormat
                              (InternalName::get_vertex(), 3,
                               NT_stdfloat, C_point,
                               InternalName::get_normal(), 3,
                               NT_stdfloat, C_normal,
                               InternalName::get_color(), 1,
                               NT_packed_dabc, C_color,
                               InternalName::get_texcoord(), 2,
                               NT_stdfloat, C_texcoord));

  // Define the OpenGL-style per-byte color formats.  This is not the same as
  // a packed format, above, because the resulting byte order is endian-
  // independent.
  _v3c4 = register_format(new GeomVertexArrayFormat
                          (InternalName::get_vertex(), 3,
                           NT_stdfloat, C_point,
                           InternalName::get_color(), 4,
                           NT_uint8, C_color));

  _v3n3c4 = register_format(new GeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_stdfloat, C_point,
                             InternalName::get_normal(), 3,
                             NT_stdfloat, C_normal,
                             InternalName::get_color(), 4,
                             NT_uint8, C_color));

  _v3c4t2 = register_format(new GeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_stdfloat, C_point,
                             InternalName::get_color(), 4,
                             NT_uint8, C_color,
                             InternalName::get_texcoord(), 2,
                             NT_stdfloat, C_texcoord));

  _v3n3c4t2 = register_format(new GeomVertexArrayFormat
                              (InternalName::get_vertex(), 3,
                               NT_stdfloat, C_point,
                               InternalName::get_normal(), 3,
                               NT_stdfloat, C_normal,
                               InternalName::get_color(), 4,
                               NT_uint8, C_color,
                               InternalName::get_texcoord(), 2,
                               NT_stdfloat, C_texcoord));
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
CPT(GeomVertexFormat) GeomVertexFormat::Registry::
register_format(GeomVertexFormat *format) {
  if (format->is_registered()) {
    return format;
  }

  // Save the incoming pointer in a local PointerTo, so that if it has a zero
  // reference count and is not added into the map below, it will be
  // automatically deleted when this function returns.
  PT(GeomVertexFormat) pt_format = format;

  GeomVertexFormat *new_format;
  {
    LightReMutexHolder holder(_lock);
    Formats::iterator fi = _formats.insert(format).first;
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
void GeomVertexFormat::Registry::
unregister_format(GeomVertexFormat *format) {
  nassertv(format->is_registered());
  Formats::iterator fi = _formats.find(format);
  nassertv(fi != _formats.end());
  _formats.erase(fi);
  format->do_unregister();
}
