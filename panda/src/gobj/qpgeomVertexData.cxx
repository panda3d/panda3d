// Filename: qpgeomVertexData.cxx
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

#include "qpgeomVertexData.h"
#include "qpgeomVertexReader.h"
#include "qpgeomVertexWriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pset.h"
#include "indent.h"

TypeHandle qpGeomVertexData::_type_handle;

PStatCollector qpGeomVertexData::_convert_pcollector("Cull:Munge:Convert");
PStatCollector qpGeomVertexData::_scale_color_pcollector("Cull:Munge:Scale color");
PStatCollector qpGeomVertexData::_set_color_pcollector("Cull:Munge:Set color");
PStatCollector qpGeomVertexData::_animate_vertices_pcollector("Cull:Animate vertices");

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::Default Constructor
//       Access: Private
//  Description: Constructs an invalid object.  This is only used when
//               reading from the bam file.
////////////////////////////////////////////////////////////////////
qpGeomVertexData::
qpGeomVertexData() :
  _this_animate_vertices_pcollector(_animate_vertices_pcollector)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexData::
qpGeomVertexData(const string &name,
                 const qpGeomVertexFormat *format,
                 qpGeomUsageHint::UsageHint usage_hint) :
  _name(name),
  _format(format),
  _usage_hint(usage_hint),
  _this_animate_vertices_pcollector(_animate_vertices_pcollector, name)
{
  nassertv(_format->is_registered());

  // Create some empty arrays as required by the format.
  CDWriter cdata(_cycler);

  int num_arrays = _format->get_num_arrays();
  for (int i = 0; i < num_arrays; i++) {
    PT(qpGeomVertexArrayData) array = new qpGeomVertexArrayData
      (_format->get_array(i), usage_hint);
    cdata->_arrays.push_back(array);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexData::
qpGeomVertexData(const qpGeomVertexData &copy) :
  TypedWritableReferenceCount(copy),
  _name(copy._name),
  _format(copy._format),
  _cycler(copy._cycler),
  _this_animate_vertices_pcollector(copy._this_animate_vertices_pcollector)
{
}
  
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
operator = (const qpGeomVertexData &copy) {
  TypedWritableReferenceCount::operator = (copy);
  _name = copy._name;
  _format = copy._format;
  _cycler = copy._cycler;
  _this_animate_vertices_pcollector = copy._this_animate_vertices_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexData::
~qpGeomVertexData() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_num_vertices
//       Access: Published
//  Description: Returns the number of vertices stored within all the
//               arrays.  All arrays store data for the same n
//               vertices.
////////////////////////////////////////////////////////////////////
int qpGeomVertexData::
get_num_vertices() const {
  CDReader cdata(_cycler);
  nassertr(_format->get_num_arrays() == (int)cdata->_arrays.size(), 0);
  if (_format->get_num_arrays() == 0) {
    // No arrays means no vertices.  Weird but legal.
    return 0;
  }

  // Look up the answer on the first array (since any array will do).
  int stride = _format->get_array(0)->get_stride();
  return cdata->_arrays[0]->get_data_size_bytes() / stride;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::clear_vertices
//       Access: Published
//  Description: Removes all of the vertices from the arrays;
//               functionally equivalent to set_num_vertices(0) (but
//               faster).
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
clear_vertices() {
  CDWriter cdata(_cycler);
  nassertv(_format->get_num_arrays() == (int)cdata->_arrays.size());

  Arrays::iterator ai;
  for (ai = cdata->_arrays.begin();
       ai != cdata->_arrays.end();
       ++ai) {
    (*ai)->clear_vertices();
  }
  cdata->_modified = qpGeom::get_next_modified();
  cdata->_animated_vertices.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::modify_array
//       Access: Published
//  Description: Returns a modifiable pointer to the indicated vertex
//               array, so that application code may directly
//               manipulate the vertices.  You should avoid changing
//               the length of this array, since all of the arrays
//               should be kept in sync--use add_vertices() instead.
////////////////////////////////////////////////////////////////////
qpGeomVertexArrayData *qpGeomVertexData::
modify_array(int i) {
  // Perform copy-on-write: if the reference count on the vertex data
  // is greater than 1, assume some other GeomVertexData has the same
  // pointer, so make a copy of it first.
  CDWriter cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_arrays.size(), NULL);

  if (cdata->_arrays[i]->get_ref_count() > 1) {
    cdata->_arrays[i] = new qpGeomVertexArrayData(*cdata->_arrays[i]);
  }
  cdata->_modified = qpGeom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();

  return cdata->_arrays[i];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::set_array
//       Access: Published
//  Description: Replaces the indicated vertex data array with
//               a completely new array.  You should be careful that
//               the new array has the same length as the old one,
//               unless you know what you are doing.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
set_array(int i, const qpGeomVertexArrayData *array) {
  CDWriter cdata(_cycler);
  nassertv(i >= 0 && i < (int)cdata->_arrays.size());
  cdata->_arrays[i] = (qpGeomVertexArrayData *)array;
  cdata->_modified = qpGeom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::modify_transform_blend_palette
//       Access: Published
//  Description: Returns a modifiable pointer to the current
//               TransformBlendPalette on this vertex data, if any, or
//               NULL if there is not a TransformBlendPalette.  See
//               get_transform_blend_palette().
////////////////////////////////////////////////////////////////////
TransformBlendPalette *qpGeomVertexData::
modify_transform_blend_palette() {
  // Perform copy-on-write: if the reference count on the palette is
  // greater than 1, assume some other GeomVertexData has the same
  // pointer, so make a copy of it first.
  CDWriter cdata(_cycler);

  if (cdata->_transform_blend_palette->get_ref_count() > 1) {
    cdata->_transform_blend_palette = new TransformBlendPalette(*cdata->_transform_blend_palette);
  }
  cdata->_modified = qpGeom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();

  return cdata->_transform_blend_palette;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::set_transform_blend_palette
//       Access: Published
//  Description: Replaces the TransformBlendPalette on this vertex
//               data with the indicated palette.  The length of this
//               palette should be consistent with the maximum palette
//               index assigned to the vertices under the
//               "transform_blend" name.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
set_transform_blend_palette(const TransformBlendPalette *palette) {
  CDWriter cdata(_cycler);
  cdata->_transform_blend_palette = (TransformBlendPalette *)palette;
  cdata->_modified = qpGeom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_num_bytes
//       Access: Published
//  Description: Returns the total number of bytes consumed by the
//               different arrays of the vertex data.
////////////////////////////////////////////////////////////////////
int qpGeomVertexData::
get_num_bytes() const {
  CDReader cdata(_cycler);
  
  int num_bytes = sizeof(qpGeomVertexData);

  Arrays::const_iterator ai;
  for (ai = cdata->_arrays.begin(); ai != cdata->_arrays.end(); ++ai) {
    num_bytes += (*ai)->get_data_size_bytes();
  }

  return num_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::convert_to
//       Access: Published
//  Description: Matches up the data types of this format with the
//               data types of the other format by name, and copies
//               the data vertex-by-vertex to a new set of data arrays
//               in the new format.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexData) qpGeomVertexData::
convert_to(const qpGeomVertexFormat *new_format) const {
  if (new_format == _format) {
    // Trivial case: no change is needed.
    return this;
  }

  // Okay, convert the data to the new format.
  int num_vertices = get_num_vertices();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Converting " << num_vertices << " vertices.\n";
  }
  PStatTimer timer(_convert_pcollector);

  PT(qpGeomVertexData) new_data = 
    new qpGeomVertexData(get_name(), new_format, get_usage_hint());
  new_data->set_transform_blend_palette(get_transform_blend_palette());

  pset<int> done_arrays;

  int num_arrays = _format->get_num_arrays();
  int i;

  // First, check to see if any arrays can be simply appropriated for
  // the new format, without changing the data.
  for (i = 0; i < num_arrays; ++i) {
    const qpGeomVertexArrayFormat *array_format = 
      _format->get_array(i);

    bool array_done = false;

    int new_num_arrays = new_format->get_num_arrays();
    for (int new_i = 0; 
         new_i < new_num_arrays && !array_done; 
         ++new_i) {
      const qpGeomVertexArrayFormat *new_array_format = 
        new_format->get_array(new_i);
      if (new_array_format->is_data_subset_of(*array_format)) {
        // Great!  Just use the same data for this one.
        new_data->set_array(new_i, get_array(i));
        array_done = true;

        done_arrays.insert(new_i);
      }
    }
  }

  // Now make sure the arrays we didn't share are all filled in.
  new_data->set_num_vertices(num_vertices);

  // Now go back through and copy any data that's left over.
  for (i = 0; i < num_arrays; ++i) {
    CPTA_uchar array_data = get_array(i)->get_data();
    const qpGeomVertexArrayFormat *array_format = _format->get_array(i);
    int num_data_types = array_format->get_num_data_types();
    for (int di = 0; di < num_data_types; ++di) {
      const qpGeomVertexDataType *data_type = array_format->get_data_type(di);

      int new_i = new_format->get_array_with(data_type->get_name());
      if (new_i >= 0 && done_arrays.count(new_i) == 0) {
        // The data type exists in the new format; we have to copy it.
        const qpGeomVertexArrayFormat *new_array_format = 
          new_format->get_array(new_i);
        const qpGeomVertexDataType *new_data_type = 
          new_array_format->get_data_type(data_type->get_name());

        if (new_data_type->is_bytewise_equivalent(*data_type)) {
          // We can do a quick bytewise copy.
          PTA_uchar new_array_data = 
            new_data->modify_array(new_i)->modify_data();

          bytewise_copy(new_array_data + new_data_type->get_start(), 
                        new_array_format->get_stride(),
                        array_data + data_type->get_start(), array_format->get_stride(),
                        data_type, num_vertices);

        } else if (new_data_type->is_packed_argb() && 
                   data_type->is_uint8_rgba()) {
          // A common special case: OpenGL color to DirectX color.
          PTA_uchar new_array_data = 
            new_data->modify_array(new_i)->modify_data();

          uint8_rgba_to_packed_argb
            (new_array_data + new_data_type->get_start(), 
             new_array_format->get_stride(),
             array_data + data_type->get_start(), array_format->get_stride(),
             num_vertices);

        } else if (new_data_type->is_uint8_rgba() && 
                   data_type->is_packed_argb()) {
          // Another common special case: DirectX color to OpenGL
          // color.
          PTA_uchar new_array_data = 
            new_data->modify_array(new_i)->modify_data();

          packed_argb_to_uint8_rgba
            (new_array_data + new_data_type->get_start(), 
             new_array_format->get_stride(),
             array_data + data_type->get_start(), array_format->get_stride(),
             num_vertices);

        } else {
          // A generic copy.
          if (gobj_cat.is_debug()) {
            gobj_cat.debug()
              << "generic copy " << *new_data_type << " from " 
              << *data_type << "\n";
          }
          qpGeomVertexReader from(this);
          from.set_data_type(i, data_type);
          qpGeomVertexWriter to(new_data);
          to.set_data_type(new_i, new_data_type);

          while (!from.is_at_end()) {
            to.set_data4f(from.get_data4f());
          }
        }
      }
    }
  }

  return new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::scale_color
//       Access: Published
//  Description: Returns a new GeomVertexData object with the color
//               table replaced with a new color table that has been
//               scaled by the indicated value.  The new color table
//               will be added as a new array; if the old color table
//               was interleaved with a previous array, the previous
//               array will not be repacked.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexData) qpGeomVertexData::
scale_color(const LVecBase4f &color_scale, int num_components,
            qpGeomVertexDataType::NumericType numeric_type,
            qpGeomVertexDataType::Contents contents) const {
  int old_color_array = _format->get_array_with(InternalName::get_color());
  if (old_color_array == -1) {
    // Oops, no color anyway.
    return this;
  }

  int num_vertices = get_num_vertices();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Scaling color for " << num_vertices << " vertices by "
      << color_scale << ".\n";
  }
  PStatTimer timer(_scale_color_pcollector);

  PT(qpGeomVertexData) new_data = replace_data_type
    (InternalName::get_color(), num_components, numeric_type,
     contents, get_usage_hint(), true);

  // Now go through and apply the scale, copying it to the new data.
  qpGeomVertexReader from(this, InternalName::get_color());
  qpGeomVertexWriter to(new_data, InternalName::get_color());

  for (int i = 0; i < num_vertices; i++) {
    Colorf color = from.get_data4f();
    to.set_data4f(color[0] * color_scale[0],
                  color[1] * color_scale[1],
                  color[2] * color_scale[2],
                  color[3] * color_scale[3]);
  }

  return new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::set_color
//       Access: Published
//  Description: Returns a new GeomVertexData object with the color
//               table replaced with a new color table for which each
//               vertex has the indicated value.  The new color table
//               will be added as a new array; if the old color table
//               was interleaved with a previous array, the previous
//               array will not be repacked.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexData) qpGeomVertexData::
set_color(const Colorf &color, int num_components,
          qpGeomVertexDataType::NumericType numeric_type,
          qpGeomVertexDataType::Contents contents) const {
  int num_vertices = get_num_vertices();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Setting color for " << num_vertices << " vertices to "
      << color << ".\n";
  }
  PStatTimer timer(_set_color_pcollector);

  PT(qpGeomVertexData) new_data = replace_data_type
    (InternalName::get_color(), num_components, numeric_type,
     contents, get_usage_hint(), true);

  // Now go through and set the new color value.
  qpGeomVertexWriter to(new_data, InternalName::get_color());

  for (int i = 0; i < num_vertices; i++) {
    to.set_data4f(color);
  }

  return new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::animate_vertices
//       Access: Published
//  Description: Returns a GeomVertexData that represents the results
//               of computing the vertex animation on the CPU for this
//               GeomVertexData.
//
//               If there is no CPU-defined vertex animation on this
//               object, this just returns the original object.
//
//               If there is vertex animation, but the VertexTransform
//               values have not changed since last time, this may
//               return the same pointer it returned previously.  Even
//               if the VertexTransform values have changed, it may
//               still return the same pointer, but with its contents
//               modified (this is preferred, since it allows the
//               graphics backend to update vertex buffers optimally).
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexData) qpGeomVertexData::
animate_vertices() const {
  CDReader cdata(_cycler);
  if (cdata->_transform_blend_palette == (TransformBlendPalette *)NULL) {
    // No vertex animation.
    return this;
  }

  if (cdata->_animated_vertices == (qpGeomVertexData *)NULL) {
    CDWriter cdataw(((qpGeomVertexData *)this)->_cycler, cdata);
    ((qpGeomVertexData *)this)->make_animated_vertices(cdataw);
    return cdataw->_animated_vertices;
  } else {
    UpdateSeq blend_modified = cdata->_transform_blend_palette->get_modified();
    if (cdata->_animated_vertices_modified == blend_modified) {
      // No changes.
      return cdata->_animated_vertices;
    }
    CDWriter cdataw(((qpGeomVertexData *)this)->_cycler, cdata);
    cdataw->_animated_vertices_modified = blend_modified;
    ((qpGeomVertexData *)this)->update_animated_vertices(cdataw);
    return cdataw->_animated_vertices;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::replace_data_type
//       Access: Published
//  Description: Returns a new GeomVertexData object, suitable for
//               modification, with the indicated data type replaced
//               with a new table filled with undefined values.  The
//               new table will be added as a new array; if the old
//               table was interleaved with a previous array, the
//               previous array will not be repacked.
//
//               If num_components is 0, the indicated name is simply
//               removed from the type, without replacing it with
//               anything else.
////////////////////////////////////////////////////////////////////
PT(qpGeomVertexData) qpGeomVertexData::
replace_data_type(const InternalName *name, int num_components,
                  qpGeomVertexDataType::NumericType numeric_type,
                  qpGeomVertexDataType::Contents contents,
                  qpGeomUsageHint::UsageHint usage_hint,
                  bool keep_animation) const {
  PT(qpGeomVertexFormat) new_format = new qpGeomVertexFormat(*_format);

  // Remove the old description of the type from the format.
  bool removed_type_array = false;
  int old_type_array = _format->get_array_with(name);
  if (old_type_array != -1) {
    qpGeomVertexArrayFormat *array_format = new_format->modify_array(old_type_array);
    if (array_format->get_num_data_types() == 1) {
      // Actually, this array didn't have any other data types, so
      // just drop the whole array.
      new_format->remove_array(old_type_array);
      removed_type_array = true;
      
    } else {
      // Remove the description for the type, but don't bother to
      // repack the array.
      array_format->remove_data_type(name);
    }
  }
    
  // Now define a new array to contain just the type.
  int new_type_array = -1;
  if (num_components != 0) {
    PT(qpGeomVertexArrayFormat) type_array_format = 
      new qpGeomVertexArrayFormat(name, num_components, numeric_type, contents);
    new_type_array = new_format->add_array(type_array_format);
  }

  CPT(qpGeomVertexFormat) format = 
    qpGeomVertexFormat::register_format(new_format);

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Replacing data type " << *name << "; converting "
      << get_num_vertices() << " vertices from " 
      << *_format << " to " << *new_format << "\n";
  }
  
  PT(qpGeomVertexData) new_data = 
    new qpGeomVertexData(get_name(), format, usage_hint);
  if (keep_animation) {
    new_data->set_transform_blend_palette(get_transform_blend_palette());
  }

  int j = 0;
  int num_arrays = get_num_arrays();
  for (int i = 0; i < num_arrays; ++i) {
    if (i == old_type_array) {
      if (!removed_type_array) {
        // Pointer-copy the original array that includes the type
        // (since it also includes other data).
        new_data->set_array(j, get_array(i));
        ++j;
      }

    } else {
      // Just pointer-copy any arrays other than type.
      new_data->set_array(j, get_array(i));
      ++j;
    }
  }

  if (new_type_array != -1) {
    nassertr(j == new_type_array, new_data);

    // For the new type array, we set up a temporary array that has
    // room for the right number of vertices.
    PT(qpGeomVertexArrayData) new_array = new qpGeomVertexArrayData
      (new_format->get_array(j), get_usage_hint());
    new_array->set_num_vertices(get_num_vertices());
    new_data->set_array(j, new_array);
  }

  return new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
output(ostream &out) const {
  out << get_num_vertices() << ": " << *get_format();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
write(ostream &out, int indent_level) const {
  _format->write_with_data(out, indent_level, this);
  if (get_transform_blend_palette() != (TransformBlendPalette *)NULL) {
    indent(out, indent_level)
      << "Transform blend palette:\n";
    get_transform_blend_palette()->write(out, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_array_info
//       Access: Public
//  Description: A convenience function to collect together the
//               important parts of the array data for rendering.
//               Given the name of a data type, fills in the start of
//               the array, the number of floats for each vertex, the
//               starting bytes number, and the number of bytes to
//               increment for each consecutive vertex.
//
//               The return value is true if the named array data
//               exists in this record, or false if it does not (in
//               which case none of the output parameters are valid).
////////////////////////////////////////////////////////////////////
bool qpGeomVertexData::
get_array_info(const InternalName *name, 
               const qpGeomVertexArrayData *&array_data,
               int &num_components, 
               qpGeomVertexDataType::NumericType &numeric_type, 
               int &start, int &stride) const {
  int array_index;
  const qpGeomVertexDataType *data_type;
  if (_format->get_array_info(name, array_index, data_type)) {
    CDReader cdata(_cycler);
    array_data = cdata->_arrays[array_index];
    num_components = data_type->get_num_components();
    numeric_type = data_type->get_numeric_type();
    start = data_type->get_start();
    stride = _format->get_array(array_index)->get_stride();
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::bytewise_copy
//       Access: Private, Static
//  Description: Quickly copies data without the need to convert it.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
bytewise_copy(unsigned char *to, int to_stride,
              const unsigned char *from, int from_stride,
              const qpGeomVertexDataType *from_type,
              int num_records) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "bytewise_copy(" << (void *)to << ", " << to_stride
      << ", " << (const void *)from << ", " << from_stride
      << ", " << *from_type << ", " << num_records << ")\n";
  }
  if (to_stride == from_type->get_total_bytes() && 
      from_stride == from_type->get_total_bytes()) {
    // Fantastic!  It's just a linear array of this one data type.
    // Copy the whole thing all at once.
    memcpy(to, from, num_records * from_type->get_total_bytes());

  } else {
    // Ok, it's interleaved in with other data.  Copy them one record
    // at a time.
    while (num_records > 0) {
      memcpy(to, from, from_type->get_total_bytes());
      to += to_stride;
      from += from_stride;
      num_records--;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::packed_argb_to_uint8_rgba
//       Access: Private, Static
//  Description: Quickly converts DirectX-style color to OpenGL-style
//               color.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
packed_argb_to_uint8_rgba(unsigned char *to, int to_stride,
                          const unsigned char *from, int from_stride,
                          int num_records) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "packed_argb_to_uint8_rgba(" << (void *)to << ", " << to_stride
      << ", " << (const void *)from << ", " << from_stride
      << ", " << num_records << ")\n";
  }
  typedef union {
    unsigned char _b[4];
    PN_uint32 _i;
  } packed_8888;

  while (num_records > 0) {
    packed_8888 dword;
    dword._i = *(const PN_uint32 *)from;
    to[0] = dword._b[1];
    to[1] = dword._b[2];
    to[2] = dword._b[3];
    to[3] = dword._b[0];

    to += to_stride;
    from += from_stride;
    num_records--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::uint8_rgba_to_packed_argb
//       Access: Private, Static
//  Description: Quickly converts OpenGL-style color to DirectX-style
//               color.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
uint8_rgba_to_packed_argb(unsigned char *to, int to_stride,
                          const unsigned char *from, int from_stride,
                          int num_records) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "uint8_rgba_to_packed_argb(" << (void *)to << ", " << to_stride
      << ", " << (const void *)from << ", " << from_stride
      << ", " << num_records << ")\n";
  }
  typedef union {
    unsigned char _b[4];
    PN_uint32 _i;
  } packed_8888;

  while (num_records > 0) {
    packed_8888 dword;
    dword._b[0] = from[3];
    dword._b[1] = from[0];
    dword._b[2] = from[1];
    dword._b[3] = from[2];
    *(PN_uint32 *)to = dword._i;

    to += to_stride;
    from += from_stride;
    num_records--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::do_set_num_vertices
//       Access: Private
//  Description: The private implementation of set_num_vertices().
////////////////////////////////////////////////////////////////////
bool qpGeomVertexData::
do_set_num_vertices(int n, qpGeomVertexData::CDWriter &cdata) {
  nassertr(_format->get_num_arrays() == (int)cdata->_arrays.size(), false);

  bool any_changed = false;

  for (size_t i = 0; i < cdata->_arrays.size(); i++) {
    if (cdata->_arrays[i]->get_num_vertices() != n) {
      // Copy-on-write.
      if (cdata->_arrays[i]->get_ref_count() > 1) {
        cdata->_arrays[i] = new qpGeomVertexArrayData(*cdata->_arrays[i]);
      }
      cdata->_arrays[i]->set_num_vertices(n);
      any_changed = true;
    }
  }

  if (any_changed) {
    cdata->_modified = qpGeom::get_next_modified();
    cdata->_animated_vertices.clear();
  }

  return any_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::make_animated_vertices
//       Access: Private
//  Description: Creates the GeomVertexData that represents the
//               results of computing the vertex animation on the CPU.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
make_animated_vertices(qpGeomVertexData::CDWriter &cdata) {
  // First, make a new format that doesn't have the transform_blend
  // array.
  cdata->_animated_vertices = replace_data_type
    (InternalName::get_transform_blend(), 0, qpGeomVertexDataType::NT_uint16,
     qpGeomVertexDataType::C_index,
     min(get_usage_hint(), qpGeomUsageHint::UH_dynamic), false);

  // Now fill it up with the appropriate data.
  update_animated_vertices(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::update_animated_vertices
//       Access: Private
//  Description: Recomputes the results of computing the vertex
//               animation on the CPU, and applies them to the
//               existing animated_vertices object.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
update_animated_vertices(qpGeomVertexData::CDWriter &cdata) {
  int num_vertices = get_num_vertices();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Animating " << num_vertices << " vertices for " << get_name()
      << "\n";
  }

  PStatTimer timer(_this_animate_vertices_pcollector);

  CPT(TransformBlendPalette) palette = cdata->_transform_blend_palette;
  nassertv(palette != (TransformBlendPalette *)NULL);

  // Recompute all the blends up front.
  int num_blends = palette->get_num_blends();
  int bi;
  for (bi = 0; bi < num_blends; bi++) {
    palette->get_blend(bi).update_blend();
  }

  PT(qpGeomVertexData) new_data = cdata->_animated_vertices;

  // Now go through and apply the scale, copying it to the new data.
  qpGeomVertexReader from(this, InternalName::get_vertex());
  qpGeomVertexReader blendi(this, InternalName::get_transform_blend());
  qpGeomVertexWriter to(new_data, InternalName::get_vertex());

  if (from.get_data_type()->get_num_values() == 4) {
    for (int i = 0; i < num_vertices; i++) {
      LPoint4f vertex = from.get_data4f();
      int bi = blendi.get_data1i();
      palette->get_blend(bi).transform_point(vertex);
      to.set_data4f(vertex);
    }
  } else {
    for (int i = 0; i < num_vertices; i++) {
      LPoint3f vertex = from.get_data3f();
      int bi = blendi.get_data1i();
      palette->get_blend(bi).transform_point(vertex);
      to.set_data3f(vertex);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeomVertexData.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeomVertexData is encountered
//               in the Bam file.  It should create the qpGeomVertexData
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomVertexData::
make_from_bam(const FactoryParams &params) {
  qpGeomVertexData *object = new qpGeomVertexData;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomVertexData.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *qpGeomVertexData::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int qpGeomVertexData::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomVertexData.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
}
