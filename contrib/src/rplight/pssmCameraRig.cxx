/**
 *
 * RenderPipeline
 *
 * Copyright (c) 2014-2016 tobspr <tobias.springer1@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */


#include "pssmCameraRig.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include "orthographicLens.h"


PStatCollector PSSMCameraRig::_update_collector("App:Show code:RP_PSSM_update");

/**
 * @brief Constructs a new PSSM camera rig
 * @details This constructs a new camera rig, with a given amount of splits.
 *   The splits can not be changed later on. Splits are also called Cascades.
 *
 *   An assertion will be triggered if the splits are below zero.
 *
 * @param num_splits Amount of PSSM splits
 */
PSSMCameraRig::PSSMCameraRig(size_t num_splits) {
  nassertv(num_splits > 0);
  _num_splits = num_splits;
  _pssm_distance = 100.0;
  _sun_distance = 500.0;
  _use_fixed_film_size = false;
  _use_stable_csm = true;
  _logarithmic_factor = 1.0;
  _resolution = 512;
  _border_bias = 0.1;
  _camera_mvps = PTA_LMatrix4::empty_array(num_splits);
  _camera_nearfar = PTA_LVecBase2::empty_array(num_splits);
  init_cam_nodes();
}

/**
 * @brief Destructs the camera rig
 * @details This destructs the camera rig, cleaning up all used resources.
 */
PSSMCameraRig::~PSSMCameraRig() {
  // TODO: Detach all cameras and call remove_node. Most likely this is not
  // an issue tho, because the camera rig will never get destructed.
}

/**
 * @brief Internal method to init the cameras
 * @details This method constructs all cameras and their required lens nodes
 *   for all splits. It also resets the film size array.
 */
void PSSMCameraRig::init_cam_nodes() {
  _cam_nodes.reserve(_num_splits);
  _max_film_sizes.resize(_num_splits);
  _cameras.resize(_num_splits);
  for (size_t i = 0; i < _num_splits; ++i)
  {
    // Construct a new lens
    Lens *lens = new OrthographicLens();
    lens->set_film_size(1, 1);
    lens->set_near_far(1, 1000);

    // Construct a new camera
    _cameras[i] = new Camera("pssm-cam-" + format_string(i), lens);
    _cam_nodes.push_back(NodePath(_cameras[i]));
    _max_film_sizes[i].fill(0);
  }
}

/**
 * @brief Reparents the camera rig
 * @details This reparents all cameras to the given parent. Usually the parent
 *   will be ShowBase.render. The parent should be the same node where the
 *   main camera is located in, too.
 *
 *   If an empty parrent is passed, an assertion will get triggered.
 *
 * @param parent Parent node path
 */
void PSSMCameraRig::reparent_to(NodePath parent) {
  nassertv(!parent.is_empty());
  for (size_t i = 0; i < _num_splits; ++i) {
    _cam_nodes[i].reparent_to(parent);
  }
  _parent = parent;
}

/**
 * @brief Internal method to compute the view-projection matrix of a camera
 * @details This returns the view-projection matrix of the given split. No bounds
 *   checking is done. If an invalid index is passed, undefined behaviour occurs.
 *
 * @param split_index Index of the split
 * @return view-projection matrix of the split
 */
LMatrix4 PSSMCameraRig::compute_mvp(size_t split_index) {
  LMatrix4 transform = _parent.get_transform(_cam_nodes[split_index])->get_mat();
  return transform * _cameras[split_index]->get_lens()->get_projection_mat();
}

/**
 * @brief Internal method used for stable CSM
 * @details This method is used when stable CSM is enabled. It ensures that each
 *   source only moves in texel-steps, thus preventing flickering. This works by
 *   projecting the point (0, 0, 0) to NDC space, making sure that it gets projected
 *   to a texel center, and then projecting that texel back.
 *
 *   This only works if the camera does not rotate, change its film size, or change
 *   its angle.
 *
 * @param mat view-projection matrix of the camera
 * @param resolution resolution of the split
 *
 * @return Offset to add to the camera position to achieve stable snapping
 */
LVecBase3 PSSMCameraRig::get_snap_offset(const LMatrix4& mat, size_t resolution) {
  // Transform origin to camera space
  LPoint4 base_point = mat.get_row(3) * 0.5 + 0.5;

  // Compute the snap offset
  float texel_size = 1.0 / (float)(resolution);
  float offset_x = fmod(base_point.get_x(), texel_size);
  float offset_y = fmod(base_point.get_y(), texel_size);

  // Reproject the offset back, for that we need the inverse MVP
  LMatrix4 inv_mat(mat);
  inv_mat.invert_in_place();
  LVecBase3 new_base_point = inv_mat.xform_point(LVecBase3(
      (base_point.get_x() - offset_x) * 2.0 - 1.0,
      (base_point.get_y() - offset_y) * 2.0 - 1.0,
      base_point.get_z() * 2.0 - 1.0
    ));
  return -new_base_point;
}

/**
 * @brief Computes the average of a list of points
 * @details This computes the average over a given set of points in 3D space.
 *   It returns the average of those points, namely sum_of_points / num_points.
 *
 *   It is designed to work with a frustum, which is why it takes two arrays
 *   with a dimension of 4. Usually the first array are the camera near points,
 *   and the second array are the camera far points.
 *
 * @param starts First array of points
 * @param ends Second array of points
 * @return Average of points
 */
LPoint3 get_average_of_points(LVecBase3 const (&starts)[4], LVecBase3 const (&ends)[4]) {
  LPoint3 mid_point(0, 0, 0);
  for (size_t k = 0; k < 4; ++k) {
    mid_point += starts[k];
    mid_point += ends[k];
  }
  return mid_point / 8.0;
}

/**
 * @brief Finds the minimum and maximum extends of the given projection
 * @details This projects each point of the given array of points using the
 *   cameras view-projection matrix, and computes the minimum and maximum
 *   of the projected points.
 *
 * @param min_extent Will store the minimum extent of the projected points in NDC space
 * @param max_extent Will store the maximum extent of the projected points in NDC space
 * @param transform The transformation matrix of the camera
 * @param proj_points The array of points to project
 * @param cam The camera to be used to project the points
 */
void find_min_max_extents(LVecBase3 &min_extent, LVecBase3 &max_extent, const LMatrix4 &transform, LVecBase3 const (&proj_points)[8], Camera *cam) {

  min_extent.fill(1e10);
  max_extent.fill(-1e10);
  LPoint2 screen_points[8];

  // Now project all points to the screen space of the current camera and also
  // find the minimum and maximum extents
  for (size_t k = 0; k < 8; ++k) {
    LVecBase4 point(proj_points[k], 1);
    LPoint4 proj_point = transform.xform(point);
    LPoint3 proj_point_3d(proj_point.get_x(), proj_point.get_y(), proj_point.get_z());
    cam->get_lens()->project(proj_point_3d, screen_points[k]);

    // Find min / max extents
    if (screen_points[k].get_x() > max_extent.get_x()) max_extent.set_x(screen_points[k].get_x());
    if (screen_points[k].get_y() > max_extent.get_y()) max_extent.set_y(screen_points[k].get_y());

    if (screen_points[k].get_x() < min_extent.get_x()) min_extent.set_x(screen_points[k].get_x());
    if (screen_points[k].get_y() < min_extent.get_y()) min_extent.set_y(screen_points[k].get_y());

    // Find min / max projected depth to adjust far plane
    if (proj_point.get_y() > max_extent.get_z()) max_extent.set_z(proj_point.get_y());
    if (proj_point.get_y() < min_extent.get_z()) min_extent.set_z(proj_point.get_y());
  }
}

/**
 * @brief Computes a film size from a given minimum and maximum extend
 * @details This takes a minimum and maximum extent in NDC space and computes
 *   the film size and film offset needed to cover that extent.
 *
 * @param film_size Output film size, can be used for Lens::set_film_size
 * @param film_offset Output film offset, can be used for Lens::set_film_offset
 * @param min_extent Minimum extent
 * @param max_extent Maximum extent
 */
inline void get_film_properties(LVecBase2 &film_size, LVecBase2 &film_offset, const LVecBase3 &min_extent, const LVecBase3 &max_extent) {
  float x_center = (min_extent.get_x() + max_extent.get_x()) * 0.5;
  float y_center = (min_extent.get_y() + max_extent.get_y()) * 0.5;
  float x_size = max_extent.get_x() - x_center;
  float y_size = max_extent.get_y() - y_center;
  film_size.set(x_size, y_size);
  film_offset.set(x_center * 0.5, y_center * 0.5);
}

/**
 * @brief Merges two arrays
 * @details This takes two arrays which each 4 members and produces an array
 *   with both arrays contained.
 *
 * @param dest Destination array
 * @param array1 First array
 * @param array2 Second array
 */
inline void merge_points_interleaved(LVecBase3 (&dest)[8], LVecBase3 const (&array1)[4], LVecBase3 const (&array2)[4]) {
  for (size_t k = 0; k < 4; ++k) {
    dest[k] = array1[k];
    dest[k+4] = array2[k];
  }
}


/**
 * @brief Internal method to compute the splits
 * @details This is the internal update method to update the PSSM splits.
 *   It distributes the camera splits over the frustum, and updates the
 *   MVP array aswell as the nearfar array.
 *
 * @param transform Main camera transform
 * @param max_distance Maximum pssm distance, relative to the camera far plane
 * @param light_vector Sun-Vector
 */
void PSSMCameraRig::compute_pssm_splits(const LMatrix4& transform, float max_distance, const LVecBase3& light_vector) {
  nassertv(!_parent.is_empty());

  // PSSM Distance should never be smaller than camera far plane.
  nassertv(max_distance <= 1.0);

  float filmsize_bias = 1.0 + _border_bias;

  // Compute the positions of all cameras
  for (size_t i = 0; i < _cam_nodes.size(); ++i) {
    float split_start = get_split_start(i) * max_distance;
    float split_end = get_split_start(i + 1) * max_distance;

    LVecBase3 start_points[4];
    LVecBase3 end_points[4];
    LVecBase3 proj_points[8];

    // Get split bounding box, and collect all points which define the frustum
    for (size_t k = 0; k < 4; ++k) {
      start_points[k] = get_interpolated_point((CoordinateOrigin)k, split_start);
      end_points[k] = get_interpolated_point((CoordinateOrigin)k, split_end);
      proj_points[k] = start_points[k];
      proj_points[k + 4] = end_points[k];
    }

    // Compute approximate split mid point
    LPoint3 split_mid = get_average_of_points(start_points, end_points);
    LPoint3 cam_start = split_mid + light_vector * _sun_distance;

    // Reset the film size, offset and far-plane
    Camera* cam = DCAST(Camera, _cam_nodes[i].node());
    cam->get_lens()->set_film_size(1, 1);
    cam->get_lens()->set_film_offset(0, 0);
    cam->get_lens()->set_near_far(1, 100);

    // Find a good initial position
    _cam_nodes[i].set_pos(cam_start);
    _cam_nodes[i].look_at(split_mid);

    LVecBase3 best_min_extent, best_max_extent;

    // Find minimum and maximum extents of the points
    LMatrix4 merged_transform = _parent.get_transform(_cam_nodes[i])->get_mat();
    find_min_max_extents(best_min_extent, best_max_extent, merged_transform, proj_points, cam);

    // Find the film size to cover all points
    LVecBase2 film_size, film_offset;
    get_film_properties(film_size, film_offset, best_min_extent, best_max_extent);

    if (_use_fixed_film_size) {
      // In case we use a fixed film size, store the maximum film size, and
      // only change the film size if a new maximum is there
      if (_max_film_sizes[i].get_x() < film_size.get_x()) _max_film_sizes[i].set_x(film_size.get_x());
      if (_max_film_sizes[i].get_y() < film_size.get_y()) _max_film_sizes[i].set_y(film_size.get_y());

      cam->get_lens()->set_film_size(_max_film_sizes[i] * filmsize_bias);
    } else {
      // If we don't use a fixed film size, we can just set the film size
      // on the lens.
      cam->get_lens()->set_film_size(film_size * filmsize_bias);
    }

    // Compute new film offset
    cam->get_lens()->set_film_offset(film_offset);
    cam->get_lens()->set_near_far(10, best_max_extent.get_z());
    _camera_nearfar[i] = LVecBase2(10, best_max_extent.get_z());

    // Compute the camera MVP
    LMatrix4 mvp = compute_mvp(i);

    // Stable CSM Snapping
    if (_use_stable_csm) {
      LPoint3 snap_offset = get_snap_offset(mvp, _resolution);
      _cam_nodes[i].set_pos(_cam_nodes[i].get_pos() + snap_offset);

      // Compute the new mvp, since we changed the snap offset
      mvp = compute_mvp(i);
    }

    _camera_mvps.set_element(i, mvp);
  }
}


/**
 * @brief Updates the PSSM camera rig
 * @details This updates the rig with an updated camera position, and a given
 *   light vector. This should be called on a per-frame basis. It will reposition
 *   all camera sources to fit the frustum based on the pssm distribution.
 *
 *   The light vector should be the vector from the light source, not the
 *   vector to the light source.
 *
 * @param cam_node Target camera node
 * @param light_vector The vector from the light to any point
 */
void PSSMCameraRig::update(NodePath cam_node, const LVecBase3 &light_vector) {
  nassertv(!cam_node.is_empty());
  _update_collector.start();

  // Get camera node transform
  LMatrix4 transform = cam_node.get_transform()->get_mat();

  // Get Camera and Lens pointers
  Camera* cam = DCAST(Camera, cam_node.get_child(0).node());
  nassertv(cam != nullptr);
  Lens* lens = cam->get_lens();

  // Extract near and far points:
  lens->extrude(LPoint2(-1, 1),  _curr_near_points[UpperLeft],  _curr_far_points[UpperLeft]);
  lens->extrude(LPoint2(1, 1),   _curr_near_points[UpperRight], _curr_far_points[UpperRight]);
  lens->extrude(LPoint2(-1, -1), _curr_near_points[LowerLeft],  _curr_far_points[LowerLeft]);
  lens->extrude(LPoint2(1, -1),  _curr_near_points[LowerRight], _curr_far_points[LowerRight]);

  // Construct MVP to project points to world space
  LMatrix4 mvp = transform * lens->get_view_mat();

  // Project all points to world space
  for (size_t i = 0; i < 4; ++i) {
    LPoint4 ws_near = mvp.xform(_curr_near_points[i]);
    LPoint4 ws_far = mvp.xform(_curr_far_points[i]);
    _curr_near_points[i].set(ws_near.get_x(), ws_near.get_y(), ws_near.get_z());
    _curr_far_points[i].set(ws_far.get_x(), ws_far.get_y(), ws_far.get_z());
  }

  // Do the actual PSSM
  compute_pssm_splits( transform, _pssm_distance / lens->get_far(), light_vector );

  _update_collector.stop();
}

