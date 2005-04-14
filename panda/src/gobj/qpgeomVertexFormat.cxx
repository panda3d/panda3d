// Filename: qpgeomVertexFormat.cxx
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

#include "qpgeomVertexFormat.h"
#include "qpgeomVertexData.h"
#include "qpgeomMunger.h"
#include "mutexHolder.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"

qpGeomVertexFormat::Registry *qpGeomVertexFormat::_registry = NULL;
TypeHandle qpGeomVertexFormat::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexFormat::
qpGeomVertexFormat() :
  _is_registered(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexFormat::
qpGeomVertexFormat(const qpGeomVertexArrayFormat *array_format) :
  _is_registered(false)
{
  add_array(array_format);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexFormat::
qpGeomVertexFormat(const qpGeomVertexFormat &copy) :
  _is_registered(false),
  _animation(copy._animation),
  _arrays(copy._arrays)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
operator = (const qpGeomVertexFormat &copy) {
  nassertv(!_is_registered);

  _animation = copy._animation;
  _arrays = copy._arrays;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexFormat::
~qpGeomVertexFormat() {
  if (is_registered()) {
    get_registry()->unregister_format(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::modify_array
//       Access: Published
//  Description: Returns a modifiable pointer to the indicated array.
//               This means duplicating it if it is shared or
//               registered.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayFormat *qpGeomVertexFormat::
modify_array(int array) {
  nassertr(!_is_registered, NULL);
  nassertr(array >= 0 && array < (int)_arrays.size(), NULL);

  if (_arrays[array]->is_registered() ||
      _arrays[array]->get_ref_count() > 1) {
    _arrays[array] = new qpGeomVertexArrayFormat(*_arrays[array]);
  }

  return _arrays[array];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::set_array
//       Access: Published
//  Description: Replaces the definition of the indicated array.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
set_array(int array, const qpGeomVertexArrayFormat *format) {
  nassertv(!_is_registered);
  nassertv(array >= 0 && array < (int)_arrays.size());
  _arrays[array] = (qpGeomVertexArrayFormat *)format;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::remove_array
//       Access: Published
//  Description: Removes the nth array from the format.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
remove_array(int array) {
  nassertv(!_is_registered);

  nassertv(array >= 0 && array < (int)_arrays.size());
  _arrays.erase(_arrays.begin() + array);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::add_array
//       Access: Published
//  Description: Adds the indicated array definition to the list of
//               arrays included within this vertex format definition.
//               The return value is the index number of the new
//               array.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
int qpGeomVertexFormat::
add_array(const qpGeomVertexArrayFormat *array_format) {
  nassertr(!_is_registered, -1);

  int new_array = (int)_arrays.size();
  _arrays.push_back((qpGeomVertexArrayFormat *)array_format);
  return new_array;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::insert_array
//       Access: Published
//  Description: Adds the indicated array definition to the list of
//               arrays at the indicated position.  This works just
//               like add_array(), except that you can specify which
//               array index the new array should have.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
insert_array(int array, const qpGeomVertexArrayFormat *array_format) {
  nassertv(!_is_registered);
  nassertv(array >= 0 && array <= (int)_arrays.size());

  _arrays.insert(_arrays.begin() + array, (qpGeomVertexArrayFormat *)array_format);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::clear_arrays
//       Access: Published
//  Description: Removes all of the array definitions from the format
//               and starts over.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
clear_arrays() {
  nassertv(!_is_registered);

  _arrays.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_num_columns
//       Access: Published
//  Description: Returns the total number of different columns in
//               the specification, across all arrays.
////////////////////////////////////////////////////////////////////
int qpGeomVertexFormat::
get_num_columns() const {
  int num_columns = 0;
  Arrays::const_iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    num_columns += (*ai)->get_num_columns();
  }
  return num_columns;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_column
//       Access: Published
//  Description: Returns the ith column of the specification,
//               across all arrays.
////////////////////////////////////////////////////////////////////
const qpGeomVertexColumn *qpGeomVertexFormat::
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
//     Function: qpGeomVertexFormat::get_array_with
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
int qpGeomVertexFormat::
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
//     Function: qpGeomVertexFormat::get_array_with
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
int qpGeomVertexFormat::
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
//     Function: qpGeomVertexFormat::get_column
//       Access: Published
//  Description: Returns the specification with the indicated name, or
//               NULL if the name is not used.  Use get_array_with()
//               to determine which array this column is associated
//               with.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
const qpGeomVertexColumn *qpGeomVertexFormat::
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
//     Function: qpGeomVertexFormat::remove_column
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
void qpGeomVertexFormat::
remove_column(const InternalName *name) {
  nassertv(!_is_registered);

  // Since the format's not registered, it doesn't yet have an index
  // of columns--so we have to search all of the arrays, one at a
  // time, until we find it.
  for (int array = 0; array < (int)_arrays.size(); ++array) {
    qpGeomVertexArrayFormat *array_format = _arrays[array];

    if (array_format->get_column(name) != (qpGeomVertexColumn *)NULL) {
      // Here's the array with the named column!
      if (array_format->is_registered() ||
          array_format->get_ref_count() > 1) {
        // Get a safe-to-modify copy of the array format.
        _arrays[array] = new qpGeomVertexArrayFormat(*array_format);
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
//     Function: qpGeomVertexFormat::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
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
//     Function: qpGeomVertexFormat::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
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
//     Function: qpGeomVertexFormat::write_with_data
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
write_with_data(ostream &out, int indent_level, 
                const qpGeomVertexData *data) const {
  indent(out, indent_level)
    << data->get_num_vertices() << " vertices.\n";
  for (size_t i = 0; i < _arrays.size(); i++) {
    CPTA_uchar array_data = data->get_array(i)->get_data();
    indent(out, indent_level)
      << "Array " << i << " (" << (void *)array_data.p() << "):\n";
    _arrays[i]->write_with_data(out, indent_level + 2, data, i);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_array_info
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
bool qpGeomVertexFormat::
get_array_info(const InternalName *name, int &array_index,
               const qpGeomVertexColumn *&column) const {
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
//     Function: qpGeomVertexFormat::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int qpGeomVertexFormat::
compare_to(const qpGeomVertexFormat &other) const {
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
//     Function: qpGeomVertexFormat::make_registry
//       Access: Private
//  Description: Returns the global registry object.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
make_registry() {
  if (_registry == (Registry *)NULL) {
    _registry = new Registry;
  }
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::do_register
//       Access: Private
//  Description: Called internally when the format is registered.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
do_register() {
  nassertv(!_is_registered);
  nassertv(_columns_by_name.empty());

  for (int array = 0; array < (int)_arrays.size(); ++array) {
    CPT(qpGeomVertexArrayFormat) array_format = _arrays[array];
    if (!array_format->is_registered()) {
      array_format = qpGeomVertexArrayFormat::register_format(array_format);
      _arrays[array] = (qpGeomVertexArrayFormat *)array_format.p();
    }

    // Now add the names to the index.
    int num_columns = array_format->get_num_columns();
    for (int i = 0; i < num_columns; i++) {
      const qpGeomVertexColumn *column = array_format->get_column(i);
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
    const qpGeomVertexColumn *column = _arrays[record._array_index]->get_column(record._column_index);

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
//     Function: qpGeomVertexFormat::do_unregister
//       Access: Private
//  Description: Called internally when the format is unregistered.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
do_unregister() {
  nassertv(_is_registered);
  _is_registered = false;

  _columns_by_name.clear();
  _points.clear();
  _vectors.clear();
  _texcoords.clear();
  _morphs.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeomVertexFormat.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
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
//     Function: qpGeomVertexFormat::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int qpGeomVertexFormat::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  Arrays::iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    (*ai) = DCAST(qpGeomVertexArrayFormat, p_list[pi++]);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeomVertexFormat is
//               encountered in the Bam file.  It should create the
//               qpGeomVertexFormat and extract its information
//               from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomVertexFormat::
make_from_bam(const FactoryParams &params) {
  qpGeomVertexFormat *object = new qpGeomVertexFormat;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomVertexFormat.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::
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
//     Function: qpGeomVertexFormat::Registry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexFormat::Registry::
Registry() {
  _v3 = register_format(new qpGeomVertexArrayFormat
                        (InternalName::get_vertex(), 3, 
                         NT_float32, C_point));

  _v3n3 = register_format(new qpGeomVertexArrayFormat
                          (InternalName::get_vertex(), 3, 
                           NT_float32, C_point,
                           InternalName::get_normal(), 3, 
                           NT_float32, C_vector));

  _v3t2 = register_format(new qpGeomVertexArrayFormat
                          (InternalName::get_vertex(), 3, 
                           NT_float32, C_point,
                           InternalName::get_texcoord(), 2, 
                           NT_float32, C_texcoord));

  _v3n3t2 = register_format(new qpGeomVertexArrayFormat
                            (InternalName::get_vertex(), 3, 
                             NT_float32, C_point,
                             InternalName::get_normal(), 3,
                             NT_float32, C_vector,
                             InternalName::get_texcoord(), 2, 
                             NT_float32, C_texcoord));

  // Define the DirectX-style packed color formats
  _v3cp = register_format(new qpGeomVertexArrayFormat
                          (InternalName::get_vertex(), 3,
                           NT_float32, C_point,
                           InternalName::get_color(), 1,
                           NT_packed_dabc, C_color));

  _v3n3cp = register_format(new qpGeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_float32, C_point,
                             InternalName::get_normal(), 3,
                             NT_float32, C_point,
                             InternalName::get_color(), 1,
                             NT_packed_dabc, C_color));

  _v3cpt2 = register_format(new qpGeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_float32, C_point,
                             InternalName::get_color(), 1,
                             NT_packed_dabc, C_color,
                             InternalName::get_texcoord(), 2,
                             NT_float32, C_texcoord));

  _v3n3cpt2 = register_format(new qpGeomVertexArrayFormat
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
  _v3c4 = register_format(new qpGeomVertexArrayFormat
                          (InternalName::get_vertex(), 3,
                           NT_float32, C_point,
                           InternalName::get_color(), 4,
                           NT_uint8, C_color));

  _v3n3c4 = register_format(new qpGeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_float32, C_point,
                             InternalName::get_normal(), 3,
                             NT_float32, C_point,
                             InternalName::get_color(), 4,
                             NT_uint8, C_color));

  _v3c4t2 = register_format(new qpGeomVertexArrayFormat
                            (InternalName::get_vertex(), 3,
                             NT_float32, C_point,
                             InternalName::get_color(), 4,
                             NT_uint8, C_color,
                             InternalName::get_texcoord(), 2,
                             NT_float32, C_texcoord));

  _v3n3c4t2 = register_format(new qpGeomVertexArrayFormat
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
//     Function: qpGeomVertexFormat::Registry::register_format
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
CPT(qpGeomVertexFormat) qpGeomVertexFormat::Registry::
register_format(qpGeomVertexFormat *format) {
  if (format->is_registered()) {
    return format;
  }

  // Save the incoming pointer in a local PointerTo, so that if it has
  // a zero reference count and is not added into the map below, it
  // will be automatically deleted when this function returns.
  PT(qpGeomVertexFormat) pt_format = format;

  Formats::iterator fi = _formats.insert(format).first;

  qpGeomVertexFormat *new_format = (*fi);
  if (!new_format->is_registered()) {
    new_format->do_register();
  }

  return new_format;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::Registry::unregister_format
//       Access: Public
//  Description: Removes the indicated format from the registry.
//               Normally this should not be done until the format is
//               destructing.
////////////////////////////////////////////////////////////////////
void qpGeomVertexFormat::Registry::
unregister_format(qpGeomVertexFormat *format) {
  nassertv(format->is_registered());
  Formats::iterator fi = _formats.find(format);
  nassertv(fi != _formats.end());
  _formats.erase(fi);
  format->do_unregister();
}
