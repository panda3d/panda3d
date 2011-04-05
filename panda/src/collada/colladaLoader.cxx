// Filename: colladaLoader.cxx
// Created by: Xidram (21Dec10)
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

#include "colladaLoader.h"
#include "virtualFileSystem.h"
#include "luse.h"
#include "string_utils.h"
#include "geomNode.h"
#include "geomVertexWriter.h"
#include "geomTriangles.h"
#include "lightNode.h"
#include "lightAttrib.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "spotlight.h"

// Ugh, undef some macros that conflict with COLLADA.
#undef INLINE
#undef tolower

#include <dae.h>
#include <dom/domCOLLADA.h>
#include <dom/domNode.h>
#include <dom/domVisual_scene.h>
#include <dom/domTranslate.h>
#include <dom/domRotate.h>
#include <dom/domMatrix.h>

#if PANDA_COLLADA_VERSION >= 15
#include <dom/domInstance_with_extra.h>
#else
#include <dom/domInstanceWithExtra.h>
#define domInstance_with_extra domInstanceWithExtra
#define domInput_localRef domInputLocalRef
#define domInput_local_offsetRef domInputLocalOffsetRef
#define domList_of_uints domListOfUInts
#define domList_of_floats domListOfFloats
#endif

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::Constructor
//  Description:
////////////////////////////////////////////////////////////////////
ColladaLoader::
ColladaLoader() :
  _record (NULL),
  _cs (CS_default),
  _error (false),
  _root (NULL),
  _collada (NULL) {

  _dae = new DAE;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::Destructor
//  Description:
////////////////////////////////////////////////////////////////////
ColladaLoader::
~ColladaLoader() {
  delete _dae;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::read
//  Description: Reads from the indicated file.
////////////////////////////////////////////////////////////////////
bool ColladaLoader::
read(const Filename &filename) {
  _filename = filename;

  string data;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  if (!vfs->read_file(_filename, data, true)) {
    collada_cat.error()
      << "Error reading " << _filename << "\n";
    _error = true;
    return false;
  }

  _collada = _dae->openFromMemory(_filename.to_os_specific(), data.c_str());
  _error = (_collada == NULL);
  return !_error;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::build_graph
//  Description: Converts scene graph structures into a Panda3D
//               scene graph, with _root being the root node.
////////////////////////////////////////////////////////////////////
void ColladaLoader::
build_graph() {
  nassertv(_collada); // read() must be called first
  nassertv(!_error);  // and have succeeded

  _root = new ModelRoot(_filename.get_basename());

  domCOLLADA::domScene* scene = _collada->getScene();
  domInstance_with_extra* inst = scene->getInstance_visual_scene();
  domVisual_scene* vscene = daeSafeCast<domVisual_scene> (inst->getUrl().getElement());
  if (vscene) {
    load_visual_scene(*vscene, _root);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::load_visual_scene
//  Description: Loads a visual scene structure.
////////////////////////////////////////////////////////////////////
void ColladaLoader::
load_visual_scene(domVisual_scene& scene, PandaNode *parent) {
  // If we already loaded it before, instantiate the stored node.
  if (scene.getUserData() != NULL) {
    parent->add_child((PandaNode *) scene.getUserData());
    return;
  }

  PT(PandaNode) pnode = new PandaNode(scene.getName());
  scene.setUserData((void *) pnode);
  parent->add_child(pnode);

  // Load in any tags.
  domExtra_Array &extras = scene.getExtra_array();
  for (size_t i = 0; i < extras.getCount(); ++i) {
    load_tags(*extras[i], pnode);
  }

  // Now load in the child nodes.
  domNode_Array &nodes = scene.getNode_array();
  for (size_t i = 0; i < nodes.getCount(); ++i) {
    load_node(*nodes[i], pnode);
  }

  // Apply any lights we've encountered to the visual scene.
  if (_lights.size() > 0) {
    CPT(LightAttrib) lattr = DCAST(LightAttrib, LightAttrib::make());
    pvector<LightNode*>::iterator it;
    for (it = _lights.begin(); it != _lights.end(); ++it) {
      lattr = DCAST(LightAttrib, lattr->add_light(*it));
    }
    pnode->set_state(RenderState::make(lattr));

    _lights.clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::load_node
//  Description: Loads a COLLADA <node>.
////////////////////////////////////////////////////////////////////
void ColladaLoader::
load_node(domNode& node, PandaNode *parent) {
  // If we already loaded it before, instantiate the stored node.
  if (node.getUserData() != NULL) {
    parent->add_child((PandaNode *) node.getUserData());
    return;
  }

  // Create the node.
  PT(PandaNode) pnode;
  pnode = new PandaNode(node.getName());
  node.setUserData((void *) pnode);
  parent->add_child(pnode);

  // Apply the transformation elements in reverse order.
  LMatrix4f transform (LMatrix4f::ident_mat());

  daeElementRefArray &elements = node.getContents();
  for (size_t i = elements.getCount() - 1; i > 0; --i) {
    daeElementRef &elem = elements[i];

    switch (elem->getElementType()) {
      case COLLADA_TYPE::LOOKAT: {
        // Didn't test this, but *should* be right.
        domFloat3x3 &l = (daeSafeCast<domLookat>(elem))->getValue();
        LPoint3f eye (l[0], l[1], l[2]);
        LVector3f up (l[6], l[7], l[8]);
        LVector3f forward = LPoint3f(l[3], l[4], l[5]) - eye;
        forward.normalize();
        LVector3f side = forward.cross(up);
        side.normalize();
        up = side.cross(forward);
        LMatrix4f mat (LMatrix4f::ident_mat());
        mat.set_col(0, side);
        mat.set_col(1, up);
        mat.set_col(2, -forward);
        transform *= mat;
        transform *= LMatrix4f::translate_mat(-eye);
        break;
      }
      case COLLADA_TYPE::MATRIX: {
        domFloat4x4 &m = (daeSafeCast<domMatrix>(elem))->getValue();
        transform *= LMatrix4f(
          m[0], m[4], m[ 8], m[12],
          m[1], m[5], m[ 9], m[13],
          m[2], m[6], m[10], m[14],
          m[3], m[7], m[11], m[15]);
        break;
      }
      case COLLADA_TYPE::ROTATE: {
        domFloat4 &r = (daeSafeCast<domRotate>(elem))->getValue();
        transform *= LMatrix4f::rotate_mat(r[3], LVecBase3f(r[0], r[1], r[2]));
        break;
      }
      case COLLADA_TYPE::SCALE: {
        domFloat3 &s = (daeSafeCast<domScale>(elem))->getValue();
        transform *= LMatrix4f::scale_mat(s[0], s[1], s[2]);
        break;
      }
      case COLLADA_TYPE::SKEW:
        // FIXME: implement skew
        collada_cat.error() << "<skew> not supported yet\n";
        break;
      case COLLADA_TYPE::TRANSLATE: {
        domFloat3 &t = (daeSafeCast<domTranslate>(elem))->getValue();
        transform *= LMatrix4f::translate_mat(t[0], t[1], t[2]);
        break;
      }
    }
  }
  //TODO: convert coordinate systems
  //transform *= LMatrix4f::convert_mat(XXX, _cs);

  // If there's a transform, set it.
  if (transform != LMatrix4f::ident_mat()) {
    pnode->set_transform(TransformState::make_mat(transform));
  }

  // See if this node instantiates any cameras.
  domInstance_camera_Array &caminst = node.getInstance_camera_array();
  for (size_t i = 0; i < caminst.getCount(); ++i) {
    domCamera* target = daeSafeCast<domCamera> (caminst[i]->getUrl().getElement());
    load_camera(*target, pnode);
  }

  // See if this node instantiates any geoms.
  domInstance_geometry_Array &ginst = node.getInstance_geometry_array();
  for (size_t i = 0; i < ginst.getCount(); ++i) {
    domGeometry* target = daeSafeCast<domGeometry> (ginst[i]->getUrl().getElement());
    load_geometry(*target, pnode);
  }

  // See if this node instantiates any lights.
  domInstance_light_Array &linst = node.getInstance_light_array();
  for (size_t i = 0; i < linst.getCount(); ++i) {
    domLight* target = daeSafeCast<domLight> (linst[i]->getUrl().getElement());
    load_light(*target, pnode);
  }

  // And instantiate any <instance_nodes> elements.
  domInstance_node_Array &ninst = node.getInstance_node_array();
  for (size_t i = 0; i < ninst.getCount(); ++i) {
    domNode* target = daeSafeCast<domNode> (ninst[i]->getUrl().getElement());
    load_node(*target, pnode);
  }

  // Now load in the child nodes.
  domNode_Array &nodes = node.getNode_array();
  for (size_t i = 0; i < nodes.getCount(); ++i) {
    load_node(*nodes[i], pnode);
  }

  // Load in any tags.
  domExtra_Array &extras = node.getExtra_array();
  for (size_t i = 0; i < extras.getCount(); ++i) {
    load_tags(*extras[i], pnode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::load_tags
//  Description: Loads tags specified in an <extra> element.
////////////////////////////////////////////////////////////////////
void ColladaLoader::
load_tags(domExtra &extra, PandaNode *node) {
  domTechnique_Array &techniques = extra.getTechnique_array();

  for (size_t t = 0; t < techniques.getCount(); ++t) {
    if (cmp_nocase(techniques[t]->getProfile(), "PANDA3D") == 0) {
      const daeElementRefArray &children = techniques[t]->getChildren();

      for (size_t c = 0; c < children.getCount(); ++c) {
        daeElement &child = *children[c];

        if (cmp_nocase(child.getElementName(), "tag") == 0) {
          const string &name = child.getAttribute("name");
          if (name.size() > 0) {
            node->set_tag(name, child.getCharData());
          } else {
            collada_cat.warning() << "Ignoring <tag> without name attribute\n";
          }
        } else if (cmp_nocase(child.getElementName(), "param") == 0) {
          collada_cat.error() <<
            "Unknown <param> attribute in PANDA3D technique. "
            "Did you mean to use <tag> instead?\n";
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::load_camera
//  Description: Loads a COLLADA <camera> as a Camera object.
////////////////////////////////////////////////////////////////////
void ColladaLoader::
load_camera(domCamera &cam, PandaNode *parent) {
  // If we already loaded it before, instantiate the stored node.
  if (cam.getUserData() != NULL) {
    parent->add_child((PandaNode *) cam.getUserData());
    return;
  }

  //TODO
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::load_geometry
//  Description: Loads a COLLADA <geometry> as a GeomNode object.
////////////////////////////////////////////////////////////////////
void ColladaLoader::
load_geometry(domGeometry &geom, PandaNode *parent) {
  // If we already loaded it before, instantiate the stored node.
  if (geom.getUserData() != NULL) {
    parent->add_child((PandaNode *) geom.getUserData());
    return;
  }

  domMesh* mesh = geom.getMesh();
  if (mesh == NULL) {
    //TODO: support non-mesh geometry.
    return;
  }

  // Create the node.
  PT(GeomNode) gnode = new GeomNode(geom.getName());
  geom.setUserData((void *) gnode);
  parent->add_child(gnode);

  // First handle the <vertices> element.
  domVertices &vertices = *mesh->getVertices();
  daeTArray<domInput_localRef> &vtx_inputs = vertices.getInput_array();

  PT(GeomVertexArrayFormat) vtx_aformat = new GeomVertexArrayFormat;
  PTA_LVecBase4f *vtx_values = new PTA_LVecBase4f[vtx_inputs.getCount()];
  CPT(InternalName) *vtx_names = new CPT(InternalName)[vtx_inputs.getCount()];

  for (size_t i = 0; i < vtx_inputs.getCount(); ++i) {
    const string semantic = vtx_inputs[i]->getSemantic();
    domSource *source = daeSafeCast<domSource>(vtx_inputs[i]->getSource().getElement());
    nassertd(source != NULL) continue;

    vtx_names[i] = load_input(vtx_aformat, vtx_values[i], semantic, *source);
  }

  //TODO: support other than just triangles.
  domTriangles_Array &triangles_array = mesh->getTriangles_array();
  for (size_t i = 0; i < triangles_array.getCount(); ++i) {
    domTriangles &tris = *triangles_array[i];

    if (tris.getP() == NULL) {
      continue;
    }
    domList_of_uints &p = tris.getP()->getValue();

    // Read out the inputs.
    daeTArray<domInput_local_offsetRef> &inputs = tris.getInput_array();

    PT(GeomVertexArrayFormat) aformat = new GeomVertexArrayFormat;
    PTA_LVecBase4f *values = new PTA_LVecBase4f[inputs.getCount()];
    CPT(InternalName) *names = new CPT(InternalName)[inputs.getCount()];

    domUint stride = 1;
    for (size_t in = 0; in < inputs.getCount(); ++in) {
      const string semantic = inputs[in]->getSemantic();
      if (semantic == "VERTEX") {
        names[in] = NULL;
        continue;
      }
      domSource *source = daeSafeCast<domSource>(inputs[in]->getSource().getElement());
      nassertd(source != NULL) continue;

      names[in] = load_input(aformat, values[in], semantic, *source, inputs[i]->getSet());

      if (inputs[in]->getOffset() >= stride) {
        stride = inputs[in]->getOffset() + 1;
      }
    }

    // Create the vertex data.
    PT(GeomVertexFormat) format = new GeomVertexFormat();
    format->add_array(vtx_aformat);
    format->add_array(aformat);
    PT(GeomVertexData) vdata = new GeomVertexData(geom.getName(), GeomVertexFormat::register_format(format), GeomEnums::UH_static);

    // Time to go and write the data.
    PT(GeomTriangles) gtris = new GeomTriangles(GeomEnums::UH_static);
    for (size_t in = 0; in < inputs.getCount(); ++in) {
      if (names[in] == NULL) {
        // Refers to a <vertices> tag, so write the vertex inputs.
        int counter = 0;
        for (size_t in = 0; in < vtx_inputs.getCount(); ++in) {
          GeomVertexWriter writer(vdata, vtx_names[in]);
          for (size_t j = 0; j < p.getCount(); j += stride * 3) {
            for (char v = 0; v < 3; ++v) {
              int idx = p[j + v * stride];
              writer.add_data4f(vtx_values[in][idx]);
              gtris->add_vertex(counter++);
            }
            gtris->close_primitive();
          }
        }
      } else {
        GeomVertexWriter writer(vdata, names[in]);
        for (size_t j = 0; j < p.getCount(); j += stride * 3) {
          for (char v = 0; v < 3; ++v) {
            int idx = p[j + v * stride + inputs[in]->getOffset()];
            writer.add_data4f(values[in][idx]);
          }
        }
      }
    }

    PT(Geom) geom = new Geom(vdata);
    geom->add_primitive(gtris);
    gnode->add_geom(geom);

    delete[] values;
    delete[] names;
  }

  delete[] vtx_values;
  delete[] vtx_names;

  // Load in any tags.
  domExtra_Array &extras = geom.getExtra_array();
  for (size_t i = 0; i < extras.getCount(); ++i) {
    load_tags(*extras[i], gnode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::load_input
//  Description: Takes a semantic and source URI, and adds a new
//               column to the format.  Also fills up the values
//               into the indicated PTA_LVecBase4f.
//               Returns the InternalName of the input.
////////////////////////////////////////////////////////////////////
CPT(InternalName) ColladaLoader::
load_input(GeomVertexArrayFormat *fmt, PTA_LVecBase4f &values,
           const string &semantic, domSource &source, unsigned int set) {

  PT(InternalName) cname;
  GeomEnums::Contents contents = GeomEnums::C_other;

  ostringstream setstr;
  setstr << set;

  if (semantic == "POSITION") {
    cname = InternalName::get_vertex();
    contents = GeomEnums::C_point;
  } else if (semantic == "COLOR") {
    cname = InternalName::get_color();
    contents = GeomEnums::C_color;
  } else if (semantic == "NORMAL") {
    cname = InternalName::get_normal();
    contents = GeomEnums::C_vector;
  } else if (semantic == "TEXCOORD") {
    cname = InternalName::get_texcoord_name(setstr.str());
    contents = GeomEnums::C_texcoord;
  } else if (semantic == "TEXBINORMAL") {
    cname = InternalName::get_binormal_name(setstr.str());
    contents = GeomEnums::C_vector;
  } else if (semantic == "TEXTANGENT") {
    cname = InternalName::get_tangent_name(setstr.str());
    contents = GeomEnums::C_vector;
  } else {
    collada_cat.warning() << "Unrecognized semantic " << semantic << "\n";
    cname = InternalName::make(downcase(semantic));
  }

  // Get this, get that
  domFloat_array* float_array = source.getFloat_array();
  nassertr(float_array != NULL, cname);
  domList_of_floats &floats = float_array->getValue();
  domAccessor &accessor = *source.getTechnique_common()->getAccessor();
  domParam_Array &params = accessor.getParam_array();

  // Count the number of params that have a name attribute.
  domUint num_bound_params = 0;
  for (size_t p = 0; p < params.getCount(); ++p) {
    if (params[p]->getName()) {
      ++num_bound_params;
    }
  }

  domUint pos = accessor.getOffset();
  for (domUint a = 0; a < accessor.getCount(); ++a) {
    domUint c = 0;
    // Yes, the last component defaults to 1 to work around a
    // perspective divide that Panda3D does internally for points.
    LVecBase4f v (0, 0, 0, 1);
    for (domUint p = 0; p < params.getCount(); ++p) {
      if (params[c]->getName()) {
        v._v.data[c++] = floats[pos + p];
      }
    }
    values.push_back(v);
    pos += accessor.getStride();
  }

  fmt->add_column(cname, num_bound_params, GeomEnums::NT_float32, contents);
  return cname;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::load_light
//  Description: Loads a COLLADA <light> as a LightNode object.
////////////////////////////////////////////////////////////////////
void ColladaLoader::
load_light(domLight &light, PandaNode *parent) {
  // If we already loaded it before, instantiate the stored node.
  if (light.getUserData() != NULL) {
    parent->add_child((PandaNode *) light.getUserData());
    return;
  }

  PT(LightNode) lnode;
  domLight::domTechnique_common &tc = *light.getTechnique_common();

  domLight::domTechnique_common::domAmbientRef ambient = tc.getAmbient();
  if (ambient != NULL) {
    PT(AmbientLight) alight = new AmbientLight(light.getName());
    lnode = DCAST(LightNode, alight);

    domFloat3 &color = ambient->getColor()->getValue();
    alight->set_color(Colorf(color[0], color[1], color[2], 1.0));
  }

  domLight::domTechnique_common::domDirectionalRef directional = tc.getDirectional();
  if (directional != NULL) {
    PT(DirectionalLight) dlight = new DirectionalLight(light.getName());
    lnode = DCAST(LightNode, dlight);

    domFloat3 &color = directional->getColor()->getValue();
    dlight->set_color(Colorf(color[0], color[1], color[2], 1.0));
    dlight->set_direction(LVector3f(0, 0, -1));
  }

  domLight::domTechnique_common::domPointRef point = tc.getPoint();
  if (point != NULL) {
    PT(PointLight) plight = new PointLight(light.getName());
    lnode = DCAST(LightNode, plight);

    domFloat3 &color = point->getColor()->getValue();
    plight->set_color(Colorf(color[0], color[1], color[2], 1.0));
    plight->set_attenuation(LVecBase3f(
      point->getConstant_attenuation()->getValue(),
      point->getLinear_attenuation()->getValue(),
      point->getQuadratic_attenuation()->getValue()
    ));
  }

  domLight::domTechnique_common::domSpotRef spot = tc.getSpot();
  if (spot != NULL) {
    PT(Spotlight) slight = new Spotlight(light.getName());
    lnode = DCAST(LightNode, slight);

    domFloat3 &color = spot->getColor()->getValue();
    slight->set_color(Colorf(color[0], color[1], color[2], 1.0));
    slight->set_attenuation(LVecBase3f(
      spot->getConstant_attenuation()->getValue(),
      spot->getLinear_attenuation()->getValue(),
      spot->getQuadratic_attenuation()->getValue()
    ));
    slight->get_lens()->set_fov(spot->getFalloff_angle()->getValue());
    slight->set_exponent(spot->getFalloff_exponent()->getValue());
  }

  if (lnode == NULL) {
    return;
  }
  parent->add_child(lnode);
  _lights.push_back(lnode);
  light.setUserData((void*) lnode);

  // Load in any tags.
  domExtra_Array &extras = light.getExtra_array();
  for (size_t i = 0; i < extras.getCount(); ++i) {
    load_tags(*extras[i], lnode);
  }
}
