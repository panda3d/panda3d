// Filename: daeToEggConverter.cxx
// Created by:  pro-rsoft (08May08)
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

#include "daeToEggConverter.h"
#include "fcollada_utils.h"
#include "config_daeegg.h"
#include "daeCharacter.h"
#include "dcast.h"
#include "string_utils.h"
#include "eggData.h"
#include "eggPrimitive.h"
#include "eggLine.h"
#include "eggPolygon.h"
#include "eggTriangleFan.h"
#include "eggTriangleStrip.h"
#include "eggPoint.h"
#include "eggXfmSAnim.h"
#include "eggSAnimData.h"
#include "pt_EggVertex.h"

#include "FCDocument/FCDAsset.h"
#include "FCDocument/FCDocumentTools.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDSceneNodeTools.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometryInstance.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDGeometrySource.h"
#include "FCDocument/FCDSkinController.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDControllerInstance.h"
#include "FCDocument/FCDMorphController.h"
#include "FCDocument/FCDMaterialInstance.h"
#include "FCDocument/FCDExtra.h"
#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDEffectStandard.h"
#if FCOLLADA_VERSION >= 0x00030005
  #include "FCDocument/FCDGeometryPolygonsInput.h"
#endif

////////////////////////////////////////////////////////////////////
//     Function: DAEToEggConverter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DAEToEggConverter::
DAEToEggConverter() {
  _document = NULL;
  _table = NULL;
  _frame_rate = -1;
  _error_handler = NULL;
  _invert_transparency = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DAEToEggConverter::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DAEToEggConverter::
DAEToEggConverter(const DAEToEggConverter &copy) :
  SomethingToEggConverter(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DAEToEggConverter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DAEToEggConverter::
~DAEToEggConverter() {
  if (_error_handler != NULL) {
    delete _error_handler;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DAEToEggConverter::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the converter.
////////////////////////////////////////////////////////////////////
SomethingToEggConverter *DAEToEggConverter::
make_copy() {
  return new DAEToEggConverter(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: DAEToEggConverter::get_name
//       Access: Public, Virtual
//  Description: Returns the English name of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string DAEToEggConverter::
get_name() const {
  return "COLLADA";
}

////////////////////////////////////////////////////////////////////
//     Function: DAEToEggConverter::get_extension
//       Access: Public, Virtual
//  Description: Returns the common extension of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string DAEToEggConverter::
get_extension() const {
  return "dae";
}

////////////////////////////////////////////////////////////////////
//     Function: DAEToEggConverter::convert_file
//       Access: Public, Virtual
//  Description: Handles the reading of the input file and converting
//               it to egg.  Returns true if successful, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool DAEToEggConverter::
convert_file(const Filename &filename) {
  // Reset stuff
  clear_error();
  _joints.clear();
  _vertex_pools.clear();
  _skeletons.clear();
  _frame_rate = -1;
  if (_error_handler == NULL) {
    _error_handler = new FUErrorSimpleHandler;
  }
  
  // The default coordinate system is Y-up
  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(CS_yup_right);
  }
  
  // Read the file
  FCollada::Initialize();
  _document = FCollada::LoadDocument(filename.to_os_specific().c_str());
  if (_document == NULL) {
    daeegg_cat.error() << "Failed to load document: " << _error_handler->GetErrorString() << endl;
    FCollada::Release();
    return false;
  }
  // Make sure the file uses consistent coordinate system and length
  if (_document->GetAsset() != NULL) {
    FCDocumentTools::StandardizeUpAxisAndLength(_document);
  }
  
  _table = new EggTable();
  _table->set_table_type(EggTable::TT_table);
  // Process the stuff
  process_asset();
  preprocess();
  FCDSceneNode* visual_scene = _document->GetVisualSceneInstance();
  if (visual_scene != NULL) {
    // First check for an <extra> tag
    const FCDExtra* extra = visual_scene->GetExtra();
    //FIXME: eek this looks horrid
    if (extra != NULL) {
      const FCDEType* etype = extra->GetDefaultType();
      if (etype != NULL) {
        const FCDENode* enode = (const FCDENode*) etype->FindTechnique("MAX3D");
        if (enode != NULL) {
          enode = enode->FindChildNode("frame_rate");
          if (enode != NULL && !string_to_int(enode->GetContent(), _frame_rate)) {
            daeegg_cat.warning() << "Invalid integer in <frame_rate> tag: '" << enode->GetContent() << "'" << endl;
    } } } }
    // Now loop through the children
    for (size_t ch = 0; ch < visual_scene->GetChildrenCount(); ++ch) {
      process_node(DCAST(EggGroupNode, _egg_data), visual_scene->GetChild(ch));
    }
  }
  SAFE_DELETE(visual_scene);
  
  _egg_data->add_child(_table);
  
  // Clean up and return
  SAFE_DELETE(_document);
  FCollada::Release();
  return true;
}

void DAEToEggConverter::process_asset() {
  if (_document->GetAsset() == NULL) return;
  // Read out the coordinate system
  FMVector3 up_axis (_document->GetAsset()->GetUpAxis());
  if (up_axis == FMVector3(0, 1, 0)) {
    _egg_data->set_coordinate_system(CS_yup_right);
  } else if (up_axis == FMVector3(0, 0, 1)) {
    _egg_data->set_coordinate_system(CS_zup_right);
  } else {
    _egg_data->set_coordinate_system(CS_invalid);
    daeegg_cat.warning() << "Unrecognized coordinate system!\n";
  }
}

// This function lists all the joints and referenced skeletons
void DAEToEggConverter::preprocess(const FCDSceneNode* node) {
  // If the node is NULL, take the visual scene instance.
  if (node == NULL) {
    assert(_document != NULL);
    _skeletons.clear();
    _joints.clear();
    node = _document->GetVisualSceneInstance();
  }
  if (node == NULL) return;
  if (node->IsJoint()) {
    _joints[FROM_FSTRING(node->GetDaeId())] = NULL;
  }
  // Loop through the instances first.
  for (size_t in = 0; in < node->GetInstanceCount(); ++in) {
    if (node->GetInstance(in)->GetType() == FCDEntityInstance::CONTROLLER) {
      // Loop through the skeleton roots now.
#if FCOLLADA_VERSION < 0x00030005
      FCDSceneNodeList roots = ((FCDControllerInstance*) node->GetInstance(in))->FindSkeletonNodes();
#else
      FCDSceneNodeList roots;
      ((FCDControllerInstance*) node->GetInstance(in))->FindSkeletonNodes(roots);
#endif
      for (FCDSceneNodeList::iterator it = roots.begin(); it != roots.end(); ++it) {
        daeegg_cat.spam() << "Found referenced skeleton root " << FROM_FSTRING((*it)->GetDaeId()) << endl;
        _skeletons.push_back(FROM_FSTRING((*it)->GetDaeId()));
      }
    }
  }
  // Now loop through the children and recurse.
  for (size_t ch = 0; ch < node->GetChildrenCount(); ++ch) {
    preprocess(node->GetChild(ch));
  }
}

// Process the node. If forced is true, it will even process it if its known to be a skeleton root.
void DAEToEggConverter::process_node(PT(EggGroupNode) parent, const FCDSceneNode* node, bool forced) {
  nassertv(node != NULL);
  string node_id = FROM_FSTRING(node->GetDaeId());
  daeegg_cat.spam() << "Processing node with ID '" << node_id << "'" << endl;
  // Important! If it's known to be a skeleton root, ignore it for now, unless we're processing forced.
  if (!forced && count(_skeletons.begin(), _skeletons.end(), node_id) > 0) {
    daeegg_cat.spam() << "Ignoring skeleton root node with ID '" << node_id << "', we'll process it later" << endl;
    return;
  }
  // Create an egg group for this node
  PT(EggGroup) node_group = new EggGroup(FROM_FSTRING(node->GetName()));
  process_extra(node_group, node->GetExtra());
  parent->add_child(node_group);
  // Check if its a joint
  if (node->IsJoint()) {
    node_group->set_group_type(EggGroup::GT_joint);
    _joints[node_id] = node_group;
  }
  // Loop through the transforms and apply them
  for (size_t tr = 0; tr < node->GetTransformCount(); ++tr) {
    apply_transform(node_group, node->GetTransform(tr));
  }
  // Loop through the instances and process them
  for (size_t in = 0; in < node->GetInstanceCount(); ++in) {
    process_instance(node_group, node->GetInstance(in));
  }
  // Loop through the children and recursively process them
  for (size_t ch = 0; ch < node->GetChildrenCount(); ++ch) {
    process_node(DCAST(EggGroupNode, node_group), node->GetChild(ch));
  }
  // Loop through any possible scene node instances and process those, too.
  for (size_t in = 0; in < node->GetInstanceCount(); ++in) {
    if (node->GetInstance(in)->GetEntity() && node->GetInstance(in)->GetEntity()->GetType() == FCDEntity::SCENE_NODE) {
      process_node(DCAST(EggGroupNode, node_group), (const FCDSceneNode*) node->GetInstance(in)->GetEntity());
    }
  }
}

void DAEToEggConverter::process_instance(PT(EggGroup) parent, const FCDEntityInstance* instance) {
  nassertv(instance != NULL);
  nassertv(instance->GetEntity() != NULL);
  // Check what kind of instance this is
  switch (instance->GetType()) {
    case FCDEntityInstance::GEOMETRY: {
      const FCDGeometry* geometry = (const FCDGeometry*) instance->GetEntity();
      assert(geometry != NULL);
      if (geometry->IsMesh()) {
        // Now, handle the mesh.
        process_mesh(parent, geometry->GetMesh(), new DaeMaterials((const FCDGeometryInstance*) instance));
      }
      if (geometry->IsSpline()) {
        process_spline(parent, FROM_FSTRING(geometry->GetName()), const_cast<FCDGeometrySpline*> (geometry->GetSpline()));
      }
      break; }
    case FCDEntityInstance::CONTROLLER: {
      // Add the dart tag and process the controller instance
      parent->set_dart_type(EggGroup::DT_default);
      process_controller(parent, (const FCDControllerInstance*) instance);
      break; }
    case FCDEntityInstance::MATERIAL:
      // We don't process this directly, handled per-geometry instead.
      break;
    case FCDEntityInstance::SIMPLE: {
      // Grab the entity and check it's type.
      const FCDEntity* entity = instance->GetEntity();
      if (entity->GetType() != FCDEntity::SCENE_NODE) {
        daeegg_cat.warning() << "Unsupported entity type found" << endl;
      }
      break; }
    default:
      daeegg_cat.warning() << "Unsupported instance type found" << endl;
  }
}

// Processes the given mesh.
void DAEToEggConverter::process_mesh(PT(EggGroup) parent, const FCDGeometryMesh* mesh, PT(DaeMaterials) materials) {
  nassertv(mesh != NULL);
  daeegg_cat.debug() << "Processing mesh with id " << FROM_FSTRING(mesh->GetDaeId()) << endl;
  
  // Create the egg stuff to hold this mesh
  PT(EggGroup) mesh_group = new EggGroup(FROM_FSTRING(mesh->GetDaeId()));
  parent->add_child(mesh_group);
  PT(EggVertexPool) mesh_pool = new EggVertexPool(FROM_FSTRING(mesh->GetDaeId()));
  mesh_group->add_child(mesh_pool);
  _vertex_pools[FROM_FSTRING(mesh->GetDaeId())] = mesh_pool;
  
  // First retrieve the vertex source
  if (mesh->GetSourceCount() == 0) {
    daeegg_cat.debug() << "Mesh with id " << FROM_FSTRING(mesh->GetDaeId()) << " has no sources" << endl;
    return;
  }
  const FCDGeometrySource* vsource = mesh->FindSourceByType(FUDaeGeometryInput::POSITION);  
  if (vsource == NULL) {
    daeegg_cat.debug() << "Mesh with id " << FROM_FSTRING(mesh->GetDaeId()) << " has no source for POSITION data" << endl;
    return;
  }
  
  // Loop through the polygon groups and add them
  daeegg_cat.spam() << "Mesh with id " << FROM_FSTRING(mesh->GetDaeId()) << " has " << mesh->GetPolygonsCount() << " polygon groups" << endl;
  if (mesh->GetPolygonsCount() == 0) return;
  
  // This is an array of pointers, I know. But since they are refcounted, I don't have a better idea.
  PT(EggGroup) *primitive_holders = new PT(EggGroup) [mesh->GetPolygonsCount()];
  for (size_t gr = 0; gr < mesh->GetPolygonsCount(); ++gr) {
    const FCDGeometryPolygons* polygons = mesh->GetPolygons(gr);
    // Stores which group holds the primitives.
    PT(EggGroup) primitiveholder;
    // If we have materials, make a group for each material. Then, apply the material's per-group stuff.
    if (materials != NULL && (!polygons->GetMaterialSemantic().empty()) && mesh->GetPolygonsCount() > 1) {
      primitiveholder = new EggGroup(FROM_FSTRING(mesh->GetDaeId()) + "." + FROM_FSTRING(polygons->GetMaterialSemantic()));
      mesh_group->add_child(primitiveholder);
    } else {
      primitiveholder = mesh_group;
    }
    primitive_holders[gr] = primitiveholder;
    // Apply the per-group data of the materials, if we have it.
    if (materials != NULL) {
      materials->apply_to_group(FROM_FSTRING(polygons->GetMaterialSemantic()), primitiveholder, _invert_transparency);
    }
    // Find the position sources
    const FCDGeometryPolygonsInput* pinput = polygons->FindInput(FUDaeGeometryInput::POSITION);
    assert(pinput != NULL);
    const uint32* indices = pinput->GetIndices();
    // Find the normal sources
    const FCDGeometrySource* nsource = mesh->FindSourceByType(FUDaeGeometryInput::NORMAL);
    const FCDGeometryPolygonsInput* ninput = polygons->FindInput(FUDaeGeometryInput::NORMAL);
    const uint32* nindices;
    if (ninput != NULL) nindices = ninput->GetIndices();
    // Find texcoord sources
    const FCDGeometrySource* tcsource = mesh->FindSourceByType(FUDaeGeometryInput::TEXCOORD);
    const FCDGeometryPolygonsInput* tcinput = polygons->FindInput(FUDaeGeometryInput::TEXCOORD);
    const uint32* tcindices;
    if (tcinput != NULL) tcindices = tcinput->GetIndices();
    // Find vcolor sources
    const FCDGeometrySource* csource = mesh->FindSourceByType(FUDaeGeometryInput::COLOR);
    const FCDGeometryPolygonsInput* cinput = polygons->FindInput(FUDaeGeometryInput::COLOR);
    const uint32* cindices;
    if (cinput != NULL) cindices = cinput->GetIndices();
    // Find binormal sources
    const FCDGeometrySource* bsource = mesh->FindSourceByType(FUDaeGeometryInput::TEXBINORMAL);
    const FCDGeometryPolygonsInput* binput = polygons->FindInput(FUDaeGeometryInput::TEXBINORMAL);
    const uint32* bindices;
    if (binput != NULL) bindices = binput->GetIndices();
    // Find tangent sources
    const FCDGeometrySource* tsource = mesh->FindSourceByType(FUDaeGeometryInput::TEXTANGENT);
    const FCDGeometryPolygonsInput* tinput = polygons->FindInput(FUDaeGeometryInput::TEXTANGENT);
    const uint32* tindices;
    if (tinput != NULL) tindices = tinput->GetIndices();
    // Get a name for potential coordinate sets
    string tcsetname ("");
    if (materials != NULL && tcinput != NULL) {
      daeegg_cat.debug() << "Assigning texcoord set " << tcinput->GetSet() << " to semantic '" << FROM_FSTRING(polygons->GetMaterialSemantic()) << "'\n";
      tcsetname = materials->get_uvset_name(FROM_FSTRING(polygons->GetMaterialSemantic()), FUDaeGeometryInput::TEXCOORD, tcinput->GetSet());
    }
    string tbsetname ("");
    if (materials != NULL && binput != NULL) {
      daeegg_cat.debug() << "Assigning texbinormal set " << binput->GetSet() << " to semantic '" << FROM_FSTRING(polygons->GetMaterialSemantic()) << "'\n";
      tbsetname = materials->get_uvset_name(FROM_FSTRING(polygons->GetMaterialSemantic()), FUDaeGeometryInput::TEXBINORMAL, binput->GetSet());
    }
    string ttsetname ("");
    if (materials != NULL && tinput != NULL) {
      daeegg_cat.debug() << "Assigning textangent set " << tinput->GetSet() << " to semantic '" << FROM_FSTRING(polygons->GetMaterialSemantic()) << "'\n";
      ttsetname = materials->get_uvset_name(FROM_FSTRING(polygons->GetMaterialSemantic()), FUDaeGeometryInput::TEXTANGENT, tinput->GetSet());
    }
    // Loop through the indices and add the vertices.
    for (size_t ix = 0; ix < pinput->GetIndexCount(); ++ix) {
      PT_EggVertex vertex = mesh_pool->make_new_vertex();
      const float* data = &vsource->GetData()[indices[ix]*3];
      vertex->set_pos(LPoint3d(data[0], data[1], data[2]));
      // Process the normal
      if (nsource != NULL && ninput != NULL) {
        assert(nsource->GetStride() == 3);
        data = &nsource->GetData()[nindices[ix]*3];
        vertex->set_normal(LVecBase3d(data[0], data[1], data[2]));
      }
      // Process the texcoords
      if (tcsource != NULL && tcinput != NULL) {
        assert(tcsource->GetStride() == 2 || tcsource->GetStride() == 3);
        data = &tcsource->GetData()[tcindices[ix]*tcsource->GetStride()];
        if (tcsource->GetStride() == 2) {
          vertex->set_uv(tcsetname, LPoint2d(data[0], data[1]));
        } else {
          vertex->set_uvw(tcsetname, LPoint3d(data[0], data[1], data[2]));
        }
      }
      // Process the color
      if (csource != NULL && cinput != NULL) {
        assert(csource->GetStride() == 3 || csource->GetStride() == 4);
        if (csource->GetStride() == 3) {
          data = &csource->GetData()[cindices[ix]*3];
          vertex->set_color(LColor(data[0], data[1], data[2], 1.0f));
        } else {
          data = &csource->GetData()[cindices[ix]*4];
          vertex->set_color(LColor(data[0], data[1], data[2], data[3]));
        }
      }
      // Possibly add a UV object
      if ((bsource != NULL && binput != NULL) || (tsource != NULL && tinput != NULL)) {
        if (bsource != NULL && binput != NULL) {
          assert(bsource->GetStride() == 3);
          data = &bsource->GetData()[bindices[ix]*3];
          PT(EggVertexUV) uv_obj = vertex->modify_uv_obj(tbsetname);
          if (uv_obj == NULL) {
            uv_obj = new EggVertexUV(tbsetname, LTexCoordd());
          }
          uv_obj->set_binormal(LVecBase3d(data[0], data[1], data[2]));
        }
        if (tsource != NULL && tinput != NULL) {
          assert(tsource->GetStride() == 3);
          data = &tsource->GetData()[tindices[ix]*3];
          PT(EggVertexUV) uv_obj = vertex->modify_uv_obj(ttsetname);
          if (uv_obj == NULL) {
            uv_obj = new EggVertexUV(ttsetname, LTexCoordd());
          }
          uv_obj->set_tangent(LVecBase3d(data[0], data[1], data[2]));
        }
      }
      vertex->transform(parent->get_node_to_vertex());
    }
  }
  // Loop again for the polygons
  for (size_t gr = 0; gr < mesh->GetPolygonsCount(); ++gr) {
    const FCDGeometryPolygons* polygons = mesh->GetPolygons(gr);
    // Now loop through the faces
    uint32 offset = 0;
    for (size_t fa = 0; fa < polygons->GetFaceVertexCountCount(); ++fa) {
      PT(EggPrimitive) primitive = NULL;
      // Create a primitive that matches the fcollada type
      switch (polygons->GetPrimitiveType()) {
        case FCDGeometryPolygons::LINES:
          primitive = new EggLine();
          break;
        case FCDGeometryPolygons::POLYGONS:
          primitive = new EggPolygon();
          break;
        case FCDGeometryPolygons::TRIANGLE_FANS:
          primitive = new EggTriangleFan();
          break;
        case FCDGeometryPolygons::TRIANGLE_STRIPS:
          primitive = new EggTriangleStrip();
          break;
        case FCDGeometryPolygons::POINTS:
          primitive = new EggPoint();
          break;
        case FCDGeometryPolygons::LINE_STRIPS:
          daeegg_cat.warning() << "Linestrips not yet supported!" << endl;
          break;
        default:
          daeegg_cat.warning() << "Unsupported primitive type found!" << endl;
      }
      if (primitive != NULL) {
        primitive_holders[gr]->add_child(primitive);
        if (materials != NULL) {
          materials->apply_to_primitive(FROM_FSTRING(polygons->GetMaterialSemantic()), primitive);
        }
        for (size_t ve = 0; ve < polygons->GetFaceVertexCount(fa); ++ve) {
          assert(mesh_pool->has_vertex(ve + polygons->GetFaceVertexOffset() + offset));
          primitive->add_vertex(mesh_pool->get_vertex(ve + polygons->GetFaceVertexOffset() + offset));
        }
      }
      offset += polygons->GetFaceVertexCount(fa);
    }
  }
  delete[] primitive_holders;
}

void DAEToEggConverter::process_spline(PT(EggGroup) parent, const string group_name, FCDGeometrySpline* geometry_spline) {
  assert(geometry_spline != NULL);
  PT(EggGroup) result = new EggGroup(group_name);
  parent->add_child(result);
  //TODO: if its not a nurbs, make it convert between the types
  if (geometry_spline->GetType() != FUDaeSplineType::NURBS) {
    daeegg_cat.warning() << "Only NURBS curves are supported (yet)!" << endl;
  } else {
    // Loop through the splines
    for (size_t sp = 0; sp < geometry_spline->GetSplineCount(); ++sp) {
      process_spline(result, geometry_spline->GetSpline(sp));
    }
  }
}

void DAEToEggConverter::process_spline(PT(EggGroup) parent, const FCDSpline* spline) {
  assert(spline != NULL);
  nassertv(spline->GetSplineType() == FUDaeSplineType::NURBS);
  // Now load in the nurbs curve to the egg library
  PT(EggNurbsCurve) nurbs_curve = new EggNurbsCurve(FROM_FSTRING(spline->GetName()));
  parent->add_child(nurbs_curve);
  //TODO: what value is this?
  nurbs_curve->setup(0, ((const FCDNURBSSpline*) spline)->GetKnotCount());
  for (size_t kn = 0; kn < ((const FCDNURBSSpline*) spline)->GetKnotCount(); ++kn) {
    const float* knot = ((const FCDNURBSSpline*) spline)->GetKnot(kn);
    assert(knot != NULL);
    nurbs_curve->set_knot(kn, *knot);
  }
  for (size_t cv = 0; cv < spline->GetCVCount(); ++cv) {
    PT_EggVertex c_vtx = new EggVertex();
    c_vtx->set_pos(TO_VEC3(*spline->GetCV(cv)));
    c_vtx->transform(parent->get_node_to_vertex());
    nurbs_curve->add_vertex(c_vtx);
  }
}

void DAEToEggConverter::process_controller(PT(EggGroup) parent, const FCDControllerInstance* instance) {
  assert(instance != NULL);
  const FCDController* controller = (const FCDController*) instance->GetEntity();
  assert(controller != NULL);
  PT(EggVertexPool) vertex_pool = NULL;
  // Add the skin geometry
  const FCDGeometry* geometry = controller->GetBaseGeometry();
  if (geometry != NULL) {
    if (geometry->IsMesh()) {
      process_mesh(parent, geometry->GetMesh(), new DaeMaterials((const FCDGeometryInstance*) instance));
      daeegg_cat.spam() << "Processing mesh for controller\n";
      if (_vertex_pools.count(FROM_FSTRING(geometry->GetMesh()->GetDaeId()))) {
        daeegg_cat.debug() << "Using vertex pool " << FROM_FSTRING(geometry->GetMesh()->GetDaeId()) << "\n";
        vertex_pool = _vertex_pools[FROM_FSTRING(geometry->GetMesh()->GetDaeId())];
      }
    }
    if (geometry->IsSpline()) {
      process_spline(parent, FROM_FSTRING(geometry->GetName()), const_cast<FCDGeometrySpline*> (geometry->GetSpline()));
    }
  }
  // Add the joint hierarchy
#if FCOLLADA_VERSION < 0x00030005
  FCDSceneNodeList roots = (const_cast<FCDControllerInstance*> (instance))->FindSkeletonNodes();
#else
  FCDSceneNodeList roots;
  (const_cast<FCDControllerInstance*> (instance))->FindSkeletonNodes(roots);
#endif
  for (FCDSceneNodeList::iterator it = roots.begin(); it != roots.end(); ++it) {
    process_node(DCAST(EggGroupNode, parent), *it, true);
  }
  if (controller->IsSkin()) {
    // Load in the vertex influences first
    pmap<int32, pvector<pair<PT_EggVertex, PN_stdfloat> > > influences;
    if (vertex_pool) {
      for (size_t in = 0; in < controller->GetSkinController()->GetInfluenceCount(); ++in) {
        assert(vertex_pool->has_vertex(in));
        for (size_t pa = 0; pa < controller->GetSkinController()->GetVertexInfluence(in)->GetPairCount(); ++pa) {
          const FCDJointWeightPair* jwpair = controller->GetSkinController()->GetVertexInfluence(in)->GetPair(pa);
          influences[jwpair->jointIndex].push_back(pair<PT_EggVertex, PN_stdfloat> (vertex_pool->get_vertex(in), jwpair->weight));
        }
      }
    }
    // Loop through the joints in the vertex influences
    for (pmap<int32, pvector<pair<PT_EggVertex, PN_stdfloat> > >::iterator it = influences.begin(); it != influences.end(); ++it) {
      if (it->first == -1) {
        daeegg_cat.warning() << "Ignoring vertex influence with negative joint index\n";
        //FIXME: Why are there joints with index -1
      } else {
        const string joint_id = FROM_FSTRING(controller->GetSkinController()->GetJoint(it->first)->GetId());
        //TODO: what if the joints have just not been defined yet?
        if (_joints.count(joint_id) > 0) {
          if (_joints[joint_id]) {
            for (pvector<pair<PT_EggVertex, PN_stdfloat> >::iterator vi = it->second.begin(); vi != it->second.end(); ++vi) {
              _joints[joint_id]->ref_vertex(vi->first, vi->second);
            }
          } else {
            daeegg_cat.warning() << "Unprocessed joint being referenced: '" << joint_id << "'" << endl;
          }
        } else {
          daeegg_cat.warning() << "Unknown joint being referenced: '" << joint_id << "'" << endl;
        }
      }
    }
  }
  if (controller->IsMorph()) {
    assert(controller != NULL);
    const FCDMorphController* morph_controller = controller->GetMorphController();
    assert(morph_controller != NULL);
    PT(EggTable) bundle = new EggTable(parent->get_name());
    bundle->set_table_type(EggTable::TT_bundle);
    PT(EggTable) morph = new EggTable("morph");
    morph->set_table_type(EggTable::TT_table);
    bundle->add_child(morph);
    // Loop through the morph targets.
    for (size_t mt = 0; mt < morph_controller->GetTargetCount(); ++mt) {
      const FCDMorphTarget* morph_target = morph_controller->GetTarget(mt);
      assert(morph_target != NULL);
      PT(EggSAnimData) target = new EggSAnimData(FROM_FSTRING(morph_target->GetGeometry()->GetName()));
      if (morph_target->IsAnimated()) {
        //TODO
      } else {
        target->add_data(morph_target->GetWeight());
      }
      morph->add_child(target);
    }
  }
  
  // Get a <Bundle> for the character and add it to the table
  PT(DaeCharacter) character = new DaeCharacter(parent->get_name(), instance);
  _table->add_child(character->as_egg_bundle());
}

void DAEToEggConverter::process_extra(PT(EggGroup) group, const FCDExtra* extra) {
  if (extra == NULL) {
    return;
  }
  nassertv(group != NULL);
  
  const FCDEType* etype = extra->GetDefaultType();
  if (etype == NULL) {
    return;
  }
  
  const FCDENode* enode = (const FCDENode*) etype->FindTechnique("PANDA3D");
  if (enode == NULL) {
    return;
  }
  
  FCDENodeList tags;
  enode->FindChildrenNodes("param", tags);
  for (FCDENodeList::iterator it = tags.begin(); it != tags.end(); ++it) {
    const FCDEAttribute* attr = (*it)->FindAttribute("sid");
    if (attr) {
      group->set_tag(FROM_FSTRING(attr->GetValue()), (*it)->GetContent());
    }
  }
}

LMatrix4d DAEToEggConverter::convert_matrix(const FMMatrix44& matrix) {
  LMatrix4d result = LMatrix4d::zeros_mat();
  for (char x = 0; x < 4; ++x) {
    for (char y = 0; y < 4; ++y) {
      result(x, y) = matrix[x][y];
    }
  }
  return result;
}

void DAEToEggConverter::apply_transform(const PT(EggGroup) to, const FCDTransform* from) {
  assert(from != NULL);
  assert(to != NULL);
  to->set_transform3d(convert_matrix(from->ToMatrix()) * to->get_transform3d());
}
