// Filename: textNode.h
// Created by:  drose (12May99)
//
////////////////////////////////////////////////////////////////////
//
#ifndef TEXTNODE_H
#define TEXTNODE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <pt_Node.h>
#include <namedNode.h>
#include <luse.h>
#include <geom.h>
#include <geomNode.h>
#include <renderRelation.h>
#include <textureTransition.h>
#include <transparencyTransition.h>
#include <allTransitionsWrapper.h>

#include <map>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
BEGIN_PUBLISH
#define TM_ALIGN_LEFT         1
#define TM_ALIGN_RIGHT        2
#define TM_ALIGN_CENTER       3
END_PUBLISH

////////////////////////////////////////////////////////////////////
//       Class : TextNode
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextNode : public NamedNode {
PUBLISHED:
  TextNode(const string &name = "");
  ~TextNode();

  INLINE int freeze();
  INLINE int get_freeze_level() const;
  INLINE int thaw();

  INLINE void set_font(Node *font_def);
  INLINE Node *get_font() const;

  INLINE float get_line_height() const;

  INLINE void set_slant(float slant);
  INLINE float get_slant() const;

  INLINE void set_align(int align_type);
  INLINE int get_align() const;

  INLINE void set_wordwrap(float width);
  INLINE void clear_wordwrap();
  INLINE bool has_wordwrap() const;
  INLINE float get_wordwrap() const;

  INLINE void set_text_color(float r, float g, float b, float a);
  INLINE void set_text_color(const Colorf &text_color);
  INLINE void clear_text_color();
  INLINE bool has_text_color() const;
  INLINE Colorf get_text_color() const;

  INLINE void set_frame_color(float r, float g, float b, float a);
  INLINE void set_frame_color(const Colorf &frame_color);
  INLINE Colorf get_frame_color() const;

  INLINE void set_card_border(float size, float uv_portion);
  INLINE void clear_card_border();
  INLINE float get_card_border_size() const;
  INLINE float get_card_border_uv_portion() const;
  INLINE bool has_card_border() const;

  INLINE void set_card_color(float r, float g, float b, float a);
  INLINE void set_card_color(const Colorf &card_color);
  INLINE Colorf get_card_color() const;

  INLINE void set_card_texture(Texture *card_texture);
  INLINE void clear_card_texture();
  INLINE bool has_card_texture() const;
  INLINE Texture *get_card_texture() const;

  INLINE void set_shadow_color(float r, float g, float b, float a);
  INLINE void set_shadow_color(const Colorf &shadow_color);
  INLINE Colorf get_shadow_color() const;
 
  INLINE void set_frame_as_margin(float left, float right, 
				  float bottom, float top);
  INLINE void set_frame_actual(float left, float right,
			       float bottom, float top);
  INLINE void clear_frame();
  INLINE bool has_frame() const;
  INLINE bool is_frame_as_margin() const;
  INLINE LVecBase4f get_frame_as_set() const;
  INLINE LVecBase4f get_frame_actual() const;

  INLINE void set_frame_line_width(float line_width);
  INLINE float get_frame_line_width() const;
  INLINE void set_frame_corners(bool corners);
  INLINE bool get_frame_corners() const;

  INLINE void set_card_as_margin(float left, float right, 
				 float bottom, float top);
  INLINE void set_card_actual(float left, float right, 
			      float bottom, float top);
  INLINE void clear_card();
  INLINE bool has_card() const;
  INLINE bool is_card_as_margin() const;
  INLINE LVecBase4f get_card_as_set() const;
  INLINE LVecBase4f get_card_actual() const;
  INLINE LVecBase4f get_card_transformed() const;

  INLINE void set_shadow(float xoffset, float yoffset);
  INLINE void clear_shadow();
  INLINE bool has_shadow() const;
  INLINE LVecBase2f get_shadow() const;

  INLINE void set_draw_order(int draw_order);
  INLINE int get_draw_order() const;

  INLINE void set_billboard(bool billboard);
  INLINE bool get_billboard() const;

  INLINE void set_transform(const LMatrix4f &transform);
  INLINE LMatrix4f get_transform() const;

  INLINE void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;

  INLINE void set_text(const string &str);
  INLINE void clear_text();
  INLINE bool has_text() const;
  INLINE string get_text() const;

  float calc_width(char ch) const;
  float calc_width(const string &line) const;
  string wordwrap_to(const string &text, float wordwrap_width) const;

  void print() const;
  void write(ostream &out) const;

  void rebuild();

  // The following functions return information about the text that
  // was last built (and is currently visible).
  INLINE float get_left() const;
  INLINE float get_right() const;
  INLINE float get_bottom() const;
  INLINE float get_top() const;
  INLINE float get_height() const;
  INLINE float get_width() const;

  INLINE LPoint3f get_upper_left_3d() const;
  INLINE LPoint3f get_lower_right_3d() const;

  INLINE int get_num_rows() const;
  

private:
  void do_rebuild();
  bool find_character_gsets(Node *root, Geom *&ch, GeomPoint *&dot,
			    AllTransitionsWrapper &trans);
  void find_characters(Node *root);
  float assemble_row(const char *&source, Node *dest);
  Node *assemble_text(const char *source, LVector2f &ul, LVector2f &lr,
		      int &num_rows);
  Node *make_frame();
  Node *make_card();
  Node *make_card_with_border();

  class EXPCL_PANDA CharDef {
  public:
    CharDef() { }
    CharDef(Geom *geom, float width, const AllTransitionsWrapper &trans);
    Geom *_geom;
    float _width;
    AllTransitionsWrapper _trans;
  };

  typedef map<int, CharDef> CharDefs;
  CharDefs _defs;
  float _font_height;
  float _slant;

  PT(RenderRelation) _root_arc;

  PT_Node _root;
  PT_Node _text_root;
  PT_Node _frame_root;
  PT_Node _card_root;
  PT_Node _font;

  PT(Texture) _card_texture;
  Colorf _text_color;
  Colorf _shadow_color;
  Colorf _frame_color;
  Colorf _card_color;

  enum Flags {
    F_has_text_color   =  0x0001,
    F_has_wordwrap     =  0x0002,
    F_has_frame        =  0x0004,
    F_frame_as_margin  =  0x0008,
    F_has_card         =  0x0010,
    F_card_as_margin   =  0x0020,
    F_has_card_texture =  0x0040,
    F_has_shadow       =  0x0080,
    F_frame_corners    =  0x0100,
    F_card_transp      =  0x0200,
    F_has_card_border  =  0x0400,
    F_billboard        =  0x0800,
  };

  int _flags;
  int _align;
  float _wordwrap_width;
  float _frame_width;
  float _card_border_size;
  float _card_border_uv_portion;

  LVector2f _frame_ul, _frame_lr;
  LVector2f _card_ul, _card_lr;
  LVector2f _shadow_offset;

  int _draw_order;

  LMatrix4f _transform;
  CoordinateSystem _coordinate_system;
  
  string _text;

  LPoint2f _ul2d, _lr2d;
  LPoint3f _ul3d, _lr3d;
  int _num_rows;
  int _freeze_level;
  bool _needs_rebuild;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NamedNode::init_type();
    register_type(_type_handle, "TextNode",
		  NamedNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  
private:
  static TypeHandle _type_handle;
};

#include "textNode.I"

#endif
