/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file maxToEggConverter.cxx
 * @author Corey Revilla and Ken Strickland
 * @date 2003-06-22
 * from mayaToEggConverter.cxx created by drose (10Nov99)
 *
 * Updated by Fei Wang, Carnegie Mellon University Entertainment
 * Technology Center student, 29Jul2009:  Fixed vertex color,
 * animation hierarchy, texture swapping bugs; added collision choices to
 * exporter.
 *
 * Updated by Andrew Gartner, Carnegie Mellon University Entertainment
 * Technology Center. 27Apr2010: Collision is now done through User Defined Properties
 * By default a plane without a standard material gets UV's as well
 * as any object without a texture but with a standard material.
 * Point objects are now supported as "locators" for a point in space
 * within the egg.
 */

#include "maxEgg.h"
#include "config_putil.h"

using std::string;

/**
 *
 */
MaxToEggConverter::
MaxToEggConverter()
{
    reset();
}

/**
 *
 */
MaxToEggConverter::
~MaxToEggConverter()
{
}

/**

 */
void MaxToEggConverter::reset() {
    _cur_tref = 0;
    _current_frame = 0;
    _textures.clear();
    _egg_data = nullptr;
}

/**
 * Fills up the egg_data structure according to the global Max model data.
 * Returns true if successful, false if there is an error.  If from_selection
 * is true, the converted geometry is based on that which is selected;
 * otherwise, it is the entire Max scene.
 */
bool MaxToEggConverter::convert(MaxEggOptions *options) {

    _options = options;

    Filename fn;
#ifdef _UNICODE
    fn = Filename::from_os_specific_w(_options->_file_name);
#else
    fn = Filename::from_os_specific(_options->_file_name);
#endif

    _options->_path_replace->_path_directory = fn.get_dirname();

    _egg_data = new EggData;
    if (_egg_data->get_coordinate_system() == CS_default) {
        _egg_data->set_coordinate_system(CS_zup_right);
    }

    // Figure out the animation parameters.

    // Get the start and end frames and the animation frame rate from Max

    Interval anim_range = _options->_max_interface->GetAnimRange();
    int start_frame = anim_range.Start()/GetTicksPerFrame();
    int end_frame = anim_range.End()/GetTicksPerFrame();

    if (!_options->_export_all_frames) {
        if (_options->_start_frame < start_frame) _options->_start_frame = start_frame;
        if (_options->_start_frame > end_frame)   _options->_start_frame = end_frame;
        if (_options->_end_frame < start_frame)   _options->_end_frame = start_frame;
        if (_options->_end_frame > end_frame)     _options->_end_frame = end_frame;
        if (_options->_end_frame < _options->_start_frame)  _options->_end_frame = _options->_start_frame;
        start_frame = _options->_start_frame;
        end_frame = _options->_end_frame;
    }

    int frame_inc = 1;
    int output_frame_rate = GetFrameRate();

    bool all_ok = true;

    if (_options->_export_whole_scene) {
        _tree._export_mesh = false;
        all_ok = _tree.build_complete_hierarchy(_options->_max_interface->GetRootNode(), nullptr, 0);
    } else {
        _tree._export_mesh = true;
        all_ok = _tree.build_complete_hierarchy(_options->_max_interface->GetRootNode(), &_options->_node_list.front(), _options->_node_list.size());
    }

    if (all_ok) {
        switch (_options->_anim_type) {
        case MaxEggOptions::AT_pose:
            // pose: set to a specific frame, then get out the static
            // geometry.  sprintf(Logger::GetLogString(), "Extracting geometry
            // from frame #%d.", start_frame); Logger::Log( MTEC,
            // Logger::SAT_MEDIUM_LEVEL, Logger::GetLogString() );
            // Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "Converting static
            // model."  );
            _current_frame = start_frame;
            all_ok = convert_hierarchy(_egg_data);
            break;

        case MaxEggOptions::AT_model:
            // model: get out an animatable model with joints and vertex
            // membership.
            all_ok = convert_char_model();
            break;

        case MaxEggOptions::AT_chan:
            // chan: get out a series of animation tables.
            all_ok = convert_char_chan(start_frame, end_frame, frame_inc,
                                       output_frame_rate);
            break;

        case MaxEggOptions::AT_both:
            // both: Put a model and its animation into the same egg file.
            _options->_anim_type = MaxEggOptions::AT_model;
            if (!convert_char_model()) {
                all_ok = false;
            }
            _options->_anim_type = MaxEggOptions::AT_chan;
            if (!convert_char_chan(start_frame, end_frame, frame_inc,
                                   output_frame_rate)) {
                all_ok = false;
            }
            // Set the type back to AT_both
            _options->_anim_type = MaxEggOptions::AT_both;
            break;

          default:
            all_ok = false;
        };

        reparent_decals(_egg_data);
    }

    if (all_ok) {
        _egg_data->recompute_tangent_binormal_auto();
        _egg_data->remove_unused_vertices(true);
    }

    _options->_successful = all_ok;

    if (all_ok) {
#ifdef _UNICODE
        Filename fn = Filename::from_os_specific_w(_options->_file_name);
#else
        Filename fn = Filename::from_os_specific(_options->_file_name);
#endif
        return _egg_data->write_egg(fn);
    } else {
        return false;
    }
}

/**
 * Converts the file as an animatable character model, with joints and vertex
 * membership.
 */
bool MaxToEggConverter::
convert_char_model() {
    std::string character_name = "character";
    _current_frame = _options->_start_frame;

    EggGroup *char_node = new EggGroup(character_name);
    _egg_data->add_child(char_node);
    char_node->set_dart_type(EggGroup::DT_default);

    return convert_hierarchy(char_node);
}

/**
 * Converts the animation as a series of tables to apply to the character
 * model, as retrieved earlier via AC_model.
 */
bool MaxToEggConverter::
convert_char_chan(double start_frame, double end_frame, double frame_inc,
                  double output_frame_rate) {
    std::string character_name = "character";

    EggTable *root_table_node = new EggTable();
    _egg_data->add_child(root_table_node);
    EggTable *bundle_node = new EggTable(character_name);
    bundle_node->set_table_type(EggTable::TT_bundle);
    root_table_node->add_child(bundle_node);
    EggTable *skeleton_node = new EggTable("<skeleton>");
    bundle_node->add_child(skeleton_node);

    // Set the frame rate before we start asking for anim tables to be
    // created.
    _tree._fps = output_frame_rate / frame_inc;
    _tree.clear_egg(_egg_data, nullptr, skeleton_node);

    // Now we can get the animation data by walking through all of the frames,
    // one at a time, and getting the joint angles at each frame.

    // This is just a temporary EggGroup to receive the transform for each
    // joint each frame.
    EggGroup* tgroup;

    int num_nodes = _tree.get_num_nodes();
    int i;

    TimeValue frame = start_frame;
    TimeValue frame_stop = end_frame;
    while (frame <= frame_stop) {
        _current_frame = frame;
        for (i = 0; i < num_nodes; i++) {
            // Find all joints in the hierarchy
            MaxNodeDesc *node_desc = _tree.get_node(i);
            if (node_desc->is_joint()) {
                tgroup = new EggGroup();
                INode *max_node = node_desc->get_max_node();

                if (node_desc->_parent && node_desc->_parent->is_joint()) {
                    // If this joint also has a joint as a parent, the
                    // parent's transformation has to be divided out of this
                    // joint's TM
                    get_joint_transform(max_node, node_desc->_parent->get_max_node(),
                                        tgroup);
                } else {
                    get_joint_transform(max_node, nullptr, tgroup);
                }

                EggXfmSAnim *anim = _tree.get_egg_anim(node_desc);
                if (!anim->add_data(tgroup->get_transform3d())) {
                    // *** log an error
                }
                delete tgroup;
            }
        }

        frame += frame_inc;
    }

    // Now optimize all of the tables we just filled up, for no real good
    // reason, except that it makes the resulting egg file a little easier to
    // read.
    for (i = 0; i < num_nodes; i++) {
        MaxNodeDesc *node_desc = _tree.get_node(i);
        if (node_desc->is_joint()) {
            _tree.get_egg_anim(node_desc)->optimize();
        }
    }

    return true;
}

/**
 * Generates egg structures for each node in the Max hierarchy.
 */
bool MaxToEggConverter::
convert_hierarchy(EggGroupNode *egg_root) {
    // int num_nodes = _tree.get_num_nodes();

    _tree.clear_egg(_egg_data, egg_root, nullptr);
    for (int i = 0; i < _tree.get_num_nodes(); i++) {
        if (!process_model_node(_tree.get_node(i))) {
            return false;
        }
    }

    return true;
}

/**
 * Converts the indicated Max node to the corresponding Egg structure.
 * Returns true if successful, false if an error was encountered.
 */
bool MaxToEggConverter::
process_model_node(MaxNodeDesc *node_desc) {
    if (!node_desc->has_max_node()) {
        // If the node has no Max equivalent, never mind.
        return true;
    }

    // Skip all nodes that represent joints in the geometry, but aren't the
    // actual joints themselves
    if (node_desc->is_node_joint()) {
        return true;
    }

    TimeValue time = 0;
    INode *max_node = node_desc->get_max_node();

    ObjectState state;
    state = max_node->EvalWorldState(_current_frame * GetTicksPerFrame());

    if (node_desc->is_joint()) {
        EggGroup *egg_group = _tree.get_egg_group(node_desc);
        // Don't bother with joints unless we're getting an animatable model.
        if (_options->_anim_type == MaxEggOptions::AT_model) {
            get_joint_transform(max_node, egg_group);
        }
    } else {
        if (state.obj) {
            EggGroup *egg_group = nullptr;
            TriObject *myMaxTriObject;
            Mesh max_mesh;
            // Call the correct exporter based on what type of object this is.
            switch( state.obj->SuperClassID() ){

            case GEOMOBJECT_CLASS_ID:
                egg_group = _tree.get_egg_group(node_desc);
                get_transform(max_node, egg_group);

                // Try converting this geometric object to a mesh we can use.
                if (!state.obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) {
                    return false;
                }
                // Convert our state object to a TriObject.
                myMaxTriObject = (TriObject *) state.obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0 ));
                // *** Want to figure this problem out If actual conversion
                // was required, then we want to delete this new mesh later to
                // avoid mem leaks.  **BROKEN. doesnt delete

                // Now, get the mesh.
                max_mesh = myMaxTriObject->GetMesh();
                make_polyset(max_node, &max_mesh, egg_group);

                if (myMaxTriObject != state.obj)
                    delete myMaxTriObject;
                break;

            case SHAPE_CLASS_ID:
                if (state.obj->ClassID() == EDITABLE_SURF_CLASS_ID) {
                    NURBSSet getSet;
                    if (GetNURBSSet(state.obj, time, getSet, TRUE)) {
                        NURBSObject *nObj = getSet.GetNURBSObject(0);
                        if (nObj->GetType() == kNCVCurve) {
                            // It's a CV Curve, process it
                            egg_group = _tree.get_egg_group(node_desc);
                            get_transform(max_node, egg_group);
                            make_nurbs_curve(max_node, (NURBSCVCurve *)nObj,
                                             time, egg_group);
                        }
                    }
                }
                break;

            case CAMERA_CLASS_ID:
                break;

            case LIGHT_CLASS_ID:
                break;

            case HELPER_CLASS_ID:
              // we should export Point objects to give Max the equivalent of
              // Maya locators
              if (state.obj->ClassID() == Class_ID(POINTHELP_CLASS_ID, 0)) {

                egg_group = _tree.get_egg_group(node_desc);
                get_transform(max_node, egg_group);

              } else {

                break;

              }




            }
        }
    }

    return true;
}

/**
 * Extracts the transform on the indicated Maya node, and applies it to the
 * corresponding Egg node.
 */
void MaxToEggConverter::
get_transform(INode *max_node, EggGroup *egg_group) {
    if (_options->_anim_type == MaxEggOptions::AT_model) {
        // When we're getting an animated model, we only get transforms for
        // joints.
        return;
    }

    if ( !egg_group ) {
        return;
    }

    // Gets the TM for this node, a matrix which encapsulates all
    // transformations it takes to get to the current node, including parent
    // transformations.
    Matrix3 pivot = max_node->GetNodeTM(_current_frame * GetTicksPerFrame());

    // This is the Panda-flava-flav-style matrix we'll be exporting to.
    Point3 row0 = pivot.GetRow(0);
    Point3 row1 = pivot.GetRow(1);
    Point3 row2 = pivot.GetRow(2);
    Point3 row3 = pivot.GetRow(3);

    LMatrix4d m4d(row0.x, row0.y, row0.z, 0.0f,
                  row1.x, row1.y, row1.z, 0.0f,
                  row2.x, row2.y, row2.z, 0.0f,
                  row3.x, row3.y, row3.z, 1.0f );

    // Now here's the tricky part.  I believe this command strips out the node
    // "frame" which is the sum of all transformations enacted by the parent
    // of this node.  This should reduce to the transformation relative to
    // this node's parent
    m4d = m4d * egg_group->get_node_frame_inv();
    if (!m4d.almost_equal(LMatrix4d::ident_mat(), 0.0001)) {
        egg_group->add_matrix4(m4d);
    }
}

/**
 * Extracts the transform on the indicated Maya node, and applies it to the
 * corresponding Egg node.
 */
LMatrix4d MaxToEggConverter::
get_object_transform(INode *max_node) {

    // Gets the TM for this node, a matrix which encapsulates all
    // transformations it takes to get to the current node, including parent
    // transformations.
    Matrix3 pivot = max_node->GetObjectTM(_current_frame * GetTicksPerFrame());

    Point3 row0 = pivot.GetRow(0);
    Point3 row1 = pivot.GetRow(1);
    Point3 row2 = pivot.GetRow(2);
    Point3 row3 = pivot.GetRow(3);

    LMatrix4d m4d(row0.x, row0.y, row0.z, 0.0f,
                  row1.x, row1.y, row1.z, 0.0f,
                  row2.x, row2.y, row2.z, 0.0f,
                  row3.x, row3.y, row3.z, 1.0f );
    return m4d;
}

/**
 * Extracts the transform on the indicated Maya node, as appropriate for a
 * joint in an animated character, and applies it to the indicated node.  This
 * is different from get_transform() in that it does not respect the
 * _transform_type flag, and it does not consider the relative transforms
 * within the egg file.
 */
void MaxToEggConverter::
get_joint_transform(INode *max_node, EggGroup *egg_group) {

    if ( !egg_group ) {
        return;
    }

    // Gets the TM for this node, a matrix which encapsulates all
    // transformations it takes to get to the current node, including parent
    // transformations.
    Matrix3 pivot = max_node->GetNodeTM(_current_frame * GetTicksPerFrame());
    Point3 row0 = pivot.GetRow(0);
    Point3 row1 = pivot.GetRow(1);
    Point3 row2 = pivot.GetRow(2);
    Point3 row3 = pivot.GetRow(3);

    LMatrix4d m4d(row0.x, row0.y, row0.z, 0.0f,
                  row1.x, row1.y, row1.z, 0.0f,
                  row2.x, row2.y, row2.z, 0.0f,
                  row3.x, row3.y, row3.z, 1.0f );

    // Now here's the tricky part.  I believe this command strips out the node
    // "frame" which is the sum of all transformations enacted by the parent
    // of this node.  This should reduce to the transformation relative to
    // this node's parent
    m4d = m4d * egg_group->get_node_frame_inv();
    if (!m4d.almost_equal(LMatrix4d::ident_mat(), 0.0001)) {
        egg_group->add_matrix4(m4d);
    }
}

/**
 * Extracts the transform on the indicated Maya node, as appropriate for a
 * joint in an animated character, and applies it to the indicated node.  This
 * is different from get_transform() in that it does not respect the
 * _transform_type flag, and it does not consider the relative transforms
 * within the egg file.
 */
void MaxToEggConverter::
get_joint_transform(INode *max_node, INode *parent_node, EggGroup *egg_group) {

    if ( !egg_group ) {
        return;
    }

    // Gets the TM for this node, a matrix which encapsulates all
    // transformations it takes to get to the current node, including parent
    // transformations.
    Matrix3 pivot = max_node->GetNodeTM(_current_frame * GetTicksPerFrame());
    Point3 row0 = pivot.GetRow(0);
    Point3 row1 = pivot.GetRow(1);
    Point3 row2 = pivot.GetRow(2);
    Point3 row3 = pivot.GetRow(3);

    LMatrix4d m4d(row0.x, row0.y, row0.z, 0.0f,
                  row1.x, row1.y, row1.z, 0.0f,
                  row2.x, row2.y, row2.z, 0.0f,
                  row3.x, row3.y, row3.z, 1.0f );

    if (parent_node) {
        Matrix3 parent_pivot = parent_node->GetNodeTM(_current_frame * GetTicksPerFrame());
        // parent_pivot.Invert();
        row0 = parent_pivot.GetRow(0);
        row1 = parent_pivot.GetRow(1);
        row2 = parent_pivot.GetRow(2);
        row3 = parent_pivot.GetRow(3);

        LMatrix4d pi_m4d(row0.x, row0.y, row0.z, 0.0f,
                         row1.x, row1.y, row1.z, 0.0f,
                         row2.x, row2.y, row2.z, 0.0f,
                         row3.x, row3.y, row3.z, 1.0f );

        // Now here's the tricky part.  I believe this command strips out the
        // node "frame" which is the sum of all transformations enacted by the
        // parent of this node.  This should reduce to the transformation
        // relative to this node's parent
        pi_m4d.invert_in_place();
        m4d = m4d * pi_m4d;
    }
    if (!m4d.almost_equal(LMatrix4d::ident_mat(), 0.0001)) {
        egg_group->add_matrix4(m4d);
    }
}

/**
 * Converts the indicated Maya NURBS curve (a standalone curve, not a trim
 * curve) to a corresponding egg structure and attaches it to the indicated
 * egg group.
 */
bool MaxToEggConverter::
make_nurbs_curve(INode *max_node, NURBSCVCurve *curve,
                 TimeValue time, EggGroup *egg_group)
{
    int degree = curve->GetOrder();
    int cvs = curve->GetNumCVs();
    int knots = curve->GetNumKnots();
    int i;

    if (knots != cvs + degree) {
        return false;
    }

#ifdef _UNICODE
    char mbname[1024];
    mbname[1023] = 0;
    wcstombs(mbname, max_node->GetName(), 1023);
    string name(mbname);
#else
    string name = max_node->GetName();
#endif

    string vpool_name = name + ".cvs";
    EggVertexPool *vpool = new EggVertexPool(vpool_name);
    egg_group->add_child(vpool);

    EggNurbsCurve *egg_curve = new EggNurbsCurve(name);
    egg_group->add_child(egg_curve);
    egg_curve->setup(degree, knots);

    for (i = 0; i < knots; i++)
        egg_curve->set_knot(i, curve->GetKnot(i));

    LMatrix4d vertex_frame_inv = egg_group->get_vertex_frame_inv();

    for (i = 0; i < cvs; i++) {
        NURBSControlVertex *cv = curve->GetCV(i);
        if (!cv) {
            char buf[1024];
            sprintf(buf, "Error getting CV %d", i);
            return false;
        } else {
            EggVertex vert;
            LPoint4d p4d(0, 0, 0, 1.0);
            cv->GetPosition(time, p4d[0], p4d[1], p4d[2]);
            p4d = p4d * vertex_frame_inv;
            vert.set_pos(p4d);
            egg_curve->add_vertex(vpool->create_unique_vertex(vert));
        }
    }

    return true;
}

/**
 * Converts the indicated Maya polyset to a bunch of EggPolygons and parents
 * them to the indicated egg group.
 */
void MaxToEggConverter::
make_polyset(INode *max_node, Mesh *mesh,
             EggGroup *egg_group, Shader *default_shader) {

    mesh->buildNormals();

    if (mesh->getNumFaces() == 0) {
        return;
    }

    // One way to convert the mesh would be to first get out all the vertices
    // in the mesh and add them into the vpool, then when we traverse the
    // polygons we would only have to index them into the vpool according to
    // their Maya vertex index.

    // Unfortunately, since Maya may store multiple normals andor colors for
    // each vertex according to which polygon it is in, that approach won't
    // necessarily work.  In egg, those split-property vertices have to become
    // separate vertices.  So instead of adding all the vertices up front,
    // we'll start with an empty vpool, and add vertices to it on the fly.

#ifdef _UNICODE
    char mbname[1024];
    mbname[1023] = 0;
    wcstombs(mbname, max_node->GetName(), 1023);
    string node_name(mbname);
#else
    string node_name = max_node->GetName();
#endif

    string vpool_name = node_name + ".verts";
    EggVertexPool *vpool = new EggVertexPool(vpool_name);
    egg_group->add_child(vpool);

    // We will need to transform all vertices from world coordinate space into
    // the vertex space appropriate to this node.  Usually, this is the same
    // thing as world coordinate space, and this matrix will be identity; but
    // if the node is under an instance (particularly, for instance, a
    // billboard) then the vertex space will be different from world space.
    LMatrix4d vertex_frame = get_object_transform(max_node) *
        egg_group->get_vertex_frame_inv();


    for ( int iFace=0; iFace < mesh->getNumFaces(); iFace++ ) {
        EggPolygon *egg_poly = new EggPolygon;
        egg_group->add_child(egg_poly);

        egg_poly->set_bface_flag(_options->_double_sided);

        Face face = mesh->faces[iFace];

        const PandaMaterial &pmat = get_panda_material(max_node->GetMtl(), face.getMatID());

        // Get the vertices for the polygon.
        for ( int iVertex=0; iVertex < 3; iVertex++ ) {
            EggVertex vert;

            // Get the vertex position
            Point3 vertex = mesh->getVert(face.v[iVertex]);
            LPoint3d p3d(vertex.x, vertex.y, vertex.z);
            p3d = p3d * vertex_frame;
            vert.set_pos(p3d);

            // Get the vertex normal
            Point3 normal = get_max_vertex_normal(mesh, iFace, iVertex);
            LVector3d n3d(normal.x, normal.y, normal.z);
            // *** Not quite sure if this transform should be applied, but it
            // may explain why normals were weird previously
            n3d = n3d * vertex_frame;
            vert.set_normal(n3d);

            // Get the vertex color
            if(mesh->vcFace)  // if has vcFace, has used vertex color
            {
                VertColor vertexColor = get_max_vertex_color(mesh, iFace, iVertex);
                LColor pVC(vertexColor.x, vertexColor.y, vertexColor.z, 1);
                vert.set_color(pVC);
            }
            // Get the UVs for this vertex

            // first check if we returned nothing in the channels slot we need
            // UV's even in this case because the user may not have put a
            // material on the object at all
            if (pmat._map_channels.size() == 0) {
              // since the channel will always be one because there's no other
              // textures then don't bother with the name
              UVVert uvw = get_max_vertex_texcoord(mesh, iFace, iVertex, 1);
              vert.set_uv( LTexCoordd(uvw.x, uvw.y));
            }
            // otherwise go through and generate the maps per channel this
            // will also generate default UV's as long as the user applies a
            // standard material to the object
            for (int iChan=0; iChan<pmat._map_channels.size(); iChan++) {
                int channel = pmat._map_channels[iChan];
                std::ostringstream uvname;
                uvname << "m" << channel;
                UVVert uvw = get_max_vertex_texcoord(mesh, iFace, iVertex, channel);
                // changes allow the first channel to be swapped
                if(channel == 1)
                    vert.set_uv( LTexCoordd(uvw.x, uvw.y));
                else
                    vert.set_uv( uvname.str(), LTexCoordd(uvw.x, uvw.y));

            }

            vert.set_external_index(face.v[iVertex]);

            egg_poly->add_vertex(vpool->create_unique_vertex(vert));
        }

        // Max uses normals, not winding, to determine which way a polygon
        // faces.  Make sure the winding and that normal agree

        EggVertex *verts[3];
        LPoint3d points[3];

        for (int i = 0; i < 3; i++) {
            verts[i] = egg_poly->get_vertex(i);
            points[i] = verts[i]->get_pos3();
        }

        LVector3d realNorm = ((points[1] - points[0]).cross(points[2] - points[0]));
        Point3 maxNormTemp = mesh->getFaceNormal(iFace);
        LVector3d maxNorm = (LVector3d(maxNormTemp.x, maxNormTemp.y, maxNormTemp.z) *
                             vertex_frame);

        if (realNorm.dot(maxNorm) < 0.0) {
            egg_poly->set_vertex(0, verts[2]);
            egg_poly->set_vertex(2, verts[0]);
        }

        for (int i=0; i<pmat._texture_list.size(); i++) {
            egg_poly->add_texture(pmat._texture_list[i]);
        }
        egg_poly->set_color(pmat._color);


    }

    // Now that we've added all the polygons (and created all the vertices),
    // go back through the vertex pool and set up the appropriate joint
    // membership for each of the vertices.

    if (_options->_anim_type == MaxEggOptions::AT_model) {
        get_vertex_weights(max_node, vpool);
    }
}

UVVert MaxToEggConverter::get_max_vertex_texcoord(Mesh *mesh, int faceNo, int vertNo, int channel) {

    // extract the texture coordinate
    UVVert uvVert(0,0,0);
    if(mesh->mapSupport(channel)) {
        TVFace *pTVFace = mesh->mapFaces(channel);
        UVVert *pUVVert = mesh->mapVerts(channel);
        uvVert = pUVVert[pTVFace[faceNo].t[vertNo]];
    } else if(mesh->numTVerts > 0) {
        uvVert = mesh->tVerts[mesh->tvFace[faceNo].t[vertNo]];
    }
    return uvVert;
}

VertColor MaxToEggConverter::get_max_vertex_color(Mesh *mesh,int FaceNo,int VertexNo, int channel) {

  VertColor vc(0,0,0);
  if(mesh->mapSupport(channel))
  {
    // We get the color from vcFace
    TVFace& _vcface = mesh->vcFace[FaceNo];
    // Get its index into the vertCol array
    int VertexColorIndex = _vcface.t[VertexNo];
    // Get its color
    vc =mesh->vertCol[VertexColorIndex];
  }
  else
  {
    TVFace *pTVFace = mesh->mapFaces(channel);
    vc = mesh->vertCol[pTVFace[FaceNo].t[VertexNo]];
  }
  return vc;
}

VertColor MaxToEggConverter::get_max_vertex_color(Mesh *mesh,int FaceNo,int VertexNo)
{
    VertColor vc(0,0,0);
    // We get the color from vcFace
    TVFace& _vcface = mesh->vcFace[FaceNo];
    // Get its index into the vertCol array
    int VertexColorIndex = _vcface.t[VertexNo];
    // Get its color
    vc =mesh->vertCol[VertexColorIndex];
    return vc;
}

Point3 MaxToEggConverter::get_max_vertex_normal(Mesh *mesh, int faceNo, int vertNo)
{
    Face f = mesh->faces[faceNo];
    DWORD smGroup = f.smGroup;
    int vert = f.getVert(vertNo);
    RVertex *rv = mesh->getRVertPtr(vert);

    int numNormals;
    Point3 vertexNormal;

    // Is normal specified SPCIFIED is not currently used, but may be used in
    // future versions.
    if (rv->rFlags & SPECIFIED_NORMAL) {
        vertexNormal = rv->rn.getNormal();
    }
    // If normal is not specified it's only available if the face belongs to a
    // smoothing group
    else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup) {
        // If there is only one vertex is found in the rn member.
        if (numNormals == 1) {
            vertexNormal = rv->rn.getNormal();
        }
        else {
            // If two or more vertices are there you need to step through them
            // and find the vertex with the same smoothing group as the
            // current face.  You will find multiple normals in the ern
            // member.
            for (int i = 0; i < numNormals; i++) {
                if (rv->ern[i].getSmGroup() & smGroup) {
                    vertexNormal = rv->ern[i].getNormal();
                }
            }
        }
    }
    else {
        // Get the normal from the Face if no smoothing groups are there
        vertexNormal = mesh->getFaceNormal(faceNo);
    }

    return vertexNormal;
}

/**
 *
 */
void MaxToEggConverter::
get_vertex_weights(INode *max_node, EggVertexPool *vpool) {
    // Try to get the weights out of a physique if one exists
    Modifier *mod = FindSkinModifier(max_node, PHYSIQUE_CLASSID);
    EggVertexPool::iterator vi;

    if (mod) {
        // create a physique export interface
        IPhysiqueExport *pPhysiqueExport = (IPhysiqueExport *)mod->GetInterface(I_PHYINTERFACE);
        if (pPhysiqueExport) {
            // create a context export interface
            IPhyContextExport *pContextExport =
                (IPhyContextExport *)pPhysiqueExport->GetContextInterface(max_node);
            if (pContextExport) {
                // set the flags in the context export interface
                pContextExport->ConvertToRigid(TRUE);
                pContextExport->AllowBlending(TRUE);

                for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
                    EggVertex *vert = (*vi);
                    int max_vi = vert->get_external_index();

                    // get the vertex export interface
                    IPhyVertexExport *pVertexExport =
                        (IPhyVertexExport *)pContextExport->GetVertexInterface(max_vi);
                    if (pVertexExport) {
                        int vertexType = pVertexExport->GetVertexType();

                        // handle the specific vertex type
                        if(vertexType == RIGID_TYPE) {
                            // typecast to rigid vertex
                            IPhyRigidVertex *pTypeVertex = (IPhyRigidVertex *)pVertexExport;
                            INode *bone_node = pTypeVertex->GetNode();
                            MaxNodeDesc *joint_node_desc = _tree.find_joint(bone_node);
                            if (joint_node_desc){
                                EggGroup *joint = _tree.get_egg_group(joint_node_desc);
                                if (joint != nullptr)
                                    joint->ref_vertex(vert, 1.0f);
                            }
                        }
                        else if(vertexType == RIGID_BLENDED_TYPE) {
                            // typecast to blended vertex
                            IPhyBlendedRigidVertex *pTypeVertex = (IPhyBlendedRigidVertex *)pVertexExport;

                            for (int ji = 0; ji < pTypeVertex->GetNumberNodes(); ++ji) {
                                PN_stdfloat weight = pTypeVertex->GetWeight(ji);
                                if (weight > 0.0f) {
                                    INode *bone_node = pTypeVertex->GetNode(ji);
                                    MaxNodeDesc *joint_node_desc = _tree.find_joint(bone_node);
                                    if (joint_node_desc){
                                        EggGroup *joint = _tree.get_egg_group(joint_node_desc);
                                        if (joint != nullptr)
                                            joint->ref_vertex(vert, weight);
                                    }
                                }
                            }
                        }
                        // Release the vertex interface
                        pContextExport->ReleaseVertexInterface(pVertexExport);
                    }
                }
                // Release the context interface
                pPhysiqueExport->ReleaseContextInterface(pContextExport);
            }
            // Release the physique export interface
            mod->ReleaseInterface(I_PHYINTERFACE, pPhysiqueExport);
        }
    }
    else {
        // No physique, try to find a skin
        mod = FindSkinModifier(max_node, SKIN_CLASSID);
        if (mod) {
            ISkin *skin = (ISkin*)mod->GetInterface(I_SKIN);
            if (skin) {
                ISkinContextData *skinMC = skin->GetContextInterface(max_node);
                if (skinMC) {
                    for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
                        EggVertex *vert = (*vi);
                        int max_vi = vert->get_external_index();

                        for (int ji = 0; ji < skinMC->GetNumAssignedBones(max_vi); ++ji) {
                            PN_stdfloat weight = skinMC->GetBoneWeight(max_vi, ji);
                            if (weight > 0.0f) {
                                INode *bone_node = skin->GetBone(skinMC->GetAssignedBone(max_vi, ji));
                                MaxNodeDesc *joint_node_desc = _tree.find_joint(bone_node);
                                if (joint_node_desc){
                                    EggGroup *joint = _tree.get_egg_group(joint_node_desc);
                                    if (joint != nullptr) {
                                        joint->ref_vertex(vert, weight);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


/**
 * Converts a Max material into a set of Panda textures and a primitive color.
 */
const MaxToEggConverter::PandaMaterial &MaxToEggConverter::
get_panda_material(Mtl *mtl, MtlID matID) {

    MaterialMap::iterator it = _material_map.find(mtl);
    if (it != _material_map.end()) {
        return (*it).second;
    }

    PandaMaterial &pandaMat = _material_map[mtl];
    pandaMat._color = LColor(1,1,1,1);
    pandaMat._any_diffuse = false;
    pandaMat._any_opacity = false;
    pandaMat._any_gloss = false;
    pandaMat._any_normal = false;




    // If it's a multi-material, dig down.

    while (( mtl != 0) && (mtl->ClassID() == Class_ID(MULTI_CLASS_ID, 0 ))) {
        if (matID < mtl->NumSubMtls()) {
            mtl = mtl->GetSubMtl(matID);
        } else {
            mtl = 0;
        }
    }

    // If it's a standard material, we're good.

    if ((mtl != 0) && (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0 ))) {
        StdMat *maxMaterial = (StdMat*)mtl;
        analyze_diffuse_maps(pandaMat, maxMaterial->GetSubTexmap(ID_DI));
        analyze_opacity_maps(pandaMat, maxMaterial->GetSubTexmap(ID_OP));
        analyze_gloss_maps(pandaMat, maxMaterial->GetSubTexmap(ID_SP));
        if (!pandaMat._any_gloss)
            analyze_gloss_maps(pandaMat, maxMaterial->GetSubTexmap(ID_SS));
        if (!pandaMat._any_gloss)
            analyze_gloss_maps(pandaMat, maxMaterial->GetSubTexmap(ID_SH));
        analyze_glow_maps(pandaMat, maxMaterial->GetSubTexmap(ID_SI));
        analyze_normal_maps(pandaMat, maxMaterial->GetSubTexmap(ID_BU));
        for (int i=0; i<pandaMat._texture_list.size(); i++) {
            EggTexture *src = pandaMat._texture_list[i];
            pandaMat._texture_list[i] =
                _textures.create_unique_texture(*src, ~EggTexture::E_tref_name);
        }

        // The existence of a texture on either color channel completely
        // replaces the corresponding flat color.
        if (!pandaMat._any_diffuse) {
            // Get the default diffuse color of the material without the
            // texture map
            Point3 diffuseColor = Point3(maxMaterial->GetDiffuse(0));
            pandaMat._color[0] = diffuseColor.x;
            pandaMat._color[1] = diffuseColor.y;
            pandaMat._color[2] = diffuseColor.z;
        }
        if (!pandaMat._any_opacity) {
            pandaMat._color[3] = (maxMaterial->GetOpacity(_current_frame * GetTicksPerFrame()));
        }
        if (pandaMat._texture_list.size() < 1) {
            // if we don't have any maps whatsoever, give the material a dummy
            // channel so that UV's get created
            pandaMat._map_channels.push_back(1);
        }
        return pandaMat;
    }

    // Otherwise, it's unrecognizable.  Leave result blank.
    return pandaMat;
}
/**
 *
 */
void MaxToEggConverter::analyze_diffuse_maps(PandaMaterial &pandaMat, Texmap *mat) {
    if (mat == 0) return;

    if (mat->ClassID() == Class_ID(RGBMULT_CLASS_ID, 0)) {
        for (int i=0; i<mat->NumSubTexmaps(); i++) {
            analyze_diffuse_maps(pandaMat, mat->GetSubTexmap(i));
        }
        return;
    }

    if (mat->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
        pandaMat._any_diffuse = true;
        PT(EggTexture) tex = new EggTexture(generate_tex_name(), "");

        BitmapTex *diffuseTex = (BitmapTex *)mat;

        Filename fullpath, outpath;
#ifdef _UNICODE
        Filename filename = Filename::from_os_specific_w(diffuseTex->GetMapName());
#else
        Filename filename = Filename::from_os_specific(diffuseTex->GetMapName());
#endif
        _options->_path_replace->full_convert_path(filename, get_model_path(),
                                                   fullpath, outpath);
        tex->set_filename(outpath);
        tex->set_fullpath(fullpath);

        apply_texture_properties(*tex, diffuseTex->GetMapChannel());
        add_map_channel(pandaMat, diffuseTex->GetMapChannel());

        Bitmap *diffuseBitmap = diffuseTex->GetBitmap(0);
        if ( diffuseBitmap && diffuseBitmap->HasAlpha()) {
            tex->set_format(EggTexture::F_rgba);
        } else {
            tex->set_format(EggTexture::F_rgb);
        }
        tex->set_env_type(EggTexture::ET_modulate);

        pandaMat._texture_list.push_back(tex);
    }
}

/**
 *
 */
void MaxToEggConverter::analyze_opacity_maps(PandaMaterial &pandaMat, Texmap *mat) {
    if (mat == 0) return;

    if (mat->ClassID() == Class_ID(RGBMULT_CLASS_ID, 0)) {
        for (int i=0; i<mat->NumSubTexmaps(); i++) {
            analyze_opacity_maps(pandaMat, mat->GetSubTexmap(i));
        }
        return;
    }

    if (mat->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
        pandaMat._any_opacity = true;
        BitmapTex *transTex = (BitmapTex *)mat;

        Filename fullpath, outpath;
#ifdef _UNICODE
        Filename filename = Filename::from_os_specific_w(transTex->GetMapName());
#else
        Filename filename = Filename::from_os_specific(transTex->GetMapName());
#endif
        _options->_path_replace->full_convert_path(filename, get_model_path(),
                                                   fullpath, outpath);

        // See if this opacity map already showed up.
        for (int i=0; i<pandaMat._texture_list.size(); i++) {
            EggTexture *tex = pandaMat._texture_list[i];
            if ((tex->get_env_type()==EggTexture::ET_modulate)&&(tex->get_fullpath() == fullpath)) {
                tex->set_format(EggTexture::F_rgba);
                return;
            }
        }

        // Try to find a diffuse map to pair this with as an alpha-texture.
        std::string uvname = get_uv_name(transTex->GetMapChannel());
        for (int i=0; i<pandaMat._texture_list.size(); i++) {
            EggTexture *tex = pandaMat._texture_list[i];
            if ((tex->get_env_type()==EggTexture::ET_modulate)&&
                (tex->get_format() == EggTexture::F_rgb)&&
                (tex->get_uv_name() == uvname)) {
                tex->set_format(EggTexture::F_rgba);
                tex->set_alpha_filename(outpath);
                tex->set_alpha_fullpath(fullpath);
                return;
            }
        }

        // Otherwise, just create it as an alpha-texture.
        PT(EggTexture) tex = new EggTexture(generate_tex_name(), "");
        tex->set_filename(outpath);
        tex->set_fullpath(fullpath);

        apply_texture_properties(*tex, transTex->GetMapChannel());
        add_map_channel(pandaMat, transTex->GetMapChannel());
        tex->set_format(EggTexture::F_alpha);

        pandaMat._texture_list.push_back(tex);
    }
}

/**
 *
 */
void MaxToEggConverter::analyze_glow_maps(PandaMaterial &pandaMat, Texmap *mat) {
    if (mat == 0) return;

    if (mat->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
        BitmapTex *gtex = (BitmapTex *)mat;

        Filename fullpath, outpath;
#ifdef _UNICODE
        Filename filename = Filename::from_os_specific_w(gtex->GetMapName());
#else
        Filename filename = Filename::from_os_specific(gtex->GetMapName());
#endif
        _options->_path_replace->full_convert_path(filename, get_model_path(),
                                                   fullpath, outpath);

        // Try to find a diffuse map to pair this with as an alpha-texture.
        std::string uvname = get_uv_name(gtex->GetMapChannel());
        for (int i=0; i<pandaMat._texture_list.size(); i++) {
            EggTexture *tex = pandaMat._texture_list[i];
            if ((tex->get_env_type()==EggTexture::ET_modulate)&&
                (tex->get_format() == EggTexture::F_rgb)&&
                (tex->get_uv_name() == uvname)) {
                tex->set_env_type(EggTexture::ET_modulate_glow);
                tex->set_format(EggTexture::F_rgba);
                tex->set_alpha_filename(outpath);
                tex->set_alpha_fullpath(fullpath);
                return;
            }
        }

        // Otherwise, just create it as a separate glow-texture.
        PT(EggTexture) tex = new EggTexture(generate_tex_name(), "");
        tex->set_env_type(EggTexture::ET_glow);
        tex->set_filename(outpath);
        tex->set_fullpath(fullpath);
        apply_texture_properties(*tex, gtex->GetMapChannel());
        add_map_channel(pandaMat, gtex->GetMapChannel());
        tex->set_format(EggTexture::F_alpha);

        pandaMat._texture_list.push_back(tex);
    }
}

/**
 *
 */
void MaxToEggConverter::analyze_gloss_maps(PandaMaterial &pandaMat, Texmap *mat) {
    if (mat == 0) return;

    if (mat->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
        pandaMat._any_gloss = true;
        BitmapTex *gtex = (BitmapTex *)mat;

        Filename fullpath, outpath;
#ifdef _UNICODE
        Filename filename = Filename::from_os_specific_w(gtex->GetMapName());
#else
        Filename filename = Filename::from_os_specific(gtex->GetMapName());
#endif
        _options->_path_replace->full_convert_path(filename, get_model_path(),
                                                   fullpath, outpath);

        // Try to find a diffuse map to pair this with as an alpha-texture.
        std::string uvname = get_uv_name(gtex->GetMapChannel());
        for (int i=0; i<pandaMat._texture_list.size(); i++) {
            EggTexture *tex = pandaMat._texture_list[i];
            if ((tex->get_env_type()==EggTexture::ET_modulate)&&
                (tex->get_format() == EggTexture::F_rgb)&&
                (tex->get_uv_name() == uvname)) {
                tex->set_env_type(EggTexture::ET_modulate_gloss);
                tex->set_format(EggTexture::F_rgba);
                tex->set_alpha_filename(outpath);
                tex->set_alpha_fullpath(fullpath);
                return;
            }
        }

        // Otherwise, just create it as a separate gloss-texture.
        PT(EggTexture) tex = new EggTexture(generate_tex_name(), "");
        tex->set_env_type(EggTexture::ET_gloss);
        tex->set_filename(outpath);
        tex->set_fullpath(fullpath);
        apply_texture_properties(*tex, gtex->GetMapChannel());
        add_map_channel(pandaMat, gtex->GetMapChannel());
        tex->set_format(EggTexture::F_alpha);

        pandaMat._texture_list.push_back(tex);
    }
}

/**
 *
 */
void MaxToEggConverter::analyze_normal_maps(PandaMaterial &pandaMat, Texmap *mat) {
    if (mat == 0) return;

    if (mat->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
        pandaMat._any_normal = true;
        BitmapTex *ntex = (BitmapTex *)mat;

        Filename fullpath, outpath;
#ifdef _UNICODE
        Filename filename = Filename::from_os_specific_w(ntex->GetMapName());
#else
        Filename filename = Filename::from_os_specific(ntex->GetMapName());
#endif
        _options->_path_replace->full_convert_path(filename, get_model_path(),
                                                   fullpath, outpath);

        PT(EggTexture) tex = new EggTexture(generate_tex_name(), "");
        tex->set_env_type(EggTexture::ET_normal);
        tex->set_filename(outpath);
        tex->set_fullpath(fullpath);
        apply_texture_properties(*tex, ntex->GetMapChannel());
        add_map_channel(pandaMat, ntex->GetMapChannel());
        tex->set_format(EggTexture::F_rgb);

        pandaMat._texture_list.push_back(tex);
    }
}

/**
 * Adds the specified map channel to the map channel list, if it's not already
 * there.
 */
void MaxToEggConverter::add_map_channel(PandaMaterial &pandaMat, int chan) {
    for (int i=0; i<pandaMat._map_channels.size(); i++) {
        if (pandaMat._map_channels[i] == chan) {
            return;
        }
    }
    pandaMat._map_channels.push_back(chan);
}

/**
 * Generates an arbitrary unused texture name.
 */
std::string MaxToEggConverter::generate_tex_name() {
    std::ostringstream name_strm;
    name_strm << "Tex" << ++_cur_tref;
    return name_strm.str();
}

/**
 * Returns the UV-name of the nth map-channel.
 */
std::string MaxToEggConverter::get_uv_name(int channel) {
    std::ostringstream uvname;
    uvname << "m" << channel;
    return uvname.str();
}

/**
 * Applies all the appropriate texture properties to the EggTexture object,
 * including wrap modes and texture matrix.
 */
void MaxToEggConverter::
apply_texture_properties(EggTexture &tex, int channel) {

    // we leave a channel 1 for texture swapping, so don't name it
    if(channel == 1)
      tex.set_uv_name("");
    else
      tex.set_uv_name(get_uv_name(channel));

    tex.set_minfilter(EggTexture::FT_linear_mipmap_linear);
    tex.set_magfilter(EggTexture::FT_linear);

    EggTexture::WrapMode wrap_u = EggTexture::WM_repeat;
    EggTexture::WrapMode wrap_v = EggTexture::WM_repeat;

    tex.set_wrap_u(wrap_u);
    tex.set_wrap_v(wrap_v);
}


/**
 * Recursively walks the egg hierarchy, reparenting "decal" type nodes below
 * their corresponding "decalbase" type nodes, and setting the flags.
 *
 * Returns true on success, false if some nodes were incorrect.
 */
bool MaxToEggConverter::
reparent_decals(EggGroupNode *egg_parent) {
    bool okflag = true;

    // First, walk through all children of this node, looking for the one
    // decal base, if any.
    EggGroup *decal_base = nullptr;
    pvector<EggGroup *> decal_children;

    EggGroupNode::iterator ci;
    for (ci = egg_parent->begin(); ci != egg_parent->end(); ++ci) {
        EggNode *child =  (*ci);
        if (child->is_of_type(EggGroup::get_class_type())) {
            EggGroup *child_group = (EggGroup *) child;
            if (child_group->has_object_type("decalbase")) {
                if (decal_base != nullptr) {
                    // error
                    okflag = false;
                }
                child_group->remove_object_type("decalbase");
                decal_base = child_group;

            } else if (child_group->has_object_type("decal")) {
                child_group->remove_object_type("decal");
                decal_children.push_back(child_group);
            }
        }
    }

    if (decal_base == nullptr) {
        if (!decal_children.empty()) {
            // warning
        }

    } else {
        if (decal_children.empty()) {
            // warning

        } else {
            // All the decal children get moved to be a child of decal base.
            // This usually will not affect the vertex positions, but it could
            // if the decal base has a transform and the decal child is an
            // instance node.  So don't do that.
            pvector<EggGroup *>::iterator di;
            for (di = decal_children.begin(); di != decal_children.end(); ++di) {
                EggGroup *child_group = (*di);
                decal_base->add_child(child_group);
            }

            // Also set the decal state on the base.
            decal_base->set_decal_flag(true);
        }
    }

    // Now recurse on each of the child nodes.
    for (ci = egg_parent->begin(); ci != egg_parent->end(); ++ci) {
        EggNode *child =  (*ci);
        if (child->is_of_type(EggGroupNode::get_class_type())) {
            EggGroupNode *child_group = (EggGroupNode *) child;
            if (!reparent_decals(child_group)) {
                okflag = false;
            }
        }
    }

    return okflag;
}

Modifier* MaxToEggConverter::FindSkinModifier (INode* node, const Class_ID &type)
{
    // Get object from node.  Abort if no object.
    Object* pObj = node->GetObjectRef();
    if (!pObj) return nullptr;

    // Is derived object ?
    while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
        // Yes -> Cast.
        IDerivedObject* pDerObj = static_cast<IDerivedObject*>(pObj);

        // Iterate over all entries of the modifier stack.
        for (int stackId = 0; stackId < pDerObj->NumModifiers(); ++stackId) {
            // Get current modifier.
            Modifier* mod = pDerObj->GetModifier(stackId);

            // Is this what we are looking for?
            if (mod->ClassID() == type )
                return mod;
        }

        // continue with next derived object
        pObj = pDerObj->GetObjRef();
    }

    // Not found.
    return nullptr;
}
