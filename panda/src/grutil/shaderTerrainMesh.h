/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderTerrainMesh.h
 * @author tobspr
 * @date 2016-02-16
 */

#ifndef SHADER_TERRAIN_MESH_H
#define SHADER_TERRAIN_MESH_H

#include "pandabase.h"
#include "luse.h"
#include "pnmImage.h"
#include "geom.h"
#include "pandaNode.h"
#include "texture.h"
#include "texturePeeker.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "pStatCollector.h"
#include "filename.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include <stdint.h>

extern ConfigVariableBool stm_use_hexagonal_layout;
extern ConfigVariableInt stm_max_chunk_count;
extern ConfigVariableInt stm_max_views;


NotifyCategoryDecl(shader_terrain, EXPCL_PANDA_GRUTIL, EXPTP_PANDA_GRUTIL);


/**
 * @brief Terrain Renderer class utilizing the GPU
 * @details This class provides functionality to render heightfields of large
 *   sizes utilizing the GPU. Internally a quadtree is used to generate the LODs.
 *   The final terrain is then rendered using instancing on the GPU. This makes
 *   it possible to use very large heightfields (8192+) with very reasonable
 *   performance. The terrain provides options to control the LOD using a
 *   target triangle width, see ShaderTerrainMesh::set_target_triangle_width().
 *
 *   Because the Terrain is rendered entirely on the GPU, it needs a special
 *   vertex shader. There is a default vertex shader available, which you can
 *   use in your own shaders. IMPORTANT: If you don't set an appropriate shader
 *   on the terrain, nothing will be visible.
 */
class EXPCL_PANDA_GRUTIL ShaderTerrainMesh : public PandaNode {

PUBLISHED:

  ShaderTerrainMesh();

  INLINE void set_heightfield(Texture* heightfield);
  INLINE Texture* get_heightfield() const;
  MAKE_PROPERTY(heightfield, get_heightfield, set_heightfield);

  INLINE void set_chunk_size(size_t chunk_size);
  INLINE size_t get_chunk_size() const;
  MAKE_PROPERTY(chunk_size, get_chunk_size, set_chunk_size);

  INLINE void set_generate_patches(bool generate_patches);
  INLINE bool get_generate_patches() const;
  MAKE_PROPERTY(generate_patches, get_generate_patches, set_generate_patches);

  INLINE void set_update_enabled(bool update_enabled);
  INLINE bool get_update_enabled() const;
  MAKE_PROPERTY(update_enabled, get_update_enabled, set_update_enabled);

  INLINE void set_target_triangle_width(PN_stdfloat target_triangle_width);
  INLINE PN_stdfloat get_target_triangle_width() const;
  MAKE_PROPERTY(target_triangle_width, get_target_triangle_width, set_target_triangle_width);

  LPoint3 uv_to_world(const LTexCoord& coord) const;
  INLINE LPoint3 uv_to_world(PN_stdfloat u, PN_stdfloat v) const;

  bool generate();

public:

  // Methods derived from PandaNode
  virtual bool is_renderable() const;
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_combine() const;
  virtual void add_for_draw(CullTraverser *trav, CullTraverserData &data);

private:
  virtual CPT(TransformState)
    calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                      bool &found_any,
                      const TransformState *transform,
                      Thread *current_thread = Thread::get_current_thread()) const;

  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

  // Chunk data
  struct Chunk {
    // Depth, starting at 0
    size_t depth;

    // Chunk position in heightfield space
    size_t x, y;

    // Chunk size in heightfield space
    size_t size;

    // Children, in the order (0, 0) (1, 0) (0, 1) (1, 1)
    Chunk* children[4];

    // Chunk heights, used for culling
    PN_stdfloat avg_height, min_height, max_height;

    // Edge heights, used for lod computation, in the same order as the children
    LVector4 edges;

    // Last CLOD factor, stored while computing LOD, used for seamless transitions between lods
    PN_stdfloat last_clod;

    INLINE void clear_children();
    INLINE Chunk();
    INLINE ~Chunk();
  };


  // Single entry in the data block
  struct ChunkDataEntry {
    // float x, y, size, clod;

    // Panda uses BGRA, the above layout shows how its actually in texture memory,
    // the layout below makes it work with BGRA.
    PN_float32 size, y, x, clod;
  };

  // Data used while traversing all chunks
  struct TraversalData {
    // Global MVP used for LOD
    LMatrix4 mvp_mat;

    // Local model matrix used for culling
    LMatrix4 model_mat;

    // Camera bounds in world space
    BoundingVolume* cam_bounds;

    // Amount of emitted chunks so far
    int emitted_chunks;

    // Screen resolution, used for LOD
    LVector2i screen_size;

    // Pointer to the texture memory, where each chunk is written to
    ChunkDataEntry* storage_ptr;
  };

  bool do_check_heightfield();
  void do_extract_heightfield();
  void do_init_data_texture();
  void do_create_chunks();
  void do_init_chunk(Chunk* chunk);
  void do_compute_bounds(Chunk* chunk);
  void do_create_chunk_geom();
  void do_traverse(Chunk* chunk, TraversalData* data, bool fully_visible = false);
  void do_emit_chunk(Chunk* chunk, TraversalData* data);
  bool do_check_lod_matches(Chunk* chunk, TraversalData* data);

  Mutex _lock;
  Chunk _base_chunk;
  size_t _size;
  size_t _chunk_size;
  bool _generate_patches;
  PNMImage _heightfield;
  PT(Texture) _heightfield_tex;
  PT(Geom) _chunk_geom;
  PT(Texture) _data_texture;
  size_t _current_view_index;
  int _last_frame_count;
  PN_stdfloat _target_triangle_width;
  bool _update_enabled;

  // PStats stuff
  static PStatCollector _lod_collector;
  static PStatCollector _basic_collector;


// Type handle stuff
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "ShaderTerrainMesh", PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shaderTerrainMesh.I"

#endif // SHADER_TERRAIN_MESH_H
