// Filename: xFile.h
// Created by:  drose (03Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef XFILE_H
#define XFILE_H

#include "pandatoolbase.h"
#include "xFileNode.h"
#include "windowsGuid.h"
#include "filename.h"
#include "pmap.h"
#include "pointerTo.h"

class XFileTemplate;
class XFileDataNodeTemplate;

////////////////////////////////////////////////////////////////////
//       Class : XFile
// Description : This represents the complete contents of an X file
//               (file.x) in memory.  It may be read or written from
//               or to a disk file.
////////////////////////////////////////////////////////////////////
class XFile : public XFileNode {
public:
  XFile();
  ~XFile();

  virtual void clear();

  bool read(Filename filename);
  bool read(istream &in, const string &filename = string());

  bool write(Filename filename) const;
  bool write(ostream &out) const;

  XFileTemplate *find_template(const string &name) const;
  XFileTemplate *find_template(const WindowsGuid &guid) const;

  XFileDataNodeTemplate *find_data_object(const string &name) const;
  XFileDataNodeTemplate *find_data_object(const WindowsGuid &guid) const;

  virtual void write_text(ostream &out, int indent_level) const;

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
  bool read_header(istream &in);
  bool write_header(ostream &out) const;
  
  static const XFile *get_standard_templates();

  int _major_version, _minor_version;
  FormatType _format_type;
  FloatSize _float_size;

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


