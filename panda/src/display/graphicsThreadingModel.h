/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsThreadingModel.h
 * @author drose
 * @date 2003-01-27
 */

#ifndef GRAPHICSTHREADINGMODEL_H
#define GRAPHICSTHREADINGMODEL_H

#include "pandabase.h"

/**
 * This represents the user's specification of how a particular frame is
 * handled by the various threads.
 */
class EXPCL_PANDA_DISPLAY GraphicsThreadingModel {
PUBLISHED:
  GraphicsThreadingModel(const std::string &model = std::string());
  INLINE GraphicsThreadingModel(const GraphicsThreadingModel &copy);
  INLINE void operator = (const GraphicsThreadingModel &copy);

  std::string get_model() const;
  INLINE const std::string &get_cull_name() const;
  INLINE void set_cull_name(const std::string &cull_name);
  INLINE int get_cull_stage() const;

  INLINE const std::string &get_draw_name() const;
  INLINE void set_draw_name(const std::string &cull_name);
  INLINE int get_draw_stage() const;

  INLINE bool get_cull_sorting() const;
  INLINE void set_cull_sorting(bool cull_sorting);

  INLINE bool is_single_threaded() const;
  INLINE bool is_default() const;
  INLINE void output(std::ostream &out) const;

private:
  void update_stages();

private:
  std::string _cull_name;
  int _cull_stage;
  std::string _draw_name;
  int _draw_stage;
  bool _cull_sorting;
};

INLINE std::ostream &operator << (std::ostream &out, const GraphicsThreadingModel &threading_model);

#include "graphicsThreadingModel.I"

#endif
