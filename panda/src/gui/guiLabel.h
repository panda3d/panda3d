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

// label-ish behavior for GUI objects (labels, buttons, rollovers)

class GuiManager;

class EXPCL_PANDA GuiLabel {
private:
  enum LabelType { NONE, SIMPLE_TEXTURE, SIMPLE_TEXT };
  LabelType _type;
  PT_Node _geom;
  RenderRelation* _arc;

  float _scale;
  LVector3f _pos;

  INLINE Node* get_geometry(void) const;
  INLINE void set_arc(RenderRelation*);
  INLINE RenderRelation* get_arc(void) const;

  friend GuiManager;

  void recompute_transform(void);
public:
  INLINE GuiLabel(void);
  virtual ~GuiLabel(void);

  static GuiLabel* make_simple_texture_label(void);
  static GuiLabel* make_simple_text_label(const string&, Node*);

  void get_extents(float&, float&, float&, float&);

  INLINE void set_scale(float);
  INLINE void set_pos(const LVector3f&);

  INLINE float get_scale(void) const;
  INLINE LVector3f get_pos(void) const;
};

#include "guiLabel.I"

#endif /* __GUILABEL_H__ */
