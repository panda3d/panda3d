// Filename: dSearchPath.I
// Created by:  drose (01Jul00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::search_path
//       Access: Public, Static
//  Description: A quick-and-easy way to search a searchpath for a
//               file when you don't feel like building or keeping
//               around a DSearchPath object.  This simply
//               constructs a temporary DSearchPath based on the
//               indicated path string, and searches that.
////////////////////////////////////////////////////////////////////
INLINE Filename DSearchPath::
search_path(const Filename &filename, const string &path, 
	    const string &delimiters) {
  DSearchPath search(path, delimiters);
  return search.find_file(filename);
}
