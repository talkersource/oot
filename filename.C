/////////////////////////////////////////////////////////////////////////
//
/*
   Copyright (C) 1996 by Dave Jarvis

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
//
// FileName class removes inauspicious characters, and allows the
// building of a file name from a relative path, file name, and file
// extension.
//
/////////////////////////////////////////////////////////////////////////

#include "filename.h"

FileName::FileName( void )
{
  setFileName( "" );
}

/////////////////////////////////////////////////////////////////////////
//
// Private member methods
//
/////////////////////////////////////////////////////////////////////////

boolean FileName::setPath( char *buffer )
{
  strcpy( fileName, buffer );
  return( TRUE );
}

boolean FileName::setName( char *buffer )
{
  strcat( fileName, buffer );
  return( TRUE );
}

boolean FileName::setExt( char *buffer )
{
  strcat( fileName, buffer );
  return( TRUE );
}

// Given a user name and path, make it a valid file name by
// eliminating everything except alpha-numeric characters and
// the / and . characters.
//
boolean FileName::setFileName( char *fName )
{
  int count = 0,
      loop  = 0;

  while( (count < FILE_NAME_LENGTH) && fName[ loop ] )
  {
    if( isalnum( fName[ loop ] ) ||
        (fName[ loop ] == '/') || (fName[ loop ] == '.') )
      fileName[ count++ ] = tolower( fName[ loop ] );

    loop++;
  }

  fileName[ count ] = 0;

  return( TRUE );
}

boolean FileName::validateName( void )
{
  char tempBuff[ FILE_NAME_LENGTH + 1 ];

  strcpy( tempBuff, fileName );
  setFileName( tempBuff );
  return( TRUE );
}

/////////////////////////////////////////////////////////////////////////
//
// Public member methods
//
/////////////////////////////////////////////////////////////////////////

boolean FileName::setFileName( char *fPath, char *fName, char *fExt )
{
  setPath( fPath );
  setName( fName );
  setExt( fExt );
  return( validateName() );
}

char *FileName::getFileName( void )
{
  return( fileName );
}
