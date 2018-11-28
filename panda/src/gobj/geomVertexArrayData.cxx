/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexArrayData.cxx
 * @author drose
 * @date 2005-03-17
 */

#include "geomVertexArrayData.h"
#include "geom.h"
#include "preparedGraphicsObjects.h"
#include "reversedNumericData.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pset.h"
#include "config_gobj.h"
#include "pStatTimer.h"
#include "configVariableInt.h"
#include "simpleAllocator.h"
#include "vertexDataBuffer.h"
#include "texture.h"

using std::max;
using std::min;

ConfigVariableInt max_independent_vertex_data
("max-independent-vertex-data", -1,
 PRC_DESC("Specifies the maximum number of bytes of all vertex data "
          "that is independent of the paging system.  This is an "
          "initial buffer before max-ram-vertex-data, specifically "
          "designed for vertex datas that are dynamic in nature and "
          "may change size or be created and destroyed frequently."));

ConfigVariableInt vertex_data_page_size
("vertex-data-page-size", 262144,
 PRC_DESC("The number of bytes to allocate at a time for vertex data.  "
          "This also controls the page size that is compressed or written "
          "to disk when vertex data pages are evicted from memory."));

SimpleLru GeomVertexArrayData::_independent_lru("independent", max_independent_vertex_data);
SimpleLru GeomVertexArrayData::_small_lru("small", max_independent_vertex_data);

VertexDataBook GeomVertexArrayData::_book(vertex_data_page_size);


TypeHandle GeomVertexArrayData::_type_handle;
TypeHandle GeomVertexArrayData::CData::_type_handle;
TypeHandle GeomVertexArrayDataHandle::_type_handle;

ALLOC_DELETED_CHAIN_DEF(GeomVertexArrayDataHandle);

/**
 * Constructs an invalid object.  This is only used when reading from the bam
 * file.
 */
GeomVertexArrayData::
GeomVertexArrayData() : SimpleLruPage(0) {
  _contexts = nullptr;

  // Can't put it in the LRU until it has been read in and made valid.
}

/**
 * Required to implement CopyOnWriteObject.
 */
PT(CopyOnWriteObject) GeomVertexArrayData::
make_cow_copy() {
  return new GeomVertexArrayData(*this);
}

/**
 *
 */
GeomVertexArrayData::
GeomVertexArrayData(const GeomVertexArrayFormat *array_format,
                    GeomVertexArrayData::UsageHint usage_hint) :
  SimpleLruPage(0),
  _array_format(array_format),
  _cycler(CData(usage_hint)),
  _contexts(nullptr)
{
  set_lru_size(0);
  nassertv(_array_format->is_registered());
}

/**
 *
 */
GeomVertexArrayData::
GeomVertexArrayData(const GeomVertexArrayData &copy) :
  CopyOnWriteObject(copy),
  SimpleLruPage(copy),
  _array_format(copy._array_format),
  _cycler(copy._cycler),
  _contexts(nullptr)
{
  copy.mark_used_lru();

  set_lru_size(get_data_size_bytes());
  nassertv(_array_format->is_registered());
}

/**
 * The copy assignment operator is not pipeline-safe.  This will completely
 * obliterate all stages of the pipeline, so don't do it for a
 * GeomVertexArrayData that is actively being used for rendering.
 */
void GeomVertexArrayData::
operator = (const GeomVertexArrayData &copy) {
  CopyOnWriteObject::operator = (copy);
  SimpleLruPage::operator = (copy);

  copy.mark_used_lru();

  _array_format = copy._array_format;
  _cycler = copy._cycler;

  OPEN_ITERATE_ALL_STAGES(_cycler) {
    CDStageWriter cdata(_cycler, pipeline_stage);
    cdata->_modified = Geom::get_next_modified();
  }
  CLOSE_ITERATE_ALL_STAGES(_cycler);

  nassertv(_array_format->is_registered());
}

/**
 *
 */
GeomVertexArrayData::
~GeomVertexArrayData() {
  release_all();
}

/**
 * Returns 0 if the two arrays are equivalent, even if they are not the same
 * pointer.
 */
int GeomVertexArrayData::
compare_to(const GeomVertexArrayData &other) const {
  Thread *current_thread = Thread::get_current_thread();

  CPT(GeomVertexArrayDataHandle) handle = get_handle(current_thread);
  CPT(GeomVertexArrayDataHandle) other_handle = other.get_handle(current_thread);

  if (handle->get_usage_hint() != other_handle->get_usage_hint()) {
    return (int)handle->get_usage_hint() - (int)other_handle->get_usage_hint();
  }
  if (handle->get_array_format() != other_handle->get_array_format()) {
    return handle->get_array_format() < other_handle->get_array_format() ? -1 : 1;
  }
  if (handle->get_data_size_bytes() != other_handle->get_data_size_bytes()) {
    return (int)handle->get_data_size_bytes() - (int)other_handle->get_data_size_bytes();
  }
  return memcmp(handle->get_read_pointer(true),
                other_handle->get_read_pointer(true),
                handle->get_data_size_bytes());
}

/**
 * Changes the UsageHint hint for this array.  See get_usage_hint().
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void GeomVertexArrayData::
set_usage_hint(GeomVertexArrayData::UsageHint usage_hint) {
  CDWriter cdata(_cycler, true);
  cdata->_usage_hint = usage_hint;
  cdata->_modified = Geom::get_next_modified();
}

/**
 *
 */
void GeomVertexArrayData::
output(std::ostream &out) const {
  out << get_num_rows() << " rows: " << *get_array_format();
}

/**
 *
 */
void GeomVertexArrayData::
write(std::ostream &out, int indent_level) const {
  _array_format->write_with_data(out, indent_level, this);
}

/**
 * Indicates that the data should be enqueued to be prepared in the indicated
 * prepared_objects at the beginning of the next frame.  This will ensure the
 * data is already loaded into the GSG if it is expected to be rendered soon.
 *
 * Use this function instead of prepare_now() to preload datas from a user
 * interface standpoint.
 */
void GeomVertexArrayData::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_vertex_buffer(this);
}

/**
 * Returns true if the data has already been prepared or enqueued for
 * preparation on the indicated GSG, false otherwise.
 */
bool GeomVertexArrayData::
is_prepared(PreparedGraphicsObjects *prepared_objects) const {
  if (_contexts == nullptr) {
    return false;
  }
  Contexts::const_iterator ci;
  ci = _contexts->find(prepared_objects);
  if (ci != _contexts->end()) {
    return true;
  }
  return prepared_objects->is_vertex_buffer_queued(this);
}

/**
 * Creates a context for the data on the particular GSG, if it does not
 * already exist.  Returns the new (or old) VertexBufferContext.  This assumes
 * that the GraphicsStateGuardian is the currently active rendering context
 * and that it is ready to accept new datas.  If this is not necessarily the
 * case, you should use prepare() instead.
 *
 * Normally, this is not called directly except by the GraphicsStateGuardian;
 * a data does not need to be explicitly prepared by the user before it may be
 * rendered.
 */
VertexBufferContext *GeomVertexArrayData::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) {
  if (_contexts == nullptr) {
    _contexts = new Contexts;
  }
  Contexts::const_iterator ci;
  ci = _contexts->find(prepared_objects);
  if (ci != _contexts->end()) {
    return (*ci).second;
  }

  VertexBufferContext *vbc = prepared_objects->prepare_vertex_buffer_now(this, gsg);
  if (vbc != nullptr) {
    (*_contexts)[prepared_objects] = vbc;
  }
  return vbc;
}

/**
 * Frees the data context only on the indicated object, if it exists there.
 * Returns true if it was released, false if it had not been prepared.
 */
bool GeomVertexArrayData::
release(PreparedGraphicsObjects *prepared_objects) {
  if (_contexts != nullptr) {
    Contexts::iterator ci;
    ci = _contexts->find(prepared_objects);
    if (ci != _contexts->end()) {
      VertexBufferContext *vbc = (*ci).second;
      prepared_objects->release_vertex_buffer(vbc);
      return true;
    }
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_vertex_buffer(this);
}

/**
 * Frees the context allocated on all objects for which the data has been
 * declared.  Returns the number of contexts which have been freed.
 */
int GeomVertexArrayData::
release_all() {
  int num_freed = 0;

  if (_contexts != nullptr) {
    // We have to traverse a copy of the _contexts list, because the
    // PreparedGraphicsObjects object will call clear_prepared() in response
    // to each release_vertex_buffer(), and we don't want to be modifying the
    // _contexts list while we're traversing it.
    Contexts temp = *_contexts;
    num_freed = (int)_contexts->size();

    Contexts::const_iterator ci;
    for (ci = temp.begin(); ci != temp.end(); ++ci) {
      PreparedGraphicsObjects *prepared_objects = (*ci).first;
      VertexBufferContext *vbc = (*ci).second;
      prepared_objects->release_vertex_buffer(vbc);
    }

    // Now that we've called release_vertex_buffer() on every known context,
    // the _contexts list should have completely emptied itself.
    nassertr(_contexts == nullptr, num_freed);
  }

  return num_freed;
}

/**
 * Marks that an epoch has passed in each LRU.  Asks the LRU's to consider
 * whether they should perform evictions.
 */
void GeomVertexArrayData::
lru_epoch() {
  _independent_lru.begin_epoch();
  VertexDataPage::get_global_lru(VertexDataPage::RC_resident)->begin_epoch();
  VertexDataPage::get_global_lru(VertexDataPage::RC_compressed)->begin_epoch();
}

/**
 * Evicts the page from the LRU.  Called internally when the LRU determines
 * that it is full.  May also be called externally when necessary to
 * explicitly evict the page.
 *
 * It is legal for this method to either evict the page as requested, do
 * nothing (in which case the eviction will be requested again at the next
 * epoch), or requeue itself on the tail of the queue (in which case the
 * eviction will be requested again much later).
 */
void GeomVertexArrayData::
evict_lru() {
  dequeue_lru();
  CDWriter cdata(_cycler, true);
  cdata->_buffer.page_out(_book);
}

/**
 * Removes the indicated PreparedGraphicsObjects table from the data array's
 * table, without actually releasing the data array.  This is intended to be
 * called only from PreparedGraphicsObjects::release_vertex_buffer(); it
 * should never be called by user code.
 */
void GeomVertexArrayData::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  nassertv(_contexts != nullptr);

  Contexts::iterator ci;
  ci = _contexts->find(prepared_objects);
  if (ci != _contexts->end()) {
    _contexts->erase(ci);
    if (_contexts->empty()) {
      delete _contexts;
      _contexts = nullptr;
    }
  } else {
    // If this assertion fails, clear_prepared() was given a prepared_objects
    // which the data array didn't know about.
    nassert_raise("unknown PreparedGraphicsObjects");
  }
}

/**
 * Fills a new data array with all numeric values expressed in the indicated
 * array reversed, byte-for-byte, to convert littleendian to bigendian and
 * vice-versa.
 */
void GeomVertexArrayData::
reverse_data_endianness(unsigned char *dest, const unsigned char *source,
                        size_t size) {
  int num_columns = _array_format->get_num_columns();

  // Walk through each row of the data.
  for (size_t pi = 0; pi < size; pi += _array_format->get_stride()) {
    // For each row, visit all of the columns; and for each column, visit all
    // of the components of that column.
    for (int ci = 0; ci < num_columns; ++ci) {
      const GeomVertexColumn *col = _array_format->get_column(ci);
      int component_bytes = col->get_component_bytes();
      if (component_bytes > 1) {
        // Get the index of the beginning of the column.
        size_t ci = pi + col->get_start();

        int num_components = col->get_num_components();
        for (int cj = 0; cj < num_components; ++cj) {
          // Reverse the bytes of each component.
          ReversedNumericData nd(source + ci, component_bytes);
          nd.store_value(dest + ci, component_bytes);
          ci += component_bytes;
        }
      }
    }
  }
}

/**
 * Tells the BamReader how to create objects of type GeomVertexArrayData.
 */
void GeomVertexArrayData::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void GeomVertexArrayData::
write_datagram(BamWriter *manager, Datagram &dg) {
  CopyOnWriteObject::write_datagram(manager, dg);

  manager->write_pointer(dg, _array_format);
  manager->write_cdata(dg, _cycler, this);
}

/**
 * Called by CData::fillin to read the raw data of the array from the
 * indicated datagram.
 */
PTA_uchar GeomVertexArrayData::
read_raw_data(BamReader *manager, DatagramIterator &scan) {
  size_t size = scan.get_uint32();
  PTA_uchar data = PTA_uchar::empty_array(size, get_class_type());
  const unsigned char *source_data =
    (const unsigned char *)scan.get_datagram().get_data();
  memcpy(data, source_data + scan.get_current_index(), size);
  scan.skip_bytes(size);

  return data;
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int GeomVertexArrayData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CopyOnWriteObject::complete_pointers(p_list, manager);

  _array_format = DCAST(GeomVertexArrayFormat, p_list[pi++]);

  return pi;
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void GeomVertexArrayData::
finalize(BamReader *manager) {
  // Now we need to register the format that we have read from the bam file
  // (since it doesn't come out of the bam file automatically registered).
  // This may change the format's pointer, which we should then update our own
  // data to reflect.  But since this may cause the unregistered object to
  // destruct, we have to also tell the BamReader to return the new object
  // from now on.

  CDWriter cdata(_cycler, true);

  CPT(GeomVertexArrayFormat) new_array_format =
    GeomVertexArrayFormat::register_format(_array_format);

  manager->change_pointer(_array_format, new_array_format);
  _array_format = new_array_format;

  PT(BamAuxData) aux_data = (BamAuxData *)manager->get_aux_data(this, "");
  if (aux_data != nullptr) {
    if (aux_data->_endian_reversed) {
      // Now is the time to endian-reverse the data.
      VertexDataBuffer new_buffer(cdata->_buffer.get_size());
      reverse_data_endianness(new_buffer.get_write_pointer(), cdata->_buffer.get_read_pointer(true), cdata->_buffer.get_size());
      cdata->_buffer.swap(new_buffer);
    }
  }

  set_lru_size(cdata->_buffer.get_size());
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type GeomVertexArrayData is encountered in the Bam file.  It should create
 * the GeomVertexArrayData and extract its information from the file.
 */
TypedWritable *GeomVertexArrayData::
make_from_bam(const FactoryParams &params) {
  GeomVertexArrayData *object = new GeomVertexArrayData;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);
  manager->register_finalize(object);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new GeomVertexArrayData.
 */
void GeomVertexArrayData::
fillin(DatagramIterator &scan, BamReader *manager) {
  CopyOnWriteObject::fillin(scan, manager);

  manager->read_pointer(scan);
  manager->read_cdata(scan, _cycler, this);
}

/**
 *
 */
GeomVertexArrayData::CData::
~CData() {
}

/**
 *
 */
CycleData *GeomVertexArrayData::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void GeomVertexArrayData::CData::
write_datagram(BamWriter *manager, Datagram &dg, void *extra_data) const {
  GeomVertexArrayData *array_data = (GeomVertexArrayData *)extra_data;
  dg.add_uint8(_usage_hint);

  dg.add_uint32(_buffer.get_size());

  if (manager->get_file_endian() == BamWriter::BE_native) {
    // For native endianness, we only have to write the data directly.
    dg.append_data(_buffer.get_read_pointer(true), _buffer.get_size());

  } else {
    // Otherwise, we have to convert it.
    unsigned char *new_data = (unsigned char *)alloca(_buffer.get_size());
    array_data->reverse_data_endianness(new_data, _buffer.get_read_pointer(true), _buffer.get_size());
    dg.append_data(new_data, _buffer.get_size());
  }
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new GeomVertexArrayData.
 */
void GeomVertexArrayData::CData::
fillin(DatagramIterator &scan, BamReader *manager, void *extra_data) {
  GeomVertexArrayData *array_data = (GeomVertexArrayData *)extra_data;
  _usage_hint = (UsageHint)scan.get_uint8();

  if (manager->get_file_minor_ver() < 8) {
    // Before bam version 6.8, the array data was a PTA_uchar.
    PTA_uchar new_data;
    READ_PTA(manager, scan, array_data->read_raw_data, new_data);
    _buffer.unclean_realloc(new_data.size());
    _buffer.set_size(new_data.size());
    memcpy(_buffer.get_write_pointer(), &new_data[0], new_data.size());

  } else {
    // Now, the array data is just stored directly.
    size_t size = scan.get_uint32();
    _buffer.unclean_realloc(size);
    _buffer.set_size(size);

    const unsigned char *source_data =
      (const unsigned char *)scan.get_datagram().get_data();
    memcpy(_buffer.get_write_pointer(), source_data + scan.get_current_index(), size);
    scan.skip_bytes(size);
  }

  bool endian_reversed = false;

  if (manager->get_file_endian() != BamReader::BE_native) {
    // For non-native endian files, we have to convert the data.

    if (array_data->_array_format == nullptr) {
      // But we can't do that until we've completed the _array_format pointer,
      // which tells us how to convert it.
      endian_reversed = true;
    } else {
      // Since we have the _array_format pointer now, we can reverse it
      // immediately (and we should, to support threaded CData updates).
      VertexDataBuffer new_buffer(_buffer.get_size());
      array_data->reverse_data_endianness(new_buffer.get_write_pointer(), _buffer.get_read_pointer(true), _buffer.get_size());
      _buffer.swap(new_buffer);
    }
  }

  if (endian_reversed) {
    PT(BamAuxData) aux_data = new BamAuxData;
    aux_data->_endian_reversed = endian_reversed;
    manager->set_aux_data(array_data, "", aux_data);
  }

  array_data->set_lru_size(_buffer.get_size());

  _modified = Geom::get_next_modified();
}

/**
 * Returns a writable pointer to the beginning of the actual data stream.
 */
unsigned char *GeomVertexArrayDataHandle::
get_write_pointer() {
  nassertr(_writable, nullptr);
  mark_used();
  _cdata->_modified = Geom::get_next_modified();
  return _cdata->_buffer.get_write_pointer();
}

/**
 *
 */
bool GeomVertexArrayDataHandle::
set_num_rows(int n) {
  nassertr(_writable, false);
  mark_used();

  int stride = _object->_array_format->get_stride();
  size_t new_size = n * stride;
  size_t orig_size = _cdata->_buffer.get_size();

  if (gobj_cat.is_spam()) {
    gobj_cat.spam()
      << _object << ".set_num_rows(" << n << "), size = " << new_size << "\n";
  }

  if (new_size != orig_size) {
    size_t orig_reserved_size = _cdata->_buffer.get_reserved_size();
    if (new_size > orig_reserved_size) {
      // Add more rows.  Go up to the next power of two bytes, mainly to
      // reduce the number of allocs needed.
      size_t new_reserved_size = (size_t)Texture::up_to_power_2((int)new_size);
      nassertr(new_reserved_size >= new_size, false);

      _cdata->_buffer.clean_realloc(new_reserved_size);

    } else if (new_size == 0) {
      // If we set the number of rows to 0, go ahead and clear the buffer
      // altogether, and let the user build it up again from nothing, to try
      // to reduce frivolous memory waste.
      _cdata->_buffer.clear();
    }

    _cdata->_buffer.set_size(new_size);

    // Now ensure that the newly-added rows are initialized to 0.
    if (new_size > orig_size) {
      memset(_cdata->_buffer.get_write_pointer() + orig_size, 0,
             new_size - orig_size);
    }

    _cdata->_modified = Geom::get_next_modified();

    if (get_current_thread()->get_pipeline_stage() == 0) {
      _object->set_lru_size(_cdata->_buffer.get_size());
    }

    nassertr(get_num_rows() == n, true);
    return true;
  }

  nassertr(get_num_rows() == n, false);
  return false;
}

/**
 *
 */
bool GeomVertexArrayDataHandle::
unclean_set_num_rows(int n) {
  nassertr(_writable, false);
  mark_used();

  int stride = _object->_array_format->get_stride();
  size_t new_size = n * stride;
  size_t orig_size = _cdata->_buffer.get_size();
  size_t orig_reserved_size = _cdata->_buffer.get_reserved_size();

  if (new_size != orig_size || new_size != orig_reserved_size) {
    // Since this is unclean_set_num_rows(), we won't be using it to
    // incrementally increase the array; instead, it will generally be used
    // only to create an array initially.  So it makes sense to set the
    // reserved size to precisely the same as the target size.

    _cdata->_buffer.unclean_realloc(new_size);
    _cdata->_buffer.set_size(new_size);

    // No need to fill to zero or copy the old buffer, since this is
    // unclean_set_num_rows().

    if (new_size != orig_size) {
      _cdata->_modified = Geom::get_next_modified();

      if (get_current_thread()->get_pipeline_stage() == 0) {
        _object->set_lru_size(_cdata->_buffer.get_size());
      }
    }
    return true;
  }

  return false;
}

/**
 *
 */
bool GeomVertexArrayDataHandle::
reserve_num_rows(int n) {
  nassertr(_writable, false);
  mark_used();

  int stride = _object->_array_format->get_stride();
  size_t new_reserved_size = n * stride;
  new_reserved_size = max(_cdata->_buffer.get_size(), new_reserved_size);
  size_t orig_reserved_size = _cdata->_buffer.get_reserved_size();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << _object << ".reserve_num_rows(" << n << "), size = " << new_reserved_size << "\n";
  }

  if (new_reserved_size != orig_reserved_size) {
    // We allow the user to set the alloc point smaller with this call,
    // assuming the user knows what he's doing.  This allows the user to
    // reduce wasted memory after completely filling up a buffer.
    _cdata->_buffer.clean_realloc(new_reserved_size);
    return true;
  }

  return false;
}

/**
 * Copies the entire data array from the other object.
 */
void GeomVertexArrayDataHandle::
copy_data_from(const GeomVertexArrayDataHandle *other) {
  nassertv(_writable);
  other->mark_used();

  size_t size = other->_cdata->_buffer.get_size();
  const unsigned char *source = other->_cdata->_buffer.get_read_pointer(true);

  copy_data_from(source, size);
}

/**
 * Copies a portion of the data array from the other object into a portion of
 * the data array of this object.  If to_size != from_size, the size of this
 * data array is adjusted accordingly.
 */
void GeomVertexArrayDataHandle::
copy_subdata_from(size_t to_start, size_t to_size,
                  const GeomVertexArrayDataHandle *other,
                  size_t from_start, size_t from_size) {
  other->mark_used();

  const VertexDataBuffer &from_buffer = other->_cdata->_buffer;
  size_t from_buffer_orig_size = from_buffer.get_size();
  from_start = min(from_start, from_buffer_orig_size);
  from_size = min(from_size, from_buffer_orig_size - from_start);

  copy_subdata_from(to_start, to_size,
                    other->get_read_pointer(true),
                    from_start, from_size);
}

/**
 * Copies the entire data array from the buffer.
 */
void GeomVertexArrayDataHandle::
copy_data_from(const unsigned char *source, size_t size) {
  nassertv(_writable);
  mark_used();

  _cdata->_buffer.unclean_realloc(size);
  _cdata->_buffer.set_size(size);

  unsigned char *dest = _cdata->_buffer.get_write_pointer();
  memcpy(dest, source, size);

  _cdata->_modified = Geom::get_next_modified();

  if (get_current_thread()->get_pipeline_stage() == 0) {
    _object->set_lru_size(_cdata->_buffer.get_size());
  }
}

/**
 * Copies a portion of the data array from the buffer into a portion of the
 * data array of this object.  If to_size != from_size, the size of this data
 * array is adjusted accordingly.
 */
void GeomVertexArrayDataHandle::
copy_subdata_from(size_t to_start, size_t to_size,
                  const unsigned char *source,
                  size_t from_start, size_t from_size) {
  nassertv(_writable);
  mark_used();

  VertexDataBuffer &to_buffer = _cdata->_buffer;
  size_t to_buffer_orig_size = to_buffer.get_size();
  to_start = min(to_start, to_buffer_orig_size);
  to_size = min(to_size, to_buffer_orig_size - to_start);

  if (from_size < to_size) {
    // Reduce the array.
    unsigned char *pointer = to_buffer.get_write_pointer();
    memmove(pointer + to_start + to_size,
            pointer + to_start + from_size,
            to_buffer_orig_size - (to_start + to_size));
    to_buffer.set_size(to_buffer_orig_size + from_size - to_size);

  } else if (to_size < from_size) {
    // Expand the array.
    size_t needed_size = to_buffer_orig_size + from_size - to_size;
    size_t to_buffer_orig_reserved_size = to_buffer.get_reserved_size();
    if (needed_size > to_buffer_orig_reserved_size) {
      size_t new_reserved_size = (size_t)Texture::up_to_power_2((int)needed_size);
      to_buffer.clean_realloc(new_reserved_size);
    }
    to_buffer.set_size(needed_size);

    unsigned char *pointer = to_buffer.get_write_pointer();
    memmove(pointer + to_start + to_size,
            pointer + to_start + from_size,
            to_buffer_orig_size - (to_start + to_size));
  }

  // Now copy the data.
  memcpy(to_buffer.get_write_pointer() + to_start,
         source + from_start, from_size);
  _cdata->_modified = Geom::get_next_modified();

  if (get_current_thread()->get_pipeline_stage() == 0) {
    _object->set_lru_size(_cdata->_buffer.get_size());
  }
}

/**
 * Replaces the entire raw data array with the contents of the indicated
 * string.  This is primarily for the benefit of high-level languages like
 * Python.
 */
void GeomVertexArrayDataHandle::
set_data(const vector_uchar &data) {
  nassertv(_writable);
  mark_used();

  _cdata->_buffer.unclean_realloc(data.size());
  _cdata->_buffer.set_size(data.size());
  memcpy(_cdata->_buffer.get_write_pointer(), data.data(), data.size());

  _cdata->_modified = Geom::get_next_modified();

  if (get_current_thread()->get_pipeline_stage() == 0) {
    _object->set_lru_size(_cdata->_buffer.get_size());
  }
}

/**
 * Replaces a portion of the data array from the indicated string.  If size !=
 * data.size(), the size of this data array is adjusted accordingly.
 *
 * This is primarily for the benefit of high-level languages like Python.
 */
void GeomVertexArrayDataHandle::
set_subdata(size_t start, size_t size, const vector_uchar &data) {
  nassertv(_writable);
  mark_used();

  VertexDataBuffer &to_buffer = _cdata->_buffer;
  size_t to_buffer_orig_size = to_buffer.get_size();
  start = min(start, to_buffer_orig_size);
  size = min(size, to_buffer_orig_size - start);

  size_t from_size = data.size();

  if (from_size < size) {
    // Reduce the array.
    unsigned char *pointer = to_buffer.get_write_pointer();
    memmove(pointer + start + from_size,
            pointer + start + size,
            to_buffer_orig_size - (start + size));
    to_buffer.set_size(to_buffer_orig_size + from_size - size);

  } else if (size < from_size) {
    // Expand the array.
    size_t needed_size = to_buffer_orig_size + from_size - size;
    size_t to_buffer_orig_reserved_size = to_buffer.get_reserved_size();
    if (needed_size > to_buffer_orig_reserved_size) {
      size_t new_reserved_size = (size_t)Texture::up_to_power_2((int)needed_size);
      to_buffer.clean_realloc(new_reserved_size);
    }
    to_buffer.set_size(needed_size);

    unsigned char *pointer = to_buffer.get_write_pointer();
    memmove(pointer + start + from_size,
            pointer + start + size,
            to_buffer_orig_size - (start + size));
  }

  // Now copy the data.
  memcpy(to_buffer.get_write_pointer() + start, data.data(), from_size);
  _cdata->_modified = Geom::get_next_modified();

  if (get_current_thread()->get_pipeline_stage() == 0) {
    _object->set_lru_size(_cdata->_buffer.get_size());
  }
}
