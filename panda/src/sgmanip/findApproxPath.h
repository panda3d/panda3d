// Filename: findApproxPath.h
// Created by:  drose (18Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef FINDAPPROXPATH_H
#define FINDAPPROXPATH_H

#include <pandabase.h>

#include <typeHandle.h>

#include <string>
#include <vector>

class Node;

////////////////////////////////////////////////////////////////////
//       Class : FindApproxPath
// Description : This class is local to this package only; it doesn't
//               get exported.  It chops a string path, as supplied to
//               find_up() or find_down(), and breaks it up into its
//               component pieces.
////////////////////////////////////////////////////////////////////
class FindApproxPath {
public:
  INLINE FindApproxPath();

  bool add_string(const string &str_path);
  bool add_component(const string &str_component);

  INLINE void add_match_name(const string &name);
  INLINE void add_match_name_glob(const string &glob);
  INLINE void add_match_exact_type(TypeHandle type);
  INLINE void add_match_inexact_type(TypeHandle type);
  INLINE void add_match_one();
  INLINE void add_match_many();
  INLINE void add_match_pointer(Node *pointer);

  INLINE int get_num_components() const;
  INLINE bool is_component_match_many(int index) const;
  INLINE bool matches_component(int index, Node *node) const;

  void output(ostream &out) const;
  INLINE void output_component(ostream &out, int index) const;

#ifndef WIN32_VC
// Visual C++ won't let us define the ostream operator functions for
// these guys if they're private--even though we declare them friends.
private:
#endif
  enum ComponentType {
    CT_match_name,
    CT_match_name_glob,
    CT_match_exact_type,
    CT_match_inexact_type,
    CT_match_one,
    CT_match_many,
    CT_match_pointer
  };

  class Component {
  public:
    bool matches(Node *node) const;
    void output(ostream &out) const;

    ComponentType _type;
    string _name;
    TypeHandle _type_handle;
    Node *_pointer;
  };

  typedef vector<Component> Path;
  Path _path;

friend ostream &operator << (ostream &, FindApproxPath::ComponentType);
friend INLINE ostream &operator << (ostream &, const FindApproxPath::Component &);
};

ostream &
operator << (ostream &out, FindApproxPath::ComponentType type);

INLINE ostream &
operator << (ostream &out, const FindApproxPath::Component &component) {
  component.output(out);
  return out;
}

INLINE ostream &
operator << (ostream &out, const FindApproxPath &path) {
  path.output(out);
  return out;
}

#include "findApproxPath.I"

#endif
