// Filename: bamWriter.I
// Created by:  jason (08Jun00)
//

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BamWriter::
BamWriter(DatagramSink *sink) : 
     //_current needs to start from 1, as a value of 0
     //for a unique object ID will be used to indicate a null pointer
     _target(sink), _writing(false), _current(1),
     _current_pta(1), _emptying(false)
{
}
