/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file preparedGraphicsObjects.h
 * @author drose
 * @date 2004-02-19
 */

#ifndef PREPAREDGRAPHICSOBJECTS_H
#define PREPAREDGRAPHICSOBJECTS_H

#include "pandabase.h"
#include "referenceCount.h"
#include "texture.h"
#include "samplerState.h"
#include "geom.h"
#include "geomVertexArrayData.h"
#include "geomPrimitive.h"
#include "shader.h"
#include "shaderBuffer.h"
#include "pointerTo.h"
#include "pStatCollector.h"
#include "pset.h"
#include "reMutex.h"
#include "bufferResidencyTracker.h"
#include "adaptiveLru.h"
#include "asyncFuture.h"

class TextureContext;
class SamplerContext;
class GeomContext;
class ShaderContext;
class VertexBufferContext;
class IndexBufferContext;
class BufferContext;
class GraphicsStateGuardianBase;
class SavedContext;

/**
 * A table of objects that are saved within the graphics context for reference
 * by handle later.  Generally, this represents things like OpenGL texture
 * objects or display lists (or their equivalent on other platforms).
 *
 * This object simply records the pointers to the context objects created by
 * the individual GSG's; these context objects will contain enough information
 * to reference or release the actual object stored within the graphics
 * context.
 *
 * These tables may potentially be shared between related graphics contexts,
 * hence their storage here in a separate object rather than as a part of the
 * GraphicsStateGuardian.
 */
class EXPCL_PANDA_GOBJ PreparedGraphicsObjects : public ReferenceCount {
public:
  PreparedGraphicsObjects();
  ~PreparedGraphicsObjects();

PUBLISHED:
  INLINE const std::string &get_name() const;

  void set_graphics_memory_limit(size_t limit);
  INLINE size_t get_graphics_memory_limit() const;
  void show_graphics_memory_lru(std::ostream &out) const;
  void show_residency_trackers(std::ostream &out) const;

  INLINE void release_all();
  INLINE int get_num_queued() const;
  INLINE int get_num_prepared() const;

  void enqueue_texture(Texture *tex);
  bool is_texture_queued(const Texture *tex) const;
  bool dequeue_texture(Texture *tex);
  bool is_texture_prepared(const Texture *tex) const;
  void release_texture(TextureContext *tc);
  void release_texture(Texture *tex);
  int release_all_textures();
  int get_num_queued_textures() const;
  int get_num_prepared_textures() const;

  TextureContext *prepare_texture_now(Texture *tex, int view,
                                      GraphicsStateGuardianBase *gsg);

  void enqueue_sampler(const SamplerState &sampler);
  bool is_sampler_queued(const SamplerState &sampler) const;
  bool dequeue_sampler(const SamplerState &sampler);
  bool is_sampler_prepared(const SamplerState &sampler) const;
  void release_sampler(SamplerContext *sc);
  void release_sampler(const SamplerState &sampler);
  int release_all_samplers();
  int get_num_queued_samplers() const;
  int get_num_prepared_samplers() const;

  SamplerContext *prepare_sampler_now(const SamplerState &sampler,
                                      GraphicsStateGuardianBase *gsg);

  void enqueue_geom(Geom *geom);
  bool is_geom_queued(const Geom *geom) const;
  bool dequeue_geom(Geom *geom);
  bool is_geom_prepared(const Geom *geom) const;
  void release_geom(GeomContext *gc);
  int release_all_geoms();
  int get_num_queued_geoms() const;
  int get_num_prepared_geoms() const;

  GeomContext *prepare_geom_now(Geom *geom, GraphicsStateGuardianBase *gsg);

  void enqueue_shader(Shader *shader);
  bool is_shader_queued(const Shader *shader) const;
  bool dequeue_shader(Shader *shader);
  bool is_shader_prepared(const Shader *shader) const;
  void release_shader(ShaderContext *sc);
  int release_all_shaders();
  int get_num_queued_shaders() const;
  int get_num_prepared_shaders() const;

  ShaderContext *prepare_shader_now(Shader *shader, GraphicsStateGuardianBase *gsg);

  void enqueue_vertex_buffer(GeomVertexArrayData *data);
  bool is_vertex_buffer_queued(const GeomVertexArrayData *data) const;
  bool dequeue_vertex_buffer(GeomVertexArrayData *data);
  bool is_vertex_buffer_prepared(const GeomVertexArrayData *data) const;
  void release_vertex_buffer(VertexBufferContext *vbc);
  int release_all_vertex_buffers();
  int get_num_queued_vertex_buffers() const;
  int get_num_prepared_vertex_buffers() const;

  VertexBufferContext *
  prepare_vertex_buffer_now(GeomVertexArrayData *data,
                            GraphicsStateGuardianBase *gsg);

  void enqueue_index_buffer(GeomPrimitive *data);
  bool is_index_buffer_queued(const GeomPrimitive *data) const;
  bool dequeue_index_buffer(GeomPrimitive *data);
  bool is_index_buffer_prepared(const GeomPrimitive *data) const;
  void release_index_buffer(IndexBufferContext *ibc);
  int release_all_index_buffers();
  int get_num_queued_index_buffers() const;
  int get_num_prepared_index_buffers() const;

  IndexBufferContext *
  prepare_index_buffer_now(GeomPrimitive *data,
                           GraphicsStateGuardianBase *gsg);

  void enqueue_shader_buffer(ShaderBuffer *data);
  bool is_shader_buffer_queued(const ShaderBuffer *data) const;
  bool dequeue_shader_buffer(ShaderBuffer *data);
  bool is_shader_buffer_prepared(const ShaderBuffer *data) const;
  void release_shader_buffer(BufferContext *bc);
  int release_all_shader_buffers();
  int get_num_queued_shader_buffers() const;
  int get_num_prepared_shader_buffers() const;

  BufferContext *
  prepare_shader_buffer_now(ShaderBuffer *data,
                            GraphicsStateGuardianBase *gsg);

public:
  /**
   * This is a handle to an enqueued object, from which the result can be
   * obtained upon completion.
   */
  class EXPCL_PANDA_GOBJ EnqueuedObject final : public AsyncFuture {
  public:
    EnqueuedObject(PreparedGraphicsObjects *pgo, TypedWritableReferenceCount *object);

    TypedWritableReferenceCount *get_object() { return _object.p(); }
    SavedContext *get_result() { return (SavedContext *)AsyncFuture::get_result(); }
    void set_result(SavedContext *result);

    void notify_removed();
    virtual bool cancel() final;

  PUBLISHED:
    MAKE_PROPERTY(object, get_object);

  private:
    PreparedGraphicsObjects *_pgo;
    PT(TypedWritableReferenceCount) const _object;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      AsyncFuture::init_type();
      register_type(_type_handle, "EnqueuedObject",
                    AsyncFuture::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };

  // These are variations of enqueue_xxx that also return a future.  They are
  // used to implement texture->prepare(), etc.  They are only marked public
  // so we don't have to define a whole bunch of friend classes.
  PT(EnqueuedObject) enqueue_texture_future(Texture *tex);
  //PT(EnqueuedObject) enqueue_geom_future(Geom *geom);
  PT(EnqueuedObject) enqueue_shader_future(Shader *shader);
  //PT(EnqueuedObject) enqueue_vertex_buffer_future(GeomVertexArrayData *data);
  //PT(EnqueuedObject) enqueue_index_buffer_future(GeomPrimitive *data);
  //PT(EnqueuedObject) enqueue_shader_buffer_future(ShaderBuffer *data);

  void begin_frame(GraphicsStateGuardianBase *gsg,
                   Thread *current_thread);
  void end_frame(Thread *current_thread);

private:
  static std::string init_name();

private:
  typedef phash_set<TextureContext *, pointer_hash> Textures;
  typedef phash_map< PT(Texture), PT(EnqueuedObject) > EnqueuedTextures;
  typedef phash_set<GeomContext *, pointer_hash> Geoms;
  typedef phash_set< PT(Geom) > EnqueuedGeoms;
  typedef phash_set<ShaderContext *, pointer_hash> Shaders;
  typedef phash_map< PT(Shader), PT(EnqueuedObject) > EnqueuedShaders;
  typedef phash_set<BufferContext *, pointer_hash> Buffers;
  typedef phash_set< PT(GeomVertexArrayData) > EnqueuedVertexBuffers;
  typedef phash_set< PT(GeomPrimitive) > EnqueuedIndexBuffers;
  typedef phash_set< PT(ShaderBuffer) > EnqueuedShaderBuffers;

  // Sampler states are stored a little bit differently, as they are mapped by
  // value and can't store the list of prepared samplers.
  typedef pmap<SamplerState, SamplerContext *> PreparedSamplers;
  typedef pset<SamplerContext *, pointer_hash> ReleasedSamplers;
  typedef pset<SamplerState> EnqueuedSamplers;

  class BufferCacheKey {
  public:
    INLINE bool operator < (const BufferCacheKey &other) const;
    INLINE bool operator == (const BufferCacheKey &other) const;
    INLINE bool operator != (const BufferCacheKey &other) const;
    size_t _data_size_bytes;
    GeomEnums::UsageHint _usage_hint;
  };
  typedef pvector<BufferContext *> BufferList;
  typedef pmap<BufferCacheKey, BufferList> BufferCache;
  typedef plist<BufferCacheKey> BufferCacheLRU;

  void cache_unprepared_buffer(BufferContext *buffer, size_t data_size_bytes,
                               GeomEnums::UsageHint usage_hint,
                               BufferCache &buffer_cache,
                               BufferCacheLRU &buffer_cache_lru,
                               size_t &buffer_cache_size,
                               int released_buffer_cache_size,
                               pvector<BufferContext *> &released_buffers);
  BufferContext *get_cached_buffer(size_t data_size_bytes,
                                   GeomEnums::UsageHint usage_hint,
                                   BufferCache &buffer_cache,
                                   BufferCacheLRU &buffer_cache_lru,
                                   size_t &buffer_cache_size);

  ReMutex _lock;
  std::string _name;
  Textures _prepared_textures;
  pvector<TextureContext *> _released_textures;
  EnqueuedTextures _enqueued_textures;
  PreparedSamplers _prepared_samplers;
  ReleasedSamplers _released_samplers;
  EnqueuedSamplers _enqueued_samplers;
  Geoms _prepared_geoms, _released_geoms;
  EnqueuedGeoms _enqueued_geoms;
  Shaders _prepared_shaders, _released_shaders;
  EnqueuedShaders _enqueued_shaders;
  Buffers _prepared_vertex_buffers;
  pvector<BufferContext *> _released_vertex_buffers;
  EnqueuedVertexBuffers _enqueued_vertex_buffers;
  Buffers _prepared_index_buffers;
  pvector<BufferContext *> _released_index_buffers;
  EnqueuedIndexBuffers _enqueued_index_buffers;
  Buffers _prepared_shader_buffers;
  pvector<BufferContext *> _released_shader_buffers;
  EnqueuedShaderBuffers _enqueued_shader_buffers;

  BufferCache _vertex_buffer_cache;
  BufferCacheLRU _vertex_buffer_cache_lru;
  size_t _vertex_buffer_cache_size;

  BufferCache _index_buffer_cache;
  BufferCacheLRU _index_buffer_cache_lru;
  size_t _index_buffer_cache_size;

public:
  BufferResidencyTracker _texture_residency;
  BufferResidencyTracker _vbuffer_residency;
  BufferResidencyTracker _ibuffer_residency;
  BufferResidencyTracker _sbuffer_residency;

  AdaptiveLru _graphics_memory_lru;
  SimpleLru _sampler_object_lru;

public:
  // This is only public as a temporary hack.  Don't mess with it unless you
  // know what you're doing.
  bool _support_released_buffer_cache;

private:
  static int _name_index;

  friend class GraphicsStateGuardian;
};

#include "preparedGraphicsObjects.I"

#endif
