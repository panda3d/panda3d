/*
  MaxToEgg.cpp
  Created by Ken Strickland 02/24/03
  Modified and maintained by Corey Revilla, (05/22/03)-(Present)
  Carnegie Mellon University, Entetainment Technology Center
*/

//Our headers, which in turn includes some Max headers. 
#include "MaxToEgg.h"

//Member Function Definitions
/* ~MaxToEgg() - Uninteresting destructor.
 */
MaxToEgg::~MaxToEgg() 
{
}

/* MaxToEgg() - Constructs a MaxToEgg "application."  It sets the types of
   options this converter can take and sets up a description of the program.
 */
MaxToEgg::MaxToEgg() : SomethingToEgg("3D Studio Max",".max") 
{
  add_path_replace_options();
  add_path_store_options();
  add_animation_options();
  add_units_options();
  add_normals_options();
  add_transform_options();

  set_program_description("This program converts 3D Studio Max model files to egg.");
  add_option("p", "", 0,
	     "Generate polygon output only.  Convert scene to triangle mesh "
	     "before converting.", &MaxToEgg::dispatch_none, &alertOnBegin);
  //Fill in the member variables.
  pMaxInterface = null;
  successfulOutput = false;
  alertOnBegin = false;
}

/* IsSuccessful() - Indicates if conversion was successful.
 */
bool MaxToEgg::IsSuccessful() 
{
  return successfulOutput;
}

char *MaxToEgg::MyClassName()
{
  return "MaxToEgg";
}

/* Run() - Runs the conversion.  Creates a MaxToEggConverter, populates it
   with the scene graph, and then writes out the egg file.
*/
void MaxToEgg::Run() 
{
  MaxToEggConverter converter;

  Logger::FunctionEntry( "MaxToEgg::Run" );
  // Now, we fill out the necessary fields of the converter, which does all
  // the necessary work.
  Logger::Log( MTE, Logger::SAT_DEBUG_SPAM_LEVEL, "Setting Max Interface." );
  converter.setMaxInterface( pMaxInterface );
  Logger::Log( MTE, Logger::SAT_DEBUG_SPAM_LEVEL,
	       "Setting converter's egg data." );
  converter.set_egg_data( &_data, false );
  // applies the parameters from the command line options
  apply_parameters(converter);
  
  //Now, do the actual file conversion.
  if (converter.convert_file(_input_filename)) {
    successfulOutput=true;
    write_egg_file();
    Logger::Log( MTE, Logger::SAT_DEBUG_SPAM_LEVEL, "Egg file written!" );
  }

  Logger::FunctionExit();
}


/* SetMaxInterface(Interface *) - This is how we know how to traverse the Max
   scene graph.  For this to be	a standalone application, we'd want to be able
   to build one of these interfaces for the input file.
*/
void MaxToEgg::SetMaxInterface(Interface *pInterface) 
{
  pMaxInterface = pInterface;
}
