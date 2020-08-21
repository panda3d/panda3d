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

#ifndef PSSMCAMERARIG_H
#define PSSMCAMERARIG_H

#include "pandabase.h"
#include "luse.h"
#include "camera.h"
#include "nodePath.h"
#include "pStatCollector.h"

#include <vector>

/**
 * @brief Main class used for handling PSSM
 * @details This is the main class for supporting PSSM, it is used by the PSSM
 *   plugin to compute the position of the splits.
 *
 *   It supports handling a varying amount of cameras, and fitting those cameras
 *   into the main camera frustum, to render distant shadows. It also supports
 *   various optimizations for fitting the frustum, e.g. rotating the sources
 *   to get a better coverage.
 *
 *   It also provides methods to get arrays of data about the used cameras
 *   view-projection matrices and their near and far plane, which is required for
 *   processing the data in the shadow sampling shader.
 *
 *   In this class, there is often referred to "Splits" or also called "Cascades".
 *   These denote the different cameras which are used to split the frustum,
 *   and are a common term related to the PSSM algorithm.
 *
 *   To understand the functionality of this class, a detailed knowledge of the
 *   PSSM algorithm is helpful.
 */
class PSSMCameraRig {
PUBLISHED:
  PSSMCameraRig(size_t num_splits);
  ~PSSMCameraRig();

  inline void set_pssm_distance(float distance);
  inline void set_sun_distance(float distance);
  inline void set_use_fixed_film_size(bool flag);
  inline void set_resolution(size_t resolution);
  inline void set_use_stable_csm(bool flag);
  inline void set_logarithmic_factor(float factor);
  inline void set_border_bias(float bias);

  void update(NodePath cam_node, const LVecBase3 &light_vector);
  inline void reset_film_size_cache();

  inline NodePath get_camera(size_t index);

  void reparent_to(NodePath parent);
  inline const PTA_LMatrix4 &get_mvp_array();
  inline const PTA_LVecBase2 &get_nearfar_array();

public:
  // Used to access the near and far points in the array
  enum CoordinateOrigin {
    UpperLeft = 0,
    UpperRight,
    LowerLeft,
    LowerRight
  };

protected:
  void init_cam_nodes();
  void compute_pssm_splits(const LMatrix4& transform, float max_distance,
               const LVecBase3 &light_vector);
  inline float get_split_start(size_t split_index);
  LMatrix4 compute_mvp(size_t cam_index);
  inline LPoint3 get_interpolated_point(CoordinateOrigin origin, float depth);
  LVecBase3 get_snap_offset(const LMatrix4& mat, size_t resolution);

  std::vector<NodePath> _cam_nodes;
  std::vector<Camera*> _cameras;
  std::vector<LVecBase2> _max_film_sizes;

  // Current near and far points
  // Order: UL, UR, LL, LR (See CoordinateOrigin)
  LPoint3 _curr_near_points[4];
  LPoint3 _curr_far_points[4];
  float _pssm_distance;
  float _sun_distance;
  float _logarithmic_factor;
  float _border_bias;
  bool _use_fixed_film_size;
  bool _use_stable_csm;
  size_t _resolution;
  size_t _num_splits;
  NodePath _parent;

  PTA_LMatrix4 _camera_mvps;
  PTA_LVecBase2 _camera_nearfar;

  static PStatCollector _update_collector;
};

#include "pssmCameraRig.I"

#endif // PSSMCAMERARIG_H
