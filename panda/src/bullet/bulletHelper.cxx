/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletHelper.cxx
 * @author enn0x
 * @date 2011-01-19
 */

#include "bulletHelper.h"

#include "bulletRigidBodyNode.h"
#include "bulletSoftBodyNode.h"
#include "bulletGhostNode.h"

#include "geomLines.h"
#include "geomTriangles.h"
#include "geomVertexRewriter.h"

#include "bullet_utils.h"

PT(InternalName) BulletHelper::_sb_index;
PT(InternalName) BulletHelper::_sb_flip;

/**
 *
 */
NodePathCollection BulletHelper::
from_collision_solids(NodePath &np, bool clear) {

  NodePathCollection result;

  // Iterate over all CollisionNodes below the given node
  NodePathCollection npc = np.find_all_matches( "**/+CollisionNode" );

  for (int i=0; i<npc.get_num_paths(); i++) {
    NodePath cnp = npc.get_path(i);
    CollisionNode *cnode = DCAST(CollisionNode, cnp.node());

    PT(PandaNode) bnode = nullptr;

    // Create a either a new rigid body or a new ghost for each CollisionNode,
    // and add one shape per CollisionSolid contained in the CollisionNode
    if (is_tangible(cnode)) {
      PT(BulletRigidBodyNode) body;

      body = new BulletRigidBodyNode();
      body->add_shapes_from_collision_solids(cnode);
      body->set_transform(cnp.get_transform(np));
      body->set_into_collide_mask(cnode->get_into_collide_mask());
      body->set_mass(0.0f);
      body->set_name(cnode->get_name());

      bnode = body;
    }
    else {
      PT(BulletGhostNode) ghost;

      ghost = new BulletGhostNode();
      ghost->add_shapes_from_collision_solids(cnode);
      ghost->set_transform(cnp.get_transform(np));
      ghost->set_into_collide_mask(cnode->get_into_collide_mask());
      ghost->set_name(cnode->get_name());

      bnode = ghost;
    }

    // Remove collision node if requested
    if (clear) {
      cnp.remove_node();
    }

    result.add_path(NodePath::any_path(bnode));
  }

  return result;
}

/**
 * Returns TRUE if at least one CollisionSolid of the given CollisionNode is
 * tangible.  Returns FALSE if all CollisionSolids are intangible.
 */
bool BulletHelper::
is_tangible(CollisionNode *cnode) {

  for (size_t j = 0; j < cnode->get_num_solids(); ++j) {
    CPT(CollisionSolid) solid = cnode->get_solid(j);
    if (solid->is_tangible()) {
        return true;
    }
  }

  return false;
}

/**
 *
 */
CPT(GeomVertexFormat) BulletHelper::
add_sb_index_column(const GeomVertexFormat *format) {

  PT(InternalName) name = BulletHelper::get_sb_index();

  if (format->has_column(name)) return format;

  PT(GeomVertexArrayFormat) array;
  PT(GeomVertexFormat) unregistered_format;
  CPT(GeomVertexFormat) registered_format;

  array = new GeomVertexArrayFormat();
  array->add_column(name, 1, Geom::NT_uint16, Geom::C_index);

  unregistered_format = new GeomVertexFormat(*format);
  unregistered_format->add_array(array);

  registered_format = GeomVertexFormat::register_format(unregistered_format);

  return registered_format;
}

/**
 *
 */
CPT(GeomVertexFormat) BulletHelper::
add_sb_flip_column(const GeomVertexFormat *format) {

  PT(InternalName) name = BulletHelper::get_sb_flip();

  if (format->has_column(name)) return format;

  PT(GeomVertexArrayFormat) array;
  PT(GeomVertexFormat) unregistered_format;
  CPT(GeomVertexFormat) registered_format;

  array = new GeomVertexArrayFormat();
  array->add_column(name, 1, Geom::NT_uint8, Geom::C_other);

  unregistered_format = new GeomVertexFormat(*format);
  unregistered_format->add_array(array);

  registered_format = GeomVertexFormat::register_format(unregistered_format);

  return registered_format;
}

/**
 *
 */
PT(Geom) BulletHelper::
make_geom_from_faces(BulletSoftBodyNode *node, const GeomVertexFormat *format, bool two_sided) {

  return make_geom(node, format, two_sided, true);
}

/**
 *
 */
PT(Geom) BulletHelper::
make_geom_from_links(BulletSoftBodyNode *node, const GeomVertexFormat *format) {

  return make_geom(node, format, false, false);
}

/**
 *
 */
PT(Geom) BulletHelper::
make_geom(BulletSoftBodyNode *node, const GeomVertexFormat *format, bool two_sided, bool use_faces) {

  btTransform trans = btTransform::getIdentity();
  get_node_transform(trans, node);

  btSoftBody *body = (btSoftBody *)node->get_object();

  PT(Geom) geom;
  PT(GeomPrimitive) prim;
  PT(GeomVertexData) vdata;

  CPT(GeomVertexFormat) fmt = (format) ? format : GeomVertexFormat::get_v3n3t2();
  fmt = BulletHelper::add_sb_flip_column(fmt);

  nassertr(fmt->has_column(InternalName::get_vertex()), nullptr);
  nassertr(fmt->has_column(InternalName::get_normal()), nullptr);

  btSoftBody::tNodeArray &nodes(body->m_nodes);

  // Vertex data
  vdata = new GeomVertexData("", fmt, Geom::UH_stream);

  GeomVertexWriter vwriter(vdata, InternalName::get_vertex());
  GeomVertexWriter nwriter(vdata, InternalName::get_normal());
  GeomVertexWriter fwriter(vdata, BulletHelper::get_sb_flip());

  for (int j=0; j<nodes.size(); ++j) {
    btVector3 v = nodes[j].m_x;
    btVector3 &n = nodes[j].m_n;

    v = trans.invXform(v);

    vwriter.add_data3((PN_stdfloat)v.getX(), (PN_stdfloat)v.getY(), (PN_stdfloat)v.getZ());
    nwriter.add_data3((PN_stdfloat)n.getX(), (PN_stdfloat)n.getY(), (PN_stdfloat)n.getZ());
    fwriter.add_data1i(0);
  }

  if (two_sided) {
    for (int j=0; j<nodes.size(); ++j) {
      btVector3 v = nodes[j].m_x;
      btVector3 &n = nodes[j].m_n;

      v = trans.invXform(v);

      vwriter.add_data3((PN_stdfloat)v.getX(), (PN_stdfloat)v.getY(), (PN_stdfloat)v.getZ());
      nwriter.add_data3((PN_stdfloat)n.getX(), (PN_stdfloat)n.getY(), (PN_stdfloat)n.getZ());
      fwriter.add_data1i(1);
    }
  }

  // Indices
  btSoftBody::Node *node0 = &nodes[0];
  int i0, i1, i2;

  if (use_faces) {

    btSoftBody::tFaceArray &faces(body->m_faces);

    prim = new GeomTriangles(Geom::UH_stream);
    prim->set_shade_model(Geom::SM_uniform);

    for (int j=0; j<faces.size(); ++j) {
      i0 = int(faces[j].m_n[0] - node0);
      i1 = int(faces[j].m_n[1] - node0);
      i2 = int(faces[j].m_n[2] - node0);

      prim->add_vertices(i0, i1, i2);
      prim->close_primitive();

      if (two_sided) {
        i0 = nodes.size() + int(faces[j].m_n[0] - node0);
        i1 = nodes.size() + int(faces[j].m_n[2] - node0);
        i2 = nodes.size() + int(faces[j].m_n[1] - node0);

        prim->add_vertices(i0, i1, i2);
        prim->close_primitive();
      }
    }
  }
  else {
    btSoftBody::tLinkArray &links(body->m_links);

    prim = new GeomLines(Geom::UH_stream);
    prim->set_shade_model(Geom::SM_uniform);

    for (int j=0; j<links.size(); ++j) {
      i0 = int(links[j].m_n[0] - node0);
      i1 = int(links[j].m_n[1] - node0);

      prim->add_vertices(i0, i1);
      prim->close_primitive();
    }
  }

  // Geom
  geom = new Geom(vdata);
  geom->add_primitive(prim);

  return geom;
}

/**
 *
 */
void BulletHelper::
make_texcoords_for_patch(Geom *geom, int resx, int resy) {

  PT(GeomVertexData) vdata = geom->modify_vertex_data();

  nassertv(vdata->has_column(InternalName::get_texcoord()));

  GeomVertexRewriter texcoords(vdata, InternalName::get_texcoord());

  int n = resx * resy;
  int i = 0;
  int ix;
  int iy;
  float u;
  float v;

  while (!texcoords.is_at_end()) {
    ix = i / resx;
    iy = i % resy;

    if (i > n) ix -= 1;

    u = (float)ix/(float)(resx - 1);
    v = (float)iy/(float)(resy - 1);

    texcoords.set_data2f(u, v);
    i++;
  }
}
