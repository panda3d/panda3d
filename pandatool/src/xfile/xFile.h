/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFile.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef XFILE_H
#define XFILE_H

#include "pandatoolbase.h"
#include "xFileNode.h"
#include "xFileDataNode.h"
#include "windowsGuid.h"
#include "filename.h"
#include "pmap.h"
#include "pointerTo.h"

class XFileTemplate;
class XFileDataNodeTemplate;

/**
 * This represents the complete contents of an X file (file.x) in memory.  It
 * may be read or written from or to a disk file.
 */
class XFile : public XFileNode {
public:
  XFile(bool keep_names=false);
  ~XFile();

  virtual void clear();

  bool read(Filename filename);
  bool read(std::istream &in, const std::string &filename = std::string());

  bool write(Filename filename) const;
  bool write(std::ostream &out) const;

  XFileTemplate *find_template(const std::string &name) const;
  XFileTemplate *find_template(const WindowsGuid &guid) const;

  static XFileTemplate *find_standard_template(const std::string &name);
  static XFileTemplate *find_standard_template(const WindowsGuid &guid);

  XFileDataNodeTemplate *find_data_object(const std::string &name) const;
  XFileDataNodeTemplate *find_data_object(const WindowsGuid &guid) const;

  virtual void write_text(std::ostream &out, int indent_level) const;

  enum FormatType {
    FT_text,
    FT_binary,
    FT_compressed,
  };
  enum FloatSize {
    FS_32,
    FS_64,
  };

private:
  bool read_header(std::istream &in);
  bool write_header(std::ostream &out) const;

  static const XFile *get_standard_templates();

  int _major_version, _minor_version;
  FormatType _format_type;
  FloatSize _float_size;
  bool _keep_names;

  typedef pmap<WindowsGuid, XFileNode *> NodesByGuid;
  NodesByGuid _nodes_by_guid;

  static PT(XFile) _standard_templates;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileNode::init_type();
    register_type(_type_handle, "XFile",
                  XFileNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class XFileNode;
};

#include "xFile.I"

#endif
