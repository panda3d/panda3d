/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_dxml.cxx
 * @author drose
 * @date 2009-08-08
 */

#include "config_dxml.h"
#include "dconfig.h"
#include <stdio.h>

BEGIN_PUBLISH
#include "tinyxml.h"
END_PUBLISH

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_DXML)
  #error Buildsystem error: BUILDING_PANDA_DXML not defined
#endif

Configure(config_dxml);
NotifyCategoryDef(dxml, "");

ConfigureFn(config_dxml) {
  init_libdxml();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libdxml() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}

BEGIN_PUBLISH
/**
 * Reads an XML document from the indicated stream.
 * @returns the document, or NULL on error.
 */
TiXmlDocument *
read_xml_stream(std::istream &in) {
  TiXmlDocument *doc = new TiXmlDocument;
  in >> *doc;
  if (in.fail() && !in.eof()) {
    delete doc;
    return nullptr;
  }

  return doc;
}
END_PUBLISH

BEGIN_PUBLISH
/**
 * Writes an XML document to the indicated stream.
 */
void
write_xml_stream(std::ostream &out, TiXmlDocument *doc) {
  out << *doc;
}
END_PUBLISH

BEGIN_PUBLISH
/**
 * Writes an XML object to stdout, with formatting.
 */
void
print_xml(TiXmlNode *xnode) {
  xnode->Print(stdout, 0);
}
END_PUBLISH

BEGIN_PUBLISH
/**
 * Writes an XML object to the indicated file, with formatting.  Unfortunately
 * the VFS cannot be supported; the file must be a real filename on disk.
 */
void
print_xml_to_file(const Filename &filename, TiXmlNode *xnode) {
  std::string os_name = filename.to_os_specific();
#ifdef _WIN32
  FILE *file;
  if (fopen_s(&file, os_name.c_str(), "w") != 0) {
#else
  FILE *file = fopen(os_name.c_str(), "w");
  if (file == nullptr) {
#endif
    dxml_cat.error() << "Failed to open " << filename << " for writing\n";
  }
  xnode->Print(file, 0);
  fclose(file);
}
END_PUBLISH
