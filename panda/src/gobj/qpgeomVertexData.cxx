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
#include "qpgeomVertexCacheManager.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pset.h"

TypeHandle qpGeomVertexData::_type_handle;

PStatCollector qpGeomVertexData::_munge_data_pcollector("Cull:Munge:Data");

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::Default Constructor
//       Access: Private
//  Description: Constructs an invalid object.  This is only used when
//               reading from the bam file.
////////////////////////////////////////////////////////////////////
qpGeomVertexData::
qpGeomVertexData() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexData::
qpGeomVertexData(const qpGeomVertexFormat *format,
                 qpGeomVertexArrayData::UsageHint usage_hint) :
  _format(format)
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
  _format(copy._format),
  _cycler(copy._cycler)  
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
  _format = copy._format;
  _cycler = copy._cycler;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexData::
~qpGeomVertexData() {
  // When we destruct, we should ensure that all of our cached
  // entries, across all pipeline stages, are properly removed from
  // the cache manager.
  qpGeomVertexCacheManager *cache_mgr = 
    qpGeomVertexCacheManager::get_global_ptr();

  int num_stages = _cycler.get_num_stages();
  for (int i = 0; i < num_stages; i++) {
    if (_cycler.is_stage_unique(i)) {
      CData *cdata = _cycler.write_stage(i);
      for (ConvertedCache::iterator ci = cdata->_converted_cache.begin();
           ci != cdata->_converted_cache.end();
           ++ci) {
        cache_mgr->remove_data(this, (*ci).first);
      }
      cdata->_converted_cache.clear();
      _cycler.release_write_stage(i, cdata);
    }
  }
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
  return cdata->_arrays[0]->get_num_bytes() / stride;
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
  clear_cache();
  CDWriter cdata(_cycler);
  nassertv(_format->get_num_arrays() == (int)cdata->_arrays.size());

  Arrays::iterator ai;
  for (ai = cdata->_arrays.begin();
       ai != cdata->_arrays.end();
       ++ai) {
    (*ai)->clear_vertices();
  }
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
  clear_cache();
  CDWriter cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_arrays.size(), NULL);

  if (cdata->_arrays[i]->get_ref_count() > 1) {
    cdata->_arrays[i] = new qpGeomVertexArrayData(*cdata->_arrays[i]);
  }

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
  clear_cache();
  CDWriter cdata(_cycler);
  nassertv(i >= 0 && i < (int)cdata->_arrays.size());
  cdata->_arrays[i] = (qpGeomVertexArrayData *)array;
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
    num_bytes += (*ai)->get_num_bytes();
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

  // Look up the format in our cache--maybe we've recently converted
  // to the requested format.
  {
    // Use read() and release_read() instead of CDReader, because the
    // call to record_data() might recursively call back into this
    // object, and require a write.
    const CData *cdata = _cycler.read();
    ConvertedCache::const_iterator ci = 
      cdata->_converted_cache.find(new_format);
    if (ci != cdata->_converted_cache.end()) {
      _cycler.release_read(cdata);
      // Record a cache hit, so this element will stay in the cache a
      // while longer.
      qpGeomVertexCacheManager *cache_mgr = 
        qpGeomVertexCacheManager::get_global_ptr();
      cache_mgr->record_data(this, new_format, (*ci).second->get_num_bytes());

      return (*ci).second;
    }
    _cycler.release_read(cdata);
  }

  // Okay, convert the data to the new format.
  int num_vertices = get_num_vertices();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Converting " << num_vertices << " vertices.\n";
  }
  PStatTimer timer(_munge_data_pcollector);

  PT(qpGeomVertexData) new_data = 
    new qpGeomVertexData(new_format, qpGeomVertexArrayData::UH_client);

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
    CPTA_uchar data = get_array(i)->get_data();
    const qpGeomVertexArrayFormat *array_format = _format->get_array(i);
    int num_data_types = array_format->get_num_data_types();
    for (int di = 0; di < num_data_types; ++di) {
      const qpGeomVertexDataType *data_type = array_format->get_data_type(di);

      int new_i = new_format->get_array_with(data_type->get_name());
      if (new_i >= 0 && done_arrays.count(new_i) == 0) {
        // The data type exists in the new format; we have to copy it.
        PT(qpGeomVertexArrayData) new_array_data = new
          qpGeomVertexArrayData(*get_array(i));
        new_data->set_array(new_i, new_array_data);
        PTA_uchar new_data = new_array_data->modify_data();

        const qpGeomVertexArrayFormat *new_array_format = 
          new_format->get_array(new_i);
        const qpGeomVertexDataType *new_data_type = 
          new_array_format->get_data_type(data_type->get_name());

        new_data_type->copy_records
          (new_data + new_data_type->get_start(), 
           new_array_format->get_stride(),
           data + data_type->get_start(), array_format->get_stride(),
           data_type, num_vertices);
      }
    }
  }

  // Record the new result in the cache.
  {
    CDWriter cdata(((qpGeomVertexData *)this)->_cycler);
    cdata->_converted_cache[new_format] = new_data;
  }

  // And tell the cache manager about the new entry.  (It might
  // immediately request a delete from the cache of the thing we just
  // added.)
  qpGeomVertexCacheManager *cache_mgr = 
    qpGeomVertexCacheManager::get_global_ptr();
  cache_mgr->record_data(this, new_format, new_data->get_num_bytes());

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
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::clear_cache
//       Access: Published
//  Description: Removes all of the previously-cached results of
//               convert_to().
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
clear_cache() {
  // Probably we shouldn't do anything at all here unless we are
  // running in pipeline stage 0.
  qpGeomVertexCacheManager *cache_mgr = 
    qpGeomVertexCacheManager::get_global_ptr();

  CData *cdata = CDWriter(_cycler);
  for (ConvertedCache::iterator ci = cdata->_converted_cache.begin();
       ci != cdata->_converted_cache.end();
       ++ci) {
    cache_mgr->remove_data(this, (*ci).first);
  }
  cdata->_converted_cache.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::set_data
//       Access: Public
//  Description: Sets the nth vertex to a particular value.  Query the
//               format to get the array index and data_type
//               parameters for the particular data type you want to
//               set.
//
//               This flavor of set_data() accepts a generic float
//               array and a specific number of dimensions.  The new
//               data will be copied from the num_values elements
//               of data.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
set_data(int array, const qpGeomVertexDataType *data_type,
         int vertex, const float *data, int num_values) {
  int stride = _format->get_array(array)->get_stride();
  int element = vertex * stride + data_type->get_start();

  {
    CDReader cdata(_cycler);
    int array_size = (int)cdata->_arrays[array]->get_num_bytes();
    if (element + data_type->get_total_bytes() > array_size) {
      // Whoops, we need more vertices!
      CDWriter cdataw(_cycler, cdata);
      do_set_num_vertices(vertex + 1, cdataw);
    }
  }

  PTA_uchar array_data = modify_array(array)->modify_data();
  nassertv(element >= 0 && element + data_type->get_total_bytes() <= (int)array_data.size());

  switch (data_type->get_numeric_type()) {
  case qpGeomVertexDataType::NT_uint8:
    {
      int i = 0;
      int min_values = min(num_values, data_type->get_num_values());
      while (i < min_values) {
        int value = (int)(data[i] * 255.0f);
        array_data[element] = value;
        element += 1;
        ++i;
      }
      while (i < data_type->get_num_values()) {
        array_data[element] = 0;
        element += 1;
        ++i;
      }        
    }
    break;

  case qpGeomVertexDataType::NT_packed_argb:
    {
      if (num_values == 4) {
        *(PN_uint32 *)&array_data[element] = pack_argb(data);
      } else {
        // Elevate (or truncate) to 4 components.
        float data4[4];
        memset(data4, 0, 4 * sizeof(float));
        memcpy(data4, data, min(4, num_values) * sizeof(float));
        *(PN_uint32 *)&array_data[element] = pack_argb(data4);
      }
    }
    break;

  case qpGeomVertexDataType::NT_float:
    if (num_values == 4 && sizeof(float) == sizeof(PN_float32)) {
      // The easy way: we can memcpy the data directly in.
      memcpy(&array_data[element], data, data_type->get_total_bytes());

    } else {
      // Elevate or truncate to the right number of components.
      int i = 0;
      int min_values = min(num_values, data_type->get_num_values());
      while (i < min_values) {
        *(PN_float32 *)&array_data[element] = data[i];
        element += sizeof(PN_float32);
        ++i;
      }
      while (i < data_type->get_num_values()) {
        if (i == 3 && data_type->get_num_values() == 4) {
          *(PN_float32 *)&array_data[element] = 1.0f;
        } else {
          *(PN_float32 *)&array_data[element] = 0.0f;
        }
        element += sizeof(PN_float32);
        ++i;
      }
    }
      
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_data
//       Access: Public
//  Description: Returns the data associated with the nth vertex for a
//               particular value.  Query the format to get the array
//               index and data_type parameters for the particular
//               data type you want to get.
//
//               This flavor of get_data() copies its data into a
//               generic float array.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
get_data(int array, const qpGeomVertexDataType *data_type,
         int vertex, float *data, int num_values) const {
  CPTA_uchar array_data = get_array(array)->get_data();
  int stride = _format->get_array(array)->get_stride();
  int element = vertex * stride + data_type->get_start();
  nassertv(element >= 0 && element + data_type->get_total_bytes() <= (int)array_data.size());

  switch (data_type->get_numeric_type()) {
  case qpGeomVertexDataType::NT_uint8:
    {
      int i = 0;
      int min_values = min(num_values, data_type->get_num_values());
      while (i < min_values) {
        int value = *(unsigned char *)&array_data[element];
        element += 1;
        data[i] = (float)value / 255.0f;
        ++i;
      }
      while (i < num_values) {
        data[i] = 0.0f;
        ++i;
      }
    }
    break;

  case qpGeomVertexDataType::NT_packed_argb:
    {
      if (num_values == 4) {
        unpack_argb(data, *(PN_uint32 *)&array_data[element]);
      } else {
        float data4[4];
        unpack_argb(data4, *(PN_uint32 *)&array_data[element]);
        memset(data, 0, num_values * sizeof(float));
        memcpy(data, data4, min(num_values, 4) * sizeof(float));
      }
    }
    break;

  case qpGeomVertexDataType::NT_float:
    if (num_values == data_type->get_num_values() && 
        sizeof(float) == sizeof(PN_float32)) {
      memcpy(data, &array_data[element], num_values * sizeof(PN_float32));
    } else {
      int i = 0;
      int min_values = min(num_values, data_type->get_num_values());
      while (i < min_values) {
        data[i] = *(PN_float32 *)&array_data[element];
        element += sizeof(PN_float32);
        ++i;
      }
      while (i < num_values) {
        if (i == 3 && num_values == 4) {
          data[i] = 1.0f;
        } else {
          data[i] = 0.0f;
        }
        ++i;
      }
    }

    break;
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
//     Function: qpGeomVertexData::to_vec2
//       Access: Public, Static
//  Description: Converts a data element of arbitrary number of
//               dimensions (1 - 4) into a vec2 in a sensible way.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
to_vec2(LVecBase2f &vec, const float *data, int num_values) {
  switch (num_values) {
  case 1:
    vec.set(data[0], 0.0f);
    break;
    
  case 2:
  case 3:
    vec.set(data[0], data[1]);
    break;
    
  default:  // 4 or more.
    vec.set(data[0] / data[3], data[1] / data[3]);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::to_vec3
//       Access: Public, Static
//  Description: Converts a data element of arbitrary number of
//               dimensions (1 - 4) into a vec3 in a sensible way.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
to_vec3(LVecBase3f &vec, const float *data, int num_values) {
  switch (num_values) {
  case 1:
    vec.set(data[0], 0.0f, 0.0f);
    break;
    
  case 2:
    vec.set(data[0], data[1], 0.0f);
    break;
    
  case 3:
    vec.set(data[0], data[1], data[2]);
    break;
    
  default:  // 4 or more.
    vec.set(data[0] / data[3], data[1] / data[3], data[2] / data[3]);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::to_vec4
//       Access: Public, Static
//  Description: Converts a data element of arbitrary number of
//               dimensions (1 - 4) into a vec4 in a sensible way.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
to_vec4(LVecBase4f &vec, const float *data, int num_values) {
  switch (num_values) {
  case 1:
    vec.set(data[0], 0.0f, 0.0f, 1.0f);
    break;
    
  case 2:
    vec.set(data[0], data[1], 0.0f, 1.0f);
    break;
    
  case 3:
    vec.set(data[0], data[1], data[2], 1.0f);
    break;
    
  default:  // 4 or more.
    vec.set(data[0], data[1], data[2], data[3]);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::pack_argb
//       Access: Public, Static
//  Description: Packs four floats, stored R, G, B, A, into a
//               packed_argb value.
////////////////////////////////////////////////////////////////////
unsigned int qpGeomVertexData::
pack_argb(const float data[4]) {
  unsigned int r = ((unsigned int)(data[0] * 255.0f)) & 0xff;
  unsigned int g = ((unsigned int)(data[1] * 255.0f)) & 0xff;
  unsigned int b = ((unsigned int)(data[2] * 255.0f)) & 0xff;
  unsigned int a = ((unsigned int)(data[3] * 255.0f)) & 0xff;
  return ((a << 24) | (r << 16) | (g << 8) | b);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::unpack_argb
//       Access: Public, Static
//  Description: Unpacks a packed_argb value into four floats, stored
//               R, G, B, A.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
unpack_argb(float data[4], unsigned int packed_argb) {
  data[0] = (float)((packed_argb >> 16) & 0xff) / 255.0f;
  data[1] = (float)((packed_argb >> 8) & 0xff) / 255.0f;
  data[2] = (float)(packed_argb & 0xff) / 255.0f;
  data[3] = (float)((packed_argb >> 24) & 0xff) / 255.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::remove_cache_entry
//       Access: Private
//  Description: Removes a particular entry from the local cache; it
//               has already been removed from the cache manager.
//               This is only called from GeomVertexCacheManager.
////////////////////////////////////////////////////////////////////
void qpGeomVertexData::
remove_cache_entry(const qpGeomVertexFormat *modifier) const {
  // We have to operate on stage 0 of the pipeline, since that's where
  // the cache really counts.  Because of the multistage pipeline, we
  // might not actually have a cache entry there (it might have been
  // added to stage 1 instead).  No big deal if we don't.
  CData *cdata = ((qpGeomVertexData *)this)->_cycler.write_stage(0);
  ConvertedCache::iterator ci = cdata->_converted_cache.find(modifier);
  if (ci != cdata->_converted_cache.end()) {
    cdata->_converted_cache.erase(ci);
  }
  ((qpGeomVertexData *)this)->_cycler.release_write_stage(0, cdata);
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
    if (cdata->_arrays[i]->set_num_vertices(n)) {
      any_changed = true;
    }
  }

  if (any_changed) {
    clear_cache();
  }

  return any_changed;
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
