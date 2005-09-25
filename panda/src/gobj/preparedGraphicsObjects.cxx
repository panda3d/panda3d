// Filename: preparedGraphicsObjects.cxx
// Created by:  drose (19Feb04)
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

#include "preparedGraphicsObjects.h"
#include "textureContext.h"
#include "vertexBufferContext.h"
#include "indexBufferContext.h"
#include "texture.h"
#include "geom.h"
#include "geomVertexArrayData.h"
#include "geomPrimitive.h"
#include "shaderExpansion.h"
#include "mutexHolder.h"

PStatCollector PreparedGraphicsObjects::_total_texusage_pcollector("Texture usage");
PStatCollector PreparedGraphicsObjects::_total_buffers_pcollector("Vertex buffer size");

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PreparedGraphicsObjects::
PreparedGraphicsObjects() {
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PreparedGraphicsObjects::
~PreparedGraphicsObjects() {
  // There may be objects that are still prepared when we destruct.
  // If this is so, then all of the GSG's that own them have already
  // destructed, so we can assume their resources were internally
  // cleaned up.  Quietly erase these remaining objects.

  MutexHolder holder(_lock);

  Textures::iterator tci;
  for (tci = _prepared_textures.begin();
       tci != _prepared_textures.end();
       ++tci) {
    TextureContext *tc = (*tci);
    _total_texusage_pcollector.sub_level(tc->estimate_texture_memory());
    tc->_texture->clear_prepared(this);
  }

  _prepared_textures.clear();
  _released_textures.clear();
  _enqueued_textures.clear();

  Geoms::iterator gci;
  for (gci = _prepared_geoms.begin();
       gci != _prepared_geoms.end();
       ++gci) {
    GeomContext *gc = (*gci);
    gc->_geom->clear_prepared(this);
  }

  _prepared_geoms.clear();
  _released_geoms.clear();
  _enqueued_geoms.clear();

  Shaders::iterator sci;
  for (sci = _prepared_shaders.begin();
       sci != _prepared_shaders.end();
       ++sci) {
    ShaderContext *sc = (*sci);
    sc->_shader_expansion->clear_prepared(this);
  }

  _prepared_shaders.clear();
  _released_shaders.clear();
  _enqueued_shaders.clear();

  VertexBuffers::iterator vbci;
  for (vbci = _prepared_vertex_buffers.begin();
       vbci != _prepared_vertex_buffers.end();
       ++vbci) {
    VertexBufferContext *vbc = (*vbci);
    _total_buffers_pcollector.sub_level(vbc->get_data_size_bytes());
    vbc->_data->clear_prepared(this);
  }

  _prepared_vertex_buffers.clear();
  _released_vertex_buffers.clear();
  _enqueued_vertex_buffers.clear();

  IndexBuffers::iterator ibci;
  for (ibci = _prepared_index_buffers.begin();
       ibci != _prepared_index_buffers.end();
       ++ibci) {
    IndexBufferContext *ibc = (*ibci);
    _total_buffers_pcollector.sub_level(ibc->get_data_size_bytes());
    ibc->_data->clear_prepared(this);
  }

  _prepared_index_buffers.clear();
  _released_index_buffers.clear();
  _enqueued_index_buffers.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::enqueue_texture
//       Access: Public
//  Description: Indicates that a texture would like to be put on the
//               list to be prepared when the GSG is next ready to
//               do this (presumably at the next frame).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
enqueue_texture(Texture *tex) {
  MutexHolder holder(_lock);

  _enqueued_textures.insert(tex);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::dequeue_texture
//       Access: Public
//  Description: Removes a texture from the queued list of textures to
//               be prepared.  Normally it is not necessary to call
//               this, unless you change your mind about preparing it
//               at the last minute, since the texture will
//               automatically be dequeued and prepared at the next
//               frame.
//
//               The return value is true if the texture is
//               successfully dequeued, false if it had not been
//               queued.
////////////////////////////////////////////////////////////////////
bool PreparedGraphicsObjects::
dequeue_texture(Texture *tex) {
  MutexHolder holder(_lock);

  EnqueuedTextures::iterator qi = _enqueued_textures.find(tex);
  if (qi != _enqueued_textures.end()) {
    _enqueued_textures.erase(qi);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_texture
//       Access: Public
//  Description: Indicates that a texture context, created by a
//               previous call to prepare_texture(), is no longer
//               needed.  The driver resources will not be freed until
//               some GSG calls update(), indicating it is at a
//               stage where it is ready to release textures--this
//               prevents conflicts from threading or multiple GSG's
//               sharing textures (we have no way of knowing which
//               graphics context is currently active, or what state
//               it's in, at the time release_texture is called).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
release_texture(TextureContext *tc) {
  MutexHolder holder(_lock);

  tc->_texture->clear_prepared(this);
  _total_texusage_pcollector.sub_level(tc->estimate_texture_memory());

  // We have to set the Texture pointer to NULL at this point, since
  // the Texture itself might destruct at any time after it has been
  // released.
  tc->_texture = (Texture *)NULL;

  bool removed = (_prepared_textures.erase(tc) != 0);
  nassertv(removed);

  _released_textures.insert(tc);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_all_textures
//       Access: Public
//  Description: Releases all textures at once.  This will force them
//               to be reloaded into texture memory for all GSG's that
//               share this object.  Returns the number of textures
//               released.
////////////////////////////////////////////////////////////////////
int PreparedGraphicsObjects::
release_all_textures() {
  MutexHolder holder(_lock);

  int num_textures = (int)_prepared_textures.size();

  Textures::iterator tci;
  for (tci = _prepared_textures.begin();
       tci != _prepared_textures.end();
       ++tci) {
    TextureContext *tc = (*tci);
    tc->_texture->clear_prepared(this);
    _total_texusage_pcollector.sub_level(tc->estimate_texture_memory());
    tc->_texture = (Texture *)NULL;

    _released_textures.insert(tc);
  }

  _prepared_textures.clear();

  return num_textures;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::prepare_texture_now
//       Access: Public
//  Description: Immediately creates a new TextureContext for the
//               indicated texture and returns it.  This assumes that
//               the GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               textures.  If this is not necessarily the case, you
//               should use enqueue_texture() instead.
//
//               Normally, this function is not called directly.  Call
//               Texture::prepare_now() instead.
//
//               The TextureContext contains all of the pertinent
//               information needed by the GSG to keep track of this
//               one particular texture, and will exist as long as the
//               texture is ready to be rendered.
//
//               When either the Texture or the
//               PreparedGraphicsObjects object destructs, the
//               TextureContext will be deleted.
////////////////////////////////////////////////////////////////////
TextureContext *PreparedGraphicsObjects::
prepare_texture_now(Texture *tex, GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  // Ask the GSG to create a brand new TextureContext.  There might
  // be several GSG's sharing the same set of textures; if so, it
  // doesn't matter which of them creates the context (since they're
  // all shared anyway).
  TextureContext *tc = gsg->prepare_texture(tex);

  if (tc != (TextureContext *)NULL) {
    bool prepared = _prepared_textures.insert(tc).second;
    nassertr(prepared, tc);

    _total_texusage_pcollector.add_level(tc->estimate_texture_memory());
  }

  return tc;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::enqueue_geom
//       Access: Public
//  Description: Indicates that a geom would like to be put on the
//               list to be prepared when the GSG is next ready to
//               do this (presumably at the next frame).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
enqueue_geom(Geom *geom) {
  MutexHolder holder(_lock);

  _enqueued_geoms.insert(geom);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::dequeue_geom
//       Access: Public
//  Description: Removes a geom from the queued list of geoms to
//               be prepared.  Normally it is not necessary to call
//               this, unless you change your mind about preparing it
//               at the last minute, since the geom will
//               automatically be dequeued and prepared at the next
//               frame.
//
//               The return value is true if the geom is
//               successfully dequeued, false if it had not been
//               queued.
////////////////////////////////////////////////////////////////////
bool PreparedGraphicsObjects::
dequeue_geom(Geom *geom) {
  MutexHolder holder(_lock);

  EnqueuedGeoms::iterator qi = _enqueued_geoms.find(geom);
  if (qi != _enqueued_geoms.end()) {
    _enqueued_geoms.erase(qi);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_geom
//       Access: Public
//  Description: Indicates that a geom context, created by a
//               previous call to prepare_geom(), is no longer
//               needed.  The driver resources will not be freed until
//               some GSG calls update(), indicating it is at a
//               stage where it is ready to release geoms--this
//               prevents conflicts from threading or multiple GSG's
//               sharing geoms (we have no way of knowing which
//               graphics context is currently active, or what state
//               it's in, at the time release_geom is called).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
release_geom(GeomContext *gc) {
  MutexHolder holder(_lock);

  gc->_geom->clear_prepared(this);

  // We have to set the Geom pointer to NULL at this point, since
  // the Geom itself might destruct at any time after it has been
  // released.
  gc->_geom = (Geom *)NULL;

  bool removed = (_prepared_geoms.erase(gc) != 0);
  nassertv(removed);

  _released_geoms.insert(gc);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_all_geoms
//       Access: Public
//  Description: Releases all geoms at once.  This will force them
//               to be reloaded into geom memory for all GSG's that
//               share this object.  Returns the number of geoms
//               released.
////////////////////////////////////////////////////////////////////
int PreparedGraphicsObjects::
release_all_geoms() {
  MutexHolder holder(_lock);

  int num_geoms = (int)_prepared_geoms.size();

  Geoms::iterator gci;
  for (gci = _prepared_geoms.begin();
       gci != _prepared_geoms.end();
       ++gci) {
    GeomContext *gc = (*gci);
    gc->_geom->clear_prepared(this);
    gc->_geom = (Geom *)NULL;

    _released_geoms.insert(gc);
  }

  _prepared_geoms.clear();

  return num_geoms;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::prepare_geom_now
//       Access: Public
//  Description: Immediately creates a new GeomContext for the
//               indicated geom and returns it.  This assumes that
//               the GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               geoms.  If this is not necessarily the case, you
//               should use enqueue_geom() instead.
//
//               Normally, this function is not called directly.  Call
//               Geom::prepare_now() instead.
//
//               The GeomContext contains all of the pertinent
//               information needed by the GSG to keep track of this
//               one particular geom, and will exist as long as the
//               geom is ready to be rendered.
//
//               When either the Geom or the
//               PreparedGraphicsObjects object destructs, the
//               GeomContext will be deleted.
////////////////////////////////////////////////////////////////////
GeomContext *PreparedGraphicsObjects::
prepare_geom_now(Geom *geom, GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  // Ask the GSG to create a brand new GeomContext.  There might
  // be several GSG's sharing the same set of geoms; if so, it
  // doesn't matter which of them creates the context (since they're
  // all shared anyway).
  GeomContext *gc = gsg->prepare_geom(geom);

  if (gc != (GeomContext *)NULL) {
    bool prepared = _prepared_geoms.insert(gc).second;
    nassertr(prepared, gc);
  }

  return gc;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::enqueue_shader
//       Access: Public
//  Description: Indicates that a shader would like to be put on the
//               list to be prepared when the GSG is next ready to
//               do this (presumably at the next frame).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
enqueue_shader(ShaderExpansion *se) {
  MutexHolder holder(_lock);

  _enqueued_shaders.insert(se);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::dequeue_shader
//       Access: Public
//  Description: Removes a shader from the queued list of shaders to
//               be prepared.  Normally it is not necessary to call
//               this, unless you change your mind about preparing it
//               at the last minute, since the shader will
//               automatically be dequeued and prepared at the next
//               frame.
//
//               The return value is true if the shader is
//               successfully dequeued, false if it had not been
//               queued.
////////////////////////////////////////////////////////////////////
bool PreparedGraphicsObjects::
dequeue_shader(ShaderExpansion *se) {
  MutexHolder holder(_lock);

  EnqueuedShaders::iterator qi = _enqueued_shaders.find(se);
  if (qi != _enqueued_shaders.end()) {
    _enqueued_shaders.erase(qi);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_shader
//       Access: Public
//  Description: Indicates that a shader context, created by a
//               previous call to prepare_shader(), is no longer
//               needed.  The driver resources will not be freed until
//               some GSG calls update(), indicating it is at a
//               stage where it is ready to release shaders--this
//               prevents conflicts from threading or multiple GSG's
//               sharing shaders (we have no way of knowing which
//               graphics context is currently active, or what state
//               it's in, at the time release_shader is called).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
release_shader(ShaderContext *sc) {
  MutexHolder holder(_lock);

  sc->_shader_expansion->clear_prepared(this);

  // We have to set the Shader pointer to NULL at this point, since
  // the Shader itself might destruct at any time after it has been
  // released.
  sc->_shader_expansion = (ShaderExpansion *)NULL;

  bool removed = (_prepared_shaders.erase(sc) != 0);
  nassertv(removed);

  _released_shaders.insert(sc);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_all_shaders
//       Access: Public
//  Description: Releases all shaders at once.  This will force them
//               to be reloaded into shader memory for all GSG's that
//               share this object.  Returns the number of shaders
//               released.
////////////////////////////////////////////////////////////////////
int PreparedGraphicsObjects::
release_all_shaders() {
  MutexHolder holder(_lock);

  int num_shaders = (int)_prepared_shaders.size();

  Shaders::iterator sci;
  for (sci = _prepared_shaders.begin();
       sci != _prepared_shaders.end();
       ++sci) {
    ShaderContext *sc = (*sci);
    sc->_shader_expansion->clear_prepared(this);
    sc->_shader_expansion = (ShaderExpansion *)NULL;

    _released_shaders.insert(sc);
  }

  _prepared_shaders.clear();

  return num_shaders;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::prepare_shader_now
//       Access: Public
//  Description: Immediately creates a new ShaderContext for the
//               indicated shader and returns it.  This assumes that
//               the GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               shaders.  If this is not necessarily the case, you
//               should use enqueue_shader() instead.
//
//               Normally, this function is not called directly.  Call
//               Shader::prepare_now() instead.
//
//               The ShaderContext contains all of the pertinent
//               information needed by the GSG to keep track of this
//               one particular shader, and will exist as long as the
//               shader is ready to be rendered.
//
//               When either the Shader or the
//               PreparedGraphicsObjects object destructs, the
//               ShaderContext will be deleted.
////////////////////////////////////////////////////////////////////
ShaderContext *PreparedGraphicsObjects::
prepare_shader_now(ShaderExpansion *se, GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  // Ask the GSG to create a brand new ShaderContext.  There might
  // be several GSG's sharing the same set of shaders; if so, it
  // doesn't matter which of them creates the context (since they're
  // all shared anyway).
  ShaderContext *sc = gsg->prepare_shader(se);

  if (sc != (ShaderContext *)NULL) {
    bool prepared = _prepared_shaders.insert(sc).second;
    nassertr(prepared, sc);
  }

  return sc;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::enqueue_vertex_buffer
//       Access: Public
//  Description: Indicates that a buffer would like to be put on the
//               list to be prepared when the GSG is next ready to
//               do this (presumably at the next frame).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
enqueue_vertex_buffer(GeomVertexArrayData *data) {
  MutexHolder holder(_lock);

  _enqueued_vertex_buffers.insert(data);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::dequeue_vertex_buffer
//       Access: Public
//  Description: Removes a buffer from the queued list of data
//               arrays to be prepared.  Normally it is not necessary
//               to call this, unless you change your mind about
//               preparing it at the last minute, since the data will
//               automatically be dequeued and prepared at the next
//               frame.
//
//               The return value is true if the buffer is
//               successfully dequeued, false if it had not been
//               queued.
////////////////////////////////////////////////////////////////////
bool PreparedGraphicsObjects::
dequeue_vertex_buffer(GeomVertexArrayData *data) {
  MutexHolder holder(_lock);

  EnqueuedVertexBuffers::iterator qi = _enqueued_vertex_buffers.find(data);
  if (qi != _enqueued_vertex_buffers.end()) {
    _enqueued_vertex_buffers.erase(qi);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_vertex_buffer
//       Access: Public
//  Description: Indicates that a data context, created by a
//               previous call to prepare_vertex_buffer(), is no longer
//               needed.  The driver resources will not be freed until
//               some GSG calls update(), indicating it is at a
//               stage where it is ready to release datas--this
//               prevents conflicts from threading or multiple GSG's
//               sharing datas (we have no way of knowing which
//               graphics context is currently active, or what state
//               it's in, at the time release_vertex_buffer is called).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
release_vertex_buffer(VertexBufferContext *vbc) {
  MutexHolder holder(_lock);

  vbc->_data->clear_prepared(this);
  _total_buffers_pcollector.sub_level(vbc->get_data_size_bytes());

  // We have to set the Data pointer to NULL at this point, since
  // the Data itself might destruct at any time after it has been
  // released.
  vbc->_data = (GeomVertexArrayData *)NULL;

  bool removed = (_prepared_vertex_buffers.erase(vbc) != 0);
  nassertv(removed);

  _released_vertex_buffers.insert(vbc);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_all_vertex_buffers
//       Access: Public
//  Description: Releases all datas at once.  This will force them
//               to be reloaded into data memory for all GSG's that
//               share this object.  Returns the number of datas
//               released.
////////////////////////////////////////////////////////////////////
int PreparedGraphicsObjects::
release_all_vertex_buffers() {
  MutexHolder holder(_lock);

  int num_vertex_buffers = (int)_prepared_vertex_buffers.size();

  VertexBuffers::iterator vbci;
  for (vbci = _prepared_vertex_buffers.begin();
       vbci != _prepared_vertex_buffers.end();
       ++vbci) {
    VertexBufferContext *vbc = (*vbci);
    vbc->_data->clear_prepared(this);
    _total_buffers_pcollector.sub_level(vbc->get_data_size_bytes());
    vbc->_data = (GeomVertexArrayData *)NULL;

    _released_vertex_buffers.insert(vbc);
  }

  _prepared_vertex_buffers.clear();

  return num_vertex_buffers;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::prepare_vertex_buffer_now
//       Access: Public
//  Description: Immediately creates a new VertexBufferContext for the
//               indicated data and returns it.  This assumes that
//               the GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               datas.  If this is not necessarily the case, you
//               should use enqueue_vertex_buffer() instead.
//
//               Normally, this function is not called directly.  Call
//               Data::prepare_now() instead.
//
//               The VertexBufferContext contains all of the pertinent
//               information needed by the GSG to keep track of this
//               one particular data, and will exist as long as the
//               data is ready to be rendered.
//
//               When either the Data or the
//               PreparedGraphicsObjects object destructs, the
//               VertexBufferContext will be deleted.
////////////////////////////////////////////////////////////////////
VertexBufferContext *PreparedGraphicsObjects::
prepare_vertex_buffer_now(GeomVertexArrayData *data, GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  // Ask the GSG to create a brand new VertexBufferContext.  There might
  // be several GSG's sharing the same set of datas; if so, it
  // doesn't matter which of them creates the context (since they're
  // all shared anyway).
  VertexBufferContext *vbc = gsg->prepare_vertex_buffer(data);

  if (vbc != (VertexBufferContext *)NULL) {
    bool prepared = _prepared_vertex_buffers.insert(vbc).second;
    nassertr(prepared, vbc);

    // The size has already been counted by
    // GraphicsStateGuardian::add_to_vertex_buffer_record(); we don't need to
    // count it again here.
    //_total_buffers_pcollector.add_level(vbc->get_data_size_bytes());
  }

  return vbc;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::enqueue_index_buffer
//       Access: Public
//  Description: Indicates that a buffer would like to be put on the
//               list to be prepared when the GSG is next ready to
//               do this (presumably at the next frame).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
enqueue_index_buffer(GeomPrimitive *data) {
  MutexHolder holder(_lock);

  _enqueued_index_buffers.insert(data);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::dequeue_index_buffer
//       Access: Public
//  Description: Removes a buffer from the queued list of data
//               arrays to be prepared.  Normally it is not necessary
//               to call this, unless you change your mind about
//               preparing it at the last minute, since the data will
//               automatically be dequeued and prepared at the next
//               frame.
//
//               The return value is true if the buffer is
//               successfully dequeued, false if it had not been
//               queued.
////////////////////////////////////////////////////////////////////
bool PreparedGraphicsObjects::
dequeue_index_buffer(GeomPrimitive *data) {
  MutexHolder holder(_lock);

  EnqueuedIndexBuffers::iterator qi = _enqueued_index_buffers.find(data);
  if (qi != _enqueued_index_buffers.end()) {
    _enqueued_index_buffers.erase(qi);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_index_buffer
//       Access: Public
//  Description: Indicates that a data context, created by a
//               previous call to prepare_index_buffer(), is no longer
//               needed.  The driver resources will not be freed until
//               some GSG calls update(), indicating it is at a
//               stage where it is ready to release datas--this
//               prevents conflicts from threading or multiple GSG's
//               sharing datas (we have no way of knowing which
//               graphics context is currently active, or what state
//               it's in, at the time release_index_buffer is called).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
release_index_buffer(IndexBufferContext *ibc) {
  MutexHolder holder(_lock);

  ibc->_data->clear_prepared(this);
  _total_buffers_pcollector.sub_level(ibc->get_data_size_bytes());

  // We have to set the Data pointer to NULL at this point, since
  // the Data itself might destruct at any time after it has been
  // released.
  ibc->_data = (GeomPrimitive *)NULL;

  bool removed = (_prepared_index_buffers.erase(ibc) != 0);
  nassertv(removed);

  _released_index_buffers.insert(ibc);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_all_index_buffers
//       Access: Public
//  Description: Releases all datas at once.  This will force them
//               to be reloaded into data memory for all GSG's that
//               share this object.  Returns the number of datas
//               released.
////////////////////////////////////////////////////////////////////
int PreparedGraphicsObjects::
release_all_index_buffers() {
  MutexHolder holder(_lock);

  int num_index_buffers = (int)_prepared_index_buffers.size();

  IndexBuffers::iterator ibci;
  for (ibci = _prepared_index_buffers.begin();
       ibci != _prepared_index_buffers.end();
       ++ibci) {
    IndexBufferContext *ibc = (*ibci);
    ibc->_data->clear_prepared(this);
    _total_buffers_pcollector.sub_level(ibc->get_data_size_bytes());
    ibc->_data = (GeomPrimitive *)NULL;

    _released_index_buffers.insert(ibc);
  }

  _prepared_index_buffers.clear();

  return num_index_buffers;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::prepare_index_buffer_now
//       Access: Public
//  Description: Immediately creates a new IndexBufferContext for the
//               indicated data and returns it.  This assumes that
//               the GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               datas.  If this is not necessarily the case, you
//               should use enqueue_index_buffer() instead.
//
//               Normally, this function is not called directly.  Call
//               Data::prepare_now() instead.
//
//               The IndexBufferContext contains all of the pertinent
//               information needed by the GSG to keep track of this
//               one particular data, and will exist as long as the
//               data is ready to be rendered.
//
//               When either the Data or the
//               PreparedGraphicsObjects object destructs, the
//               IndexBufferContext will be deleted.
////////////////////////////////////////////////////////////////////
IndexBufferContext *PreparedGraphicsObjects::
prepare_index_buffer_now(GeomPrimitive *data, GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  // Ask the GSG to create a brand new IndexBufferContext.  There might
  // be several GSG's sharing the same set of datas; if so, it
  // doesn't matter which of them creates the context (since they're
  // all shared anyway).
  IndexBufferContext *ibc = gsg->prepare_index_buffer(data);

  if (ibc != (IndexBufferContext *)NULL) {
    bool prepared = _prepared_index_buffers.insert(ibc).second;
    nassertr(prepared, ibc);

    // The size has already been counted by
    // GraphicsStateGuardian::add_to_index_buffer_record(); we don't need to
    // count it again here.
    //_total_buffers_pcollector.add_level(ibc->get_data_size_bytes());
  }

  return ibc;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::update
//       Access: Public
//  Description: This is called by the GraphicsStateGuardian to
//               indicate that it is in a state to load or release
//               textures.
//
//               Any texture contexts that were previously passed to
//               release_texture() are actually passed to the GSG to
//               be freed at this point; textures that were previously
//               passed to prepare_texture are actually loaded.
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
update(GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  // First, release all the textures, geoms, and buffers awaiting
  // release.
  Textures::iterator tci;
  for (tci = _released_textures.begin();
       tci != _released_textures.end();
       ++tci) {
    TextureContext *tc = (*tci);
    gsg->release_texture(tc);
  }

  _released_textures.clear();

  Geoms::iterator gci;
  for (gci = _released_geoms.begin();
       gci != _released_geoms.end();
       ++gci) {
    GeomContext *gc = (*gci);
    gsg->release_geom(gc);
  }

  _released_geoms.clear();

  Shaders::iterator sci;
  for (sci = _released_shaders.begin();
       sci != _released_shaders.end();
       ++sci) {
    ShaderContext *sc = (*sci);
    gsg->release_shader(sc);
  }

  _released_shaders.clear();

  VertexBuffers::iterator vbci;
  for (vbci = _released_vertex_buffers.begin();
       vbci != _released_vertex_buffers.end();
       ++vbci) {
    VertexBufferContext *vbc = (*vbci);
    gsg->release_vertex_buffer(vbc);
  }

  _released_vertex_buffers.clear();

  IndexBuffers::iterator ibci;
  for (ibci = _released_index_buffers.begin();
       ibci != _released_index_buffers.end();
       ++ibci) {
    IndexBufferContext *ibc = (*ibci);
    gsg->release_index_buffer(ibc);
  }

  _released_index_buffers.clear();

  // Now prepare all the textures, geoms, and buffers awaiting
  // preparation.
  EnqueuedTextures::iterator qti;
  for (qti = _enqueued_textures.begin();
       qti != _enqueued_textures.end();
       ++qti) {
    Texture *tex = (*qti);
    tex->prepare_now(this, gsg);
  }

  _enqueued_textures.clear();

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
    ShaderExpansion *shader = (*qsi);
    shader->prepare_now(this, gsg);
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
    data->prepare_now(this, gsg);
  }

  _enqueued_index_buffers.clear();
}
