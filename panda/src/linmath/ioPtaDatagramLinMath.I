// Filename: ioPtaDatagramLinMath.I
// Created by:  jason (26Jun00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagamLinMath::write_datagram
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class LinMathElement>
void IoPtaDatagramLinMath<LinMathElement>::
write_datagram(Datagram &dest, CPTA(LinMathElement) array)
{
  dest.add_uint32(array.size());
  for(int i = 0; i < (int)array.size(); i++)
  {
    array[i].write_datagram(dest);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagamLinMath::read_datagram
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class LinMathElement>
PTA(LinMathElement) IoPtaDatagramLinMath<LinMathElement>::
read_datagram(DatagramIterator &source)
{
  PTA(LinMathElement) array;
  LinMathElement temp;

  int size = source.get_uint32();
  for(int i = 0; i < size; i++)
  {
    temp.read_datagram(source);
    array.push_back(temp);
  }

  return array;
}


