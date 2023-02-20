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
// Message class encapsulates reading and writing dated messages to
// files.
//
/////////////////////////////////////////////////////////////////////////

#include "message.h"

Message::Message( void )
{
  setFileName( "" );
}

Message::~Message( void )
{
}

// See if messageNumber is part of lineNumbers; return TRUE if it was.
//
boolean Message::parseNumbers( char *lineNumbers, int messageNumber )
{
  char copyBuff[ SAY_LENGTH + 1 ],   // A copy of lineNumbers
       *tempBuff,
       *peekBuff;             // For peeking to the left of a -
  int  minNumber = 0,         // Used for checking contexts
       maxNumber = 0;

  // See if the messageNumber lands within - symbols, within contexts:
  //     -44     (1)
  //   14-44     (2)
  //   44-       (3)
  //
  // A 0 return from atoi() means there was an error in converting (user's
  // messages are numbered from 1 to n)
  //

  // The space before the %s is required (allows isdigit to look one char
  // before a - sign in context (1))
  //
  sprintf( copyBuff, " %s", lineNumbers );

  tempBuff = copyBuff;

  // Only parse out - signs--if there are any ;-)
  //
  while( (tempBuff = strchr( copyBuff, '-')) != NULL )
  {
    // Blank out the '-' with a * (arbitrary: can be anything save - or #)
    //
    *tempBuff = '*';
    peekBuff  = tempBuff - 1;

    // Is there a number to the left?
    //
    if( isdigit( *peekBuff ) )
    {
      // Figure out where the number stops ...
      //
      while( isdigit( *(--peekBuff) ) )
        ;

      // Convert the string to a number
      //
      minNumber = atoi( ++peekBuff );

      // Get the number to the right (if there is any)
      //
      if( isdigit( tempBuff[ 1 ] ) )
      {
        maxNumber = atoi( &tempBuff[ 1 ] );

        // If the messageNumber is within the bounds of context (2) then
        // it is valid
        //
        if( (messageNumber >= minNumber) && (messageNumber <= maxNumber) &&
            (maxNumber >= minNumber) )
          return( TRUE );
      }
      else
      // There is only a number to the left of the '-'; context (3)
      //
      if( (messageNumber >= minNumber) && (minNumber > 0) )
        return( TRUE );
    }
    else
    {
      // What is the number to the right of the '-'; context (1)
      //
      maxNumber = atoi( &tempBuff[ 1 ] );

      // Is the messageNumber less than (or equal to) the specified number?
      //
      if( (messageNumber <= maxNumber) && (maxNumber != 0) )
        return( TRUE );
    }
  }

  // Reset copyBuff (all '-' signs have been changed!)
  //
  strcpy( copyBuff, lineNumbers );

  tempBuff = copyBuff;

  while( tempBuff != NULL )
  {
    // Get the number that was found
    //
    minNumber = atoi( tempBuff );

    if( messageNumber == minNumber )
      return( TRUE );

    // Look past the next comma; point tempBuff to the next number
    //
    if( (peekBuff = strchr( tempBuff, ',' )) != NULL )
      tempBuff = ++peekBuff;
    else
      tempBuff = NULL;
  }

  return( FALSE );
}

boolean Message::appendMessage( UserPtr user, char *msg )
{
  TTime aTime;
  FILE  *fp;

  if(  (strcmp( "", getFileName() ) == 0) ||
      ((fp = fopen( getFileName(), "a" )) == NULL) )
    return( FALSE );

  // Append the date & time, who it was from, and what the message was
  //
  fprintf( fp, "%lu %c%s%c: %s\n",
               aTime.getCurrentTime(),
               HIGHLIGHT_CHAR,
               user->getName(),
               HIGHLIGHT_CHAR,
               msg );

  fclose( fp );
  return( TRUE );
}

// Return the number of messages read, or -1 if there were none (either
// there were none on the message board; the numbers passed didn't match;
// or the toSearch string came up without any hits).
//
int Message::readFile( UserPtr user, char *numbers, char *toSearch )
{
  Say   toSay;
  TTime aTime;
  FILE  *fp;

  char  userFrom[ COMMON_NAME_LEN + 5 ],
        timeBuff[ MAX_TIME_LENGTH + 1 ],
        messageBuff[ SAY_LENGTH + 1 ],
        tempBuff[ SAY_LENGTH * 2 + 1 ],
        lowerBuff[ SAY_LENGTH * 2 + 1 ],
        searchBuff[ SAY_LENGTH + 1 ];

  int   msgNumber = 0,
        msgCount  = 0;

  long  messageTime = 0;

  if(  (strcmp( "", getFileName() ) == 0) ||
      ((fp = fopen( getFileName(), "r" )) == NULL) )
    return( -1 );

  strncpy( searchBuff, toSearch, SAY_LENGTH );
  searchBuff[ SAY_LENGTH ] = 0;
  lowerCase( searchBuff );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  while( fscanf( fp, "%lu %s", &messageTime, userFrom ) != EOF )
  {
    fgets( messageBuff, SAY_LENGTH, fp );
    messageBuff[ SAY_LENGTH ] = 0;
    messageBuff[ strlen( messageBuff ) - 1 ] = 0;

    msgNumber++;

    aTime.difference( messageTime, aTime.getCurrentTime(), timeBuff );

    if( (strlen( numbers ) > 0) &&
        (parseNumbers( numbers, msgNumber ) == FALSE) )
      continue;

    sprintf( tempBuff, "[^6%2d^] %s %s%s",
                       msgNumber,
                       timeBuff,
                       userFrom,
                       messageBuff );

    // If the user decided to search for something within smail/message boards,
    // then let her!
    //
    if( searchBuff[ 0 ] != 0 )
    {
      sprintf( lowerBuff, "%s%s", userFrom, messageBuff );
      lowerCase( lowerBuff );

      if( strstr( lowerBuff, searchBuff ) == NULL )
        continue;
    }

    toSay.setMessage( tempBuff );
    user->sendString( toSay );
    msgCount++;
  }

  fclose( fp );
  return( msgCount );
}

int Message::readSuggest( UserPtr user )
{
  FILE *fp;
  char tempBuff[ SAY_LENGTH + 1 ],
       readBuff[ SAY_LENGTH * 2 + 1 ];
  Say  toSay;
  int  count = 0;

  if(  (strcmp( "", getFileName() ) == 0) ||
      ((fp = fopen( getFileName(), "r" )) == NULL) )
    return( FALSE );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  while( fgets( readBuff, SAY_LENGTH * 2, fp ) != NULL )
  {
    readBuff[ strlen( readBuff ) - 1 ] = 0;

    count++;
    sprintf( tempBuff, "[^5%2d^] %s", count, readBuff );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );
  }
}

int Message::removeLines( char *numbers )
{
  FILE *fIn,
       *fOut;

  char outFile[ FILE_NAME_LENGTH + 1 ],
       messageBuff[ SAY_LENGTH * 2 + 1 ];

  int  msgNumber   = 0,
       deleteCount = 0;

  if( (fIn = fopen( getFileName(), "r" )) == NULL )
    return( -1 );

  // Create a unique temporary file for deleting mail.
  //
  sprintf( outFile, "%s%s", fileName, TEMP_EXTENSION );

  if( (fOut = fopen( outFile, "w" )) == NULL )
  {
    fclose( fIn );
    return( -1 );
  }

  while( fgets( messageBuff, SAY_LENGTH * 2, fIn ) != NULL )
  {
    msgNumber++;

    // If the number matched, then don't write that line out to the file
    // (i.e., delete the line)
    //
    if( parseNumbers( numbers, msgNumber ) == TRUE )
      continue;

    deleteCount++;

    fputs( messageBuff, fOut );
  }

  fclose( fOut );
  fclose( fIn );

  // Swap the old smail with the resulting smail file
  //
  unlink( getFileName() );
  rename( outFile, getFileName() );

  return( msgNumber - deleteCount );
}

boolean Message::suggestMessage( char *msg )
{
  FILE *fp;

  if(  (strcmp( "", getFileName() ) == 0) ||
      ((fp = fopen( getFileName(), "a" )) == NULL) )
    return( FALSE );

  // Append the message to the suggestion file
  //
  fprintf( fp, "%s\n", msg );
  fclose( fp );
  return( TRUE );
}

boolean Message::appendMessage( UserPtr user, char *msg, char *fName )
{
  setFileName( fName );
  return( appendMessage( user, msg ) );
}

int Message::readFile( UserPtr user, char *numbers, char *toSearch, char *fName )
{
  setFileName( fName );
  return( readFile( user, numbers, toSearch ) );
}

int Message::readSuggest( UserPtr user, char *fName )
{
  setFileName( fName );
  return( readSuggest( user ) );
}

int Message::removeLines( char *numbers, char *fName )
{
  setFileName( fName );
  return( removeLines( numbers ) );
}

int Message::searchFile( UserPtr user, char *toSearch, char *fName )
{
  setFileName( fName );
  return( readFile( user, "1-", toSearch ) );
}

boolean Message::suggestMessage( char *msg, char *fName )
{
  setFileName( fName );
  return( suggestMessage( msg ) );
}

