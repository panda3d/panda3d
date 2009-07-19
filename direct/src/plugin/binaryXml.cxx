// Filename: binaryXml.cxx
// Created by:  drose (13Jul09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "binaryXml.h"
#include <sstream>


static const bool debug_xml_output = true;

#define DO_BINARY_XML 1

enum NodeType {
  NT_unknown,
  NT_document,
  NT_element,
  NT_text,
};

////////////////////////////////////////////////////////////////////
//     Function: write_xml_node
//  Description: Recursively writes a node and all of its children to
//               the given stream.
////////////////////////////////////////////////////////////////////
static void
write_xml_node(ostream &out, TiXmlNode *xnode) {
  NodeType type = NT_element;
  if (xnode->ToDocument() != NULL) {
    type = NT_document;
  } else if (xnode->ToElement() != NULL) {
    type = NT_element;
  } else if (xnode->ToText() != NULL) {
    type = NT_text;
  } else {
    type = NT_unknown;
  }

  out.put((char)type);
  // We don't bother to write any data for the unknown types.
  if (type == NT_unknown) {
    return;
  }

  const string &value = xnode->ValueStr();
  size_t value_length = value.length();
  out.write((char *)&value_length, sizeof(value_length));
  out.write(value.data(), value_length);

  if (type == NT_element) {
    // Write the element attributes.
    TiXmlElement *xelement = xnode->ToElement();
    assert(xelement != NULL);
    const TiXmlAttribute *xattrib = xelement->FirstAttribute();

    while (xattrib != NULL) {
      // We have an attribute.
      out.put((char)true);
      
      string name = xattrib->Name();
      size_t name_length = name.length();
      out.write((char *)&name_length, sizeof(name_length));
      out.write(name.data(), name_length);
      
      const string &value = xattrib->ValueStr();
      size_t value_length = value.length();
      out.write((char *)&value_length, sizeof(value_length));
      out.write(value.data(), value_length);
      
      xattrib = xattrib->Next();
    }

    // The end of the attributes list.
    out.put((char)false);
  }

  // Now write all of the children.
  TiXmlNode *xchild = xnode->FirstChild();
  while (xchild != NULL) {
    // We have a child.
    out.put((char)true);
    write_xml_node(out, xchild);
    xchild = xchild->NextSibling();
  }
  
  // The end of the children list.
  out.put((char)false);
}

////////////////////////////////////////////////////////////////////
//     Function: read_xml_node
//  Description: Recursively reads a node and all of its children to
//               the given stream.  Returns the newly-allocated node.
//               The caller is responsible for eventually deleting the
//               return value.  Returns NULL on error.
////////////////////////////////////////////////////////////////////
static TiXmlNode *
read_xml_node(istream &in) {
  NodeType type = (NodeType)in.get();
  if (type == NT_unknown) {
    return NULL;
  }

  size_t value_length;
  in.read((char *)&value_length, sizeof(value_length));
  if (in.gcount() != sizeof(value_length)) {
    return NULL;
  }

  char *buffer = new char[value_length];
  in.read(buffer, value_length);
  string value(buffer, value_length);
  delete[] buffer;

  TiXmlNode *xnode = NULL;
  if (type == NT_element) {
    xnode = new TiXmlElement(value);
  } else if (type == NT_document) {
    xnode = new TiXmlDocument;
  } else if (type == NT_text) {
    xnode = new TiXmlText(value);
  } else {
    assert(false);
  }

  if (type == NT_element) {
    // Read the element attributes.
    TiXmlElement *xelement = xnode->ToElement();
    assert(xelement != NULL);
    bool got_attrib = (bool)in.get();

    while (got_attrib && in && !in.eof()) {
      // We have an attribute.
      size_t name_length;
      in.read((char *)&name_length, sizeof(name_length));
      if (in.gcount() != sizeof(name_length)) {
        delete xnode;
        return NULL;
      }

      buffer = new char[name_length];
      in.read(buffer, name_length);
      string name(buffer, name_length);
      delete[] buffer;

      size_t value_length;
      in.read((char *)&value_length, sizeof(value_length));
      if (in.gcount() != sizeof(value_length)) {
        delete xnode;
        return NULL;
      }

      buffer = new char[value_length];
      in.read(buffer, value_length);
      string value(buffer, value_length);
      delete[] buffer;

      xelement->SetAttribute(name, value);

      got_attrib = (bool)in.get();
    }
  }

  // Now read all of the children.
  bool got_child = (bool)in.get();
  
  while (got_child && in && !in.eof()) {
    // We have a child.
    TiXmlNode *xchild = read_xml_node(in);
    if (xchild != NULL) {
      xnode->LinkEndChild(xchild);
    }

    got_child = (bool)in.get();
  }

  return xnode;
}



////////////////////////////////////////////////////////////////////
//     Function: write_xml
//  Description: Writes the indicated TinyXml document to the given
//               stream.
////////////////////////////////////////////////////////////////////
void
write_xml(ostream &out, TiXmlDocument *doc, ostream &logfile) {
#ifdef DO_BINARY_XML
  // Binary write.
  write_xml_node(out, doc);

#else
  // Formatted ASCII write.
  out << *doc;
#endif

  out << flush;

  if (debug_xml_output) {
    // Write via ostringstream, so it all goes in one operation, to
    // help out the interleaving from multiple threads.
    ostringstream logout;
    logout << "sent: " << *doc << "\n";
    logfile << logout.str() << flush;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: read_xml
//  Description: Reads a TinyXml document from the given stream, and
//               returns it.  If the document is not yet available,
//               blocks until it is, or until there is an error
//               condition on the input.
//
//               The return value is NULL if there is an error, or the
//               newly-allocated document if it is successfully read.
//               If not NULL, the document has been allocated with
//               new, and should be eventually freed by the caller
//               with delete.
////////////////////////////////////////////////////////////////////
TiXmlDocument *
read_xml(istream &in, ostream &logfile) {
#if DO_BINARY_XML
  // binary read.
  TiXmlNode *xnode = read_xml_node(in);
  if (xnode == NULL) {
    return NULL;
  }

  TiXmlDocument *doc = xnode->ToDocument();
  assert(doc != NULL);

#else
  // standard ASCII read.
  TiXmlDocument *doc = new TiXmlDocument;
  in >> *doc;
#endif

  if (debug_xml_output) {
    // Write via ostringstream, so it all goes in one operation, to
    // help out the interleaving from multiple threads.
    ostringstream logout;
    logout << "received: " << *doc << "\n";
    logfile << logout.str() << flush;
  }
    
  return doc;
}
