/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderBuffer.cxx
 * @author rdb
 * @date 2016-12-12
 */

#include "shaderBuffer.h"
#include "preparedGraphicsObjects.h"

TypeHandle ShaderBuffer::_type_handle;

/**
 * Destructor.
 */
ShaderBuffer::
~ShaderBuffer() {
  release_all();
}

/**
 *
 */
void ShaderBuffer::
output(std::ostream &out) const {
  out << "buffer " << get_name() << ", " << _data_size_bytes << "B, " << _usage_hint;
}

/**
 * Indicates that the data should be enqueued to be prepared in the indicated
 * prepared_objects at the beginning of the next frame.  This will ensure the
 * data is already loaded into the GSG if it is expected to be rendered soon.
 *
 * Use this function instead of prepare_now() to preload datas from a user
 * interface standpoint.
 */
void ShaderBuffer::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_shader_buffer(this);
}

/**
 * Returns true if the data has already been prepared or enqueued for
 * preparation on the indicated GSG, false otherwise.
 */
bool ShaderBuffer::
is_prepared(PreparedGraphicsObjects *prepared_objects) const {
  if (_contexts == nullptr) {
    return false;
  }
  Contexts::const_iterator ci;
  ci = _contexts->find(prepared_objects);
  if (ci != _contexts->end()) {
    return true;
  }
  return prepared_objects->is_shader_buffer_queued(this);
}

/**
 * Creates a context for the data on the particular GSG, if it does not
 * already exist.  Returns the new (or old) BufferContext.  This assumes
 * that the GraphicsStateGuardian is the currently active rendering context
 * and that it is ready to accept new datas.  If this is not necessarily the
 * case, you should use prepare() instead.
 *
 * Normally, this is not called directly except by the GraphicsStateGuardian;
 * a data does not need to be explicitly prepared by the user before it may be
 * rendered.
 */
BufferContext *ShaderBuffer::
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

  BufferContext *vbc = prepared_objects->prepare_shader_buffer_now(this, gsg);
  if (vbc != nullptr) {
    (*_contexts)[prepared_objects] = vbc;
  }
  return vbc;
}

/**
 * Frees the data context only on the indicated object, if it exists there.
 * Returns true if it was released, false if it had not been prepared.
 */
bool ShaderBuffer::
release(PreparedGraphicsObjects *prepared_objects) {
  if (_contexts != nullptr) {
    Contexts::iterator ci;
    ci = _contexts->find(prepared_objects);
    if (ci != _contexts->end()) {
      BufferContext *vbc = (*ci).second;
      prepared_objects->release_shader_buffer(vbc);
      return true;
    }
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_shader_buffer(this);
}

/**
 * Frees the context allocated on all objects for which the data has been
 * declared.  Returns the number of contexts which have been freed.
 */
int ShaderBuffer::
release_all() {
  int num_freed = 0;

  if (_contexts != nullptr) {
    // We have to traverse a copy of the _contexts list, because the
    // PreparedGraphicsObjects object will call clear_prepared() in response
    // to each release_shader_buffer(), and we don't want to be modifying the
    // _contexts list while we're traversing it.
    Contexts temp = *_contexts;
    num_freed = (int)_contexts->size();

    Contexts::const_iterator ci;
    for (ci = temp.begin(); ci != temp.end(); ++ci) {
      PreparedGraphicsObjects *prepared_objects = (*ci).first;
      BufferContext *vbc = (*ci).second;
      prepared_objects->release_shader_buffer(vbc);
    }

    // Now that we've called release_shader_buffer() on every known context,
    // the _contexts list should have completely emptied itself.
    nassertr(_contexts == nullptr, num_freed);
  }

  return num_freed;
}

/**
 * Removes the indicated PreparedGraphicsObjects table from the buffer's
 * table, without actually releasing the texture.  This is intended to be
 * called only from PreparedGraphicsObjects::release_shader_buffer(); it
 * should never be called by user code.
 */
void ShaderBuffer::
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
 * Tells the BamReader how to create objects of type ParamValue.
 */
void ShaderBuffer::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderBuffer::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_string(get_name());
  dg.add_uint64(_data_size_bytes);
  dg.add_uint8(_usage_hint);
  dg.add_bool(!_initial_data.empty());
  dg.append_data(_initial_data.data(), _initial_data.size());
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ParamValue is encountered in the Bam file.  It should create the
 * ParamValue and extract its information from the file.
 */
TypedWritable *ShaderBuffer::
make_from_bam(const FactoryParams &params) {
  ShaderBuffer *param = new ShaderBuffer;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ParamValue.
 */
void ShaderBuffer::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
  _data_size_bytes = scan.get_uint64();
  _usage_hint = (UsageHint)scan.get_uint8();

  if (scan.get_bool() && _data_size_bytes > 0) {
    nassertv_always(_data_size_bytes <= scan.get_remaining_size());
    _initial_data.resize((_data_size_bytes + 15u) & ~15u);
    scan.extract_bytes(&_initial_data[0], _data_size_bytes);
  } else {
    _initial_data.clear();
  }
}
