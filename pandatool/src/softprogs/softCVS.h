// Filename: softCVS.h
// Created by:  drose (10Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOFTCVS_H
#define SOFTCVS_H

#include <pandatoolbase.h>

#include "softFilename.h"

#include <programBase.h>
#include <vector_string.h>

#include <vector>
#include <set>

////////////////////////////////////////////////////////////////////
// 	 Class : SoftCVS
// Description : This program prepares a SoftImage database for CVS by
//               renaming everything to version 1-0, and adding new
//               files to CVS.
////////////////////////////////////////////////////////////////////
class SoftCVS : public ProgramBase {
public:
  SoftCVS();

  void run();

private:
  void traverse(const string &dirname);

  bool rename_file(const string &dirname,
		   vector<SoftFilename>::const_iterator begin, 
		   vector<SoftFilename>::const_iterator end);
  bool scan_cvs(const string &dirname, set<string> &cvs_elements);
  void consider_add_cvs(const string &dirname, const string &filename, 
			const set<string> &cvs_elements);
  void consider_scene_file(Filename path);
  bool scan_scene_file(istream &in, ostream &out);

  bool cvs_add(const string &path);
  bool cvs_add_all();

  bool prompt_yesno(const string &message);
  string prompt(const string &message);

  set<string> _scene_files;
  set<string> _versioned_files;

  vector_string _cvs_paths;
  
protected:
  bool _interactive;
  bool _no_cvs;
  string _cvs_binary;
};

#endif
