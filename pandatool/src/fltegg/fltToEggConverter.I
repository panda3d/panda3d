// Filename: fltToEggConverter.I
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FltToEggConverter::
~FltToEggConverter() {
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::set_input_units
//       Access: Public
//  Description: Sets the units the input flt file is considered to
//               represent.  If this is unset or DU_invalid, the units
//               are taken from the flt header.
////////////////////////////////////////////////////////////////////
INLINE void FltToEggConverter::
set_input_units(DistanceUnit input_units) {
  _input_units = input_units;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::set_output_units
//       Access: Public
//  Description: Sets the units to generate in the resulting egg file.
//               If this is unset, no units conversion is performed.
////////////////////////////////////////////////////////////////////
INLINE void FltToEggConverter::
set_output_units(DistanceUnit output_units) {
  _output_units = output_units;
}
