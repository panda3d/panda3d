// Filename: parse_vrml.h
// Created by:  drose (24Jun99)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

#ifndef PARSE_VRML_H
#define PARSE_VRML_H

#include "vrmlNode.h"
#include "filename.h"

VrmlScene *parse_vrml(Filename filename);
VrmlScene *parse_vrml(istream &in, const string &filename);

#endif
