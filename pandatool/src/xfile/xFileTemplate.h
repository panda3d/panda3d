/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileTemplate.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef XFILETEMPLATE_H
#define XFILETEMPLATE_H

#include "pandatoolbase.h"
#include "xFileNode.h"
#include "windowsGuid.h"

class XFileDataDef;

/**
 * A template definition in the X file.  This defines the data structures that
 * may be subsequently read.
 */
class XFileTemplate : public XFileNode {
public:
  XFileTemplate(XFile *x_file, const std::string &name, const WindowsGuid &guid);
  virtual ~XFileTemplate();

  virtual bool has_guid() const;
  virtual const WindowsGuid &get_guid() const;

  virtual bool is_template_def() const;

  virtual void clear();
  virtual void write_text(std::ostream &out, int indent_level) const;

  INLINE bool is_standard() const;

  INLINE void set_open(bool open);
  INLINE bool get_open() const;

  INLINE void add_option(XFileTemplate *option);
  INLINE int get_num_options() const;
  INLINE XFileTemplate *get_option(int n) const;

  virtual bool matches(const XFileNode *other) const;

private:
  WindowsGuid _guid;
  bool _is_standard;
  bool _open;

  typedef pvector< PT(XFileTemplate) > Options;
  Options _options;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileNode::init_type();
    register_type(_type_handle, "XFileTemplate",
                  XFileNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class XFile;
};

#include "xFileTemplate.I"

#endif
