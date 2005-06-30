// Filename: geomVertexData.cxx
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

#include "geomVertexData.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "geomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pset.h"
#include "indent.h"

TypeHandle GeomVertexData::_type_handle;

PStatCollector GeomVertexData::_convert_pcollector("*:Munge:Convert");
PStatCollector GeomVertexData::_scale_color_pcollector("*:Munge:Scale color");
PStatCollector GeomVertexData::_set_color_pcollector("*:Munge:Set color");
PStatCollector GeomVertexData::_animation_pcollector("*:Animation");

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::Default Constructor
//       Access: Private
//  Description: Constructs an invalid object.  This is only used when
//               reading from the bam file.
////////////////////////////////////////////////////////////////////
GeomVertexData::
GeomVertexData() :
  _char_pcollector(_animation_pcollector, "unnamed"),
  _skinning_pcollector(_char_pcollector, "Skinning"),
  _morphs_pcollector(_char_pcollector, "Morphs")
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexData::
GeomVertexData(const string &name,
               const GeomVertexFormat *format,
               GeomVertexData::UsageHint usage_hint) :
  _name(name),
  _format(format),
  _char_pcollector(PStatCollector(_animation_pcollector, name)),
  _skinning_pcollector(_char_pcollector, "Skinning"),
  _morphs_pcollector(_char_pcollector, "Morphs")
{
  nassertv(_format->is_registered());

  set_usage_hint(usage_hint);

  // Create some empty arrays as required by the format.
  CDWriter cdata(_cycler);

  int num_arrays = _format->get_num_arrays();
  for (int i = 0; i < num_arrays; i++) {
    PT(GeomVertexArrayData) array = new GeomVertexArrayData
      (_format->get_array(i), usage_hint);
    cdata->_arrays.push_back(array);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexData::
GeomVertexData(const GeomVertexData &copy) :
  TypedWritableReferenceCount(copy),
  _name(copy._name),
  _format(copy._format),
  _cycler(copy._cycler),
  _char_pcollector(copy._char_pcollector),
  _skinning_pcollector(copy._skinning_pcollector),
  _morphs_pcollector(copy._morphs_pcollector)
{
  CDWriter cdata(_cycler);
  // It's important that we *not* copy the animated_vertices pointer.
  cdata->_animated_vertices = NULL;
  cdata->_animated_vertices_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::Constructor
//       Access: Published
//  Description: This constructor copies all of the basic properties
//               of the source VertexData, like usage_hint and
//               animation tables, but does not copy the actual data,
//               and it allows you to specify a different format.
////////////////////////////////////////////////////////////////////
GeomVertexData::
GeomVertexData(const GeomVertexData &copy, 
               const GeomVertexFormat *format) :
  TypedWritableReferenceCount(copy),
  _name(copy._name),
  _format(format),
  _cycler(copy._cycler),
  _char_pcollector(copy._char_pcollector),
  _skinning_pcollector(copy._skinning_pcollector),
  _morphs_pcollector(copy._morphs_pcollector)
{
  nassertv(_format->is_registered());

  // Create some empty arrays as required by the format.
  CDWriter cdata(_cycler);

  UsageHint usage_hint = cdata->_usage_hint;
  cdata->_arrays.clear();
  int num_arrays = _format->get_num_arrays();
  for (int i = 0; i < num_arrays; i++) {
    PT(GeomVertexArrayData) array = new GeomVertexArrayData
      (_format->get_array(i), usage_hint);
    cdata->_arrays.push_back(array);
  }

  // It's important that we *not* copy the animated_vertices pointer.
  cdata->_animated_vertices = NULL;
  cdata->_animated_vertices_modified = UpdateSeq();
}
  
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexData::
operator = (const GeomVertexData &copy) {
  TypedWritableReferenceCount::operator = (copy);
  _name = copy._name;
  _format = copy._format;
  _cycler = copy._cycler;
  _char_pcollector = copy._char_pcollector;
  _skinning_pcollector = copy._skinning_pcollector;
  _morphs_pcollector = copy._morphs_pcollector;

  CDWriter cdata(_cycler);
  clear_cache();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomVertexData::
~GeomVertexData() {
  // When we destruct, we should ensure that all of our cached
  // entries, across all pipeline stages, are properly removed from
  // the cache manager.
  int num_stages = _cycler.get_num_stages();
  for (int i = 0; i < num_stages; i++) {
    if (_cycler.is_stage_unique(i)) {
      CData *cdata = _cycler.write_stage(i);
      for (Cache::iterator ci = cdata->_cache.begin();
           ci != cdata->_cache.end();
           ++ci) {
        CacheEntry *entry = (*ci);
        entry->erase();
      }
      cdata->_cache.clear();
      _cycler.release_write_stage(i, cdata);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::set_name
//       Access: Published
//  Description: Changes the name of the vertex data.  This name is
//               reported on the PStats graph for vertex computations.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
set_name(const string &name) {
  _name = name;
  _char_pcollector = PStatCollector(_animation_pcollector, name);
  _skinning_pcollector = PStatCollector(_char_pcollector, "Skinning");
  _morphs_pcollector = PStatCollector(_char_pcollector, "Morphs");
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::set_usage_hint
//       Access: Published
//  Description: Changes the UsageHint hint for this vertex data, and
//               for all of the arrays that share this data.  See
//               get_usage_hint().
////////////////////////////////////////////////////////////////////
void GeomVertexData::
set_usage_hint(GeomVertexData::UsageHint usage_hint) {
  CDWriter cdata(_cycler);
  cdata->_usage_hint = usage_hint;

  Arrays::iterator ai;
  for (ai = cdata->_arrays.begin();
       ai != cdata->_arrays.end();
       ++ai) {
    if ((*ai)->get_ref_count() > 1) {
      (*ai) = new GeomVertexArrayData(*(*ai));
    }
    (*ai)->set_usage_hint(usage_hint);
  }
  clear_cache();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_num_rows
//       Access: Published
//  Description: Returns the number of rows stored within all the
//               arrays.  All arrays store data for the same n
//               rows.
////////////////////////////////////////////////////////////////////
int GeomVertexData::
get_num_rows() const {
  CDReader cdata(_cycler);
  nassertr(_format->get_num_arrays() == (int)cdata->_arrays.size(), 0);
  if (_format->get_num_arrays() == 0) {
    // No arrays means no rows.  Weird but legal.
    return 0;
  }

  // Look up the answer on the first array (since any array will do).
  int stride = _format->get_array(0)->get_stride();
  return cdata->_arrays[0]->get_data_size_bytes() / stride;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::clear_rows
//       Access: Published
//  Description: Removes all of the rows from the arrays;
//               functionally equivalent to set_num_rows(0) (but
//               faster).
////////////////////////////////////////////////////////////////////
void GeomVertexData::
clear_rows() {
  CDWriter cdata(_cycler);
  nassertv(_format->get_num_arrays() == (int)cdata->_arrays.size());

  Arrays::iterator ai;
  for (ai = cdata->_arrays.begin();
       ai != cdata->_arrays.end();
       ++ai) {
    if ((*ai)->get_ref_count() > 1) {
      (*ai) = new GeomVertexArrayData(*(*ai));
    }
    (*ai)->clear_rows();
  }
  clear_cache();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::modify_array
//       Access: Published
//  Description: Returns a modifiable pointer to the indicated vertex
//               array, so that application code may directly
//               manipulate the data.  You should avoid changing
//               the length of this array, since all of the arrays
//               should be kept in sync--use set_num_rows()
//               instead.
////////////////////////////////////////////////////////////////////
GeomVertexArrayData *GeomVertexData::
modify_array(int i) {
  // Perform copy-on-write: if the reference count on the vertex data
  // is greater than 1, assume some other GeomVertexData has the same
  // pointer, so make a copy of it first.
  CDWriter cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_arrays.size(), NULL);

  if (cdata->_arrays[i]->get_ref_count() > 1) {
    cdata->_arrays[i] = new GeomVertexArrayData(*cdata->_arrays[i]);
  }
  clear_cache();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();

  return cdata->_arrays[i];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::set_array
//       Access: Published
//  Description: Replaces the indicated vertex data array with
//               a completely new array.  You should be careful that
//               the new array has the same length and format as the
//               old one, unless you know what you are doing.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
set_array(int i, const GeomVertexArrayData *array) {
  CDWriter cdata(_cycler);
  nassertv(i >= 0 && i < (int)cdata->_arrays.size());
  cdata->_arrays[i] = (GeomVertexArrayData *)array;
  clear_cache();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::set_transform_table
//       Access: Published
//  Description: Replaces the TransformTable on this vertex
//               data with the indicated table.  The length of this
//               table should be consistent with the maximum table
//               index assigned to the vertices under the
//               "transform_index" name.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
set_transform_table(const TransformTable *table) {
  nassertv(table == (TransformTable *)NULL || table->is_registered());

  CDWriter cdata(_cycler);
  cdata->_transform_table = (TransformTable *)table;
  clear_cache();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::modify_transform_blend_table
//       Access: Published
//  Description: Returns a modifiable pointer to the current
//               TransformBlendTable on this vertex data, if any, or
//               NULL if there is not a TransformBlendTable.  See
//               get_transform_blend_table().
////////////////////////////////////////////////////////////////////
TransformBlendTable *GeomVertexData::
modify_transform_blend_table() {
  // Perform copy-on-write: if the reference count on the table is
  // greater than 1, assume some other GeomVertexData has the same
  // pointer, so make a copy of it first.
  CDWriter cdata(_cycler);

  if (cdata->_transform_blend_table->get_ref_count() > 1) {
    cdata->_transform_blend_table = new TransformBlendTable(*cdata->_transform_blend_table);
  }
  clear_cache();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();

  return cdata->_transform_blend_table;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::set_transform_blend_table
//       Access: Published
//  Description: Replaces the TransformBlendTable on this vertex
//               data with the indicated table.  The length of this
//               table should be consistent with the maximum table
//               index assigned to the vertices under the
//               "transform_blend" name.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
set_transform_blend_table(const TransformBlendTable *table) {
  CDWriter cdata(_cycler);
  cdata->_transform_blend_table = (TransformBlendTable *)table;
  clear_cache();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::set_slider_table
//       Access: Published
//  Description: Replaces the SliderTable on this vertex
//               data with the indicated table.  There should be an
//               entry in this table for each kind of morph offset
//               defined in the vertex data.
//
//               The SliderTable object must have been registered
//               prior to setting it on the GeomVertexData.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
set_slider_table(const SliderTable *table) {
  nassertv(table == (SliderTable *)NULL || table->is_registered());

  CDWriter cdata(_cycler);
  cdata->_slider_table = (SliderTable *)table;
  clear_cache();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_num_bytes
//       Access: Published
//  Description: Returns the total number of bytes consumed by the
//               different arrays of the vertex data.
////////////////////////////////////////////////////////////////////
int GeomVertexData::
get_num_bytes() const {
  CDReader cdata(_cycler);
  
  int num_bytes = sizeof(GeomVertexData);

  Arrays::const_iterator ai;
  for (ai = cdata->_arrays.begin(); ai != cdata->_arrays.end(); ++ai) {
    num_bytes += (*ai)->get_data_size_bytes();
  }

  return num_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::copy_from
//       Access: Published
//  Description: Copies all the data from the other array into the
//               corresponding data types in this array, by matching
//               data types name-by-name.
//
//               keep_data_objects specifies what to do when one or
//               more of the arrays can be copied without the need to
//               apply any conversion operation.  If it is true, the
//               original GeomVertexArrayData objects in this object
//               are retained, but their data arrays are copied
//               pointerwise from the source; if it is false, then the
//               GeomVertexArrayData objects themselves are copied
//               pointerwise from the source.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
copy_from(const GeomVertexData *source, bool keep_data_objects) {
  const GeomVertexFormat *source_format = source->get_format();
  const GeomVertexFormat *dest_format = get_format();

  int num_rows = source->get_num_rows();
  int num_arrays = source_format->get_num_arrays();
  int source_i;

  // First, check to see if any arrays can be simply appropriated for
  // the new format, without changing the data.
  pset<int> done_arrays;

  for (source_i = 0; source_i < num_arrays; ++source_i) {
    const GeomVertexArrayFormat *source_array_format = 
      source_format->get_array(source_i);

    bool array_done = false;

    int dest_num_arrays = dest_format->get_num_arrays();
    for (int dest_i = 0; 
         dest_i < dest_num_arrays && !array_done; 
         ++dest_i) {
      const GeomVertexArrayFormat *dest_array_format = 
        dest_format->get_array(dest_i);
      if (dest_array_format->is_data_subset_of(*source_array_format)) {
        // Great!  Just use the same data for this one.
        if (keep_data_objects) {
          // Copy the data, but keep the same GeomVertexArrayData object.  

          // Maybe it even has the same data pointer already.  If so,
          // avoid flipping the modified flag.
          CPTA_uchar source_data = source->get_array(source_i)->get_data();
          if (get_array(dest_i)->get_data() != source_data) {
            modify_array(dest_i)->set_data(source_data);
          }
        } else {
          // Copy the GeomVertexArrayData object.
          if (get_array(dest_i) != source->get_array(source_i)) {
            set_array(dest_i, source->get_array(source_i));
          }
        }

        array_done = true;
        done_arrays.insert(dest_i);
      }
    }
  }

  // Now make sure the arrays we didn't share are all filled in.
  set_num_rows(num_rows);

  // Now go back through and copy any data that's left over.
  for (source_i = 0; source_i < num_arrays; ++source_i) {
    CPTA_uchar array_data = source->get_array(source_i)->get_data();
    const GeomVertexArrayFormat *source_array_format = source_format->get_array(source_i);
    int num_columns = source_array_format->get_num_columns();
    for (int di = 0; di < num_columns; ++di) {
      const GeomVertexColumn *source_column = source_array_format->get_column(di);

      int dest_i = dest_format->get_array_with(source_column->get_name());
      if (dest_i >= 0 && done_arrays.count(dest_i) == 0) {
        // The data type exists in the new format; we have to copy it.
        const GeomVertexArrayFormat *dest_array_format = 
          dest_format->get_array(dest_i);
        const GeomVertexColumn *dest_column = 
          dest_array_format->get_column(source_column->get_name());
        nassertv(dest_column != (const GeomVertexColumn *)NULL);

        if (dest_column->is_bytewise_equivalent(*source_column)) {
          // We can do a quick bytewise copy.
          PTA_uchar dest_array_data = modify_array(dest_i)->modify_data();

          bytewise_copy(dest_array_data + dest_column->get_start(), 
                        dest_array_format->get_stride(),
                        array_data + source_column->get_start(), source_array_format->get_stride(),
                        source_column, num_rows);

        } else if (dest_column->is_packed_argb() && 
                   source_column->is_uint8_rgba()) {
          // A common special case: OpenGL color to DirectX color.
          PTA_uchar dest_array_data = modify_array(dest_i)->modify_data();

          uint8_rgba_to_packed_argb
            (dest_array_data + dest_column->get_start(), 
             dest_array_format->get_stride(),
             array_data + source_column->get_start(), source_array_format->get_stride(),
             num_rows);

        } else if (dest_column->is_uint8_rgba() && 
                   source_column->is_packed_argb()) {
          // Another common special case: DirectX color to OpenGL
          // color.
          PTA_uchar dest_array_data = modify_array(dest_i)->modify_data();

          packed_argb_to_uint8_rgba
            (dest_array_data + dest_column->get_start(), 
             dest_array_format->get_stride(),
             array_data + source_column->get_start(), source_array_format->get_stride(),
             num_rows);

        } else {
          // A generic copy.
          if (gobj_cat.is_debug()) {
            gobj_cat.debug()
              << "generic copy " << *dest_column << " from " 
              << *source_column << "\n";
          }
          GeomVertexWriter to(this);
          to.set_column(dest_i, dest_column);
          GeomVertexReader from(source);
          from.set_column(source_i, source_column);

          while (!from.is_at_end()) {
            to.set_data4f(from.get_data4f());
          }
        }
      }
    }
  }

    // Also convert the animation tables as necessary.
  const GeomVertexAnimationSpec &source_animation = source_format->get_animation();
  const GeomVertexAnimationSpec &dest_animation = dest_format->get_animation();
  if (source_animation != dest_animation) {
    if (dest_animation.get_animation_type() == AT_hardware) {
      // Convert Panda-style animation tables to hardware-style
      // animation tables.
      CPT(TransformBlendTable) blend_table = source->get_transform_blend_table();
      if (blend_table != (TransformBlendTable *)NULL) {
        PT(TransformTable) transform_table = new TransformTable;
        TransformMap already_added;

        if (dest_animation.get_indexed_transforms()) {
          // Build an indexed transform array.  This is easier; this
          // means we can put the blends in any order.
          GeomVertexWriter weight(this, InternalName::get_transform_weight());
          GeomVertexWriter index(this, InternalName::get_transform_index());
          GeomVertexReader from(source, InternalName::get_transform_blend());
        
          while (!from.is_at_end()) {
            const TransformBlend &blend = blend_table->get_blend(from.get_data1i());
            LVecBase4f weights = LVecBase4f::zero();
            int indices[4] = {0, 0, 0, 0};
            nassertv(blend.get_num_transforms() <= 4);
            
            for (int i = 0; i < blend.get_num_transforms(); i++) {
              weights[i] = blend.get_weight(i);
              indices[i] = add_transform(transform_table, blend.get_transform(i),
                                         already_added);
            }
            if (weight.has_column()) {
              weight.set_data4f(weights);
            }
            index.set_data4i(indices);
          }
        } else {
          // Build a nonindexed transform array.  This means we have to
          // use the same n transforms, in the same order, for each vertex.
          GeomVertexWriter weight(this, InternalName::get_transform_weight());
          GeomVertexReader from(source, InternalName::get_transform_blend());
        
          while (!from.is_at_end()) {
            const TransformBlend &blend = blend_table->get_blend(from.get_data1i());
            LVecBase4f weights = LVecBase4f::zero();
            
            for (int i = 0; i < blend.get_num_transforms(); i++) {
              int index = add_transform(transform_table, blend.get_transform(i),
                                        already_added);
              nassertv(index <= 4);
              weights[index] = blend.get_weight(i);
            }
            if (weight.has_column()) {
              weight.set_data4f(weights);
            }
          }
        }
        
        clear_transform_blend_table();
        set_transform_table(TransformTable::register_table(transform_table));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::copy_row_from
//       Access: Published
//  Description: Copies a single row of the data from the other array
//               into the indicated row of this array.  In this case,
//               the source format must exactly match the destination
//               format.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
copy_row_from(int dest_row, const GeomVertexData *source, 
              int source_row) {
  const GeomVertexFormat *source_format = source->get_format();
  const GeomVertexFormat *dest_format = get_format();
  nassertv(source_format == dest_format);
  nassertv(source_row >= 0 && source_row < source->get_num_rows());

  if (dest_row >= get_num_rows()) {
    // Implicitly add enough rows to get to the indicated row.
    set_num_rows(dest_row + 1);
  }

  int num_arrays = source_format->get_num_arrays();

  for (int i = 0; i < num_arrays; ++i) {
    PTA_uchar dest_array_data = modify_array(i)->modify_data();
    CPTA_uchar source_array_data = source->get_array(i)->get_data();
    const GeomVertexArrayFormat *array_format = source_format->get_array(i);
    int stride = array_format->get_stride();

    memcpy(dest_array_data + stride * dest_row,
           source_array_data + stride * source_row,
           stride);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::convert_to
//       Access: Published
//  Description: Returns a new GeomVertexData that represents the same
//               contents as this one, with all data types matched up
//               name-by-name to the indicated new format.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexData) GeomVertexData::
convert_to(const GeomVertexFormat *new_format) const {
  if (new_format == _format) {
    // Trivial case: no change is needed.
    return this;
  }

  // Look up the new format in our cache--maybe we've recently applied
  // it.
  {
    CDReader cdata(_cycler);
    CacheEntry temp_entry(new_format);
    temp_entry.local_object();
    Cache::const_iterator ci = cdata->_cache.find(&temp_entry);
    if (ci != cdata->_cache.end()) {
      CacheEntry *entry = (*ci);
      // Record a cache hit, so this element will stay in the cache a
      // while longer.
      entry->refresh();
      return entry->_result;
    }
  }

  // Okay, convert the data to the new format.
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Converting " << get_num_rows() << " rows from " << *_format
      << " to " << *new_format << "\n";
  }
  PStatTimer timer(_convert_pcollector);

  PT(GeomVertexData) new_data = 
    new GeomVertexData(get_name(), new_format, get_usage_hint());
  new_data->set_transform_blend_table(get_transform_blend_table());
  new_data->set_slider_table(get_slider_table());

  new_data->copy_from(this, false);

  {
    // Record the new result in the cache.
    CacheEntry *entry;
    {
      CDWriter cdata(((GeomVertexData *)this)->_cycler);
      entry = new CacheEntry((GeomVertexData *)this, new_format, new_data);
      bool inserted = cdata->_cache.insert(entry).second;
      nassertr(inserted, new_data);
    }
    
    // And tell the cache manager about the new entry.  (It might
    // immediately request a delete from the cache of the thing we
    // just added.)
    entry->record();
  }

  return new_data;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::scale_color
//       Access: Published
//  Description: Returns a new GeomVertexData object with the color
//               table modified in-place to apply the indicated scale.
//
//               If the vertex data does not include a color column, a
//               new one will not be added.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexData) GeomVertexData::
scale_color(const LVecBase4f &color_scale) const {
  const GeomVertexColumn *old_column = 
    _format->get_column(InternalName::get_color());
  if (old_column == (GeomVertexColumn *)NULL) {
    return this;
  }

  PT(GeomVertexData) new_data = new GeomVertexData(*this);
  GeomVertexRewriter data(new_data, InternalName::get_color());
  while (!data.is_at_end()) {
    Colorf color = data.get_data4f();
    data.set_data4f(color[0] * color_scale[0],
                    color[1] * color_scale[1],
                    color[2] * color_scale[2],
                    color[3] * color_scale[3]);
  }

  return new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::scale_color
//       Access: Published
//  Description: Returns a new GeomVertexData object with the color
//               table replaced with a new color table that has been
//               scaled by the indicated value.  The new color table
//               will be added as a new array; if the old color table
//               was interleaved with a previous array, the previous
//               array will not be repacked.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexData) GeomVertexData::
scale_color(const LVecBase4f &color_scale, int num_components,
            GeomVertexData::NumericType numeric_type,
            GeomVertexData::Contents contents) const {
  int old_color_array = _format->get_array_with(InternalName::get_color());
  if (old_color_array == -1) {
    // Oops, no color anyway.
    return set_color(color_scale, num_components, numeric_type, contents);
  }

  int num_rows = get_num_rows();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Scaling color for " << num_rows << " vertices by "
      << color_scale << ".\n";
  }
  PStatTimer timer(_scale_color_pcollector);

  PT(GeomVertexData) new_data = replace_column
    (InternalName::get_color(), num_components, numeric_type, contents);

  // Now go through and apply the scale, copying it to the new data.
  GeomVertexWriter to(new_data, InternalName::get_color());
  GeomVertexReader from(this, InternalName::get_color());

  for (int i = 0; i < num_rows; i++) {
    Colorf color = from.get_data4f();
    to.set_data4f(color[0] * color_scale[0],
                  color[1] * color_scale[1],
                  color[2] * color_scale[2],
                  color[3] * color_scale[3]);
  }

  return new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::set_color
//       Access: Published
//  Description: Returns a new GeomVertexData object with the color
//               data modified in-place with the new value.
//
//               If the vertex data does not include a color column, a
//               new one will not be added.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexData) GeomVertexData::
set_color(const Colorf &color) const {
  const GeomVertexColumn *old_column = 
    _format->get_column(InternalName::get_color());
  if (old_column == (GeomVertexColumn *)NULL) {
    return this;
  }

  PT(GeomVertexData) new_data = new GeomVertexData(*this);
  GeomVertexWriter to(new_data, InternalName::get_color());
  while (!to.is_at_end()) {
    to.set_data4f(color);
  }

  return new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::set_color
//       Access: Published
//  Description: Returns a new GeomVertexData object with the color
//               table replaced with a new color table for which each
//               vertex has the indicated value.  The new color table
//               will be added as a new array; if the old color table
//               was interleaved with a previous array, the previous
//               array will not be repacked.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexData) GeomVertexData::
set_color(const Colorf &color, int num_components,
          GeomVertexData::NumericType numeric_type,
          GeomVertexData::Contents contents) const {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Setting color for " << get_num_rows() << " vertices to "
      << color << ".\n";
  }
  PStatTimer timer(_set_color_pcollector);

  PT(GeomVertexData) new_data = replace_column
    (InternalName::get_color(), num_components, numeric_type, contents);

  // Now go through and set the new color value.
  GeomVertexWriter to(new_data, InternalName::get_color());
  while (!to.is_at_end()) {
    to.set_data4f(color);
  }

  return new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::animate_vertices
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
CPT(GeomVertexData) GeomVertexData::
animate_vertices() const {
  CDReader cdata(_cycler);

  if (_format->get_animation().get_animation_type() != AT_panda) {
    return this;
  }

  UpdateSeq modified;
  if (cdata->_transform_blend_table != (TransformBlendTable *)NULL) {
    if (cdata->_slider_table != (SliderTable *)NULL) {
      modified = 
        max(cdata->_transform_blend_table->get_modified(),
            cdata->_slider_table->get_modified());
    } else {
      modified = cdata->_transform_blend_table->get_modified();
    }

  } else if (cdata->_slider_table != (SliderTable *)NULL) {
    modified = cdata->_slider_table->get_modified();

  } else {
    // No transform blend table or slider table--ergo, no vertex
    // animation.
    return this;
  }

  if (cdata->_animated_vertices_modified == modified &&
      cdata->_animated_vertices != (GeomVertexData *)NULL) {
    // No changes.
    return cdata->_animated_vertices;
  }
  CDWriter cdataw(((GeomVertexData *)this)->_cycler, cdata);
  cdataw->_animated_vertices_modified = modified;
  ((GeomVertexData *)this)->update_animated_vertices(cdataw);
  return cdataw->_animated_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::bytewise_copy
//       Access: Private, Static
//  Description: Quickly copies data without the need to convert it.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
bytewise_copy(unsigned char *to, int to_stride,
              const unsigned char *from, int from_stride,
              const GeomVertexColumn *from_type,
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
//     Function: GeomVertexData::replace_column
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
PT(GeomVertexData) GeomVertexData::
replace_column(const InternalName *name, int num_components,
               GeomVertexData::NumericType numeric_type,
               GeomVertexData::Contents contents) const {
  PT(GeomVertexFormat) new_format = new GeomVertexFormat(*_format);

  // Remove the old description of the type from the format.
  bool removed_type_array = false;
  int old_type_array = _format->get_array_with(name);
  if (old_type_array != -1) {
    GeomVertexArrayFormat *array_format = new_format->modify_array(old_type_array);
    if (array_format->get_num_columns() == 1) {
      // Actually, this array didn't have any other data types, so
      // just drop the whole array.
      new_format->remove_array(old_type_array);
      removed_type_array = true;
      
    } else {
      // Remove the description for the type, but don't bother to
      // repack the array.
      array_format->remove_column(name);
    }
  }
    
  // Now define a new array to contain just the type.
  int new_type_array = -1;
  if (num_components != 0) {
    PT(GeomVertexArrayFormat) type_array_format = 
      new GeomVertexArrayFormat(name, num_components, numeric_type, contents);
    new_type_array = new_format->add_array(type_array_format);
  }

  CPT(GeomVertexFormat) format = 
    GeomVertexFormat::register_format(new_format);

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Replacing data type " << *name << "; converting "
      << get_num_rows() << " rows from " 
      << *_format << " to " << *format << "\n";
  }
  
  PT(GeomVertexData) new_data = new GeomVertexData(*this, format);

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
    // room for the right number of rows.
    PT(GeomVertexArrayData) new_array = new GeomVertexArrayData
      (format->get_array(j), get_usage_hint());
    new_array->set_num_rows(get_num_rows());
    new_data->set_array(j, new_array);
  }

  return new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexData::
output(ostream &out) const {
  if (!get_name().empty()) {
    out << get_name() << " ";
  }
  out << get_num_rows() << " rows: " << *get_format();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexData::
write(ostream &out, int indent_level) const {
  if (!get_name().empty()) {
    indent(out, indent_level) << get_name() << "\n";
  }
  _format->write_with_data(out, indent_level + 2, this);
  if (get_transform_blend_table() != (TransformBlendTable *)NULL) {
    indent(out, indent_level)
      << "Transform blend table:\n";
    get_transform_blend_table()->write(out, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::clear_cache
//       Access: Published
//  Description: Removes all of the previously-cached results of
//               convert_to().
////////////////////////////////////////////////////////////////////
void GeomVertexData::
clear_cache() {
  // Probably we shouldn't do anything at all here unless we are
  // running in pipeline stage 0.
  CData *cdata = CDWriter(_cycler);
  for (Cache::iterator ci = cdata->_cache.begin();
       ci != cdata->_cache.end();
       ++ci) {
    CacheEntry *entry = (*ci);
    entry->erase();
  }
  cdata->_cache.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_array_info
//       Access: Public
//  Description: A convenience function to collect together the
//               important parts of the array data for rendering.
//               Given the name of a data type, fills in the start of
//               the array, the number of numeric values for each
//               vertex, the starting bytes number, and the number of
//               bytes to increment for each consecutive vertex.
//
//               The return value is true if the named array data
//               exists in this record, or false if it does not (in
//               which case none of the output parameters are valid).
////////////////////////////////////////////////////////////////////
bool GeomVertexData::
get_array_info(const InternalName *name, 
               const GeomVertexArrayData *&array_data,
               int &num_values, 
               GeomVertexData::NumericType &numeric_type, 
               int &start, int &stride) const {
  int array_index;
  const GeomVertexColumn *column;
  if (_format->get_array_info(name, array_index, column)) {
    CDReader cdata(_cycler);
    array_data = cdata->_arrays[array_index];
    num_values = column->get_num_values();
    numeric_type = column->get_numeric_type();
    start = column->get_start();
    stride = _format->get_array(array_index)->get_stride();
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_vertex_info
//       Access: Public
//  Description: A shortcut to get_array_info() for the "vertex"
//               column.
////////////////////////////////////////////////////////////////////
bool GeomVertexData::
get_vertex_info(const GeomVertexArrayData *&array_data,
                int &num_values, 
                GeomVertexData::NumericType &numeric_type, 
                int &start, int &stride) const {
  int array_index = _format->get_vertex_array_index();
  if (array_index >= 0) {
    const GeomVertexColumn *column = _format->get_vertex_column();

    CDReader cdata(_cycler);
    array_data = cdata->_arrays[array_index];
    num_values = column->get_num_values();
    numeric_type = column->get_numeric_type();
    start = column->get_start();
    stride = _format->get_array(array_index)->get_stride();
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_normal_info
//       Access: Public
//  Description: A shortcut to get_array_info() for the "normal"
//               column.  Note that there is no num_values return,
//               since normals should always have three values.
////////////////////////////////////////////////////////////////////
bool GeomVertexData::
get_normal_info(const GeomVertexArrayData *&array_data,
                GeomVertexData::NumericType &numeric_type, 
                int &start, int &stride) const {
  int array_index = _format->get_normal_array_index();
  if (array_index >= 0) {
    const GeomVertexColumn *column = _format->get_normal_column();
    nassertr(column->get_num_values() == 3, false);

    CDReader cdata(_cycler);
    array_data = cdata->_arrays[array_index];
    numeric_type = column->get_numeric_type();
    start = column->get_start();
    stride = _format->get_array(array_index)->get_stride();
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::get_color_info
//       Access: Public
//  Description: A shortcut to get_array_info() for the "color"
//               column.
////////////////////////////////////////////////////////////////////
bool GeomVertexData::
get_color_info(const GeomVertexArrayData *&array_data,
                int &num_values, 
                GeomVertexData::NumericType &numeric_type, 
                int &start, int &stride) const {
  int array_index = _format->get_color_array_index();
  if (array_index >= 0) {
    const GeomVertexColumn *column = _format->get_color_column();

    CDReader cdata(_cycler);
    array_data = cdata->_arrays[array_index];
    num_values = column->get_num_values();
    numeric_type = column->get_numeric_type();
    start = column->get_start();
    stride = _format->get_array(array_index)->get_stride();
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::packed_argb_to_uint8_rgba
//       Access: Private, Static
//  Description: Quickly converts DirectX-style color to OpenGL-style
//               color.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
packed_argb_to_uint8_rgba(unsigned char *to, int to_stride,
                          const unsigned char *from, int from_stride,
                          int num_records) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "packed_argb_to_uint8_rgba(" << (void *)to << ", " << to_stride
      << ", " << (const void *)from << ", " << from_stride
      << ", " << num_records << ")\n";
  }

  while (num_records > 0) {
    PN_uint32 dword = *(const PN_uint32 *)from;
    to[0] = unpack_abcd_b(dword);
    to[1] = unpack_abcd_c(dword);
    to[2] = unpack_abcd_d(dword);
    to[3] = unpack_abcd_a(dword);

    to += to_stride;
    from += from_stride;
    num_records--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::uint8_rgba_to_packed_argb
//       Access: Private, Static
//  Description: Quickly converts OpenGL-style color to DirectX-style
//               color.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
uint8_rgba_to_packed_argb(unsigned char *to, int to_stride,
                          const unsigned char *from, int from_stride,
                          int num_records) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "uint8_rgba_to_packed_argb(" << (void *)to << ", " << to_stride
      << ", " << (const void *)from << ", " << from_stride
      << ", " << num_records << ")\n";
  }

  while (num_records > 0) {
    *(PN_uint32 *)to = pack_abcd(from[3], from[0], from[1], from[2]);

    to += to_stride;
    from += from_stride;
    num_records--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::do_set_num_rows
//       Access: Private
//  Description: The private implementation of set_num_rows().
////////////////////////////////////////////////////////////////////
bool GeomVertexData::
do_set_num_rows(int n, GeomVertexData::CDWriter &cdata) {
  nassertr(_format->get_num_arrays() == (int)cdata->_arrays.size(), false);

  bool any_changed = false;

  int color_array = -1;
  int orig_color_rows = -1;

  for (size_t i = 0; i < cdata->_arrays.size(); i++) {
    if (cdata->_arrays[i]->get_num_rows() != n) {
      // Copy-on-write.
      if (cdata->_arrays[i]->get_ref_count() > 1) {
        cdata->_arrays[i] = new GeomVertexArrayData(*cdata->_arrays[i]);
      }
      if (cdata->_arrays[i]->has_column(InternalName::get_color())) {
        color_array = i;
        orig_color_rows = cdata->_arrays[i]->get_num_rows();
      }
      cdata->_arrays[i]->set_num_rows(n);
      any_changed = true;
    }
  }

  if (color_array >= 0 && orig_color_rows < n) {
    // We have just added some rows, fill the "color" column with
    // (1, 1, 1, 1), for the programmer's convenience.
    GeomVertexArrayData *array_data = cdata->_arrays[color_array];
    const GeomVertexColumn *column = 
      array_data->get_array_format()->get_column(InternalName::get_color());
    int stride = array_data->get_array_format()->get_stride();
    unsigned char *start = 
      array_data->modify_data() + column->get_start();
    unsigned char *stop = start + array_data->get_data_size_bytes();
    unsigned char *pointer = start + stride * orig_color_rows;
    int num_values = column->get_num_values();

    switch (column->get_numeric_type()) {
    case NT_packed_dcba:
    case NT_packed_dabc:
    case NT_uint8:
    case NT_uint16:
    case NT_uint32:
      while (pointer < stop) {
        memset(pointer, 0xff, column->get_total_bytes());
        pointer += stride;
      }
      break;

    case NT_float32:
      while (pointer < stop) {
        PN_float32 *pi = (PN_float32 *)pointer;
        for (int i = 0; i < num_values; i++) {
          pi[i] = 1.0f;
        }
        pointer += stride;
      }
      break;
    }          
  }

  if (any_changed) {
    clear_cache();
    cdata->_modified = Geom::get_next_modified();
    cdata->_animated_vertices.clear();
  }

  return any_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::update_animated_vertices
//       Access: Private
//  Description: Recomputes the results of computing the vertex
//               animation on the CPU, and applies them to the
//               existing animated_vertices object.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
update_animated_vertices(GeomVertexData::CDWriter &cdata) {
  int num_rows = get_num_rows();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Animating " << num_rows << " vertices for " << get_name()
      << "\n";
  }

  PStatTimer timer(_char_pcollector);

  if (cdata->_animated_vertices == (GeomVertexData *)NULL) {
    CPT(GeomVertexFormat) new_format = _format->get_post_animated_format();
    cdata->_animated_vertices = 
      new GeomVertexData(get_name(), new_format,
                           min(get_usage_hint(), UH_dynamic));
  }
  PT(GeomVertexData) new_data = cdata->_animated_vertices;

  // We have to make a complete copy of the data first so we can
  // modify it.  If we were clever, we could maybe just figure out the
  // subset of the data that might have changed since last frame, but
  // that's too much trouble (and isn't obviously faster than just
  // copying the whole thing).
  new_data->copy_from(this, true);

  // First, apply all of the morphs.
  CPT(SliderTable) slider_table = cdata->_slider_table;
  if (slider_table != (SliderTable *)NULL) {
    PStatTimer timer2(_morphs_pcollector);
    int num_morphs = _format->get_num_morphs();
    for (int mi = 0; mi < num_morphs; mi++) {
      CPT(InternalName) slider_name = _format->get_morph_slider(mi);
      const VertexSlider *slider = slider_table->find_slider(slider_name);
      if (slider != (VertexSlider *)NULL) {
        float slider_value = slider->get_slider();
        if (slider_value != 0.0f) {
          CPT(InternalName) base_name = _format->get_morph_base(mi);
          CPT(InternalName) delta_name = _format->get_morph_delta(mi);
          
          GeomVertexRewriter data(new_data, base_name);
          GeomVertexReader delta(this, delta_name);

          if (data.get_column()->get_num_values() == 4) {
            if (data.get_column()->has_homogeneous_coord()) {
              for (int i = 0; i < num_rows; i++) {
                // Scale the delta by the homogeneous coordinate.
                LPoint4f vertex = data.get_data4f();
                LPoint3f d = delta.get_data3f();
                d *= slider_value * vertex[3];
                data.set_data4f(vertex[0] + d[0],
                                vertex[1] + d[1],
                                vertex[2] + d[2],
                                vertex[3]);
              }
            } else {
              // Just apply the four-component delta.
              for (int i = 0; i < num_rows; i++) {
                const LPoint4f &vertex = data.get_data4f();
                LPoint4f d = delta.get_data4f();
                data.set_data4f(vertex + d * slider_value);
              }
            }
          } else {
            // 3-component or smaller values; don't worry about a
            // homogeneous coordinate.
            for (int i = 0; i < num_rows; i++) {
              const LPoint3f &vertex = data.get_data3f();
              LPoint3f d = delta.get_data3f();
              data.set_data3f(vertex + d * slider_value);
            }
          }
        }
      }
    }
  }

  // Then apply the transforms.
  CPT(TransformBlendTable) tb_table = cdata->_transform_blend_table;
  if (tb_table != (TransformBlendTable *)NULL) {
    PStatTimer timer3(_skinning_pcollector);

    // Recompute all the blends up front, so we don't have to test
    // each one for staleness at each vertex.
    int num_blends = tb_table->get_num_blends();
    int bi;
    for (bi = 0; bi < num_blends; bi++) {
      tb_table->get_blend(bi).update_blend();
    }

    // Now go through and apply the transforms.
    GeomVertexReader blendi(this, InternalName::get_transform_blend());
    if (!blendi.has_column()) {
      gobj_cat.warning()
        << "Vertex data " << get_name()
        << " has a transform_blend_table, but no transform_blend data.\n";
      return;
    }

    int ci;
    for (ci = 0; ci < _format->get_num_points(); ci++) {
      GeomVertexRewriter data(new_data, _format->get_point(ci));
      blendi.set_row(0);
      
      if (data.get_column()->get_num_values() == 4) {
        for (int i = 0; i < num_rows; i++) {
          LPoint4f vertex = data.get_data4f();
          int bi = blendi.get_data1i();
          tb_table->get_blend(bi).transform_point(vertex);
          data.set_data4f(vertex);
        }
      } else {
        for (int i = 0; i < num_rows; i++) {
          LPoint3f vertex = data.get_data3f();
          int bi = blendi.get_data1i();
          tb_table->get_blend(bi).transform_point(vertex);
          data.set_data3f(vertex);
        }
      }
    }
    for (ci = 0; ci < _format->get_num_vectors(); ci++) {
      GeomVertexRewriter data(new_data, _format->get_vector(ci));
      blendi.set_row(0);
      
      for (int i = 0; i < num_rows; i++) {
        LVector3f vertex = data.get_data3f();
        int bi = blendi.get_data1i();
        tb_table->get_blend(bi).transform_vector(vertex);
        data.set_data3f(vertex);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               GeomVertexData.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  dg.add_string(_name);
  manager->write_pointer(dg, _format);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type GeomVertexData is encountered
//               in the Bam file.  It should create the GeomVertexData
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomVertexData::
make_from_bam(const FactoryParams &params) {
  GeomVertexData *object = new GeomVertexData;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);
  manager->register_finalize(object);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int GeomVertexData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  _format = DCAST(GeomVertexFormat, p_list[pi++]);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
finalize(BamReader *manager) {
  // NOTE: This method may be called more than once, because the
  // Geom::finalize() will call it explicitly.  We have to be prepared
  // to accept multiple finalize() calls.

  // Now we need to register the format that we have read from the bam
  // file (since it doesn't come out of the bam file automatically
  // registered).  This may change the format's pointer, which we
  // should then update our own data to reflect.  But since this may
  // cause the unregistered object to destruct, we have to also tell
  // the BamReader to return the new object from now on.

  // This extends to the nested array datas, as well as the transform
  // table and slider tables, as well.

  CDWriter cdata(_cycler);

  CPT(GeomVertexFormat) new_format = 
    GeomVertexFormat::register_format(_format);

  for (size_t i = 0; i < cdata->_arrays.size(); ++i) {
    CPT(GeomVertexArrayFormat) new_array_format = new_format->get_array(i);
    nassertv(cdata->_arrays[i]->_array_format->compare_to(*new_array_format) == 0);

    manager->change_pointer(cdata->_arrays[i]->_array_format, new_array_format);
    cdata->_arrays[i]->_array_format = new_array_format;
  }

  manager->change_pointer(_format, new_format);
  _format = new_format;

  if (cdata->_transform_table != (TransformTable *)NULL) {
    CPT(TransformTable) new_transform_table = 
      TransformTable::register_table(cdata->_transform_table);
    manager->change_pointer(cdata->_transform_table, new_transform_table);
    cdata->_transform_table = new_transform_table;
  }

  if (cdata->_slider_table != (SliderTable *)NULL) {
    CPT(SliderTable) new_slider_table = 
      SliderTable::register_table(cdata->_slider_table);
    manager->change_pointer(cdata->_slider_table, new_slider_table);
    cdata->_slider_table = new_slider_table;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomVertexData.
////////////////////////////////////////////////////////////////////
void GeomVertexData::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  set_name(scan.get_string());
  manager->read_pointer(scan);

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CacheEntry::evict_callback
//       Access: Public, Virtual
//  Description: Called when the entry is evicted from the cache, this
//               should clean up the owning object appropriately.
////////////////////////////////////////////////////////////////////
void GeomVertexData::CacheEntry::
evict_callback() {
  // We have to operate on stage 0 of the pipeline, since that's where
  // the cache really counts.  Because of the multistage pipeline, we
  // might not actually have a cache entry there (it might have been
  // added to stage 1 instead).  No big deal if we don't.
  CData *cdata = _source->_cycler.write_stage(0);
  Cache::iterator ci = cdata->_cache.find(this);
  if (ci != cdata->_cache.end()) {
    cdata->_cache.erase(ci);
  }
  _source->_cycler.release_write_stage(0, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CacheEntry::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexData::CacheEntry::
output(ostream &out) const {
  out << "vertex data " << (void *)_source << " to " 
      << *_modifier;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *GeomVertexData::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void GeomVertexData::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_uint8(_usage_hint);

  dg.add_uint16(_arrays.size());
  Arrays::const_iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    manager->write_pointer(dg, *ai);
  }

  manager->write_pointer(dg, _transform_table);
  manager->write_pointer(dg, _transform_blend_table);
  manager->write_pointer(dg, _slider_table);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int GeomVertexData::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  Arrays::iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    (*ai) = DCAST(GeomVertexArrayData, p_list[pi++]);    
  }

  _transform_table = DCAST(TransformTable, p_list[pi++]);
  _transform_blend_table = DCAST(TransformBlendTable, p_list[pi++]);
  _slider_table = DCAST(SliderTable, p_list[pi++]);

  _modified = Geom::get_next_modified();

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexData::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomVertexData.
////////////////////////////////////////////////////////////////////
void GeomVertexData::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _usage_hint = (UsageHint)scan.get_uint8();

  size_t num_arrays = scan.get_uint16();
  _arrays.reserve(num_arrays);
  for (size_t i = 0; i < num_arrays; ++i) {
    manager->read_pointer(scan);
    _arrays.push_back(NULL);
  }

  manager->read_pointer(scan);
  manager->read_pointer(scan);
  manager->read_pointer(scan);
}
