/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_datagram.cxx
 * @author jns
 * @date 2000-02-07
 */

#include "netDatagram.h"
#include "datagramIterator.h"

using std::cout;
using std::endl;

int
main() {
  NetDatagram dg;

  dg.add_int8((signed char) -10);
  dg.add_uint8((unsigned char) 200);
  dg.add_int16(-32000);
  dg.add_uint16(64000);
  dg.add_int32(100000);
  dg.add_float64(3.14159);
  dg.add_float64(3.141592654378765689009887765567);
  dg.add_string("Monkey");
  dg.add_string("Purple Monkey!");
  dg.add_int32(200000);

  cout << "Length of datagram: " << dg.get_length() << endl;

  DatagramIterator dgi(dg);

  cout << "int8: " << (int)dgi.get_int8() << endl;
  cout << "uint8: " << (int)dgi.get_uint8() << endl;
  cout << "int16: " << dgi.get_int16() << endl;
  cout << "uint16: " << dgi.get_uint16() << endl;
  cout << "int32: " << dgi.get_int32() << endl;
  cout << "float64: " << dgi.get_float64() << endl;
  cout << "float64: " << dgi.get_float64() << endl;
  cout << "string: " << dgi.get_string() << endl;
  cout << "string: " << dgi.get_string() << endl;
  cout << "int: " << dgi.get_int32() << endl;

}
