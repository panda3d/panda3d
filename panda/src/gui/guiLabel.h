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

class GuiLabel {
private:
  enum LabelType { NONE, SIMPLE_TEXTURE, SIMPLE_TEXT };
  LabelType _type;
  PT_Node _geom;
  RenderRelation* _arc;

  INLINE Node* get_geometry(void) const;
  INLINE void set_arc(RenderRelation*);
  INLINE RenderRelation* get_arc(void) const;

  friend GuiManager;
public:
  INLINE GuiLabel(void);
  virtual ~GuiLabel(void);

  static GuiLabel* make_simple_texture_label(void);
  static GuiLabel* make_simple_text_label(const string&, Node*);

  void get_extents(float&, float&, float&, float&);
};

#include "guiLabel.I"

#endif /* __GUILABEL_H__ */
