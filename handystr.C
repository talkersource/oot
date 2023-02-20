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
// HandyStr class is the jewel that holds "all" the strings that the
// talker uses.  
//
/////////////////////////////////////////////////////////////////////////

#include "handystr.h"

    //////                                               //////
   //                                                       //
  // Class Constructors, Destructors, and Initializations  //
 //                                                       //
//////                                               //////

HandyStr::HandyStr( void )
{
  init();
  reset();
  setSuccess( load() );
}

void HandyStr::init( void )
{
  int count = 0;

  while( count < HANDY_STRINGS )
    usefulStr[ count++ ][ 0 ] = 0;
}

    //////          //////
   //                  //
  // Clean Up Methods //
 //                  //
//////          //////

void HandyStr::reset( void )
{
  setCrewName( "" );
  setUserName( "" );
  setRoomName( "" );
  setUserDesc( "" );
  setLevelName( "" );
  setNumber( 0 );
  setSayMsg( "" );
}

    //////               //////
   //                       //
  // Setter/Getter Methods //
 //                       //
//////               //////

void HandyStr::setSuccess( boolean worked )
{
  success = worked;
}

boolean HandyStr::successful( void )
{
  return( success );
}

boolean HandyStr::setCrewName( char *cName )
{
  strncpy( crewName, cName, MULTIPLE_ALIAS_LEN );
  crewName[ MULTIPLE_ALIAS_LEN ] = 0;
  return( TRUE );
}

boolean HandyStr::setLevelName( char *lName )
{
  strncpy( levelName, lName, LEVEL_NAME_LENGTH );
  levelName[ LEVEL_NAME_LENGTH ] = 0;
  return( TRUE );
}

boolean HandyStr::setNumber( int iNumber )
{
  sprintf( number, "%d", iNumber );
  return( TRUE );
}

boolean HandyStr::setNumber( char *sNumber )
{
  strncpy( number, sNumber, NUMBER_LENGTH );
  number[ NUMBER_LENGTH ] = 0;
  return( TRUE );
}

boolean HandyStr::setRoomName( char *rName )
{
  strncpy( roomName, rName, COMMON_NAME_LEN );
  roomName[ COMMON_NAME_LEN ] = 0;
  return( TRUE );
}

boolean HandyStr::setSayMsg( char *sMessage )
{
  strncpy( sayMessage, sMessage, BIG_SAY_LENGTH );
  sayMessage[ BIG_SAY_LENGTH ] = 0;
  return( TRUE );
}

boolean HandyStr::setUserDesc( char *dName )
{
  strncpy( userDesc, dName, DESCRIPTION_LEN );
  userDesc[ DESCRIPTION_LEN ] = 0;
  return( TRUE );
}

boolean HandyStr::setUserName( char *uName )
{
  strncpy( userName, uName, MULTIPLE_ALIAS_LEN );
  userName[ MULTIPLE_ALIAS_LEN ] = 0;
  return( TRUE );
}

    //////            //////
   //                    //
  // String Persistence //
 //                    //
//////            //////

boolean HandyStr::load( void )
{
  char fileName[ FILE_NAME_LENGTH + 1 ];

  sprintf( fileName, "%s%s", DATA_DIR, HANDY_NAME );
  return( load( fileName ) );
}

// This gives the user a chance to load all these neat strings and have
// a handle on its own, instead of using the default registered strings.
//
// The strings.dat file has a form similar to:
//
// # Ignored comment
// # More ignored comments
//  12 blah blah blah blah!
//
boolean HandyStr::load( char *fileName )
{
  FILE *fp;
  char tempBuff[ HANDY_STRING_LENGTH * 2 + 1 ],
       *buffPtr;
  int  strNumber = 0;

  if( (fp = fopen( fileName, "r" )) == NULL )
    return( FALSE );

  // Now read them in!
  //
  while( fgets( tempBuff, HANDY_STRING_LENGTH * 2, fp ) != NULL )
  {
    // Get rid of the newline character
    //
    tempBuff[ strlen( tempBuff ) - 1 ] = 0;

    // Weed out the number ...
    //
    buffPtr = tempBuff;

    while( isspace( *buffPtr ) && (*buffPtr != 0) )
      buffPtr++;

    // If no digit was found, just ignore the line
    //
    if( !isdigit( *buffPtr ) )
      continue;
    
    // Otherwise, obtain the number, then skip past it to the first real
    // string (strNumber indexes where the string is to be placed within
    // the usefulStr array).
    //
    strNumber = atoi( buffPtr );

    while( (*buffPtr != ' ' )  && (*buffPtr != 0) )
      buffPtr++;

    buffPtr++;

    strncpy( tempBuff, buffPtr, HANDY_STRING_LENGTH );
    tempBuff[ HANDY_STRING_LENGTH ] = 0;

    // Plop it where it belongs.
    //
    if( strNumber < HANDY_STRINGS )
      strcpy( usefulStr[ strNumber ], tempBuff );
  }

  fclose( fp );

  return( TRUE );
}

    //////                 //////
   //                         //
  // Called by other Objects //
 //                         //
//////                 //////

char *HandyStr::getString( int strToGet )
{
  String tempStr = usefulStr[ strToGet ];

  tempStr.gsub( "%w", crewName );
  tempStr.gsub( "%u", userName );
  tempStr.gsub( "%r", roomName );
  tempStr.gsub( "%l", levelName );
  tempStr.gsub( "%desc", userDesc );
  tempStr.gsub( "%number", number );
  tempStr.gsub( "%s", sayMessage );

  strncpy( tempBuff, (const char *)tempStr, SAY_LENGTH );
  reset();
  return( tempBuff );
}

void lowerCase( char *str )
{
  int count = 0;

  while( str[ count ] )
  {
    str[ count ] = (char)tolower( (int)str[ count ] );
    count++;
  }
}
