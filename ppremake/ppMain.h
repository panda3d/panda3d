// Filename: ppMain.h
// Created by:  drose (28Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PPMAIN_H
#define PPMAIN_H

#include "ppremake.h"
#include "ppDirectoryTree.h"
#include "ppNamedScopes.h"

class PPScope;
class PPCommandFile;

///////////////////////////////////////////////////////////////////
// 	 Class : PPMain
// Description : Handles the toplevel processing in this program:
//               holds the tree of source files, and all the scopes,
//               etc.  Generally get the ball rolling.
////////////////////////////////////////////////////////////////////
class PPMain {
public:
  PPMain(PPScope *global_scope);
  ~PPMain();

  bool read_source(const string &root);

  bool process_all();
  bool process(const string &dirname);

private:
  bool r_process_all(PPDirectoryTree *dir);
  bool p_process(PPDirectoryTree *dir);


  PPScope *_global_scope;
  PPScope *_def_scope;
  PPCommandFile *_defs;

  PPDirectoryTree _tree;
  PPNamedScopes _named_scopes;
  PPScope *_parent_scope;
};

#endif

