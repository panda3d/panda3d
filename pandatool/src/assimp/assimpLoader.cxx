/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file assimpLoader.cxx
 * @author rdb
 * @date 2011-03-29
 */

#include "assimpLoader.h"

#include "geomNode.h"
#include "luse.h"
#include "geomVertexWriter.h"
#include "geomPoints.h"
#include "geomLines.h"
#include "geomTriangles.h"
#include "pnmFileTypeRegistry.h"
#include "pnmImage.h"
#include "alphaTestAttrib.h"
#include "materialAttrib.h"
#include "textureAttrib.h"
#include "cullFaceAttrib.h"
#include "transparencyAttrib.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "spotlight.h"
#include "pointLight.h"
#include "look_at.h"
#include "texturePool.h"
#include "character.h"
#include "animBundle.h"
#include "animBundleNode.h"
#include "animChannelMatrixXfmTable.h"
#include "pvector.h"
#include "cmath.h"
#include "deg_2_rad.h"
#include "string_utils.h"

#include "pandaIOSystem.h"
#include "pandaLogger.h"

#include <assimp/postprocess.h>

#ifndef AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR
#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR "$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 0
#endif

#ifndef AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR
#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR "$mat.gltf.pbrMetallicRoughness.metallicFactor", 0, 0
#endif

#ifndef AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR
#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR "$mat.gltf.pbrMetallicRoughness.roughnessFactor", 0, 0
#endif

#ifndef AI_MATKEY_GLTF_ALPHAMODE
#define AI_MATKEY_GLTF_ALPHAMODE "$mat.gltf.alphaMode", 0, 0
#endif

#ifndef AI_MATKEY_GLTF_ALPHACUTOFF
#define AI_MATKEY_GLTF_ALPHACUTOFF "$mat.gltf.alphaCutoff", 0, 0
#endif

// Older versions of Assimp used these glTF-specific keys instead.
#ifndef AI_MATKEY_BASE_COLOR
#define AI_MATKEY_BASE_COLOR AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR
#endif

#ifndef AI_MATKEY_METALLIC_FACTOR
#define AI_MATKEY_METALLIC_FACTOR AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR
#endif

#ifndef AI_MATKEY_ROUGHNESS_FACTOR
#define AI_MATKEY_ROUGHNESS_FACTOR AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR
#endif

using std::ostringstream;
using std::stringstream;
using std::string;

struct BoneWeight {
  CPT(JointVertexTransform) joint_vertex_xform;
  float weight;

  BoneWeight(CPT(JointVertexTransform) joint_vertex_xform, float weight)
    : joint_vertex_xform(joint_vertex_xform), weight(weight)
  {}
};
typedef pvector<BoneWeight> BoneWeightList;

/**
 *
 */
AssimpLoader::
AssimpLoader() :
  _error (false),
  _geoms (nullptr) {

  PandaLogger::set_default();
  _importer.SetIOHandler(new PandaIOSystem);
}

/**
 *
 */
AssimpLoader::
~AssimpLoader() {
  _importer.FreeScene();
}

/**
 * Returns a space-separated list of extensions that Assimp can load, without
 * the leading dots.
 */
void AssimpLoader::
get_extensions(string &ext) const {
  aiString aexts;
  _importer.GetExtensionList(aexts);

  // The format is like: *.mdc;*.mdl;*.mesh.xml;*.mot
  char *sub = strtok(aexts.data, ";");
  while (sub != nullptr) {
    ext += sub + 2;
    sub = strtok(nullptr, ";");

    if (sub != nullptr) {
      ext += ' ';
    }
  }
}

/**
 * Reads from the indicated file.
 */
bool AssimpLoader::
read(const Filename &filename) {
  _filename = filename;

  unsigned int flags = aiProcess_Triangulate | aiProcess_GenUVCoords;

  if (assimp_calc_tangent_space) {
    flags |= aiProcess_CalcTangentSpace;
  }
  if (assimp_join_identical_vertices) {
    flags |= aiProcess_JoinIdenticalVertices;
  }
  if (assimp_improve_cache_locality) {
    flags |= aiProcess_ImproveCacheLocality;
  }
  if (assimp_remove_redundant_materials) {
    flags |= aiProcess_RemoveRedundantMaterials;
  }
  if (assimp_fix_infacing_normals) {
    flags |= aiProcess_FixInfacingNormals;
  }
  if (assimp_optimize_meshes) {
    flags |= aiProcess_OptimizeMeshes;
  }
  if (assimp_optimize_graph) {
    flags |= aiProcess_OptimizeGraph;
  }
  if (assimp_flip_winding_order) {
    flags |= aiProcess_FlipWindingOrder;
  }
  if (assimp_gen_normals) {
    if (assimp_smooth_normal_angle == 0.0) {
      flags |= aiProcess_GenNormals;
    }
    else {
      flags |= aiProcess_GenSmoothNormals;
      _importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE,
        assimp_smooth_normal_angle);
    }
  }

  _scene = _importer.ReadFile(_filename.c_str(), flags);
  if (_scene == nullptr) {
    _error = true;
    return false;
  }

  _error = false;
  return true;
}

/**
 * Converts scene graph structures into a Panda3D scene graph, with _root
 * being the root node.
 */
void AssimpLoader::
build_graph() {
  nassertv(_scene != nullptr); // read() must be called first
  nassertv(!_error);        // and have succeeded

  // Protect the import process
  MutexHolder holder(_lock);

  _root = new ModelRoot(_filename.get_basename());

  // Import all of the embedded textures first.
  _textures = new PT(Texture)[_scene->mNumTextures];
  for (size_t i = 0; i < _scene->mNumTextures; ++i) {
    load_texture(i);
  }

  // Then the materials.
  _mat_states = new CPT(RenderState)[_scene->mNumMaterials];
  for (size_t i = 0; i < _scene->mNumMaterials; ++i) {
    load_material(i);
  }

  // And then the meshes.
  _geoms = new Geoms[_scene->mNumMeshes];
  for (size_t i = 0; i < _scene->mNumMeshes; ++i) {
    load_mesh(i);
  }

  // And now the node structure.
  if (_scene->mRootNode != nullptr) {
    load_node(*_scene->mRootNode, _root);
  }

  // And lastly, the lights.
  for (size_t i = 0; i < _scene->mNumLights; ++i) {
    load_light(*_scene->mLights[i]);
  }

  delete[] _textures;
  delete[] _mat_states;
  delete[] _geoms;
}

/**
 * Finds a node by name.
 */
const aiNode *AssimpLoader::
find_node(const aiNode &root, const aiString &name) {
  const aiNode *node;

  if (root.mName == name) {
    return &root;
  } else {
    for (size_t i = 0; i < root.mNumChildren; ++i) {
      node = find_node(*root.mChildren[i], name);
      if (node) {
          return node;
      }
    }
  }

  return nullptr;
}

/**
 * Converts an aiTexture into a Texture.
 */
void AssimpLoader::
load_texture(size_t index) {
  const aiTexture &tex = *_scene->mTextures[index];

  PT(Texture) ptex = new Texture;

  if (tex.mHeight == 0) {
    // Compressed texture.
    if (assimp_cat.is_debug()) {
      assimp_cat.debug()
        << "Reading embedded compressed texture with format "
        << tex.achFormatHint << " and size " << tex.mWidth << "\n";
    }
    stringstream str;
    str.write((char*) tex.pcData, tex.mWidth);

    if (strncmp(tex.achFormatHint, "dds", 3) == 0) {
      ptex->read_dds(str);

    } else {
      const PNMFileTypeRegistry *reg = PNMFileTypeRegistry::get_global_ptr();
      PNMFileType *ftype;
      PNMImage img;

      // Work around a bug in Assimp, it sometimes writes jp instead of jpg
      if (strncmp(tex.achFormatHint, "jp\0", 3) == 0) {
        ftype = reg->get_type_from_extension("jpg");
      } else {
        ftype = reg->get_type_from_extension(tex.achFormatHint);
      }

      if (img.read(str, "", ftype)) {
        ptex->load(img);
      } else {
        ptex = nullptr;
      }
    }
  } else {
    if (assimp_cat.is_debug()) {
      assimp_cat.debug()
        << "Reading embedded raw texture with size "
        << tex.mWidth << "x" << tex.mHeight << "\n";
    }

    ptex->setup_2d_texture(tex.mWidth, tex.mHeight, Texture::T_unsigned_byte, Texture::F_rgba);
    PTA_uchar data = ptex->modify_ram_image();

    size_t p = 0;
    for (size_t i = 0; i < tex.mWidth * tex.mHeight; ++i) {
      const aiTexel &texel = tex.pcData[i];
      data[p++] = texel.b;
      data[p++] = texel.g;
      data[p++] = texel.r;
      data[p++] = texel.a;
    }
  }

  // ostringstream path; path << "tmp" << index << ".png";
  // ptex->write(path.str());

  _textures[index] = ptex;

}

/**
 * Converts an aiMaterial into a RenderState.
 */
void AssimpLoader::
load_texture_stage(const aiMaterial &mat, const aiTextureType &ttype,
                   TextureStage::Mode mode, CPT(TextureAttrib) &tattr,
                   CPT(TexMatrixAttrib) &tmattr) {
  aiString path;
  aiTextureMapping mapping;
  unsigned int uvindex;
  float blend;
  aiTextureOp op;
  aiTextureMapMode mapmode[3];

  for (size_t i = 0; i < mat.GetTextureCount(ttype); ++i) {
    mat.GetTexture(ttype, i, &path, &mapping, nullptr, &blend, &op, mapmode);

    if (AI_SUCCESS != mat.Get(AI_MATKEY_UVWSRC(ttype, i), uvindex)) {
      // If there's no texture coordinate set for this texture, assume that
      // it's the same as the index on the stack.  TODO: if there's only one
      // set on the mesh, force everything to use just the first stage.
      uvindex = i;
    }

    if (ttype == aiTextureType_DIFFUSE && i == 1) {
      // The glTF 2 importer duplicates this slot in older versions of Assimp.
      // Since glTF doesn't support multiple diffuse textures anyway, we check
      // for this old glTF-specific key, and if present, ignore this texture.
      aiColor4D col;
      if (AI_SUCCESS == mat.Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, col)) {
        return;
      }
    }

    std::string uvindex_str = format_string(uvindex);
    PT(TextureStage) stage = new TextureStage(uvindex_str);
    stage->set_mode(mode);
    if (uvindex > 0) {
      stage->set_texcoord_name(InternalName::get_texcoord_name(uvindex_str));
    }
    PT(Texture) ptex;

    // I'm not sure if this is the right way to handle it, as I couldn't find
    // much information on embedded textures.
    if (path.data[0] == '*') {
      long num = strtol(path.data + 1, nullptr, 10);
      ptex = _textures[num];

    } else if (path.length > 0) {
      Filename fn = Filename::from_os_specific(string(path.data, path.length));

      // Try to find the file by moving up twice in the hierarchy.
      VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
      Filename dir (_filename);
      _filename.make_canonical();
      dir = _filename.get_dirname();

      // Quake 3 BSP doesn't specify an extension for textures.
      if (vfs->is_regular_file(Filename(dir, fn))) {
        fn = Filename(dir, fn);
      } else if (vfs->is_regular_file(Filename(dir, fn + ".tga"))) {
        fn = Filename(dir, fn + ".tga");
      } else if (vfs->is_regular_file(Filename(dir, fn + ".jpg"))) {
        fn = Filename(dir, fn + ".jpg");
      } else {
        dir = _filename.get_dirname();
        if (vfs->is_regular_file(Filename(dir, fn))) {
          fn = Filename(dir, fn);
        } else if (vfs->is_regular_file(Filename(dir, fn + ".tga"))) {
          fn = Filename(dir, fn + ".tga");
        } else if (vfs->is_regular_file(Filename(dir, fn + ".jpg"))) {
          fn = Filename(dir, fn + ".jpg");
        }
      }

      ptex = TexturePool::load_texture(fn);
    }

    if (ptex != nullptr) {
      // Apply the mapping modes.
      switch (mapmode[0]) {
      case aiTextureMapMode_Wrap:
        ptex->set_wrap_u(SamplerState::WM_repeat);
        break;
      case aiTextureMapMode_Clamp:
        ptex->set_wrap_u(SamplerState::WM_clamp);
        break;
      case aiTextureMapMode_Decal:
        ptex->set_wrap_u(SamplerState::WM_border_color);
        ptex->set_border_color(LColor(0, 0, 0, 0));
        break;
      case aiTextureMapMode_Mirror:
        ptex->set_wrap_u(SamplerState::WM_mirror);
        break;
      default:
        break;
      }
      switch (mapmode[1]) {
      case aiTextureMapMode_Wrap:
        ptex->set_wrap_v(SamplerState::WM_repeat);
        break;
      case aiTextureMapMode_Clamp:
        ptex->set_wrap_v(SamplerState::WM_clamp);
        break;
      case aiTextureMapMode_Decal:
        ptex->set_wrap_v(SamplerState::WM_border_color);
        ptex->set_border_color(LColor(0, 0, 0, 0));
        break;
      case aiTextureMapMode_Mirror:
        ptex->set_wrap_v(SamplerState::WM_mirror);
        break;
      default:
        break;
      }
      switch (mapmode[2]) {
      case aiTextureMapMode_Wrap:
        ptex->set_wrap_w(SamplerState::WM_repeat);
        break;
      case aiTextureMapMode_Clamp:
        ptex->set_wrap_w(SamplerState::WM_clamp);
        break;
      case aiTextureMapMode_Decal:
        ptex->set_wrap_w(SamplerState::WM_border_color);
        ptex->set_border_color(LColor(0, 0, 0, 0));
        break;
      case aiTextureMapMode_Mirror:
        ptex->set_wrap_w(SamplerState::WM_mirror);
        break;
      default:
        break;
      }

      tattr = DCAST(TextureAttrib, tattr->add_on_stage(stage, ptex));

      // Is there a texture transform?
      aiUVTransform transform;
      if (AI_SUCCESS == mat.Get(AI_MATKEY_UVTRANSFORM(ttype, i), transform)) {
        // Reconstruct the original origin from the glTF file.
        PN_stdfloat rcos, rsin;
        csincos(-transform.mRotation, &rsin, &rcos);
        transform.mTranslation.x -= (0.5 * transform.mScaling.x) * (-rcos + rsin + 1);
        transform.mTranslation.y -= ((0.5 * transform.mScaling.y) * (rsin + rcos - 1)) + 1 - transform.mScaling.y;

        LMatrix3 matrix =
          LMatrix3::translate_mat(0, -1) *
          LMatrix3::scale_mat(transform.mScaling.x, transform.mScaling.y) *
          LMatrix3::rotate_mat(rad_2_deg(-transform.mRotation)) *
          LMatrix3::translate_mat(transform.mTranslation.x, 1 + transform.mTranslation.y);

        CPT(TransformState) cstate =
          TransformState::make_mat3(matrix);

        CPT(RenderAttrib) new_attr = (tmattr == nullptr)
          ? TexMatrixAttrib::make(stage, std::move(cstate))
          : tmattr->add_stage(stage, std::move(cstate));
        tmattr = DCAST(TexMatrixAttrib, std::move(new_attr));
      }
    }
  }
}

/**
 * Converts an aiMaterial into a RenderState.
 */
void AssimpLoader::
load_material(size_t index) {
  const aiMaterial &mat = *_scene->mMaterials[index];

  CPT(RenderState) state = RenderState::make_empty();

  aiColor4D col;
  bool have;
  int ival;
  PN_stdfloat fval;

  // XXX a lot of this is untested.

  // First do the material attribute.
  PT(Material) pmat = new Material;
  have = false;
  if (AI_SUCCESS == mat.Get(AI_MATKEY_BASE_COLOR, col)) {
    pmat->set_base_color(LColor(col.r, col.g, col.b, col.a));
    have = true;
  }
  else if (AI_SUCCESS == mat.Get(AI_MATKEY_COLOR_DIFFUSE, col)) {
    pmat->set_diffuse(LColor(col.r, col.g, col.b, 1));
    have = true;
  }
  if (AI_SUCCESS == mat.Get(AI_MATKEY_COLOR_SPECULAR, col)) {
    if (AI_SUCCESS == mat.Get(AI_MATKEY_SHININESS_STRENGTH, fval)) {
      pmat->set_specular(LColor(col.r * fval, col.g * fval, col.b * fval, 1));
    } else {
      pmat->set_specular(LColor(col.r, col.g, col.b, 1));
    }
    have = true;
  }
  //else {
  //  if (AI_SUCCESS == mat.Get(AI_MATKEY_SHININESS_STRENGTH, fval)) {
  //    pmat->set_specular(LColor(fval, fval, fval, 1));
  //  } else {
  //    pmat->set_specular(LColor(1, 1, 1, 1));
  //  }
  //}
  if (AI_SUCCESS == mat.Get(AI_MATKEY_COLOR_AMBIENT, col)) {
    pmat->set_specular(LColor(col.r, col.g, col.b, 1));
    have = true;
  }
  if (AI_SUCCESS == mat.Get(AI_MATKEY_COLOR_EMISSIVE, col)) {
    pmat->set_emission(LColor(col.r, col.g, col.b, 1));
    have = true;
  }
  if (AI_SUCCESS == mat.Get(AI_MATKEY_COLOR_TRANSPARENT, col)) {
    // FIXME: ???
  }
  if (AI_SUCCESS == mat.Get(AI_MATKEY_SHININESS, fval)) {
    pmat->set_shininess(fval);
    have = true;
  }
  if (AI_SUCCESS == mat.Get(AI_MATKEY_METALLIC_FACTOR, fval)) {
    pmat->set_metallic(fval);
    have = true;
  }
  if (AI_SUCCESS == mat.Get(AI_MATKEY_ROUGHNESS_FACTOR, fval)) {
    pmat->set_roughness(fval);
    have = true;
  }
  if (AI_SUCCESS == mat.Get(AI_MATKEY_REFRACTI, fval)) {
    pmat->set_refractive_index(fval);
    have = true;
  }
  else if (pmat->has_metallic()) {
    // Default refractive index to 1.5 for PBR models
    pmat->set_refractive_index(1.5);
  }
  if (have) {
    state = state->add_attrib(MaterialAttrib::make(pmat));
  }

  // Wireframe.
  if (AI_SUCCESS == mat.Get(AI_MATKEY_ENABLE_WIREFRAME, ival)) {
    if (ival) {
      state = state->add_attrib(RenderModeAttrib::make(RenderModeAttrib::M_wireframe));
    } else {
      state = state->add_attrib(RenderModeAttrib::make(RenderModeAttrib::M_filled));
    }
  }

  // Backface culling.  Not sure if this is also supposed to set the twoside
  // flag in the material, I'm guessing not.
  if (AI_SUCCESS == mat.Get(AI_MATKEY_TWOSIDED, ival)) {
    if (ival) {
      state = state->add_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_none));
    } else {
      state = state->add_attrib(CullFaceAttrib::make_default());
    }
  }

  // Alpha mode.
  aiString alpha_mode;
  if (AI_SUCCESS == mat.Get(AI_MATKEY_GLTF_ALPHAMODE, alpha_mode)) {
    if (strcmp(alpha_mode.C_Str(), "MASK") == 0) {
      PN_stdfloat cutoff = 0.5;
      mat.Get(AI_MATKEY_GLTF_ALPHACUTOFF, cutoff);
      state = state->add_attrib(AlphaTestAttrib::make(AlphaTestAttrib::M_greater_equal, cutoff));
    }
    else if (strcmp(alpha_mode.C_Str(), "BLEND") == 0) {
      state = state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    }
  }

  // And let's not forget the textures!
  CPT(TextureAttrib) tattr = DCAST(TextureAttrib, TextureAttrib::make());
  CPT(TexMatrixAttrib) tmattr;
  load_texture_stage(mat, aiTextureType_DIFFUSE, TextureStage::M_modulate, tattr, tmattr);

  // Check for an ORM map, from the glTF/OBJ importer.  glTF also puts it in the
  // LIGHTMAP slot, despite only having the lightmap in the red channel, so we
  // have to ignore it.
  if (mat.GetTextureCount(aiTextureType_UNKNOWN) > 0) {
    load_texture_stage(mat, aiTextureType_UNKNOWN, TextureStage::M_selector, tattr, tmattr);
  } else {
    load_texture_stage(mat, aiTextureType_LIGHTMAP, TextureStage::M_modulate, tattr, tmattr);
  }

  load_texture_stage(mat, aiTextureType_NORMALS, TextureStage::M_normal, tattr, tmattr);
  load_texture_stage(mat, aiTextureType_EMISSIVE, TextureStage::M_emission, tattr, tmattr);
  load_texture_stage(mat, aiTextureType_HEIGHT, TextureStage::M_height, tattr, tmattr);
  if (tattr->get_num_on_stages() > 0) {
    state = state->add_attrib(tattr);
  }
  if (tmattr != nullptr) {
    state = state->add_attrib(tmattr);
  }

  _mat_states[index] = std::move(state);
}

/**
 * Creates a CharacterJoint from an aiNode
 */
void AssimpLoader::
create_joint(Character *character, CharacterJointBundle *bundle, PartGroup *parent, const aiNode &node) {
  const aiMatrix4x4 &t = node.mTransformation;
  LMatrix4 mat(t.a1, t.b1, t.c1, t.d1,
                t.a2, t.b2, t.c2, t.d2,
                t.a3, t.b3, t.c3, t.d3,
                t.a4, t.b4, t.c4, t.d4);
  PT(CharacterJoint) joint = new CharacterJoint(character, bundle, parent, node.mName.C_Str(), mat);

  if (assimp_cat.is_debug()) {
    assimp_cat.debug()
      << "Creating joint for: " << node.mName.C_Str() << "\n";
  }

  for (size_t i = 0; i < node.mNumChildren; ++i) {
    if (_bonemap.find(node.mChildren[i]->mName.C_Str()) != _bonemap.end()) {
      create_joint(character, bundle, joint, *node.mChildren[i]);
    }
  }
}

/**
 * Creates a AnimChannelMatrixXfmTable from an aiNodeAnim
 */
void AssimpLoader::
create_anim_channel(const aiAnimation &anim, AnimBundle *bundle, AnimGroup *parent, const aiNode &node) {
  PT(AnimChannelMatrixXfmTable) group = new AnimChannelMatrixXfmTable(parent, node.mName.C_Str());

  // See if there is a channel for this node
  aiNodeAnim *node_anim = nullptr;
  for (size_t i = 0; i < anim.mNumChannels; ++i) {
    if (anim.mChannels[i]->mNodeName == node.mName) {
      node_anim = anim.mChannels[i];
    }
  }

  if (node_anim) {
    if (assimp_cat.is_debug()) {
      assimp_cat.debug()
        << "Found channel for node: " << node.mName.C_Str() << "\n";
    }
    // assimp_cat.debug() << "Num Position Keys " <<
    // node_anim->mNumPositionKeys << "\n"; assimp_cat.debug() << "Num
    // Rotation Keys " << node_anim->mNumRotationKeys << "\n";
    // assimp_cat.debug() << "Num Scaling Keys " << node_anim->mNumScalingKeys
    // << "\n";

    // Convert positions
    PTA_stdfloat tablex = PTA_stdfloat::empty_array(node_anim->mNumPositionKeys);
    PTA_stdfloat tabley = PTA_stdfloat::empty_array(node_anim->mNumPositionKeys);
    PTA_stdfloat tablez = PTA_stdfloat::empty_array(node_anim->mNumPositionKeys);
    for (size_t i = 0; i < node_anim->mNumPositionKeys; ++i) {
      tablex[i] = node_anim->mPositionKeys[i].mValue.x;
      tabley[i] = node_anim->mPositionKeys[i].mValue.y;
      tablez[i] = node_anim->mPositionKeys[i].mValue.z;
    }
    group->set_table('x', tablex);
    group->set_table('y', tabley);
    group->set_table('z', tablez);

    // Convert rotations
    PTA_stdfloat tableh = PTA_stdfloat::empty_array(node_anim->mNumRotationKeys);
    PTA_stdfloat tablep = PTA_stdfloat::empty_array(node_anim->mNumRotationKeys);
    PTA_stdfloat tabler = PTA_stdfloat::empty_array(node_anim->mNumRotationKeys);
    for (size_t i = 0; i < node_anim->mNumRotationKeys; ++i) {
      aiQuaternion ai_quat = node_anim->mRotationKeys[i].mValue;
      LVecBase3 hpr = LQuaternion(ai_quat.w, ai_quat.x, ai_quat.y, ai_quat.z).get_hpr();
      tableh[i] = hpr.get_x();
      tablep[i] = hpr.get_y();
      tabler[i] = hpr.get_z();
    }
    group->set_table('h', tableh);
    group->set_table('p', tablep);
    group->set_table('r', tabler);

    // Convert scales
    PTA_stdfloat tablei = PTA_stdfloat::empty_array(node_anim->mNumScalingKeys);
    PTA_stdfloat tablej = PTA_stdfloat::empty_array(node_anim->mNumScalingKeys);
    PTA_stdfloat tablek = PTA_stdfloat::empty_array(node_anim->mNumScalingKeys);
    for (size_t i = 0; i < node_anim->mNumScalingKeys; ++i) {
      tablei[i] = node_anim->mScalingKeys[i].mValue.x;
      tablej[i] = node_anim->mScalingKeys[i].mValue.y;
      tablek[i] = node_anim->mScalingKeys[i].mValue.z;
    }
    group->set_table('i', tablei);
    group->set_table('j', tablej);
    group->set_table('k', tablek);
  }
  else if (assimp_cat.is_debug()) {
    assimp_cat.debug()
      << "No channel found for node: " << node.mName.C_Str() << "\n";
  }


  for (size_t i = 0; i < node.mNumChildren; ++i) {
    if (_bonemap.find(node.mChildren[i]->mName.C_Str()) != _bonemap.end()) {
      create_anim_channel(anim, bundle, group, *node.mChildren[i]);
    }
  }
}

/**
 * Converts an aiMesh into a Geom.
 */
void AssimpLoader::
load_mesh(size_t index) {
  const aiMesh &mesh = *_scene->mMeshes[index];

  // Check if we need to make a Character
  PT(Character) character = nullptr;
  if (mesh.HasBones()) {
    if (assimp_cat.is_debug()) {
      assimp_cat.debug()
        << "Creating character for mesh '" << mesh.mName.C_Str() << "' with "
        << mesh.mNumBones << " bones\n";
    }

    // Find and add all bone nodes to the bone map
    for (size_t i = 0; i < mesh.mNumBones; ++i) {
      const aiBone &bone = *mesh.mBones[i];
      const aiNode *node = find_node(*_scene->mRootNode, bone.mName);
      _bonemap[bone.mName.C_Str()] = node;
    }

    // Now create a character from the bones
    character = new Character(mesh.mName.C_Str());
    PT(CharacterJointBundle) bundle = character->get_bundle(0);
    PT(PartGroup) skeleton = new PartGroup(bundle, "<skeleton>");

    for (size_t i = 0; i < mesh.mNumBones; ++i) {
      const aiBone &bone = *mesh.mBones[i];

      // Find the root bone node
      const aiNode *root = _bonemap[bone.mName.C_Str()];
      while (root->mParent && _bonemap.find(root->mParent->mName.C_Str()) != _bonemap.end()) {
        root = root->mParent;
      }

      // Don't process this root if we already have a joint for it
      if (character->find_joint(root->mName.C_Str())) {
        continue;
      }

      create_joint(character, bundle, skeleton, *root);
    }
  }

  // Create transform blend table
  PT(TransformBlendTable) tbtable = new TransformBlendTable;
  pvector<BoneWeightList> bone_weights(mesh.mNumVertices);
  if (character) {
    for (size_t i = 0; i < mesh.mNumBones; ++i) {
      const aiBone &bone = *mesh.mBones[i];
      CharacterJoint *joint = character->find_joint(bone.mName.C_Str());
      if (joint == nullptr) {
        if (assimp_cat.is_debug()) {
          assimp_cat.debug()
            << "Could not find joint for bone: " << bone.mName.C_Str() << "\n";
        }
        continue;
      }

      CPT(JointVertexTransform) jvt = new JointVertexTransform(joint);

      for (size_t j = 0; j < bone.mNumWeights; ++j) {
          const aiVertexWeight &weight = bone.mWeights[j];

          bone_weights[weight.mVertexId].push_back(BoneWeight(jvt, weight.mWeight));
      }
    }
  }

  // Create the vertex format.
  PT(GeomVertexArrayFormat) aformat = new GeomVertexArrayFormat;
  aformat->add_column(InternalName::get_vertex(), 3, Geom::NT_stdfloat, Geom::C_point);
  if (mesh.HasNormals()) {
    aformat->add_column(InternalName::get_normal(), 3, Geom::NT_stdfloat, Geom::C_normal);
  }
  if (mesh.HasVertexColors(0)) {
    aformat->add_column(InternalName::get_color(), 4, Geom::NT_stdfloat, Geom::C_color);
  }
  unsigned int num_uvs = mesh.GetNumUVChannels();
  if (num_uvs > 0) {
    // UV sets are named texcoord, texcoord.1, texcoord.2...
    aformat->add_column(InternalName::get_texcoord(), 3, Geom::NT_stdfloat, Geom::C_texcoord);
    for (unsigned int u = 1; u < num_uvs; ++u) {
      ostringstream out;
      out << u;
      aformat->add_column(InternalName::get_texcoord_name(out.str()), 3, Geom::NT_stdfloat, Geom::C_texcoord);
    }
  }
  if (mesh.HasTangentsAndBitangents()) {
    aformat->add_column(InternalName::get_tangent(), 3, Geom::NT_stdfloat, Geom::C_vector);
    aformat->add_column(InternalName::get_binormal(), 3, Geom::NT_stdfloat, Geom::C_vector);
  }

  PT(GeomVertexArrayFormat) tb_aformat = new GeomVertexArrayFormat;
  tb_aformat->add_column(InternalName::make("transform_blend"), 1, Geom::NT_uint16, Geom::C_index);

  // Check to see if we need to convert any animations
  for (size_t i = 0; i < _scene->mNumAnimations; ++i) {
    aiAnimation &ai_anim = *_scene->mAnimations[i];
    bool convert_anim = false;

    if (assimp_cat.is_debug()) {
      assimp_cat.debug()
        << "Checking to see if anim (" << ai_anim.mName.C_Str()
        << ") matches character (" << mesh.mName.C_Str() << ")\n";
    }
    for (size_t j = 0; j < ai_anim.mNumChannels; ++j) {
      if (assimp_cat.is_spam()) {
        assimp_cat.spam()
          << "Searching for " << ai_anim.mChannels[j]->mNodeName.C_Str()
          << " in bone map" << "\n";
      }
      if (_bonemap.find(ai_anim.mChannels[j]->mNodeName.C_Str()) != _bonemap.end()) {
        convert_anim = true;
        break;
      }
    }

    if (convert_anim) {
      if (assimp_cat.is_debug()) {
        assimp_cat.debug()
          << "Found animation (" << ai_anim.mName.C_Str()
          << ") for character (" << mesh.mName.C_Str() << ")\n";
      }

      // Now create the animation
      unsigned int frames = 0;
      for (size_t j = 0; j < ai_anim.mNumChannels; ++j) {
        if (ai_anim.mChannels[j]->mNumPositionKeys > frames) {
          frames = ai_anim.mChannels[j]->mNumPositionKeys;
        }
        if (ai_anim.mChannels[j]->mNumRotationKeys > frames) {
          frames = ai_anim.mChannels[j]->mNumRotationKeys;
        }
        if (ai_anim.mChannels[j]->mNumScalingKeys > frames) {
          frames = ai_anim.mChannels[j]->mNumScalingKeys;
        }
      }
      PN_stdfloat fps = frames / (ai_anim.mTicksPerSecond * ai_anim.mDuration);
      if (assimp_cat.is_debug()) {
        assimp_cat.debug()
          << "FPS " << fps << "\n";
        assimp_cat.debug()
          << "Frames " << frames << "\n";
      }

      PT(AnimBundle) bundle = new AnimBundle(mesh.mName.C_Str(), fps, frames);
      PT(AnimGroup) skeleton = new AnimGroup(bundle, "<skeleton>");

      for (size_t i = 0; i < mesh.mNumBones; ++i) {
        const aiBone &bone = *mesh.mBones[i];

        // Find the root bone node
        const aiNode *root = _bonemap[bone.mName.C_Str()];
        while (root->mParent && _bonemap.find(root->mParent->mName.C_Str()) != _bonemap.end()) {
          root = root->mParent;
        }

        // Only convert root nodes
        if (root->mName == bone.mName) {
          create_anim_channel(ai_anim, bundle, skeleton, *root);

          // Attach the animation to the character node
          PT(AnimBundleNode) bundle_node = new AnimBundleNode(bone.mName.C_Str(), bundle);
          character->add_child(bundle_node);
        }
      }
    }
  }

  // TODO: if there is only one UV set, hackily iterate over the texture
  // stages and clear the texcoord name things

  PT(GeomVertexFormat) format = new GeomVertexFormat;
  format->add_array(aformat);
  if (character) {
    format->add_array(tb_aformat);

    GeomVertexAnimationSpec aspec;
    aspec.set_panda();
    format->set_animation(aspec);
  }

  // Create the GeomVertexData.
  string name (mesh.mName.data, mesh.mName.length);
  PT(GeomVertexData) vdata = new GeomVertexData(name, GeomVertexFormat::register_format(format), Geom::UH_static);
  if (character) {
    vdata->set_transform_blend_table(tbtable);
  }
  vdata->unclean_set_num_rows(mesh.mNumVertices);

  // Read out the vertices.
  GeomVertexWriter vertex (vdata, InternalName::get_vertex());
  for (size_t i = 0; i < mesh.mNumVertices; ++i) {
    const aiVector3D &vec = mesh.mVertices[i];
    vertex.set_data3(vec.x, vec.y, vec.z);
  }

  // Now the normals, if any.
  if (mesh.HasNormals()) {
    GeomVertexWriter normal (vdata, InternalName::get_normal());
    for (size_t i = 0; i < mesh.mNumVertices; ++i) {
      const aiVector3D &vec = mesh.mNormals[i];
      normal.set_data3(vec.x, vec.y, vec.z);
    }
  }

  // Vertex colors, if any.  We only import the first set.
  if (mesh.HasVertexColors(0)) {
    GeomVertexWriter color (vdata, InternalName::get_color());
    for (size_t i = 0; i < mesh.mNumVertices; ++i) {
      const aiColor4D &col = mesh.mColors[0][i];
      color.set_data4(col.r, col.g, col.b, col.a);
    }
  }

  // Now the texture coordinates.
  if (num_uvs > 0) {
    // UV sets are named texcoord, texcoord.1, texcoord.2...
    GeomVertexWriter texcoord0 (vdata, InternalName::get_texcoord());
    for (size_t i = 0; i < mesh.mNumVertices; ++i) {
      const aiVector3D &vec = mesh.mTextureCoords[0][i];
      texcoord0.set_data3(vec.x, vec.y, vec.z);
    }
    for (unsigned int u = 1; u < num_uvs; ++u) {
      ostringstream out;
      out << u;
      GeomVertexWriter texcoord (vdata, InternalName::get_texcoord_name(out.str()));
      for (size_t i = 0; i < mesh.mNumVertices; ++i) {
        const aiVector3D &vec = mesh.mTextureCoords[u][i];
        texcoord.set_data3(vec.x, vec.y, vec.z);
      }
    }
  }

  // Now the tangents and bitangents, if any.
  if (mesh.HasTangentsAndBitangents()) {
    GeomVertexWriter tangent (vdata, InternalName::get_tangent());
    GeomVertexWriter binormal (vdata, InternalName::get_binormal());
    for (size_t i = 0; i < mesh.mNumVertices; ++i) {
      const aiVector3D &tvec = mesh.mTangents[i];
      const aiVector3D &bvec = mesh.mBitangents[i];
      tangent.set_data3(tvec.x, tvec.y, tvec.z);
      binormal.set_data3(bvec.x, bvec.y, bvec.z);
    }
  }

  // Now the transform blend table
  if (character) {
    GeomVertexWriter transform_blend (vdata, InternalName::get_transform_blend());

    for (size_t i = 0; i < mesh.mNumVertices; ++i) {
      TransformBlend tblend;

      for (size_t j = 0; j < bone_weights[i].size(); ++j) {
        tblend.add_transform(bone_weights[i][j].joint_vertex_xform, bone_weights[i][j].weight);
      }
      transform_blend.set_data1i(tbtable->add_blend(tblend));
    }

    tbtable->set_rows(SparseArray::lower_on(vdata->get_num_rows()));
  }

  // Now read out the primitives.  Keep in mind that we called ReadFile with
  // the aiProcess_Triangulate flag earlier, so we don't have to worry about
  // polygons.
  PT(GeomPoints) points = new GeomPoints(Geom::UH_static);
  PT(GeomLines) lines = new GeomLines(Geom::UH_static);
  PT(GeomTriangles) triangles = new GeomTriangles(Geom::UH_static);

  // Now add the vertex indices.
  for (size_t i = 0; i < mesh.mNumFaces; ++i) {
    const aiFace &face = mesh.mFaces[i];

    if (face.mNumIndices == 0) {
      // It happens, strangely enough.
      continue;
    } else if (face.mNumIndices == 1) {
      points->add_vertex(face.mIndices[0]);
      points->close_primitive();
    } else if (face.mNumIndices == 2) {
      lines->add_vertices(face.mIndices[0], face.mIndices[1]);
      lines->close_primitive();
    } else if (face.mNumIndices == 3) {
      triangles->add_vertices(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
      triangles->close_primitive();
    } else {
      nassertd(false) continue;
    }
  }

  // Create a geom and add the primitives to it.
  Geoms &geoms = _geoms[index];
  geoms._mat_index = mesh.mMaterialIndex;

  if (points->get_num_primitives() > 0) {
    geoms._points = new Geom(vdata);
    geoms._points->add_primitive(points);
  }
  if (lines->get_num_primitives() > 0) {
    geoms._lines = new Geom(vdata);
    geoms._lines->add_primitive(lines);
  }
  if (triangles->get_num_primitives() > 0) {
    geoms._triangles = new Geom(vdata);
    geoms._triangles->add_primitive(triangles);
  }

  if (character) {
    geoms._character = character;

    PT(GeomNode) gnode = new GeomNode("");
    if (geoms._points != nullptr) {
      gnode->add_geom(geoms._points);
    }
    if (geoms._lines != nullptr) {
      gnode->add_geom(geoms._lines);
    }
    if (geoms._triangles != nullptr) {
      gnode->add_geom(geoms._triangles);
    }
    gnode->set_state(_mat_states[mesh.mMaterialIndex]);
    character->add_child(gnode);
  }
}

/**
 * Converts an aiNode into a PandaNode.  Returns true if the node had anything
 * of interest under it, false otherwise.
 */
bool AssimpLoader::
load_node(const aiNode &node, PandaNode *parent, bool under_joint) {
  PT(PandaNode) pnode;
  string name (node.mName.data, node.mName.length);

  if (assimp_cat.is_debug()) {
    assimp_cat.debug()
      << "Converting node '" << name << "' with " << node.mNumMeshes
      << " meshes and " << node.mNumChildren << " children\n";
  }

  if (!under_joint) {
    under_joint = (_bonemap.find(node.mName.C_Str()) != _bonemap.end());
  }

  bool prune = false;

  if (node.mNumMeshes == 0) {
    if (parent == _root && assimp_collapse_dummy_root_node && !under_joint &&
        (name.empty() || name[0] == '$' || name == "RootNode" || name == "ROOT" || name == "Root" || (name.size() > 2 && name[0] == '<' && name[name.size() - 1] == '>') || name == _root->get_name())) {
      // Collapse root node.
      pnode = _root;
    } else {
      pnode = new PandaNode(name);

      // Possibly prune this if this is a joint or under a joint.
      prune = under_joint;
    }
  }
  else if (node.mNumMeshes == 1) {
    size_t meshIndex = node.mMeshes[0];
    const Geoms &geoms = _geoms[meshIndex];

    if (geoms._character != nullptr) {
      pnode = new PandaNode(name);
      pnode->add_child(geoms._character);
    }
    else {
      PT(GeomNode) gnode = new GeomNode(name);
      const RenderState *state = _mat_states[geoms._mat_index];
      if (geoms._points != nullptr) {
        gnode->add_geom(geoms._points);
      }
      if (geoms._lines != nullptr) {
        gnode->add_geom(geoms._lines);
      }
      if (geoms._triangles != nullptr) {
        gnode->add_geom(geoms._triangles);
      }
      if (state != nullptr) {
        // Only set the state on the GeomNode if there are no child nodes.
        if (node.mNumChildren == 0) {
          gnode->set_state(state);
        } else {
          for (int i = 0; i < gnode->get_num_geoms(); ++i) {
            gnode->set_geom_state(i, state);
          }
        }
      }
      pnode = gnode;
    }
  }
  else {
    // Do we have regular meshes or just animated meshes?
    bool character_only = true;

    // First add all the regular meshes.
    for (size_t i = 0; i < node.mNumMeshes; ++i) {
      size_t meshIndex = node.mMeshes[i];

      if (_geoms[meshIndex]._character == nullptr) {
        character_only = false;
        break;
      }
    }

    PT(GeomNode) gnode;
    if (character_only) {
      pnode = new PandaNode(name);
    } else {
      gnode = new GeomNode(name);
      pnode = gnode;
    }

    for (size_t i = 0; i < node.mNumMeshes; ++i) {
      size_t meshIndex = node.mMeshes[i];
      const Geoms &geoms = _geoms[meshIndex];

      if (geoms._character != nullptr) {
        // An animated mesh, which already is converted as Character with an
        // attached GeomNode.
        pnode->add_child(geoms._character);
      }
      else {
        // A non-animated mesh.
        const RenderState *state = _mat_states[geoms._mat_index];
        if (geoms._points != nullptr) {
          gnode->add_geom(geoms._points, state);
        }
        if (geoms._lines != nullptr) {
          gnode->add_geom(geoms._lines, state);
        }
        if (geoms._triangles != nullptr) {
          gnode->add_geom(geoms._triangles, state);
        }
      }
    }
  }

  if (parent != pnode) {
    parent->add_child(pnode);
  }

  if (node.mMetaData != nullptr) {
    for (unsigned i = 0; i < node.mMetaData->mNumProperties; ++i) {
      const aiMetadataEntry &entry = node.mMetaData->mValues[i];
      std::string value;
      switch (entry.mType) {
      //case AI_BOOL:
      //  value = (*static_cast<bool *>(entry.mData)) ? "1" : "";
      //  break;
      case AI_INT32:
        value = format_string(*static_cast<int32_t *>(entry.mData));
        break;
      case AI_UINT64:
        value = format_string(*static_cast<uint64_t *>(entry.mData));
        break;
      case AI_FLOAT:
        value = format_string(*static_cast<float *>(entry.mData));
        break;
      case AI_DOUBLE:
        value = format_string(*static_cast<double *>(entry.mData));
        break;
      case AI_AISTRING:
        {
          const aiString *str = static_cast<const aiString *>(entry.mData);
          value = std::string(str->data, str->length);
        }
        break;
      default:
        continue;
      }
      const aiString &key = node.mMetaData->mKeys[i];
      pnode->set_tag(std::string(key.data, key.length), std::move(value));
    }
  }

  // Load in the transformation matrix.
  const aiMatrix4x4 &t = node.mTransformation;
  if (!t.IsIdentity()) {
    LMatrix4 mat(t.a1, t.b1, t.c1, t.d1,
                  t.a2, t.b2, t.c2, t.d2,
                  t.a3, t.b3, t.c3, t.d3,
                  t.a4, t.b4, t.c4, t.d4);
    pnode->set_transform(TransformState::make_mat(mat));
  }

  for (size_t i = 0; i < node.mNumChildren; ++i) {
    if (load_node(*node.mChildren[i], pnode, under_joint)) {
      prune = false;
    }
  }

  if (prune) {
    // This is an empty node in a hierarchy of joints, prune it.
    parent->remove_child(pnode);
    if (assimp_cat.is_debug()) {
      assimp_cat.debug()
        << "Pruning node '" << name << "'\n";
    }
    return false;
  } else {
    return true;
  }
}

/**
 * Converts an aiLight into a LightNode.
 */
void AssimpLoader::
load_light(const aiLight &light) {
  string name (light.mName.data, light.mName.length);
  if (assimp_cat.is_debug()) {
    assimp_cat.debug() << "Found light '" << name << "'\n";
  }

  aiColor3D col;
  aiVector3D vec;

  switch (light.mType) {
  case aiLightSource_DIRECTIONAL: {
    PT(DirectionalLight) dlight = new DirectionalLight(name);
    _root->add_child(dlight);

    col = light.mColorDiffuse;
    dlight->set_color(LColor(col.r, col.g, col.b, 1));

    col = light.mColorSpecular;
    dlight->set_specular_color(LColor(col.r, col.g, col.b, 1));

    vec = light.mPosition;
    dlight->set_point(LPoint3(vec.x, vec.y, vec.z));

    vec = light.mDirection;
    dlight->set_direction(LVector3(vec.x, vec.y, vec.z));
    break; }

  case aiLightSource_POINT: {
    PT(PointLight) plight = new PointLight(name);
    _root->add_child(plight);

    col = light.mColorDiffuse;
    plight->set_color(LColor(col.r, col.g, col.b, 1));

    col = light.mColorSpecular;
    plight->set_specular_color(LColor(col.r, col.g, col.b, 1));

    vec = light.mPosition;
    plight->set_point(LPoint3(vec.x, vec.y, vec.z));

    plight->set_attenuation(LVecBase3(light.mAttenuationConstant,
                                       light.mAttenuationLinear,
                                       light.mAttenuationQuadratic));
    break; }

  case aiLightSource_SPOT: {
    PT(Spotlight) plight = new Spotlight(name);
    _root->add_child(plight);

    col = light.mColorDiffuse;
    plight->set_color(LColor(col.r, col.g, col.b, 1));

    col = light.mColorSpecular;
    plight->set_specular_color(LColor(col.r, col.g, col.b, 1));

    plight->set_attenuation(LVecBase3(light.mAttenuationConstant,
                                       light.mAttenuationLinear,
                                       light.mAttenuationQuadratic));

    plight->get_lens()->set_fov(light.mAngleOuterCone);
    // TODO: translate mAngleInnerCone to an exponent, somehow

    // This *should* be about right.
    vec = light.mDirection;
    LPoint3 pos (light.mPosition.x, light.mPosition.y, light.mPosition.z);
    LQuaternion quat;
    ::look_at(quat, LPoint3(vec.x, vec.y, vec.z), LVector3::up());
    plight->set_transform(TransformState::make_pos_quat(pos, quat));
    break; }

  case aiLightSource_AMBIENT:
    // This is handled below.
    break;

  default:
    assimp_cat.warning() << "Light '" << name << "' has an unknown type!\n";
    return;
  }

  // If there's an ambient color, add it as ambient light.
  col = light.mColorAmbient;
  LVecBase4 ambient (col.r, col.g, col.b, 0);
  if (ambient != LVecBase4::zero()) {
    PT(AmbientLight) alight = new AmbientLight(name);
    alight->set_color(ambient);
    _root->add_child(alight);
  }
}
