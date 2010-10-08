// Filename: speedTreeNode.cxx
// Created by:  drose (13Mar09)
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

#include "pandabase.h"
#include "speedTreeNode.h"
#include "virtualFileSystem.h"
#include "config_util.h"
#include "cullTraverser.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "omniBoundingVolume.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "clockObject.h"
#include "geomDrawCallbackData.h"
#include "graphicsStateGuardian.h"
#include "textureAttrib.h"
#include "lightAttrib.h"
#include "directionalLight.h"
#include "loader.h"
#include "deg_2_rad.h"

#ifdef SPEEDTREE_OPENGL
#include "glew/glew.h"
#endif  // SPEEDTREE_OPENGL

bool SpeedTreeNode::_authorized;
bool SpeedTreeNode::_done_first_init;
TypeHandle SpeedTreeNode::_type_handle;
TypeHandle SpeedTreeNode::DrawCallback::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
SpeedTreeNode::
SpeedTreeNode(const string &name) :
  PandaNode(name)
#ifdef ST_DELETE_FOREST_HACK
  // Early versions of SpeedTree don't destruct unused CForestRender
  // objects correctly.  To avoid crashes, we have to leak these
  // things.
  , _forest(*(new SpeedTree::CForestRender))
#endif
{
  init_node();
  // For now, set an infinite bounding volume.  Maybe in the future
  // we'll change this to match whatever set of trees we're holding,
  // though it probably doesn't really matter too much.
  //set_internal_bounds(new OmniBoundingVolume);
  //  set_internal_bounds(new BoundingSphere(LPoint3f::zero(), 10.0f));

  // Intialize the render params.
  SpeedTree::SForestRenderInfo render_info;

  // First, get the shader directory.
  if (!speedtree_shaders_dir.get_value().is_directory()) {
    speedtree_cat.warning()
      << "speedtree-shaders-dir is set to " << speedtree_shaders_dir
      << ", which doesn't exist.\n";
  }

  string shaders_dir = speedtree_shaders_dir.get_value().to_os_specific();
  // Ensure the path ends with a terminal slash; SpeedTree requires this.
#ifdef WIN32
  if (!shaders_dir.empty() && shaders_dir[shaders_dir.length() - 1] != '\\') {
    shaders_dir += "\\";
  }
#else
  if (!shaders_dir.empty() && shaders_dir[shaders_dir.length() - 1] != '/') {
    shaders_dir += "/";
  }
#endif

  render_info.m_strShaderPath = shaders_dir.c_str();
  render_info.m_nMaxBillboardImagesByBase = speedtree_max_billboard_images_by_base;
  render_info.m_nNumShadowMaps = 1;
  render_info.m_nShadowMapResolution = 0;
  _forest.SetRenderInfo(render_info);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::count_total_instances
//       Access: Published
//  Description: Returns the total number of trees that will be
//               rendered by this node, counting all instances of all
//               trees.
////////////////////////////////////////////////////////////////////
int SpeedTreeNode::
count_total_instances() const {
  int total_instances = 0;
  Trees::const_iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    total_instances += instance_list->get_num_instances();
  }

  return total_instances;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::add_tree
//       Access: Published
//  Description: Adds a new tree for rendering.  Returns the
//               InstanceList which can be used to add to the
//               instances for this tree.  If the tree has previously
//               been added, returns the existing InstanceList.
////////////////////////////////////////////////////////////////////
SpeedTreeNode::InstanceList &SpeedTreeNode::
add_tree(const STTree *tree) {
  nassertr(is_valid(), *(InstanceList *)NULL);
  nassertr(tree->is_valid(), *(InstanceList *)NULL);

  InstanceList ilist(tree);
  Trees::iterator ti = _trees.find(&ilist);
  if (ti == _trees.end()) {
    // This is the first time that this particular tree has been
    // added.
    InstanceList *instance_list = new InstanceList(tree);
    pair<Trees::iterator, bool> result = _trees.insert(instance_list);
    ti = result.first;
    bool inserted = result.second;
    nassertr(inserted, *(*ti));

    if (!_forest.RegisterTree((SpeedTree::CTree *)tree->get_tree())) {
      speedtree_cat.warning()
	<< "Failed to register tree " << tree->get_filename() << "\n";
      speedtree_cat.warning()
	<< SpeedTree::CCore::GetError() << "\n";
    }
  }

  _needs_repopulate = true;
  mark_internal_bounds_stale();
  InstanceList *instance_list = (*ti);
  return *instance_list;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::remove_tree
//       Access: Published
//  Description: Removes all instances of the indicated tree.  Returns
//               the number of instances removed.
////////////////////////////////////////////////////////////////////
int SpeedTreeNode::
remove_tree(const STTree *tree) {
  InstanceList ilist(tree);
  Trees::iterator ti = _trees.find(&ilist);
  if (ti == _trees.end()) {
    // The tree was not already present.
    return 0;
  }

  if (!_forest.UnregisterTree(tree->get_tree())) {
    speedtree_cat.warning()
      << "Failed to unregister tree " << tree->get_filename() << "\n";
    speedtree_cat.warning()
      << SpeedTree::CCore::GetError() << "\n";
  }

  _needs_repopulate = true;
  mark_internal_bounds_stale();

  InstanceList *instance_list = (*ti);
  int num_removed = instance_list->get_num_instances();
  _trees.erase(ti);
  delete instance_list;

  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::remove_all_trees
//       Access: Published
//  Description: Removes all instances of all trees from the node.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
remove_all_trees() {
  Trees::iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    const STTree *tree = instance_list->get_tree();
    if (!_forest.UnregisterTree(tree->get_tree())) {
      speedtree_cat.warning()
	<< "Failed to unregister tree " << tree->get_filename() << "\n";
      speedtree_cat.warning()
	<< SpeedTree::CCore::GetError() << "\n";
    }
    delete instance_list;
  }

  _trees.clear();
  _needs_repopulate = true;
  mark_internal_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::has_instance_list
//       Access: Published
//  Description: Returns true if the indicated tree has any instances
//               within this node, false otherwise.
////////////////////////////////////////////////////////////////////
bool SpeedTreeNode::
has_instance_list(const STTree *tree) const {
  InstanceList ilist(tree);
  Trees::const_iterator ti = _trees.find(&ilist);
  return (ti != _trees.end());
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::get_instance_list
//       Access: Published
//  Description: Returns a list of transforms that corresponds to the
//               instances at which the indicated tree appears.  You
//               should ensure that has_instance_list() returns true
//               before calling this method.
////////////////////////////////////////////////////////////////////
const SpeedTreeNode::InstanceList &SpeedTreeNode::
get_instance_list(const STTree *tree) const {
  InstanceList ilist(tree);
  Trees::const_iterator ti = _trees.find(&ilist);
  if (ti == _trees.end()) {
    // The tree was not already present.
    static InstanceList empty_list((STTree *)NULL);
    return empty_list;
  }

  InstanceList *instance_list = (*ti);
  return *instance_list;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::modify_instance_list
//       Access: Published
//  Description: Returns a modifiable list of transforms that
//               corresponds to the instances of this tree.  This is
//               equivalent to add_tree().
////////////////////////////////////////////////////////////////////
SpeedTreeNode::InstanceList &SpeedTreeNode::
modify_instance_list(const STTree *tree) {
  return add_tree(tree);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::add_instance
//       Access: Published
//  Description: Adds a new instance of the indicated tree at the
//               indicated transform.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
add_instance(const STTree *tree, const STTransform &transform) {
  add_tree(tree).add_instance(transform);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::add_instances
//       Access: Published
//  Description: Walks the scene graph beginning at root, looking for
//               nested SpeedTreeNodes.  For each SpeedTreeNode found,
//               adds all of the instances defined within that
//               SpeedTreeNode as instances of this node, after
//               applying the indicated scene-graph transform.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
add_instances(const NodePath &root, const TransformState *transform) {
  nassertv(!root.is_empty());
  r_add_instances(root.node(), transform->compose(root.get_transform()),
		  Thread::get_current_thread());
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::add_from_stf
//       Access: Published
//  Description: Opens and reads the named STF (SpeedTree Forest)
//               file, and adds the SRT files named within as
//               instances of this node.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool SpeedTreeNode::
add_from_stf(const Filename &pathname, const LoaderOptions &options) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename filename = Filename::text_filename(pathname);
  PT(VirtualFile) file = vfs->get_file(filename);
  if (file == (VirtualFile *)NULL) {
    // No such file.
    speedtree_cat.error()
      << "Could not find " << pathname << "\n";
    return false;
  }

  if (speedtree_cat.is_debug()) {
    speedtree_cat.debug()
      << "Reading STF file " << filename << "\n";
  }

  istream *in = file->open_read_file(true);
  bool success = add_from_stf(*in, pathname, options);
  vfs->close_read_file(in);

  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::add_from_stf
//       Access: Published
//  Description: Reads text data from the indicated stream, which is
//               understood to represent the named STF (SpeedTree
//               Forest) file, and adds the SRT files named within as
//               instances of this node.  Returns true on success,
//               false on failure.
//
//               The pathname is used for reference only; if nonempty,
//               it provides a search directory for named SRT files.
//
//               The Loader and LoaderOptions, if provided, are used
//               to load the SRT files.  If the Loader pointer is
//               NULL, the default global Loader is used instead.
////////////////////////////////////////////////////////////////////
bool SpeedTreeNode::
add_from_stf(istream &in, const Filename &pathname, 
	     const LoaderOptions &options, Loader *loader) {
  if (loader == NULL) {
    loader = Loader::get_global_ptr();
  }
  string os_filename;

  Filename dirname = pathname.get_dirname();
  dirname.make_absolute();
  DSearchPath search;
  search.append_directory(dirname);

  typedef pmap<Filename, CPT(STTree) > AlreadyLoaded;
  AlreadyLoaded already_loaded;

  // The STF file format doesn't allow for spaces in the SRT filename.
  in >> os_filename;
  while (in && !in.eof()) {
    CPT(STTree) tree;
    Filename srt_filename = Filename::from_os_specific(os_filename);
    AlreadyLoaded::iterator ai = already_loaded.find(srt_filename);
    if (ai != already_loaded.end()) {
      tree = (*ai).second;
    } else {
      // Resolve the SRT filename relative to the STF file first.
      srt_filename.resolve_filename(search);

      // Now load up the SRT file using the Panda loader (which will
      // also search the model-path if necessary).
      PT(PandaNode) srt_root = loader->load_sync(srt_filename);

      if (srt_root != NULL) {
	NodePath srt(srt_root);
	NodePath srt_np = srt.find("**/+SpeedTreeNode");
	if (!srt_np.is_empty()) {
	  SpeedTreeNode *srt_node = DCAST(SpeedTreeNode, srt_np.node());
	  if (srt_node->get_num_trees() >= 1) {
	    tree = srt_node->get_tree(0);
	  }
	}
      }
      already_loaded[srt_filename] = tree;
    }

    // Now we've loaded the SRT data, so apply it the appropriate
    // number of times to the locations specified.
    int num_instances;
    in >> num_instances;
    for (int ni = 0; ni < num_instances && in && !in.eof(); ++ni) {
      LPoint3f pos;
      float rotate, scale;
      in >> pos[0] >> pos[1] >> pos[2] >> rotate >> scale;

      if (!speedtree_5_2_stf) {
	// 5.1 or earlier stf files also included these additional
	// values, which we will ignore:
	float elev_min, elev_max, slope_min, slope_max;
	in >> elev_min >> elev_max >> slope_min >> slope_max;
      }

      if (tree != NULL) {
	add_instance(tree, STTransform(pos, rad_2_deg(rotate), scale));
      }
    }
    in >> os_filename;
  }

  // Consume any whitespace at the end of the file.
  in >> ws;

  if (!in.eof()) {
    // If we didn't read all the way to end-of-file, there was an
    // error.
    in.clear();
    string text;
    in >> text;
    speedtree_cat.error()
      << "Unexpected text in " << pathname << " at \"" << text << "\"\n";
    return false;
  }

  // Return true if we successfully read all the way to end-of-file.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::authorize
//       Access: Published, Static
//  Description: Make this call to initialized the SpeedTree API and
//               verify the license.  If an empty string is passed for
//               the license, the config variable speedtree-license is
//               consulted.  Returns true on success, false on
//               failure.  If this call is not made explicitly, it
//               will be made implicitly the first time a
//               SpeedTreeNode is created.
////////////////////////////////////////////////////////////////////
bool SpeedTreeNode::
authorize(const string &license) {
  if (!_authorized) {
    if (!license.empty()) {
      SpeedTree::CCore::Authorize(license.c_str());
    } else {
      if (!speedtree_license.empty()) {
	SpeedTree::CCore::Authorize(speedtree_license.c_str());
      }
    }
							     
    _authorized = SpeedTree::CCore::IsAuthorized();

    SpeedTree::CCore::SetTextureFlip(true);
  }
  
  return _authorized;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
SpeedTreeNode::
SpeedTreeNode(const SpeedTreeNode &copy) :
  PandaNode(copy)
#ifdef ST_DELETE_FOREST_HACK
  // Early versions of SpeedTree don't destruct unused CForestRender
  // objects correctly.  To avoid crashes, we have to leak these
  // things.
  , _forest(*(new SpeedTree::CForestRender))
#endif
{
  init_node();

  _forest.SetRenderInfo(copy._forest.GetRenderInfo());

  Trees::const_iterator ti;
  for (ti = copy._trees.begin(); ti != copy._trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    const STTree *tree = instance_list->get_tree();
    if (!_forest.RegisterTree((SpeedTree::CTree *)tree->get_tree())) {
      speedtree_cat.warning()
	<< "Failed to register tree " << tree->get_filename() << "\n";
      speedtree_cat.warning()
	<< SpeedTree::CCore::GetError() << "\n";
    }

    _trees.push_back(new InstanceList(*instance_list));
  }

  _needs_repopulate = true;
  mark_internal_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
SpeedTreeNode::
~SpeedTreeNode() {
  // Help reduce memory waste from ST_DELETE_FOREST_HACK.
  _forest.ClearInstances();
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *SpeedTreeNode::
make_copy() const {
  return new SpeedTreeNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes of compatible type, adding children or
//               whatever.  For instance, an LODNode should not be
//               combined with any other PandaNode, because its set of
//               children is meaningful.
////////////////////////////////////////////////////////////////////
bool SpeedTreeNode::
safe_to_combine() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::cull_callback
//       Access: Public, Virtual
//  Description: This function will be called during the cull
//               traversal to perform any additional operations that
//               should be performed at cull time.  This may include
//               additional manipulation of render state or additional
//               visible/invisible decisions, or any other arbitrary
//               operation.
//
//               Note that this function will *not* be called unless
//               set_cull_callback() is called in the constructor of
//               the derived class.  It is necessary to call
//               set_cull_callback() to indicated that we require
//               cull_callback() to be called.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool SpeedTreeNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  if (!_is_valid) {
    return false;
  }

  GraphicsStateGuardian *gsg = DCAST(GraphicsStateGuardian, trav->get_gsg());
  nassertr(gsg != (GraphicsStateGuardian *)NULL, true);
  if (!validate_api(gsg)) {
    return false;
  }

  ClockObject *clock = ClockObject::get_global_clock();
  _forest.SetGlobalTime(clock->get_frame_time());
  _forest.AdvanceGlobalWind();
  
  // Compute the modelview and camera transforms, to pass to the
  // SpeedTree CView structure.
  CPT(TransformState) orig_modelview = data.get_modelview_transform(trav);
  CPT(TransformState) modelview = gsg->get_cs_transform()->compose(orig_modelview);
  CPT(TransformState) camera_transform = modelview->invert_compose(TransformState::make_identity());
  const LMatrix4f &modelview_mat = modelview->get_mat();
  const LPoint3f &camera_pos = camera_transform->get_pos();
  const Lens *lens = trav->get_scene()->get_lens();
  
  LMatrix4f projection_mat =
    LMatrix4f::convert_mat(gsg->get_internal_coordinate_system(), lens->get_coordinate_system()) *
    lens->get_projection_mat();
  
  _view.Set(SpeedTree::Vec3(camera_pos[0], camera_pos[1], camera_pos[2]),
	    SpeedTree::Mat4x4(projection_mat.get_data()),
	    SpeedTree::Mat4x4(modelview_mat.get_data()),
	    lens->get_near(), lens->get_far());

  // Convert the render state to SpeedTree's input.
  const RenderState *state = data._state;

  // Check texture state.  If all textures are disabled, then we ask
  // SpeedTree to disable textures.
  bool show_textures = true;
  const TextureAttrib *ta = DCAST(TextureAttrib, state->get_attrib(TextureAttrib::get_class_slot()));
  if (ta != (TextureAttrib *)NULL) {
    show_textures = !ta->has_all_off();
  }
  _forest.EnableTexturing(show_textures);

  // Check lighting state.  SpeedTree only supports a single
  // directional light; we look for a directional light in the
  // lighting state and pass its direction to SpeedTree.
  NodePath light;
  const LightAttrib *la = DCAST(LightAttrib, state->get_attrib(LightAttrib::get_class_slot()));
  if (la != (LightAttrib *)NULL) {
    light = la->get_most_important_light();
  }
  if (!light.is_empty() && light.node()->is_of_type(DirectionalLight::get_class_type())) {
    DirectionalLight *light_obj = DCAST(DirectionalLight, light.node());

    CPT(TransformState) transform = light.get_transform(trav->get_scene()->get_scene_root().get_parent());
    LVector3f dir = light_obj->get_direction() * transform->get_mat();
    _forest.SetLightDir(SpeedTree::Vec3(dir[0], dir[1], dir[2]));

  } else {
    // No light.  But there's no way to turn off lighting in
    // SpeedTree.  In lieu of this, we just shine a light from
    // above.
    _forest.SetLightDir(SpeedTree::Vec3(0.0, 0.0, -1.0));
  }

  if (!_needs_repopulate) {
    // Don't bother culling now unless we're correctly fully
    // populated.  (Culling won't be accurate unless the forest has
    // been populated, but we have to be in the draw traversal to
    // populate.)
    _forest.CullAndComputeLOD(_view, _visible_trees);
  }

  // Recurse onto the node's children.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::is_renderable
//       Access: Public, Virtual
//  Description: Returns true if there is some value to visiting this
//               particular node during the cull traversal for any
//               camera, false otherwise.  This will be used to
//               optimize the result of get_net_draw_show_mask(), so
//               that any subtrees that contain only nodes for which
//               is_renderable() is false need not be visited.
////////////////////////////////////////////////////////////////////
bool SpeedTreeNode::
is_renderable() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::add_for_draw
//       Access: Public, Virtual
//  Description: Adds the node's contents to the CullResult we are
//               building up during the cull traversal, so that it
//               will be drawn at render time.  For most nodes other
//               than GeomNodes, this is a do-nothing operation.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  if (_is_valid) {
    // We create a CullableObject that has an explicit draw_callback
    // into this node, so that we can make the appropriate calls into
    // SpeedTree to render the forest during the actual draw.
    CullableObject *object = 
      new CullableObject(NULL, data._state,
			 TransformState::make_identity(),
			 TransformState::make_identity(),
			 trav->get_gsg());
    object->set_draw_callback(new DrawCallback(this));
    trav->get_cull_handler()->record_object(object, trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::prepare_scene
//       Access: Published
//  Description: Walks through the scene graph beginning at this node,
//               and does whatever initialization is required to
//               render the scene properly with the indicated GSG.  It
//               is not strictly necessary to call this, since the GSG
//               will initialize itself when the scene is rendered,
//               but this may take some of the overhead away from that
//               process.
//
//               In particular, this will ensure that textures within
//               the scene are loaded in texture memory, and display
//               lists are built up from static geometry.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
prepare_scene(GraphicsStateGuardianBase *gsgbase, const RenderState *) {
  GraphicsStateGuardian *gsg = DCAST(GraphicsStateGuardian, gsgbase);
  if (validate_api(gsg)) {
    setup_for_render(gsg);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::compute_internal_bounds
//       Access: Protected, Virtual
//  Description: Returns a newly-allocated BoundingVolume that
//               represents the internal contents of the node.  Should
//               be overridden by PandaNode classes that contain
//               something internally.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  internal_vertices = 0;

  SpeedTree::CExtents extents;
  Trees::const_iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    const STTree *tree = instance_list->get_tree();

    const STInstances &st_instances = instance_list->_instances;
    STInstances::const_iterator ii;
    for (ii = st_instances.begin(); ii != st_instances.end(); ++ii) {
      SpeedTree::CExtents tree_extents = tree->get_tree()->GetExtents();
      tree_extents.Rotate((*ii).GetRotationAngle());
      tree_extents.Scale((*ii).GetScale());
      tree_extents.Translate((*ii).GetPos());
      extents.ExpandAround(tree_extents);
    }
  }

  const SpeedTree::Vec3 &emin = extents.Min();
  const SpeedTree::Vec3 &emax = extents.Max();
  internal_bounds = new BoundingBox(LPoint3f(emin[0], emin[1], emin[2]),
				    LPoint3f(emax[0], emax[1], emax[2]));
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::output
//       Access: Public, Virtual
//  Description: Writes a brief description of the node to the
//               indicated output stream.  This is invoked by the <<
//               operator.  It may be overridden in derived classes to
//               include some information relevant to the class.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
output(ostream &out) const {
  PandaNode::output(out);
  out
    << " (" << get_num_trees() << " unique trees with "
    << count_total_instances() << " total instances)";
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
write(ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);

  Trees::const_iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    instance_list->write(out, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::init_node
//       Access: Private
//  Description: Called from the constructor to initialize some
//               internal values.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
init_node() {
  PandaNode::set_cull_callback();

  _is_valid = false;
  _needs_repopulate = false;

  // Ensure we have a license.
  if (!authorize()) {
    speedtree_cat.warning()
      << "SpeedTree license not available.\n";
    return;
  }

  _forest.SetHint(SpeedTree::CForest::HINT_MAX_NUM_VISIBLE_CELLS, speedtree_max_num_visible_cells);

  _forest.SetCullCellSize(speedtree_cull_cell_size);

  _is_valid = true;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::r_add_instances
//       Access: Private
//  Description: The recursive implementation of add_instances().
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
r_add_instances(PandaNode *node, const TransformState *transform,
		Thread *current_thread) {
  if (node->is_of_type(SpeedTreeNode::get_class_type()) && node != this) {
    SpeedTreeNode *other = DCAST(SpeedTreeNode, node);

    int num_trees = other->get_num_trees();
    for (int ti = 0; ti < num_trees; ++ti) {
      const InstanceList &other_instance_list = other->get_instance_list(ti);
      const STTree *tree = other_instance_list.get_tree();
      InstanceList &this_instance_list = add_tree(tree);
      
      int num_instances = other_instance_list.get_num_instances();
      for (int i = 0; i < num_instances; ++i) {
	CPT(TransformState) other_trans = other_instance_list.get_instance(i);
	CPT(TransformState) new_trans = transform->compose(other_trans);
	this_instance_list.add_instance(new_trans.p());
      }
    }
  }

  Children children = node->get_children(current_thread);
  for (int i = 0; i < children.get_num_children(); i++) {
    PandaNode *child = children.get_child(i);
    CPT(TransformState) child_transform = transform->compose(child->get_transform());
    r_add_instances(child, child_transform, current_thread);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::repopulate
//       Access: Private
//  Description: Rebuilds the internal structures as necessary for
//               rendering.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
repopulate() {
  _forest.ClearInstances();

  Trees::iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    const STTree *tree = instance_list->get_tree();
    const STInstances &instances = instance_list->_instances;
    if (instances.empty()) {
      // There are no instances, so don't bother.  (This shouldn't
      // happen often, because we remove trees from the SpeedTreeNode
      // when their instance list goes empty, though it's possible if
      // the user has explicitly removed all of the instances.)
      continue;
    }

    if (!_forest.AddInstances(tree->get_tree(), &instances[0], instances.size())) {
      speedtree_cat.warning()
	<< "Failed to add " << instances.size()
	<< " instances for " << *tree << "\n";
      speedtree_cat.warning()
	<< SpeedTree::CCore::GetError() << "\n";
    }
  }
  
  _forest.GetPopulationStats(_population_stats);
  print_forest_stats(_population_stats);

  // setup billboard caps based on instances-per-cell stats
  int max_instances_by_cell = 1;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    const STTree *tree = instance_list->get_tree();
    const STInstances &instances = instance_list->_instances;
    if (instances.empty()) {
      continue;
    }

    int max_instances = 1;
    SpeedTree::CMap<const SpeedTree::CTree*, SpeedTree::st_int32>::const_iterator si;
    si = _population_stats.m_mMaxNumInstancesPerCellPerBase.find(tree->get_tree());
    if (si != _population_stats.m_mMaxNumInstancesPerCellPerBase.end()) {
      max_instances = max(max_instances, (int)si->second);
    }

    max_instances_by_cell = max(max_instances_by_cell, max_instances);
  }

  _visible_trees.Reserve(_forest.GetBaseTrees(),
			 _forest.GetBaseTrees().size(), 
			 speedtree_max_num_visible_cells, 
			 max_instances_by_cell,
			 speedtree_allow_horizontal_billboards);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::validate_api
//       Access: Private
//  Description: Returns true if the indicated GSG shares the
//               appropriate API for this SpeedTreeNode, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool SpeedTreeNode::
validate_api(GraphicsStateGuardian *gsg) {
  GraphicsPipe *pipe = gsg->get_pipe();
  nassertr(pipe != (GraphicsPipe *)NULL, true);

#if defined(SPEEDTREE_OPENGL)
  static const string compiled_api = "OpenGL";
#elif defined(SPEEDTREE_DIRECTX9)
  static const string compiled_api = "DirectX9";
#else
  #error Unexpected graphics API.
#endif

  if (pipe->get_interface_name() != compiled_api) {
    speedtree_cat.error()
      << "SpeedTree is compiled for " << compiled_api
      << ", cannot render with " << pipe->get_interface_name()
      << "\n";
    _is_valid = false;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::draw_callback
//       Access: Private
//  Description: Called when the node is visited during the draw
//               traversal, by virtue of our DrawCallback construct.
//               This makes the calls into SpeedTree to perform the
//               actual rendering.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
draw_callback(CallbackData *data) {
  GeomDrawCallbackData *geom_cbdata;
  DCAST_INTO_V(geom_cbdata, data);

  GraphicsStateGuardian *gsg = DCAST(GraphicsStateGuardian, geom_cbdata->get_gsg());

  setup_for_render(gsg);
  
  // start the forest render
  _forest.StartRender();
  
  bool branches = _forest.RenderBranches(_visible_trees, SpeedTree::RENDER_PASS_STANDARD);
  bool fronds = _forest.RenderFronds(_visible_trees, SpeedTree::RENDER_PASS_STANDARD);
  bool leaf_meshes = _forest.RenderLeafMeshes(_visible_trees, SpeedTree::RENDER_PASS_STANDARD);
  bool leaf_cards = _forest.RenderLeafCards(_visible_trees, SpeedTree::RENDER_PASS_STANDARD, _view);
  bool billboards = _forest.RenderBillboards(_visible_trees, SpeedTree::RENDER_PASS_STANDARD, _view);

  if (!branches || !fronds || !leaf_meshes || !leaf_cards || !billboards) {
    speedtree_cat.warning()
      << "Failed to render forest completely: "
      << branches << " " << fronds << " " << leaf_meshes << " " << leaf_cards << " " << billboards << "\n";
    speedtree_cat.warning()
      << SpeedTree::CCore::GetError() << "\n";
  }

  _forest.EndRender();

  // SpeedTree leaves the graphics state indeterminate.  But this
  // doesn't help?
  geom_cbdata->set_lost_state(true);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::setup_for_render
//       Access: Private
//  Description: Does whatever calls are necessary to set up the
//               forest for rendering--create vbuffers, load shaders,
//               and whatnot.  Primarily, this is the calls to
//               InitTreeGraphics and the like.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
setup_for_render(GraphicsStateGuardian *gsg) {
  if (!_done_first_init) {
    // This is the first time we have entered the draw callback since
    // creating any SpeedTreeNode.  Now we have an opportunity to do
    // any initial setup that requires a graphics context.
    
#ifdef SPEEDTREE_OPENGL
    // For OpenGL, we have to ensure GLEW has been initialized.
    // (SpeedTree uses it, though Panda doesn't.)
    GLenum err = glewInit();
    if (err != GLEW_OK) {
      speedtree_cat.error()
	<< "GLEW initialization failed: %s\n", glewGetErrorString(err);
      // Can't proceed without GLEW.
      _is_valid = false;
      return;
    }

    // Insist that OpenGL 2.0 is available as the SpeedTree renderer
    // requires it.
    if (!GLEW_VERSION_2_0) {
      speedtree_cat.error()
	<< "The SpeedTree OpenGL implementation requires OpenGL 2.0 or better to run; this system has version " << glGetString(GL_VERSION) << "\n";
      _is_valid = false;
      return;
    }
#endif  // SPEEDTREE_OPENGL

    _done_first_init = true;
  }

  if (_needs_repopulate) {
    repopulate();

    // Now init per-tree graphics
    Trees::const_iterator ti;
    for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
      InstanceList *instance_list = (*ti);
      const STTree *tree = instance_list->get_tree();
      const STInstances &instances = instance_list->_instances;
      if (instances.empty()) {
	continue;
      }
      
      int max_instances = 2;
      SpeedTree::CMap<const SpeedTree::CTree*, SpeedTree::st_int32>::const_iterator si;
      si = _population_stats.m_mMaxNumInstancesPerCellPerBase.find(tree->get_tree());
      if (si != _population_stats.m_mMaxNumInstancesPerCellPerBase.end()) {
	max_instances = max(max_instances, (int)si->second);
      }
      
      if (!_forest.InitTreeGraphics((SpeedTree::CTreeRender *)tree->get_tree(), 
				    max_instances, speedtree_allow_horizontal_billboards)) {
	speedtree_cat.warning()
	  << "Failed to init tree graphics for " << *tree << "\n";
	speedtree_cat.warning()
	  << SpeedTree::CCore::GetError() << "\n";
      }
    }

    // Init overall graphics
    if (!_forest.InitGraphics(false)) {
      speedtree_cat.warning()
	<< "Failed to init graphics\n";
      speedtree_cat.warning()
	<< SpeedTree::CCore::GetError() << "\n";
      _is_valid = false;
      return;
    }

    // This call apparently must be made at draw time, not earlier,
    // because it might attempt to create OpenGL index buffers and
    // such.
    _forest.UpdateTreeCellExtents();

    // If we needed to repopulate, it means we didn't cull in the cull
    // traversal.  Do it now.
    _forest.CullAndComputeLOD(_view, _visible_trees);

    _needs_repopulate = false;
  }

  if (!_forest.UploadViewShaderParameters(_view)) {
    speedtree_cat.warning()
      << "Couldn't set view parameters\n";
    speedtree_cat.warning()
      << SpeedTree::CCore::GetError() << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::print_forest_stats
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
print_forest_stats(const SpeedTree::CForest::SPopulationStats &forest_stats) const {
  fprintf(stderr, "\n                Forest Population Statistics\n");
  fprintf(stderr, "   ---------------------------------------------------\n");
  fprintf(stderr, "                    # of tree cull cells: %d\n", forest_stats.m_nNumCells);
  fprintf(stderr, "                  # of unique base trees: %d\n", forest_stats.m_nNumBaseTrees);
  fprintf(stderr, "                    total # of instances: %d\n", forest_stats.m_nNumInstances);
  fprintf(stderr, "         average # of instances per base: %g\n", forest_stats.m_fAverageNumInstancesPerBase);
  fprintf(stderr, "  max # of billboards/instances per cell: %d\n", forest_stats.m_nMaxNumBillboardsPerCell);
  fprintf(stderr, "    max # of instances per cell per base:\n");
  SpeedTree::CMap<const SpeedTree::CTree*, SpeedTree::st_int32>::const_iterator i;
  for (i = forest_stats.m_mMaxNumInstancesPerCellPerBase.begin( ); i != forest_stats.m_mMaxNumInstancesPerCellPerBase.end( ); ++i) {
    fprintf(stderr, "        %35s: %4d\n", SpeedTree::CFixedString(i->first->GetFilename( )).NoPath( ).c_str( ), i->second);
  }
  fprintf(stderr, "            average # instances per cell: %g\n", forest_stats.m_fAverageInstancesPerCell);
  fprintf(stderr, "               max # of billboard images: %d\n", forest_stats.m_nMaxNumBillboardImages);
  fprintf(stderr, "\n");
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               SpeedTreeNode.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type SpeedTreeNode is encountered
//               in the Bam file.  It should create the SpeedTreeNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *SpeedTreeNode::
make_from_bam(const FactoryParams &params) {
  SpeedTreeNode *node = new SpeedTreeNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new SpeedTreeNode.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::InstanceList::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::InstanceList::
output(ostream &out) const {
  out << *_tree << ": " << _instances.size() << " instances.";
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::InstanceList::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::InstanceList::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << *_tree << ": " << _instances.size() << " instances.\n";
  STInstances::const_iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    indent(out, indent_level + 2)
      << STTransform(*ii) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SpeedTreeNode::DrawCallback::do_callback
//       Access: Public, Virtual
//  Description: This method called when the callback is triggered; it
//               *replaces* the original function.  To continue
//               performing the original function, you must call
//               cbdata->upcall() during the callback.
////////////////////////////////////////////////////////////////////
void SpeedTreeNode::DrawCallback::
do_callback(CallbackData *data) {
  _node->draw_callback(data);
}
