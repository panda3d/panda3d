// Filename: interrogateComponent.C
// Created by:  drose (08Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "interrogateComponent.h"
#include "interrogate_datafile.h"


////////////////////////////////////////////////////////////////////
//     Function: InterrogateComponent::output
//       Access: Public
//  Description: Formats the component for output to a data file.
////////////////////////////////////////////////////////////////////
void InterrogateComponent::
output(ostream &out) const {
  idf_output_string(out, _name);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateComponent::input
//       Access: Public
//  Description: Reads the data file as previously formatted by
//               output().
////////////////////////////////////////////////////////////////////
void InterrogateComponent::
input(istream &in) {
  idf_input_string(in, _name);
}
