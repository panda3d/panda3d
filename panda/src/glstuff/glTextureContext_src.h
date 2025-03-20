/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glTextureContext_src.h
 * @author drose
 * @date 1999-10-07
 */

#include "pandabase.h"
#include "textureContext.h"
#include "deletedChain.h"
#include "samplerState.h"

class CLP(GraphicsStateGuardian);
class CLP(SamplerContext);

/**
 *
 */
class EXPCL_GL CLP(TextureContext) : public TextureContext {
public:
  INLINE CLP(TextureContext)(CLP(GraphicsStateGuardian) *glgsg,
                             PreparedGraphicsObjects *pgo, Texture *tex);
  ALLOC_DELETED_CHAIN(CLP(TextureContext));

  virtual ~CLP(TextureContext)();
  virtual void evict_lru();
  void reset_data(GLenum target, int num_views = 1);

  virtual uint64_t get_native_id() const;
  virtual uint64_t get_native_buffer_id() const;

  void set_num_views(int num_views);

  INLINE GLuint get_view_index(int view) const;
  INLINE GLuint get_view_buffer(int view) const;

#ifdef OPENGLES_1
  static constexpr bool needs_barrier(GLbitfield barrier, bool writing) { return false; };
#else
  bool needs_barrier(GLbitfield barrier, bool writing);
  void mark_incoherent(bool wrote);
#endif

  INLINE bool is_upload_pending() const;
  INLINE void wait_pending_uploads() const;
  INLINE void cancel_pending_uploads();

  void return_pbo(GLuint pbo, size_t size);
  void delete_unused_pbos();
  INLINE void wait_for_unused_pbo(int limit) const;

private:
  void do_wait_pending_uploads() const;
  void do_wait_for_unused_pbo(int limit) const;

private:
  // This is the GL "name" of the texture object.
  GLuint _index;

  // This is only used for buffer textures.
  GLuint _buffer;

public:
  // Multiview textures have multiple of the above.  For a single-view texture,
  // these are simply pointers to the above fields.
  int _num_views;
  GLuint *_indices;
  GLuint *_buffers;

  // These are the parameters that we specified with the last glTexImage2D()
  // or glTexStorage2D() call.  If none of these have changed, we can reload
  // the texture image with a glTexSubImage2D().
  bool _has_storage;
  bool _immutable;
  bool _may_reload_with_mipmaps;
  bool _generate_mipmaps;
  GLint _internal_format;
  GLsizei _width;
  GLsizei _height;
  GLsizei _depth;
  int _num_levels;
  GLenum _target;
  SamplerState _active_sampler;

  // These counters are used to prevent out-of-order updates.
  int _uploads_started = 0;
  int _uploads_finished = 0;
  int _uploads_pending = 0;
  pdeque<GLuint> _unused_pbos;
  int _num_pbos = 0;
  size_t _pbo_size = 0;

  CLP(GraphicsStateGuardian) *_glgsg;

  // These are set to the equivalent counter in glgsg when a write is performed.
  int _texture_fetch_barrier_counter = -1;
  int _shader_image_read_barrier_counter = -1;
  int _shader_image_write_barrier_counter = -1;
  int _texture_read_barrier_counter = -1;
  int _texture_write_barrier_counter = -1;
  int _framebuffer_read_barrier_counter = -1;
  int _framebuffer_write_barrier_counter = -1;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "TextureContext",
                  TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glTextureContext_src.I"
