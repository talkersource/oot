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
// The Profile Class encapsulates the behaviour of profiles.  Both Users
// and Rooms have profiles.
//
// Eventually this class will be made to allocate memory as required,
// and have a maximum set ... so that you may have a different number
// of lines for a room's profile vs. a user's profile.
//
// A getFirstLine() and getNextLine() would also come in handy ...
//
/////////////////////////////////////////////////////////////////////////

#include "profile.h"

Profile::Profile( void )
{
  clear();
}

Profile::~Profile( void )
{
  clear();
}

boolean Profile::clear( void )
{
  int count = 0;

  for( count = 0; count < PROFILE_LINES; count++ )
    info[ count ][ 0 ] = 0;

  setNumLines( 0 );
  return( TRUE );
}

int Profile::validateLineNum( int lineNum )
{
  if( lineNum < 0 )
    return( 0 );

  if( lineNum < PROFILE_LINES )
    return( lineNum );

  return( PROFILE_LINES - 1 );
}

int Profile::getNumLines( void )
{
  return( numLines );
}

boolean Profile::setNumLines( int lineNum )
{
  numLines = validateLineNum( lineNum );
}

char *Profile::getLine( int lineNum )
{
  return( info[ validateLineNum( lineNum ) ] );
}

boolean Profile::setLine( char *buffer, int lineNum )
{
  int lNum  = validateLineNum( lineNum ),
      speed = getNumLines();

  if( lNum > speed )
  {
    // Fill from the getNumLines() to "lineNum" with blank lines.
    //
    while( speed < lNum )
      info[ speed++ ][ 0 ] = 0;

    setNumLines( lNum + 1 );
  }

  strncpy( info[ lNum ], buffer, PROFILE_COLUMNS );
  return( TRUE );
}

// Returns FALSE if the line to be added would have added past the end
// of the buffer
//
boolean Profile::addLine( char *buffer )
{
  int speed = getNumLines();

  if( (speed + 1) == PROFILE_LINES )
    return( FALSE );

  strncpy( info[ speed ], buffer, PROFILE_COLUMNS );
  info[ speed ][ PROFILE_COLUMNS ] = 0;

  setNumLines( speed + 1 );

  return( TRUE );
}

int Profile::deleteFrom( int deleteIndex )
{
  int count = deleteIndex;

  while( (count < PROFILE_LINES) && (info[ count ][ 0 ] != 0) )
    info[ count++ ][ 0 ] = 0;

printf( "index = %d; count = %d\n", deleteIndex, count );

  setNumLines( deleteIndex );
  return( count - deleteIndex );
}

