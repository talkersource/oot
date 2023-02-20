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
// This class defines the behaviour for a room.
//
// A couple things to note:
//
//   1) When a user is removed from the room, it is then checked to see
//      if the room should still be locked or not
//
//   2) When a room is unlocked the buffer is cleared
//
/////////////////////////////////////////////////////////////////////////

#include "room.h"

    //////                                               //////
   //                                                       //
  // Class Constructors, Destructors, and Initializations  //
 //                                                       //
//////                                               //////

Room::Room( char *title )
{
  init( title );
}

Room::~Room( void )
{
  SayPtr  wasSaid = NULL;
  UserPtr user    = NULL;

  // Clear memory for all the users in the room
  //
  while( (user = (UserPtr)userList.at( 0 )) != NULL )
  {
    userList -= *user;
    delete user;
  }

  // Free memory for the review buffer
  //
  while( (wasSaid = (SayPtr)sayList.at( 0 )) != NULL )
  {
    sayList -= *wasSaid;
    delete wasSaid;
  }
}

void Room::init( char *title )
{
  setName( title );
  setTopic( "<No Topic>" );
  setLocked( FALSE );
  clearReviewBuffer();

  // Some default access levels
  //
  setLevel( 1 );
  setLockRank( 2 );
  setViewRank( 1 );
  setWarpRank( 3 );
  setReadRank( 2 );
  setWriteRank( 2 );
  setWipeRank( 3 );
  setReviewRank( 2 );

  // If the room exists, load it.  This will fail if the room doesn't
  // exist (but who cares?).
  //
  load();
  setNumMessages( countMessages() );
}

// Count the number of messages on the message board.  This accesses the
// "roomname.msg" file, and is a fairly expensive operation.  Thus it is
// only used when the room is loaded up ...
//
int Room::countMessages( void )
{
  FILE     *fp;
  FileName fileName;

  char tempBuff[ SAY_LENGTH + 1 ],
       userFrom[ COMMON_NAME_LEN + 5 ];

  int  count = 0;
  long messageTime;

  fileName.setFileName( ROOM_DIR, getName(), ROOM_BOARD_EXT );

  if( (fp = fopen( fileName.getFileName(), "r" )) == NULL )
    return( 0 );

  while( fscanf( fp, "%lu %s", &messageTime, userFrom ) != EOF )
  {
    fgets( tempBuff, SAY_LENGTH, fp );
    count++;
  }

  fclose( fp );
  return( count );
}

    //////          //////
   //                  //
  // Clean Up Methods //
 //                  //
//////          //////

boolean Room::clearReviewBuffer( void )
{
  SayPtr wasSaid = NULL;

  while( (wasSaid = (SayPtr)sayList.at( 0 )) != NULL )
  {
    sayList -= *wasSaid;
    delete wasSaid;
  }

  return( TRUE );
}

boolean Room::removeAttached( void )
{
  RoomPtr room;

  while( (room = (RoomPtr)attachedRooms.at( 0 )) != NULL )
    attachedRooms -= *room;

  return( TRUE );
}

    //////               //////
   //                       //
  // Setter/Getter Methods //
 //                       //
//////               //////

boolean Room::addAttachedRoom( RoomPtr room )
{
  return( attachedRooms += *room );
}

ListPtr Room::getAttachedRooms( void )
{
  return( &attachedRooms );
}

boolean Room::setLevel( iLevel l )
{
  enterRank = l;
  return( TRUE );
}

iLevel Room::getLevel( void )
{
  return( enterRank );
}

boolean Room::setLocked( boolean lockIt )
{
  locked = lockIt;

  if( !locked )
    clearReviewBuffer();

  return( TRUE );
}

boolean Room::getLocked( void )
{
  return( locked );
}

boolean Room::setLockRank( iLevel l )
{
  lockRank = l;
  return( TRUE );
}

iLevel Room::getLockRank( void )
{
  return( lockRank );
}

boolean Room::setNumMessages( int newNumber )
{
  if( newNumber < 0 )
    newNumber = 0;

  numMessages = newNumber;
  return( TRUE );
}

int Room::getNumMessages( void )
{
  return( numMessages );
}

boolean Room::setReadRank( iLevel l )
{
  readRank = l;
  return( TRUE );
}

iLevel Room::getReadRank( void )
{
  return( readRank );
}

boolean Room::addToReviewBuffer( Say aSay )
{
  SayPtr wasSaid = NULL,
         buffer  = NULL;

  wasSaid = new Say( aSay.getMessage(),
                     aSay.getUserFrom(),
                     aSay.getSayType() );

  wasSaid->setInvisible( aSay.getInvisible() );

  if( sayList.at( REVIEW_BUFFER_LEN ) != NULL )
  {
    buffer = (SayPtr)sayList.at( REVIEW_BUFFER_LEN );
    sayList -= *buffer;
    delete buffer;
  }

  // Add it to the end of the buffer.
  //
  sayList += *wasSaid;

  return( TRUE );
}

ListPtr Room::getReviewBuffer( void )
{
  return( &sayList );
}

SayPtr Room::getReviewBufferAt( int index )
{
  return( (SayPtr)sayList.at( index ) );
}

boolean Room::setReviewRank( iLevel l )
{
  reviewRank = l;
  return( TRUE );
}

iLevel Room::getReviewRank( void )
{
  return( reviewRank );
}

boolean Room::setTopic( char *top )
{
  strncpy( topic, top, TOPIC_LENGTH );
  topic[ TOPIC_LENGTH ] = 0;
  return( TRUE );
}

char *Room::getTopic( void )
{
  return( topic );
}

ListPtr Room::getUserList( void )
{
  return( &userList );
}

boolean Room::setViewRank( iLevel l )
{
  viewRank = l;
  return( TRUE );
}

iLevel Room::getViewRank( void )
{
  return( viewRank );
}

boolean Room::setWarpRank( iLevel l )
{
  warpRank = l;
  return( TRUE );
}

iLevel Room::getWarpRank( void )
{
  return( warpRank );
}

boolean Room::setWipeRank( iLevel l )
{
  wipeRank = l;
  return( TRUE );
}

iLevel Room::getWipeRank( void )
{
  return( wipeRank );
}

boolean Room::setWriteRank( iLevel l )
{
  writeRank = l;
  return( TRUE );
}

iLevel Room::getWriteRank( void )
{
  return( writeRank );
}

void Room::addUser( UserPtr user )
{
  user->uninvite( this );

  userList += *user;
}

void Room::removeUser( UserPtr user )
{
  UserPtr userInRoom = NULL;

  user->uninvite( this );

  userList -= *user;

  if( (userInRoom = (UserPtr)userList.at( 0 )) == NULL )
  {
    if( getLocked() == TRUE )
      setLocked( FALSE );

    return;
  }

  if( userList.at( 1 ) == NULL )
    if( (userInRoom->getLevel() < getLockRank()) && (getLocked() == TRUE) )
      setLocked( FALSE );
}

    //////       //////
   //               //
  // Query Methods //
 //               //
//////       //////

boolean Room::isAdjoined( RoomPtr room )
{
  RoomPtr tempRoom  = NULL;
  int     roomCount = 0;

  while( (tempRoom = (RoomPtr)attachedRooms.at( roomCount++ )) != NULL )
    if( tempRoom == room )
      return( TRUE );

  return( FALSE );
}

    //////        //////
   //                //
  // Search Methods //
 //                //
//////        //////

// Search the room for a given user (or abbreviation).  Return NULL
// if either the name wasn't found, or multiple matches.
//
UserPtr Room::findUser( char *userName )
{
  UserPtr user  = NULL;
  int     count = 0;
  char    tempAlias[ COMMON_NAME_LEN + 1 ],
          tempName[ COMMON_NAME_LEN + 1 ];

  strncpy( tempName, userName, COMMON_NAME_LEN );
  tempName[ COMMON_NAME_LEN ] = 0;
  lowerCase( tempName );

  if( findUserMatches( tempName ) > 1 )
    return( NULL );

  while( (user = (UserPtr)userList.at( count++ )) != NULL )
  {
    strcpy( tempAlias, user->getName() );
    lowerCase( tempAlias );

    if( strncmp( tempAlias, tempName, strlen( tempName ) ) == 0 )
      return( user );
  }

  return( user );
}

// Return TRUE if user is within the room
//
boolean Room::findUser( UserPtr user )
{
  UserPtr tempUser = NULL;
  int     count    = 0;

  while( (tempUser = (UserPtr)userList.at( count++ )) != NULL )
    if( user == tempUser )
      return( TRUE );

  return( FALSE );
}

// Counts the number of partial user name matches in a room, or returns
// 1 if the match was exact.
//
int Room::findUserMatches( char *userName )
{
  UserPtr user    = NULL;
  int     count   = 0,
          matches = 0;
  char    tempAlias[ COMMON_NAME_LEN + 1 ],
          tempName[ COMMON_NAME_LEN + 1 ];

  strncpy( tempName, userName, COMMON_NAME_LEN );
  tempName[ COMMON_NAME_LEN ] = 0;
  lowerCase( tempName );

  while( (user = (UserPtr)userList.at( count++ )) != NULL )
  {
    strncpy( tempAlias, user->getName(), COMMON_NAME_LEN );
    tempAlias[ COMMON_NAME_LEN ] = 0;
    lowerCase( tempAlias );

    if( strcmp( tempAlias, tempName ) == 0 )
      return( 1 );

    if( strncmp( tempAlias, tempName, strlen( tempName ) ) == 0 )
      matches++;
  }

  return( matches );
}

// Return NULL if no user with a name of userName could be found
//
UserPtr Room::findUserExactMatch( char *userName )
{
  UserPtr user  = NULL;
  int     count = 0;
  char    tempAlias[ COMMON_NAME_LEN + 1 ];

  while( (user = (UserPtr)userList.at( count++ )) != NULL )
  {
    strcpy( tempAlias, user->getName() );
    lowerCase( tempAlias );

    if( strcmp( tempAlias, userName ) == 0 )
      return( user );
  }

  return( user );
}

    //////          //////
   //                  //
  // Room Persistence //
 //                  //
//////          //////

// A room must have a name; this is safe
//
boolean Room::load( void )
{
  return( load( getName() ) );
}

// Load in a given room.  Return FALSE if no name could be found.
//
boolean Room::load( char *title )
{
  FILE     *fp;
  FileName fileName;

  char buffer[ SAY_LENGTH + 1 ],
       lowerBuff[ SAY_LENGTH + 1 ];    // Lower case version of buffer

  int  infoCount = 0,
       done      = FALSE,
       foundName = FALSE;

  fileName.setFileName( ROOM_DIR, title, "" );

  if( (fp = fopen( fileName.getFileName(), "r" )) == NULL )
    return( FALSE );

  profile.clear();

  // Parse out the file ...
  //
  while( fgets( buffer, SAY_LENGTH, fp ) != NULL )
  {
    strcpy( lowerBuff, buffer );
    lowerCase( lowerBuff );

    if( strncmp( "name", lowerBuff, 4 ) == 0 )
    {
      buffer[ strlen( buffer ) - 1 ] = 0;
      setName( splitString( buffer, '=' ) );
      foundName = TRUE;
    }
    else if( strncmp( "access", lowerBuff, 6 ) == 0 )
      setLevel( (iLevel)atoi( splitString( buffer, '=' ) ) );
    else if( strncmp( "lock", lowerBuff, 4 ) == 0 )
      setLockRank( (iLevel)atoi( splitString( buffer, '=' ) ) );
    else if( strncmp( "lock", lowerBuff, 4 ) == 0 )
      setLockRank( (iLevel)atoi( splitString( buffer, '=' ) ) );
    else if( strncmp( "review", lowerBuff, 6 ) == 0 )
      setReviewRank( (iLevel)atoi( splitString( buffer, '=' ) ) );
    else if( strncmp( "view", lowerBuff, 4 ) == 0 )
      setViewRank( (iLevel)atoi( splitString( buffer, '=' ) ) );
    else if( strncmp( "read", lowerBuff, 4 ) == 0 )
      setReadRank( (iLevel)atoi( splitString( buffer, '=' ) ) );
    else if( strncmp( "warp", lowerBuff, 4 ) == 0 )
      setWarpRank( (iLevel)atoi( splitString( buffer, '=' ) ) );
    else if( strncmp( "write", lowerBuff, 5 ) == 0 )
      setWriteRank( (iLevel)atoi( splitString( buffer, '=' ) ) );
    else if( (strncmp( "<desc>", lowerBuff, 6 ) == 0) && !done )
    {
      // Read in the room's profile
      //
      while( (fgets( buffer, SAY_LENGTH, fp ) != NULL) && !done )
      {
        done = (strncmp( "</desc>", buffer, 7 ) == 0);
        buffer[ strlen( buffer ) - 1 ] = 0;

        if( !done )
          done = (profile.addLine( buffer ) == FALSE);
      }
    }
  }

  fclose( fp );

  return( foundName );
}

// There are some things about a room you just want to keep persistent!
//
boolean Room::save( void )
{
  int   count = 0,
        speed = profile.getNumLines();

  FILE     *fp;
  FileName fileName;

  char  tempBuff[ SAY_LENGTH + 1 ];

  fileName.setFileName( ROOM_DIR, getName(), "" );

  // Overwrite the file
  //
  if( (fp = fopen( fileName.getFileName(), "w" )) == NULL )
    return( FALSE );

  // Save the room's stuff
  //
  fprintf( fp, "Name = %s\n", getName() );
  fprintf( fp, "Access = %u\nLock = %u\nRead = %u\nReview = %u\nView = %u\nWarp = %u\nWipe = %u\nWrite = %u\n",
               getLevel(), getLockRank(), getReadRank(), getReviewRank(),
               getViewRank(), getWarpRank(), getWipeRank(), getWriteRank() );

  fprintf( fp, "\n<desc>\n" );

  // Save the room's profile
  //
  while( count < speed )
    fprintf( fp, "%s\n", profile.getLine( count++ ) );

  fprintf( fp, "</desc>\n" );
  fclose( fp );
  return( TRUE );
}

    //////       //////
   //               //
  // Misc. Methods //
 //               //
//////       //////

int Room::countUsers( void )
{
  return( userList.howMany() );
}
