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
#include "texture.h"
#include "geom.h"
#include "qpgeomVertexArrayData.h"
#include "mutexHolder.h"

PStatCollector PreparedGraphicsObjects::_total_texusage_pcollector("Texture usage");
PStatCollector PreparedGraphicsObjects::_total_buffers_pcollector("Vertex buffers");

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

  Datas::iterator dci;
  for (dci = _prepared_datas.begin();
       dci != _prepared_datas.end();
       ++dci) {
    DataContext *dc = (*dci);
    _total_texusage_pcollector.sub_level(dc->get_num_bytes());
    dc->_data->clear_prepared(this);
  }

  _prepared_datas.clear();
  _released_datas.clear();
  _enqueued_datas.clear();
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
//     Function: PreparedGraphicsObjects::enqueue_data
//       Access: Public
//  Description: Indicates that a data array would like to be put on the
//               list to be prepared when the GSG is next ready to
//               do this (presumably at the next frame).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
enqueue_data(qpGeomVertexArrayData *data) {
  MutexHolder holder(_lock);

  _enqueued_datas.insert(data);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::dequeue_data
//       Access: Public
//  Description: Removes a data array from the queued list of data
//               arrays to be prepared.  Normally it is not necessary
//               to call this, unless you change your mind about
//               preparing it at the last minute, since the data will
//               automatically be dequeued and prepared at the next
//               frame.
//
//               The return value is true if the data array is
//               successfully dequeued, false if it had not been
//               queued.
////////////////////////////////////////////////////////////////////
bool PreparedGraphicsObjects::
dequeue_data(qpGeomVertexArrayData *data) {
  MutexHolder holder(_lock);

  EnqueuedDatas::iterator qi = _enqueued_datas.find(data);
  if (qi != _enqueued_datas.end()) {
    _enqueued_datas.erase(qi);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_data
//       Access: Public
//  Description: Indicates that a data context, created by a
//               previous call to prepare_data(), is no longer
//               needed.  The driver resources will not be freed until
//               some GSG calls update(), indicating it is at a
//               stage where it is ready to release datas--this
//               prevents conflicts from threading or multiple GSG's
//               sharing datas (we have no way of knowing which
//               graphics context is currently active, or what state
//               it's in, at the time release_data is called).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
release_data(DataContext *dc) {
  MutexHolder holder(_lock);

  dc->_data->clear_prepared(this);
  _total_buffers_pcollector.sub_level(dc->get_num_bytes());

  // We have to set the Data pointer to NULL at this point, since
  // the Data itself might destruct at any time after it has been
  // released.
  dc->_data = (qpGeomVertexArrayData *)NULL;

  bool removed = (_prepared_datas.erase(dc) != 0);
  nassertv(removed);

  _released_datas.insert(dc);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_all_datas
//       Access: Public
//  Description: Releases all datas at once.  This will force them
//               to be reloaded into data memory for all GSG's that
//               share this object.  Returns the number of datas
//               released.
////////////////////////////////////////////////////////////////////
int PreparedGraphicsObjects::
release_all_datas() {
  MutexHolder holder(_lock);

  int num_datas = (int)_prepared_datas.size();

  Datas::iterator dci;
  for (dci = _prepared_datas.begin();
       dci != _prepared_datas.end();
       ++dci) {
    DataContext *dc = (*dci);
    dc->_data->clear_prepared(this);
    _total_buffers_pcollector.sub_level(dc->get_num_bytes());
    dc->_data = (qpGeomVertexArrayData *)NULL;

    _released_datas.insert(dc);
  }

  _prepared_datas.clear();

  return num_datas;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::prepare_data_now
//       Access: Public
//  Description: Immediately creates a new DataContext for the
//               indicated data and returns it.  This assumes that
//               the GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               datas.  If this is not necessarily the case, you
//               should use enqueue_data() instead.
//
//               Normally, this function is not called directly.  Call
//               Data::prepare_now() instead.
//
//               The DataContext contains all of the pertinent
//               information needed by the GSG to keep track of this
//               one particular data, and will exist as long as the
//               data is ready to be rendered.
//
//               When either the Data or the
//               PreparedGraphicsObjects object destructs, the
//               DataContext will be deleted.
////////////////////////////////////////////////////////////////////
DataContext *PreparedGraphicsObjects::
prepare_data_now(qpGeomVertexArrayData *data, GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  // Ask the GSG to create a brand new DataContext.  There might
  // be several GSG's sharing the same set of datas; if so, it
  // doesn't matter which of them creates the context (since they're
  // all shared anyway).
  DataContext *dc = gsg->prepare_data(data);

  if (dc != (DataContext *)NULL) {
    bool prepared = _prepared_datas.insert(dc).second;
    nassertr(prepared, dc);

    // The size has already been counted by
    // GraphicsStateGuardian::add_to_data_record(); we don't need to
    // count it again here.
    //_total_buffers_pcollector.add_level(dc->get_num_bytes());
  }

  return dc;
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

  // First, release all the textures, geoms, and data arrays awaiting
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

  Datas::iterator dci;
  for (dci = _released_datas.begin();
       dci != _released_datas.end();
       ++dci) {
    DataContext *dc = (*dci);
    gsg->release_data(dc);
  }

  _released_datas.clear();

  // Now prepare all the textures, geoms, and data arrays awaiting
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

  EnqueuedDatas::iterator qdi;
  for (qdi = _enqueued_datas.begin();
       qdi != _enqueued_datas.end();
       ++qdi) {
    qpGeomVertexArrayData *data = (*qdi);
    data->prepare_now(this, gsg);
  }

  _enqueued_datas.clear();
}
