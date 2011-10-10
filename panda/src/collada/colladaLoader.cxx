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

#include "colladaBindMaterial.h"
#include "colladaPrimitive.h"

// Collada DOM includes.  No other includes beyond this point.
#include "pre_collada_include.h"
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
#define domTargetable_floatRef domTargetableFloatRef
#endif

#define TOSTRING(x) (x == NULL ? "" : x)

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

  PT(PandaNode) pnode = new PandaNode(TOSTRING(scene.getName()));
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
      lattr = DCAST(LightAttrib, lattr->add_on_light(*it));
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
  pnode = new PandaNode(TOSTRING(node.getName()));
  node.setUserData((void *) pnode);
  parent->add_child(pnode);

  // Apply the transformation elements in reverse order.
  LMatrix4f transform (LMatrix4f::ident_mat());

  daeElementRefArray &elements = node.getContents();
  for (size_t i = elements.getCount(); i > 0; --i) {
    daeElementRef &elem = elements[i - 1];

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
        //FIXME: implement skew
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

  // See if this node instantiates any controllers.
  domInstance_controller_Array &ctrlinst = node.getInstance_controller_array();
  for (size_t i = 0; i < ctrlinst.getCount(); ++i) {
    domController* target = daeSafeCast<domController> (ctrlinst[i]->getUrl().getElement());
    //TODO: implement controllers.  For now, let's just read the geometry
    if (target->getSkin() != NULL) {
      domGeometry* geom = daeSafeCast<domGeometry> (target->getSkin()->getSource().getElement());
      //TODO
      //load_geometry(*geom, ctrlinst[i]->getBind_material(), pnode);
    }
  }

  // See if this node instantiates any geoms.
  domInstance_geometry_Array &ginst = node.getInstance_geometry_array();
  for (size_t i = 0; i < ginst.getCount(); ++i) {
    load_instance_geometry(*ginst[i], pnode);
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
    //TODO: load SI_Visibility under XSI profile
    //TODO: support OpenSceneGraph's switch nodes
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
//     Function: ColladaLoader::load_instance_geometry
//  Description: Loads a COLLADA <instance_geometry> as a GeomNode
//               object.
////////////////////////////////////////////////////////////////////
void ColladaLoader::
load_instance_geometry(domInstance_geometry &inst, PandaNode *parent) {
  // If we already loaded it before, instantiate the stored node.
  if (inst.getUserData() != NULL) {
    parent->add_child((PandaNode *) inst.getUserData());
    return;
  }

  domGeometry* geom = daeSafeCast<domGeometry> (inst.getUrl().getElement());
  nassertv(geom != NULL);

  // Create the node.
  PT(GeomNode) gnode = new GeomNode(TOSTRING(geom->getName()));
  inst.setUserData((void *) gnode);
  parent->add_child(gnode);

  domBind_materialRef bind_mat = inst.getBind_material();
  ColladaBindMaterial cbm;
  if (bind_mat != NULL) {
    cbm.load_bind_material(*bind_mat);
  }

  load_geometry(*geom, gnode, cbm);

  // Load in any tags.
  domExtra_Array &extras = geom->getExtra_array();
  for (size_t i = 0; i < extras.getCount(); ++i) {
    load_tags(*extras[i], gnode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaLoader::load_geometry
//  Description: Loads a COLLADA <geometry> and adds the primitives
//               to the given GeomNode object.
////////////////////////////////////////////////////////////////////
void ColladaLoader::
load_geometry(domGeometry &geom, GeomNode *gnode, ColladaBindMaterial &bind_mat) {
  domMesh* mesh = geom.getMesh();
  if (mesh == NULL) {
    //TODO: support non-mesh geometry.
    return;
  }

  //TODO: support other than just triangles.
  domLines_Array &lines_array = mesh->getLines_array();
  for (size_t i = 0; i < lines_array.getCount(); ++i) {
    PT(ColladaPrimitive) prim = ColladaPrimitive::from_dom(*lines_array[i]);
    if (prim != NULL) {
      gnode->add_geom(prim->get_geom());
    }
  }

  domLinestrips_Array &linestrips_array = mesh->getLinestrips_array();
  for (size_t i = 0; i < linestrips_array.getCount(); ++i) {
    PT(ColladaPrimitive) prim = ColladaPrimitive::from_dom(*linestrips_array[i]);
    if (prim != NULL) {
      gnode->add_geom(prim->get_geom());
    }
  }

  domPolygons_Array &polygons_array = mesh->getPolygons_array();
  for (size_t i = 0; i < polygons_array.getCount(); ++i) {
    PT(ColladaPrimitive) prim = ColladaPrimitive::from_dom(*polygons_array[i]);
    if (prim != NULL) {
      gnode->add_geom(prim->get_geom());
    }
  }

  domPolylist_Array &polylist_array = mesh->getPolylist_array();
  for (size_t i = 0; i < polylist_array.getCount(); ++i) {
    PT(ColladaPrimitive) prim = ColladaPrimitive::from_dom(*polylist_array[i]);
    if (prim != NULL) {
      gnode->add_geom(prim->get_geom());
    }
  }

  domTriangles_Array &triangles_array = mesh->getTriangles_array();
  for (size_t i = 0; i < triangles_array.getCount(); ++i) {
    PT(ColladaPrimitive) prim = ColladaPrimitive::from_dom(*triangles_array[i]);
    if (prim != NULL) {
      gnode->add_geom(prim->get_geom());
    }
  }

  domTrifans_Array &trifans_array = mesh->getTrifans_array();
  for (size_t i = 0; i < trifans_array.getCount(); ++i) {
    PT(ColladaPrimitive) prim = ColladaPrimitive::from_dom(*trifans_array[i]);
    if (prim != NULL) {
      gnode->add_geom(prim->get_geom());
    }
  }

  domTristrips_Array &tristrips_array = mesh->getTristrips_array();
  for (size_t i = 0; i < tristrips_array.getCount(); ++i) {
    PT(ColladaPrimitive) prim = ColladaPrimitive::from_dom(*tristrips_array[i]);
    if (prim != NULL) {
      gnode->add_geom(prim->get_geom());
    }
  }
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

  // Check for an ambient light.
  domLight::domTechnique_common::domAmbientRef ambient = tc.getAmbient();
  if (ambient != NULL) {
    PT(AmbientLight) alight = new AmbientLight(TOSTRING(light.getName()));
    lnode = DCAST(LightNode, alight);

    domFloat3 &color = ambient->getColor()->getValue();
    alight->set_color(LColor(color[0], color[1], color[2], 1.0));
  }

  // Check for a directional light.
  domLight::domTechnique_common::domDirectionalRef directional = tc.getDirectional();
  if (directional != NULL) {
    PT(DirectionalLight) dlight = new DirectionalLight(TOSTRING(light.getName()));
    lnode = DCAST(LightNode, dlight);

    domFloat3 &color = directional->getColor()->getValue();
    dlight->set_color(LColor(color[0], color[1], color[2], 1.0));
    dlight->set_direction(LVector3f(0, 0, -1));
  }

  // Check for a point light.
  domLight::domTechnique_common::domPointRef point = tc.getPoint();
  if (point != NULL) {
    PT(PointLight) plight = new PointLight(TOSTRING(light.getName()));
    lnode = DCAST(LightNode, plight);

    domFloat3 &color = point->getColor()->getValue();
    plight->set_color(LColor(color[0], color[1], color[2], 1.0));

    LVecBase3f atten (1.0f, 0.0f, 0.0f);
    domTargetable_floatRef fval = point->getConstant_attenuation();
    if (fval != NULL) {
      atten[0] = fval->getValue();
    }
    fval = point->getLinear_attenuation();
    if (fval != NULL) {
      atten[1] = fval->getValue();
    }
    fval = point->getQuadratic_attenuation();
    if (fval != NULL) {
      atten[2] = fval->getValue();
    }

    plight->set_attenuation(atten);
  }

  // Check for a spot light.
  domLight::domTechnique_common::domSpotRef spot = tc.getSpot();
  if (spot != NULL) {
    PT(Spotlight) slight = new Spotlight(TOSTRING(light.getName()));
    lnode = DCAST(LightNode, slight);

    domFloat3 &color = spot->getColor()->getValue();
    slight->set_color(LColor(color[0], color[1], color[2], 1.0));

    LVecBase3f atten (1.0f, 0.0f, 0.0f);
    domTargetable_floatRef fval = spot->getConstant_attenuation();
    if (fval != NULL) {
      atten[0] = fval->getValue();
    }
    fval = spot->getLinear_attenuation();
    if (fval != NULL) {
      atten[1] = fval->getValue();
    }
    fval = spot->getQuadratic_attenuation();
    if (fval != NULL) {
      atten[2] = fval->getValue();
    }

    slight->set_attenuation(atten);

    fval = spot->getFalloff_angle();
    if (fval != NULL) {
      slight->get_lens()->set_fov(fval->getValue());
    } else {
      slight->get_lens()->set_fov(180.0f);
    }

    fval = spot->getFalloff_exponent();
    if (fval != NULL) {
      slight->set_exponent(fval->getValue());
    } else {
      slight->set_exponent(0.0f);
    }
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
