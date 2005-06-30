// Filename: geomVertexFormat.cxx
// Created by:  drose (07Mar05)
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
#include "geomVertexData.h"
#include "geomMunger.h"
#include "mutexHolder.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"

GeomVertexFormat::Registry *GeomVertexFormat::_registry = NULL;
TypeHandle GeomVertexFormat::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexFormat::
GeomVertexFormat() :
  _is_registered(false),
  _post_animated_format(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexFormat::
GeomVertexFormat(const GeomVertexArrayFormat *array_format) :
  _is_registered(false),
  _post_animated_format(NULL)
{
  add_array(array_format);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexFormat::
GeomVertexFormat(const GeomVertexFormat &copy) :
  _is_registered(false),
  _animation(copy._animation),
  _arrays(copy._arrays),
  _post_animated_format(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
operator = (const GeomVertexFormat &copy) {
  nassertv(!is_registered());

  _animation = copy._animation;
  _arrays = copy._arrays;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexFormat::
~GeomVertexFormat() {
  if (is_registered()) {
    get_registry()->unregister_format(this);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_post_animated_format
//       Access: Published
//  Description: Returns a suitable vertex format for sending the
//               animated vertices to the graphics backend.  This is
//               the same format as the source format, with the
//               CPU-animation data elements removed.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexFormat) GeomVertexFormat::
get_post_animated_format() const {
  nassertr(is_registered(), NULL);

  if (_post_animated_format == (GeomVertexFormat *)NULL) {
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
      // We only keep the reference count if the new pointer is not
      // the same as this, to avoid a circular dependency.
      _post_animated_format->ref();
    }
  }

  _post_animated_format->test_ref_count_integrity();

  return _post_animated_format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::modify_array
//       Access: Published
//  Description: Returns a modifiable pointer to the indicated array.
//               This means duplicating it if it is shared or
//               registered.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
GeomVertexArrayFormat *GeomVertexFormat::
modify_array(int array) {
  nassertr(!is_registered(), NULL);
  nassertr(array >= 0 && array < (int)_arrays.size(), NULL);

  if (_arrays[array]->is_registered() ||
      _arrays[array]->get_ref_count() > 1) {
    _arrays[array] = new GeomVertexArrayFormat(*_arrays[array]);
  }

  return _arrays[array];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::set_array
//       Access: Published
//  Description: Replaces the definition of the indicated array.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
set_array(int array, const GeomVertexArrayFormat *format) {
  nassertv(!is_registered());
  nassertv(array >= 0 && array < (int)_arrays.size());
  _arrays[array] = (GeomVertexArrayFormat *)format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::remove_array
//       Access: Published
//  Description: Removes the nth array from the format.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
remove_array(int array) {
  nassertv(!is_registered());

  nassertv(array >= 0 && array < (int)_arrays.size());
  _arrays.erase(_arrays.begin() + array);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::add_array
//       Access: Published
//  Description: Adds the indicated array definition to the list of
//               arrays included within this vertex format definition.
//               The return value is the index number of the new
//               array.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
int GeomVertexFormat::
add_array(const GeomVertexArrayFormat *array_format) {
  nassertr(!is_registered(), -1);

  int new_array = (int)_arrays.size();
  _arrays.push_back((GeomVertexArrayFormat *)array_format);
  return new_array;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::insert_array
//       Access: Published
//  Description: Adds the indicated array definition to the list of
//               arrays at the indicated position.  This works just
//               like add_array(), except that you can specify which
//               array index the new array should have.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
insert_array(int array, const GeomVertexArrayFormat *array_format) {
  nassertv(!is_registered());
  nassertv(array >= 0 && array <= (int)_arrays.size());

  _arrays.insert(_arrays.begin() + array, (GeomVertexArrayFormat *)array_format);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::clear_arrays
//       Access: Published
//  Description: Removes all of the array definitions from the format
//               and starts over.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
clear_arrays() {
  nassertv(!is_registered());

  _arrays.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_num_columns
//       Access: Published
//  Description: Returns the total number of different columns in
//               the specification, across all arrays.
////////////////////////////////////////////////////////////////////
int GeomVertexFormat::
get_num_columns() const {
  int num_columns = 0;
  Arrays::const_iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    num_columns += (*ai)->get_num_columns();
  }
  return num_columns;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_column
//       Access: Published
//  Description: Returns the ith column of the specification,
//               across all arrays.
////////////////////////////////////////////////////////////////////
const GeomVertexColumn *GeomVertexFormat::
get_column(int i) const {
  Arrays::const_iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    if (i < (*ai)->get_num_columns()) {
      return (*ai)->get_column(i);
    }
    i -= (*ai)->get_num_columns();
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_array_with
//       Access: Published
//  Description: Returns the index number of the array with the
//               ith column.
//
//               The return value can be passed to get_array_format()
//               to get the format of the array.  It may also be
//               passed to GeomVertexData::get_array_data() or
//               get_data() or set_data() to manipulate the actual
//               array data.
////////////////////////////////////////////////////////////////////
int GeomVertexFormat::
get_array_with(int i) const {
  int array_index = 0;
  for (array_index = 0; array_index < (int)_arrays.size(); array_index++) {
    if (i < _arrays[array_index]->get_num_columns()) {
      return array_index;
    }
    i -= _arrays[array_index]->get_num_columns();
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_array_with
//       Access: Published
//  Description: Returns the index number of the array with the
//               indicated column, or -1 if no arrays contained
//               that name.
//
//               The return value can be passed to get_array_format()
//               to get the format of the array.  It may also be
//               passed to GeomVertexData::get_array_data() or
//               get_data() or set_data() to manipulate the actual
//               array data.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_column
//       Access: Published
//  Description: Returns the specification with the indicated name, or
//               NULL if the name is not used.  Use get_array_with()
//               to determine which array this column is associated
//               with.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
const GeomVertexColumn *GeomVertexFormat::
get_column(const InternalName *name) const {
  nassertr(_is_registered, NULL);

  DataTypesByName::const_iterator ai;
  ai = _columns_by_name.find(name);
  if (ai != _columns_by_name.end()) {
    int array_index = (*ai).second._array_index;
    int column_index = (*ai).second._column_index;

    nassertr(array_index >= 0 && array_index < (int)_arrays.size(), NULL);
    return _arrays[array_index]->get_column(column_index);
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::remove_column
//       Access: Published
//  Description: Removes the named column from the format, from
//               whichever array it exists in.  If there are other
//               columns remaining in the array, the array is left
//               with a gap where the column used to be; if this
//               was the only column in the array, the array is
//               removed.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
remove_column(const InternalName *name) {
  nassertv(!is_registered());

  // Since the format's not registered, it doesn't yet have an index
  // of columns--so we have to search all of the arrays, one at a
  // time, until we find it.
  for (int array = 0; array < (int)_arrays.size(); ++array) {
    GeomVertexArrayFormat *array_format = _arrays[array];

    if (array_format->get_column(name) != (GeomVertexColumn *)NULL) {
      // Here's the array with the named column!
      if (array_format->is_registered() ||
          array_format->get_ref_count() > 1) {
        // Get a safe-to-modify copy of the array format.
        _arrays[array] = new GeomVertexArrayFormat(*array_format);
        array_format = _arrays[array];
      }

      array_format->remove_column(name);

      // Are there any columns remaining in the array?
      if (array_format->get_num_columns() == 0) {
        // Remove the whole array.
        remove_array(array);
      }
      return;
    }
  }

  // It appears that column wasn't part of the format anyway.  No
  // problem; quietly return.
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
output(ostream &out) const {
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

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
write(ostream &out, int indent_level) const {
  for (size_t i = 0; i < _arrays.size(); i++) {
    indent(out, indent_level)
      << "Array " << i << ":\n";
    _arrays[i]->write(out, indent_level + 2);
  }

  if (_animation.get_animation_type() != AT_none) {
    indent(out, indent_level)
      << "anim " << _animation;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::write_with_data
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
write_with_data(ostream &out, int indent_level, 
                const GeomVertexData *data) const {
  indent(out, indent_level)
    << data->get_num_rows() << " rows.\n";
  for (size_t i = 0; i < _arrays.size(); i++) {
    CPTA_uchar array_data = data->get_array(i)->get_data();
    indent(out, indent_level)
      << "Array " << i << " (" << (void *)array_data.p() << ", "
      << *_arrays[i] << "):\n";
    _arrays[i]->write_with_data(out, indent_level + 2, data->get_array(i));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_array_info
//       Access: Public
//  Description: Quickly looks up the indicated column within all
//               of the nested arrays and sets array_index and
//               column appropriately.  Returns true if the data
//               type exists in this format, false if it does not.  If
//               it returns false, array_index is set to -1, and
//               column is set to NULL.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
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
  column = NULL;

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::make_registry
//       Access: Private
//  Description: Returns the global registry object.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
make_registry() {
  if (_registry == (Registry *)NULL) {
    _registry = new Registry;
  }
}
 
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::do_register
//       Access: Private
//  Description: Called internally when the format is registered.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
do_register() {
  nassertv(!is_registered());
  nassertv(_columns_by_name.empty());

  for (int array = 0; array < (int)_arrays.size(); ++array) {
    CPT(GeomVertexArrayFormat) array_format = _arrays[array];
    if (!array_format->is_registered()) {
      array_format = GeomVertexArrayFormat::register_format(array_format);
      _arrays[array] = (GeomVertexArrayFormat *)array_format.p();
    }

    // Now add the names to the index.
    int num_columns = array_format->get_num_columns();
    for (int i = 0; i < num_columns; i++) {
      const GeomVertexColumn *column = array_format->get_column(i);
      pair<DataTypesByName::iterator, bool> result;
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

  // Go back through the index now and identify the points, vectors,
  // and morph descriptions, so we can quickly look these up later.
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
      // Some other type of value we don't care about caching a
      // pointer to.
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
 
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::do_unregister
//       Access: Private
//  Description: Called internally when the format is unregistered.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
do_unregister() {
  nassertv(_is_registered);
  _is_registered = false;

  _columns_by_name.clear();
  _points.clear();
  _vectors.clear();
  _texcoords.clear();
  _morphs.clear();

  if (_post_animated_format != (GeomVertexFormat *)NULL && 
      _post_animated_format != this) {
    unref_delete(_post_animated_format);
  }
  _post_animated_format = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               GeomVertexFormat.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int GeomVertexFormat::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  Arrays::iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    (*ai) = DCAST(GeomVertexArrayFormat, p_list[pi++]);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type GeomVertexFormat is
//               encountered in the Bam file.  It should create the
//               GeomVertexFormat and extract its information
//               from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomVertexFormat::
make_from_bam(const FactoryParams &params) {
  GeomVertexFormat *object = new GeomVertexFormat;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomVertexFormat.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  _animation.fillin(scan, manager);

  int num_arrays = scan.get_uint16();
  _arrays.reserve(num_arrays);
  for (int i = 0; i < num_arrays; i++) {
    manager->read_pointer(scan);
    _arrays.push_back(NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::Registry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexFormat::Registry::
Registry() {
  _v3 = register_format(new GeomVertexArrayFormat
                        (InternalName::get_vertex(), 3, 
                         NT_float32, C_point));

  _v3n3 = register_format(new GeomVertexArrayFormat
                          (InternalName::get_vertex(), 3, 
                           NT_float32, C_point,
                           InternalName::get_normal(), 3, 
                           NT_float32, C_vector));

  _v3t2 = register_format(new GeomVertexArrayFormat
                          (InternalName::get_vertex(), 3, 
                           NT_float32, C_point,
                           InternalName::get_texcoord(), 2, 
                           NT_float32, C_texcoord));

  _v3n3t2 = register_format(new GeomVertexArrayFormat
                            (InternalName::get_vertex(), 3, 
                             NT_float32, C_point,
                             InternalName::get_normal(), 3,
                             NT_float32, C_vector,
                             InternalName::get_texcoord(), 2, 
                             NT_float32, C_texcoord));

  // Define the DirectX-style packed color formats
  _v3cp = register_format(new GeomVertexArrayFormat
                          (InternalName::get_vertex(), 3,
                           NT_float32, C_point,
                           InternalName::get_color(), 1,
                           NT_packed_dabc, C_color));

  _v3n3cp = register_format(new GeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_float32, C_point,
                             InternalName::get_normal(), 3,
                             NT_float32, C_point,
                             InternalName::get_color(), 1,
                             NT_packed_dabc, C_color));

  _v3cpt2 = register_format(new GeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_float32, C_point,
                             InternalName::get_color(), 1,
                             NT_packed_dabc, C_color,
                             InternalName::get_texcoord(), 2,
                             NT_float32, C_texcoord));

  _v3n3cpt2 = register_format(new GeomVertexArrayFormat
                              (InternalName::get_vertex(), 3,
                               NT_float32, C_point,
                               InternalName::get_normal(), 3,
                               NT_float32, C_point,
                               InternalName::get_color(), 1,
                               NT_packed_dabc, C_color,
                               InternalName::get_texcoord(), 2,
                               NT_float32, C_texcoord));

  // Define the OpenGL-style per-byte color formats.  This is not the
  // same as a packed format, above, because the resulting byte order
  // is endian-independent.
  _v3c4 = register_format(new GeomVertexArrayFormat
                          (InternalName::get_vertex(), 3,
                           NT_float32, C_point,
                           InternalName::get_color(), 4,
                           NT_uint8, C_color));

  _v3n3c4 = register_format(new GeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_float32, C_point,
                             InternalName::get_normal(), 3,
                             NT_float32, C_point,
                             InternalName::get_color(), 4,
                             NT_uint8, C_color));

  _v3c4t2 = register_format(new GeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_float32, C_point,
                             InternalName::get_color(), 4,
                             NT_uint8, C_color,
                             InternalName::get_texcoord(), 2,
                             NT_float32, C_texcoord));

  _v3n3c4t2 = register_format(new GeomVertexArrayFormat
                              (InternalName::get_vertex(), 3,
                               NT_float32, C_point,
                               InternalName::get_normal(), 3,
                               NT_float32, C_point,
                               InternalName::get_color(), 4,
                               NT_uint8, C_color,
                               InternalName::get_texcoord(), 2,
                               NT_float32, C_texcoord));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::Registry::register_format
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
CPT(GeomVertexFormat) GeomVertexFormat::Registry::
register_format(GeomVertexFormat *format) {
  if (format->is_registered()) {
    return format;
  }

  // Save the incoming pointer in a local PointerTo, so that if it has
  // a zero reference count and is not added into the map below, it
  // will be automatically deleted when this function returns.
  PT(GeomVertexFormat) pt_format = format;

  Formats::iterator fi = _formats.insert(format).first;

  GeomVertexFormat *new_format = (*fi);
  if (!new_format->is_registered()) {
    new_format->do_register();
  }

  return new_format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::Registry::unregister_format
//       Access: Public
//  Description: Removes the indicated format from the registry.
//               Normally this should not be done until the format is
//               destructing.
////////////////////////////////////////////////////////////////////
void GeomVertexFormat::Registry::
unregister_format(GeomVertexFormat *format) {
  nassertv(format->is_registered());
  Formats::iterator fi = _formats.find(format);
  nassertv(fi != _formats.end());
  _formats.erase(fi);
  format->do_unregister();
}
