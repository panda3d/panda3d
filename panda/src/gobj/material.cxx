// Filename: material.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include "material.h"

#include <indent.h>

#include <stddef.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle Material::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
Material::Material( void )
{
    set_ambient( Colorf( 0.2, 0.2, 0.2, 1 ) );
    set_diffuse( Colorf( 0.8, 0.8, 0.8, 1 ) );
    set_specular( Colorf( 0, 0, 0, 1 ) );
    set_shininess( 0.0 );
    set_emission( Colorf( 0, 0, 0, 1 ) );

    set_local( false );
    set_twoside( false );
}

////////////////////////////////////////////////////////////////////
//     Function: Destructor 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
Material::~Material( void )
{
}


////////////////////////////////////////////////////////////////////
//     Function: output 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void Material::output( ostream &out ) const
{
  out << "a" << _ambient[0] << ",d" << _diffuse[0] 
      << ",s" << _specular[0] << ",s" << _shininess
      << ",e" << _emission[0] << ",l" << _local
      << ",t" << _twoside;
}

////////////////////////////////////////////////////////////////////
//     Function: write 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void Material::write( ostream &out, int indent_level ) const
{
  indent(out, indent_level) << "ambient = " << _ambient << "\n";
  indent(out, indent_level) << "diffuse = " << _diffuse << "\n";
  indent(out, indent_level) << "specular = " << _specular << "\n";
  indent(out, indent_level) << "shininess = " << _shininess << "\n";
  indent(out, indent_level) << "emission = " << _emission << "\n";
  indent(out, indent_level) << "local = " << _local << "\n";
  indent(out, indent_level) << "twoside = " << _twoside << "\n";
}
