// Filename: mayaToEggConverter.cxx
// Created by:  drose (10Nov99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "mayaToEggConverter.h"
#include "mayaShader.h"
#include "maya_funcs.h"
#include "config_mayaegg.h"

#include "eggData.h"
#include "eggGroup.h"
#include "eggVertex.h"
#include "eggVertexPool.h"
#include "eggNurbsSurface.h"
#include "eggNurbsCurve.h"
#include "eggPolygon.h"
#include "eggPrimitive.h"
#include "eggTexture.h"
#include "eggTextureCollection.h"
#include "string_utils.h"

#include "pre_maya_include.h"
#include <maya/MArgList.h>
#include <maya/MColor.h>
#include <maya/MDagPath.h>
#include <maya/MFnCamera.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnLight.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnPlugin.h>
#include <maya/MItDag.h>
#include <maya/MLibrary.h>
#include <maya/MMatrix.h>
#include <maya/MObject.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>
#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MVector.h>
#include <maya/MTesselationParams.h>
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaToEggConverter::
MayaToEggConverter(const string &program_name) :
  _shaders(this),
  _program_name(program_name)
{
  _polygon_output = false;
  _polygon_tolerance = 0.01;
  _ignore_transforms = false;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaToEggConverter::
MayaToEggConverter(const MayaToEggConverter &copy) :
  _shaders(this),
  _maya(copy._maya)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MayaToEggConverter::
~MayaToEggConverter() {
  close_api();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the converter.
////////////////////////////////////////////////////////////////////
SomethingToEggConverter *MayaToEggConverter::
make_copy() {
  return new MayaToEggConverter(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::get_name
//       Access: Public, Virtual
//  Description: Returns the English name of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string MayaToEggConverter::
get_name() const {
  return "Maya";
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::get_extension
//       Access: Public, Virtual
//  Description: Returns the common extension of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string MayaToEggConverter::
get_extension() const {
  return "mb";
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::convert_file
//       Access: Public, Virtual
//  Description: Handles the reading of the input file and converting
//               it to egg.  Returns true if successful, false
//               otherwise.
//
//               This is designed to be as generic as possible,
//               generally in support of run-time loading.
//               Command-line converters may choose to use
//               convert_maya() instead, as it provides more control.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
convert_file(const Filename &filename) {
  if (!open_api()) {
    mayaegg_cat.error()
      << "Maya is not available.\n";
    return false;
  }
  if (!_maya->read(filename)) {
    mayaegg_cat.error()
      << "Unable to read " << filename << "\n";
    return false;
  }
  return convert_maya();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::convert_maya
//       Access: Public
//  Description: Fills up the egg_data structure according to the
//               global maya model data.  Returns true if successful,
//               false if there is an error.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
convert_maya() {
  _textures.clear();
  _shaders.clear();
  _groups.clear();

  if (!open_api()) {
    mayaegg_cat.error()
      << "Maya is not available.\n";
    return false;
  }

  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(_maya->get_coordinate_system());
  }

  MStatus status;

  MItDag dag_iterator(MItDag::kDepthFirst, MFn::kTransform, &status);
  if (!status) {
    status.perror("MItDag constructor");
    return false;
  }

  mayaegg_cat.info()
    << "Converting from Maya.\n";

  // This while loop walks through the entire Maya hierarchy, one node
  // at a time.  Maya's MItDag object automatically performs a
  // depth-first traversal of its scene graph.
  bool all_ok = true;
  while (!dag_iterator.isDone()) {
    MDagPath dag_path;
    status = dag_iterator.getPath(dag_path);
    if (!status) {
      status.perror("MItDag::getPath");
    } else {
      if (!process_node(dag_path, get_egg_data())) {
        all_ok = false;
      }
    }

    dag_iterator.next();
  }

  if (all_ok) {
    mayaegg_cat.info()
      << "Converted, no errors.\n";
  } else {
    mayaegg_cat.info()
      << "Errors encountered in conversion.\n";
  }

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::open_api
//       Access: Public
//  Description: Attempts to open the Maya API if it was not already
//               open, and returns true if successful, or false if
//               there is an error.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
open_api() {
  if (_maya == (MayaApi *)NULL || !_maya->is_valid()) {
    _maya = MayaApi::open_api(_program_name);
  }
  return _maya->is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::close_api
//       Access: Public
//  Description: Closes the Maya API, if it was previously opened.
//               Caution!  Maya appears to call exit() when its API is
//               closed.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
close_api() {
  // We have to clear the shaders before we release the Maya API.
  _shaders.clear();
  _maya.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::process_node
//       Access: Private
//  Description: Converts the indicated Maya node (given a MDagPath,
//               similar in concept to Panda's NodePath) to the
//               corresponding Egg structure.  Returns true if
//               successful, false if an error was encountered.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
process_node(const MDagPath &dag_path, EggData &data) {
  MStatus status;
  MFnDagNode dag_node(dag_path, &status);
  if (!status) {
    status.perror("MFnDagNode constructor");
    return false;
  }

  if (mayaegg_cat.is_debug()) {
    mayaegg_cat.debug()
      << dag_node.name() << ": " << dag_node.typeName() << "\n"
      << "  dag_path: " << dag_path.fullPathName() << "\n";
  }

  if (dag_path.hasFn(MFn::kCamera)) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring camera node " << dag_path.fullPathName() << "\n";
    }

  } else if (dag_path.hasFn(MFn::kLight)) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring light node " << dag_path.fullPathName() << "\n";
    }

  } else if (dag_path.hasFn(MFn::kNurbsSurface)) {
    EggGroup *egg_group =
      get_egg_group(dag_path.fullPathName().asChar(), data);

    if (egg_group == (EggGroup *)NULL) {
      mayaegg_cat.error()
        << "Cannot determine group node.\n";
      return false;

    } else {
      get_transform(dag_path, egg_group);

      MFnNurbsSurface surface(dag_path, &status);
      if (!status) {
        mayaegg_cat.info()
          << "Error in node " << dag_path.fullPathName() << ":\n"
          << "  it appears to have a NURBS surface, but does not.\n";
      } else {
        make_nurbs_surface(dag_path, surface, egg_group);
      }
    }

  } else if (dag_path.hasFn(MFn::kNurbsCurve)) {
    EggGroup *egg_group =
      get_egg_group(dag_path.fullPathName().asChar(), data);

    if (egg_group == (EggGroup *)NULL) {
      nout << "Cannot determine group node.\n";

    } else {
      get_transform(dag_path, egg_group);

      MFnNurbsCurve curve(dag_path, &status);
      if (!status) {
        mayaegg_cat.info()
          << "Error in node " << dag_path.fullPathName() << ":\n"
          << "  it appears to have a NURBS curve, but does not.\n";
      } else {
        make_nurbs_curve(dag_path, curve, egg_group);
      }
    }

  } else if (dag_path.hasFn(MFn::kMesh)) {
    EggGroup *egg_group =
      get_egg_group(dag_path.fullPathName().asChar(), data);

    if (egg_group == (EggGroup *)NULL) {
      mayaegg_cat.error()
        << "Cannot determine group node.\n";
      return false;

    } else {
      get_transform(dag_path, egg_group);

      MFnMesh mesh(dag_path, &status);
      if (!status) {
        mayaegg_cat.info()
          << "Error in node " << dag_path.fullPathName() << ":\n"
          << "  it appears to have a polygon mesh, but does not.\n";
      } else {
        make_polyset(dag_path, mesh, egg_group);
      }
    }

  } else {
    // Get the translation/rotation/scale data
    EggGroup *egg_group =
      get_egg_group(dag_path.fullPathName().asChar(), data);

    if (egg_group != (EggGroup *)NULL) {
      get_transform(dag_path, egg_group);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::get_transform
//       Access: Private
//  Description: Extracts the transform on the indicated Maya node,
//               and applies it to the corresponding Egg node.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
get_transform(const MDagPath &dag_path, EggGroup *egg_group) {
  if (_ignore_transforms) {
    return;
  }

  MStatus status;
  MObject transformNode = dag_path.transform(&status);
  // This node has no transform - i.e., it's the world node
  if (!status && status.statusCode() == MStatus::kInvalidParameter) {
    return;
  }

  MFnDagNode transform(transformNode, &status);
  if (!status) {
    status.perror("MFnDagNode constructor");
    return;
  }

  MTransformationMatrix matrix(transform.transformationMatrix());

  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam()
      << "  translation: " << matrix.translation(MSpace::kWorld)
      << "\n";
    double d[3];
    MTransformationMatrix::RotationOrder rOrder;

    matrix.getRotation(d, rOrder, MSpace::kWorld);
    mayaegg_cat.spam()
      << "  rotation: ["
      << d[0] << ", "
      << d[1] << ", "
      << d[2] << "]\n";
    matrix.getScale(d, MSpace::kWorld);
    mayaegg_cat.spam()
      << "  scale: ["
      << d[0] << ", "
      << d[1] << ", "
      << d[2] << "]\n";
  }

  MMatrix mat = matrix.asMatrix();
  MMatrix ident_mat;
  ident_mat.setToIdentity();

  if (!mat.isEquivalent(ident_mat, 0.0001)) {
    egg_group->set_transform
      (LMatrix4d(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                 mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                 mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                 mat[3][0], mat[3][1], mat[3][2], mat[3][3]));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_nurbs_surface
//       Access: Private
//  Description: Converts the indicated Maya NURBS surface to a
//               corresponding egg structure, and attaches it to the
//               indicated egg group.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
make_nurbs_surface(const MDagPath &dag_path, MFnNurbsSurface &surface,
                   EggGroup *egg_group) {
  MStatus status;
  string name = surface.name().asChar();

  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam()
      << "  numCVs: "
      << surface.numCVsInU()
      << " * "
      << surface.numCVsInV()
      << "\n";
    mayaegg_cat.spam()
      << "  numKnots: "
      << surface.numKnotsInU()
      << " * "
      << surface.numKnotsInV()
      << "\n";
    mayaegg_cat.spam()
      << "  numSpans: "
      << surface.numSpansInU()
      << " * "
      << surface.numSpansInV()
      << "\n";
  }

  MayaShader *shader = _shaders.find_shader_for_node(surface.object());

  if (_polygon_output) {
    // If we want polygon output only, tesselate the NURBS and output
    // that.
    MTesselationParams params;
    params.setFormatType(MTesselationParams::kStandardFitFormat);
    params.setOutputType(MTesselationParams::kQuads);
    params.setStdFractionalTolerance(_polygon_tolerance);

    // We'll create the tesselation as a sibling of the NURBS surface.
    // That way we inherit all of the transformations.
    MDagPath polyset_path = dag_path;
    MObject polyset_parent = polyset_path.node();
    MObject polyset =
      surface.tesselate(params, polyset_parent, &status);
    if (!status) {
      status.perror("MFnNurbsSurface::tesselate");
      return;
    }

    status = polyset_path.push(polyset);
    if (!status) {
      status.perror("MDagPath::push");
    }

    MFnMesh polyset_fn(polyset, &status);
    if (!status) {
      status.perror("MFnMesh constructor");
      return;
    }
    make_polyset(polyset_path, polyset_fn, egg_group, shader);

    return;
  }

  MPointArray cv_array;
  status = surface.getCVs(cv_array, MSpace::kWorld);
  if (!status) {
    status.perror("MFnNurbsSurface::getCVs");
    return;
  }
  MDoubleArray u_knot_array, v_knot_array;
  status = surface.getKnotsInU(u_knot_array);
  if (!status) {
    status.perror("MFnNurbsSurface::getKnotsInU");
    return;
  }
  status = surface.getKnotsInV(v_knot_array);
  if (!status) {
    status.perror("MFnNurbsSurface::getKnotsInV");
    return;
  }

  /*
    We don't use these variables currently.
  MFnNurbsSurface::Form u_form = surface.formInU();
  MFnNurbsSurface::Form v_form = surface.formInV();
  */

  int u_degree = surface.degreeU();
  int v_degree = surface.degreeV();

  int u_cvs = surface.numCVsInU();
  int v_cvs = surface.numCVsInV();

  int u_knots = surface.numKnotsInU();
  int v_knots = surface.numKnotsInV();

  assert(u_knots == u_cvs + u_degree - 1);
  assert(v_knots == v_cvs + v_degree - 1);

  string vpool_name = name + ".cvs";
  EggVertexPool *vpool = new EggVertexPool(vpool_name);
  egg_group->add_child(vpool);

  EggNurbsSurface *egg_nurbs = new EggNurbsSurface(name);
  egg_nurbs->setup(u_degree + 1, v_degree + 1,
                   u_knots + 2, v_knots + 2);

  int i;

  egg_nurbs->set_u_knot(0, u_knot_array[0]);
  for (i = 0; i < u_knots; i++) {
    egg_nurbs->set_u_knot(i + 1, u_knot_array[i]);
  }
  egg_nurbs->set_u_knot(u_knots + 1, u_knot_array[u_knots - 1]);

  egg_nurbs->set_v_knot(0, v_knot_array[0]);
  for (i = 0; i < v_knots; i++) {
    egg_nurbs->set_v_knot(i + 1, v_knot_array[i]);
  }
  egg_nurbs->set_v_knot(v_knots + 1, v_knot_array[v_knots - 1]);

  for (i = 0; i < egg_nurbs->get_num_cvs(); i++) {
    int ui = egg_nurbs->get_u_index(i);
    int vi = egg_nurbs->get_v_index(i);

    double v[4];
    MStatus status = cv_array[v_cvs * ui + vi].get(v);
    if (!status) {
      status.perror("MPoint::get");
    } else {
      EggVertex vert;
      vert.set_pos(LPoint4d(v[0], v[1], v[2], v[3]));
      egg_nurbs->add_vertex(vpool->create_unique_vertex(vert));
    }
  }

  // Now consider the trim curves, if any.
  unsigned num_trims = surface.numRegions();
  int trim_curve_index = 0;
  for (unsigned ti = 0; ti < num_trims; ti++) {
    unsigned num_loops = surface.numBoundaries(ti);

    if (num_loops > 0) {
      egg_nurbs->_trims.push_back(EggNurbsSurface::Trim());
      EggNurbsSurface::Trim &egg_trim = egg_nurbs->_trims.back();

      for (unsigned li = 0; li < num_loops; li++) {
        egg_trim.push_back(EggNurbsSurface::Loop());
        EggNurbsSurface::Loop &egg_loop = egg_trim.back();
        
        MFnNurbsSurface::BoundaryType type =
          surface.boundaryType(ti, li, &status);
        bool keep_loop = false;
        
        if (!status) {
          status.perror("MFnNurbsSurface::BoundaryType");
        } else {
          keep_loop = (type == MFnNurbsSurface::kInner ||
                       type == MFnNurbsSurface::kOuter);
        }
        
        if (keep_loop) {
          unsigned num_edges = surface.numEdges(ti, li);
          for (unsigned ei = 0; ei < num_edges; ei++) {
            MObjectArray edge = surface.edge(ti, li, ei, true, &status);
            if (!status) {
              status.perror("MFnNurbsSurface::edge");
            } else {
              unsigned num_segs = edge.length();
              for (unsigned si = 0; si < num_segs; si++) {
                MObject segment = edge[si];
                if (segment.hasFn(MFn::kNurbsCurve)) {
                  MFnNurbsCurve curve(segment, &status);
                  if (!status) {
                    nout << "Trim curve appears to be a nurbs curve, but isn't.\n";
                  } else {
                    // Finally, we have a valid curve!
                    EggNurbsCurve *egg_curve =
                      make_trim_curve(curve, name, egg_group, trim_curve_index);
                    trim_curve_index++;
                    if (egg_curve != (EggNurbsCurve *)NULL) {
                      egg_loop.push_back(egg_curve);
                    }
                  }
                } else {
                  nout << "Trim curve segment is not a nurbs curve.\n";
                }
              }
            }
          }
        }
      }
    }
  }

  // We add the NURBS to the group down here, after all of the vpools
  // for the trim curves have been added.
  egg_group->add_child(egg_nurbs);

  if (shader != (MayaShader *)NULL) {
    set_shader_attributes(*egg_nurbs, *shader);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_trim_curve
//       Access: Private
//  Description: Converts the indicated Maya NURBS trim curve to a
//               corresponding egg structure, and returns it, or NULL
//               if there is a problem.
////////////////////////////////////////////////////////////////////
EggNurbsCurve *MayaToEggConverter::
make_trim_curve(const MFnNurbsCurve &curve, const string &nurbs_name,
                EggGroupNode *egg_group, int trim_curve_index) {
  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam()
      << "Trim curve:\n";
    mayaegg_cat.spam()
      << "  numCVs: "
      << curve.numCVs()
      << "\n";
    mayaegg_cat.spam()
      << "  numKnots: "
      << curve.numKnots()
      << "\n";
    mayaegg_cat.spam()
      << "  numSpans: "
      << curve.numSpans()
      << "\n";
  }

  MStatus status;

  MPointArray cv_array;
  status = curve.getCVs(cv_array, MSpace::kWorld);
  if (!status) {
    status.perror("MFnNurbsCurve::getCVs");
    return (EggNurbsCurve *)NULL;
  }
  MDoubleArray knot_array;
  status = curve.getKnots(knot_array);
  if (!status) {
    status.perror("MFnNurbsCurve::getKnots");
    return (EggNurbsCurve *)NULL;
  }

  /*
  MFnNurbsCurve::Form form = curve.form();
  */

  int degree = curve.degree();
  int cvs = curve.numCVs();
  int knots = curve.numKnots();

  assert(knots == cvs + degree - 1);

  string trim_name = "trim" + format_string(trim_curve_index);

  string vpool_name = nurbs_name + "." + trim_name;
  EggVertexPool *vpool = new EggVertexPool(vpool_name);
  egg_group->add_child(vpool);

  EggNurbsCurve *egg_curve = new EggNurbsCurve(trim_name);
  egg_curve->setup(degree + 1, knots + 2);

  int i;

  egg_curve->set_knot(0, knot_array[0]);
  for (i = 0; i < knots; i++) {
    egg_curve->set_knot(i + 1, knot_array[i]);
  }
  egg_curve->set_knot(knots + 1, knot_array[knots - 1]);

  for (i = 0; i < egg_curve->get_num_cvs(); i++) {
    double v[4];
    MStatus status = cv_array[i].get(v);
    if (!status) {
      status.perror("MPoint::get");
    } else {
      EggVertex vert;
      vert.set_pos(LPoint3d(v[0], v[1], v[3]));
      egg_curve->add_vertex(vpool->create_unique_vertex(vert));
    }
  }

  return egg_curve;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_nurbs_curve
//       Access: Private
//  Description: Converts the indicated Maya NURBS curve (a standalone
//               curve, not a trim curve) to a corresponding egg
//               structure and attaches it to the indicated egg group.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
make_nurbs_curve(const MDagPath &, const MFnNurbsCurve &curve,
                 EggGroup *egg_group) {
  MStatus status;
  string name = curve.name().asChar();

  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam()
      << "  numCVs: "
      << curve.numCVs()
      << "\n";
    mayaegg_cat.spam()
      << "  numKnots: "
      << curve.numKnots()
      << "\n";
    mayaegg_cat.spam()
      << "  numSpans: "
      << curve.numSpans()
      << "\n";
  }

  MPointArray cv_array;
  status = curve.getCVs(cv_array, MSpace::kWorld);
  if (!status) {
    status.perror("MFnNurbsCurve::getCVs");
    return;
  }
  MDoubleArray knot_array;
  status = curve.getKnots(knot_array);
  if (!status) {
    status.perror("MFnNurbsCurve::getKnots");
    return;
  }

  /*
  MFnNurbsCurve::Form form = curve.form();
  */

  int degree = curve.degree();
  int cvs = curve.numCVs();
  int knots = curve.numKnots();

  assert(knots == cvs + degree - 1);

  string vpool_name = name + ".cvs";
  EggVertexPool *vpool = new EggVertexPool(vpool_name);
  egg_group->add_child(vpool);

  EggNurbsCurve *egg_curve = new EggNurbsCurve(name);
  egg_group->add_child(egg_curve);
  egg_curve->setup(degree + 1, knots + 2);

  int i;

  egg_curve->set_knot(0, knot_array[0]);
  for (i = 0; i < knots; i++) {
    egg_curve->set_knot(i + 1, knot_array[i]);
  }
  egg_curve->set_knot(knots + 1, knot_array[knots - 1]);

  for (i = 0; i < egg_curve->get_num_cvs(); i++) {
    double v[4];
    MStatus status = cv_array[i].get(v);
    if (!status) {
      status.perror("MPoint::get");
    } else {
      EggVertex vert;
      vert.set_pos(LPoint4d(v[0], v[1], v[2], v[3]));
      egg_curve->add_vertex(vpool->create_unique_vertex(vert));
    }
  }

  MayaShader *shader = _shaders.find_shader_for_node(curve.object());
  if (shader != (MayaShader *)NULL) {
    shader->set_attributes(*egg_curve, *this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_polyset
//       Access: Private
//  Description: Converts the indicated Maya polyset to a bunch of
//               EggPolygons and parents them to the indicated egg
//               group.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
make_polyset(const MDagPath &dag_path, const MFnMesh &mesh,
             EggGroup *egg_group, MayaShader *default_shader) {
  MStatus status;
  string name = mesh.name().asChar();

  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam()
      << "  numPolygons: "
      << mesh.numPolygons()
      << "\n";
    mayaegg_cat.spam()
      << "  numVertices: "
      << mesh.numVertices()
      << "\n";
  }

  if (mesh.numPolygons() == 0) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring empty mesh " << name << "\n";
    }
    return;
  }

  string vpool_name = name + ".verts";
  EggVertexPool *vpool = new EggVertexPool(vpool_name);
  egg_group->add_child(vpool);

  /*
  MDagPath mesh_path;
  status = mesh.getPath(mesh_path);
  if (!status) {
    status.perror("MFnMesh::dagPath");
    return;
  }
  */
  MObject component_obj;
  MItMeshPolygon pi(dag_path, component_obj, &status);
  if (!status) {
    status.perror("MItMeshPolygon constructor");
    return;
  }

  MObjectArray shaders;
  MIntArray poly_shader_indices;

  status = mesh.getConnectedShaders(dag_path.instanceNumber(),
                                    shaders, poly_shader_indices);
  if (!status) {
    status.perror("MFnMesh::getConnectedShaders");
  }

  while (!pi.isDone()) {
    EggPolygon *egg_poly = new EggPolygon;
    egg_group->add_child(egg_poly);

    long num_verts = pi.polygonVertexCount();
    for (long i = 0; i < num_verts; i++) {
      EggVertex vert;

      MPoint p = pi.point(i, MSpace::kWorld);
      vert.set_pos(LPoint3d(p[0], p[1], p[2]));

      MVector n;
      status = pi.getNormal(i, n, MSpace::kWorld);
      if (!status) {
        status.perror("MItMeshPolygon::getNormal");
      } else {
        vert.set_normal(LVector3d(n[0], n[1], n[2]));
      }

      if (pi.hasUVs()) {
        float2 uvs;
        status = pi.getUV(i, uvs);
        if (!status) {
          status.perror("MItMeshPolygon::getUV");
        } else {
          vert.set_uv(TexCoordd(uvs[0], uvs[1]));
        }
      }

      if (pi.hasColor()) {
        MColor c;
        status = pi.getColor(c, i);
        if (!status) {
          status.perror("MItMeshPolygon::getColor");
        } else {
          vert.set_color(Colorf(c.r, c.g, c.b, 1.0));
        }
      }

      egg_poly->add_vertex(vpool->create_unique_vertex(vert));
    }

    // Determine the shader for this particular polygon.
    int index = pi.index();
    assert(index >= 0 && index < (int)poly_shader_indices.length());
    int shader_index = poly_shader_indices[index];
    if (shader_index != -1) {
      assert(shader_index >= 0 && shader_index < (int)shaders.length());
      MObject engine = shaders[shader_index];
      MayaShader *shader =
        _shaders.find_shader_for_shading_engine(engine);
      if (shader != (MayaShader *)NULL) {
        shader->set_attributes(*egg_poly, *this);
      }

    } else if (default_shader != (MayaShader *)NULL) {
      default_shader->set_attributes(*egg_poly, *this);
    }

    pi.next();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::get_egg_group
//       Access: Private
//  Description: Returns the EggGroup corresponding to the indicated
//               fully-qualified Maya path name.  If there is not
//               already an EggGroup corresponding to this Maya path,
//               creates one and returns it.
//
//               In this way we generate a unique EggGroup for each
//               Maya node we care about about, and also preserve the
//               Maya hierarchy sensibly.
////////////////////////////////////////////////////////////////////
EggGroup *MayaToEggConverter::
get_egg_group(const string &name, EggData &data) {
  // If we have already encountered this pathname, return the
  // corresponding EggGroup immediately.
  Groups::const_iterator gi = _groups.find(name);
  if (gi != _groups.end()) {
    return (*gi).second;
  }

  // Otherwise, we have to create it.  Do this recursively, so we
  // create each node along the path.
  EggGroup *egg_group;

  if (name.empty()) {
    // This is the top.
    egg_group = (EggGroup *)NULL;

  } else {
    // Maya uses vertical bars to separate path components.  Remove
    // everything from the rightmost bar on; this will give us the
    // parent's path name.
    size_t bar = name.rfind("|");
    string parent_name, local_name;
    if (bar != string::npos) {
      parent_name = name.substr(0, bar);
      local_name = name.substr(bar + 1);
    } else {
      local_name = name;
    }

    EggGroup *parent_egg_group = get_egg_group(parent_name, data);
    egg_group = new EggGroup(local_name);

    if (parent_egg_group != (EggGroup *)NULL) {
      parent_egg_group->add_child(egg_group);
    } else {
      data.add_child(egg_group);
    }
  }

  _groups.insert(Groups::value_type(name, egg_group));
  return egg_group;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::set_shader_attributes
//       Access: Public
//  Description: Applies the known shader attributes to the indicated
//               egg primitive.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
set_shader_attributes(EggPrimitive &primitive, const MayaShader &shader) {
  // In Maya, a polygon is either textured or colored.  The texture,
  // if present, replaces the color.

  if (shader._has_texture) {
    Filename pathname = convert_texture_path(_texture);
    EggTexture tex(shader._name, pathname);
    tex.set_wrap_u(shader._wrap_u ? EggTexture::WM_repeat : EggTexture::WM_clamp);
    tex.set_wrap_v(shader._wrap_v ? EggTexture::WM_repeat : EggTexture::WM_clamp);
 
    // Let's mipmap all textures by default.
    tex.set_minfilter(EggTexture::FT_linear_mipmap_linear);
    tex.set_magfilter(EggTexture::FT_linear);

    LMatrix3d mat = shader.compute_texture_matrix();
    if (!mat.almost_equal(LMatrix3d::ident_mat())) {
      tex.set_transform(mat);
    }

    EggTexture *new_tex =
      _textures.create_unique_texture(tex, ~EggTexture::E_tref_name);

    primitive.set_texture(new_tex);

  } else if (shader._has_color) {
    primitive.set_color(Colorf(shader._color[0], shader._color[1], 
                               shader._color[2], 1.0f));
  }
}
