/**************************************************
 * VRML 2.0 Parser
 * Copyright (C) 1996 Silicon Graphics, Inc.
 *
 * Author(s)    : Gavin Bell
 *                Daniel Woods (first port)
 **************************************************
 */

#ifndef VRMLNODETYPE_H
#define VRMLNODETYPE_H

//
// The VrmlNodeType class is responsible for storing information about node
// or prototype types.
//


#include "pandatoolbase.h"

#include "plist.h"
#include "pvector.h"

class VrmlNode;

struct SFNodeRef {
  VrmlNode *_p;
  enum { T_null, T_unnamed, T_def, T_use } _type;
  char *_name;
};

union VrmlFieldValue {
  bool _sfbool;
  double _sffloat;
  long _sfint32;
  char *_sfstring;
  double _sfvec[4];
  SFNodeRef _sfnode;
  pvector<VrmlFieldValue> *_mf;
};

typedef pvector<VrmlFieldValue> MFArray;


std::ostream &output_value(std::ostream &out, const VrmlFieldValue &value, int type,
                      int indent = 0);


class VrmlNodeType {
public:
  // Constructor.  Takes name of new type (e.g. "Transform" or "Box")
  // Copies the string given as name.
  VrmlNodeType(const char *nm);
  
  // Destructor exists mainly to deallocate storage for name
  ~VrmlNodeType();
  
  // Namespace management functions.  PROTO definitions add node types
  // to the namespace.  PROTO implementations are a separate node
  // namespace, and require that any nested PROTOs NOT be available
  // outside the PROTO implementation.
  // addToNameSpace will print an error to stderr if the given type
  // is already defined.
  static void addToNameSpace(VrmlNodeType *);
  static void pushNameSpace();
  static void popNameSpace();
  
  // Find a node type, given its name.  Returns NULL if type is not defined.
  static const VrmlNodeType *find(const char *nm);
  
  // Routines for adding/getting eventIns/Outs/fields
  void addEventIn(const char *name, int type, 
                  const VrmlFieldValue *dflt = nullptr);
  void addEventOut(const char *name, int type,
                   const VrmlFieldValue *dflt = nullptr);
  void addField(const char *name, int type,
                const VrmlFieldValue *dflt = nullptr);
  void addExposedField(const char *name, int type,
                       const VrmlFieldValue *dflt = nullptr);
  
  typedef struct {
    char *name;
    int type;
    VrmlFieldValue dflt;
  } NameTypeRec;
  
  const NameTypeRec *hasEventIn(const char *name) const;
  const NameTypeRec *hasEventOut(const char *name) const;
  const NameTypeRec *hasField(const char *name) const;
  const NameTypeRec *hasExposedField(const char *name) const;
  
  const char *getName() const { return name; }
  
private:
  void add(plist<NameTypeRec*> &,const char *,int, const VrmlFieldValue *dflt);
  const NameTypeRec *has(const plist<NameTypeRec*> &,const char *) const;
  
  char *name;
  
  // Node types are stored in this data structure:
  static plist<VrmlNodeType*> typeList;
  
  plist<NameTypeRec*> eventIns;
  plist<NameTypeRec*> eventOuts;
  plist<NameTypeRec*> fields;
};

#endif
