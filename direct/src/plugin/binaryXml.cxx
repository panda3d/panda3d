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
#include "p3d_lock.h"
#include <sstream>

static const bool debug_xml_output = false;

static LOCK xml_lock;
static bool xml_lock_initialized = false;

#define DO_BINARY_XML 1

enum NodeType {
  NT_unknown,
  NT_document,
  NT_element,
  NT_text,
};

// This typedef defines a 32-bit unsigned integer.  It's used for
// passing values through the binary XML stream.
typedef unsigned int xml_uint32;

// These are both prime numbers, though I don't know if that really
// matters.  Mainly, they're big random numbers.
static const xml_uint32 length_nonce1 = 812311453;
static const xml_uint32 length_nonce2 = 612811373;

////////////////////////////////////////////////////////////////////
//     Function: init_xml
//  Description: Should be called before spawning any threads to
//               ensure the lock is initialized.
////////////////////////////////////////////////////////////////////
void
init_xml() {
  if (!xml_lock_initialized) {
    INIT_LOCK(xml_lock);
    xml_lock_initialized = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: write_xml_node
//  Description: Recursively writes a node and all of its children to
//               the given stream.
////////////////////////////////////////////////////////////////////
static void
write_xml_node(ostream &out, TiXmlNode *xnode) {
  const string &value = xnode->ValueStr();
  xml_uint32 value_length = value.length();
  xml_uint32 value_proof = (value_length + length_nonce1) * length_nonce2;

  // We write out not only value_length, but the same value again
  // hashed by length_nonce1 and 2 (and truncated back to xml_uint32),
  // just to prove to the reader that we're still on the same page.
  // We do this only on the top node; we don't bother for the nested
  // nodes.
  out.write((char *)&value_length, sizeof(value_length));
  out.write((char *)&value_proof, sizeof(value_proof));
  out.write(value.data(), value_length);

  // Now write out the node type.
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
  // We don't bother to write any further data for the unknown types.
  if (type == NT_unknown) {
    return;
  }

  if (type == NT_element) {
    // Write the element attributes.
    TiXmlElement *xelement = xnode->ToElement();
    assert(xelement != NULL);
    const TiXmlAttribute *xattrib = xelement->FirstAttribute();

    while (xattrib != NULL) {
      // We have an attribute.
      out.put((char)true);
      
      string name = xattrib->Name();
      xml_uint32 name_length = name.length();
      out.write((char *)&name_length, sizeof(name_length));
      out.write(name.data(), name_length);
      
      const string &value = xattrib->ValueStr();
      xml_uint32 value_length = value.length();
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
read_xml_node(istream &in, char *&buffer, xml_uint32 &buffer_length,
              ostream &logfile) {
  xml_uint32 value_length;
  in.read((char *)&value_length, sizeof(value_length));
  if (in.gcount() != sizeof(value_length)) {
    return NULL;
  }
  xml_uint32 value_proof_expect = (value_length + length_nonce1) * length_nonce2;
  xml_uint32 value_proof;
  in.read((char *)&value_proof, sizeof(value_proof));
  if (in.gcount() != sizeof(value_proof)) {
    return NULL;
  }
  if (value_proof != value_proof_expect) {
    // Hey, we ran into garbage: the proof value didn't match our
    // expected proof value.
    logfile << "Garbage on XML stream!\n";

    // Print out the garbage; maybe it will help the developer figure
    // out where it came from.
    logfile << "Begin garbage:\n";
    ostringstream strm;
    strm.write((char *)&value_length, sizeof(value_length));
    strm.write((char *)&value_proof, sizeof(value_proof));
    logfile << strm.str();
    for (size_t i = 0; i < 100; ++i) {
      int ch = in.get();
      if (ch != EOF) {
        logfile.put(ch);
      }
    }
    logfile << "\n";
    logfile << "End garbage.\n";
    return NULL;
  }

  if (value_length > buffer_length) {
    delete[] buffer;
    buffer_length = value_length;
    buffer = new char[buffer_length];
  }

  in.read(buffer, value_length);
  string value(buffer, value_length);

  // Read the node type.
  NodeType type = (NodeType)in.get();
  if (type == NT_unknown) {
    return NULL;
  }

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
    bool got_attrib = (bool)(in.get() != 0);

    while (got_attrib && in && !in.eof()) {
      // We have an attribute.
      xml_uint32 name_length;
      in.read((char *)&name_length, sizeof(name_length));
      if (in.gcount() != sizeof(name_length)) {
        delete xnode;
        return NULL;
      }

      if (name_length > buffer_length) {
        delete[] buffer;
        buffer_length = name_length;
        buffer = new char[buffer_length];
      }

      in.read(buffer, name_length);
      string name(buffer, name_length);

      xml_uint32 value_length;
      in.read((char *)&value_length, sizeof(value_length));
      if (in.gcount() != sizeof(value_length)) {
        delete xnode;
        return NULL;
      }

      if (value_length > buffer_length) {
        delete[] buffer;
        buffer_length = value_length;
        buffer = new char[buffer_length];
      }

      in.read(buffer, value_length);
      string value(buffer, value_length);

      xelement->SetAttribute(name, value);

      got_attrib = (bool)(in.get() != 0);
    }
  }

  // Now read all of the children.
  bool got_child = (bool)(in.get() != 0);
  
  while (got_child && in && !in.eof()) {
    // We have a child.
    TiXmlNode *xchild = read_xml_node(in, buffer, buffer_length, logfile);
    if (xchild != NULL) {
      xnode->LinkEndChild(xchild);
    }

    got_child = (bool)(in.get() != 0);
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
  assert(xml_lock_initialized);
  ACQUIRE_LOCK(xml_lock);

#ifdef DO_BINARY_XML
  // Binary write.
  write_xml_node(out, doc);

#else
  // Formatted ASCII write.

  // We need a declaration to write it safely.
  TiXmlDeclaration decl("1.0", "utf-8", "");
  doc->InsertBeforeChild(doc->FirstChild(), decl);

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

  RELEASE_LOCK(xml_lock);
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
  // We don't acquire xml_lock while reading.  We can't, because our
  // XML readers are all designed to block until data is available,
  // and they can't block while holding the lock.

  // Fortunately, there should be only one reader at a time, so a lock
  // isn't really needed here.

#if DO_BINARY_XML
  // binary read.
  xml_uint32 buffer_length = 128;
  char *buffer = new char[buffer_length];
  TiXmlNode *xnode = read_xml_node(in, buffer, buffer_length, logfile);
  delete[] buffer;
  if (xnode == NULL) {
    return NULL;
  }

  TiXmlDocument *doc = xnode->ToDocument();
  assert(doc != NULL);

#else
  // standard ASCII read.
  TiXmlDocument *doc = new TiXmlDocument;
  in >> *doc;
  if (in.fail() || in.eof()) {
    delete doc;
    return NULL;
  }
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
