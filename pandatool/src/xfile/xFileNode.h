/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileNode.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef XFILENODE_H
#define XFILENODE_H

#include "pandatoolbase.h"
#include "typedObject.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "namable.h"
#include "pnotify.h"
#include "pvector.h"
#include "pmap.h"
#include "luse.h"

class XFile;
class WindowsGuid;
class XFileParseDataList;
class XFileDataDef;
class XFileDataObject;
class XFileDataNode;
class XFileDataNodeTemplate;
class Filename;

/**
 * A single node of an X file.  This may be either a template or a data node.
 */
class XFileNode : public TypedObject, public Namable,
                  virtual public ReferenceCount {
protected:
  INLINE XFileNode(XFile *x_file);

public:
  XFileNode(XFile *x_file, const std::string &name);
  virtual ~XFileNode();

  INLINE XFile *get_x_file() const;

  INLINE int get_num_children() const;
  INLINE XFileNode *get_child(int n) const;
  XFileNode *find_child(const std::string &name) const;
  int find_child_index(const std::string &name) const;
  int find_child_index(const XFileNode *child) const;
  XFileNode *find_descendent(const std::string &name) const;

  INLINE int get_num_objects() const;
  INLINE XFileDataNode *get_object(int n) const;

  virtual bool has_guid() const;
  virtual const WindowsGuid &get_guid() const;

  virtual bool is_template_def() const;
  virtual bool is_reference() const;
  virtual bool is_object() const;
  virtual bool is_standard_object(const std::string &template_name) const;

  void add_child(XFileNode *node);
  virtual void clear();

  virtual void write_text(std::ostream &out, int indent_level) const;

  typedef pmap<const XFileDataDef *, XFileDataObject *> PrevData;

  virtual bool repack_data(XFileDataObject *object,
                           const XFileParseDataList &parse_data_list,
                           PrevData &prev_data,
                           size_t &index, size_t &sub_index) const;

  virtual bool fill_zero_data(XFileDataObject *object) const;

  virtual bool matches(const XFileNode *other) const;

  // The following methods can be used to create instances of the standard
  // template objects.  These definitions match those defined in
  // standardTemplates.x in this directory (and compiled into the executable).
  XFileDataNode *add_Mesh(const std::string &name);
  XFileDataNode *add_MeshNormals(const std::string &name);
  XFileDataNode *add_MeshVertexColors(const std::string &name);
  XFileDataNode *add_MeshTextureCoords(const std::string &name);
  XFileDataNode *add_MeshMaterialList(const std::string &name);
  XFileDataNode *add_Material(const std::string &name, const LColor &face_color,
                              double power, const LRGBColor &specular_color,
                              const LRGBColor &emissive_color);
  XFileDataNode *add_TextureFilename(const std::string &name,
                                     const Filename &filename);
  XFileDataNode *add_Frame(const std::string &name);
  XFileDataNode *add_FrameTransformMatrix(const LMatrix4d &mat);

public:
  static std::string make_nice_name(const std::string &str);

protected:
  XFile *_x_file;

  typedef pvector< PT(XFileNode) > Children;
  Children _children;

  typedef pvector<XFileDataNode *> Objects;
  Objects _objects;

  typedef pmap<std::string, int> ChildrenByName;
  ChildrenByName _children_by_name;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "XFileNode",
                  TypedObject::get_class_type(),
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class XFileDataNodeReference;
};

#include "xFileNode.I"

#endif
