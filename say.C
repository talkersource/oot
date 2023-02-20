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
// The Say class encapsulates each type of Say.  Tells, Logons, Topic,
// Room Descriptions, etc.  This allows controlling what the user
// sees, as well has how to format the Say itself when it comes time
// to display it to the user.
//
// Everything a user or command displays (directly or not) to other
// user(s) is a say.
//
// Says have properties:
//
//   Normal, Private Tell, Shout, Emote, Broadcast, Picture, Log On/Off,
//   Unignorable, etc.
//
/////////////////////////////////////////////////////////////////////////

#include "say.h"

    //////                                               //////
   //                                                       //
  // Class Constructors, Destructors, and Initializations  //
 //                                                       //
//////                                               //////

Say::Say( void )
{
  init( "", "", ISNORMAL, -1 );
}

Say::Say( char *said, sayValue type )
{
  init( said, "", type, -1 );
}

Say::Say( char *said, char *user, sayValue type )
{
  init( said, user, type, -1 );
}

Say::Say( char *said, char *user, sayValue type, iColour c )
{
  init( said, user, type, c );
}

boolean Say::init( char *said, char *user, sayValue type, iColour c )
{
  setMessage( said );
  setUserFrom( user );
  setUserTo( "" );
  setSayType( type );
  setColour( c );
  setLineFeed( TRUE );
  setInvisible( FALSE );
  setFromBot( FALSE );

  return( TRUE );
}

    //////               //////
   //                       //
  // Setter/Getter Methods //
 //                       //
//////               //////

boolean Say::setColour( iColour c )
{
  colour = c;
  return( TRUE );
}

iColour Say::getColour( void )
{
  return( colour );
}

boolean Say::setFromBot( boolean toSet )
{
  fromBot = toSet;
  return( TRUE );
}

boolean Say::getFromBot( void )
{
  return( fromBot );
}

boolean Say::setInvisible( boolean toSet )
{
  invisible = toSet;
  return( TRUE );
}

boolean Say::getInvisible( void )
{
  return( invisible );
}

boolean Say::setLineFeed( boolean toSet )
{
  lineFeed = toSet;
  return( TRUE );
}

boolean Say::getLineFeed( void )
{
  return( lineFeed );
}

boolean Say::setMessage( char *msg )
{
  strncpy( message, msg, SAY_LENGTH );
  message[ SAY_LENGTH ] = 0;
  return( TRUE );
}

char *Say::getMessage( void )
{
  return( message );
}

boolean Say::setSayType( sayValue type )
{
  sayType = type;
  return( TRUE );
}

sayValue Say::getSayType( void )
{
  return( sayType );
}

boolean Say::setUserFrom( char *userAlias )
{
  strncpy( userFrom, userAlias, SAY_LENGTH );
  userFrom[ SAY_LENGTH ] = 0;
  return( TRUE );
}

char *Say::getUserFrom( void )
{
  return( userFrom );
}

boolean Say::setUserTo( char *userAliases )
{
  strncpy( userTo, userAliases, SAY_LENGTH );
  userTo[ SAY_LENGTH ] = 0;
  return( TRUE );
}

char *Say::getUserTo( void )
{
  return( userTo );
}
