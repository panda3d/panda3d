// Filename: test_bamWrite.cxx
// Created by:  jason (09Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "notify.h"

#include "test_bam.h"

int main(int argc, char* argv[])
{
   string test_file("bamTest.out");
   datagram_file stream(test_file);

   BamWriter manager(&stream);
   stream.open(file::FILE_WRITE);

   // This initialization would normally be done by a ConfigureFn
   // block.

   PointerTo<Parent> dad = new Parent("Attila", Person::MALE);
   PointerTo<Parent> mom = new Parent("Brunhilda", Person::FEMALE);
   PointerTo<Child> bro = new Child("Bob", Person::MALE);
   PointerTo<Child> sis = new Child("Mary Poppins", Person::FEMALE);

   //Set up relationships
   dad->setSon(bro.p());
   dad->setDaughter(sis.p());

   mom->setSon(bro.p());
   mom->setDaughter(sis.p());

   bro->setMother(mom.p());
   bro->setFather(dad.p());
   bro->setSister(sis.p());

   sis->setFather(dad.p());
   sis->setMother(mom.p());
   sis->setBrother(bro.p());

   manager.init();

   manager.write_object(dad.p());
   manager.write_object(mom.p());
   manager.write_object(bro.p());
   manager.write_object(sis.p());

   stream.close();
   return 1;
}

