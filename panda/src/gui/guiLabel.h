// Filename: guiLabel.h
// Created by:  cary (26Oct00)
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

class EXPCL_PANDA GuiLabel : public TypedReferenceCount {
private:
  enum LabelType { NONE, SIMPLE_TEXTURE, SIMPLE_TEXT, SIMPLE_CARD };
  LabelType _type;
  PT_Node _geom;
  RenderRelation* _arc;
  PT(Texture) _tex;
  RenderRelation* _internal;
  Geom* _gset;

  float _scale;
  LVector3f _pos;
  Colorf _foreground;
  bool _have_background;
  Colorf _background;
  bool _have_width;
  float _width;
  bool _have_height;
  float _height;
  float _depth;

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
  static GuiLabel* make_simple_text_label(const string&, Node*,
					  Texture* = (Texture*)0L);
  static GuiLabel* make_simple_card_label(void);

  int freeze();
  int thaw();

  void get_extents(float&, float&, float&, float&);
  float get_width(void);
  float get_height(void);
  INLINE float get_depth(void) const;

  INLINE void set_width(float);
  INLINE void set_height(float);

  INLINE void set_scale(float);
  INLINE void set_pos(float, float, float);
  INLINE void set_pos(const LVector3f&);
  INLINE void set_depth(float);

  INLINE float get_scale(void) const;
  INLINE LVector3f get_pos(void) const;

  INLINE void set_foreground_color(float, float, float, float);
  void set_foreground_color(const Colorf&);
  INLINE void set_background_color(float, float, float, float);
  void set_background_color(const Colorf&);

  INLINE Colorf get_foreground_color(void) const;
  INLINE Colorf get_background_color(void) const;

  void set_text(const string&);

  INLINE void recompute(void);

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
