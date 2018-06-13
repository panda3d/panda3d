
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

#ifndef _BOOL
#define _BOOL 1
#endif

#define REQUIRE_IOSTREAM

#include <maya/MGlobal.h>
#include <maya/MFileIO.h>
#include <maya/MLibrary.h>
#include <maya/MStatus.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnBlendShapeDeformer.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>

using std::cerr;

void
scan_nodes() {
  MStatus status;

  MItDag dag_iterator(MItDag::kDepthFirst, MFn::kTransform, &status);
  if (!status) {
    status.perror("MItDag constructor");
    exit(1);
  }

  while (!dag_iterator.isDone()) {
    MDagPath dag_path;
    status = dag_iterator.getPath(dag_path);
    if (!status) {
      status.perror("MItDag::getPath");
      exit(1);
    }

    MFnDagNode dag_node(dag_path, &status);
    if (!status) {
      status.perror("MFnDagNode constructor");
      exit(1);
    }

    cerr << dag_node.name() << "\n";

    dag_iterator.next();
  }
}

MFnBlendShapeDeformer *
get_slider(MString slider_name) {
  MStatus status;

  status = MGlobal::selectByName(slider_name, MGlobal::kReplaceList);
  if (!status) {
    status.perror(slider_name);
    exit(1);
  }
  MSelectionList list;
  status = MGlobal::getActiveSelectionList(list);
  if (!status) {
    status.perror("MGLobal::getActiveSelectionList");
    exit(1);
  }

  unsigned int num_objects = list.length();
  if (num_objects != 1) {
    cerr << "Warning: multiple objects match " << slider_name << "\n";
  }

  for (unsigned int i = 0; i < num_objects; i++) {
    MObject obj;
    status = list.getDependNode(i, obj);
    if (!status) {
      cerr << "selected element is not a dependency node.\n";
      status.perror("getDependNode");
    } else {
      if (obj.hasFn(MFn::kBlendShape)) {
        MFnBlendShapeDeformer *slider = new MFnBlendShapeDeformer(obj, &status);
        if (!status) {
          status.perror("MFnBlendShapeDeformer constructor");
        } else {
          cerr << "got slider " << slider->name() << "\n";
          return slider;
        }
      }

      cerr << "selected element is not a blend shape\n";
    }
  }

  cerr << "Couldn't find slider " << slider_name << "\n";
  exit(1);
}

MFnMesh *
get_mesh(MString mesh_name) {
  MStatus status;

  status = MGlobal::selectByName(mesh_name, MGlobal::kReplaceList);
  if (!status) {
    status.perror(mesh_name);
    exit(1);
  }
  MSelectionList list;
  status = MGlobal::getActiveSelectionList(list);
  if (!status) {
    status.perror("MGLobal::getActiveSelectionList");
    exit(1);
  }

  unsigned int num_objects = list.length();
  if (num_objects != 1) {
    cerr << "Warning: multiple objects match " << mesh_name << "\n";
  }

  for (unsigned int i = 0; i < num_objects; i++) {
    MObject obj;
    status = list.getDependNode(i, obj);
    if (!status) {
      cerr << "selected element is not a dependency node.\n";
      status.perror("getDependNode");

    } else {
      if (obj.hasFn(MFn::kMesh)) {
        // Maybe it's a mesh object itself.
        MFnMesh *mesh = new MFnMesh(obj, &status);
        if (!status) {
          status.perror("MFnMesh constructor");
        } else {
          cerr << "got mesh " << mesh->name() << "\n";
          return mesh;
        }

      } else if (obj.hasFn(MFn::kDagNode)) {
        // Maybe it's a dag node.
        MDagPath path;
        status = MDagPath::getAPathTo(obj, path);
        if (!status) {
          status.perror("MDagPath::getAPathTo");
          exit(1);
        }
        if (path.hasFn(MFn::kMesh)) {
          MFnMesh *mesh = new MFnMesh(path, &status);
          if (!status) {
            status.perror("MFnMesh constructor");
          } else {
            cerr << "got mesh " << mesh->name() << "\n";
            return mesh;
          }
        }
      }

      cerr << "selected element is not a mesh\n";
    }
  }

  cerr << "Couldn't find mesh " << mesh_name << "\n";
  exit(1);
}

void
output_vertices(const char *filename, MFnMesh &mesh) {
  MStatus status;

  MPointArray verts;
  // status = mesh.getPoints(verts, MSpace::kObject);
  status = mesh.getPoints(verts, MSpace::kWorld);
  if (!status) {
    status.perror("mesh.getPoints");
    exit(1);
  }

  std::ofstream file(filename, std::ios::out | std::ios::trunc);
  if (!file) {
    cerr << "Couldn't open " << filename << " for output.\n";
    exit(1);
  }

  unsigned int num_verts = verts.length();
  cerr << "writing " << num_verts << " vertices to " << filename << "\n";
  for (unsigned int i = 0; i < num_verts; i++) {
    file << i << ". " << verts[i] << "\n";
  }
}

int
main(int argc, char *argv[]) {
  cerr << "Initializing Maya\n";
  MStatus status;
  status = MLibrary::initialize(argv[0]);
  if (!status) {
    status.perror("Could not initialize");
    exit(1);
  }

  cerr << "Using Maya library version " << MGlobal::mayaVersion() << "\n";

  if (argc < 4) {
    cerr << "\nUsage:\n\nblend_test.cxx file.mb slider_name mesh_name\n\n";
    exit(1);
  }

  MString filename = argv[1];
  MString slider_name = argv[2];
  MString mesh_name = argv[3];

  MFileIO::newFile(true);

  cerr << "Reading " << filename << "\n";
  status = MFileIO::open(filename);
  if (!status) {
    status.perror(filename);
    exit(1);
  }

  MFnBlendShapeDeformer *slider = get_slider(slider_name);
  MFnMesh *mesh = get_mesh(mesh_name);

  cerr << "\nOriginal slider value is " << slider->weight(0) << "\n";

  output_vertices("before.txt", *mesh);

  cerr << "Setting slider to 1\n";
  status = slider->setWeight(0, 1.0);
  if (!status) {
    status.perror("MFnBlendShapeDeformer::setWeight");
    exit(1);
  }

  output_vertices("after.txt", *mesh);

  return 0;
}
