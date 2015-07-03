// Filename: assimpLoader.cxx
// Created by:  rdb (29Mar11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "assimpLoader.h"

#include "geomNode.h"
#include "luse.h"
#include "geomVertexWriter.h"
#include "geomPoints.h"
#include "geomLines.h"
#include "geomTriangles.h"
#include "pnmFileTypeRegistry.h"
#include "pnmImage.h"
#include "materialAttrib.h"
#include "textureAttrib.h"
#include "cullFaceAttrib.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "spotlight.h"
#include "pointLight.h"
#include "look_at.h"
#include "texturePool.h"

#include "pandaIOSystem.h"
#include "pandaLogger.h"

#include "assimp/postprocess.h"

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AssimpLoader::
AssimpLoader() :
  _error (false),
  _geoms (NULL) {

  PandaLogger::set_default();
  _importer.SetIOHandler(new PandaIOSystem);
}

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AssimpLoader::
~AssimpLoader() {
  _importer.FreeScene();
}

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::get_extensions
//       Access: Public
//  Description: Returns a space-separated list of extensions that
//               Assimp can load, without the leading dots.
////////////////////////////////////////////////////////////////////
void AssimpLoader::
get_extensions(string &ext) const {
  aiString aexts;
  _importer.GetExtensionList(aexts);

  // The format is like: *.mdc;*.mdl;*.mesh.xml;*.mot
  char *sub = strtok(aexts.data, ";");
  while (sub != NULL) {
    ext += sub + 2;
    sub = strtok(NULL, ";");

    if (sub != NULL) {
      ext += ' ';
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::read
//       Access: Public
//  Description: Reads from the indicated file.
////////////////////////////////////////////////////////////////////
bool AssimpLoader::
read(const Filename &filename) {
  _filename = filename;

  // I really don't know why we need to flip the winding order,
  // but otherwise the models I tested with are showing inside out.
  _scene = _importer.ReadFile(_filename.c_str(), aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_FlipWindingOrder);
  if (_scene == NULL) {
    _error = true;
    return false;
  }

  _error = false;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::build_graph
//       Access: Public
//  Description: Converts scene graph structures into a Panda3D
//               scene graph, with _root being the root node.
////////////////////////////////////////////////////////////////////
void AssimpLoader::
build_graph() {
  nassertv(_scene != NULL); // read() must be called first
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
  _geoms = new PT(Geom)[_scene->mNumMeshes];
  _geom_matindices = new unsigned int[_scene->mNumMeshes];
  for (size_t i = 0; i < _scene->mNumMeshes; ++i) {
    load_mesh(i);
  }

  // And now the node structure.
  if (_scene->mRootNode != NULL) {
    load_node(*_scene->mRootNode, _root);
  }

  // And lastly, the lights.
  for (size_t i = 0; i < _scene->mNumLights; ++i) {
    load_light(*_scene->mLights[i]);
  }

  delete[] _textures;
  delete[] _mat_states;
  delete[] _geoms;
  delete[] _geom_matindices;
}

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::load_texture
//       Access: Private
//  Description: Converts an aiTexture into a Texture.
////////////////////////////////////////////////////////////////////
void AssimpLoader::
load_texture(size_t index) {
  const aiTexture &tex = *_scene->mTextures[index];

  PT(Texture) ptex = new Texture;

  if (tex.mHeight == 0) {
    // Compressed texture.
    assimp_cat.debug()
      << "Reading embedded compressed texture with format " << tex.achFormatHint << " and size " << tex.mWidth << "\n";
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
        ptex = NULL;
      }
    }
  } else {
    assimp_cat.debug()
      << "Reading embedded raw texture with size " << tex.mWidth << "x" << tex.mHeight << "\n";

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

  //ostringstream path;
  //path << "/tmp/" << index << ".png";
  //ptex->write(path.str());

  _textures[index] = ptex;

}

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::load_texture_stage
//       Access: Private
//  Description: Converts an aiMaterial into a RenderState.
////////////////////////////////////////////////////////////////////
void AssimpLoader::
load_texture_stage(const aiMaterial &mat, const aiTextureType &ttype, CPT(TextureAttrib) &tattr) {
  aiString path;
  aiTextureMapping mapping;
  unsigned int uvindex;
  PN_stdfloat blend;
  aiTextureOp op;
  aiTextureMapMode mapmode;

  for (size_t i = 0; i < mat.GetTextureCount(ttype); ++i) {
    mat.GetTexture(ttype, i, &path, &mapping, NULL, &blend, &op, &mapmode);

    if (AI_SUCCESS != mat.Get(AI_MATKEY_UVWSRC(ttype, i), uvindex)) {
      // If there's no texture coordinate set for this texture,
      // assume that it's the same as the index on the stack.
      //TODO: if there's only one set on the mesh,
      //      force everything to use just the first stage.
      uvindex = i;
    }

    stringstream str;
    str << uvindex;
    PT(TextureStage) stage = new TextureStage(str.str());
    if (uvindex > 0) {
      stage->set_texcoord_name(InternalName::get_texcoord_name(str.str()));
    }
    PT(Texture) ptex = NULL;

    // I'm not sure if this is the right way to handle it, as
    // I couldn't find much information on embedded textures.
    if (path.data[0] == '*') {
      long num = strtol(path.data + 1, NULL, 10);
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

    if (ptex != NULL) {
      tattr = DCAST(TextureAttrib, tattr->add_on_stage(stage, ptex));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::load_material
//       Access: Private
//  Description: Converts an aiMaterial into a RenderState.
////////////////////////////////////////////////////////////////////
void AssimpLoader::
load_material(size_t index) {
  const aiMaterial &mat = *_scene->mMaterials[index];

  CPT(RenderState) state = RenderState::make_empty();

  aiColor3D col;
  bool have;
  int ival;
  PN_stdfloat fval;

  // XXX a lot of this is untested.

  // First do the material attribute.
  PT(Material) pmat = new Material;
  have = false;
  if (AI_SUCCESS == mat.Get(AI_MATKEY_COLOR_DIFFUSE, col)) {
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
  if (AI_SUCCESS == mat.Get(AI_MATKEY_COLOR_AMBIENT, col)) {
    pmat->set_specular(LColor(col.r, col.g, col.b, 1));
    have = true;
  }
  if (AI_SUCCESS == mat.Get(AI_MATKEY_COLOR_EMISSIVE, col)) {
    pmat->set_emission(LColor(col.r, col.g, col.b, 1));
    have = true;
  }
  if (AI_SUCCESS == mat.Get(AI_MATKEY_COLOR_TRANSPARENT, col)) {
    //FIXME: ???
  }
  if (AI_SUCCESS == mat.Get(AI_MATKEY_SHININESS, fval)) {
    pmat->set_shininess(fval);
    have = true;
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

  // Backface culling.  Not sure if this is also supposed to
  // set the twoside flag in the material, I'm guessing not.
  if (AI_SUCCESS == mat.Get(AI_MATKEY_TWOSIDED, ival)) {
    if (ival) {
      state = state->add_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_none));
    } else {
      state = state->add_attrib(CullFaceAttrib::make_default());
    }
  }

  // And let's not forget the textures!
  CPT(TextureAttrib) tattr = DCAST(TextureAttrib, TextureAttrib::make());
  load_texture_stage(mat, aiTextureType_DIFFUSE, tattr);
  load_texture_stage(mat, aiTextureType_LIGHTMAP, tattr);
  if (tattr->get_num_on_stages() > 0) {
    state = state->add_attrib(tattr);
  }

  _mat_states[index] = state;
}

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::load_mesh
//       Access: Private
//  Description: Converts an aiMesh into a Geom.
////////////////////////////////////////////////////////////////////
void AssimpLoader::
load_mesh(size_t index) {
  const aiMesh &mesh = *_scene->mMeshes[index];

  // Create the vertex format.
  PT(GeomVertexArrayFormat) aformat = new GeomVertexArrayFormat;
  aformat->add_column(InternalName::get_vertex(), 3, Geom::NT_stdfloat, Geom::C_point);
  if (mesh.HasNormals()) {
    aformat->add_column(InternalName::get_normal(), 3, Geom::NT_stdfloat, Geom::C_vector);
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
  //TODO: if there is only one UV set, hackily iterate over the texture stages and clear the texcoord name things

  PT(GeomVertexFormat) format = new GeomVertexFormat;
  format->add_array(aformat);

  // Create the GeomVertexData.
  string name (mesh.mName.data, mesh.mName.length);
  PT(GeomVertexData) vdata = new GeomVertexData(name, GeomVertexFormat::register_format(format), Geom::UH_static);
  vdata->unclean_set_num_rows(mesh.mNumVertices);

  // Read out the vertices.
  GeomVertexWriter vertex (vdata, InternalName::get_vertex());
  for (size_t i = 0; i < mesh.mNumVertices; ++i) {
    const aiVector3D &vec = mesh.mVertices[i];
    vertex.add_data3(vec.x, vec.y, vec.z);
  }

  // Now the normals, if any.
  if (mesh.HasNormals()) {
    GeomVertexWriter normal (vdata, InternalName::get_normal());
    for (size_t i = 0; i < mesh.mNumVertices; ++i) {
      const aiVector3D &vec = mesh.mNormals[i];
      normal.add_data3(vec.x, vec.y, vec.z);
    }
  }

  // Vertex colors, if any.  We only import the first set.
  if (mesh.HasVertexColors(0)) {
    GeomVertexWriter color (vdata, InternalName::get_color());
    for (size_t i = 0; i < mesh.mNumVertices; ++i) {
      const aiColor4D &col = mesh.mColors[0][i];
      color.add_data4(col.r, col.g, col.b, col.a);
    }
  }

  // Now the texture coordinates.
  if (num_uvs > 0) {
    // UV sets are named texcoord, texcoord.1, texcoord.2...
    GeomVertexWriter texcoord0 (vdata, InternalName::get_texcoord());
    for (size_t i = 0; i < mesh.mNumVertices; ++i) {
      const aiVector3D &vec = mesh.mTextureCoords[0][i];
      texcoord0.add_data3(vec.x, vec.y, vec.z);
    }
    for (unsigned int u = 1; u < num_uvs; ++u) {
      ostringstream out;
      out << u;
      GeomVertexWriter texcoord (vdata, InternalName::get_texcoord_name(out.str()));
      for (size_t i = 0; i < mesh.mNumVertices; ++i) {
        const aiVector3D &vec = mesh.mTextureCoords[u][i];
        texcoord.add_data3(vec.x, vec.y, vec.z);
      }
    }
  }

  // Now read out the primitives.
  // Keep in mind that we called ReadFile with the aiProcess_Triangulate
  // flag earlier, so we don't have to worry about polygons.
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
  PT(Geom) geom = new Geom(vdata);
  if (points->get_num_primitives() > 0) {
    geom->add_primitive(points);
  }
  if (lines->get_num_primitives() > 0) {
    geom->add_primitive(lines);
  }
  if (triangles->get_num_primitives() > 0) {
    geom->add_primitive(triangles);
  }

  _geoms[index] = geom;
  _geom_matindices[index] = mesh.mMaterialIndex;
}

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::load_node
//       Access: Private
//  Description: Converts an aiNode into a PandaNode.
////////////////////////////////////////////////////////////////////
void AssimpLoader::
load_node(const aiNode &node, PandaNode *parent) {
  PT(PandaNode) pnode;

  // Create the node and give it a name.
  string name (node.mName.data, node.mName.length);
  if (node.mNumMeshes > 0) {
    pnode = new GeomNode(name);
  } else {
    pnode = new PandaNode(name);
  }
  parent->add_child(pnode);

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
    load_node(*node.mChildren[i], pnode);
  }

  if (node.mNumMeshes > 0) {
    // Remember, we created this as GeomNode earlier.
    PT(GeomNode) gnode = DCAST(GeomNode, pnode);
    size_t meshIndex;

    // If there's only mesh, don't bother using a per-geom state.
    if (node.mNumMeshes == 1) {
      meshIndex = node.mMeshes[0];
      gnode->add_geom(_geoms[meshIndex]);
      gnode->set_state(_mat_states[_geom_matindices[meshIndex]]);

    } else {
      for (size_t i = 0; i < node.mNumMeshes; ++i) {
        meshIndex = node.mMeshes[i];
        gnode->add_geom(_geoms[node.mMeshes[i]],
          _mat_states[_geom_matindices[meshIndex]]);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AssimpLoader::load_light
//       Access: Private
//  Description: Converts an aiLight into a LightNode.
////////////////////////////////////////////////////////////////////
void AssimpLoader::
load_light(const aiLight &light) {
  string name (light.mName.data, light.mName.length);
  assimp_cat.debug() << "Found light '" << name << "'\n";

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
    //TODO: translate mAngleInnerCone to an exponent, somehow

    // This *should* be about right.
    vec = light.mDirection;
    LPoint3 pos (light.mPosition.x, light.mPosition.y, light.mPosition.z);
    LQuaternion quat;
    ::look_at(quat, LPoint3(vec.x, vec.y, vec.z), LVector3::up());
    plight->set_transform(TransformState::make_pos_quat_scale(pos, quat, LVecBase3(1, 1, 1)));
    break; }

  // This is a somewhat recent addition to Assimp, so let's be kind to
  // those that don't have an up-to-date version of Assimp.
  case 0x4: //aiLightSource_AMBIENT:
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
