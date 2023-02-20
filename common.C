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
// The Common Class splits out the common behaviour that can be found
// in the Rooms, Users, and CommandParser class.  The splitStriing()
// method is used by the Rooms & CommandParser class.  The rest is used
// by the User and Room classes.
//
/////////////////////////////////////////////////////////////////////////

#include "common.h"

Common::Common( void )
{
  setName( "" );
}

boolean Common::setName( char *newName )
{
  strncpy( name, newName, COMMON_NAME_LEN );
  name[ COMMON_NAME_LEN ] = 0;
  return( TRUE );
}

char *Common::getName( void )
{
  return( name );
}

// Split out the lvalue and rvalue from something like:
//
// Room Name = Logon Limbo
// ---------   -----------
//  lvalue        rvalue
//
char *Common::splitString( char *strToSplit, char splitAt )
{
  char *buffPtr;

  // Point past the lvalue
  //
  if( (buffPtr = strchr( strToSplit, (int)splitAt )) == NULL )
    return( NULL );

  // Point to the start of the rvalue
  //
  while( *buffPtr != 0 )
  {
    buffPtr++;

    if( !isspace( *buffPtr ) )
      break;
  }

  return( buffPtr );
}

