// Filename: xFileMaker.cxx
// Created by:  drose (19Jun01)
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

#include "xFileMaker.h"
#include "notify.h"
#include "eggGroupNode.h"
#include "eggGroup.h"
#include "eggBin.h"
#include "eggPolysetMaker.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPolygon.h"
#include "eggData.h"
#include "pvector.h"
#include "vector_int.h"
#include "string_utils.h"

// This must be included only in exactly one .cxx file, since
// including defines the structure!
#include <rmxftmpl.h>

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileMaker::
XFileMaker() {
  _dx_file = NULL;
  _dx_file_save = NULL;
  _mesh_index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileMaker::
~XFileMaker() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::open
//       Access: Public
//  Description: Opens the indicated filename for writing, and writes
//               the .x header information; returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
open(const Filename &filename) {
  HRESULT hr;

  close();
  hr = DirectXFileCreate(&_dx_file);
  if (hr != DXFILE_OK) {
    nout << "Unable to create X interface.\n";
    return false;
  }

  // Register our templates.
  hr = _dx_file->RegisterTemplates(D3DRM_XTEMPLATES, D3DRM_XTEMPLATE_BYTES);
  if (hr != DXFILE_OK) {
    nout << "Unable to register templates.\n";
    return false;
  }

  string os_file = filename.to_os_specific();
  hr = _dx_file->CreateSaveObject(os_file.c_str(), DXFILEFORMAT_TEXT,
                                  &_dx_file_save);
  if (hr != DXFILE_OK) {
    nout << "Unable to open X file: " << os_file << "\n";
    return false;
  }

  // Save the templates we will use.
  static const GUID *temps[] = {
    &TID_D3DRMVector,
    &TID_D3DRMMeshFace,
    &TID_D3DRMMesh,
    &TID_D3DRMFrame,
  };
  static const int num_temps = sizeof(temps) / sizeof(temps[0]);
  hr = _dx_file_save->SaveTemplates(num_temps, temps);
  if (hr != DXFILE_OK) {
    nout << "Unable to save templates.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::close
//       Access: Public
//  Description: Finalizes and closes the file previously opened via
//               open().
////////////////////////////////////////////////////////////////////
void XFileMaker::
close() {
  if (_dx_file != NULL) {
    if (_dx_file_save != NULL) {
      _dx_file_save->Release();
      _dx_file_save = NULL;
    }
    _dx_file->Release();
    _dx_file = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_tree
//       Access: Public
//  Description: Adds the egg tree rooted at the indicated node to the
//               DX structure.  This may be somewhat destructive of
//               the egg tree.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_tree(EggData &egg_data) {
  // Now collect all the polygons together into polysets.
  EggPolysetMaker pmaker;
  int num_bins = pmaker.make_bins(&egg_data);

  // And now we're ready to traverse the egg hierarchy.
  return add_node(&egg_data, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_node
//       Access: Private
//  Description: Adds the node to the DX structure, in whatever form
//               it is supported.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_node(EggNode *egg_node, LPDIRECTXFILEDATA dx_parent) {
  if (egg_node->is_of_type(EggBin::get_class_type())) {
    return add_bin(DCAST(EggBin, egg_node), dx_parent);

  } else if (egg_node->is_of_type(EggGroup::get_class_type())) {
    return add_group(DCAST(EggGroup, egg_node), dx_parent);

  } else if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    // A grouping node of some kind.
    EggGroupNode *egg_group = DCAST(EggGroupNode, egg_node);
    LPDIRECTXFILEDATA data;
    if (!create_frame(data, egg_group->get_name())) {
      return false;
    }
    
    if (!recurse_nodes(egg_group, data)) {
      data->Release();
      return false;
    }
    
    if (!attach_and_release(data, dx_parent)) {
      return false;
    }
    
    return true;
  }

  // Some unsupported node type.  Ignore it.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_group
//       Access: Private
//  Description: Adds a frame for the indicated group node.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_group(EggGroup *egg_group, LPDIRECTXFILEDATA dx_parent) {
  LPDIRECTXFILEDATA data;
  if (!create_frame(data, egg_group->get_name())) {
    return false;
  }

  if (!recurse_nodes(egg_group, data)) {
    data->Release();
    return false;
  }

  if (!attach_and_release(data, dx_parent)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_bin
//       Access: Private
//  Description: Determines what kind of object needs to be added for
//               the indicated bin node.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_bin(EggBin *egg_bin, LPDIRECTXFILEDATA dx_parent) {
  switch (egg_bin->get_bin_number()) {
  case EggPolysetMaker::BN_polyset:
    return add_polyset(egg_bin, dx_parent);
  }

  cerr << "Unexpected bin type " << egg_bin->get_bin_number() << "\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_polyset
//       Access: Private
//  Description: Adds a mesh object corresponding to the collection of
//               polygons within the indicated bin.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_polyset(EggBin *egg_bin, LPDIRECTXFILEDATA dx_parent) {
  // Make sure that all our polygons are reasonable.
  egg_bin->remove_invalid_primitives();

  // First, we need to collect all the common vertices for the
  // polygons in this polyset.  We can do this fairly easily using a
  // vertex pool.
  PT(EggVertexPool) vpool = new EggVertexPool("");

  int num_polys = 0;

  EggGroupNode::iterator ci;
  for (ci = egg_bin->begin(); ci != egg_bin->end(); ++ci) {
    EggPolygon *poly;
    DCAST_INTO_R(poly, *ci, false);

    // A temporary holder for the newly converted vertices of the
    // polygon.
    PT(EggPolygon) vertex_holder = new EggPolygon;

    num_polys++;
    EggPolygon::iterator vi;
    for (vi = poly->begin(); vi != poly->end(); ++vi) {
      // Make a copy of the polygon's original vertex.
      PT(EggVertex) vtx_copy = new EggVertex(*(*vi));
      
      // Now change the properties on the vertex as appropriate to our
      // mesh.
      if (!vtx_copy->has_color()) {
        vtx_copy->set_color(poly->get_color());
      }
      vtx_copy->_dnormals.clear();
      vtx_copy->_duvs.clear();
      vtx_copy->_drgbas.clear();
      vtx_copy->_dxyzs.clear();

      // And create the unique vertex.
      EggVertex *vtx = vpool->create_unique_vertex(*vtx_copy);
      vertex_holder->add_vertex(vtx);
    }

    poly->copy_vertices(*vertex_holder);
  }

  // Now create the raw data for the Mesh object.
  int highest_index = vpool->get_highest_index();
  Datagram raw_data;
  raw_data.add_int32(highest_index);
  for (int i = 1; i <= highest_index; i++) {
    EggVertex *vtx = vpool->get_vertex(i);
    nassertr(vtx != (EggVertex *)NULL, false);
    Vertexd pos = vtx->get_pos3();
    raw_data.add_float32(pos[0]);
    raw_data.add_float32(pos[1]);
    raw_data.add_float32(pos[2]);
  }

  raw_data.add_int32(num_polys);
  for (ci = egg_bin->begin(); ci != egg_bin->end(); ++ci) {
    EggPolygon *poly;
    DCAST_INTO_R(poly, *ci, false);

    raw_data.add_int32(poly->size());
    EggPolygon::reverse_iterator vi;
    for (vi = poly->rbegin(); vi != poly->rend(); ++vi) {
      int index = (*vi)->get_index();
      raw_data.add_int32(index - 1);
    }
  }

  // Finally, create the Mesh object.
  HRESULT hr;
  LPDIRECTXFILEDATA data;

  string name = "mesh" + format_string(_mesh_index);
  _mesh_index++;

  hr = _dx_file_save->CreateDataObject
    (TID_D3DRMMesh, name.c_str(), NULL, 
     raw_data.get_length(), (void *)raw_data.get_data(),
     &data);
  if (hr != DXFILE_OK) {
    nout << "Unable to create Mesh object\n";
    return false;
  }

  if (!attach_and_release(data, dx_parent)) {
    return false;
  }
  return true;
}
  
////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::recurse_nodes
//       Access: Private
//  Description: Adds each child of the indicated Node as a child of
//               the indicated DX object.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
recurse_nodes(EggGroupNode *egg_node, LPDIRECTXFILEDATA dx_parent) {
  EggGroupNode::iterator ci;
  for (ci = egg_node->begin(); ci != egg_node->end(); ++ci) {
    EggNode *node = (*ci);
    if (!add_node(node, dx_parent)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::create_frame
//       Access: Private
//  Description: Creates a "frame" object with the indicated name.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
create_frame(LPDIRECTXFILEDATA &data, const string &name) {
  HRESULT hr;

  string nice_name = make_nice_name(name);
  hr = _dx_file_save->CreateDataObject
    (TID_D3DRMFrame, nice_name.c_str(), NULL, 0, NULL, &data);
  if (hr != DXFILE_OK) {
    nout << "Unable to create frame object for " << name << "\n";
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::attach_and_release
//       Access: Private
//  Description: Assigns the indicated X data object to the indicated
//               parent, and releases the pointer.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
attach_and_release(LPDIRECTXFILEDATA data, LPDIRECTXFILEDATA dx_parent) {
  HRESULT hr;

  if (dx_parent == NULL) {
    // No parent; it's a toplevel object.
    hr = _dx_file_save->SaveData(data);
    if (hr != DXFILE_OK) {
      nout << "Unable to save data object\n";
      data->Release();
      return false;
    }
  } else {
    // Got a parent; it's a child of the indicated object.
    hr = dx_parent->AddDataObject(data);
    if (hr != DXFILE_OK) {
      nout << "Unable to save data object\n";
      data->Release();
      return false;
    }
  }

  data->Release();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::make_nice_name
//       Access: Private, Static
//  Description: Transforms the indicated egg name to a name that is
//               acceptable to the DirectX format.
////////////////////////////////////////////////////////////////////
string XFileMaker::
make_nice_name(const string &str) {
  string result;

  string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    if (isalnum(*si)) {
      result += *si;
    } else {
      result += "_";
    }
  }

  return result;
}
