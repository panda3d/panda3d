/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderTerrainMesh.cxx
 * @author tobspr
 * @date 2016-02-16
 */


#include "shaderTerrainMesh.h"
#include "geom.h"
#include "geomVertexFormat.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomNode.h"
#include "geomTriangles.h"
#include "geomPatches.h"
#include "omniBoundingVolume.h"
#include "cullableObject.h"
#include "cullTraverser.h"
#include "cullHandler.h"
#include "cullTraverserData.h"
#include "clockObject.h"
#include "shaderAttrib.h"
#include "renderAttrib.h"
#include "shaderInput.h"
#include "boundingBox.h"
#include "boundingSphere.h"
#include "samplerState.h"
#include "config_grutil.h"
#include "typeHandle.h"

using std::endl;
using std::max;
using std::min;

ConfigVariableBool stm_use_hexagonal_layout
("stm-use-hexagonal-layout", false,
 PRC_DESC("Set this to true to use a hexagonal vertex layout. This approximates "
          "the heightfield in a better way, however the CLOD transitions might be "
          "visible due to the vertices not matching exactly."));

ConfigVariableInt stm_max_chunk_count
("stm-max-chunk-count", 2048,
 PRC_DESC("Controls the maximum amount of chunks the Terrain can display. If you use "
          "a high LOD, you might have to increment this value. The lower this value is "
          "the less data has to be transferred to the GPU."));

ConfigVariableInt stm_max_views
("stm-max-views", 8,
 PRC_DESC("Controls the maximum amount of different views the Terrain can be rendered "
          "with. Each camera rendering the terrain corresponds to a view. Lowering this "
          "value will reduce the data that has to be transferred to the GPU."));

ConfigVariableEnum<SamplerState::FilterType> stm_heightfield_minfilter
("stm-heightfield-minfilter", SamplerState::FT_linear,
 PRC_DESC("This specifies the minfilter that is applied for a heightfield texture. This "
         "can be used to create heightfield that is visual correct with collision "
         "geometry (for example bullet terrain mesh) by changing it to nearest"));

ConfigVariableEnum<SamplerState::FilterType> stm_heightfield_magfilter
("stm-heightfield-magfilter", SamplerState::FT_linear,
 PRC_DESC("This specifies the magfilter that is applied for a heightfield texture. This "
         "can be used to create heightfield that is visual correct with collision "
         "geometry (for example bullet terrain mesh) by changing it to nearest"));

PStatCollector ShaderTerrainMesh::_basic_collector("Cull:ShaderTerrainMesh:Setup");
PStatCollector ShaderTerrainMesh::_lod_collector("Cull:ShaderTerrainMesh:CollectLOD");

NotifyCategoryDef(shader_terrain, "");

TypeHandle ShaderTerrainMesh::_type_handle;

/**
 * @brief Helper function to check for a power of two
 * @details This method checks for a power of two by using bitmasks
 *
 * @param x Number to check
 * @return true if x is a power of two, false otherwise
 */
int check_power_of_two(size_t x)
{
  return ((x != 0) && ((x & (~x + 1)) == x));
}

/**
 * @brief Constructs a new Terrain Mesh
 * @details This constructs a new terrain mesh. By default, no transform is set
 *   on the mesh, causing it to range over the unit box from (0, 0, 0) to
 *   (1, 1, 1). Usually you want to set a custom transform with NodePath::set_scale()
 */
ShaderTerrainMesh::ShaderTerrainMesh() :
  PandaNode("ShaderTerrainMesh"),
  _size(0),
  _chunk_size(32),
  _generate_patches(false),
  _data_texture(nullptr),
  _chunk_geom(nullptr),
  _current_view_index(0),
  _last_frame_count(-1),
  _target_triangle_width(10.0f),
  _update_enabled(true),
  _heightfield_tex(nullptr)
{
}

/**
 * @brief Generates the terrain mesh
 * @details This generates the terrain mesh, initializing all chunks of the
 *   internal used quadtree. At this point, a heightfield and a chunk size should
 *   have been set, otherwise an error is thrown.
 *
 *   If anything goes wrong, like a missing heightfield, then an error is printed
 *   and false is returned.
 *
 * @return true if the terrain was initialized, false if an error occured
 */
bool ShaderTerrainMesh::generate() {
  MutexHolder holder(_lock);
  if (!do_check_heightfield())
    return false;

  if (_chunk_size < 8 || !check_power_of_two(_chunk_size)) {
    shader_terrain_cat.error() << "Invalid chunk size! Has to be >= 8 and a power of two!" << endl;
    return false;
  }

  if (_chunk_size > _size / 4) {
    shader_terrain_cat.error() << "Chunk size too close or greater than the actual terrain size!" << endl;
    return false;
  }

  do_extract_heightfield();
  do_create_chunks();
  do_compute_bounds(&_base_chunk);
  do_create_chunk_geom();
  do_init_data_texture();

  // Clear image after using it, otherwise we have two copies of the heightfield
  // in memory.
  _heightfield.clear();

  return true;
}

/**
 * @brief Converts the internal used Texture to a PNMImage
 * @details This converts the texture passed with set_heightfield to a PNMImage,
 *   so we can read the pixels in a fast way. This is only used while generating
 *   the chunks, and the PNMImage is destroyed afterwards.
 */
void ShaderTerrainMesh::do_extract_heightfield() {
  if (!_heightfield_tex->has_ram_image()) {
    _heightfield_tex->reload();
  }

  _heightfield_tex->store(_heightfield);

  if (_heightfield.get_maxval() != 65535) {
    shader_terrain_cat.warning() << "Using non 16-bit heightfield!" << endl;
  } else {
    _heightfield_tex->set_format(Texture::F_r16);
  }
  _heightfield_tex->set_minfilter(stm_heightfield_minfilter);
  _heightfield_tex->set_magfilter(stm_heightfield_magfilter);
  _heightfield_tex->set_wrap_u(SamplerState::WM_clamp);
  _heightfield_tex->set_wrap_v(SamplerState::WM_clamp);
}

/**
 * @brief Intermal method to check the heightfield
 * @details This method cecks the heightfield generated from the heightfield texture,
 *   and performs some basic checks, including a check for a power of two,
 *   and same width and height.
 *
 * @return true if the heightfield meets the requirements
 */
bool ShaderTerrainMesh::do_check_heightfield() {
  if (_heightfield_tex->get_x_size() != _heightfield_tex->get_y_size()) {
    shader_terrain_cat.error() << "Only square heightfields are supported!";
    return false;
  }

  _size = _heightfield_tex->get_x_size();
  if (_size < 32 || !check_power_of_two(_size)) {
    shader_terrain_cat.error() << "Invalid heightfield! Needs to be >= 32 and a power of two (was: "
         << _size << ")!" << endl;
    return false;
  }

  return true;
}

/**
 * @brief Internal method to init the terrain data texture
 * @details This method creates the data texture, used to store all chunk data.
 *   The data texture is set as a shader input later on, and stores the position
 *   and scale of each chunk. Every row in the data texture denotes a view on
 *   the terrain.
 */
void ShaderTerrainMesh::do_init_data_texture() {
  _data_texture = new Texture("TerrainDataTexture");
  _data_texture->setup_2d_texture(stm_max_chunk_count, stm_max_views, Texture::T_float, Texture::F_rgba32);
  _data_texture->set_clear_color(LVector4(0));
  _data_texture->clear_image();
}

/**
 * @brief Internal method to init the quadtree
 * @details This method creates the base chunk and then inits all chunks recursively
 *   by using ShaderTerrainMesh::do_init_chunk().
 */
void ShaderTerrainMesh::do_create_chunks() {

  // Release any previously stored children
  _base_chunk.clear_children();

  // Create the base chunk
  _base_chunk.depth = 0;
  _base_chunk.x = 0;
  _base_chunk.y = 0;
  _base_chunk.size = _size;
  _base_chunk.edges.set(0, 0, 0, 0);
  _base_chunk.avg_height = 0.5;
  _base_chunk.min_height = 0.0;
  _base_chunk.max_height = 1.0;
  _base_chunk.last_clod = 0.0;
  do_init_chunk(&_base_chunk);
}

/**
 * @brief Internal method to recursively init the quadtree
 * @details This method inits the quadtree. Starting from a given node, it
 *   first examines if that node should be subdivided.
 *
 *   If the node should be subdivided, four children are created and this method
 *   is called on the children again. If the node is a leaf, all children are
 *   set to NULL and nothing else happens.
 *
 *   The chunk parameter may not be zero or undefined behaviour occurs.
 *
 * @param chunk The parent chunk
 */
void ShaderTerrainMesh::do_init_chunk(Chunk* chunk) {
  if (chunk->size > _chunk_size) {

    // Compute children chunk size
    size_t child_chunk_size = chunk->size / 2;

    // Subdivide chunk into 4 children
    for (size_t y = 0; y < 2; ++y) {
      for (size_t x = 0; x < 2; ++x) {
        Chunk* child = new Chunk();
        child->size = child_chunk_size;
        child->depth = chunk->depth + 1;
        child->x = chunk->x + x * child_chunk_size;
        child->y = chunk->y + y * child_chunk_size;
        do_init_chunk(child);
        chunk->children[x + 2*y] = child;
      }
    }
  } else {
    // Final chunk, initialize all children to zero
    for (size_t i = 0; i < 4; ++i) {
      chunk->children[i] = nullptr;
    }
  }
}

/**
 * @brief Recursively computes the bounds for a given chunk
 * @details This method takes a parent chunk, and computes the bounds recursively,
 *   depending on whether the chunk is a leaf or a node.
 *
 *   If the chunk is a leaf, then the average, min and max values for that chunk
 *   are computed by iterating over the heightfield region of that chunk.
 *
 *   If the chunk is a node, this method is called recursively on all children
 *   first, and after that, the average, min and max values for that chunk
 *   are computed by merging those values of the children.
 *
 *   If chunk is NULL, undefined behaviour occurs.
 *
 * @param chunk The parent chunk
 */
void ShaderTerrainMesh::do_compute_bounds(Chunk* chunk) {

  // Final chunk (Leaf)
  if (chunk->size == _chunk_size) {

    // Get a pointer to the PNMImage data, this is faster than using get_xel()
    // for all pixels, since get_xel() also includes bounds checks and so on.
    xel* data = _heightfield.get_array();

    // Pixel getter function. Note that we have to flip the Y-component, since
    // panda itself also flips it
    // auto get_xel = [&](size_t x, size_t y){ return data[x + (_size - 1 - y) * _size].b / (PN_stdfloat)PGM_MAXMAXVAL; };
    #define get_xel(x, y) (data[(x) + (_size - 1 - (y)) * _size].b / (PN_stdfloat)PGM_MAXMAXVAL)

    // Iterate over all pixels
    PN_stdfloat avg_height = 0.0, min_height = 1.0, max_height = 0.0;
    for (size_t x = 0; x < _chunk_size; ++x) {
      for (size_t y = 0; y < _chunk_size; ++y) {

        // Access data directly, to improve performance
        PN_stdfloat height = get_xel(chunk->x + x, chunk->y + y);
        avg_height += height;
        min_height = min(min_height, height);
        max_height = max(max_height, height);
      }
    }

    // Normalize average height
    avg_height /= _chunk_size * _chunk_size;

    // Store values
    chunk->min_height = min_height;
    chunk->max_height = max_height;
    chunk->avg_height = avg_height;

    // Get edges in the order (0, 0) (1, 0) (0, 1) (1, 1)
    for (size_t y = 0; y < 2; ++y) {
      for (size_t x = 0; x < 2; ++x) {
        chunk->edges.set_cell(x + 2 * y, get_xel(
            chunk->x + x * (_chunk_size - 1),
            chunk->y + y * (_chunk_size - 1)
          ));
      }
    }

    #undef get_xel

  } else {

    // Reset heights
    chunk->avg_height = 0.0;
    chunk->min_height = 1.0;
    chunk->max_height = 0.0;

    // Perform bounds computation for every children and merge the children values
    for (size_t i = 0; i < 4; ++i) {
      do_compute_bounds(chunk->children[i]);
      chunk->avg_height += chunk->children[i]->avg_height / 4.0;
      chunk->min_height = min(chunk->min_height, chunk->children[i]->min_height);
      chunk->max_height = max(chunk->max_height, chunk->children[i]->max_height);
    }

    // Also take the edge points from the children
    chunk->edges.set_x(chunk->children[0]->edges.get_x());
    chunk->edges.set_y(chunk->children[1]->edges.get_y());
    chunk->edges.set_z(chunk->children[2]->edges.get_z());
    chunk->edges.set_w(chunk->children[3]->edges.get_w());
  }
}

/**
 * @brief Internal method to create the chunk geom
 * @details This method generates the internal used base chunk. The base chunk geom
 *   is used to render the actual terrain, and will get instanced for every chunk.
 *
 *   The chunk has a size of (size+3) * (size+3), since additional triangles are
 *   inserted at the borders to prevent holes between chunks of a different LOD.
 *
 *   If the generate patches option is set, patches will be generated instead
 *   of triangles, which allows the terrain to use a tesselation shader.
 */
void ShaderTerrainMesh::do_create_chunk_geom() {

  // Convert chunk size to an integer, because we operate on integers and get
  // signed/unsigned mismatches otherwise
  int size = (int)_chunk_size;

  // Create vertex data
  PT(GeomVertexData) gvd = new GeomVertexData("vertices", GeomVertexFormat::get_v3(), Geom::UH_static);
  gvd->reserve_num_rows( (size + 3) * (size + 3) );
  GeomVertexWriter vertex_writer(gvd, "vertex");

  // Create primitive
  PT(GeomPrimitive) triangles = nullptr;
  if (_generate_patches) {
    triangles = new GeomPatches(3, Geom::UH_static);
  } else {
    triangles = new GeomTriangles(Geom::UH_static);
  }

  // Insert chunk vertices
  for (int y = -1; y <= size + 1; ++y) {
    for (int x = -1; x <= size + 1; ++x) {
      LVector3 vtx_pos(x / (PN_stdfloat)size, y / (PN_stdfloat)size, 0.0f);
      // Stitched vertices at the cornders
      if (x == -1 || y == -1 || x == size + 1 || y == size + 1) {
        vtx_pos.set_z(-1.0f / (PN_stdfloat)size);
        vtx_pos.set_x(max((PN_stdfloat)0, min((PN_stdfloat)1, vtx_pos.get_x())));
        vtx_pos.set_y(max((PN_stdfloat)0, min((PN_stdfloat)1, vtx_pos.get_y())));
      }
      vertex_writer.add_data3(vtx_pos);
    }
  }

  // Its important to use int and not size_t here, since we do store negative values
  // auto get_point_index = [&size](int x, int y){ return (x + 1) + (size + 3) * (y + 1); };
  #define get_point_index(x, y) (((x) + 1) + (size + 3) * ((y) + 1))

  // Create triangles
  for (int y = -1; y <= size; ++y) {
    for (int x = -1; x <= size; ++x) {
      // Get point indices of the quad vertices
      int tl = get_point_index(x, y);
      int tr = get_point_index(x + 1, y);
      int bl = get_point_index(x, y + 1);
      int br = get_point_index(x + 1, y + 1);

      // Vary triangle scheme on each uneven quad
      if (stm_use_hexagonal_layout && (x + y) % 2 == 0 ) {
        triangles->add_vertices(tl, tr, br);
        triangles->add_vertices(tl, br, bl);
      } else {
        triangles->add_vertices(tl, tr, bl);
        triangles->add_vertices(bl, tr, br);
      }
    }
  }

  #undef get_point_index

  // Construct geom
  PT(Geom) geom = new Geom(gvd);
  geom->add_primitive(triangles);

  // Do not set any bounds, we do culling ourself
  geom->clear_bounds();
  geom->set_bounds(new OmniBoundingVolume());
  _chunk_geom = geom;
}

/**
 * @copydoc PandaNode::is_renderable()
 */
bool ShaderTerrainMesh::is_renderable() const {
  return true;
}

/**
 * @copydoc PandaNode::is_renderable()
 */
bool ShaderTerrainMesh::safe_to_flatten() const {
  return false;
}

/**
 * @copydoc PandaNode::safe_to_combine()
 */
bool ShaderTerrainMesh::safe_to_combine() const {
  return false;
}

/**
 * @copydoc PandaNode::add_for_draw()
 */
void ShaderTerrainMesh::add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  MutexHolder holder(_lock);

  // Make sure the terrain was properly initialized, and the geom was created
  // successfully
  nassertv(_data_texture != nullptr);
  nassertv(_chunk_geom != nullptr);

  _basic_collector.start();

  // Get current frame count
  int frame_count = ClockObject::get_global_clock()->get_frame_count();

  if (_last_frame_count != frame_count) {
    // Frame count changed, this means we are at the beginning of a new frame.
    // In this case, update the frame count and reset the view index.
    _last_frame_count = frame_count;
    _current_view_index = 0;
  }

  // Get transform and render state for this render pass
  CPT(TransformState) modelview_transform = data.get_internal_transform(trav);
  CPT(RenderState) state = data._state->compose(get_state());

  // Store a handle to the scene setup
  const SceneSetup* scene = trav->get_scene();

  // Get the MVP matrix, this is required for the LOD
  const Lens* current_lens = scene->get_lens();
  const LMatrix4& projection_mat = current_lens->get_projection_mat();

  // Get the current lens bounds
  PT(BoundingVolume) cam_bounds = scene->get_cull_bounds();

  // Transform the camera bounds with the main camera transform
  DCAST(GeometricBoundingVolume, cam_bounds)->xform(scene->get_camera_transform()->get_mat());

  TraversalData traversal_data;
  traversal_data.cam_bounds = cam_bounds;
  traversal_data.model_mat = get_transform()->get_mat();
  traversal_data.mvp_mat = modelview_transform->get_mat() * projection_mat;
  traversal_data.emitted_chunks = 0;
  traversal_data.storage_ptr = (ChunkDataEntry*)_data_texture->modify_ram_image().p();
  traversal_data.screen_size.set(scene->get_viewport_width(), scene->get_viewport_height());

  // Move write pointer so it points to the beginning of the current view
  traversal_data.storage_ptr += _data_texture->get_x_size() * _current_view_index;

  if (_update_enabled) {
    // Traverse recursively
    _lod_collector.start();
    do_traverse(&_base_chunk, &traversal_data);
    _lod_collector.stop();
  } else {
    // Do a rough guess of the emitted chunks, we don't know the actual count
    // (we would have to store it). This is only for debugging anyways, so
    // its not important we get an accurate count here.
    traversal_data.emitted_chunks = _data_texture->get_x_size();
  }

  // Set shader inputs
  CPT(RenderAttrib) current_shader_attrib = state->get_attrib_def(ShaderAttrib::get_class_slot());

  // Make sure the user didn't forget to set a shader
  if (!DCAST(ShaderAttrib, current_shader_attrib)->has_shader()) {
    shader_terrain_cat.warning() << "No shader set on the terrain! You need to set the appropriate shader!" << endl;
  }

  // Should never happen
  nassertv(current_shader_attrib != nullptr);

  current_shader_attrib = DCAST(ShaderAttrib, current_shader_attrib)->set_shader_input(
    ShaderInput("ShaderTerrainMesh.terrain_size", LVecBase2i(_size)));
  current_shader_attrib = DCAST(ShaderAttrib, current_shader_attrib)->set_shader_input(
    ShaderInput("ShaderTerrainMesh.chunk_size", LVecBase2i(_chunk_size)));
  current_shader_attrib = DCAST(ShaderAttrib, current_shader_attrib)->set_shader_input(
    ShaderInput("ShaderTerrainMesh.view_index", LVecBase2i(_current_view_index)));
  current_shader_attrib = DCAST(ShaderAttrib, current_shader_attrib)->set_shader_input(
    ShaderInput("ShaderTerrainMesh.data_texture", _data_texture));
  current_shader_attrib = DCAST(ShaderAttrib, current_shader_attrib)->set_shader_input(
    ShaderInput("ShaderTerrainMesh.heightfield", _heightfield_tex));
  current_shader_attrib = DCAST(ShaderAttrib, current_shader_attrib)->set_instance_count(
    traversal_data.emitted_chunks);

  state = state->set_attrib(current_shader_attrib, 10000);

  // Emit chunk
  CullableObject *object = new CullableObject(_chunk_geom, std::move(state), std::move(modelview_transform));
  trav->get_cull_handler()->record_object(object, trav);

  // After rendering, increment the view index
  ++_current_view_index;

  if (_current_view_index > (size_t)stm_max_views) {
    shader_terrain_cat.error() << "More views than supported! Increase the stm-max-views config variable!" << endl;
  }

  _basic_collector.stop();
}

/**
 * This is used to support NodePath::calc_tight_bounds().  It is not intended
 * to be called directly, and it has nothing to do with the normal Panda
 * bounding-volume computation.
 *
 * If the node contains any geometry, this updates min_point and max_point to
 * enclose its bounding box.  found_any is to be set true if the node has any
 * geometry at all, or left alone if it has none.  This method may be called
 * over several nodes, so it may enter with min_point, max_point, and
 * found_any already set.
 */
CPT(TransformState) ShaderTerrainMesh::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {
  CPT(TransformState) next_transform =
    PandaNode::calc_tight_bounds(min_point, max_point, found_any, transform,
                                 current_thread);

  const LMatrix4 &mat = next_transform->get_mat();
  LPoint3 terrain_min_point = LPoint3(0, 0, 0) * mat;
  LPoint3 terrain_max_point = LPoint3(1, 1, 1) * mat;
  if (!found_any) {
    min_point = terrain_min_point;
    max_point = terrain_max_point;
    found_any = true;
  } else {
    min_point = min_point.fmin(terrain_min_point);
    max_point = max_point.fmax(terrain_max_point);
  }

  return next_transform;
}

/**
 * Returns a newly-allocated BoundingVolume that represents the internal
 * contents of the node.  Should be overridden by PandaNode classes that
 * contain something internally.
 */
void ShaderTerrainMesh::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {

  BoundingVolume::BoundsType btype = get_bounds_type();
  if (btype == BoundingVolume::BT_default) {
    btype = bounds_type;
  }

  if (btype == BoundingVolume::BT_sphere) {
    internal_bounds = new BoundingSphere(LPoint3(0.5, 0.5, 0.5), csqrt(0.75));
  } else {
    internal_bounds = new BoundingBox(LPoint3(0, 0, 0), LPoint3(1, 1, 1));
  }

  internal_vertices = 0;
}

/**
 * @brief Traverses the quadtree
 * @details This method traverses the given chunk, deciding whether it should
 *   be rendered or subdivided.
 *
 *   In case the chunk is decided to be subdivided, this method is called on
 *   all children.
 *
 *   In case the chunk is decided to be rendered, ShaderTerrainMesh::do_emit_chunk() is
 *   called. Otherwise nothing happens, and the chunk does not get rendered.
 *
 * @param chunk Chunk to traverse
 * @param data Traversal data
 */
void ShaderTerrainMesh::do_traverse(Chunk* chunk, TraversalData* data, bool fully_visible) {

  // Don't check bounds if we are fully visible
  if (!fully_visible) {

    // Construct chunk bounding volume
    PN_stdfloat scale = 1.0 / (PN_stdfloat)_size;
    LPoint3 bb_min(chunk->x * scale, chunk->y * scale, chunk->min_height);
    LPoint3 bb_max((chunk->x + chunk->size) * scale, (chunk->y + chunk->size) * scale, chunk->max_height);

    BoundingBox bbox = BoundingBox(bb_min, bb_max);
    DCAST(GeometricBoundingVolume, &bbox)->xform(data->model_mat);
    int intersection = data->cam_bounds->contains(&bbox);

    if (intersection == BoundingVolume::IF_no_intersection) {
      // No intersection with frustum
      return;
    }

    // If the bounds are fully visible, there is no reason to perform culling
    // on the children, so we set this flag to prevent any bounding computation
    // on the child nodes.
    fully_visible = (intersection & BoundingVolume::IF_all) != 0;
  }

  // Check if the chunk should be subdivided. In case the chunk is a leaf node,
  // the chunk will never get subdivided.
  // NOTE: We still always perform the LOD check. This is for the reason that
  // the lod check also computes the CLOD factor, which is useful.
  if (do_check_lod_matches(chunk, data) || chunk->size == _chunk_size) {
    do_emit_chunk(chunk, data);
  } else {
    // Traverse children
    for (size_t i = 0; i < 4; ++i) {
      do_traverse(chunk->children[i], data, fully_visible);
    }
  }
}

/**
 * @brief Checks whether a chunk should get subdivided
 * @details This method checks whether a chunk fits on screen, or should be
 *   subdivided in order to provide bigger detail.
 *
 *   In case this method returns true, the chunk lod is fine, and the chunk
 *   can be rendered. If the method returns false, the chunk should be subdivided.
 *
 * @param chunk Chunk to check
 * @param data Traversal data
 *
 * @return true if the chunk is sufficient, false if the chunk should be subdivided
 */
bool ShaderTerrainMesh::do_check_lod_matches(Chunk* chunk, TraversalData* data) {

  // Project all points to world space
  LVector2 projected_points[4];
  for (size_t y = 0; y < 2; ++y) {
    for (size_t x = 0; x < 2; ++x) {

      // Compute point in model space (0,0,0 to 1,1,1)
      LVector3 edge_pos = LVector3(
        (PN_stdfloat)(chunk->x + x * (chunk->size - 1)) / (PN_stdfloat)_size,
        (PN_stdfloat)(chunk->y + y * (chunk->size - 1)) / (PN_stdfloat)_size,
        chunk->edges.get_cell(x + 2 * y)
      );
      LVector4 projected = data->mvp_mat.xform(LVector4(edge_pos, 1.0));
      if (projected.get_w() == 0.0) {
        projected.set(0.0, 0.0, -1.0, 1.0f);
      }
      projected *= 1.0 / projected.get_w();
      projected_points[x + 2 * y].set(
        projected.get_x() * data->screen_size.get_x(),
        projected.get_y() * data->screen_size.get_y());
    }
  }

  // Compute the length of the edges in screen space
  PN_stdfloat edge_top = (projected_points[1] - projected_points[3]).length_squared();
  PN_stdfloat edge_right = (projected_points[0] - projected_points[2]).length_squared();
  PN_stdfloat edge_bottom = (projected_points[2] - projected_points[3]).length_squared();
  PN_stdfloat edge_left = (projected_points[0] - projected_points[1]).length_squared();

  // CLOD factor
  PN_stdfloat max_edge = max(edge_top, max(edge_right, max(edge_bottom, edge_left)));

  // Micro-Optimization: We use length_squared() instead of length() to compute the
  // maximum edge length. This reduces it to one csqrt instead of four.
  max_edge = csqrt(max_edge);

  PN_stdfloat tesselation_factor = (max_edge / _target_triangle_width) / (PN_stdfloat)_chunk_size;
  PN_stdfloat clod_factor = max(0.0, min(1.0, 2.0 - tesselation_factor));

  // Store the clod factor
  chunk->last_clod = clod_factor;

  return tesselation_factor <= 2.0;
}

/**
 * @brief Internal method to spawn a chunk
 * @details This method is used to spawn a chunk in case the traversal decided
 *   that the chunk gets rendered. It writes the chunks data to the texture, and
 *   increments the write pointer
 *
 * @param chunk Chunk to spawn
 * @param data Traversal data
 */
void ShaderTerrainMesh::do_emit_chunk(Chunk* chunk, TraversalData* data) {
  if (data->emitted_chunks >= _data_texture->get_x_size()) {

    // Only print warning once
    if (data->emitted_chunks == _data_texture->get_x_size()) {
      shader_terrain_cat.error() << "Too many chunks in the terrain! Consider lowering the desired LOD, or increase the stm-max-chunk-count variable." << endl;
      data->emitted_chunks++;
    }
    return;
  }

  ChunkDataEntry& data_entry = *data->storage_ptr;
  data_entry.x = chunk->x;
  data_entry.y = chunk->y;
  data_entry.size = chunk->size / _chunk_size;
  data_entry.clod = chunk->last_clod;

  data->emitted_chunks++;
  data->storage_ptr++;
}

/**
 * @brief Transforms a texture coordinate to world space
 * @details This transforms a texture coordinatefrom uv-space (0 to 1) to world
 *   space. This takes the terrains transform into account, and also samples the
 *   heightmap. This method should be called after generate().
 *
 * @param coord Coordinate in uv-space from 0, 0 to 1, 1
 * @return World-Space point
 */
LPoint3 ShaderTerrainMesh::uv_to_world(const LTexCoord& coord) const {
  MutexHolder holder(_lock);
  nassertr(_heightfield_tex != nullptr, LPoint3(0)); // Heightfield not set yet
  nassertr(_heightfield_tex->has_ram_image(), LPoint3(0)); // Heightfield not in memory

  PT(TexturePeeker) peeker = _heightfield_tex->peek();
  nassertr(peeker != nullptr, LPoint3(0));

  LColor result;
  if (!peeker->lookup_bilinear(result, coord.get_x(), coord.get_y())) {
    shader_terrain_cat.error() << "UV out of range, cant transform to world!" << endl;
    return LPoint3(0);
  }
  LPoint3 unit_point(coord.get_x(), coord.get_y(), result.get_x());
  return get_transform()->get_mat().xform_point_general(unit_point);
}
