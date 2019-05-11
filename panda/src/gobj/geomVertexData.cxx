/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexData.cxx
 * @author drose
 * @date 2005-03-06
 */

#include "geomVertexData.h"
#include "geom.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "geomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pset.h"
#include "indent.h"

using std::ostream;

TypeHandle GeomVertexData::_type_handle;
TypeHandle GeomVertexData::CDataCache::_type_handle;
TypeHandle GeomVertexData::CacheEntry::_type_handle;
TypeHandle GeomVertexData::CData::_type_handle;
TypeHandle GeomVertexDataPipelineReader::_type_handle;
TypeHandle GeomVertexDataPipelineWriter::_type_handle;

PStatCollector GeomVertexData::_convert_pcollector("*:Munge:Convert");
PStatCollector GeomVertexData::_scale_color_pcollector("*:Munge:Scale color");
PStatCollector GeomVertexData::_set_color_pcollector("*:Munge:Set color");
PStatCollector GeomVertexData::_animation_pcollector("*:Animation");


/**
 * Constructs an invalid object.  This is only used when reading from the bam
 * file.
 */
GeomVertexData::
GeomVertexData() :
  _char_pcollector(_animation_pcollector, "unnamed"),
  _skinning_pcollector(_char_pcollector, "Skinning"),
  _morphs_pcollector(_char_pcollector, "Morphs"),
  _blends_pcollector(_char_pcollector, "Calc blends")
{
}

/**
 * Required to implement CopyOnWriteObject.
 */
PT(CopyOnWriteObject) GeomVertexData::
make_cow_copy() {
  return new GeomVertexData(*this);
}

/**
 *
 */
GeomVertexData::
GeomVertexData(const std::string &name,
               const GeomVertexFormat *format,
               GeomVertexData::UsageHint usage_hint) :
  _name(name),
  _char_pcollector(PStatCollector(_animation_pcollector, name)),
  _skinning_pcollector(_char_pcollector, "Skinning"),
  _morphs_pcollector(_char_pcollector, "Morphs"),
  _blends_pcollector(_char_pcollector, "Calc blends"),
  _cycler(GeomVertexData::CData(format, usage_hint))
{
  nassertv(format->is_registered());
}

/**
 *
 */
GeomVertexData::
GeomVertexData(const GeomVertexData &copy) :
  CopyOnWriteObject(copy),
  _name(copy._name),
  _cycler(copy._cycler),
  _char_pcollector(copy._char_pcollector),
  _skinning_pcollector(copy._skinning_pcollector),
  _morphs_pcollector(copy._morphs_pcollector),
  _blends_pcollector(copy._blends_pcollector)
{
  OPEN_ITERATE_ALL_STAGES(_cycler) {
    CDStageWriter cdata(_cycler, pipeline_stage);
    // It's important that we *not* copy the animated_vertices pointer.
    cdata->_animated_vertices = nullptr;
    cdata->_animated_vertices_modified = UpdateSeq();
  }
  CLOSE_ITERATE_ALL_STAGES(_cycler);
}

/**
 * This constructor copies all of the basic properties of the source
 * VertexData, like usage_hint and animation tables, but does not copy the
 * actual data, and it allows you to specify a different format.
 */
GeomVertexData::
GeomVertexData(const GeomVertexData &copy,
               const GeomVertexFormat *format) :
  CopyOnWriteObject(copy),
  _name(copy._name),
  _cycler(copy._cycler),
  _char_pcollector(copy._char_pcollector),
  _skinning_pcollector(copy._skinning_pcollector),
  _morphs_pcollector(copy._morphs_pcollector),
  _blends_pcollector(copy._blends_pcollector)
{
  nassertv(format->is_registered());

  // Create some empty arrays as required by the format.
  OPEN_ITERATE_ALL_STAGES(_cycler) {
    CDStageWriter cdata(_cycler, pipeline_stage);

    UsageHint usage_hint = cdata->_usage_hint;
    cdata->_arrays.clear();
    cdata->_format = format;
    int num_arrays = format->get_num_arrays();
    for (int i = 0; i < num_arrays; i++) {
      PT(GeomVertexArrayData) array = new GeomVertexArrayData
        (format->get_array(i), usage_hint);
      cdata->_arrays.push_back(array.p());
    }

    // It's important that we *not* copy the animated_vertices pointer.
    cdata->_animated_vertices = nullptr;
    cdata->_animated_vertices_modified = UpdateSeq();
  }
  CLOSE_ITERATE_ALL_STAGES(_cycler);
}

/**
 * The copy assignment operator is not pipeline-safe.  This will completely
 * obliterate all stages of the pipeline, so don't do it for a GeomVertexData
 * that is actively being used for rendering.
 */
void GeomVertexData::
operator = (const GeomVertexData &copy) {
  CopyOnWriteObject::operator = (copy);

  clear_cache();

  _name = copy._name;
  _cycler = copy._cycler;
  _char_pcollector = copy._char_pcollector;
  _skinning_pcollector = copy._skinning_pcollector;
  _morphs_pcollector = copy._morphs_pcollector;
  _blends_pcollector = copy._blends_pcollector;

  OPEN_ITERATE_ALL_STAGES(_cycler) {
    CDStageWriter cdata(_cycler, pipeline_stage);
    cdata->_modified = Geom::get_next_modified();
    cdata->_animated_vertices = nullptr;
    cdata->_animated_vertices_modified = UpdateSeq();
  }
  CLOSE_ITERATE_ALL_STAGES(_cycler);
}

/**
 *
 */
GeomVertexData::
~GeomVertexData() {
  clear_cache();
}

/**
 * Returns 0 if the two objects are equivalent, even if they are not the same
 * pointer.
 */
int GeomVertexData::
compare_to(const GeomVertexData &other) const {
  CDReader cdata(_cycler);
  CDReader other_cdata(other._cycler);

  if (cdata->_usage_hint != other_cdata->_usage_hint) {
    return (int)cdata->_usage_hint - (int)other_cdata->_usage_hint;
  }
  if (cdata->_format != other_cdata->_format) {
    return cdata->_format < other_cdata->_format ? -1 : 1;
  }
  if (cdata->_transform_table != other_cdata->_transform_table) {
    return cdata->_transform_table < other_cdata->_transform_table ? -1 : 1;
  }
  if (cdata->_transform_blend_table != other_cdata->_transform_blend_table) {
    return cdata->_transform_blend_table < other_cdata->_transform_blend_table ? -1 : 1;
  }
  if (cdata->_slider_table != other_cdata->_slider_table) {
    return cdata->_slider_table < other_cdata->_slider_table ? -1 : 1;
  }
  if (cdata->_arrays.size() != other_cdata->_arrays.size()) {
    return (int)cdata->_arrays.size() - (int)other_cdata->_arrays.size();
  }
  for (size_t i = 0; i < cdata->_arrays.size(); ++i) {
    if (cdata->_arrays[i] != other_cdata->_arrays[i]) {
      return cdata->_arrays[i] < other_cdata->_arrays[i] ? -1 : 1;
    }
  }

  return 0;
}

/**
 * Changes the name of the vertex data.  This name is reported on the PStats
 * graph for vertex computations.
 */
void GeomVertexData::
set_name(const std::string &name) {
  _name = name;
  _char_pcollector = PStatCollector(_animation_pcollector, name);
  _skinning_pcollector = PStatCollector(_char_pcollector, "Skinning");
  _morphs_pcollector = PStatCollector(_char_pcollector, "Morphs");
  _blends_pcollector = PStatCollector(_char_pcollector, "Calc blends");
}

/**
 * Changes the UsageHint hint for this vertex data, and for all of the arrays
 * that share this data.  See get_usage_hint().
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexData::
set_usage_hint(GeomVertexData::UsageHint usage_hint) {
  CDWriter cdata(_cycler, true);
  cdata->_usage_hint = usage_hint;

  Arrays::iterator ai;
  for (ai = cdata->_arrays.begin();
       ai != cdata->_arrays.end();
       ++ai) {
    PT(GeomVertexArrayData) array_obj = (*ai).get_write_pointer();
    array_obj->set_usage_hint(usage_hint);
  }
  clear_cache_stage();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

/**
 * Changes the format of the vertex data.  If the data is not empty, this will
 * implicitly change every row to match the new format.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexData::
set_format(const GeomVertexFormat *format) {
  Thread *current_thread = Thread::get_current_thread();
  nassertv(format->is_registered());

  CDLockedReader cdata(_cycler, current_thread);

  if (format == cdata->_format) {
    // Trivially no-op.
    return;
  }

  CDWriter cdataw(_cycler, cdata, true);

  // Put the current data aside, so we can copy it back in below.
  CPT(GeomVertexData) orig_data = new GeomVertexData(*this);

  // Assign the new format.  This means clearing out all of our current arrays
  // and replacing them with new, empty arrays.
  cdataw->_format = format;

  UsageHint usage_hint = cdataw->_usage_hint;
  cdataw->_arrays.clear();
  int num_arrays = cdataw->_format->get_num_arrays();
  for (int i = 0; i < num_arrays; i++) {
    PT(GeomVertexArrayData) array = new GeomVertexArrayData
      (cdataw->_format->get_array(i), usage_hint);
    cdataw->_arrays.push_back(array.p());
  }

  // Now copy the original data back in.  This will automatically convert it
  // to the new format.
  copy_from(orig_data, false, current_thread);

  clear_cache_stage();
  cdataw->_modified = Geom::get_next_modified();
  cdataw->_animated_vertices.clear();
}

/**
 * Changes the format of the vertex data, without reformatting the data to
 * match.  The data is exactly the same after this operation, but will be
 * reinterpreted according to the new format.  This assumes that the new
 * format is fundamentally compatible with the old format; in particular, it
 * must have the same number of arrays with the same stride in each one.  No
 * checking is performed that the data remains sensible.
 */
void GeomVertexData::
unclean_set_format(const GeomVertexFormat *format) {
  Thread *current_thread = Thread::get_current_thread();
  nassertv(format->is_registered());

  CDLockedReader cdata(_cycler, current_thread);

  if (format == cdata->_format) {
    // Trivially no-op.
    return;
  }

#ifndef NDEBUG
  nassertv(format->get_num_arrays() == cdata->_format->get_num_arrays());
  for (size_t ai = 0; ai < format->get_num_arrays(); ++ai) {
    nassertv(format->get_array(ai)->get_stride() == cdata->_format->get_array(ai)->get_stride());
  }
  nassertv(cdata->_arrays.size() == cdata->_format->get_num_arrays());
#endif  // NDEBUG

  CDWriter cdataw(_cycler, cdata, true);

  // Assign the new format.
  cdataw->_format = format;

  for (size_t ai = 0; ai < cdataw->_arrays.size(); ++ai) {
    PT(GeomVertexArrayData) array_obj = cdataw->_arrays[ai].get_write_pointer();
    array_obj->_array_format = format->get_array(ai);
  }

  clear_cache_stage();
  cdataw->_modified = Geom::get_next_modified();
  cdataw->_animated_vertices.clear();
}

/**
 * Removes all of the rows from the arrays; functionally equivalent to
 * set_num_rows(0) (but faster).
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexData::
clear_rows() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  nassertv(cdata->_format->get_num_arrays() == cdata->_arrays.size());

  Arrays::iterator ai;
  for (ai = cdata->_arrays.begin();
       ai != cdata->_arrays.end();
       ++ai) {
    PT(GeomVertexArrayData) array_obj = (*ai).get_write_pointer();
    array_obj->clear_rows();
  }
  clear_cache_stage();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices.clear();
}

/**
 * Replaces the TransformTable on this vertex data with the indicated table.
 * The length of this table should be consistent with the maximum table index
 * assigned to the vertices under the "transform_index" name.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexData::
set_transform_table(const TransformTable *table) {
  Thread *current_thread = Thread::get_current_thread();
  nassertv(table == nullptr || table->is_registered());

  CDWriter cdata(_cycler, true, current_thread);
  cdata->_transform_table = (TransformTable *)table;
  clear_cache_stage();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

/**
 * Returns a modifiable pointer to the current TransformBlendTable on this
 * vertex data, if any, or NULL if there is not a TransformBlendTable.  See
 * get_transform_blend_table().
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
PT(TransformBlendTable) GeomVertexData::
modify_transform_blend_table() {
  CDWriter cdata(_cycler, true);

  clear_cache_stage();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();

  return cdata->_transform_blend_table.get_write_pointer();
}

/**
 * Replaces the TransformBlendTable on this vertex data with the indicated
 * table.  The length of this table should be consistent with the maximum
 * table index assigned to the vertices under the "transform_blend" name.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexData::
set_transform_blend_table(const TransformBlendTable *table) {
  CDWriter cdata(_cycler, true);
  cdata->_transform_blend_table = (TransformBlendTable *)table;
  clear_cache_stage();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

/**
 * Replaces the SliderTable on this vertex data with the indicated table.
 * There should be an entry in this table for each kind of morph offset
 * defined in the vertex data.
 *
 * The SliderTable object must have been registered prior to setting it on the
 * GeomVertexData.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexData::
set_slider_table(const SliderTable *table) {
  nassertv(table == nullptr || table->is_registered());

  CDWriter cdata(_cycler, true);
  cdata->_slider_table = (SliderTable *)table;
  clear_cache_stage();
  cdata->_modified = Geom::get_next_modified();
  cdata->_animated_vertices_modified = UpdateSeq();
}

/**
 * Returns true if the vertex data is currently resident in memory.  If this
 * returns false, the vertex data will be brought back into memory shortly;
 * try again later.
 */
bool GeomVertexData::
request_resident() const {
  CDReader cdata(_cycler);

  bool resident = true;

  Arrays::const_iterator ai;
  for (ai = cdata->_arrays.begin();
       ai != cdata->_arrays.end();
       ++ai) {
    if (!(*ai).get_read_pointer()->request_resident()) {
      resident = false;
    }
  }

  return resident;
}

/**
 * Copies all the data from the other array into the corresponding data types
 * in this array, by matching data types name-by-name.
 *
 * keep_data_objects specifies what to do when one or more of the arrays can
 * be copied without the need to apply any conversion operation.  If it is
 * true, the original GeomVertexArrayData objects in this object are retained,
 * and their data arrays are copied byte-by-byte from the source; if it is
 * false, then the GeomVertexArrayData objects are copied pointerwise from the
 * source.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexData::
copy_from(const GeomVertexData *source, bool keep_data_objects,
          Thread *current_thread) {
  const GeomVertexFormat *source_format = source->get_format();
  const GeomVertexFormat *dest_format = get_format();

  int num_rows = source->get_num_rows();
  int num_arrays = source_format->get_num_arrays();
  int source_i;

  // First, check to see if any arrays can be simply appropriated for the new
  // format, without changing the data.
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

          modify_array_handle(dest_i)->copy_data_from(source->get_array_handle(source_i));
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
  {
    GeomVertexDataPipelineWriter writer(this, true, Thread::get_current_thread());
    writer.check_array_writers();
    writer.reserve_num_rows(num_rows);
    writer.set_num_rows(num_rows);
  }

  // Now go back through and copy any data that's left over.
  for (source_i = 0; source_i < num_arrays; ++source_i) {
    CPT(GeomVertexArrayDataHandle) array_handle = source->get_array_handle(source_i);
    const unsigned char *array_data = array_handle->get_read_pointer(true);
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
        nassertv(dest_column != nullptr);

        if (dest_column->is_bytewise_equivalent(*source_column)) {
          // We can do a quick bytewise copy.
          PT(GeomVertexArrayDataHandle) dest_handle = modify_array_handle(dest_i);
          unsigned char *dest_array_data = dest_handle->get_write_pointer();

          bytewise_copy(dest_array_data + dest_column->get_start(),
                        dest_array_format->get_stride(),
                        array_data + source_column->get_start(), source_array_format->get_stride(),
                        source_column, num_rows);

        } else if (dest_column->is_packed_argb() &&
                   source_column->is_uint8_rgba()) {
          // A common special case: OpenGL color to DirectX color.
          PT(GeomVertexArrayDataHandle) dest_handle = modify_array_handle(dest_i);
          unsigned char *dest_array_data = dest_handle->get_write_pointer();

          uint8_rgba_to_packed_argb
            (dest_array_data + dest_column->get_start(),
             dest_array_format->get_stride(),
             array_data + source_column->get_start(), source_array_format->get_stride(),
             num_rows);

        } else if (dest_column->is_uint8_rgba() &&
                   source_column->is_packed_argb()) {
          // Another common special case: DirectX color to OpenGL color.
          PT(GeomVertexArrayDataHandle) dest_handle = modify_array_handle(dest_i);
          unsigned char *dest_array_data = dest_handle->get_write_pointer();

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
            to.set_data4(from.get_data4());
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
      // Convert Panda-style animation tables to hardware-style animation
      // tables.
      CPT(TransformBlendTable) blend_table = source->get_transform_blend_table();
      if (blend_table != nullptr) {
        PT(TransformTable) transform_table = new TransformTable;
        TransformMap already_added;

        if (dest_animation.get_indexed_transforms()) {
          // Build an indexed transform array.  This is easier; this means we
          // can put the blends in any order.
          GeomVertexWriter weight(this, InternalName::get_transform_weight());
          GeomVertexWriter index(this, InternalName::get_transform_index());
          GeomVertexReader from(source, InternalName::get_transform_blend());

          while (!from.is_at_end()) {
            const TransformBlend &blend = blend_table->get_blend(from.get_data1i());
            LVecBase4 weights = LVecBase4::zero();
            LVecBase4i indices(0, 0, 0, 0);

            if (blend.get_num_transforms() <= 4) {
              for (size_t i = 0; i < blend.get_num_transforms(); i++) {
                weights[i] = blend.get_weight(i);
                indices[i] = add_transform(transform_table, blend.get_transform(i),
                                           already_added);
              }
            } else {
              // Limit the number of blends to the four with highest weights.
              TransformBlend blend2(blend);
              blend2.limit_transforms(4);
              blend2.normalize_weights();

              for (size_t i = 0; i < 4; i++) {
                weights[i] = blend2.get_weight(i);
                indices[i] = add_transform(transform_table, blend2.get_transform(i),
                                           already_added);
              }
            }

            if (weight.has_column()) {
              weight.set_data4(weights);
            }
            index.set_data4i(indices);
          }
        } else {
          // Build a nonindexed transform array.  This means we have to use
          // the same n transforms, in the same order, for each vertex.
          GeomVertexWriter weight(this, InternalName::get_transform_weight());
          GeomVertexReader from(source, InternalName::get_transform_blend());

          while (!from.is_at_end()) {
            const TransformBlend &blend = blend_table->get_blend(from.get_data1i());
            LVecBase4 weights = LVecBase4::zero();

            for (size_t i = 0; i < blend.get_num_transforms(); i++) {
              int index = add_transform(transform_table, blend.get_transform(i),
                                        already_added);
              nassertv(index <= 4);
              weights[index] = blend.get_weight(i);
            }
            if (weight.has_column()) {
              weight.set_data4(weights);
            }
          }
        }

        clear_transform_blend_table();
        set_transform_table(TransformTable::register_table(transform_table));
      }
    }
  }
}

/**
 * Copies a single row of the data from the other array into the indicated row
 * of this array.  In this case, the source format must exactly match the
 * destination format.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexData::
copy_row_from(int dest_row, const GeomVertexData *source,
              int source_row, Thread *current_thread) {

  GeomVertexDataPipelineReader reader(source, current_thread);
  reader.check_array_readers();

  GeomVertexDataPipelineWriter writer(this, true, current_thread);
  writer.check_array_writers();
  writer.copy_row_from(dest_row, reader, source_row);
}

/**
 * Returns a new GeomVertexData that represents the same contents as this one,
 * with all data types matched up name-by-name to the indicated new format.
 */
CPT(GeomVertexData) GeomVertexData::
convert_to(const GeomVertexFormat *new_format) const {
  Thread *current_thread = Thread::get_current_thread();

  if (new_format == get_format()) {
    // Trivial case: no change is needed.
    return this;
  }

  // Look up the new format in our cache--maybe we've recently applied it.
  PT(CacheEntry) entry;

  CacheKey key(new_format);

  _cache_lock.acquire();
  Cache::const_iterator ci = _cache.find(&key);
  if (ci == _cache.end()) {
    _cache_lock.release();

  } else {
    entry = (*ci).second;
    _cache_lock.release();
    nassertr(entry->_source == this, nullptr);

    // Here's an element in the cache for this computation.  Record a cache
    // hit, so this element will stay in the cache a while longer.
    entry->refresh(current_thread);

    CDCacheReader cdata(entry->_cycler);
    if (cdata->_result != nullptr) {
      return cdata->_result;
    }

    // The cache entry is stale, but we'll recompute it below.  Note that
    // there's a small race condition here; another thread might recompute the
    // cache at the same time.  No big deal, since it'll compute the same
    // result.
  }

  // Okay, convert the data to the new format.
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Converting " << get_num_rows() << " rows from " << *get_format()
      << " to " << *new_format << "\n";
  }
  PStatTimer timer(_convert_pcollector);

  PT(GeomVertexData) new_data =
    new GeomVertexData(get_name(), new_format, get_usage_hint());
  new_data->set_transform_blend_table(get_transform_blend_table());
  new_data->set_slider_table(get_slider_table());

  new_data->copy_from(this, false);

  // Record the new result in the cache.
  if (entry == nullptr) {
    // Create a new entry for the result.
    // We don't need the key anymore, move the pointers into the CacheEntry.
    entry = new CacheEntry((GeomVertexData *)this, std::move(key));

    {
      LightMutexHolder holder(_cache_lock);
      bool inserted = ((GeomVertexData *)this)->_cache.insert(Cache::value_type(&entry->_key, entry)).second;
      if (!inserted) {
        // Some other thread must have beat us to the punch.  Never mind.
        return new_data;
      }
    }

    // And tell the cache manager about the new entry.  (It might immediately
    // request a delete from the cache of the thing we just added.)
    entry->record(current_thread);
  }

  // Finally, store the cached result on the entry.
  CDCacheWriter cdata(entry->_cycler, true, current_thread);
  cdata->_result = new_data;

  return new_data;
}

/**
 * Returns a new GeomVertexData object with the color table modified in-place
 * to apply the indicated scale.
 *
 * If the vertex data does not include a color column, a new one will not be
 * added.
 */
CPT(GeomVertexData) GeomVertexData::
scale_color(const LVecBase4 &color_scale) const {
  const GeomVertexColumn *old_column =
    get_format()->get_column(InternalName::get_color());
  if (old_column == nullptr) {
    return this;
  }

  PT(GeomVertexData) new_data = new GeomVertexData(*this);
  GeomVertexRewriter data(new_data, InternalName::get_color());
  while (!data.is_at_end()) {
    LColor color = data.get_data4();
    data.set_data4(color[0] * color_scale[0],
                    color[1] * color_scale[1],
                    color[2] * color_scale[2],
                    color[3] * color_scale[3]);
  }

  return new_data;
}

/**
 * Returns a new GeomVertexData object with the color table replaced with a
 * new color table that has been scaled by the indicated value.  The new color
 * table will be added as a new array; if the old color table was interleaved
 * with a previous array, the previous array will not be repacked.
 */
CPT(GeomVertexData) GeomVertexData::
scale_color(const LVecBase4 &color_scale, int num_components,
            GeomVertexData::NumericType numeric_type,
            GeomVertexData::Contents contents) const {
  int old_color_array = get_format()->get_array_with(InternalName::get_color());
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
    LColor color = from.get_data4();
    to.set_data4(color[0] * color_scale[0],
                  color[1] * color_scale[1],
                  color[2] * color_scale[2],
                  color[3] * color_scale[3]);
  }

  return new_data;
}

/**
 * Returns a new GeomVertexData object with the color data modified in-place
 * with the new value.
 *
 * If the vertex data does not include a color column, a new one will not be
 * added.
 */
CPT(GeomVertexData) GeomVertexData::
set_color(const LColor &color) const {
  const GeomVertexColumn *old_column =
    get_format()->get_column(InternalName::get_color());
  if (old_column == nullptr) {
    return this;
  }

  PT(GeomVertexData) new_data = new GeomVertexData(*this);
  do_set_color(new_data, color);
  return new_data;
}

/**
 * Returns a new GeomVertexData object with the color table replaced with a
 * new color table for which each vertex has the indicated value.  The new
 * color table will be added as a new array; if the old color table was
 * interleaved with a previous array, the previous array will not be repacked.
 */
CPT(GeomVertexData) GeomVertexData::
set_color(const LColor &color, int num_components,
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

  do_set_color(new_data, color);
  return new_data;
}

/**
 * Returns a new GeomVertexData object with the normal data modified in-place,
 * so that each lighting normal is now facing in the opposite direction.
 *
 * If the vertex data does not include a normal column, this returns the
 * original GeomVertexData object, unchanged.
 */
CPT(GeomVertexData) GeomVertexData::
reverse_normals() const {
  const GeomVertexColumn *old_column =
    get_format()->get_column(InternalName::get_normal());
  if (old_column == nullptr) {
    return this;
  }

  PT(GeomVertexData) new_data = new GeomVertexData(*this);
  GeomVertexRewriter to(new_data, InternalName::get_normal());
  while (!to.is_at_end()) {
    to.set_data3(-to.get_data3());
  }

  return new_data;
}

/**
 * Returns a GeomVertexData that represents the results of computing the
 * vertex animation on the CPU for this GeomVertexData.
 *
 * If there is no CPU-defined vertex animation on this object, this just
 * returns the original object.
 *
 * If there is vertex animation, but the VertexTransform values have not
 * changed since last time, this may return the same pointer it returned
 * previously.  Even if the VertexTransform values have changed, it may still
 * return the same pointer, but with its contents modified (this is preferred,
 * since it allows the graphics backend to update vertex buffers optimally).
 *
 * If force is false, this method may return immediately with stale data, if
 * the vertex data is not completely resident.  If force is true, this method
 * will never return stale data, but may block until the data is available.
 */
CPT(GeomVertexData) GeomVertexData::
animate_vertices(bool force, Thread *current_thread) const {
#ifdef DO_PIPELINING
  {
    // In the pipelining case, we take a simple short-route optimization: if
    // the vdata isn't animated, we don't need to grab any mutex first.
    CDReader cdata(_cycler, current_thread);
    if (cdata->_format->get_animation().get_animation_type() != AT_panda) {
      return this;
    }
  }
#endif  // DO_PIPELINING

  PStatTimer timer(((GeomVertexData *)this)->_char_pcollector, current_thread);

  // Now that we've short-circuited the short route, we reasonably believe the
  // vdata is animated.  Grab the mutex and make sure it's still animated
  // after we've acquired it.
  CDLockedReader cdata(_cycler, current_thread);
  if (cdata->_format->get_animation().get_animation_type() != AT_panda) {
    return this;
  }

  UpdateSeq modified;
  {
    PStatTimer timer2(((GeomVertexData *)this)->_blends_pcollector, current_thread);
    if (!cdata->_transform_blend_table.is_null()) {
      if (cdata->_slider_table != nullptr) {
        modified =
          std::max(cdata->_transform_blend_table.get_read_pointer()->get_modified(current_thread),
              cdata->_slider_table->get_modified(current_thread));
      } else {
        modified = cdata->_transform_blend_table.get_read_pointer()->get_modified(current_thread);
      }

    } else if (cdata->_slider_table != nullptr) {
      modified = cdata->_slider_table->get_modified(current_thread);

    } else {
      // No transform blend table or slider table--ergo, no vertex animation.
      return this;
    }
  }

  if (cdata->_animated_vertices_modified == modified &&
      cdata->_animated_vertices != nullptr) {
    // No changes.
    return cdata->_animated_vertices;
  }

  if (!force && !request_resident()) {
    // The vertex data isn't resident.  Return the best information we've got.
    if (cdata->_animated_vertices != nullptr) {
      return cdata->_animated_vertices;
    }
    return this;
  }

  CDWriter cdataw(((GeomVertexData *)this)->_cycler, cdata, false);
  cdataw->_animated_vertices_modified = modified;
  ((GeomVertexData *)this)->update_animated_vertices(cdataw, current_thread);

  return cdataw->_animated_vertices;
}

/**
 * Removes the cache of animated vertices computed by a previous call to
 * animate_vertices() within the same frame.  This will force the next call to
 * animate_vertices() to recompute these values from scratch.  Normally it is
 * not necessary to call this.
 */
void GeomVertexData::
clear_animated_vertices() {
  CDWriter cdata(_cycler, true);
  cdata->_animated_vertices_modified.clear();
  cdata->_animated_vertices.clear();
}


/**
 * Applies the indicated transform matrix to all of the vertices in the
 * GeomVertexData.  The transform is applied to all "point" and "vector" type
 * columns described in the format.
 */
void GeomVertexData::
transform_vertices(const LMatrix4 &mat) {
  transform_vertices(mat, 0, get_num_rows());
}

/**
 * Applies the indicated transform matrix to all of the vertices from
 * begin_row up to but not including end_row.  The transform is applied to all
 * "point" and "vector" type columns described in the format.
 */
void GeomVertexData::
transform_vertices(const LMatrix4 &mat, int begin_row, int end_row) {
  if (end_row <= begin_row) {
    // Trivial no-op.
    return;
  }

  const GeomVertexFormat *format = get_format();

  size_t ci;
  for (ci = 0; ci < format->get_num_points(); ci++) {
    GeomVertexRewriter data(this, format->get_point(ci));
    do_transform_point_column(format, data, mat, begin_row, end_row);
  }

  for (ci = 0; ci < format->get_num_vectors(); ci++) {
    GeomVertexRewriter data(this, format->get_vector(ci));
    do_transform_vector_column(format, data, mat, begin_row, end_row);
  }
}

/**
 * Applies the indicated transform matrix to all of the vertices mentioned in
 * the sparse array.  The transform is applied to all "point" and "vector"
 * type columns described in the format.
 */
void GeomVertexData::
transform_vertices(const LMatrix4 &mat, const SparseArray &rows) {
  if (rows.is_zero()) {
    // Trivial no-op.
    return;
  }

  const GeomVertexFormat *format = get_format();

  size_t ci;
  for (ci = 0; ci < format->get_num_points(); ci++) {
    GeomVertexRewriter data(this, format->get_point(ci));

    for (size_t i = 0; i < rows.get_num_subranges(); ++i) {
      int begin_row = rows.get_subrange_begin(i);
      int end_row = rows.get_subrange_end(i);
      do_transform_point_column(format, data, mat, begin_row, end_row);
    }
  }

  for (ci = 0; ci < format->get_num_vectors(); ci++) {
    GeomVertexRewriter data(this, format->get_vector(ci));

    for (size_t i = 0; i < rows.get_num_subranges(); ++i) {
      int begin_row = rows.get_subrange_begin(i);
      int end_row = rows.get_subrange_end(i);
      do_transform_vector_column(format, data, mat, begin_row, end_row);
    }
  }
}

/**
 * Fills in the color column of the given vertex data object with a constant
 * color.  Assumes that there is already a color column present.
 */
void GeomVertexData::
do_set_color(GeomVertexData *vdata, const LColor &color) {
  // This function is used relatively often (by the SceneGraphReducer, when
  // flattening colors, and by the munger, when munging colors), so I've
  // written out a version that avoids the performance overhead of the packer
  // and GeomVertexWriter.

  const GeomVertexFormat *format = vdata->get_format();
  const GeomVertexColumn *column;
  int array_index;
  if (!format->get_array_info(InternalName::get_color(), array_index, column)) {
    nassert_raise("no color column");
    return;
  }

  size_t stride = format->get_array(array_index)->get_stride();

  GeomVertexColumn::Packer *packer = column->_packer;
  nassertv(packer != nullptr);

  // Pack into a buffer, which we will then copy.
  unsigned char buffer[32];
  size_t bufsize = column->get_total_bytes();
#ifdef STDFLOAT_DOUBLE
  packer->set_data4d(buffer, color);
#else
  packer->set_data4f(buffer, color);
#endif

  PT(GeomVertexArrayDataHandle) handle = vdata->modify_array_handle(array_index);
  unsigned char *write_ptr = handle->get_write_pointer();
  unsigned char *end_ptr = write_ptr + handle->get_data_size_bytes();
  write_ptr += column->get_start();

  if (bufsize == 4) {
    // Most common case.
    while (write_ptr < end_ptr) {
      write_ptr[0] = buffer[0];
      write_ptr[1] = buffer[1];
      write_ptr[2] = buffer[2];
      write_ptr[3] = buffer[3];
      write_ptr += stride;
    }
  } else {
    while (write_ptr < end_ptr) {
      memcpy(write_ptr, buffer, bufsize);
      write_ptr += stride;
    }
  }
}

/**
 * Quickly copies data without the need to convert it.
 */
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
    // Fantastic!  It's just a linear array of this one data type.  Copy the
    // whole thing all at once.
    memcpy(to, from, num_records * from_type->get_total_bytes());

  } else {
    // Ok, it's interleaved in with other data.  Copy them one record at a
    // time.
    while (num_records > 0) {
      memcpy(to, from, from_type->get_total_bytes());
      to += to_stride;
      from += from_stride;
      num_records--;
    }
  }
}

/**
 * Returns a new GeomVertexData object, suitable for modification, with the
 * indicated data type replaced with a new table filled with undefined values.
 * The new table will be added as a new array; if the old table was
 * interleaved with a previous array, the previous array will not be repacked.
 *
 * If num_components is 0, the indicated name is simply removed from the type,
 * without replacing it with anything else.
 */
PT(GeomVertexData) GeomVertexData::
replace_column(InternalName *name, int num_components,
               GeomVertexData::NumericType numeric_type,
               GeomVertexData::Contents contents) const {
  CDReader cdata(_cycler);
  PT(GeomVertexFormat) new_format = new GeomVertexFormat(*cdata->_format);

  // Remove the old description of the type from the format.
  bool removed_type_array = false;
  int old_type_array = cdata->_format->get_array_with(name);
  if (old_type_array != -1) {
    GeomVertexArrayFormat *array_format = new_format->modify_array(old_type_array);
    if (array_format->get_num_columns() == 1) {
      // Actually, this array didn't have any other data types, so just drop
      // the whole array.
      new_format->remove_array(old_type_array);
      removed_type_array = true;

    } else {
      // Remove the description for the type, but don't bother to repack the
      // array.
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
      << *cdata->_format << " to " << *format << "\n";
  }

  PT(GeomVertexData) new_data = new GeomVertexData(*this, format);

  int j = 0;
  int num_arrays = get_num_arrays();
  for (int i = 0; i < num_arrays; ++i) {
    if (i == old_type_array) {
      if (!removed_type_array) {
        // Pointer-copy the original array that includes the type (since it
        // also includes other data).
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

    // For the new type array, we set up a temporary array that has room for
    // the right number of rows.
    PT(GeomVertexArrayData) new_array = new GeomVertexArrayData
      (format->get_array(j), get_usage_hint());
    new_array->set_num_rows(get_num_rows());
    new_data->set_array(j, new_array);
  }

  return new_data;
}

/**
 *
 */
void GeomVertexData::
output(ostream &out) const {
  if (!get_name().empty()) {
    out << get_name() << " ";
  }
  out << get_num_rows() << " rows: " << *get_format();
}

/**
 *
 */
void GeomVertexData::
write(ostream &out, int indent_level) const {
  if (!get_name().empty()) {
    indent(out, indent_level) << get_name() << "\n";
  }
  get_format()->write_with_data(out, indent_level + 2, this);
  CPT(TransformBlendTable) table = get_transform_blend_table();
  if (table != nullptr) {
    indent(out, indent_level)
      << "Transform blend table:\n";
    table->write(out, indent_level + 2);
  }
}

/**
 * Writes a verbose, human-friendly description of the indicated vertex
 * number.
 */
void GeomVertexData::
describe_vertex(ostream &out, int row) const {
  nassertv_always(row >= 0 && row < get_num_rows());

  out << "Vertex " << row << ":\n";

  GeomVertexReader reader(this);
  reader.set_row_unsafe(row);
  const GeomVertexFormat *format = get_format();

  const TransformBlendTable *tb_table = nullptr;
  if (format->get_animation().get_animation_type() == AT_panda) {
    tb_table = get_transform_blend_table();
  }

  int num_columns = format->get_num_columns();
  for (int ci = 0; ci < num_columns; ++ci) {
    int ai = format->get_array_with(ci);
    const GeomVertexColumn *column = format->get_column(ci);
    reader.set_column(ai, column);

    int num_values = std::min(column->get_num_values(), 4);
    const LVecBase4 &d = reader.get_data4();

    out << "  " << *column->get_name();
    for (int v = 0; v < num_values; v++) {
      out << " " << d[v];
    }
    out << "\n";

    if (column->get_name() == InternalName::get_transform_blend() &&
        tb_table != nullptr) {
      // This is an index into the transform blend table.  Look up the index
      // and report the vertex weighting.
      reader.set_column(ai, column);
      int bi = reader.get_data1i();
      if (bi >= 0 && (size_t)bi < tb_table->get_num_blends()) {
        const TransformBlend &blend = tb_table->get_blend(bi);
        out << "    " << blend << "\n";
      }
    }
  }

  // Also show the raw vertex data, why not?
  out << "\nraw data:\n";
  int num_arrays = format->get_num_arrays();
  for (int ai = 0; ai < num_arrays; ++ai) {
    const GeomVertexArrayData *array = get_array(ai);
    const GeomVertexArrayFormat *aformat = format->get_array(ai);
    nassertv(array != nullptr && aformat != nullptr);
    out << "  " << *aformat << "\n";
    CPT(GeomVertexArrayDataHandle) handle = array->get_handle();
    nassertv(handle != nullptr);
    const unsigned char *data = handle->get_read_pointer(true);
    nassertv(data != nullptr);
    int stride = aformat->get_stride();
    int start = stride * row;
    if (data != nullptr) {
      Datagram dg(data + start, stride);
      dg.dump_hex(out, 4);
    }
  }
}

/**
 * Removes all of the previously-cached results of convert_to().
 *
 * This blows away the entire cache, upstream and downstream the pipeline.
 * Use clear_cache_stage() instead if you only want to blow away the cache at
 * the current stage and upstream.
 */
void GeomVertexData::
clear_cache() {
  LightMutexHolder holder(_cache_lock);
  for (Cache::iterator ci = _cache.begin();
       ci != _cache.end();
       ++ci) {
    CacheEntry *entry = (*ci).second;
    entry->erase();
  }
  _cache.clear();
}

/**
 * Removes all of the previously-cached results of convert_to(), at the
 * current pipeline stage and upstream.  Does not affect the downstream cache.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexData::
clear_cache_stage() {
  LightMutexHolder holder(_cache_lock);
  for (Cache::iterator ci = _cache.begin();
       ci != _cache.end();
       ++ci) {
    CacheEntry *entry = (*ci).second;
    CDCacheWriter cdata(entry->_cycler);
    cdata->_result = nullptr;
  }
}

/**
 * Quickly converts DirectX-style color to OpenGL-style color.
 */
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
    uint32_t dword = *(const uint32_t *)from;
    to[0] = unpack_abcd_b(dword);
    to[1] = unpack_abcd_c(dword);
    to[2] = unpack_abcd_d(dword);
    to[3] = unpack_abcd_a(dword);

    to += to_stride;
    from += from_stride;
    num_records--;
  }
}

/**
 * Quickly converts OpenGL-style color to DirectX-style color.
 */
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
    *(uint32_t *)to = pack_abcd(from[3], from[0], from[1], from[2]);

    to += to_stride;
    from += from_stride;
    num_records--;
  }
}

/**
 * Recomputes the results of computing the vertex animation on the CPU, and
 * applies them to the existing animated_vertices object.
 */
void GeomVertexData::
update_animated_vertices(GeomVertexData::CData *cdata, Thread *current_thread) {
  PStatTimer timer(_char_pcollector, current_thread);

  int num_rows = get_num_rows();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Animating " << num_rows << " vertices for " << get_name()
      << "\n";
  }

  const GeomVertexFormat *orig_format = cdata->_format;
  CPT(GeomVertexFormat) new_format = orig_format;

  if (cdata->_animated_vertices == nullptr) {
    new_format = orig_format->get_post_animated_format();
    cdata->_animated_vertices =
      new GeomVertexData(get_name(), new_format,
                         std::min(get_usage_hint(), UH_dynamic));
  }
  PT(GeomVertexData) new_data = cdata->_animated_vertices;

  // We have to make a complete copy of the data first so we can modify it.
  // If we were clever, we could maybe just figure out the subset of the data
  // that might have changed since last frame, but that's too much trouble
  // (and isn't obviously faster than just copying the whole thing).
  new_data->copy_from(this, true);

  // First, apply all of the morphs.
  CPT(SliderTable) slider_table = cdata->_slider_table;
  if (slider_table != nullptr) {
    PStatTimer timer2(_morphs_pcollector);
    int num_morphs = orig_format->get_num_morphs();
    for (int mi = 0; mi < num_morphs; mi++) {
      CPT(InternalName) slider_name = orig_format->get_morph_slider(mi);

      const SparseArray &sliders = slider_table->find_sliders(slider_name);
      if (!sliders.is_zero()) {
        nassertv(!sliders.is_inverse());
        int num_slider_subranges = sliders.get_num_subranges();
        for (int sni = 0; sni < num_slider_subranges; ++sni) {
          int slider_begin = sliders.get_subrange_begin(sni);
          int slider_end = sliders.get_subrange_end(sni);
          for (int sn = slider_begin; sn < slider_end; ++sn) {
            const VertexSlider *slider = slider_table->get_slider(sn);
            const SparseArray &rows = slider_table->get_slider_rows(sn);
            nassertv(!rows.is_inverse());

            PN_stdfloat slider_value = slider->get_slider();
            if (slider_value != 0.0f) {
              CPT(InternalName) base_name = orig_format->get_morph_base(mi);
              CPT(InternalName) delta_name = orig_format->get_morph_delta(mi);

              GeomVertexRewriter data(new_data, base_name);
              GeomVertexReader delta(this, delta_name);
              int num_subranges = rows.get_num_subranges();

              if (data.get_column()->get_num_values() == 4) {
                if (data.get_column()->has_homogeneous_coord()) {
                  // Scale the delta by the homogeneous coordinate.
                  for (int i = 0; i < num_subranges; ++i) {
                    int begin = rows.get_subrange_begin(i);
                    int end = rows.get_subrange_end(i);
                    data.set_row_unsafe(begin);
                    delta.set_row_unsafe(begin);
                    for (int j = begin; j < end; ++j) {
                      LPoint4 vertex = data.get_data4();
                      LPoint3 d = delta.get_data3();
                      d *= slider_value * vertex[3];
                      data.set_data4(vertex[0] + d[0],
                                      vertex[1] + d[1],
                                      vertex[2] + d[2],
                                      vertex[3]);
                    }
                  }
                } else {
                  // Just apply the four-component delta.
                  for (int i = 0; i < num_subranges; ++i) {
                    int begin = rows.get_subrange_begin(i);
                    int end = rows.get_subrange_end(i);
                    data.set_row_unsafe(begin);
                    delta.set_row_unsafe(begin);
                    for (int j = begin; j < end; ++j) {
                      const LPoint4 &vertex = data.get_data4();
                      LPoint4 d = delta.get_data4();
                      data.set_data4(vertex + d * slider_value);
                    }
                  }
                }
              } else {
                // 3-component or smaller values; don't worry about a
                // homogeneous coordinate.
                for (int i = 0; i < num_subranges; ++i) {
                  int begin = rows.get_subrange_begin(i);
                  int end = rows.get_subrange_end(i);
                  data.set_row_unsafe(begin);
                  delta.set_row_unsafe(begin);
                  for (int j = begin; j < end; ++j) {
                    const LPoint3 &vertex = data.get_data3();
                    LPoint3 d = delta.get_data3();
                    data.set_data3(vertex + d * slider_value);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Then apply the transforms.
  CPT(TransformBlendTable) tb_table = cdata->_transform_blend_table.get_read_pointer(current_thread);
  if (tb_table != nullptr) {
    // Recompute all the blends up front, so we don't have to test each one
    // for staleness at each vertex.
    {
      PStatTimer timer4(_blends_pcollector);
      int num_blends = tb_table->get_num_blends();
      for (int bi = 0; bi < num_blends; bi++) {
        tb_table->get_blend(bi).update_blend(current_thread);
      }
    }

    // Now go through and apply the transforms.
    PStatTimer timer3(_skinning_pcollector);

    const SparseArray &rows = tb_table->get_rows();
    int num_subranges = rows.get_num_subranges();

    int blend_array_index = orig_format->get_array_with(InternalName::get_transform_blend());
    if (blend_array_index < 0) {
      gobj_cat.warning()
        << "Vertex data " << get_name()
        << " has a transform_blend_table, but no transform_blend data.\n";
      return;
    }

    CPT(GeomVertexArrayFormat) blend_array_format = orig_format->get_array(blend_array_index);

    if (blend_array_format->get_stride() == 2 &&
        blend_array_format->get_column(0)->get_component_bytes() == 2) {
      // The blend indices are a table of ushorts.  Optimize this common case.
      CPT(GeomVertexArrayDataHandle) blend_array_handle =
        new GeomVertexArrayDataHandle(cdata->_arrays[blend_array_index].get_read_pointer(current_thread), current_thread);
      const unsigned short *blendt = (const unsigned short *)blend_array_handle->get_read_pointer(true);

      size_t ci;
      for (ci = 0; ci < new_format->get_num_points(); ci++) {
        GeomVertexRewriter data(new_data, new_format->get_point(ci));

        for (int i = 0; i < num_subranges; ++i) {
          int begin = rows.get_subrange_begin(i);
          int end = rows.get_subrange_end(i);
          nassertv(begin < end);

          int first_vertex = begin;
          int first_bi = blendt[first_vertex];

          while (first_vertex < end) {
            // At this point, first_vertex is the first of a series of
            // vertices that shares the blend index first_bi.

            // Scan for the end of this series of vertices--we're looking for
            // the next vertex with a different blend index.
            int next_vertex = first_vertex;
            int next_bi = first_bi;
            ++next_vertex;
            while (next_vertex < end) {
              next_bi = blendt[next_vertex];
              if (next_bi != first_bi) {
                break;
              }
              ++next_vertex;
            }

            // We've just reached the end of the vertices with a matching
            // blend index.  Transform all those vertices as a block.
            LMatrix4 mat;
            tb_table->get_blend(first_bi).get_blend(mat, current_thread);
            new_data->do_transform_point_column(new_format, data, mat, first_vertex, next_vertex);

            first_vertex = next_vertex;
            first_bi = next_bi;
          }
        }
      }

      for (ci = 0; ci < new_format->get_num_vectors(); ci++) {
        GeomVertexRewriter data(new_data, new_format->get_vector(ci));

        for (int i = 0; i < num_subranges; ++i) {
          int begin = rows.get_subrange_begin(i);
          int end = rows.get_subrange_end(i);
          nassertv(begin < end);

          int first_vertex = begin;
          int first_bi = blendt[first_vertex];

          while (first_vertex < end) {
            // At this point, first_vertex is the first of a series of
            // vertices that shares the blend index first_bi.

            // Scan for the end of this series of vertices--we're looking for
            // the next vertex with a different blend index.
            int next_vertex = first_vertex;
            int next_bi = first_bi;
            ++next_vertex;
            while (next_vertex < end) {
              next_bi = blendt[next_vertex];
              if (next_bi != first_bi) {
                break;
              }
              ++next_vertex;
            }

            // We've just reached the end of the vertices with a matching
            // blend index.  Transform all those vertices as a block.
            LMatrix4 mat;
            tb_table->get_blend(first_bi).get_blend(mat, current_thread);
            new_data->do_transform_vector_column(new_format, data, mat, first_vertex, next_vertex);

            first_vertex = next_vertex;
            first_bi = next_bi;
          }
        }
      }

    } else {
      // The blend indices are anything else.  Use the GeomVertexReader to
      // iterate through them.
      GeomVertexReader blendi(this, InternalName::get_transform_blend());
      nassertv(blendi.has_column());

      size_t ci;
      for (ci = 0; ci < new_format->get_num_points(); ci++) {
        GeomVertexRewriter data(new_data, new_format->get_point(ci));

        for (int i = 0; i < num_subranges; ++i) {
          int begin = rows.get_subrange_begin(i);
          int end = rows.get_subrange_end(i);
          nassertv(begin < end);
          blendi.set_row_unsafe(begin);

          int first_vertex = begin;
          int first_bi = blendi.get_data1i();

          while (first_vertex < end) {
            // At this point, first_vertex is the first of a series of
            // vertices that shares the blend index first_bi.

            // Scan for the end of this series of vertices--we're looking for
            // the next vertex with a different blend index.
            int next_vertex = first_vertex;
            int next_bi = first_bi;
            ++next_vertex;
            while (next_vertex < end) {
              next_bi = blendi.get_data1i();
              if (next_bi != first_bi) {
                break;
              }
              ++next_vertex;
            }

            // We've just reached the end of the vertices with a matching
            // blend index.  Transform all those vertices as a block.
            LMatrix4 mat;
            tb_table->get_blend(first_bi).get_blend(mat, current_thread);
            new_data->do_transform_point_column(new_format, data, mat, first_vertex, next_vertex);

            first_vertex = next_vertex;
            first_bi = next_bi;
          }
        }
      }

      for (ci = 0; ci < new_format->get_num_vectors(); ci++) {
        GeomVertexRewriter data(new_data, new_format->get_vector(ci));

        for (int i = 0; i < num_subranges; ++i) {
          int begin = rows.get_subrange_begin(i);
          int end = rows.get_subrange_end(i);
          nassertv(begin != end);
          blendi.set_row_unsafe(begin);

          int first_vertex = begin;
          int first_bi = blendi.get_data1i();

          while (first_vertex < end) {
            // At this point, first_vertex is the first of a series of
            // vertices that shares the blend index first_bi.

            // Scan for the end of this series of vertices--we're looking for
            // the next vertex with a different blend index.
            int next_vertex = first_vertex;
            int next_bi = first_bi;
            ++next_vertex;
            while (next_vertex < end) {
              next_bi = blendi.get_data1i();
              if (next_bi != first_bi) {
                break;
              }
              ++next_vertex;
            }

            // We've just reached the end of the vertices with a matching
            // blend index.  Transform all those vertices as a block.
            LMatrix4 mat;
            tb_table->get_blend(first_bi).get_blend(mat, current_thread);
            new_data->do_transform_vector_column(new_format, data, mat, first_vertex, next_vertex);

            first_vertex = next_vertex;
            first_bi = next_bi;
          }
        }
      }
    }
  }
}


/**
 * Transforms a range of vertices for one particular column, as a point.
 */
void GeomVertexData::
do_transform_point_column(const GeomVertexFormat *format, GeomVertexRewriter &data,
                          const LMatrix4 &mat, int begin_row, int end_row) {
  const GeomVertexColumn *data_column = data.get_column();
  int num_values = data_column->get_num_values();

  if ((num_values == 3 || num_values == 4) &&
      data_column->get_numeric_type() == NT_float32) {
    // The table of points is a table of LPoint3f's or LPoint4f's.  Optimize
    // this common case.
    GeomVertexArrayDataHandle *data_handle = data.get_array_handle();

    size_t stride = data.get_stride();
    size_t num_rows = end_row - begin_row;
    unsigned char *datat = data_handle->get_write_pointer();
    datat += data_column->get_start() + begin_row * stride;
    LMatrix4f matf = LCAST(float, mat);

    if (num_values == 3) {
      table_xform_point3f(datat, num_rows, stride, matf);
    } else {
      table_xform_vecbase4f(datat, num_rows, stride, matf);
    }

  } else if (num_values == 4) {
    // Use the GeomVertexRewriter to adjust the 4-component points.

    data.set_row_unsafe(begin_row);
    for (int j = begin_row; j < end_row; ++j) {
      LPoint4 vertex = data.get_data4();
      data.set_data4(vertex * mat);
    }

  } else {
    // Use the GeomVertexRewriter to adjust the 3-component points.

    data.set_row_unsafe(begin_row);
    for (int j = begin_row; j < end_row; ++j) {
      LPoint3 vertex = data.get_data3();
      data.set_data3(vertex * mat);
    }
  }
}

/**
 * Transforms a range of vertices for one particular column, as a vector.
 */
void GeomVertexData::
do_transform_vector_column(const GeomVertexFormat *format, GeomVertexRewriter &data,
                           const LMatrix4 &mat, int begin_row, int end_row) {
  const GeomVertexColumn *data_column = data.get_column();
  int num_values = data_column->get_num_values();

  LMatrix4 xform;
  bool normalize = false;
  if (data_column->get_contents() == C_normal) {
    // This is to preserve perpendicularity to the surface.
    LVecBase3 scale_sq(mat.get_row3(0).length_squared(),
                       mat.get_row3(1).length_squared(),
                       mat.get_row3(2).length_squared());
    if (IS_THRESHOLD_EQUAL(scale_sq[0], scale_sq[1], 2.0e-3f) &&
        IS_THRESHOLD_EQUAL(scale_sq[0], scale_sq[2], 2.0e-3f)) {
      // There is a uniform scale.
      LVecBase3 scale, shear, hpr;
      if (IS_THRESHOLD_EQUAL(scale_sq[0], 1, 2.0e-3f)) {
        // No scale to worry about.
        xform = mat;
      } else if (decompose_matrix(mat.get_upper_3(), scale, shear, hpr)) {
        // Make a new matrix with scale/translate taken out of the equation.
        compose_matrix(xform, LVecBase3(1, 1, 1), shear, hpr, LVecBase3::zero());
      } else {
        normalize = true;
      }
    } else {
      // There is a non-uniform scale, so we need to do all this to preserve
      // orthogonality to the surface.
      xform.invert_from(mat);
      xform.transpose_in_place();
      normalize = true;
    }
  } else {
    xform = mat;
  }

  if ((num_values == 3 || num_values == 4) &&
      data_column->get_numeric_type() == NT_float32) {
    // The table of vectors is a table of LVector3f's or LVector4f's.
    // Optimize this common case.
    GeomVertexArrayDataHandle *data_handle = data.get_array_handle();

    size_t stride = data.get_stride();
    size_t num_rows = end_row - begin_row;
    unsigned char *datat = data_handle->get_write_pointer();
    datat += data_column->get_start() + begin_row * stride;
    LMatrix4f matf = LCAST(float, xform);

    if (normalize) {
      table_xform_normal3f(datat, num_rows, stride, matf);
    } else if (num_values == 3) {
      table_xform_vector3f(datat, num_rows, stride, matf);
    } else {
      table_xform_vecbase4f(datat, num_rows, stride, matf);
    }

  } else {
    // Use the GeomVertexRewriter to transform the vectors.
    data.set_row_unsafe(begin_row);

    if (normalize) {
      for (int j = begin_row; j < end_row; ++j) {
        LVector3 vector = data.get_data3();
        vector *= xform;
        vector.normalize();
        data.set_data3(vector);
      }
    } else {
      for (int j = begin_row; j < end_row; ++j) {
        LVector3 vector = data.get_data3();
        data.set_data3(vector * xform);
      }
    }
  }
}

/**
 * Transforms each of the LPoint3f objects in the indicated table by the
 * indicated matrix.
 */
void GeomVertexData::
table_xform_point3f(unsigned char *datat, size_t num_rows, size_t stride,
                    const LMatrix4f &matf) {
  // We don't bother checking for the unaligned case here, because in practice
  // it doesn't matter with a 3-component point.
  for (size_t i = 0; i < num_rows; ++i) {
    LPoint3f &vertex = *(LPoint3f *)(&datat[i * stride]);
    vertex *= matf;
  }
}

/**
 * Transforms each of the LVector3f objects in the indicated table by the
 * indicated matrix, and also normalizes them.
 */
void GeomVertexData::
table_xform_normal3f(unsigned char *datat, size_t num_rows, size_t stride,
                     const LMatrix4f &matf) {
  // We don't bother checking for the unaligned case here, because in practice
  // it doesn't matter with a 3-component vector.
  for (size_t i = 0; i < num_rows; ++i) {
    LNormalf &vertex = *(LNormalf *)(&datat[i * stride]);
    vertex *= matf;
    vertex.normalize();
  }
}

/**
 * Transforms each of the LVector3f objects in the indicated table by the
 * indicated matrix.
 */
void GeomVertexData::
table_xform_vector3f(unsigned char *datat, size_t num_rows, size_t stride,
                     const LMatrix4f &matf) {
  // We don't bother checking for the unaligned case here, because in practice
  // it doesn't matter with a 3-component vector.
  for (size_t i = 0; i < num_rows; ++i) {
    LVector3f &vertex = *(LVector3f *)(&datat[i * stride]);
    vertex *= matf;
  }
}

/**
 * Transforms each of the LVecBase4f objects in the indicated table by the
 * indicated matrix.
 */
void GeomVertexData::
table_xform_vecbase4f(unsigned char *datat, size_t num_rows, size_t stride,
                      const LMatrix4f &matf) {
#if defined(HAVE_EIGEN) && defined(LINMATH_ALIGN)
  // Check if the table is unaligned.  If it is, we can't use the LVecBase4f
  // object directly, which assumes 16-byte alignment.
  if (((size_t)datat & 0xf) != 0 || (stride & 0xf) != 0) {
    // Instead, we'll use low-level Eigen calls to multiply out the unaligned
    // memory.
    Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, 4, Eigen::RowMajor>, Eigen::Unaligned, Eigen::OuterStride<> > table((float *)datat, num_rows, 4, Eigen::OuterStride<>(stride / sizeof(float)));
    for (size_t i = 0; i < num_rows; ++i) {
      table.row(i) *= matf._m;
    }
    return;
  }
#endif  // HAVE_EIGEN

  // If the table is properly aligned (or we don't require alignment), we can
  // directly use the high-level LVecBase4f object, which will do the right
  // thing.
  for (size_t i = 0; i < num_rows; ++i) {
    LVecBase4f &vertex = *(LVecBase4f *)(&datat[i * stride]);
    vertex *= matf;
  }
}

/**
 * Tells the BamReader how to create objects of type GeomVertexData.
 */
void GeomVertexData::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void GeomVertexData::
write_datagram(BamWriter *manager, Datagram &dg) {
  CopyOnWriteObject::write_datagram(manager, dg);

  dg.add_string(_name);
  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type GeomVertexData is encountered in the Bam file.  It should create the
 * GeomVertexData and extract its information from the file.
 */
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

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int GeomVertexData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CopyOnWriteObject::complete_pointers(p_list, manager);
  return pi;
}

/**
 * Some objects require all of their nested pointers to have been completed
 * before the objects themselves can be completed.  If this is the case,
 * override this method to return true, and be careful with circular
 * references (which would make the object unreadable from a bam file).
 */
bool GeomVertexData::
require_fully_complete() const {
  return true;
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void GeomVertexData::
finalize(BamReader *manager) {
  // NOTE: This method may be called more than once, because the
  // Geom::finalize() will call it explicitly.  We have to be prepared to
  // accept multiple finalize() calls.

  // Now we need to register the format that we have read from the bam file
  // (since it doesn't come out of the bam file automatically registered).
  // This may change the format's pointer, which we should then update our own
  // data to reflect.  But since this may cause the unregistered object to
  // destruct, we have to also tell the BamReader to return the new object
  // from now on.

  // This extends to the nested array datas, as well as the transform table
  // and slider tables, as well.

  CDWriter cdata(_cycler, true);

  for (size_t i = 0; i < cdata->_arrays.size(); ++i) {
    CPT(GeomVertexFormat) new_format =
      GeomVertexFormat::register_format(cdata->_format);
    manager->change_pointer(cdata->_format, new_format);
    cdata->_format = new_format;

    CPT(GeomVertexArrayFormat) new_array_format = new_format->get_array(i);
    PT(GeomVertexArrayData) array_obj = cdata->_arrays[i].get_unsafe_pointer();
    nassertv(new_array_format->is_data_subset_of(*array_obj->_array_format));

    manager->change_pointer(array_obj->_array_format, new_array_format);
    array_obj->_array_format = new_array_format;
  }

  if (cdata->_transform_table != nullptr) {
    CPT(TransformTable) new_transform_table =
      TransformTable::register_table(cdata->_transform_table);
    manager->change_pointer(cdata->_transform_table, new_transform_table);
    cdata->_transform_table = new_transform_table;
  }

  if (cdata->_slider_table != nullptr) {
    CPT(SliderTable) new_slider_table =
      SliderTable::register_table(cdata->_slider_table);
    manager->change_pointer(cdata->_slider_table, new_slider_table);
    cdata->_slider_table = new_slider_table;
  }
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new GeomVertexData.
 */
void GeomVertexData::
fillin(DatagramIterator &scan, BamReader *manager) {
  CopyOnWriteObject::fillin(scan, manager);

  set_name(scan.get_string());
  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
CycleData *GeomVertexData::CDataCache::
make_copy() const {
  return new CDataCache(*this);
}

/**
 * Called when the entry is evicted from the cache, this should clean up the
 * owning object appropriately.
 */
void GeomVertexData::CacheEntry::
evict_callback() {
  LightMutexHolder holder(_source->_cache_lock);
  Cache::iterator ci = _source->_cache.find(&_key);
  nassertv(ci != _source->_cache.end());
  nassertv((*ci).second == this);
  _source->_cache.erase(ci);
}

/**
 *
 */
void GeomVertexData::CacheEntry::
output(ostream &out) const {
  out << "vertex data " << (void *)_source << " to "
      << *_key._modifier;
}

/**
 *
 */
CycleData *GeomVertexData::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void GeomVertexData::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  manager->write_pointer(dg, _format);
  dg.add_uint8(_usage_hint);

  dg.add_uint16(_arrays.size());
  Arrays::const_iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    manager->write_pointer(dg, (*ai).get_read_pointer());
  }

  manager->write_pointer(dg, _transform_table);
  manager->write_pointer(dg, _transform_blend_table.get_read_pointer());
  manager->write_pointer(dg, _slider_table);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int GeomVertexData::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  _format = DCAST(GeomVertexFormat, p_list[pi++]);

  Arrays::iterator ai;
  for (ai = _arrays.begin(); ai != _arrays.end(); ++ai) {
    (*ai) = DCAST(GeomVertexArrayData, p_list[pi++]);
  }

  _transform_table = DCAST(TransformTable, p_list[pi++]);
  _transform_blend_table = DCAST(TransformBlendTable, p_list[pi++]);
  _slider_table = DCAST(SliderTable, p_list[pi++]);

  _modified = Geom::get_next_modified();

  if (!_arrays.empty() && manager->get_file_minor_ver() < 7) {
    // Bam files prior to 6.7 did not store a SparseArray in the SliderTable
    // or TransformBlendTable entries.  We need to make up a SparseArray for
    // each of them that reflects the complete number of rows in the data.
    SparseArray all_rows;
    CPT(GeomVertexArrayData) adata = _arrays[0].get_read_pointer();
    all_rows.set_range(0, adata->get_num_rows());

    if (_slider_table != nullptr) {
      int num_sliders = _slider_table->get_num_sliders();
      for (int i = 0; i < num_sliders; ++i) {
        ((SliderTable *)_slider_table.p())->set_slider_rows(i, all_rows);
      }
    }
    if (!_transform_blend_table.is_null()) {
      _transform_blend_table.get_unsafe_pointer()->set_rows(all_rows);
    }
  }

  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new GeomVertexData.
 */
void GeomVertexData::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan);
  _usage_hint = (UsageHint)scan.get_uint8();

  size_t num_arrays = scan.get_uint16();
  _arrays.reserve(num_arrays);
  for (size_t i = 0; i < num_arrays; ++i) {
    manager->read_pointer(scan);
    _arrays.push_back(nullptr);
  }

  manager->read_pointer(scan);
  manager->read_pointer(scan);
  manager->read_pointer(scan);
}

/**
 *
 */
int GeomVertexDataPipelineBase::
get_num_bytes() const {
  int num_bytes = sizeof(GeomVertexData);

  GeomVertexData::Arrays::const_iterator ai;
  for (ai = _cdata->_arrays.begin(); ai != _cdata->_arrays.end(); ++ai) {
    num_bytes += (*ai).get_read_pointer()->get_data_size_bytes();
  }

  return num_bytes;
}

/**
 *
 */
int GeomVertexDataPipelineReader::
get_num_rows() const {
  nassertr(_cdata->_format->get_num_arrays() == _cdata->_arrays.size(), 0);
  nassertr(_got_array_readers, 0);

  if (_cdata->_format->get_num_arrays() == 0) {
    // No arrays means no rows.  Weird but legal.
    return 0;
  }

  // Look up the answer on the first array (since any array will do).
  int stride = _cdata->_format->get_array(0)->get_stride();
  return _array_readers[0]->get_data_size_bytes() / stride;
}

/**
 *
 */
bool GeomVertexDataPipelineReader::
get_array_info(const InternalName *name,
               const GeomVertexArrayDataHandle *&array_reader,
               int &num_values,
               GeomVertexDataPipelineReader::NumericType &numeric_type,
               int &start, int &stride) const {
  nassertr(_got_array_readers, false);
  int array_index;
  const GeomVertexColumn *column;
  if (_cdata->_format->get_array_info(name, array_index, column)) {
    array_reader = _array_readers[array_index];
    num_values = column->get_num_values();
    numeric_type = column->get_numeric_type();
    start = column->get_start();
    stride = _cdata->_format->get_array(array_index)->get_stride();
    return true;
  }
  return false;
}

/**
 *
 */
bool GeomVertexDataPipelineReader::
get_array_info(const InternalName *name,
               const GeomVertexArrayDataHandle *&array_reader,
               int &num_values,
               GeomVertexDataPipelineReader::NumericType &numeric_type,
               bool &normalized, int &start, int &stride, int &divisor,
               int &num_elements, int &element_stride) const {
  nassertr(_got_array_readers, false);
  int array_index;
  const GeomVertexColumn *column;
  if (_cdata->_format->get_array_info(name, array_index, column)) {
    array_reader = _array_readers[array_index];
    num_values = column->get_num_values();
    numeric_type = column->get_numeric_type();
    normalized = (column->get_contents() == GeomEnums::C_color);
    start = column->get_start();
    stride = _cdata->_format->get_array(array_index)->get_stride();
    divisor = _cdata->_format->get_array(array_index)->get_divisor();
    num_elements = column->get_num_elements();
    element_stride = column->get_element_stride();
    return true;
  }
  return false;
}

/**
 *
 */
bool GeomVertexDataPipelineReader::
get_vertex_info(const GeomVertexArrayDataHandle *&array_reader,
                int &num_values,
                GeomVertexDataPipelineReader::NumericType &numeric_type,
                int &start, int &stride) const {
  nassertr(_got_array_readers, false);
  int array_index = _cdata->_format->get_vertex_array_index();
  if (array_index >= 0) {
    const GeomVertexColumn *column = _cdata->_format->get_vertex_column();

    array_reader = _array_readers[array_index];
    num_values = column->get_num_values();
    numeric_type = column->get_numeric_type();
    start = column->get_start();
    stride = _cdata->_format->get_array(array_index)->get_stride();
    return true;
  }
  return false;
}

/**
 *
 */
bool GeomVertexDataPipelineReader::
get_normal_info(const GeomVertexArrayDataHandle *&array_reader,
                GeomVertexDataPipelineReader::NumericType &numeric_type,
                int &start, int &stride) const {
  nassertr(_got_array_readers, false);
  int array_index = _cdata->_format->get_normal_array_index();
  if (array_index >= 0) {
    const GeomVertexColumn *column = _cdata->_format->get_normal_column();

    array_reader = _array_readers[array_index];
    numeric_type = column->get_numeric_type();
    start = column->get_start();
    stride = _cdata->_format->get_array(array_index)->get_stride();
    return true;
  }
  return false;
}

/**
 *
 */
bool GeomVertexDataPipelineReader::
get_color_info(const GeomVertexArrayDataHandle *&array_reader,
               int &num_values,
               GeomVertexDataPipelineReader::NumericType &numeric_type,
               int &start, int &stride) const {
  nassertr(_got_array_readers, false);
  int array_index = _cdata->_format->get_color_array_index();
  if (array_index >= 0) {
    const GeomVertexColumn *column = _cdata->_format->get_color_column();

    array_reader = _array_readers[array_index];
    num_values = column->get_num_values();
    numeric_type = column->get_numeric_type();
    start = column->get_start();
    stride = _cdata->_format->get_array(array_index)->get_stride();
    return true;
  }
  return false;
}

/**
 *
 */
void GeomVertexDataPipelineReader::
make_array_readers() {
  nassertv(!_got_array_readers);

  _array_readers.reserve(_cdata->_arrays.size());
  GeomVertexData::Arrays::const_iterator ai;
  for (ai = _cdata->_arrays.begin(); ai != _cdata->_arrays.end(); ++ai) {
    _array_readers.push_back(new GeomVertexArrayDataHandle((*ai).get_read_pointer(_current_thread), _current_thread));
  }

  _got_array_readers = true;
}

/**
 *
 */
int GeomVertexDataPipelineWriter::
get_num_rows() const {
  nassertr(_cdata->_format->get_num_arrays() == _cdata->_arrays.size(), 0);
  nassertr(_got_array_writers, 0);

  if (_cdata->_format->get_num_arrays() == 0) {
    // No arrays means no rows.  Weird but legal.
    return 0;
  }

  // Look up the answer on the first array (since any array will do).
  int stride = _cdata->_format->get_array(0)->get_stride();
  return _array_writers[0]->get_data_size_bytes() / stride;
}

/**
 *
 */
bool GeomVertexDataPipelineWriter::
set_num_rows(int n) {
  nassertr(_got_array_writers, false);
  nassertr(_cdata->_format->get_num_arrays() == _cdata->_arrays.size(), false);

  bool any_changed = false;

  int color_array = -1;
  int orig_color_rows = -1;

  for (size_t i = 0; i < _cdata->_arrays.size(); i++) {
    if (_array_writers[i]->get_num_rows() != n) {
      if (_array_writers[i]->get_object()->has_column(InternalName::get_color())) {
        color_array = i;
        orig_color_rows = _array_writers[i]->get_num_rows();
      }
      _array_writers[i]->set_num_rows(n);
      any_changed = true;
    }
  }

  if (color_array >= 0 && orig_color_rows < n) {
    // We have just added some rows; fill the "color" column with (1, 1, 1,
    // 1), for the programmer's convenience.
    GeomVertexArrayDataHandle *array_writer = _array_writers[color_array];
    const GeomVertexArrayFormat *array_format = array_writer->get_array_format();
    const GeomVertexColumn *column =
      array_format->get_column(InternalName::get_color());
    int stride = array_format->get_stride();
    unsigned char *start =
      array_writer->get_write_pointer() + column->get_start();
    unsigned char *stop = start + array_writer->get_data_size_bytes();
    unsigned char *pointer = start + stride * orig_color_rows;
    int num_values = column->get_num_values();

    switch (column->get_numeric_type()) {
    case NT_uint8:
    case NT_uint16:
    case NT_uint32:
    case NT_packed_dcba:
    case NT_packed_dabc:
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

    case NT_float64:
      while (pointer < stop) {
        PN_float64 *pi = (PN_float64 *)pointer;
        for (int i = 0; i < num_values; i++) {
          pi[i] = 1.0;
        }
        pointer += stride;
      }
      break;

    case NT_stdfloat:
    case NT_int8:
    case NT_int16:
    case NT_int32:
      // Shouldn't have this type in the format.
      nassertr(false, false);
      break;

    case NT_packed_ufloat:
      while (pointer < stop) {
        *(int32_t *)pointer = 0x781e03c0;
        pointer += stride;
      }
      break;
    }
  }

  if (any_changed) {
    _object->clear_cache_stage();
    _cdata->_modified = Geom::get_next_modified();
    _cdata->_animated_vertices.clear();
  }

  return any_changed;
}

/**
 *
 */
bool GeomVertexDataPipelineWriter::
unclean_set_num_rows(int n) {
  nassertr(_got_array_writers, false);
  nassertr(_cdata->_format->get_num_arrays() == _cdata->_arrays.size(), false);

  bool any_changed = false;

  for (size_t i = 0; i < _cdata->_arrays.size(); i++) {
    if (_array_writers[i]->get_num_rows() != n) {
      if (_array_writers[i]->unclean_set_num_rows(n)) {
        any_changed = true;
      }
    }
  }

  if (any_changed) {
    _object->clear_cache_stage();
    _cdata->_modified = Geom::get_next_modified();
    _cdata->_animated_vertices.clear();
  }

  return any_changed;
}

/**
 *
 */
bool GeomVertexDataPipelineWriter::
reserve_num_rows(int n) {
  nassertr(_got_array_writers, false);
  nassertr(_cdata->_format->get_num_arrays() == _cdata->_arrays.size(), false);

  bool any_changed = false;

  for (size_t i = 0; i < _cdata->_arrays.size(); i++) {
    if (_array_writers[i]->reserve_num_rows(n)) {
      any_changed = true;
    }
  }

  return any_changed;
}

/**
 *
 */
PT(GeomVertexArrayData) GeomVertexDataPipelineWriter::
modify_array(size_t i) {
  nassertr(i < _cdata->_arrays.size(), nullptr);

  PT(GeomVertexArrayData) new_data;
  if (_got_array_writers) {
    new_data = _array_writers[i]->get_object();
  } else {
    new_data = _cdata->_arrays[i].get_write_pointer();
  }

  _object->clear_cache_stage();
  _cdata->_modified = Geom::get_next_modified();
  _cdata->_animated_vertices_modified = UpdateSeq();

  return new_data;
}

/**
 *
 */
void GeomVertexDataPipelineWriter::
set_array(size_t i, const GeomVertexArrayData *array) {
  nassertv(i < _cdata->_arrays.size());
  _cdata->_arrays[i] = (GeomVertexArrayData *)array;
  _object->clear_cache_stage();
  _cdata->_modified = Geom::get_next_modified();
  _cdata->_animated_vertices_modified = UpdateSeq();

  if (_got_array_writers) {
    _array_writers[i] = new GeomVertexArrayDataHandle(_cdata->_arrays[i].get_write_pointer(), _current_thread);
  }
}

/**
 * Copies a single row of the data from the other array into the indicated row
 * of this array.  In this case, the source format must exactly match the
 * destination format.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexDataPipelineWriter::
copy_row_from(int dest_row, const GeomVertexDataPipelineReader &source,
              int source_row) {
  const GeomVertexFormat *source_format = source.get_format();
  const GeomVertexFormat *dest_format = get_format();
  nassertv(source_format == dest_format);
  nassertv(source_row >= 0 && source_row < source.get_num_rows());
  nassertv(_got_array_writers);

  if (dest_row >= get_num_rows()) {
    // Implicitly add enough rows to get to the indicated row.
    set_num_rows(dest_row + 1);
  }

  size_t num_arrays = source_format->get_num_arrays();
  for (size_t i = 0; i < num_arrays; ++i) {
    GeomVertexArrayDataHandle *dest_handle = get_array_writer(i);
    unsigned char *dest_array_data = dest_handle->get_write_pointer();

    const GeomVertexArrayDataHandle *source_array_handle = source.get_array_reader(i);
    const unsigned char *source_array_data = source_array_handle->get_read_pointer(true);

    const GeomVertexArrayFormat *array_format = source_format->get_array(i);
    int stride = array_format->get_stride();

    memcpy(dest_array_data + stride * dest_row,
           source_array_data + stride * source_row,
           stride);
  }
}

/**
 *
 */
void GeomVertexDataPipelineWriter::
make_array_writers() {
  nassertv(!_got_array_writers);

  _array_writers.reserve(_cdata->_arrays.size());
  GeomVertexData::Arrays::iterator ai;
  for (ai = _cdata->_arrays.begin(); ai != _cdata->_arrays.end(); ++ai) {
    _array_writers.push_back(new GeomVertexArrayDataHandle((*ai).get_write_pointer(), _current_thread));
  }

  _object->clear_cache_stage();
  _cdata->_modified = Geom::get_next_modified();
  _cdata->_animated_vertices_modified = UpdateSeq();

  _got_array_writers = true;
}

/**
 *
 */
void GeomVertexDataPipelineWriter::
delete_array_writers() {
  nassertv(_got_array_writers);

  _array_writers.clear();
  _got_array_writers = false;
}
