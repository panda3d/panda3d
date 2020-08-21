/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileNode.cxx
 * @author drose
 * @date 2004-10-03
 */

#include "xFileNode.h"
#include "windowsGuid.h"
#include "xFile.h"
#include "xLexerDefs.h"
#include "xFileParseData.h"
#include "xFile.h"
#include "xFileDataNodeTemplate.h"
#include "filename.h"
#include "string_utils.h"

using std::string;

TypeHandle XFileNode::_type_handle;

/**
 *
 */
XFileNode::
XFileNode(XFile *x_file, const string &name) :
  Namable(),
  _x_file(x_file)
{
  if (x_file && x_file->_keep_names) {
    set_name(name);
  } else {
    set_name(make_nice_name(name));
  }
}

/**
 *
 */
XFileNode::
~XFileNode() {
  clear();
}

/**
 * Returns the child with the indicated name, if any, or NULL if none.
 */
XFileNode *XFileNode::
find_child(const string &name) const {
  ChildrenByName::const_iterator ni;
  ni = _children_by_name.find(downcase(name));
  if (ni != _children_by_name.end()) {
    return get_child((*ni).second);
  }

  return nullptr;
}

/**
 * Returns the index number of the child with the indicated name, if any, or
 * -1 if none.
 */
int XFileNode::
find_child_index(const string &name) const {
  ChildrenByName::const_iterator ni;
  ni = _children_by_name.find(downcase(name));
  if (ni != _children_by_name.end()) {
    return (*ni).second;
  }

  return -1;
}

/**
 * Returns the index number of the indicated child, or -1 if none.
 */
int XFileNode::
find_child_index(const XFileNode *child) const {
  for (int i = 0; i < (int)_children.size(); i++) {
    if (_children[i] == child) {
      return i;
    }
  }

  return -1;
}

/**
 * Returns the first child or descendent found with the indicated name after a
 * depth-first search, if any, or NULL if none.
 */
XFileNode *XFileNode::
find_descendent(const string &name) const {
  XFileNode *child = find_child(name);
  if (child != nullptr) {
    return child;
  }

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    XFileNode *child = (*ci)->find_descendent(name);
    if (child != nullptr){
      return child;
    }
  }

  return nullptr;
}

/**
 * Returns true if this node has a GUID associated.
 */
bool XFileNode::
has_guid() const {
  return false;
}

/**
 * If has_guid() returned true, returns the particular GUID associated with
 * this node.
 */
const WindowsGuid &XFileNode::
get_guid() const {
  static WindowsGuid empty;
  return empty;
}

/**
 * Returns true if this node represents the definition of some template.  This
 * is the template definition, not an actual data object that represents an
 * instance of the template.  If the file strictly uses standard templates,
 * the presence of template definitions is optional.
 *
 * If this returns true, the node must be of type XFileTemplate.
 */
bool XFileNode::
is_template_def() const {
  return false;
}

/**
 * Returns true if this node represents an indirect reference to an object
 * defined previously in the file.  References are generally transparent, so
 * in most cases you never need to call this, unless you actually need to
 * differentiate between references and instances; you can simply use the
 * reference node as if it were itself the object it references.
 *
 * If this returns true, the node must be of type XFileDataNodeReference.
 */
bool XFileNode::
is_reference() const {
  return false;
}

/**
 * Returns true if this node represents a data object that is the instance of
 * some template, or false otherwise.  This also returns true for references
 * to objects (which are generally treated just like the objects themselves).
 *
 * If this returns true, the node must be of type XFileDataNode (it is either
 * an XFileDataNodeTemplate or an XFileDataNodeReference).
 */
bool XFileNode::
is_object() const {
  return false;
}

/**
 * Returns true if this node represents an instance of the standard template
 * with the indicated name, or false otherwise.  This returns also returns
 * true for references to standard objects.
 *
 * If this returns true, the node must be of type XFileDataNode (it is either
 * an XFileDataNodeTemplate or an XFileDataNodeReference).
 */
bool XFileNode::
is_standard_object(const string &template_name) const {
  return false;
}

/**
 * Adds the indicated node as a child of this node.
 */
void XFileNode::
add_child(XFileNode *node) {
  if (node->has_name()) {
    _children_by_name[downcase(node->get_name())] = (int)_children.size();
  }
  if (node->has_guid()) {
    _x_file->_nodes_by_guid[node->get_guid()] = node;
  }
  if (node->is_of_type(XFileDataNode::get_class_type())) {
    _objects.push_back(DCAST(XFileDataNode, node));
  }
  _children.push_back(node);
}

/**
 * Removes all children from the node, and otherwise resets it to its initial
 * state.
 */
void XFileNode::
clear() {
  _children.clear();
  _objects.clear();
  _children_by_name.clear();
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileNode::
write_text(std::ostream &out, int indent_level) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write_text(out, indent_level);
  }
}

/**
 * This is called on the template that defines an object, once the data for
 * the object has been parsed.  It is responsible for identifying which
 * component of the template owns each data element, and packing the data
 * elements appropriately back into the object.
 *
 * It returns true on success, or false on an error (e.g.  not enough data
 * elements, mismatched data type).
 */
bool XFileNode::
repack_data(XFileDataObject *object,
            const XFileParseDataList &parse_data_list,
            XFileNode::PrevData &prev_data,
            size_t &index, size_t &sub_index) const {
  // This method should be specialized for data types that actually consume a
  // data element.  Here in the base class, it just walks through its
  // children, asking each one to pull off the appropriate number of data
  // elements.

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    if (!(*ci)->repack_data(object, parse_data_list,
                            prev_data, index, sub_index)) {
      return false;
    }
  }

  return true;
}

/**
 * This is similar to repack_data(), except it is used to fill the initial
 * values for a newly-created template object to zero.
 */
bool XFileNode::
fill_zero_data(XFileDataObject *object) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    if (!(*ci)->fill_zero_data(object)) {
      return false;
    }
  }

  return true;
}

/**
 * Returns true if the node, particularly a template node, is structurally
 * equivalent to the other node (which must be of the same type).  This checks
 * data element types, but does not compare data element names.
 */
bool XFileNode::
matches(const XFileNode *other) const {
  if (other->get_type() != get_type()) {
    return false;
  }

  if (other->get_num_children() != get_num_children()) {
    return false;
  }

  for (int i = 0; i < get_num_children(); i++) {
    if (!get_child(i)->matches(other->get_child(i))) {
      return false;
    }
  }

  return true;
}

/**
 * Creates a new Mesh instance, as a child of this node.
 */
XFileDataNode *XFileNode::
add_Mesh(const string &name) {
  XFileTemplate *xtemplate = XFile::find_standard_template("Mesh");
  nassertr(xtemplate != nullptr, nullptr);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(get_x_file(), name, xtemplate);
  add_child(node);
  node->zero_fill();

  return node;
}

/**
 * Creates a new MeshNormals instance, as a child of this node.
 */
XFileDataNode *XFileNode::
add_MeshNormals(const string &name) {
  XFileTemplate *xtemplate = XFile::find_standard_template("MeshNormals");
  nassertr(xtemplate != nullptr, nullptr);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(get_x_file(), name, xtemplate);
  add_child(node);
  node->zero_fill();

  return node;
}

/**
 * Creates a new MeshVertexColors instance, as a child of this node.
 */
XFileDataNode *XFileNode::
add_MeshVertexColors(const string &name) {
  XFileTemplate *xtemplate = XFile::find_standard_template("MeshVertexColors");
  nassertr(xtemplate != nullptr, nullptr);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(get_x_file(), name, xtemplate);
  add_child(node);
  node->zero_fill();

  return node;
}

/**
 * Creates a new MeshTextureCoords instance, as a child of this node.
 */
XFileDataNode *XFileNode::
add_MeshTextureCoords(const string &name) {
  XFileTemplate *xtemplate = XFile::find_standard_template("MeshTextureCoords");
  nassertr(xtemplate != nullptr, nullptr);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(get_x_file(), name, xtemplate);
  add_child(node);
  node->zero_fill();

  return node;
}

/**
 * Creates a new MeshMaterialList instance, as a child of this node.
 */
XFileDataNode *XFileNode::
add_MeshMaterialList(const string &name) {
  XFileTemplate *xtemplate = XFile::find_standard_template("MeshMaterialList");
  nassertr(xtemplate != nullptr, nullptr);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(get_x_file(), name, xtemplate);
  add_child(node);
  node->zero_fill();

  return node;
}

/**
 * Creates a new Material instance, as a child of this node.
 */
XFileDataNode *XFileNode::
add_Material(const string &name, const LColor &face_color,
             double power, const LRGBColor &specular_color,
             const LRGBColor &emissive_color) {
  XFileTemplate *xtemplate = XFile::find_standard_template("Material");
  nassertr(xtemplate != nullptr, nullptr);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(get_x_file(), name, xtemplate);
  add_child(node);
  node->zero_fill();

  (*node)["faceColor"]["red"] = face_color[0];
  (*node)["faceColor"]["green"] = face_color[1];
  (*node)["faceColor"]["blue"] = face_color[2];
  (*node)["faceColor"]["alpha"] = face_color[3];
  (*node)["power"] = power;
  (*node)["specularColor"]["red"] = specular_color[0];
  (*node)["specularColor"]["green"] = specular_color[1];
  (*node)["specularColor"]["blue"] = specular_color[2];
  (*node)["emissiveColor"]["red"] = emissive_color[0];
  (*node)["emissiveColor"]["green"] = emissive_color[1];
  (*node)["emissiveColor"]["blue"] = emissive_color[2];

  return node;
}

/**
 * Creates a new TextureFilename instance, as a child of this node.
 */
XFileDataNode *XFileNode::
add_TextureFilename(const string &name, const Filename &filename) {
  XFileTemplate *xtemplate = XFile::find_standard_template("TextureFilename");
  nassertr(xtemplate != nullptr, nullptr);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(get_x_file(), name, xtemplate);
  add_child(node);
  node->zero_fill();

  (*node)["filename"] = filename.to_os_specific();

  return node;
}

/**
 * Creates a new Frame instance, as a child of this node.
 */
XFileDataNode *XFileNode::
add_Frame(const string &name) {
  XFileTemplate *xtemplate = XFile::find_standard_template("Frame");
  nassertr(xtemplate != nullptr, nullptr);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(get_x_file(), name, xtemplate);
  add_child(node);
  node->zero_fill();

  return node;
}

/**
 * Creates a new FrameTransformMatrix instance, as a child of this node.
 */
XFileDataNode *XFileNode::
add_FrameTransformMatrix(const LMatrix4d &mat) {
  XFileTemplate *xtemplate =
    XFile::find_standard_template("FrameTransformMatrix");
  nassertr(xtemplate != nullptr, nullptr);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(get_x_file(), "", xtemplate);
  add_child(node);
  node->zero_fill();

  XFileDataObject &xmat = (*node)["frameMatrix"]["matrix"];
  xmat[0] = mat(0, 0);
  xmat[1] = mat(0, 1);
  xmat[2] = mat(0, 2);
  xmat[3] = mat(0, 3);

  xmat[4] = mat(1, 0);
  xmat[5] = mat(1, 1);
  xmat[6] = mat(1, 2);
  xmat[7] = mat(1, 3);

  xmat[8] = mat(2, 0);
  xmat[9] = mat(2, 1);
  xmat[10] = mat(2, 2);
  xmat[11] = mat(2, 3);

  xmat[12] = mat(3, 0);
  xmat[13] = mat(3, 1);
  xmat[14] = mat(3, 2);
  xmat[15] = mat(3, 3);

  return node;
}

/**
 * Transforms the indicated egg name to a name that is acceptable for a node
 * in the X File format.
 */
string XFileNode::
make_nice_name(const string &str) {
  string result;

  string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    if (isalnum(*si)) {
      result += (*si);
    } else {
      switch (*si) {
      case '-':
        result += (*si);
        break;
      default:
        result += "_";
      }
    }
  }

  if (str.empty() || isdigit(str[0])) {
    // If the name begins with a digit, or if it is empty, then we must make
    // it begin with something else, like for instance an underscore.
    result = '_' + result;
  }

  return result;
}
