// Filename: eggComponentData.I
// Created by:  drose (26Feb01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggComponentData::get_num_models
//       Access: Public
//  Description: Returns the maximum number of back pointers this
//               component may have.  The component may store a back
//               pointer for models indexed 0 .. num_models -
//               1.  You must call has_model() on each model
//               index to confirm whether a particular model in that
//               range has a back pointer.
////////////////////////////////////////////////////////////////////
INLINE int EggComponentData::
get_num_models() const {
  return _back_pointers.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggComponentData::has_model
//       Access: Public
//  Description: Returns true if the component has a back pointer to
//               an egg file somewhere for the indicated model, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggComponentData::
has_model(int model_index) const {
  if (model_index >= 0 && model_index < (int)_back_pointers.size()) {
    return _back_pointers[model_index] != (EggBackPointer *)NULL;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggComponentData::get_model
//       Access: Public
//  Description: Returns the back pointer to an egg file for the
//               indicated model if it exists, or NULL if it does not.
////////////////////////////////////////////////////////////////////
INLINE EggBackPointer *EggComponentData::
get_model(int model_index) const {
  if (model_index >= 0 && model_index < (int)_back_pointers.size()) {
    return _back_pointers[model_index];
  }
  return (EggBackPointer *)NULL;
}
