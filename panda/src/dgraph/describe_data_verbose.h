// Filename: describe_data_verbose.h
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DESCRIBE_DATA_VERBOSE_H
#define DESCRIBE_DATA_VERBOSE_H

#include <pandabase.h>

class NodeAttributes;

////////////////////////////////////////////////////////////////////
//     Function: describe_data_verbose
//  Description: Writes to the indicated output stream a
//               nicely-formatted, multi-line description of all the
//               data values included in the indicated state.
////////////////////////////////////////////////////////////////////
void describe_data_verbose(ostream &out, const NodeAttributes &state,
                           int indent_level = 0);

#endif
