/**************************************************
 * VRML 2.0 Parser
 * Copyright (C) 1996 Silicon Graphics, Inc.
 *
 * Author(s)    : Gavin Bell
 *                Daniel Woods (first port)
 **************************************************
 */

//
// The VrmlNodeType class is responsible for storing information about node
// or prototype types.
//

#include "vrmlNodeType.h"
#include "vrmlNode.h"
#include "vrmlParser.h"
#include "pnotify.h"
#include "indent.h"

#include <stdio.h>  // for sprintf()

using std::ostream;


//
// Static list of node types.
//
plist<VrmlNodeType*> VrmlNodeType::typeList;

static ostream &
output_array(ostream &out, const MFArray *mf,
             int type, int indent_level, int items_per_row) {
  if (mf->empty()) {
    out << "[ ]";
  } else {
    out << "[";
    MFArray::const_iterator mi;
    int col = 0;
    for (mi = mf->begin(); mi != mf->end(); ++mi) {
      if (col == 0) {
        out << "\n";
        indent(out, indent_level + 2);
      }      
      output_value(out, (*mi), type, indent_level + 2);
      if (++col >= items_per_row) {
        col = 0;
      } else {
        out << " ";
      }
    }
    out << "\n";
    indent(out, indent_level) << "]";
  }
  return out;
}

ostream & 
output_value(ostream &out, const VrmlFieldValue &value, int type,
             int indent) {
  switch (type) {
  case SFBOOL:
    return out << (value._sfbool ? "TRUE" : "FALSE");

  case SFFLOAT:
  case SFTIME:
    return out << value._sffloat;

  case SFINT32:
    return out << value._sfint32;

  case SFSTRING:
    {
      out << '"';
      for (const char *p = value._sfstring; *p != '\0'; p++) {
        if (*p == '"') {
          out << "\\\"";
        } else {
          out << *p;
        }
      }
      return out << '"';
    }

  case SFVEC2F:
    return out << value._sfvec[0] << " " << value._sfvec[1];

  case SFCOLOR:
  case SFVEC3F:
    return out << value._sfvec[0] << " " << value._sfvec[1] << " "
               << value._sfvec[2];

  case SFROTATION:
    return out << value._sfvec[0] << " " << value._sfvec[1] << " "
               << value._sfvec[2] << " " << value._sfvec[3];

  case SFNODE:
    switch (value._sfnode._type) {
    case SFNodeRef::T_null:
      return out << "NULL";

    case SFNodeRef::T_unnamed:
      nassertr(value._sfnode._p != nullptr, out);
      value._sfnode._p->output(out, indent);
      return out;

    case SFNodeRef::T_def:
      out << "DEF " << value._sfnode._name << " ";
      value._sfnode._p->output(out, indent);
      return out;

    case SFNodeRef::T_use:
      return out << "USE " << value._sfnode._name;
    }
    return out << "(invalid)";

  case SFIMAGE:
    return out << "(image)";

  case MFCOLOR:
    return output_array(out, value._mf, SFCOLOR, indent, 1);

  case MFFLOAT:
    return output_array(out, value._mf, SFFLOAT, indent, 5);

  case MFINT32:
    return output_array(out, value._mf, SFINT32, indent, 10);

  case MFROTATION:
    return output_array(out, value._mf, SFROTATION, indent, 1);

  case MFSTRING:
    return output_array(out, value._mf, SFSTRING, indent, 1);

  case MFVEC2F:
    return output_array(out, value._mf, SFVEC2F, indent, 1);

  case MFVEC3F:
    return output_array(out, value._mf, SFVEC3F, indent, 1);

  case MFNODE:
    return output_array(out, value._mf, SFNODE, indent, 1);
  }

  return out << "(unknown)";
}

VrmlNodeType::VrmlNodeType(const char *nm)
{
    nassertv(nm != nullptr);
    name = strdup(nm);
}

VrmlNodeType::~VrmlNodeType()
{
    free(name);

    // Free strings duplicated when fields/eventIns/eventOuts added:
    plist<NameTypeRec*>::iterator i;

    for (i = eventIns.begin(); i != eventIns.end(); i++) {
        NameTypeRec *r = *i;
        free(r->name);
        delete r;
    }
    for (i = eventOuts.begin(); i != eventOuts.end(); i++) {
        NameTypeRec *r = *i;
        free(r->name);
        delete r;
    }
    for (i = fields.begin(); i != fields.end(); i++) {
        NameTypeRec *r = *i;
        free(r->name);
        delete r;
    }
}

void
VrmlNodeType::addToNameSpace(VrmlNodeType *_type)
{
    if (find(_type->getName()) != nullptr) {
      std::cerr << "PROTO " << _type->getName() << " already defined\n";
      return;
    }
    typeList.push_front(_type);
}

//
// One list is used to store all the node types.  Nested namespaces are
// separated by NULL elements.
// This isn't terribly efficient, but it is nice and simple.
//
void
VrmlNodeType::pushNameSpace()
{
    typeList.push_front(nullptr);
}

void
VrmlNodeType::popNameSpace()
{
    // Remove everything up to and including the next NULL marker:
    plist<VrmlNodeType*>::iterator i;
    for (i = typeList.begin(); i != typeList.end();) {
        VrmlNodeType *nodeType = *i;
        ++i;
        typeList.pop_front();

        if (nodeType == nullptr) {
            break;
        }
        else {
            // NOTE:  Instead of just deleting the VrmlNodeTypes, you will
            // probably want to reference count or garbage collect them, since
            // any nodes created as part of the PROTO implementation will
            // probably point back to their VrmlNodeType structure.
            delete nodeType;
        }
    }
}

const VrmlNodeType *
VrmlNodeType::find(const char *_name)
{
    // Look through the type stack:
    plist<VrmlNodeType*>::iterator i;
    for (i = typeList.begin(); i != typeList.end(); i++) {
        const VrmlNodeType *nt = *i;
        if (nt != nullptr && strcmp(nt->getName(),_name) == 0) {
            return nt;
        }
    }
    return nullptr;
}

void
VrmlNodeType::addEventIn(const char *name, int type,
                         const VrmlFieldValue *dflt)
{
    add(eventIns, name, type, dflt);
};
void
VrmlNodeType::addEventOut(const char *name, int type,
                          const VrmlFieldValue *dflt)
{
    add(eventOuts, name, type, dflt);
};
void
VrmlNodeType::addField(const char *name, int type,
                       const VrmlFieldValue *dflt)
{
    add(fields, name, type, dflt);
};
void
VrmlNodeType::addExposedField(const char *name, int type,
                              const VrmlFieldValue *dflt)
{
    char tmp[1000];
    add(fields, name, type, dflt);
    sprintf(tmp, "set_%s", name);
    add(eventIns, tmp, type, dflt);
    sprintf(tmp, "%s_changed", name);
    add(eventOuts, tmp, type, dflt);
};

void
VrmlNodeType::add(plist<NameTypeRec*> &recs, const char *name, int type,
                  const VrmlFieldValue *dflt)
{
    NameTypeRec *r = new NameTypeRec;
    r->name = strdup(name);
    r->type = type;
    if (dflt != nullptr) {
      r->dflt = *dflt;
    } else {
      memset(&r->dflt, 0, sizeof(r->dflt));
    }
    recs.push_front(r);
}

const VrmlNodeType::NameTypeRec *
VrmlNodeType::hasEventIn(const char *name) const
{
    return has(eventIns, name);
}

const VrmlNodeType::NameTypeRec *
VrmlNodeType::hasEventOut(const char *name) const
{
    return has(eventOuts, name);
}

const VrmlNodeType::NameTypeRec *
VrmlNodeType::hasField(const char *name) const
{
    return has(fields, name);
}

const VrmlNodeType::NameTypeRec *
VrmlNodeType::hasExposedField(const char *name) const
{
    // Must have field "name", eventIn "set_name", and eventOut
    // "name_changed", all with same type:
    char tmp[1000];
    const NameTypeRec *base, *set_name, *name_changed;

    base = has(fields, name);

    sprintf(tmp, "set_%s\n", name);
    nassertr(strlen(tmp) < 1000, nullptr);
    set_name = has(eventIns, tmp);

    sprintf(tmp, "%s_changed\n", name);
    nassertr(strlen(tmp) < 1000, nullptr);
    name_changed = has(eventOuts, tmp);

    if (base == nullptr || set_name == nullptr || name_changed == nullptr) {
      return nullptr;
    }

    if (base->type != set_name->type || base->type != name_changed->type) {
      return nullptr;
    }

    return base;
}

const VrmlNodeType::NameTypeRec *
VrmlNodeType::has(const plist<NameTypeRec*> &recs, const char *name) const
{
    plist<NameTypeRec*>::const_iterator i;
    for (i = recs.begin(); i != recs.end(); i++) {
        if (strcmp((*i)->name, name) == 0)
            return (*i);
    }
    return nullptr;
}

