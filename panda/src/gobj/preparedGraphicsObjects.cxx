/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file preparedGraphicsObjects.cxx
 * @author drose
 * @date 2004-02-19
 */

#include "preparedGraphicsObjects.h"
#include "textureContext.h"
#include "vertexBufferContext.h"
#include "indexBufferContext.h"
#include "texture.h"
#include "geom.h"
#include "geomVertexArrayData.h"
#include "geomPrimitive.h"
#include "samplerContext.h"
#include "shader.h"
#include "reMutexHolder.h"
#include "geomContext.h"
#include "shaderContext.h"
#include "config_gobj.h"
#include "throw_event.h"

TypeHandle PreparedGraphicsObjects::EnqueuedObject::_type_handle;

int PreparedGraphicsObjects::_name_index = 0;

/**
 *
 */
PreparedGraphicsObjects::
PreparedGraphicsObjects() :
  _lock("PreparedGraphicsObjects::_lock"),
  _name(init_name()),
  _vertex_buffer_cache_size(0),
  _index_buffer_cache_size(0),
  _texture_residency(_name, "texture"),
  _vbuffer_residency(_name, "vbuffer"),
  _ibuffer_residency(_name, "ibuffer"),
  _sbuffer_residency(_name, "sbuffer"),
  _graphics_memory_lru("graphics_memory_lru", graphics_memory_limit),
  _sampler_object_lru("sampler_object_lru", sampler_object_limit)
{
  // GLGSG will turn this flag on.  This is a temporary hack to disable this
  // feature for DX8DX9 for now, until we work out the fine points of updating
  // the fvf properly.
  _support_released_buffer_cache = false;
}

/**
 *
 */
PreparedGraphicsObjects::
~PreparedGraphicsObjects() {
  // There may be objects that are still prepared when we destruct.  If this
  // is so, then all of the GSG's that own them have already destructed, so we
  // can assume their resources were internally cleaned up.  Besides, we may
  // not even be allowed to call the GSG release methods since some APIs (eg.
  // OpenGL) require a context current.  So we just call the destructors.
  ReMutexHolder holder(_lock);

  release_all_textures();
  for (TextureContext *tc : _released_textures) {
    delete tc;
  }
  _released_textures.clear();

  release_all_samplers();
  for (SamplerContext *sc : _released_samplers) {
    delete sc;
  }
  _released_samplers.clear();

  release_all_geoms();
  for (GeomContext *gc : _released_geoms) {
    delete gc;
  }
  _released_geoms.clear();

  release_all_shaders();
  for (ShaderContext *sc : _released_shaders) {
    delete sc;
  }
  _released_shaders.clear();

  release_all_vertex_buffers();
  for (BufferContext *bc : _released_vertex_buffers) {
    delete bc;
  }
  _released_vertex_buffers.clear();

  release_all_index_buffers();
  for (BufferContext *bc : _released_index_buffers) {
    delete bc;
  }
  _released_index_buffers.clear();

  release_all_shader_buffers();
  for (BufferContext *bc : _released_shader_buffers) {
    delete bc;
  }
  _released_shader_buffers.clear();
}

/**
 * Sets an artificial cap on graphics memory that will be imposed on this GSG.
 *
 * This limits the total amount of graphics memory, including texture memory
 * and vertex buffer memory, that will be consumed by the GSG, regardless of
 * whether the hardware claims to provide more graphics memory than this.  It
 * is useful to put a ceiling on graphics memory consumed, since some drivers
 * seem to allow the application to consume more memory than the hardware can
 * realistically support.
 */
void PreparedGraphicsObjects::
set_graphics_memory_limit(size_t limit) {
  if (limit != _graphics_memory_lru.get_max_size()) {
    _graphics_memory_lru.set_max_size(limit);

    // We throw an event here so global objects (particularly the
    // TexMemWatcher) can automatically respond to this change.
    throw_event("graphics_memory_limit_changed");
  }
}

/**
 * Writes to the indicated ostream a report of how the various textures and
 * vertex buffers are allocated in the LRU.
 */
void PreparedGraphicsObjects::
show_graphics_memory_lru(std::ostream &out) const {
  _graphics_memory_lru.write(out, 0);
}

/**
 * Writes to the indicated ostream a report of how the various textures and
 * vertex buffers are allocated in the LRU.
 */
void PreparedGraphicsObjects::
show_residency_trackers(std::ostream &out) const {
  out << "Textures:\n";
  _texture_residency.write(out, 2);

  out << "\nVertex buffers:\n";
  _vbuffer_residency.write(out, 2);

  out << "\nIndex buffers:\n";
  _ibuffer_residency.write(out, 2);

  out << "\nShader buffers:\n";
  _sbuffer_residency.write(out, 2);
}

/**
 * Indicates that a texture would like to be put on the list to be prepared
 * when the GSG is next ready to do this (presumably at the next frame).
 */
void PreparedGraphicsObjects::
enqueue_texture(Texture *tex) {
  ReMutexHolder holder(_lock);

  _enqueued_textures.insert(EnqueuedTextures::value_type(tex, nullptr));
}

/**
 * Like enqueue_texture, but returns an AsyncFuture that can be used to query
 * the status of the texture's preparation.
 */
PT(PreparedGraphicsObjects::EnqueuedObject) PreparedGraphicsObjects::
enqueue_texture_future(Texture *tex) {
  ReMutexHolder holder(_lock);

  std::pair<EnqueuedTextures::iterator, bool> result =
    _enqueued_textures.insert(EnqueuedTextures::value_type(tex, nullptr));
  if (result.first->second == nullptr) {
    result.first->second = new EnqueuedObject(this, tex);
  }
  PT(EnqueuedObject) fut = result.first->second;
  nassertr(!fut->cancelled(), fut)
  return fut;
}

/**
 * Returns true if the texture has been queued on this GSG, false otherwise.
 */
bool PreparedGraphicsObjects::
is_texture_queued(const Texture *tex) const {
  ReMutexHolder holder(_lock);

  EnqueuedTextures::const_iterator qi = _enqueued_textures.find((Texture *)tex);
  return (qi != _enqueued_textures.end());
}

/**
 * Removes a texture from the queued list of textures to be prepared.
 * Normally it is not necessary to call this, unless you change your mind
 * about preparing it at the last minute, since the texture will automatically
 * be dequeued and prepared at the next frame.
 *
 * The return value is true if the texture is successfully dequeued, false if
 * it had not been queued.
 */
bool PreparedGraphicsObjects::
dequeue_texture(Texture *tex) {
  ReMutexHolder holder(_lock);

  EnqueuedTextures::iterator qi = _enqueued_textures.find(tex);
  if (qi != _enqueued_textures.end()) {
    if (qi->second != nullptr) {
      qi->second->notify_removed();
    }
    _enqueued_textures.erase(qi);
    return true;
  }
  return false;
}

/**
 * Returns true if the texture has been prepared on this GSG, false otherwise.
 */
bool PreparedGraphicsObjects::
is_texture_prepared(const Texture *tex) const {
  return tex->is_prepared((PreparedGraphicsObjects *)this);
}

/**
 * Indicates that a texture context, created by a previous call to
 * prepare_texture(), is no longer needed.  The driver resources will not be
 * freed until some GSG calls update(), indicating it is at a stage where it
 * is ready to release textures--this prevents conflicts from threading or
 * multiple GSG's sharing textures (we have no way of knowing which graphics
 * context is currently active, or what state it's in, at the time
 * release_texture is called).
 */
void PreparedGraphicsObjects::
release_texture(TextureContext *tc) {
  ReMutexHolder holder(_lock);

  tc->get_texture()->clear_prepared(tc->get_view(), this);

  // We have to set the Texture pointer to NULL at this point, since the
  // Texture itself might destruct at any time after it has been released.
  tc->_object = nullptr;

  bool removed = (_prepared_textures.erase(tc) != 0);
  nassertv(removed);

  _released_textures.push_back(tc);
}

/**
 * Releases a texture if it has already been prepared, or removes it from the
 * preparation queue.
 */
void PreparedGraphicsObjects::
release_texture(Texture *tex) {
  tex->release(this);
}

/**
 * Releases all textures at once.  This will force them to be reloaded into
 * texture memory for all GSG's that share this object.  Returns the number of
 * textures released.
 */
int PreparedGraphicsObjects::
release_all_textures() {
  ReMutexHolder holder(_lock);

  int num_textures = (int)_prepared_textures.size() + (int)_enqueued_textures.size();

  Textures::iterator tci;
  for (tci = _prepared_textures.begin();
       tci != _prepared_textures.end();
       ++tci) {
    TextureContext *tc = (*tci);
    tc->get_texture()->clear_prepared(tc->get_view(), this);
    tc->_object = nullptr;

    _released_textures.push_back(tc);
  }

  _prepared_textures.clear();

  // Mark any futures as cancelled.
  EnqueuedTextures::iterator qti;
  for (qti = _enqueued_textures.begin();
       qti != _enqueued_textures.end();
       ++qti) {
    if (qti->second != nullptr) {
      qti->second->notify_removed();
    }
  }

  _enqueued_textures.clear();

  return num_textures;
}

/**
 * Returns the number of textures that have been enqueued to be prepared on
 * this GSG.
 */
int PreparedGraphicsObjects::
get_num_queued_textures() const {
  return _enqueued_textures.size();
}

/**
 * Returns the number of textures that have already been prepared on this GSG.
 */
int PreparedGraphicsObjects::
get_num_prepared_textures() const {
  return _prepared_textures.size();
}

/**
 * Immediately creates a new TextureContext for the indicated texture and
 * returns it.  This assumes that the GraphicsStateGuardian is the currently
 * active rendering context and that it is ready to accept new textures.  If
 * this is not necessarily the case, you should use enqueue_texture() instead.
 *
 * Normally, this function is not called directly.  Call
 * Texture::prepare_now() instead.
 *
 * The TextureContext contains all of the pertinent information needed by the
 * GSG to keep track of this one particular texture, and will exist as long as
 * the texture is ready to be rendered.
 *
 * When either the Texture or the PreparedGraphicsObjects object destructs,
 * the TextureContext will be deleted.
 */
TextureContext *PreparedGraphicsObjects::
prepare_texture_now(Texture *tex, int view, GraphicsStateGuardianBase *gsg) {
  ReMutexHolder holder(_lock);

  // Ask the GSG to create a brand new TextureContext.  There might be several
  // GSG's sharing the same set of textures; if so, it doesn't matter which of
  // them creates the context (since they're all shared anyway).
  TextureContext *tc = gsg->prepare_texture(tex, view);

  if (tc != nullptr) {
    bool prepared = _prepared_textures.insert(tc).second;
    nassertr(prepared, tc);
  }

  return tc;
}

/**
 * Indicates that a sampler would like to be put on the list to be prepared
 * when the GSG is next ready to do this (presumably at the next frame).
 */
void PreparedGraphicsObjects::
enqueue_sampler(const SamplerState &sampler) {
  ReMutexHolder holder(_lock);

  _enqueued_samplers.insert(sampler);
}

/**
 * Returns true if the sampler has been queued on this GSG, false otherwise.
 */
bool PreparedGraphicsObjects::
is_sampler_queued(const SamplerState &sampler) const {
  ReMutexHolder holder(_lock);

  EnqueuedSamplers::const_iterator qi = _enqueued_samplers.find(sampler);
  return (qi != _enqueued_samplers.end());
}

/**
 * Removes a sampler from the queued list of samplers to be prepared.
 * Normally it is not necessary to call this, unless you change your mind
 * about preparing it at the last minute, since the sampler will automatically
 * be dequeued and prepared at the next frame.
 *
 * The return value is true if the sampler is successfully dequeued, false if
 * it had not been queued.
 */
bool PreparedGraphicsObjects::
dequeue_sampler(const SamplerState &sampler) {
  ReMutexHolder holder(_lock);

  EnqueuedSamplers::iterator qi = _enqueued_samplers.find(sampler);
  if (qi != _enqueued_samplers.end()) {
    _enqueued_samplers.erase(qi);
    return true;
  }
  return false;
}

/**
 * Returns true if the sampler has been prepared on this GSG, false otherwise.
 */
bool PreparedGraphicsObjects::
is_sampler_prepared(const SamplerState &sampler) const {
  ReMutexHolder holder(_lock);

  PreparedSamplers::const_iterator it = _prepared_samplers.find(sampler);
  return (it != _prepared_samplers.end());
}

/**
 * Indicates that a sampler context, created by a previous call to
 * prepare_sampler(), is no longer needed.  The driver resources will not be
 * freed until some GSG calls update(), indicating it is at a stage where it
 * is ready to release samplers.
 */
void PreparedGraphicsObjects::
release_sampler(SamplerContext *sc) {
  ReMutexHolder holder(_lock);

  _released_samplers.insert(sc);
}

/**
 * Releases a sampler if it has already been prepared, or removes it from the
 * preparation queue.
 */
void PreparedGraphicsObjects::
release_sampler(const SamplerState &sampler) {
  ReMutexHolder holder(_lock);

  PreparedSamplers::iterator it = _prepared_samplers.find(sampler);
  if (it != _prepared_samplers.end()) {
    _released_samplers.insert(it->second);
    _prepared_samplers.erase(it);
  }

  _enqueued_samplers.erase(sampler);
}

/**
 * Releases all samplers at once.  This will force them to be reloaded for all
 * GSG's that share this object.  Returns the number of samplers released.
 */
int PreparedGraphicsObjects::
release_all_samplers() {
  ReMutexHolder holder(_lock);

  int num_samplers = (int)_prepared_samplers.size() + (int)_enqueued_samplers.size();

  PreparedSamplers::iterator sci;
  for (sci = _prepared_samplers.begin();
       sci != _prepared_samplers.end();
       ++sci) {
    _released_samplers.insert(sci->second);
  }

  _prepared_samplers.clear();
  _enqueued_samplers.clear();

  return num_samplers;
}

/**
 * Returns the number of samplers that have been enqueued to be prepared on
 * this GSG.
 */
int PreparedGraphicsObjects::
get_num_queued_samplers() const {
  return _enqueued_samplers.size();
}

/**
 * Returns the number of samplers that have already been prepared on this GSG.
 */
int PreparedGraphicsObjects::
get_num_prepared_samplers() const {
  return _prepared_samplers.size();
}

/**
 * Immediately creates a new SamplerContext for the indicated sampler and
 * returns it.  This assumes that the GraphicsStateGuardian is the currently
 * active rendering context and that it is ready to accept new samplers.  If
 * this is not necessarily the case, you should use enqueue_sampler() instead.
 *
 * Normally, this function is not called directly.  Call
 * Sampler::prepare_now() instead.
 *
 * The SamplerContext contains all of the pertinent information needed by the
 * GSG to keep track of this one particular sampler, and will exist as long as
 * the sampler is ready to be rendered.
 *
 * When either the Sampler or the PreparedGraphicsObjects object destructs,
 * the SamplerContext will be deleted.
 */
SamplerContext *PreparedGraphicsObjects::
prepare_sampler_now(const SamplerState &sampler, GraphicsStateGuardianBase *gsg) {
  ReMutexHolder holder(_lock);

  PreparedSamplers::const_iterator it = _prepared_samplers.find(sampler);
  if (it != _prepared_samplers.end()) {
    return it->second;
  }

  // Ask the GSG to create a brand new SamplerContext.
  SamplerContext *sc = gsg->prepare_sampler(sampler);

  if (sc != nullptr) {
    _prepared_samplers[sampler] = sc;
  }

  return sc;
}

/**
 * Indicates that a geom would like to be put on the list to be prepared when
 * the GSG is next ready to do this (presumably at the next frame).
 */
void PreparedGraphicsObjects::
enqueue_geom(Geom *geom) {
  ReMutexHolder holder(_lock);

  _enqueued_geoms.insert(geom);
}

/**
 * Returns true if the geom has been queued on this GSG, false otherwise.
 */
bool PreparedGraphicsObjects::
is_geom_queued(const Geom *geom) const {
  ReMutexHolder holder(_lock);

  EnqueuedGeoms::const_iterator qi = _enqueued_geoms.find((Geom *)geom);
  return (qi != _enqueued_geoms.end());
}

/**
 * Removes a geom from the queued list of geoms to be prepared.  Normally it
 * is not necessary to call this, unless you change your mind about preparing
 * it at the last minute, since the geom will automatically be dequeued and
 * prepared at the next frame.
 *
 * The return value is true if the geom is successfully dequeued, false if it
 * had not been queued.
 */
bool PreparedGraphicsObjects::
dequeue_geom(Geom *geom) {
  ReMutexHolder holder(_lock);

  EnqueuedGeoms::iterator qi = _enqueued_geoms.find(geom);
  if (qi != _enqueued_geoms.end()) {
    _enqueued_geoms.erase(qi);
    return true;
  }
  return false;
}

/**
 * Returns true if the vertex buffer has been prepared on this GSG, false
 * otherwise.
 */
bool PreparedGraphicsObjects::
is_geom_prepared(const Geom *geom) const {
  return geom->is_prepared((PreparedGraphicsObjects *)this);
}

/**
 * Indicates that a geom context, created by a previous call to
 * prepare_geom(), is no longer needed.  The driver resources will not be
 * freed until some GSG calls update(), indicating it is at a stage where it
 * is ready to release geoms--this prevents conflicts from threading or
 * multiple GSG's sharing geoms (we have no way of knowing which graphics
 * context is currently active, or what state it's in, at the time
 * release_geom is called).
 */
void PreparedGraphicsObjects::
release_geom(GeomContext *gc) {
  ReMutexHolder holder(_lock);

  gc->_geom->clear_prepared(this);

  // We have to set the Geom pointer to NULL at this point, since the Geom
  // itself might destruct at any time after it has been released.
  gc->_geom = nullptr;

  bool removed = (_prepared_geoms.erase(gc) != 0);
  nassertv(removed);

  _released_geoms.insert(gc);
}

/**
 * Releases all geoms at once.  This will force them to be reloaded into geom
 * memory for all GSG's that share this object.  Returns the number of geoms
 * released.
 */
int PreparedGraphicsObjects::
release_all_geoms() {
  ReMutexHolder holder(_lock);

  int num_geoms = (int)_prepared_geoms.size() + (int)_enqueued_geoms.size();

  Geoms::iterator gci;
  for (gci = _prepared_geoms.begin();
       gci != _prepared_geoms.end();
       ++gci) {
    GeomContext *gc = (*gci);
    gc->_geom->clear_prepared(this);
    gc->_geom = nullptr;

    _released_geoms.insert(gc);
  }

  _prepared_geoms.clear();
  _enqueued_geoms.clear();

  return num_geoms;
}

/**
 * Returns the number of geoms that have been enqueued to be prepared on this
 * GSG.
 */
int PreparedGraphicsObjects::
get_num_queued_geoms() const {
  return _enqueued_geoms.size();
}

/**
 * Returns the number of geoms that have already been prepared on this GSG.
 */
int PreparedGraphicsObjects::
get_num_prepared_geoms() const {
  return _prepared_geoms.size();
}

/**
 * Immediately creates a new GeomContext for the indicated geom and returns
 * it.  This assumes that the GraphicsStateGuardian is the currently active
 * rendering context and that it is ready to accept new geoms.  If this is not
 * necessarily the case, you should use enqueue_geom() instead.
 *
 * Normally, this function is not called directly.  Call Geom::prepare_now()
 * instead.
 *
 * The GeomContext contains all of the pertinent information needed by the GSG
 * to keep track of this one particular geom, and will exist as long as the
 * geom is ready to be rendered.
 *
 * When either the Geom or the PreparedGraphicsObjects object destructs, the
 * GeomContext will be deleted.
 */
GeomContext *PreparedGraphicsObjects::
prepare_geom_now(Geom *geom, GraphicsStateGuardianBase *gsg) {
  ReMutexHolder holder(_lock);

  // Ask the GSG to create a brand new GeomContext.  There might be several
  // GSG's sharing the same set of geoms; if so, it doesn't matter which of
  // them creates the context (since they're all shared anyway).
  GeomContext *gc = gsg->prepare_geom(geom);

  if (gc != nullptr) {
    bool prepared = _prepared_geoms.insert(gc).second;
    nassertr(prepared, gc);
  }

  return gc;
}

/**
 * Indicates that a shader would like to be put on the list to be prepared
 * when the GSG is next ready to do this (presumably at the next frame).
 */
void PreparedGraphicsObjects::
enqueue_shader(Shader *shader) {
  ReMutexHolder holder(_lock);

  _enqueued_shaders.insert(EnqueuedShaders::value_type(shader, nullptr));
}

/**
 * Like enqueue_shader, but returns an AsyncFuture that can be used to query
 * the status of the shader's preparation.
 */
PT(PreparedGraphicsObjects::EnqueuedObject) PreparedGraphicsObjects::
enqueue_shader_future(Shader *shader) {
  ReMutexHolder holder(_lock);

  std::pair<EnqueuedShaders::iterator, bool> result =
    _enqueued_shaders.insert(EnqueuedShaders::value_type(shader, nullptr));
  if (result.first->second == nullptr) {
    result.first->second = new EnqueuedObject(this, shader);
  }
  PT(EnqueuedObject) fut = result.first->second;
  nassertr(!fut->cancelled(), fut)
  return fut;
}

/**
 * Returns true if the shader has been queued on this GSG, false otherwise.
 */
bool PreparedGraphicsObjects::
is_shader_queued(const Shader *shader) const {
  ReMutexHolder holder(_lock);

  EnqueuedShaders::const_iterator qi = _enqueued_shaders.find((Shader *)shader);
  return (qi != _enqueued_shaders.end());
}

/**
 * Removes a shader from the queued list of shaders to be prepared.  Normally
 * it is not necessary to call this, unless you change your mind about
 * preparing it at the last minute, since the shader will automatically be
 * dequeued and prepared at the next frame.
 *
 * The return value is true if the shader is successfully dequeued, false if
 * it had not been queued.
 */
bool PreparedGraphicsObjects::
dequeue_shader(Shader *se) {
  ReMutexHolder holder(_lock);

  EnqueuedShaders::iterator qi = _enqueued_shaders.find(se);
  if (qi != _enqueued_shaders.end()) {
    if (qi->second != nullptr) {
      qi->second->notify_removed();
    }
    _enqueued_shaders.erase(qi);
    return true;
  }
  return false;
}

/**
 * Returns true if the shader has been prepared on this GSG, false otherwise.
 */
bool PreparedGraphicsObjects::
is_shader_prepared(const Shader *shader) const {
  return shader->is_prepared((PreparedGraphicsObjects *)this);
}

/**
 * Indicates that a shader context, created by a previous call to
 * prepare_shader(), is no longer needed.  The driver resources will not be
 * freed until some GSG calls update(), indicating it is at a stage where it
 * is ready to release shaders--this prevents conflicts from threading or
 * multiple GSG's sharing shaders (we have no way of knowing which graphics
 * context is currently active, or what state it's in, at the time
 * release_shader is called).
 */
void PreparedGraphicsObjects::
release_shader(ShaderContext *sc) {
  ReMutexHolder holder(_lock);

  sc->_shader->clear_prepared(this);

  // We have to set the Shader pointer to NULL at this point, since the Shader
  // itself might destruct at any time after it has been released.
  sc->_shader = nullptr;

  bool removed = (_prepared_shaders.erase(sc) != 0);
  nassertv(removed);

  _released_shaders.insert(sc);
}

/**
 * Releases all shaders at once.  This will force them to be reloaded into
 * shader memory for all GSG's that share this object.  Returns the number of
 * shaders released.
 */
int PreparedGraphicsObjects::
release_all_shaders() {
  ReMutexHolder holder(_lock);

  int num_shaders = (int)_prepared_shaders.size() + (int)_enqueued_shaders.size();

  Shaders::iterator sci;
  for (sci = _prepared_shaders.begin();
       sci != _prepared_shaders.end();
       ++sci) {
    ShaderContext *sc = (*sci);
    sc->_shader->clear_prepared(this);
    sc->_shader = nullptr;

    _released_shaders.insert(sc);
  }

  _prepared_shaders.clear();

  // Mark any futures as cancelled.
  EnqueuedShaders::iterator qsi;
  for (qsi = _enqueued_shaders.begin();
       qsi != _enqueued_shaders.end();
       ++qsi) {
    if (qsi->second != nullptr) {
      qsi->second->notify_removed();
    }
  }

  _enqueued_shaders.clear();

  return num_shaders;
}

/**
 * Returns the number of shaders that have been enqueued to be prepared on
 * this GSG.
 */
int PreparedGraphicsObjects::
get_num_queued_shaders() const {
  return _enqueued_shaders.size();
}

/**
 * Returns the number of shaders that have already been prepared on this GSG.
 */
int PreparedGraphicsObjects::
get_num_prepared_shaders() const {
  return _prepared_shaders.size();
}

/**
 * Immediately creates a new ShaderContext for the indicated shader and
 * returns it.  This assumes that the GraphicsStateGuardian is the currently
 * active rendering context and that it is ready to accept new shaders.  If
 * this is not necessarily the case, you should use enqueue_shader() instead.
 *
 * Normally, this function is not called directly.  Call Shader::prepare_now()
 * instead.
 *
 * The ShaderContext contains all of the pertinent information needed by the
 * GSG to keep track of this one particular shader, and will exist as long as
 * the shader is ready to be rendered.
 *
 * When either the Shader or the PreparedGraphicsObjects object destructs, the
 * ShaderContext will be deleted.
 */
ShaderContext *PreparedGraphicsObjects::
prepare_shader_now(Shader *se, GraphicsStateGuardianBase *gsg) {
  ReMutexHolder holder(_lock);

  // Ask the GSG to create a brand new ShaderContext.  There might be several
  // GSG's sharing the same set of shaders; if so, it doesn't matter which of
  // them creates the context (since they're all shared anyway).
  ShaderContext *sc = gsg->prepare_shader(se);

  if (sc != nullptr) {
    bool prepared = _prepared_shaders.insert(sc).second;
    nassertr(prepared, sc);
  }

  return sc;
}

/**
 * Indicates that a buffer would like to be put on the list to be prepared
 * when the GSG is next ready to do this (presumably at the next frame).
 */
void PreparedGraphicsObjects::
enqueue_vertex_buffer(GeomVertexArrayData *data) {
  ReMutexHolder holder(_lock);

  _enqueued_vertex_buffers.insert(data);
}

/**
 * Returns true if the vertex buffer has been queued on this GSG, false
 * otherwise.
 */
bool PreparedGraphicsObjects::
is_vertex_buffer_queued(const GeomVertexArrayData *data) const {
  ReMutexHolder holder(_lock);

  EnqueuedVertexBuffers::const_iterator qi = _enqueued_vertex_buffers.find((GeomVertexArrayData *)data);
  return (qi != _enqueued_vertex_buffers.end());
}

/**
 * Removes a buffer from the queued list of data arrays to be prepared.
 * Normally it is not necessary to call this, unless you change your mind
 * about preparing it at the last minute, since the data will automatically be
 * dequeued and prepared at the next frame.
 *
 * The return value is true if the buffer is successfully dequeued, false if
 * it had not been queued.
 */
bool PreparedGraphicsObjects::
dequeue_vertex_buffer(GeomVertexArrayData *data) {
  ReMutexHolder holder(_lock);

  EnqueuedVertexBuffers::iterator qi = _enqueued_vertex_buffers.find(data);
  if (qi != _enqueued_vertex_buffers.end()) {
    _enqueued_vertex_buffers.erase(qi);
    return true;
  }
  return false;
}

/**
 * Returns true if the vertex buffer has been prepared on this GSG, false
 * otherwise.
 */
bool PreparedGraphicsObjects::
is_vertex_buffer_prepared(const GeomVertexArrayData *data) const {
  return data->is_prepared((PreparedGraphicsObjects *)this);
}

/**
 * Indicates that a data context, created by a previous call to
 * prepare_vertex_buffer(), is no longer needed.  The driver resources will
 * not be freed until some GSG calls update(), indicating it is at a stage
 * where it is ready to release datas--this prevents conflicts from threading
 * or multiple GSG's sharing datas (we have no way of knowing which graphics
 * context is currently active, or what state it's in, at the time
 * release_vertex_buffer is called).
 */
void PreparedGraphicsObjects::
release_vertex_buffer(VertexBufferContext *vbc) {
  ReMutexHolder holder(_lock);

  vbc->get_data()->clear_prepared(this);

  size_t data_size_bytes = vbc->get_data()->get_data_size_bytes();
  GeomEnums::UsageHint usage_hint = vbc->get_data()->get_usage_hint();

  // We have to set the Data pointer to NULL at this point, since the Data
  // itself might destruct at any time after it has been released.
  vbc->_object = nullptr;

  bool removed = (_prepared_vertex_buffers.erase(vbc) != 0);
  nassertv(removed);

  if (_support_released_buffer_cache) {
    cache_unprepared_buffer(vbc, data_size_bytes, usage_hint,
                            _vertex_buffer_cache,
                            _vertex_buffer_cache_lru, _vertex_buffer_cache_size,
                            released_vbuffer_cache_size,
                            _released_vertex_buffers);
  } else {
    _released_vertex_buffers.push_back(vbc);
  }
}

/**
 * Releases all datas at once.  This will force them to be reloaded into data
 * memory for all GSG's that share this object.  Returns the number of datas
 * released.
 */
int PreparedGraphicsObjects::
release_all_vertex_buffers() {
  ReMutexHolder holder(_lock);

  int num_vertex_buffers = (int)_prepared_vertex_buffers.size() + (int)_enqueued_vertex_buffers.size();

  Buffers::iterator vbci;
  for (vbci = _prepared_vertex_buffers.begin();
       vbci != _prepared_vertex_buffers.end();
       ++vbci) {
    VertexBufferContext *vbc = (VertexBufferContext *)(*vbci);
    vbc->get_data()->clear_prepared(this);
    vbc->_object = nullptr;

    _released_vertex_buffers.push_back(vbc);
  }

  _prepared_vertex_buffers.clear();
  _enqueued_vertex_buffers.clear();

  // Also clear the cache of recently-unprepared vertex buffers.
  BufferCache::iterator bci;
  for (bci = _vertex_buffer_cache.begin();
       bci != _vertex_buffer_cache.end();
       ++bci) {
    BufferList &buffer_list = (*bci).second;
    nassertr(!buffer_list.empty(), num_vertex_buffers);
    BufferList::iterator li;
    for (li = buffer_list.begin(); li != buffer_list.end(); ++li) {
      VertexBufferContext *vbc = (VertexBufferContext *)(*li);
      _released_vertex_buffers.push_back(vbc);
    }
  }
  _vertex_buffer_cache.clear();
  _vertex_buffer_cache_lru.clear();
  _vertex_buffer_cache_size = 0;

  return num_vertex_buffers;
}

/**
 * Returns the number of vertex buffers that have been enqueued to be prepared
 * on this GSG.
 */
int PreparedGraphicsObjects::
get_num_queued_vertex_buffers() const {
  return _enqueued_vertex_buffers.size();
}

/**
 * Returns the number of vertex buffers that have already been prepared on
 * this GSG.
 */
int PreparedGraphicsObjects::
get_num_prepared_vertex_buffers() const {
  return _prepared_vertex_buffers.size();
}

/**
 * Immediately creates a new VertexBufferContext for the indicated data and
 * returns it.  This assumes that the GraphicsStateGuardian is the currently
 * active rendering context and that it is ready to accept new datas.  If this
 * is not necessarily the case, you should use enqueue_vertex_buffer()
 * instead.
 *
 * Normally, this function is not called directly.  Call Data::prepare_now()
 * instead.
 *
 * The VertexBufferContext contains all of the pertinent information needed by
 * the GSG to keep track of this one particular data, and will exist as long
 * as the data is ready to be rendered.
 *
 * When either the Data or the PreparedGraphicsObjects object destructs, the
 * VertexBufferContext will be deleted.
 */
VertexBufferContext *PreparedGraphicsObjects::
prepare_vertex_buffer_now(GeomVertexArrayData *data, GraphicsStateGuardianBase *gsg) {
  ReMutexHolder holder(_lock);

  // First, see if there might be a cached context of the appropriate size.
  size_t data_size_bytes = data->get_data_size_bytes();
  GeomEnums::UsageHint usage_hint = data->get_usage_hint();
  VertexBufferContext *vbc = (VertexBufferContext *)
    get_cached_buffer(data_size_bytes, usage_hint,
                      _vertex_buffer_cache, _vertex_buffer_cache_lru,
                      _vertex_buffer_cache_size);
  if (vbc != nullptr) {
    vbc->_object = data;

  } else {
    // Ask the GSG to create a brand new VertexBufferContext.  There might be
    // several GSG's sharing the same set of datas; if so, it doesn't matter
    // which of them creates the context (since they're all shared anyway).
    vbc = gsg->prepare_vertex_buffer(data);
  }

  if (vbc != nullptr) {
    bool prepared = _prepared_vertex_buffers.insert(vbc).second;
    nassertr(prepared, vbc);
  }

  return vbc;
}

/**
 * Indicates that a buffer would like to be put on the list to be prepared
 * when the GSG is next ready to do this (presumably at the next frame).
 */
void PreparedGraphicsObjects::
enqueue_index_buffer(GeomPrimitive *data) {
  ReMutexHolder holder(_lock);

  _enqueued_index_buffers.insert(data);
}

/**
 * Returns true if the index buffer has been queued on this GSG, false
 * otherwise.
 */
bool PreparedGraphicsObjects::
is_index_buffer_queued(const GeomPrimitive *data) const {
  ReMutexHolder holder(_lock);

  EnqueuedIndexBuffers::const_iterator qi = _enqueued_index_buffers.find((GeomPrimitive *)data);
  return (qi != _enqueued_index_buffers.end());
}

/**
 * Removes a buffer from the queued list of data arrays to be prepared.
 * Normally it is not necessary to call this, unless you change your mind
 * about preparing it at the last minute, since the data will automatically be
 * dequeued and prepared at the next frame.
 *
 * The return value is true if the buffer is successfully dequeued, false if
 * it had not been queued.
 */
bool PreparedGraphicsObjects::
dequeue_index_buffer(GeomPrimitive *data) {
  ReMutexHolder holder(_lock);

  EnqueuedIndexBuffers::iterator qi = _enqueued_index_buffers.find(data);
  if (qi != _enqueued_index_buffers.end()) {
    _enqueued_index_buffers.erase(qi);
    return true;
  }
  return false;
}

/**
 * Returns true if the index buffer has been prepared on this GSG, false
 * otherwise.
 */
bool PreparedGraphicsObjects::
is_index_buffer_prepared(const GeomPrimitive *data) const {
  return data->is_prepared((PreparedGraphicsObjects *)this);
}

/**
 * Indicates that a data context, created by a previous call to
 * prepare_index_buffer(), is no longer needed.  The driver resources will not
 * be freed until some GSG calls update(), indicating it is at a stage where
 * it is ready to release datas--this prevents conflicts from threading or
 * multiple GSG's sharing datas (we have no way of knowing which graphics
 * context is currently active, or what state it's in, at the time
 * release_index_buffer is called).
 */
void PreparedGraphicsObjects::
release_index_buffer(IndexBufferContext *ibc) {
  ReMutexHolder holder(_lock);

  ibc->get_data()->clear_prepared(this);

  size_t data_size_bytes = ibc->get_data()->get_data_size_bytes();
  GeomEnums::UsageHint usage_hint = ibc->get_data()->get_usage_hint();

  // We have to set the Data pointer to NULL at this point, since the Data
  // itself might destruct at any time after it has been released.
  ibc->_object = nullptr;

  bool removed = (_prepared_index_buffers.erase(ibc) != 0);
  nassertv(removed);

  if (_support_released_buffer_cache) {
    cache_unprepared_buffer(ibc, data_size_bytes, usage_hint,
                            _index_buffer_cache,
                            _index_buffer_cache_lru, _index_buffer_cache_size,
                            released_ibuffer_cache_size,
                            _released_index_buffers);
  } else {
    _released_index_buffers.push_back(ibc);
  }
}

/**
 * Releases all datas at once.  This will force them to be reloaded into data
 * memory for all GSG's that share this object.  Returns the number of datas
 * released.
 */
int PreparedGraphicsObjects::
release_all_index_buffers() {
  ReMutexHolder holder(_lock);

  int num_index_buffers = (int)_prepared_index_buffers.size() + (int)_enqueued_index_buffers.size();

  Buffers::iterator ibci;
  for (ibci = _prepared_index_buffers.begin();
       ibci != _prepared_index_buffers.end();
       ++ibci) {
    IndexBufferContext *ibc = (IndexBufferContext *)(*ibci);
    ibc->get_data()->clear_prepared(this);
    ibc->_object = nullptr;

    _released_index_buffers.push_back(ibc);
  }

  _prepared_index_buffers.clear();
  _enqueued_index_buffers.clear();

  // Also clear the cache of recently-unprepared index buffers.
  BufferCache::iterator bci;
  for (bci = _index_buffer_cache.begin();
       bci != _index_buffer_cache.end();
       ++bci) {
    BufferList &buffer_list = (*bci).second;
    nassertr(!buffer_list.empty(), num_index_buffers);
    BufferList::iterator li;
    for (li = buffer_list.begin(); li != buffer_list.end(); ++li) {
      IndexBufferContext *vbc = (IndexBufferContext *)(*li);
      _released_index_buffers.push_back(vbc);
    }
  }
  _index_buffer_cache.clear();
  _index_buffer_cache_lru.clear();
  _index_buffer_cache_size = 0;

  return num_index_buffers;
}

/**
 * Returns the number of index buffers that have been enqueued to be prepared
 * on this GSG.
 */
int PreparedGraphicsObjects::
get_num_queued_index_buffers() const {
  return _enqueued_index_buffers.size();
}

/**
 * Returns the number of index buffers that have already been prepared on this
 * GSG.
 */
int PreparedGraphicsObjects::
get_num_prepared_index_buffers() const {
  return _prepared_index_buffers.size();
}

/**
 * Immediately creates a new IndexBufferContext for the indicated data and
 * returns it.  This assumes that the GraphicsStateGuardian is the currently
 * active rendering context and that it is ready to accept new datas.  If this
 * is not necessarily the case, you should use enqueue_index_buffer() instead.
 *
 * Normally, this function is not called directly.  Call Data::prepare_now()
 * instead.
 *
 * The IndexBufferContext contains all of the pertinent information needed by
 * the GSG to keep track of this one particular data, and will exist as long
 * as the data is ready to be rendered.
 *
 * When either the Data or the PreparedGraphicsObjects object destructs, the
 * IndexBufferContext will be deleted.
 */
IndexBufferContext *PreparedGraphicsObjects::
prepare_index_buffer_now(GeomPrimitive *data, GraphicsStateGuardianBase *gsg) {
  ReMutexHolder holder(_lock);

  // First, see if there might be a cached context of the appropriate size.
  size_t data_size_bytes = data->get_data_size_bytes();
  GeomEnums::UsageHint usage_hint = data->get_usage_hint();
  IndexBufferContext *ibc = (IndexBufferContext *)
    get_cached_buffer(data_size_bytes, usage_hint,
                      _index_buffer_cache, _index_buffer_cache_lru,
                      _index_buffer_cache_size);
  if (ibc != nullptr) {
    ibc->_object = data;

  } else {
    // Ask the GSG to create a brand new IndexBufferContext.  There might be
    // several GSG's sharing the same set of datas; if so, it doesn't matter
    // which of them creates the context (since they're all shared anyway).
    ibc = gsg->prepare_index_buffer(data);
  }

  if (ibc != nullptr) {
    bool prepared = _prepared_index_buffers.insert(ibc).second;
    nassertr(prepared, ibc);
  }

  return ibc;
}

/**
 * Indicates that a buffer would like to be put on the list to be prepared
 * when the GSG is next ready to do this (presumably at the next frame).
 */
void PreparedGraphicsObjects::
enqueue_shader_buffer(ShaderBuffer *data) {
  ReMutexHolder holder(_lock);

  _enqueued_shader_buffers.insert(data);
}

/**
 * Returns true if the index buffer has been queued on this GSG, false
 * otherwise.
 */
bool PreparedGraphicsObjects::
is_shader_buffer_queued(const ShaderBuffer *data) const {
  ReMutexHolder holder(_lock);

  EnqueuedShaderBuffers::const_iterator qi = _enqueued_shader_buffers.find((ShaderBuffer *)data);
  return (qi != _enqueued_shader_buffers.end());
}

/**
 * Removes a buffer from the queued list of data arrays to be prepared.
 * Normally it is not necessary to call this, unless you change your mind
 * about preparing it at the last minute, since the data will automatically be
 * dequeued and prepared at the next frame.
 *
 * The return value is true if the buffer is successfully dequeued, false if
 * it had not been queued.
 */
bool PreparedGraphicsObjects::
dequeue_shader_buffer(ShaderBuffer *data) {
  ReMutexHolder holder(_lock);

  EnqueuedShaderBuffers::iterator qi = _enqueued_shader_buffers.find(data);
  if (qi != _enqueued_shader_buffers.end()) {
    _enqueued_shader_buffers.erase(qi);
    return true;
  }
  return false;
}

/**
 * Returns true if the index buffer has been prepared on this GSG, false
 * otherwise.
 */
bool PreparedGraphicsObjects::
is_shader_buffer_prepared(const ShaderBuffer *data) const {
  return data->is_prepared((PreparedGraphicsObjects *)this);
}

/**
 * Indicates that a data context, created by a previous call to
 * prepare_shader_buffer(), is no longer needed.  The driver resources will not
 * be freed until some GSG calls update(), indicating it is at a stage where
 * it is ready to release datas--this prevents conflicts from threading or
 * multiple GSG's sharing datas (we have no way of knowing which graphics
 * context is currently active, or what state it's in, at the time
 * release_shader_buffer is called).
 */
void PreparedGraphicsObjects::
release_shader_buffer(BufferContext *bc) {
  ReMutexHolder holder(_lock);

  ShaderBuffer *buffer = (ShaderBuffer *)bc->_object;
  buffer->clear_prepared(this);

  // We have to set the ShaderBuffer pointer to NULL at this point, since the
  // buffer itself might destruct at any time after it has been released.
  bc->_object = nullptr;

  bool removed = (_prepared_shader_buffers.erase(bc) != 0);
  nassertv(removed);

  _released_shader_buffers.push_back(bc);
}

/**
 * Releases all datas at once.  This will force them to be reloaded into data
 * memory for all GSG's that share this object.  Returns the number of datas
 * released.
 */
int PreparedGraphicsObjects::
release_all_shader_buffers() {
  ReMutexHolder holder(_lock);

  int num_shader_buffers = (int)_prepared_shader_buffers.size() + (int)_enqueued_shader_buffers.size();

  Buffers::iterator bci;
  for (bci = _prepared_shader_buffers.begin();
       bci != _prepared_shader_buffers.end();
       ++bci) {

    BufferContext *bc = (BufferContext *)(*bci);
    _released_shader_buffers.push_back(bc);
  }

  _prepared_shader_buffers.clear();
  _enqueued_shader_buffers.clear();

  return num_shader_buffers;
}

/**
 * Returns the number of index buffers that have been enqueued to be prepared
 * on this GSG.
 */
int PreparedGraphicsObjects::
get_num_queued_shader_buffers() const {
  return _enqueued_shader_buffers.size();
}

/**
 * Returns the number of index buffers that have already been prepared on this
 * GSG.
 */
int PreparedGraphicsObjects::
get_num_prepared_shader_buffers() const {
  return _prepared_shader_buffers.size();
}

/**
 * Immediately creates a new BufferContext for the indicated data and
 * returns it.  This assumes that the GraphicsStateGuardian is the currently
 * active rendering context and that it is ready to accept new datas.  If this
 * is not necessarily the case, you should use enqueue_shader_buffer() instead.
 *
 * Normally, this function is not called directly.  Call Data::prepare_now()
 * instead.
 *
 * The BufferContext contains all of the pertinent information needed by
 * the GSG to keep track of this one particular data, and will exist as long
 * as the data is ready to be rendered.
 *
 * When either the Data or the PreparedGraphicsObjects object destructs, the
 * BufferContext will be deleted.
 */
BufferContext *PreparedGraphicsObjects::
prepare_shader_buffer_now(ShaderBuffer *data, GraphicsStateGuardianBase *gsg) {
  ReMutexHolder holder(_lock);

  // Ask the GSG to create a brand new BufferContext.  There might be
  // several GSG's sharing the same set of datas; if so, it doesn't matter
  // which of them creates the context (since they're all shared anyway).
  BufferContext *bc = gsg->prepare_shader_buffer(data);

  if (bc != nullptr) {
    bool prepared = _prepared_shader_buffers.insert(bc).second;
    nassertr(prepared, bc);
  }

  return bc;
}

/**
 * Creates a new future for the given object.
 */
PreparedGraphicsObjects::EnqueuedObject::
EnqueuedObject(PreparedGraphicsObjects *pgo, TypedWritableReferenceCount *object) :
  _pgo(pgo),
  _object(object) {
}

/**
 * Indicates that the preparation request is done.
 */
void PreparedGraphicsObjects::EnqueuedObject::
set_result(SavedContext *context) {
  nassertv(!done());
  AsyncFuture::set_result(context);
  _pgo = nullptr;
}

/**
 * Called by PreparedGraphicsObjects to indicate that the preparation request
 * has been cancelled.
 */
void PreparedGraphicsObjects::EnqueuedObject::
notify_removed() {
  _pgo = nullptr;
  nassertv_always(AsyncFuture::cancel());
}

/**
 * Cancels the pending preparation request.  Has no effect if the preparation
 * is already complete or was already cancelled.
 */
bool PreparedGraphicsObjects::EnqueuedObject::
cancel() {
  PreparedGraphicsObjects *pgo = _pgo;
  if (_object == nullptr || pgo == nullptr) {
    nassertr(done(), false);
    return false;
  }

  // We don't upcall here, because the dequeue function will end up calling
  // notify_removed().
  _result = nullptr;
  _pgo = nullptr;

  if (_object->is_of_type(Texture::get_class_type())) {
    return pgo->dequeue_texture((Texture *)_object.p());

  } else if (_object->is_of_type(Geom::get_class_type())) {
    return pgo->dequeue_geom((Geom *)_object.p());

  } else if (_object->is_of_type(Shader::get_class_type())) {
    return pgo->dequeue_shader((Shader *)_object.p());

  } else if (_object->is_of_type(GeomVertexArrayData::get_class_type())) {
    return pgo->dequeue_vertex_buffer((GeomVertexArrayData *)_object.p());

  } else if (_object->is_of_type(GeomPrimitive::get_class_type())) {
    return pgo->dequeue_index_buffer((GeomPrimitive *)_object.p());

  } else if (_object->is_of_type(ShaderBuffer::get_class_type())) {
    return pgo->dequeue_shader_buffer((ShaderBuffer *)_object.p());
  }
  return false;
}

/**
 * This is called by the GraphicsStateGuardian to indicate that it is about to
 * begin processing of the frame.
 *
 * Any texture contexts that were previously passed to release_texture() are
 * actually passed to the GSG to be freed at this point; textures that were
 * previously passed to prepare_texture are actually loaded.
 */
void PreparedGraphicsObjects::
begin_frame(GraphicsStateGuardianBase *gsg, Thread *current_thread) {
  ReMutexHolder holder(_lock, current_thread);

  // First, release all the textures, geoms, and buffers awaiting release.
  if (!_released_textures.empty()) {
    gsg->release_textures(_released_textures);
    _released_textures.clear();
  }

  if (!_released_samplers.empty()) {
    for (SamplerContext *sc : _released_samplers) {
      gsg->release_sampler(sc);
    }
    _released_samplers.clear();
  }

  if (!_released_geoms.empty()) {
    for (GeomContext *gc : _released_geoms) {
      gsg->release_geom(gc);
    }
    _released_geoms.clear();
  }

  if (!_released_shaders.empty()) {
    for (ShaderContext *sc : _released_shaders) {
      gsg->release_shader(sc);
    }
    _released_shaders.clear();
  }

  if (!_released_vertex_buffers.empty()) {
    gsg->release_vertex_buffers(_released_vertex_buffers);
    _released_vertex_buffers.clear();
  }

  if (!_released_index_buffers.empty()) {
    gsg->release_index_buffers(_released_index_buffers);
    _released_index_buffers.clear();
  }

  if (!_released_shader_buffers.empty()) {
    gsg->release_shader_buffers(_released_shader_buffers);
    _released_shader_buffers.clear();
  }

  // Reset the residency trackers.
  _texture_residency.begin_frame(current_thread);
  _vbuffer_residency.begin_frame(current_thread);
  _ibuffer_residency.begin_frame(current_thread);
  _sbuffer_residency.begin_frame(current_thread);

  // Now prepare all the textures, geoms, and buffers awaiting preparation.
  EnqueuedTextures::iterator qti;
  for (qti = _enqueued_textures.begin();
       qti != _enqueued_textures.end();
       ++qti) {
    Texture *tex = qti->first;
    for (int view = 0; view < tex->get_num_views(); ++view) {
      TextureContext *tc = tex->prepare_now(view, this, gsg);
      if (tc != nullptr) {
        gsg->update_texture(tc, true);
        if (view == 0 && qti->second != nullptr) {
          qti->second->set_result(tc);
        }
      }
    }
  }

  _enqueued_textures.clear();

  EnqueuedSamplers::iterator qsmi;
  for (qsmi = _enqueued_samplers.begin();
       qsmi != _enqueued_samplers.end();
       ++qsmi) {
    const SamplerState &sampler = (*qsmi);
    sampler.prepare_now(this, gsg);
  }

  _enqueued_samplers.clear();

  EnqueuedGeoms::iterator qgi;
  for (qgi = _enqueued_geoms.begin();
       qgi != _enqueued_geoms.end();
       ++qgi) {
    Geom *geom = (*qgi);
    geom->prepare_now(this, gsg);
  }

  _enqueued_geoms.clear();

  EnqueuedShaders::iterator qsi;
  for (qsi = _enqueued_shaders.begin();
       qsi != _enqueued_shaders.end();
       ++qsi) {
    Shader *shader = qsi->first;
    ShaderContext *sc = shader->prepare_now(this, gsg);
    if (qsi->second != nullptr) {
      qsi->second->set_result(sc);
    }
  }

  _enqueued_shaders.clear();

  EnqueuedVertexBuffers::iterator qvbi;
  for (qvbi = _enqueued_vertex_buffers.begin();
       qvbi != _enqueued_vertex_buffers.end();
       ++qvbi) {
    GeomVertexArrayData *data = (*qvbi);
    data->prepare_now(this, gsg);
  }

  _enqueued_vertex_buffers.clear();

  EnqueuedIndexBuffers::iterator qibi;
  for (qibi = _enqueued_index_buffers.begin();
       qibi != _enqueued_index_buffers.end();
       ++qibi) {
    GeomPrimitive *data = (*qibi);
    // We need this check because the actual index data may not actually have
    // propagated to the draw thread yet.
    if (data->is_indexed()) {
      data->prepare_now(this, gsg);
    }
  }

  _enqueued_index_buffers.clear();
}

/**
 * This is called by the GraphicsStateGuardian to indicate that it has
 * finished processing of the frame.
 */
void PreparedGraphicsObjects::
end_frame(Thread *current_thread) {
  ReMutexHolder holder(_lock, current_thread);

  _texture_residency.end_frame(current_thread);
  _vbuffer_residency.end_frame(current_thread);
  _ibuffer_residency.end_frame(current_thread);
  _sbuffer_residency.end_frame(current_thread);
}

/**
 * Returns a new, unique name for a newly-constructed object.
 */
std::string PreparedGraphicsObjects::
init_name() {
  ++_name_index;
  std::ostringstream strm;
  strm << "context" << _name_index;
  return strm.str();
}

/**
 * Called when a vertex or index buffer is no longer officially "prepared".
 * However, we still have the context on the graphics card, and we might be
 * able to reuse that context if we're about to re-prepare a different buffer,
 * especially one exactly the same size.  So instead of immediately enqueuing
 * the vertex buffer for release, we cache it.
 */
void PreparedGraphicsObjects::
cache_unprepared_buffer(BufferContext *buffer, size_t data_size_bytes,
                        GeomEnums::UsageHint usage_hint,
                        PreparedGraphicsObjects::BufferCache &buffer_cache,
                        PreparedGraphicsObjects::BufferCacheLRU &buffer_cache_lru,
                        size_t &buffer_cache_size,
                        int released_buffer_cache_size,
                        pvector<BufferContext *> &released_buffers) {
  BufferCacheKey key;
  key._data_size_bytes = data_size_bytes;
  key._usage_hint = usage_hint;

  buffer_cache[key].push_back(buffer);
  buffer_cache_size += data_size_bytes;

  // Move the key to the head of the LRU.
  BufferCacheLRU::iterator li =
    find(buffer_cache_lru.begin(), buffer_cache_lru.end(), key);
  if (li != buffer_cache_lru.end()) {
    buffer_cache_lru.erase(li);
  }
  buffer_cache_lru.insert(buffer_cache_lru.begin(), key);

  // Now release not-recently-used buffers until we fit within the constrained
  // size.
  while ((int)buffer_cache_size > released_buffer_cache_size) {
    nassertv(!buffer_cache_lru.empty());
    const BufferCacheKey &release_key = *buffer_cache_lru.rbegin();
    BufferList &buffer_list = buffer_cache[release_key];
    while (!buffer_list.empty() &&
           (int)buffer_cache_size > released_buffer_cache_size) {
      BufferContext *released_buffer = buffer_list.back();
      buffer_list.pop_back();
      if (released_buffer->_object != nullptr) {
        released_buffer->_object = nullptr;
        released_buffers.push_back(released_buffer);
      }
      buffer_cache_size -= release_key._data_size_bytes;
    }

    if (buffer_list.empty()) {
      buffer_cache.erase(release_key);
      buffer_cache_lru.pop_back();
    }
  }
}

/**
 * Returns a previously-cached buffer from the cache, or NULL if there is no
 * such buffer.
 */
BufferContext *PreparedGraphicsObjects::
get_cached_buffer(size_t data_size_bytes, GeomEnums::UsageHint usage_hint,
                  PreparedGraphicsObjects::BufferCache &buffer_cache,
                  PreparedGraphicsObjects::BufferCacheLRU &buffer_cache_lru,
                  size_t &buffer_cache_size) {
  BufferCacheKey key;
  key._data_size_bytes = data_size_bytes;
  key._usage_hint = usage_hint;

  BufferCache::iterator bci = buffer_cache.find(key);
  if (bci == buffer_cache.end()) {
    return nullptr;
  }

  BufferList &buffer_list = (*bci).second;
  nassertr(!buffer_list.empty(), nullptr);

  BufferContext *buffer = buffer_list.back();
  buffer_list.pop_back();
  if (buffer_list.empty()) {
    buffer_cache.erase(bci);
    BufferCacheLRU::iterator li =
      find(buffer_cache_lru.begin(), buffer_cache_lru.end(), key);
    if (li != buffer_cache_lru.end()) {
      buffer_cache_lru.erase(li);
    }
  }

  buffer_cache_size -= data_size_bytes;
  return buffer;
}
