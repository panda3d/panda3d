// Filename: describe_data_verbose.cxx
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "describe_data_verbose.h"
#include "dataGraphTraversal.h"

#include <indent.h>
#include <nodeAttributes.h>

// The number of columns in from the start of the name to print the
// data value.
static const int data_indent_level = 12;

  
////////////////////////////////////////////////////////////////////
//     Function: describe_data_verbose
//  Description: Writes to the indicated output stream a
//               nicely-formatted, multi-line description of all the
//               data values included in the indicated state.
////////////////////////////////////////////////////////////////////
void
describe_data_verbose(ostream &out, const NodeAttributes &state,
		      int indent_level) {
  NodeAttributes::const_iterator nai;

  for (nai = state.begin(); nai != state.end(); ++nai) {
    // We'll exclude the spam flag.  No one wants to see that.
    TypeHandle type = (*nai).first;
    if (type != DataGraphTraversal::_spam_flag_type) {
      const PT(NodeAttribute) &attrib = (*nai).second;
      if (attrib != (NodeAttribute *)NULL) {
	// Now extract the type name out of the type flag.
	string actual_name = type.get_name();
	string::size_type underscore = actual_name.rfind('_');
	string name;
	if (underscore == string::npos) {
	  // There was no underscore, so this name wasn't created via
	  // register_data_transition().  Huh.  Well, that's legitimate
	  // (if unexpected), so just print the whole name.
	  name = actual_name;
	} else {
	  // Assume that, if the name was created via
	  // register_data_transition(), the original name was the part
	  // before the last underscore.
	  name = actual_name.substr(0, underscore);
	}
	
	indent(out, indent_level) << name;
	if (name.length() < data_indent_level) {
	  indent(out, data_indent_level - name.length());
	} else {
	  out << "  ";
	}
	out << *attrib << "\n";
      }
    }
  }
}

