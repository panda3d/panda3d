/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_dxml.h
 * @author drose
 * @date 2009-08-08
 */

#ifndef CONFIG_DXML_H
#define CONFIG_DXML_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

// The purpose of this directory is to expose tinyxml interfaces to
// Python via interrogate.

// tinyxml.h requires having the symbol TIXML_USE_STL already defined
// before you include it.

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

NotifyCategoryDecl(dxml, EXPCL_PANDA_DXML, EXPTP_PANDA_DXML);

extern EXPCL_PANDA_DXML void init_libdxml();

class TiXmlDocument;
class TiXmlNode;
BEGIN_PUBLISH
EXPCL_PANDA_DXML TiXmlDocument *read_xml_stream(std::istream &in);
EXPCL_PANDA_DXML void write_xml_stream(std::ostream &out, TiXmlDocument *doc);
EXPCL_PANDA_DXML void print_xml(TiXmlNode *xnode);
EXPCL_PANDA_DXML void print_xml_to_file(const Filename &filename, TiXmlNode *xnode);
END_PUBLISH

#endif
