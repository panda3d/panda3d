// Filename: test_builder.cxx
// Created by:  drose (09Sep97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "builder.h"
#include "builderPrim.h"
#include "mesher.h"

#include "notify.h"
#include "geom.h"
#include "geomNode.h"
#include "namedNode.h"
#include "dftraverser.h"
#include "traverserVisitor.h"
#include "renderRelation.h"
#include "nullTransitionWrapper.h"
#include "nullLevelState.h"
#include "pta_Vertexf.h"
#include "pta_Normalf.h"
#include "pta_Colorf.h"
#include "pta_TexCoordf.h"

class polygon : public BuilderPrimI {
public:
};

extern int hedface_vt_len;
extern Vertexf hedface_vt[];
extern polygon polys[];
extern int num_polys;


class ReportGeoms :
  public TraverserVisitor<NullTransitionWrapper, NullLevelState> {
public:
  bool reached_node(Node *node, const NullTransitionWrapper &, NullLevelState &);
};

bool ReportGeoms::
reached_node(Node *node, const NullTransitionWrapper &, NullLevelState &) {
  if (node->is_of_type(GeomNode::get_class_type())) {
    GeomNode *geomNode = (GeomNode *)node;
    nout << "\n" << *geomNode << ", " << geomNode->get_num_geoms()
         << " geoms:\n";
    int num_geoms = geomNode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      dDrawable *draw = geomNode->get_geom(i);
      Geom *geom = DCAST(Geom, draw);
      nout << *geom << ":\n";
      geom->output_verbose(nout);
    }

  } else {
    nout << *node << "\n";
  }
  return true;
}

int
main(void) {

#if 1
  NamedNode *root = new NamedNode("root");

  Builder builder;

  PTA_Vertexf v_array(hedface_vt_len);
  int i;
  for (i = 0; i < hedface_vt_len; i++) {
    v_array[i] = hedface_vt[i];
  }
  PTA_Colorf c_array(num_polys);
  for (i = 0; i < num_polys; i++) {
    c_array[i].set(1.0, 1.0, 1.0, 1.0);
  }

  BuilderBucket bucket;
  bucket._node = root;
  bucket.set_coords(v_array);
  bucket.set_colors(c_array);
  bucket._mesh = true;

  for (i = 0; i < num_polys; i++) {
    polys[i].set_color(i);
    //    nout << "Adding polygon " << i << ": " << polys[i] << "\n";
    builder.add_prim_nonindexed(bucket, polys[i]);
  }

  ReportGeoms rg;

  GeomNode *gn = builder.build();
  nout << "Built " << (void *)gn << "\n";
  if (gn != NULL) {
    NullLevelState level_state;
    rg.reached_node(gn, NullTransitionWrapper(), level_state);
  }

  nout << "\nTraversing root:\n";
  df_traverse(root, rg, NullTransitionWrapper(), NullLevelState(),
              RenderRelation::get_class_type());

#else
  Builder builder;
  BuilderPrimI prim;

  BuilderBucket bucket;

  ushort p = 0;
  int xi, yi;
  xi = 4;
  yi = xi;

  int x, y;
  PTA_Vertexf coords(xi * yi);
  PTA_Normalf normals(xi * yi);
  PTA_Colorf colors(3);

  double xv, yv, zv;
  for (y = 0; y < yi; y++) {
    for (x = 0; x < xi; x++) {
      xv = (double)x / (double)(xi-1) - 0.5;
      yv = (double)y / (double)(yi-1) - 0.5;

      //zv = (y <= x) ? 0.0 : ((float)(y-x)*(y-x) / (float)(yi));
      //zv = sqrt(1.0 - (xv*xv + yv*yv));
      //zv = sqrt(1.0 - (xv*xv + yv*yv));
      //zv = sqrt(max(0.25 - (xv*xv + yv*yv), 0.0));
      zv = 0.0;

      p = y*xi + x;
      coords[p].set(xv, yv, zv);
    }
  }

  for (y = 0; y < yi-1; y++) {
    for (x = 0; x < xi-1; x++) {
      p = y*xi + x;
      pfVec3 p1 = coords[p+xi];
      pfVec3 p2 = coords[p];
      pfVec3 p3 = coords[p+1];
      normals[p] = normalize(cross(p1-p2, p2-p3));
    }
  }

  colors[0].set(1.0, 1.0, 1.0, 1.0);
  colors[1].set(1.0, 0.0, 0.0, 1.0);
  colors[2].set(0.0, 0.0, 1.0, 1.0);

  bucket.set_coords(coords);
  bucket.set_normals(normals);
  bucket.set_colors(colors);

  bucket.setAttr(PFSTATE_FRONTMTL, new pfMaterial);
  bucket.setMode(PFSTATE_ENLIGHTING, PF_ON);

  bucket._mesh = true;
  bucket._max_tfan_angle = 60;
  bucket._show_tstrips = true;
  bucket._retesselate_coplanar = true;
  builder.add_bucket(bucket);

  nout << "Adding polygons.\n";
  for (y = 0; y < yi-1; y++) {
    for (x = 0; x < xi-1; x++) {
      p = y*xi + x;
      BuilderVertexI bv0(p), bv1(p+1), bv4(p+xi), bv5(p+xi+1);
      if (p==0) {
        builder.add_prim(BuilderPrimI(bv0, bv5, bv4).set_color(1));
        builder.add_prim(BuilderPrimI(bv1, bv5, bv0).set_color(1));
      } else if (p==5) {
        builder.add_prim(BuilderPrimI(bv0, bv5, bv4).set_color(1));
        builder.add_prim(BuilderPrimI(bv1, bv5, bv0).set_color(1));
      } else if (p==1 || p==4) {
        builder.add_prim(BuilderPrimI(bv0, bv1, bv4).set_color(1));
        builder.add_prim(BuilderPrimI(bv1, bv5, bv4).set_color(1));
      } else {
        builder.add_prim(BuilderPrimI(bv0, bv5, bv4).set_color(0));
        builder.add_prim(BuilderPrimI(bv1, bv5, bv0).set_color(0));
      }

    }
  }

  nout << "Building.\n";
  pfNode *root = builder.build_all_buckets();

  if (root!=NULL) {
    report_geosets(root);
  }

  pfdStoreFile(root, "builder.pfa");

#endif

  return 0;
}

