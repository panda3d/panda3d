// Filename: chatInput.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef CHATINPUT_H
#define CHATINPUT_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <dataNode.h>
#include <pointerTo.h>
#include <textNode.h>
#include <nodeAttributes.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : ChatInput
// Description : Reads keyboard input in as that of a chat.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ChatInput : public DataNode {
PUBLISHED:
  ChatInput(TextNode* text_node, const string& name = "");
  void reset(void);

  INLINE void set_max_chars(int max_chars);
  INLINE void clear_max_chars();
  INLINE bool has_max_chars() const;
  INLINE int get_max_chars() const;

  INLINE const string &get_string() const;

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:
  virtual void
  transmit_data(NodeAttributes &data);

  void append(const string &str);

  NodeAttributes _attrib;

  // inputs
  static TypeHandle _button_events_type;

protected:
  PT(TextNode)			_text_node;
  string			_str;
  int                           _max_chars;
  bool                          _has_max_chars;
	
public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "chatInput.I"

#endif
