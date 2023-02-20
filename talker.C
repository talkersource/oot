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
// The Talker class stores things such as shouts, rooms, and
// wizcasts.  It also indicates if the talker has been shut down
// or not.
//
/////////////////////////////////////////////////////////////////////////

#include "talker.h"

    //////                                               //////
   //                                                       //
  // Class Constructors, Destructors, and Initializations  //
 //                                                       //
//////                                               //////

Talker::Talker( void )
{
  TTime aTime;
  char  roomName[ SAY_LENGTH + 1 ];

  printInfo();
  printf( "\nPlease wait ... loading OOT ...\n" );

  validateFiles();

  setNumMacros( 0 );
  loadMacros();
  loadBannedSites();

  // Only run the talker if the room links were loaded succesfully
  //
  setRunning( loadRoomLinks( roomName ) );
  setBootTime( aTime.getCurrentTime() );

  if( roomName[ 0 ] != 0 )
    printf( "\nOOT Error: The room \"%s\" could not be loaded!\n", roomName );
}

Talker::~Talker( void )
{
  RoomPtr room = NULL;
  SayPtr  aSay = NULL;

  saveBannedSites();

  while( (room = (RoomPtr)roomList.at( 0 )) != NULL )
  {
    roomList -= *room;
    delete room;
  }

  while( (aSay = (SayPtr)shoutList.at( 0 )) != NULL )
  {
    shoutList -= *aSay;
    delete aSay;
  }

  while( (aSay = (SayPtr)crewcastList.at( 0 )) != NULL )
  {
    crewcastList -= *aSay;
    delete aSay;
  }
}

    //////          //////
   //                  //
  // Clean Up Methods //
 //                  //
//////          //////

// Goes through the list of rooms and makes sure no rooms are
// adjoined to any other room.
//
boolean Talker::removeRoomLinks( void )
{
  ListPtr rooms;
  RoomPtr room;
  int     roomCount = 0;

  rooms = getRoomList();

  while( (room = (RoomPtr)rooms->at( roomCount++ )) != NULL )
    room->removeAttached();

  return( TRUE );
}

    //////       //////
   //               //
  // Macro Methods //
 //               //
//////       //////

boolean Talker::addMacro( char *newMacro )
{
  strncpy( macros[ getNumMacros() ], newMacro, SAY_LENGTH );
  macros[ getNumMacros() ][ SAY_LENGTH ] = 0;
  setNumMacros( getNumMacros() + 1 );

  return( TRUE );
}

boolean Talker::loadMacros( void )
{
  FILE *fp;
  char fileName[ FILE_NAME_LENGTH + 1 ],
       tempBuff[ SAY_LENGTH + 1 ];

  setNumMacros( 0 );

  sprintf( fileName, "%s%s", DATA_DIR, MACRO_FILENAME );

  if( (fp = fopen( fileName, "r" )) == NULL )
    return( FALSE );

  while( (fgets( tempBuff, SAY_LENGTH, fp ) != NULL) )
    if( tempBuff[ 0 ] == '.' )
    {
      tempBuff[ strlen( tempBuff ) - 1 ] = 0;
      addMacro( tempBuff );
    }

  fclose( fp );
  return( TRUE );
}

char *Talker::getMacroAt( int index )
{
  if( (index < TALKER_MACRO_MAX) && (index > -1) )
    return( macros[ index ] );

  return( NULL );
}

    //////               //////
   //                       //
  // Setter/Getter Methods //
 //                       //
//////               //////

boolean Talker::setBootTime( time_t newBootTime )
{
  bootTime = newBootTime;

  return( TRUE );
}

time_t Talker::getBootTime( void )
{
  return( bootTime );
}

// Return NULL if the siteNum went beyond the index
//
char *Talker::getBannedSite( int siteNum )
{
  if( (siteNum > getNumBannedSites()) || (siteNum < 1) )
    return( NULL );

  return( bannedSites[ siteNum - 1 ] );
}

boolean Talker::setNumBannedSites( int numSites )
{
  bannedIndex = numSites;

  return( TRUE );
}

int Talker::getNumBannedSites( void )
{
  return( bannedIndex );
}

boolean Talker::setNumMacros( int newNum )
{
  if( (newNum < TALKER_MACRO_MAX) && (newNum > -1) )
    numMacros = newNum;

  return( TRUE );
}

int Talker::getNumMacros( void )
{
  return( numMacros );
}

boolean Talker::setRunning( boolean toSet )
{
  running = toSet;

  return( TRUE );
}

boolean Talker::getRunning( void )
{
  return( running );
}

boolean Talker::addToCrewcastList( Say aCrewcast )
{
  SayPtr wasCasted = NULL,
         buffer    = NULL;

  wasCasted = new Say( aCrewcast.getMessage(),
                       aCrewcast.getUserFrom(),
                       aCrewcast.getSayType() );

  // Get rid of the "first" crewcast
  //
  if( crewcastList.at( MAX_CREWCASTS - 1 ) != NULL )
  {
    buffer        = (SayPtr)crewcastList.at( MAX_CREWCASTS - 1 );
    crewcastList -= *buffer;
    delete buffer;
  }

  // Add the newest tell to the end of the buffer.
  //
  return( crewcastList += *wasCasted );
}

ListPtr Talker::getCrewcastList( void )
{
  return( &crewcastList );
}

ListPtr Talker::getRoomList( void )
{
  return( &roomList );
}

boolean Talker::addToShoutList( Say aShout )
{
  SayPtr wasShouted = NULL,
         buffer     = NULL;

  wasShouted = new Say( aShout.getMessage(),
                        aShout.getUserFrom(),
                        aShout.getSayType() );

  wasShouted->setInvisible( aShout.getInvisible() );

  // Get rid of the "first" shout
  //
  if( shoutList.at( MAX_SHOUTS - 1 ) != NULL )
  {
    buffer     = (SayPtr)shoutList.at( MAX_SHOUTS - 1 );
    shoutList -= *buffer;
    delete buffer;
  }

  // Add the newest tell to the end of the buffer.
  //
  return( shoutList += *wasShouted );
}

ListPtr Talker::getShoutList( void )
{
  return( &shoutList );
}

    //////       //////
   //               //
  // Query Methods //
 //               //
//////       //////

// Return TRUE if the rooms are near each other:
//
boolean Talker::areAdjoined( RoomPtr roomA, RoomPtr roomB )
{
  return( roomA->isAdjoined( roomB ) );
}

boolean Talker::isRoomLoaded( char *roomName )
{
  return( findRoom( roomName ) != NULL );
}

boolean Talker::isBannedSite( char *site )
{
  int count = getNumBannedSites();
  char *tempBuff;

  while( (tempBuff = getBannedSite( count-- )) != NULL )
    if( strstr( site, tempBuff ) != NULL )
      return( TRUE );

  return( FALSE );
}

    //////        //////
   //                //
  // Search Methods //
 //                //
//////        //////

// A Talker knows all about rooms, and hence users.  Thus is should
// be able to search for any of them given a name OR a pointer.
//
// All "findX()" methods should return a pointer corresponding to the
// name X.

// Return NULL if roomName wasn't found, otherwise return a pointer
// to the room which FIRST matches roomName.  No multiplicity; suffer.
//
RoomPtr Talker::findRoom( char *roomName )
{
  RoomPtr room  = NULL;
  int     count = 0;
  char    tempName[ COMMON_NAME_LEN + 1 ],
          lowerName[ COMMON_NAME_LEN + 1 ];

  strncpy( lowerName, roomName, COMMON_NAME_LEN );
  lowerName[ COMMON_NAME_LEN ] = 0;
  lowerCase( lowerName );

  while( (room = (RoomPtr)roomList.at( count++ )) != NULL )
  {
    strncpy( tempName, room->getName(), COMMON_NAME_LEN );
    tempName[ COMMON_NAME_LEN ] = 0;
    lowerCase( tempName );

    //if( strncmp( tempName, lowerName, strlen( lowerName ) ) == 0 )
    if( strncmp( lowerName, tempName, strlen( lowerName ) ) == 0 )
      return( room );
  }

  return( room );
}

RoomPtr Talker::findRoom( int index )
{
  // The list is a stack ...
  //
  return( (RoomPtr)roomList.at( roomList.howMany() - index - 1 ) );
}

// Return NULL if no, or multiple matches were found
//
UserPtr Talker::findUser( char *userName )
{
  RoomPtr room  = NULL;
  UserPtr user  = NULL;
  int     count = 0;
  char    tempBuff[ COMMON_NAME_LEN + 1 ];

  strncpy( tempBuff, userName, COMMON_NAME_LEN );
  tempBuff[ COMMON_NAME_LEN ] = 0;
  lowerCase( tempBuff );

  if( findUserMatches( tempBuff ) > 1 )
    return( NULL );

  while( (room = (RoomPtr)roomList.at( count++ )) != NULL )
    if( (user = room->findUser( tempBuff )) != NULL )
      return( user );

  return( user );
}

// Find if the user name passed exactly matches a user that is online;
// return NULL if none.
//
UserPtr Talker::findUserExact( char *userName )
{
  RoomPtr room  = NULL;
  UserPtr user  = NULL;
  int     count = 0;
  char    tempName[ COMMON_NAME_LEN + 1 ];

  strncpy( tempName, userName, COMMON_NAME_LEN );
  tempName[ COMMON_NAME_LEN ] = 0;
  lowerCase( tempName );

  while( (room = (RoomPtr)roomList.at( count++ )) != NULL )
    if( (user = room->findUserExactMatch( tempName )) != NULL )
      return( user );

  return( NULL );
}

// Return the number of names in the talker that a username matches
//
int Talker::findUserMatches( char *userName )
{
  RoomPtr room    = NULL;
  int     count   = 0,
          matches = NO_MATCHES;

  while( (room = (RoomPtr)roomList.at( count++ )) != NULL )
    matches += room->findUserMatches( userName );

  return( matches );
}

// Return NULL if no exact match was found.  Otherwise
// return a pointer to the room which contains userName.
//
RoomPtr Talker::findUserRoom( char *userName )
{
  RoomPtr room  = NULL;
  int     count = 0;

  while( (room = (RoomPtr)roomList.at( count++ )) != NULL )
    if( room->findUserExactMatch( userName ) != NULL )
      return( room );

  return( room );
}

// Return the room which user is in.
//
RoomPtr Talker::findUserRoom( UserPtr user )
{
  RoomPtr room  = NULL;
  int     count = 0;

  while( (room = (RoomPtr)roomList.at( count++ )) != NULL )
    if( room->findUser( user ) == TRUE )
      return( room );

  return( room );
}

    //////             //////
   //                     //
  // Banned Site Methods //
 //                     //
//////             //////

boolean Talker::addBannedSite( char *site )
{
  int numBanned = getNumBannedSites();

  if( (numBanned < MAX_BANNED_SITES) && (isBannedSite( site ) == FALSE) )
  {
    strncpy( bannedSites[ numBanned ], site, SITE_LENGTH );
    setNumBannedSites( numBanned + 1 );
    return( TRUE );
  }

  return( FALSE );
}

boolean Talker::loadBannedSites( void )
{
  FILE *fp;
  FileName fileName;
  char tempBuff[ SAY_LENGTH + 1 ];

  setNumBannedSites( 0 );

  fileName.setFileName( DATA_DIR, BAN_FILENAME, "" );

  if( (fp = fopen( fileName.getFileName(), "r" )) == NULL )
    return( TRUE );

  while( fgets( tempBuff, SITE_LENGTH, fp ) != NULL )
  {
    tempBuff[ strlen( tempBuff ) - 1 ] = 0;
    addBannedSite( tempBuff );
  }

  fclose( fp );

  return( TRUE );
}

boolean Talker::saveBannedSites( void )
{
  FILE *fp;
  FileName fileName;
  int  count = getNumBannedSites();

  fileName.setFileName( DATA_DIR, BAN_FILENAME, "" );

  if( (fp = fopen( fileName.getFileName(), "w+" )) == NULL )
    return( FALSE );

  while( count > 0 )
    fprintf( fp, "%s\n", getBannedSite( count-- ) );

  fclose( fp );
  return( TRUE );
}

boolean Talker::unBanSite( char *site )
{
  return( TRUE );
}

    //////           //////
   //                   //
  // Room Load Methods //
 //                   //
//////           //////

boolean Talker::addRoom( RoomPtr room )
{
  return( roomList += *room );
}

boolean Talker::addRoomNamed( char *name )
{
  RoomPtr room = NULL;

  // If the room is already in memory, just re-load it
  //
  if( (room = findRoom( name )) != NULL )
    return( room->load() );

  if( (room = new Room( name )) == NULL )
    return( FALSE );

  if( room->load() == FALSE )
  {
    delete( room );
    return( FALSE );
  }

  return( addRoom( room ) );
}

// Load how the rooms are linked for the talker.  Load them as they are
// discovered while loading the links.
//
// The roomlink.dat file has the following definition:
//
//   Logon Limbo
//   Main
//   Jail Room
//   Main  > RoomA
//   Main  < RoomB
//   Main  = RoomC
//   RoomA = RoomD
//
// Which means:
//   1) logonlimbo is the room (since it is first) that the users get
//      placed upon telnetting into the talker
//   2) main is the room (since it is second) that the users get placed
//      upon entering a valid password (or are a new user)
//   2) From main you CAN go to roomA; from roomA you CANNOT go to main
//   3) From main you CANNOT go to roomB; from roomB you CAN go to main
//   4) From main you CAN go to roomC; and vice-versa
//   5) From roomA you CAN go to roomD; and vice-versa
//
// If this method returns FALSE, the talker must not be permitted to
// continue running.  It will copy into "buffer" the name of the room
// if it couldn't load it.
//
boolean Talker::loadRoomLinks( char *badRoom )
{
  FILE     *fp;
  FileName fileName;

  char tempBuff[ SAY_LENGTH + 1 ],
       roomName[ COMMON_NAME_LEN + 2 ],
       parseRoom[ SCREEN_COLUMNS + 1 ],   // String loaded from link file
       *parse,                            // For setting things to ASCII-Z
       *parseWhite,                       // Parse whitespace
       tempCh;                            // Holder for < > or =

  RoomPtr roomA, roomB;

  removeRoomLinks();

  fileName.setFileName( DATA_DIR, ROOM_LINK, "" );

  // Open the room map data file for reading
  //
  if( (fp = fopen( fileName.getFileName(), "r" ) ) == NULL )
    return( FALSE );

  // Read the name of the first room, and load it.  This is a special
  // case because it is the room in which users get logged into upon
  // telnetting into the talker.
  //
  if( fgets( roomName, COMMON_NAME_LEN + 1, fp ) == NULL )
    return( FALSE );

  roomName[ strlen( roomName ) - 1 ] = 0;

  // This is adding "Logon Limbo" to the talker--first room a user sees
  //
  if( addRoomNamed( roomName ) == FALSE )
  {
    strcpy( badRoom, roomName );
    return( FALSE );
  }

  // Read the name of the second room, and load it
  //
  if( fgets( roomName, COMMON_NAME_LEN + 1, fp ) == NULL )
    return( FALSE );

  roomName[ strlen( roomName ) - 1 ] = 0;

  // This is adding "Main Deck" to the talker--first room a user can
  // actually do stuff in (for now).  Possibly putting them into the
  // room which they left will come later ... (doubtful)
  //
  if( addRoomNamed( roomName ) == FALSE )
  {
    strcpy( badRoom, roomName );
    return( FALSE );
  }

  // Read the name of the third room, and load it
  //
  if( fgets( roomName, COMMON_NAME_LEN + 1, fp ) == NULL )
    return( FALSE );

  roomName[ strlen( roomName ) - 1 ] = 0;

  // This is adding "The Brig".  The room which a user is forceably placed
  // when arrested.
  //
  if( addRoomNamed( roomName ) == FALSE )
  {
    strcpy( badRoom, roomName );
    return( FALSE );
  }

  // Now for the fun part--parsing the rest of the file
  //
  while( fgets( parseRoom, SCREEN_COLUMNS, fp ) != NULL )
  {
    // Get rid of whitespace from the end of the string
    //
    parse = &parseRoom[ strlen( parseRoom ) - 1 ];

    while( isspace( *parse ) )
      parse--;

    *(parse + 1) = 0;

    // Find the name on the left hand side, by finding the first
    // character that indicates one of the documented definitions
    //
    parse = strpbrk( parseRoom, "<>=" );

    if( parse != NULL )
    {
      tempCh     = *parse;
      parseWhite = parse - 1;

      // Read back from parse, deleting any whitespace (if any)
      //
      while( isspace( *parseWhite ) )
        parseWhite--;

      *(parseWhite + 1) = 0;

      // Try to add the room to the talker (this will reload it if
      // the room already exists) ...
      //
      if( addRoomNamed( parseRoom ) == FALSE )
      {
        strcpy( badRoom, parseRoom );
        fclose( fp );
        return( FALSE );
      }

      // Find the next room name
      //
      parse++;

      while( (*parse != 0) && isspace( *parse ) )
        parse++;

      strcpy( roomName, parse );

      // If the second room couldn't be loaded, then simply move on
      // to the next line in the room link file--this is an error
      // condition, but isn't so fatal as to 'cause a crash.
      //
      if( addRoomNamed( roomName ) == FALSE )
        continue;

      roomA = findRoom( parseRoom );
      roomB = findRoom( roomName );

      if( (roomB == NULL) || (roomA == NULL) )
        return( FALSE );

      if( roomA == roomB )
        continue;

      switch( tempCh )
      {
        // roomA is the room that you are in; roomB is the room
        // that you can get to from roomA.  But not vice-versa.
        //
        case '>':
          roomA->addAttachedRoom( roomB );
          break;

        // See the above comment?  Reverse it.
        //
        case '<':
          roomB->addAttachedRoom( roomA );
          break;

        // Go ahead, figure it out for yourself.
        //
        case '=':
          roomA->addAttachedRoom( roomB );
          roomB->addAttachedRoom( roomA );
          break;
      }
    }
    else if( strlen( parseRoom ) > 0 )
    {
      // If nothing was found, and there is something that was read,
      // assume it is the name of a room.  Try to load it, but don't
      // make any attachments.
      //
      // If it couldn't be added, then something has gone really wrong
      //
      if( addRoomNamed( parseRoom ) == FALSE )
      {
        strcpy( badRoom, parseRoom );
        fclose( fp );
        return( FALSE );
      }
    }
  }

  fclose( fp );

  return( TRUE );
}

    //////       //////
   //               //
  // Misc. Methods //
 //               //
//////       //////

void Talker::printInfo( void )
{
  printf( "OOT -- The Object Oriented Talker.\n" );
  printf( "%s\n", TALKER_VERSION );
}

void Talker::validateFile( char *fName )
{
  FILE *fp;
  FileName fileName;

  fileName.setFileName( DATA_DIR, fName, "" );
  if( (fp = fopen( fileName.getFileName(), "r" )) == NULL )
    printf( "OOT Error: The file \"%s\" could not be loaded!\n", fileName.getFileName() );
  else
    fclose( fp );
}

// Ensure the required files exist, and the directory structure is fine.
//
boolean Talker::validateFiles( void )
{
  FILE     *fp;
  FileName fileName;

  printf( "\nVerifying files & directories ...\n" );

  // Check for required files in data directory
  //
  validateFile( CMDS_FILENAME );
  validateFile( LEVEL_FILENAME );
  validateFile( ROOM_LINK );
  validateFile( HANDY_NAME );
  return( TRUE );
}

