// Filename: seaPatchNode.cxx
// Created by:  sshodhan (18Jun04)
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


#include "config_pgraph.h"
#include "nodePath.h"
#include "clockObject.h"
#include "dcast.h"
#include "cullTraverserData.h"
#include "cullTraverser.h"
#include "seaPatchNode.h"
#include "geomNode.h"
#include "geom.h"


#include <math.h>

TypeHandle SeaPatchNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::Constructor
//       Access: Published
//  Description: Use SeaPatchNode() to construct a new
//               SeaPatchNode object.
////////////////////////////////////////////////////////////////////
SeaPatchNode::
SeaPatchNode(const string &name) :
PandaNode(name)
{
  // center
  LPoint3f c;
  c[0] = 0.0;
  c[1] = 0.0;
  c[2] = 0.0;
  _center = c;
  _enabled = true;
  // continuously increasing parameter to advance the wave
  _wave_movement = 0.0;
  _wavelength = 25.0;
  _amplitude = 0.05;
  _frequency = 0.01;
  // from threshold -> radius the waves will fade out
  _radius = 100.0;
  _threshold = 80.0;
  // sliding of UVs
  _passive_move[0] = 0.0001;
  _passive_move[1] = 0.0001;
  // UV noise
  _noise_amp = 0.001;
  _noise_on_uv = true;
  _noise_f = 10.0;
  _noise_detail = 4;
  _xsize = 256.0;
  _ysize = 256.0;
  _u_scale = 256.0;
  _v_scale = 256.0;
  _u_sin_scale = 1/100.0;
  _v_sin_scale = 1/100.0;
  // randomize noise tables
  noise_init();
  // shading
  _alpha_scale = 1.0;
  _color_scale = 1.0;
  _light_color = Colorf(0.2, 0.4, 0.6, 1.0);
  _alpha_mode = ANONE;
}


////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool SeaPatchNode::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
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
bool SeaPatchNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {

  PandaNode *node = data.node(); // Get SeaPatchNode
  // Get net transoformation matrix
  LMatrix4f net_trans_mat = 
    data._node_path.get_node_path().get_net_transform()->get_mat();

  if(is_enabled()) {
    // Go through all the children, find Geom nodes and pass waves
    recurse_children(node,net_trans_mat);
    // Advance the wave for all children
    _wave_movement += _frequency; // Advance the wave
  }

  // Continue traversal
  int num_children = node->get_num_children();
  for(int i = 0; i < num_children; i++) {
    CullTraverserData child_data(data, node->get_child(i));
    trav->traverse(child_data);
  }
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::recurse_children
//       Access: Published
//  Description: Dig out the GeomNodes and do_wave on them
////////////////////////////////////////////////////////////////////
void SeaPatchNode::
recurse_children(PandaNode *node, LMatrix4f net_trans_mat) {
   
  if (node->is_of_type(GeomNode::get_class_type())) {
    GeomNode *geom_node = DCAST(GeomNode, node);
    int num_geoms = geom_node->get_num_geoms();
    for(int j =0; j < num_geoms; j++) { 
      PT(Geom) geom = geom_node->get_geom(j);
      do_wave(geom, net_trans_mat);
    }
  }
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    recurse_children(cr.get_child(i), net_trans_mat);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::do_wave
//       Access: Published
//  Description: pass sine wave through geometry
//               add UV noise and movement
//               apply vertex color based shading
////////////////////////////////////////////////////////////////////
void SeaPatchNode::
do_wave(Geom * geom, LMatrix4f net_trans_mat) {
  float dist;
  unsigned int num_vertices = geom->get_num_vertices();
  PTA_ushort vindex;
  PTA_ushort tindex;
  PTA_ushort cindex;
  PTA_ushort nindex;
  PTA_Vertexf vert;
  PTA_TexCoordf texc;
  PTA_Colorf colors;
  PTA_Normalf norms;
  Colorf c;
  GeomBindType btypet;
  float u = 0.0, v = 0.0;
  float unoise,vnoise;
  float height_change;

  // Get vertices and UVs
  geom->get_coords(vert, vindex);
  geom->get_texcoords(texc, btypet, tindex);

  for (unsigned int k = 0; k < num_vertices; k++) { // For all vertices
    LPoint3f V = vert[k] * net_trans_mat; // multiply with net transform
    LPoint3f new_center = _center * net_trans_mat;

    height_change = (sin(_wave_movement + (V[0] / _wavelength[0]) ) 
    + sin(_wave_movement + (V[1] / _wavelength[1]))) * _amplitude; // Do the wave

    // need this distance for fading out waves
    // ** add fading out to getHeight function
    dist = (float) sqrt( pow(V[0] - new_center[0],2) + pow(V[1] - new_center[1],2));
    
    if(dist > _threshold) {
        if(dist >= _radius) {
          vert[k][2] = 0.0; // completely faded out
        }
        else {
          vert[k][2] = height_change * ((_radius - dist)/ _radius); // linear fade
        }
    }
    else {
      vert[k][2] = height_change;
    }

    // Recompute normals based on height
    norms.push_back(get_normal(V[0],V[1]));
    nindex.push_back(k);
    geom->set_normals(norms, G_PER_VERTEX, nindex);

    // Vertex color based shading
    //  ** add shade_mode (high, low , all, none)
    c[0] = (_color_scale + _light_color[0] + (height_change / (2 * _amplitude)))/3.0;
    c[1] = (_color_scale + _light_color[1] + (height_change / (2 * _amplitude)))/3.0;
    c[2] = (_color_scale + _light_color[2] + (height_change / (2 * _amplitude)))/3.0;
    if(_alpha_mode == AHIGH) {
      c[3] = (_alpha_scale + _light_color[3] + (1.0 - (height_change / (2 * _amplitude))))/3.0;
    } 
    else if(_alpha_mode == ALOW) {
      c[3] = (_alpha_scale + _light_color[3] + (height_change / (2 * _amplitude)))/3.0;
    }
    else if(_alpha_mode == ATOTAL) {
      c[3] = (_alpha_scale + _light_color[3]) /2.0;
    }
    else if(_alpha_mode == ANONE) {
      c[3] = 1.0;
    }

    colors.push_back(c);
    cindex.push_back(k);
      
    // UV COMPUTATION
    // Divide vertex X and Y coords by some scale
    // Also, pass sin wave through the scale for UV scaling
    // ** Need to make this transform independant
    _u_scale += sin (_wave_movement) * _u_sin_scale; 
    _v_scale += sin (_wave_movement) * _v_sin_scale;

    if(_u_scale == 0) {
        _u_scale = 1;
    }
    if(_v_scale == 0) {
        _v_scale = 1;
    }

    u = V[0]  / _u_scale;
    v = V[1]  / _v_scale;


    // UV Noise (using the DQ Pirates noise function)
    if(_noise_on_uv) {
      unoise = do_noise(V[0], V[1], ClockObject::get_global_clock()->get_frame_time(), 0);
      vnoise = do_noise(V[0], V[1], ClockObject::get_global_clock()->get_frame_time(), 1);
      u  +=  unoise;
      v  +=  vnoise;
    }
    // Texture sliding
    texc[k][0] = u + _move_variable[0];
    texc[k][1] = v + _move_variable[1];
  }

  // Apply vertex shading
  geom->set_colors (colors, G_PER_VERTEX , cindex);

  _move_variable[0] += _passive_move[0];
  _move_variable[1] += _passive_move[1];

  if(_move_variable[0] > 1.0) {
    _move_variable[0] = 0.0;
  }
  if(_move_variable[1] > 1.0) {
    _move_variable[1] = 0.0;
  }  
}


////////////////////////////////////////////////////////////////////
//     Function: NoiseWave::Constructor
//       Access: Public, Scheme
//  Description: Initialize noise permutation and gradient table
//               This function is adapted from the old DQ pirates code
//               Everything is done for both U and V noise
////////////////////////////////////////////////////////////////////
void SeaPatchNode::
noise_init() {

  int i, j, k, t;

  // Randomize the permutation table.
  // U TABLE
  for (i = 0; i < noise_table_size; i++) {
    _perm_tab_u[i] = i;
  }
  for (i = 0; i < (noise_table_size / 2); i++) {
    j = rand() % noise_table_size;
    k = rand() % noise_table_size;
    t = _perm_tab_u[j];
    _perm_tab_u[j] = _perm_tab_u[k];
    _perm_tab_u[k] = t;
  }
  

  // Randomize the permutation table.
  // V TABLE
  for (i = 0; i < noise_table_size; i++) {
    _perm_tab_v[i] = i;
  }
  for (i = 0; i < (noise_table_size / 2); i++) {
    j = rand() % noise_table_size;
    k = rand() % noise_table_size;
    t = _perm_tab_v[j];
    _perm_tab_v[j] = _perm_tab_v[k];
    _perm_tab_v[k] = t;
  }


  // Choose a number of random unit vectors for the gradient table.
  // U TABLE
  for (i = 0; i < noise_table_size; i++) {
    float m_u;
    do {
      _grad_tab_u[i][0] = (float)(rand() - (RAND_MAX / 2)) / (RAND_MAX / 2);
      _grad_tab_u[i][1] = (float)(rand() - (RAND_MAX / 2)) / (RAND_MAX / 2);
      _grad_tab_u[i][2] = (float)(rand() - (RAND_MAX / 2)) / (RAND_MAX / 2);
      m_u = _grad_tab_u[i].dot(_grad_tab_u[i]);
    } while (m_u == 0.0 || m_u > 1.0);

    _grad_tab_u[i] /= sqrt(m_u);
  }

  // Choose a number of random unit vectors for the gradient table.
  // V TABLE
  for (i = 0; i < noise_table_size; i++) {
    float m_v;
    do {
      _grad_tab_v[i][0] = (float)(rand() - (RAND_MAX / 2)) / (RAND_MAX / 2);
      _grad_tab_v[i][1] = (float)(rand() - (RAND_MAX / 2)) / (RAND_MAX / 2);
      _grad_tab_v[i][2] = (float)(rand() - (RAND_MAX / 2)) / (RAND_MAX / 2);
      m_v = _grad_tab_v[i].dot(_grad_tab_v[i]);
    } while (m_v == 0.0 || m_v > 1.0);

    _grad_tab_v[i] /= sqrt(m_v);
  }


}



////////////////////////////////////////////////////////////////////
//     Function: NoiseWave::PlainNoise
//       Access: Protected
//  Description: 3D noise function 
//               Takes x, y position , time, some scale s
//               uvi determines if its U noise or V noise
////////////////////////////////////////////////////////////////////
float SeaPatchNode::
plain_noise(float x, float y, float z, unsigned int s, int uvi ) {
  x *= s;
  y *= s;
  z *= s;
  int a = (int)floor(x);
  int b = (int)floor(y);
  int c = (int)floor(z);

  float sum = 0.0;
  int i, j, k;
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 2; j++) {
      for (k = 0; k < 2; k++) {
        if(uvi ==0 ) {
	      int n_u = _perm_tab_u[Mod(c + k,noise_table_size)];
	      n_u = _perm_tab_u[Mod(b + j + n_u , noise_table_size)];
	      n_u = _perm_tab_u[Mod(a + i + n_u , noise_table_size)];
	      LVecBase3f v_u(x - a - i, y - b - j, z - c - k);
	  	  sum += Weight(v_u[0]) * Weight(v_u[1]) * Weight(v_u[2]) *
	        (_grad_tab_u[n_u].dot(v_u));
        }
        else {
            int n_v = _perm_tab_v[Mod(c + k , noise_table_size)];
	      n_v = _perm_tab_v[Mod(b + j + n_v  , noise_table_size)];
	      n_v = _perm_tab_v[Mod(a + i + n_v , noise_table_size)];
	      LVecBase3f v_v(x - a - i, y - b - j, z - c - k);
	  	  sum += Weight(v_v[0]) * Weight(v_v[1]) * Weight(v_v[2]) *
	        (_grad_tab_v[n_v].dot(v_v));
        }

      }
    }
  }

  return sum / s;
}


////////////////////////////////////////////////////////////////////
//     Function: NoiseWave::do_noise
//       Access: Protected
//  Description: Based on detail additively compute various level
//               of detail noises
////////////////////////////////////////////////////////////////////
float SeaPatchNode:: 
do_noise(float x, float y, float t, int uvi) {
 
  unsigned int s = 1;
  x /= _xsize;
  y /= _ysize;
  t *= _noise_f * 0.1;
  
  float sum = 0.0;
  int i;
  for (i = 0; i <= _noise_detail; i++) {
    sum += plain_noise(x, y, t, s, uvi);
    s *= 2;
  }

  return sum * _noise_amp; 
}


////////////////////////////////////////////////////////////////////
//     Function: set_alpha_mode
//  Description: Alpha Modes are AHIGH, ALOW, ATOTAL, ANONE
//               high and low are height based affecting high and low
//               points respectively. total affects all vertices
//               none is no alpha. you have to manually setTransparency
//////////////////////////////////////////////////////////////////// 
void SeaPatchNode::
set_alpha_mode(Alpha_Type a) {
  _alpha_mode = a;
}

// ** IMPLEMENT compare_to properly once API is more finalized

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this SeaPatchNode
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.
//
//               Two SeaPatchNodes are considered equivalent if they
//               consist of exactly the same properties
//				 Otherwise, they are different; different
//               SeaPatchNodes will be ranked in a consistent but
//               undefined ordering; the ordering is useful only for
//               placing the SeaPatchNodes in a sorted container like an
//               STL set.
////////////////////////////////////////////////////////////////////
int SeaPatchNode::
compare_to(const SeaPatchNode &other) const {
   if (_enabled != other._enabled) {
	return _enabled ? 1 :-1;
   }
   if (_wave_movement != other._wave_movement) {
       return _wave_movement < other._wave_movement ? -1 : 1;
   }
  return 0;
}

