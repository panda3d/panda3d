// Filename: guiLabel.h
// Created by:  cary (26Oct00)
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

#ifndef __GUILABEL_H__
#define __GUILABEL_H__

#include "config_gui.h"

#include <pandabase.h>
#include <node.h>
#include <pt_Node.h>
#include <renderRelation.h>
#include <texture.h>
#include <typedReferenceCount.h>
#include <geom.h>

// label-ish behavior for GUI objects (labels, buttons, rollovers)

class GuiManager;
class TextFont;

class EXPCL_PANDA GuiLabel : public TypedReferenceCount {
PUBLISHED:
  enum PriorityType { P_NONE, P_LOWEST, P_LOWER, P_HIGHER, P_HIGHEST };
private:
  typedef map<GuiLabel*, PriorityType> PriorityMap;
  enum LabelType { NONE, L_NULL, SIMPLE_TEXTURE, SIMPLE_TEXT, SIMPLE_CARD, MODEL };

  LabelType _type;
  PT_Node _geom;
  RenderRelation* _arc;
  PT(Texture) _tex;
  RenderRelation* _internal;
  Geom* _gset;
  float _model_width, _model_height;

  float _scale;
  float _scale_x, _scale_y, _scale_z;
  LVector3f _pos;
  LVector3f _model_pos;
  bool _have_foreground;
  Colorf _foreground;
  bool _have_background;
  Colorf _background;
  bool _have_width;
  float _width;
  bool _have_height;
  float _height;
  bool _mirror_x;
  bool _mirror_y;

  PriorityMap _priorities;
  int _hard_pri;
  bool _highest_pri;
  bool _lowest_pri;
  bool _has_hard_pri;

  INLINE Node* get_geometry(void) const;
  INLINE void set_arc(RenderRelation*);
  INLINE RenderRelation* get_arc(void) const;

  friend GuiManager;

  void recompute_transform(void);
  void set_properties(void);

PUBLISHED:
  INLINE GuiLabel(void);
  virtual ~GuiLabel(void);

  static GuiLabel* make_simple_texture_label(Texture*);
  static GuiLabel* make_simple_text_label(const string&, TextFont*,
                                          Texture* = (Texture*)0L);
  static GuiLabel* make_simple_card_label(void);
  static GuiLabel* make_null_label(void);
  static GuiLabel* make_model_label(Node*, float, float);
  static GuiLabel* make_model_label(Node*, float, float, float, float);

  int freeze();
  int thaw();

  void get_extents(float&, float&, float&, float&);
  float get_width(void);
  float get_height(void);

  INLINE void set_width(float);
  INLINE void set_height(float);

  INLINE void set_scale(float);
  INLINE void set_scale(float, float, float);
  INLINE void set_mirror_x(bool);
  INLINE void set_mirror_y(bool);
  INLINE void set_pos(float, float, float);
  INLINE void set_pos(const LVector3f&);

  INLINE float get_scale(void) const;
  INLINE bool get_mirror_x(void) const;
  INLINE bool get_mirror_y(void) const;
  INLINE LVector3f get_pos(void) const;

  INLINE void set_foreground_color(float, float, float, float);
  void set_foreground_color(const Colorf&);
  INLINE void set_background_color(float, float, float, float);
  void set_background_color(const Colorf&);

  INLINE Colorf get_foreground_color(void) const;
  INLINE Colorf get_background_color(void) const;

  void set_text(const string&);
  INLINE void set_shadow_color(float, float, float, float);
  void set_shadow_color(const Colorf&);
  void set_shadow(float, float);
  void set_align(int);

  INLINE void recompute(void);

  // used for the priority system
  bool operator<(const GuiLabel&) const;
  INLINE void set_priority(GuiLabel*, const PriorityType);
  int soft_set_draw_order(int);
  int set_draw_order(int);
  INLINE bool has_hard_draw_order(void) const;
  INLINE int get_draw_order(void) const;

  void write(ostream&) const;

public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GuiLabel",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#include "guiLabel.I"

#endif /* __GUILABEL_H__ */
