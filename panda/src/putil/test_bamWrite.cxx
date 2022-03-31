/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_bamWrite.cxx
 * @author jason
 * @date 2000-06-09
 */

#include "pandabase.h"
#include "pnotify.h"

#include "test_bam.h"
#include "datagramOutputFile.h"

int main(int argc, char* argv[])
{
   std::string test_file("bamTest.out");
   DatagramOutputFile stream;
   bool success = stream.open(test_file);
   nassertr(success, 1);

   BamWriter manager(&stream);

   PointerTo<Parent> dad = new Parent("Attila", Person::MALE);
   PointerTo<Parent> mom = new Parent("Brunhilda", Person::FEMALE);
   PointerTo<Child> bro = new Child("Bob", Person::MALE);
   PointerTo<Child> sis = new Child("Mary Poppins", Person::FEMALE);

   // Set up relationships
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
   return 0;
}
