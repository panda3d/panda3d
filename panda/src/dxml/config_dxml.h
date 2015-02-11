// Filename: config_dxml.h
// Created by: drose (08Aug09)
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

NotifyCategoryDecl(dxml, EXPCL_PANDA, EXPTP_PANDA);

extern EXPCL_PANDA void init_libdxml();

class TiXmlDocument;
class TiXmlNode;
BEGIN_PUBLISH
EXPCL_PANDA TiXmlDocument *read_xml_stream(istream &in);
EXPCL_PANDA void write_xml_stream(ostream &out, TiXmlDocument *doc);
EXPCL_PANDA void print_xml(TiXmlNode *xnode);
EXPCL_PANDA void print_xml_to_file(const Filename &filename, TiXmlNode *xnode);
END_PUBLISH

#endif
