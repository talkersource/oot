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
// Captures the behaviour for all the commands.
//
/////////////////////////////////////////////////////////////////////////

#include "command.h"

    //////                                               //////
   //                                                       //
  // Class Constructors, Destructors, and Initializations  //
 //                                                       //
//////                                               //////

Command::Command( void )
{
  setWizLevel( WIZ_LEVEL );
}

boolean Command::initCommands( void )
{
  // If loadLevels() worked, then return an indicator to if loadCommands()
  // worked.  Otherwise return FALSE.
  //
  return( loadLevels() ? commandParser.loadCommands() : FALSE );
}

    //////                         //////
   //                                 //
  // Registration with other Objects //
 //                                 //
//////                         //////

// In order to issue commands to the talker, the Command class
// needs to know about the talker.  (In theory, you could have
// a slew of talkers running.)
//
boolean Command::registerTalker( TalkerPtr t )
{
  talker = t;
  return( TRUE );
}

boolean Command::registerHandyStr( HandyStrPtr h )
{
  handyStr = h;
  return( TRUE );
}

boolean Command::registerCmdWithParser( CommandPtr c )
{
  commandParser.registerCommand( c );
  return( TRUE );
}

    //////                       //////
   //                               //
  // Handles execution of Commands //
 //                               //
//////                       //////

boolean Command::execMacro( UserPtr user, char *macroName, char *macroParam )
{
  char macroCmd[ BIG_SAY_LENGTH + 1 ],
       *buffPtr;
  int  count = 0,
       maxCount = 0;

  boolean findTalkerMacro = FALSE, // TRUE - look in talker macros
          done            = FALSE;

  maxCount = MACRO_MAX;

  while( !done )
  {
    count = 0;

    do
    {
      strncpy( macroCmd,
               findMacro( user, findTalkerMacro, count ),
               BIG_SAY_LENGTH );

      splitCommand( macroCmd );

      if( strncmp( getCommand(), macroName, strlen( macroName ) ) == 0 )
      {
        // If there is an * within the macro's parameter (getParam()), then
        // substitute it with what the user typed in after the macro
        // (macroParam).
        //
        if( (buffPtr = strchr( getParam(), MACRO_SUB_CHAR )) != NULL )
        {
          *buffPtr = 0;
          strncpy( macroCmd, getParam(), SAY_LENGTH );
          strncat( macroCmd, macroParam, SAY_LENGTH );
          *buffPtr = MACRO_SUB_CHAR;
          buffPtr++;
          strncat( macroCmd, buffPtr, SAY_LENGTH );
        }
        else
        {
          strncpy( macroCmd, getParam(), SAY_LENGTH );
          strncat( macroCmd, macroParam, SAY_LENGTH );
        }

        setRecurse( TRUE );
        issueCommand( user, macroCmd );
        setRecurse( FALSE );
        break;
      }
    }
    while( ++count != maxCount );

    // Done checking the user's macros, now check the talker's
    //
    if( (count == MACRO_MAX) && (findTalkerMacro == FALSE) )
    {
      findTalkerMacro = TRUE;
      maxCount = TALKER_MACRO_MAX;
    }
    else
      done = TRUE;
  }

  return( (count == maxCount) ? FALSE : TRUE );
}

// This'll save a lot of code cut and waste; find a macro either of a user
// or of the talker.
//
char *Command::findMacro( UserPtr user, boolean isTalkerMacro, int index )
{
  return( (isTalkerMacro == TRUE) ? talker->getMacroAt( index ) :
                                    user->getMacroAt( index ) );
}

// Returns FALSE if the command wasn't found.
//
boolean Command::issueCommand( UserPtr user, char *cmd )
{
  Say   toSay;
  TTime aTime;

  char tempCmd[ COMMAND_LENGTH + 1 ],
       cmdOverflow[ BIG_SAY_LENGTH + 1 ],
       sayParam[ BIG_SAY_LENGTH + 1 ];

  int  retVal = 0;

  // The user is logging on ... Check the state of the log on, then do
  // the appropriate action.
  //
  if( user->getLoggingOn() == TRUE )
    return( logonUser( user, cmd ) );

  // Reset the user's idle time
  //
  user->setIdleTime( aTime.getCurrentTime() );

  toSay.setColour( CYAN );
  toSay.setSayType( ISUNIGNORABLE );

  if( user->getReading() == TRUE )
  {
    handyStr->setUserName( user->getName() );

    // Might wish to display tells, says, and such, if the user is
    // ignoring everything (disturbable()) while reading.
    //
    if( (cmd[ 0 ] == 'q') || (cmd[ 0 ] == 'Q') )
    {
      user->clearBufferedSays();
      user->setReading( FALSE );
      toSay.setMessage( handyStr->getString( SUCCESSFUL_COMMAND ) );
      return( user->displaySay( toSay ) );
    }
    else if( user->setReading( user->displaySays() ) )
    {
      toSay.setColour( GREEN );
      toSay.setMessage( handyStr->getString( USER_PRESS_ENTER_Q ) );
      return( user->displaySay( toSay ) );
    }

    toSay.setMessage( handyStr->getString( SUCCESSFUL_COMMAND ) );
    return( user->displaySay( toSay ) );
  }

  // If the user is editing, then give the cmd string to the user's editor.
  //
  if( user->getEditing() == TRUE )
    return( user->updateEdit( cmd ) );

  // No longer away from the keyboard?
  //
  if( user->getAfk() == TRUE )
  {
    user->setAfk( FALSE );
    user->setAfkMessage( "" );
    sayUserType( user, handyStr->getString( USER_NOT_AFK ), ISEMOTE );
  }

  // A Talker means you SAY things ...
  //
  if( cmd[ 0 ] == '\r' )
    return( FALSE );

  // Parse the command out into the command itself, and its parameters.
  // Use "getCommand()" and "getParam()" to obtain the respective strings.
  //
  // This also takes care of adding ".say" and translating ";smiles" and
  // such.
  //
  strncpy( cmdOverflow, cmd, SAY_LENGTH );
  parseCommand( user, cmdOverflow );

  user->setBufferNextSay( TRUE );

  // Even if this failed, the user may have a macro which satisfies the
  // situation.  If the issueCommand returns -2 (hee hee) it means that
  // the command was found, but the user hadn't enough access.
  //
  retVal = commandParser.issueCommand( user, getCommand(), getParam() );

  if( retVal == USER_QUIT )
    return( USER_QUIT );

  user->setReading( user->displaySays() );
  user->setBufferNextSay( FALSE );

  if( user->getReading() == TRUE )
  {
    handyStr->setUserName( user->getName() );
    toSay.setColour( GREEN );
    toSay.setMessage( handyStr->getString( USER_PRESS_ENTER_Q ) );
    return( user->displaySay( toSay ) );
  }
 
  switch( retVal )
  {
    // The command succeeded; no need to go any further
    //
    case TRUE:
      return( TRUE );
      break;
 
    // If the command parser returned FALSE, it is because the command
    // wasn't found--maybe it is a macro!
    //
    case FALSE:
      break;
 
    // The command was there; the user hadn't enough access for it
    //
    case -2:
      return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );
      break;
  }

  // Very naughty to do recursion in your macros!
  //
  if( getRecurse() == TRUE )
  {
    setRecurse( FALSE );
    return( userMsg( user, RECURSIVE_COMMAND, ISUNIGNORABLE, CYAN ) );
  }

  // Want to keep the parameter, but the macro command can go bye-bye
  //
  strcpy( sayParam, getParam() );
  strcpy( tempCmd, getCommand() );
  lowerCase( tempCmd );

  if( execMacro( user, tempCmd, sayParam ) == FALSE )
    return( userMsg( user, INVALID_COMMAND, ISUNIGNORABLE, CYAN ) );

  return( TRUE );
}

    //////         //////
   //                 //
  // Parsing Methods //
 //                 //
//////         //////

// Given a list of user names, this builds a string like:
//
// "thang morn gaz elvis"
//
int Command::buildAliasList( ListPtr userList, char *aliasList )
{
  UserPtr user;
  int     aliasCount = 0;

  strcpy( aliasList, "" );

  while( (user = (UserPtr)userList->at( aliasCount++ )) != NULL )
  {
    strncat( aliasList, user->getName(), BIG_SAY_LENGTH -
                                          strlen( aliasList ) - 10 );
    strcat( aliasList, " " );
  }

  // Get rid of that dang trailing space ...
  //
  aliasList[ strlen( aliasList ) - 1 ] = 0;
  return( aliasCount );
}

void Command::parseCommand( UserPtr user, char *cmd )
{
  char tempCmd[ BIG_SAY_LENGTH + 1 ];
  int  count = 0;

  // If the start character isn't a command character, default to ".say "
  // command.
  //
  if( !isCommandChar( user, cmd[ 0 ] ) )
  {
    sprintf( tempCmd, ".say %s", cmd );
    strncpy( cmd, tempCmd, SAY_LENGTH );
  }
  else
  {
    strncpy( tempCmd, cmd, SAY_LENGTH );
    tempCmd[ SAY_LENGTH ] = 0;

    // See which one-key short-cut was used.
    //
    while( count < ABBR_MAX )
    {
      if( user->getAbbr( count ) == cmd[ 0 ] )
        sprintf( tempCmd, "%s %s", user->getAbbr( cmd[ 0 ] ), &cmd[ 1 ] );

      count++;
    }

    strncpy( cmd, tempCmd, SAY_LENGTH );
    cmd[ SAY_LENGTH ] = 0;
  }

  // Split the command into two parts; accessed by getCommand() and getParam()
  //
  splitCommand( cmd );
}

// Returns the number of users that were found; list contains a list
// of all the users that were found.
//
int Command::parseUserList( UserPtr user, ListPtr list, char *userList,
                            char *theText )
{
  Say     toSay;
  UserPtr aUser     = NULL;
  ListPtr usersList = NULL;    // A list of the users in a room
  RoomPtr aRoom     = NULL;

  char userAliases[ SAY_LENGTH + 1 ],
       *peekBuff = NULL,
       *oldPeek  = NULL;

  int aliasCount = 0,                // The # of aliases found
      userCount  = 0;                // Count through users in aRoom
  boolean done   = FALSE;

  strcpy( userAliases, userList );

  peekBuff = userAliases;
  oldPeek  = peekBuff;

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  while( *peekBuff && !done )
  {
    // Find the next space or comma
    //
    while( *peekBuff && (*peekBuff != ' ') && (*peekBuff != ',' ) )
      peekBuff++;

    if( *peekBuff == ',' )
    {
      *peekBuff = 0;

      // Explicity check for an ! to denote a room ...
      //
      if( oldPeek[ 0 ] == '!' )
      {
        if( (aRoom = (RoomPtr)talker->findRoom( &oldPeek[ 1 ] )) != NULL )
        {
          usersList = (ListPtr)aRoom->getUserList();
          userCount = 0;

          while( (aUser = (UserPtr)usersList->at( userCount++ )) != NULL )
            if( (*list += *aUser) == TRUE )
              aliasCount++;

          if( userCount == 1 )
          {
            handyStr->setRoomName( aRoom->getName() );
            toSay.setMessage( handyStr->getString( EMPTY_ROOM ) );
            user->displaySay( toSay );
          }
        }
        else
        {
          handyStr->setRoomName( &oldPeek[ 1 ] );
          toSay.setMessage( handyStr->getString( INVALID_ROOM ) );
          user->displaySay( toSay );
        }
      }
      else 
      // Check to see if the user name matches a room.  If so, find all
      // the users in that room and add them to the list.  First check to
      // see if there is a user by that name online ...
      //
      if( talker->findUserMatches( oldPeek ) > 0 )
      {
        if( (aUser = isMultipleUser( user, oldPeek )) != NULL )
          if( (*list += *aUser) == TRUE )
            aliasCount++;
      }
      else
      {
         handyStr->setUserName( oldPeek );
         toSay.setMessage( handyStr->getString( INVALID_USER ) );
         user->displaySay( toSay );
      }

      peekBuff++;

      // Find the start of the next alias
      //
      while( *peekBuff && isspace( *peekBuff ) )
        peekBuff++;

      // oldPeek now points to the start of the next alias
      //
      oldPeek = peekBuff;
    }
    else if( *peekBuff == ' ' )
    {
      *peekBuff = 0;

      if( oldPeek[ 0 ] == '!' )
      {
        if( (aRoom = (RoomPtr)talker->findRoom( &oldPeek[ 1 ] )) != NULL )
        {
          usersList = (ListPtr)aRoom->getUserList();
          userCount = 0;

          while( (aUser = (UserPtr)usersList->at( userCount++ )) != NULL )
            if( (*list += *aUser) == TRUE )
              aliasCount++;

          if( userCount == 1 )
          {
            handyStr->setRoomName( aRoom->getName() );
            toSay.setMessage( handyStr->getString( EMPTY_ROOM ) );
            user->displaySay( toSay );
          }
        }
        else
        {
          handyStr->setRoomName( &oldPeek[ 1 ] );
          toSay.setMessage( handyStr->getString( INVALID_ROOM ) );
          user->displaySay( toSay );
        }
      }
      else if( talker->findUserMatches( oldPeek ) > 0 )
      {
        if( (aUser = isMultipleUser( user, oldPeek )) != NULL )
          if( (*list += *aUser) == TRUE )
            aliasCount++;
      }
      else
      {
         handyStr->setUserName( oldPeek );
         toSay.setMessage( handyStr->getString( INVALID_USER ) );
         user->displaySay( toSay );
      }

      peekBuff++;
      done = TRUE;
    }
  }

  // peekBuff is pointing to the start of what the user wants to type
  // to the users that were specified.  Return this information to the
  // calling method.
  //
  strcpy( theText, peekBuff );
  return( aliasCount );
}

// Split the "command" into two parts:
//
// 1) The word before the first space
// 2) The word after the first space
//
void Command::splitCommand( char *cmd )
{
  int  count = 0;
  char tempCmd[ COMMAND_LENGTH + 1 ],
       tempParam[ BIG_SAY_LENGTH + 1 ];

  // The first whitespace denotes the end of the command
  //
  while( !isspace( cmd[ count ] ) && (cmd[ count ] != 0) )
    count++;

  // Split the command from the parameter
  //
  strncpy( tempCmd, cmd, count );

  // Terminate the command string
  //
  tempCmd[ count ] = 0;

  // Weed out whitespace after the command part (if any)
  //
  if( (count > 0) && (cmd[ count ] != 0) )
  {
    while( isspace( cmd[ ++count ] ) && (cmd[ count ] != 0) )
      ;

    strncpy( tempParam, &cmd[ count ], BIG_SAY_LENGTH );
  }
  else
    strcpy( tempParam, "" );

  // The word before the first space becomes the "command" part
  //
  setCommand( tempCmd );

  // Everything else becomes the "parameter" part
  //
  setParam( tempParam );
}

    //////               //////
   //                       //
  // Setter/Getter Methods //
 //                       //
//////               //////

boolean Command::setCommand( char *cmd )
{
  strncpy( userCmd, cmd, COMMAND_LENGTH );
  userCmd[ COMMAND_LENGTH ] = 0;
  return( TRUE );
}

char *Command::getCommand( void )
{
  return( userCmd );
}

boolean Command::setHighLevel( int newHighLevel )
{
  highLevel = newHighLevel;
  return( TRUE );
}

int Command::getHighLevel( void )
{
  return( highLevel );
}

boolean Command::setParam( char *param )
{
  strncpy( userParam, param, BIG_SAY_LENGTH );
  userParam[ BIG_SAY_LENGTH ] = 0;
  return( TRUE );
}

char *Command::getParam( void )
{
  return( userParam );
}

boolean Command::setRecurse( boolean toSet )
{
  recurse = toSet;
  return( TRUE );
}

boolean Command::getRecurse( void )
{
  return( recurse );
}

// Returns NULL if no user could be found, according to "online".  If
// "online" is TRUE, then UserPtr will point to a user name which exactly
// matches "alias" whom is online; "online" will be set to TRUE.  If
// "online" is FALSE, then UserPtr will point to a user name which exactly
// matches "alias" whom is either online or offline; "online" will be set
// to "FALSE" if the user was offline (and hence the memory should be
// deleted).
//
UserPtr Command::getUser( char *alias, boolean *online )
{
  char    tempBuff[ SAY_LENGTH + 1 ];
  UserPtr tempUser = NULL;

  strncpy( tempBuff, alias, COMMON_NAME_LEN );
  tempBuff[ COMMON_NAME_LEN ] = 0;
  lowerCase( tempBuff );

  if( (tempUser = talker->findUserExact( tempBuff )) == NULL )
    tempUser = talker->findUser( tempBuff );

  // Only search online for the exactly matching user name
  //
  if( (*online == TRUE) || (tempUser != NULL) )
  {
    // Needed to satisfy the condition of if seaching offline was requested
    // (i.e., the user was online, even though the search could have gone
    // offline).  This informs the calling method that the memory need not
    // be cleared.
    //
    *online = TRUE;
    return( tempUser );
  }

  tempUser = new User( tempBuff );

  // Load up their site from disk ...
  //
  tempUser->setSaveUser( TRUE );
  *online = FALSE;

  if( tempUser->load() == TRUE )
  {
    // Don't save any data ...
    //
    tempUser->setSaveUser( FALSE );
    return( tempUser );
  }

  tempUser->setSaveUser( FALSE );
  delete( tempUser );
  return( NULL );
}

// For smailing, .ex'ing, etc., where you want to first check for exactly
// online and then exactly offline, and then not exactly online ... "online"
// becomes TRUE if the user is online at the time, "FALSE" if not (this let's
// the calling method know to clear up the memory that this method will
// allocate).
//
// This has the opposite functionality of getUser(...)
//
UserPtr Command::getExactUser( char *userAlias, boolean *online )
{
  char    tempBuff[ SAY_LENGTH + 1 ];
  UserPtr tempUser = NULL;

  strncpy( tempBuff, userAlias, COMMON_NAME_LEN );
  tempBuff[ COMMON_NAME_LEN ] = 0;
  lowerCase( tempBuff );

  // See if that exact user is online ...
  //
  if( (tempUser = talker->findUserExact( tempBuff )) != NULL )
  {
    *online = TRUE;
    return( tempUser );
  }

  // See if that user is offline ...
  //
  tempUser = new User( tempBuff );
  tempUser->setSaveUser( TRUE );

  *online = FALSE;

  if( tempUser->load() == TRUE )
  {
    tempUser->setSaveUser( FALSE );
    return( tempUser );
  }

  tempUser->setSaveUser( FALSE );
  delete( tempUser );

  if( (tempUser = talker->findUser( tempBuff )) == NULL )
    return( NULL );

  *online = TRUE;
  return( tempUser );
}

boolean Command::setWizLevel( iLevel newLevel )
{
  wizLevel = newLevel;
  return( TRUE );
}

iLevel Command::getWizLevel( void )
{
  return( wizLevel );
}

    //////       //////
   //               //
  // Query Methods //
 //               //
//////       //////

// Return TRUE if the character passed denotes an abbreviated command
//
boolean Command::isCommandChar( UserPtr user, char ch )
{
  return( ((user->getAbbr( ch ) == NULL) && (ch != '.')) ? FALSE : TRUE );
}

boolean Command::isInvalidAlias( char *alias )
{
  // If the alias isn't long enough, then it is invalid
  //
  if( strlen( alias ) < MIN_ALIAS_LENGTH )
    return( TRUE );

  // Check for any invalid characters within an alias.  strpbrk scans
  // alias for the first occurance of INVALID_ALIAS_CHARS, and returns
  // NULL if none were found (if none were found then it is a VALID alias)
  //
  return( (strpbrk( alias, INVALID_ALIAS_CHARS ) == NULL) ? FALSE : TRUE );
}

boolean Command::isMultipleUser( char *alias )
{
  return( (talker->findUserMatches( alias ) > 1) ? TRUE : FALSE );
}

// Returns NULL if the user isn't original (sends a message to user
// indicating what the problem was); otherwise a pointer to the user
// matching "alias"
//
UserPtr Command::isMultipleUser( UserPtr user, char *alias )
{
  // How many matches to the user alias (in *msg) were there?
  // (It does lowercase comparisons!)
  //
  UserPtr tempUser = NULL;
  int     matches  = 0;
  char    tempBuff[ SAY_LENGTH + 1 ];
  Say     toSay;

  strncpy( tempBuff, alias, SAY_LENGTH );
  tempBuff[ SAY_LENGTH ] = 0;
  lowerCase( tempBuff );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  // User is not online
  //
  if( (matches - talker->findUserMatches( tempBuff )) == NO_MATCHES )
  {
    handyStr->setUserName( alias );
    toSay.setMessage( handyStr->getString( INVALID_USER ) );
    user->sendString( toSay );
    return( NULL );
  }

  if( (tempUser = talker->findUserExact( tempBuff )) != NULL )
    return( tempUser );

  // Multiple matches of the alias
  //
  if( matches > 1 )
  {
    toSay.setMessage( handyStr->getString( MULTIPLE_USERS ) );
    user->sendString( toSay );
    return( NULL );
  }

  return( talker->findUser( tempBuff ) );
}

    //////       //////
   //               //
  // Level Methods //
 //               //
//////       //////

// For now, don't have files with their respective levels
//
boolean Command::addLevel( char *newLevelName )
{
  strncpy( levelNames[ getHighLevel() ], newLevelName, LEVEL_NAME_LENGTH );
  levelNames[ getHighLevel() ][ LEVEL_NAME_LENGTH ] = 0;

  setHighLevel( getHighLevel() + 1 );
  return( TRUE );
}

char *Command::getLevelName( int levelToGet )
{
  if( levelToGet > getHighLevel() )
    return( NULL );

  return( levelNames[ levelToGet ] );
}

boolean Command::loadLevels( void )
{
  FILE *fp;
  char tempBuff[ SAY_LENGTH + 1 ];

  sprintf( tempBuff, "%s%s", DATA_DIR, LEVEL_FILENAME );

  // Very bad to have this file missing ...
  //
  if( (fp = fopen( tempBuff, "r" )) == NULL )
    return( FALSE );

  setHighLevel( 0 );

  while( (fgets( tempBuff, SAY_LENGTH, fp ) != NULL) &&
         (getHighLevel() < MAX_LEVEL) )
  {
    if( strlen( tempBuff ) > 2 )
    {
      tempBuff[ strlen( tempBuff ) - 1 ] = 0;
      addLevel( tempBuff );
    }
  }

  fclose( fp );

  // Compensate for the level being 1 rank too high
  //
  setHighLevel( getHighLevel() - 1 );

  return( TRUE );
}

    //////        //////
   //                //
  // Helper Methods //
 //                //
//////        //////

// Forceably move a user from one room into another ...
//
boolean Command::moveUser( UserPtr userToMove, int index )
{
  return( moveUser( userToMove,
                    userToMove->getRoom(),
                    talker->findRoom( index ) ) );
}

// Move a user from a room to a room.  Minimal error checking performed;
// do more before calling this method.
//
boolean Command::moveUser( UserPtr userToMove,
                           RoomPtr roomFrom, RoomPtr roomTo )
{
  // Remove the user from the old room; ensure that it becomes unlocked
  // if nobody is there, or that if one person is there that they don't have
  // lock access.
  //
  if( roomFrom != NULL )
    roomFrom->removeUser( userToMove );

  // Add the user to the new room
  //
  roomTo->addUser( userToMove );

  userToMove->setRoom( roomTo );

  return( lookUser( userToMove, roomTo->getName() ) );
}

// Move a user to a room given the alias to move and the room to move to.
//
// This is where security checks go:
//   1) "user" has enough access to move "userToMove"
//   2) "userToMove" is allowing self to be moved!
//   3) Room's private status
//
boolean Command::moveUser( UserPtr user, char *userAlias, char *roomName )
{
  Say     toSay;
  UserPtr userToMove = NULL;
  RoomPtr roomTo     = NULL,
          roomFrom   = NULL;
  char    tempBuff[ SAY_LENGTH + 1 ],
          tempRoom[ COMMON_NAME_LEN + 1 ];

  if( strcmp( userAlias, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  if( strcmp( roomName, "" ) == 0 )
    return( userMsg( user, NEED_ROOM_NAME, ISFEEDBACK, CYAN ) );

  if( (userToMove = isMultipleUser( user, userAlias )) == NULL )
    return( FALSE );

  roomFrom = userToMove->getRoom();

  toSay.setColour( CYAN );
  toSay.setSayType( ISUNIGNORABLE );

  handyStr->setCrewName( user->getName() );
  handyStr->setUserName( userToMove->getName() );

  strncpy( tempRoom, roomName, COMMON_NAME_LEN );
  tempRoom[ COMMON_NAME_LEN ] = 0;
  lowerCase( tempRoom );

  if( (roomTo = talker->findRoom( tempRoom )) == NULL )
  {
    handyStr->setRoomName( roomName );
    toSay.setMessage( handyStr->getString( INVALID_ROOM ) );
    user->sendString( toSay );
    return( FALSE );
  }

  handyStr->setRoomName( roomTo->getName() );

  if( roomFrom == roomTo )
  {
    toSay.setMessage( handyStr->getString( USER_IN_ROOM ) );
    user->sendString( toSay );
    return( FALSE );
  }

  // Only can move users who have less access than you ... OR ...
  // If the userToMove is less than the room, and the user that moved
  // her is less than the room's level, indicate an error
  //
  if( (userToMove->getLevel() > user->getLevel()) ||
        ((userToMove->getLevel() < roomTo->getLevel()) &&
         (user->getLevel() < roomTo->getLevel())) )
    return( userMsg( user, NOT_ENOUGH_ACCESS, ISFEEDBACK, CYAN ) );

  // If the room is private indicate such.
  //
  if( (roomTo->getLocked() == TRUE) &&
      (userToMove->isInvited( roomTo ) == FALSE) )
  {
    toSay.setMessage( handyStr->getString( ROOM_PRIVATE ) );
    user->sendString( toSay );
    return( FALSE );
  }

  // If the user is logging in, don't allow moving her
  //
  if( userToMove->getLoggingOn() == TRUE )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  if( userToMove->getLevel() > 2 )
    strcpy( tempBuff, handyStr->getString( CREW_GRABBED ) );
  else
    strcpy( tempBuff, handyStr->getString( USER_GRABBED ) );

  // Tell everybody that the user is leaving
  //
  sayUserType( userToMove, tempBuff, ISEMOTE, FALSE );
  moveUser( userToMove, roomFrom, roomTo );
  handyStr->setRoomName( roomFrom->getName() );

  if( userToMove->getLevel() > 2 )
    strcpy( tempBuff, handyStr->getString( ENTRANCE_CREW ) );
  else
    strcpy( tempBuff, handyStr->getString( ENTRANCE_USER ) );

  // Tell everybody that the user has arrived
  //
  return( sayUserType( userToMove, tempBuff, ISEMOTE ) );
}

// Called by things like "reviewBufferUser" and "lookUser" to find the
// room, or NULL if the room was invalid.
//
RoomPtr Command::peekRoom( UserPtr user, char *roomName )
{
  RoomPtr room = NULL;
  Say     toSay;

  if( strcmp( roomName, "" ) == 0 )
    room = user->getRoom();
  else
    room = talker->findRoom( roomName );

  handyStr->setUserName( user->getName() );
  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  if( room == NULL )
  {
    handyStr->setRoomName( roomName );
    toSay.setMessage( handyStr->getString( INVALID_ROOM ) );
    user->sendString( toSay );
    return( NULL );
  }

  handyStr->setRoomName( room->getName() );

  if( room != user->getRoom() )
  {
    if( room->getLocked() == TRUE )
    {
      toSay.setMessage( handyStr->getString( ROOM_PRIVATE ) );
      user->sendString( toSay );
      return( NULL );
    }

    if( (user->getLevel() < getWizLevel()) ||
        (user->getLevel() < room->getReviewRank()) )
    {
      userMsg( user, NOT_ENOUGH_ACCESS, ISUNIGNORABLE, CYAN );
      return( NULL );
    }
  }

  return( room );
}

// minChange is the minimum level required to have that state on
//
boolean Command::setUserOption( UserPtr user, char *optionStr, uState state,
                                int stateOn, int stateOff, iLevel minOn )
{
  // If the users's level is less than minOn, then indicate she hasn't enough
  // access to set the given state (that's if the command name is the same).
  //
  if( strncmp( optionStr, getCommand(), strlen( getCommand() ) ) == 0 )
    if( user->getLevel() < minOn )
    {
      user->setState( state, FALSE );
      return( userMsg( user, CANNOT_CHANGE_STATE, ISUNIGNORABLE, CYAN ) );
    }

  return( setUserOption( user, optionStr, state, stateOn, stateOff ) );
}

boolean Command::setUserOption( UserPtr user, char *optionStr, uState state,
                                int stateOn, int stateOff )
{
  Say toSay;

  if( strncmp( optionStr, getCommand(), strlen( getCommand() ) ) == 0 )
  {
    toSay.setColour( CYAN );
    toSay.setSayType( ISFEEDBACK );

    if( strcmp( "off", getParam() ) == 0 )
      user->setState( state, FALSE );
    else
    if( strcmp( "on", getParam() ) == 0 )
      user->setState( state, TRUE );
    else
      user->setState( state, !user->getState( state ) );

    if( user->getState( state ) == TRUE )
      toSay.setMessage( handyStr->getString( stateOn ) );
    else
      toSay.setMessage( handyStr->getString( stateOff ) );

    user->sendString( toSay );

    return( TRUE );
  }

  return( FALSE );
}

// Write a message to all the users in the room (ISNORMAL, ISEMOTE, etc.)
//
boolean Command::sayUserType( UserPtr user, char *msg, sayValue type )
{
  return( sayUserType( user, msg, type, TRUE ) );
}

boolean Command::sayUserType( UserPtr user, char *msg, sayValue type,
                              boolean toUser )
{
  Say toSay;

  toSay.setSayType( type );
  toSay.setInvisible( user->getState( INVISIBLE ) );
  toSay.setUserFrom( user->getName() );
  toSay.setMessage( msg );

  return( sayToRoom( user, toSay, toUser ) );
}

boolean Command::sayWizType( UserPtr user, char *msg, sayValue type )
{
  RoomPtr room       = NULL;
  ListPtr userList   = NULL,
          roomList   = NULL;
  UserPtr userInRoom = NULL;
  Say     toSay;

  int roomCount = 0,
      userCount = 0;

  if( user->getState( JAILED ) == TRUE )
    return( userMsg( user, USER_CANNOT_CREWCAST, ISUNIGNORABLE, CYAN ) );

  // Show the last few wiz says
  //
  if( strcmp( msg, "" ) == 0 )
  {
    ListPtr castList;
    SayPtr  aSay  = NULL;
    int     count = MAX_CREWCASTS;
    boolean crewOn;

    toSay.setMessage( handyStr->getString( CREWCAST_BEGIN_REVIEW ) );
    toSay.setColour( CYAN + BOLD );
    toSay.setSayType( ISUNIGNORABLE );
    user->sendString( toSay );

    castList = talker->getCrewcastList();

    // Have to do this backwards ...
    //
    while( (count >= 0) &&
           (aSay = (SayPtr)castList->at( count-- )) == NULL )
      ;

    toSay.setColour( CYAN );

    crewOn = user->getState( CREWCASTS );
    user->setState( CREWCASTS, TRUE );

    while( aSay != NULL )
    {
      user->sendString( *aSay );
      aSay = (SayPtr)castList->at( count-- );
    }

    user->setState( CREWCASTS, crewOn );

    toSay.setMessage( handyStr->getString( CREWCAST_END_REVIEW ) );
    toSay.setColour( CYAN + BOLD );
    return( user->sendString( toSay ) );
  }

  toSay.setSayType( type );
  toSay.setUserFrom( user->getName() );
  toSay.setMessage( msg );

  roomList = talker->getRoomList();

  while( (room = (RoomPtr)roomList->at( roomCount++ )) != NULL )
  {
    userList = room->getUserList();

    userCount = 0;

    while( (userInRoom = (UserPtr)userList->at( userCount++ )) != NULL )
      if( userInRoom->getLevel() > 2 )
        userInRoom->sendString( toSay );
  }

  talker->addToCrewcastList( toSay );

  return( TRUE );
}

// Depending on the sayValue's type, this method performs a shout or a
// shouted emote (semote).
//
boolean Command::shoutUserType( UserPtr user, char *msg, sayValue type )
{
  RoomPtr room       = NULL;
  ListPtr userList   = NULL,
          roomList   = NULL;
  UserPtr userInRoom = NULL;
  Say     toSay;

  int roomCount = 0,
      userCount = 0;

  if( user->getState( JAILED ) == TRUE )
    return( userMsg( user, USER_CANNOT_SHOUT, ISUNIGNORABLE, CYAN ) );

  // Show the last shouts
  //
  if( strcmp( msg, "" ) == 0 )
  {
    ListPtr shoutList;
    SayPtr  aSay  = NULL;
    int     count = MAX_SHOUTS;
    boolean shoutsOn;

    toSay.setMessage( handyStr->getString( SHOUTS_BEGIN_REVIEW ) );
    toSay.setColour( CYAN + BOLD );
    toSay.setSayType( ISUNIGNORABLE );
    user->sendString( toSay );

    shoutList = talker->getShoutList();

    // Have to do this backwards ...
    //
    while( (count >= 0) &&
           (aSay = (SayPtr)shoutList->at( count-- )) == NULL )
      ;

    toSay.setColour( CYAN );

    // Allow reviewing of shouts if you are ignoring them
    //
    shoutsOn = user->getState( SHOUTS );
    user->setState( SHOUTS, TRUE );

    while( aSay != NULL )
    {
      user->sendString( *aSay );
      aSay = (SayPtr)shoutList->at( count-- );
    }

    user->setState( SHOUTS, shoutsOn );

    toSay.setMessage( handyStr->getString( SHOUTS_END_REVIEW ) );
    toSay.setColour( CYAN + BOLD );
    return( user->sendString( toSay ) );
  }

  // Make sure the user isn't muzzled.
  //
  if( user->getState( MUZZLED ) == TRUE )
    return( userMsg( user, USER_IS_MUZZLED, ISUNIGNORABLE, CYAN ) );

  toSay.setSayType( type );
  toSay.setInvisible( user->getState( INVISIBLE ) );
  toSay.setUserFrom( user->getName() );
  toSay.setMessage( msg );

  roomList = talker->getRoomList();

  while( (room = (RoomPtr)roomList->at( roomCount++ )) != NULL )
  {
    userList  = room->getUserList();
    userCount = 0;

    while( (userInRoom = (UserPtr)userList->at( userCount++ )) != NULL )
      userInRoom->sendString( toSay );
  }

  return( talker->addToShoutList( toSay ) );
}

// Depending on the sayValue's type, this method performs a tell or a
// private emote (pemote).
//
boolean Command::tellUserType( UserPtr user, char *msg, sayValue type )
{
  UserPtr userTo = NULL;
  List    usersToTell;
  Say     toSay;

  char tempBuff[ BIG_SAY_LENGTH + 1 ],
       aliasList[ BIG_SAY_LENGTH + 1 ],
       afterUsers[ SAY_LENGTH + 1 ];

  int aliasCount = 0,               // The # of aliases .tell'd
      succTell   = 0;               // The # of successful tells

  if( user->getState( JAILED ) == TRUE )
    return( userMsg( user, USER_CANNOT_TELL, ISUNIGNORABLE, CYAN ) );

  // If nobody was specified, then display the user's tells back to her
  //
  if( strcmp( msg, "" ) == 0 )
  {
    ListPtr tellList;
    SayPtr  aSay  = NULL;
    int     count = 1;

    tellList = user->getTellList();
    userMsg( user, BEGIN_REVIEW_TELLS, ISUNIGNORABLE, CYAN + BOLD );

    while( (aSay = (SayPtr)tellList->at( tellList->howMany() - count )) != NULL )
    {
      user->sendString( *aSay );
      count++;
    }

    userMsg( user, END_REVIEW_TELLS, ISUNIGNORABLE, CYAN + BOLD );
    return( TRUE );
  }

  // afterUsers will point to the text which comes after the user list,
  // or be pointing to ASCII-Z
  //
  if( parseUserList( user, &usersToTell, getParam(), afterUsers ) == 0 )
    return( FALSE );
    //return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  // Build up a list of user names from the usersToTell list
  //
  //  [User User User] Thangalin tells you: blah
  //
  aliasCount = buildAliasList( &usersToTell, tempBuff );

  sprintf( aliasList, "[^%s^]", tempBuff );

  if( strcmp( afterUsers, "" ) == 0 )
    return( userMsg( user, NEED_TELL, ISFEEDBACK, CYAN ) );

  // What does "user" wish to tell the users?
  //
  toSay.setMessage( afterUsers );
  toSay.setSayType( type );
  toSay.setInvisible( user->getState( INVISIBLE ) );
  toSay.setUserFrom( user->getName() );

  // A value of > 2 means more than 1 user was .telled
  //
  if( aliasCount > 2 )
  {
    toSay.setUserTo( aliasList );

    toSay.setSayType( (toSay.getSayType() == ISTELL) ?
                      ISMULTITELL : ISMULTIPEMOTE );

/*
    if( toSay.getSayType() == ISTELL )
      toSay.setSayType( ISMULTITELL );
    else
      toSay.setSayType( ISMULTIPEMOTE );
*/

    type = toSay.getSayType();
  }

  aliasCount = 0;

  toSay.setFromBot( user->getState( ISBOT ) );

  while( (userTo = (UserPtr)usersToTell.at( aliasCount++ )) != NULL )
  {
    // Is userTo listening?
    //
    if( userTo->getState( TELLS ) == FALSE )
    {
      invalidName( user, userTo->getName(), USER_IGNORE_TELL );
      continue;
    }
    else if( userTo->getEditing() == TRUE )
    {
      invalidName( user, userTo->getName(), USER_EDITING );
      continue;
    }

    if( userTo->sendString( toSay ) == FALSE )
    {
      invalidName( user, userTo->getName(), USER_UNDISTURBABLE );
      continue;
    }

    succTell++;
    userTo->addToTellList( toSay );

    // Is the userTo away from the keyboard?
    //
    if( userTo->getAfk() == TRUE )
    {
      sprintf( tempBuff, "%s %s",
                         userTo->getName(),
                         userTo->getAfkMessage() );

      userMsgStr( user, tempBuff, ISFEEDBACK, CYAN + BOLD );
    }
  }

  if( succTell == 0 )
    return( FALSE );

  // Why is this 2?  ... 'Cause 2 means ONE person was .telled ...
  //
  if( aliasCount == 2 )
  {
    userTo = (UserPtr)usersToTell.at( 0 );

    handyStr->setUserName( userTo->getName() );

    if( type == ISTELL )
      sprintf( tempBuff, "%s%s",
                         handyStr->getString( SUCCESSFUL_TELL ),
                         toSay.getMessage() );
    else
    if( type == ISPEMOTE )
      sprintf( tempBuff, "%s%s %s",
                         handyStr->getString( SUCCESSFUL_EMOTE ),
                         user->getName(),
                         toSay.getMessage() );
  }
  else
  {
    if( type == ISMULTITELL )
      sprintf( tempBuff, "%s%s: %s",
                         handyStr->getString( MULTI_SUCC_TELL ),
                         aliasList,
                         toSay.getMessage() );
    else
      sprintf( tempBuff, "%s%s: %s %s",
                         handyStr->getString( MULTI_SUCC_EMOTE ),
                         aliasList,
                         user->getName(),
                         toSay.getMessage() );
  }

  toSay.setSayType( ISFEEDBACK );
  toSay.setMessage( tempBuff );
  toSay.setColour( CYAN );
  user->sendString( toSay );
  return( user->addToTellList( toSay ) );
}

    //////                           //////
   //                                   //
  // Common messages sent to/by a user //
 //                                   //
//////                           //////

// Send a say to all users
//
boolean Command::broadcastSay( Say toSay )
{
  ListPtr roomList   = NULL,
          userList   = NULL;
  RoomPtr room       = NULL;
  UserPtr userInRoom = NULL;

  int     roomCount = 0,
          userCount = 0;

  roomList = talker->getRoomList();

  while( (room = (RoomPtr)roomList->at( roomCount++ )) != NULL )
  {
    userList  = room->getUserList();
    userCount = 0;

    while( (userInRoom = (UserPtr)userList->at( userCount++ )) != NULL )
      userInRoom->sendString( toSay );
  }

  return( TRUE );
}

boolean Command::examineUser( UserPtr user, UserPtr toEx, boolean showLineNum )
{
  Say  toSay;
  int  speed = toEx->profile.getNumLines(),
       count = 0;
  char tempBuff[ SAY_LENGTH + 1 ];

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  // Display toEx's profile
  //
  while( count <= speed )
  {
    if( showLineNum )
    {
      sprintf( tempBuff, "[^5%2d^] %s", count + 1, toEx->profile.getLine( count ) );
      toSay.setMessage( tempBuff );
      count++;
    }
    else
      toSay.setMessage( toEx->profile.getLine( count++ ) );

    user->sendString( toSay );
  }

  return( TRUE );
}

// Always return FALSE; this function writes string followed by an error
// message.  Comes in quite handy.
//
boolean Command::invalidName( UserPtr user, char *name, stringConst msgNum )
{
  handyStr->setUserName( name );
  userMsgStr( user, handyStr->getString( msgNum ), ISUNIGNORABLE, CYAN );
  return( FALSE );
}

boolean Command::sayToRoom( UserPtr user, Say toSay )
{
  return( sayToRoom( user, toSay, TRUE ) );
}

// Write a generic Say to all the users in the room.  If toUser is TRUE,
// then display it to the user herself, as well.
//
boolean Command::sayToRoom( UserPtr user, Say toSay, boolean toUser )
{
  ListPtr userList   = user->getRoom()->getUserList();
  UserPtr userInRoom = NULL;
  int     userCount  = 0;

  user->getRoom()->addToReviewBuffer( toSay );

  while( (userInRoom = (UserPtr)userList->at( userCount++ )) != NULL )
    if( (userInRoom != user) || toUser )
      userInRoom->sendString( toSay );

  return( TRUE );
}

boolean Command::showAdjoinedRooms( UserPtr user, RoomPtr room )
{
  int     count     = 0,
          roomCount = 0,
          extra     = 0;    // For counting the HIGHLIGHT_CHAR's (^)

  ListPtr roomList = NULL;           // List of attached rooms
  RoomPtr tempRoom = NULL;
  Say     toSay;

  boolean displayed = FALSE;

  char    tempBuff[ SAY_LENGTH + 1 ];

  roomList = (ListPtr)room->getAttachedRooms();

  // Indent (the ^ should really be an sprintf using HIGHLIGHT_CHAR) ...
  //
  strcpy( tempBuff, "  ^" );
  extra = 1;

  toSay.setColour( CYAN );

  // For all the rooms in the room ...
  //
  while( (tempRoom = (RoomPtr)roomList->at( count++ )) != NULL )
  {
    if( strlen( tempBuff ) > (user->getCols() - COMMON_NAME_LEN + extra) )
    {
      if( displayed == FALSE )
      {
        displayed = TRUE;
        toSay.setSayType( ISUNIGNORABLE );
        toSay.setMessage( handyStr->getString( ADJOINING_ROOMS ) );
        user->displaySay( toSay );
        toSay.setSayType( ISROOMLIST );
      }

      tempBuff[ strlen( tempBuff ) - 3 ] = 0;
      toSay.setMessage( tempBuff );
      user->displaySay( toSay );

      // Indent
      //
      strcpy( tempBuff, "  ^" );
      extra = 1;
    }

    if( tempRoom->getViewRank() <= user->getLevel() )
    {
      roomCount++;
      strcat( tempBuff, tempRoom->getName() );
      strcat( tempBuff, "^, ^" );
      extra += 2;
    }
  }

  if( roomCount == 0 )
    return( roomCount );
  else
    // Remove the last three characters from the end of tempBuff (the comma,
    // space, and ^)
    //
    tempBuff[ strlen( tempBuff ) - 3 ] = 0;

  if( displayed == FALSE )
  {
    toSay.setSayType( ISUNIGNORABLE );
    toSay.setMessage( handyStr->getString( ADJOINING_ROOMS ) );
    user->displaySay( toSay );
    toSay.setSayType( ISROOMLIST );
  }

  toSay.setMessage( tempBuff );
  user->displaySay( toSay );
  toSay.setMessage( "" );
  toSay.setSayType( ISUNIGNORABLE );
  user->displaySay( toSay );

  return( roomCount );
}

// Returns TRUE only if the user has NEW smail
//
boolean Command::showSmail( UserPtr user )
{
  char     tempBuff[ SAY_LENGTH + 1 ];
  FileName fileName;
  Say      toSay;

  if( user->getState( NEWSMAIL ) == TRUE )
    return( userMsg( user, USER_NEW_SMAIL, ISNEWSMAIL, GREEN ) );
  else
  {
    FILE *fp = NULL;

    fileName.setFileName( USER_DIR, user->getName(), SMAIL_EXTENSION );

    if( (fp = fopen( fileName.getFileName(), "r" )) != NULL )
    {
      // The file can be opened, but can some text be read?
      //
      if( fgets( tempBuff, SAY_LENGTH, fp ) != NULL )
        userMsg( user, USER_HAS_SMAIL, ISUNIGNORABLE, CYAN );

      fclose( fp );
    }
  }

  return( FALSE );
}

int Command::showUsersInRoom( UserPtr user, RoomPtr room )
{
  ListPtr userList   = NULL;           // List of users in the room
  UserPtr userInRoom = NULL;           // One of the users in the room
  Say     toSay;

  int  count     = 0,                  // Counts through users in the room
       userCount = 0;                  // Number of users in the room

  char tempBuff[ SAY_LENGTH + 1 ];

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setMessage( handyStr->getString( USERS_IN_ROOM ) );
  toSay.setColour( CYAN + BOLD );
  user->displaySay( toSay );

  // Are there any users in the room?
  //
  userList = room->getUserList();

  toSay.setColour( CYAN );
  toSay.setSayType( ISUNIGNORABLE );

  // Indent
  //
  strcpy( tempBuff, "  " );

  toSay.setSayType( ISUSERLIST );

  // For all the users in the room ...
  //
  while( (userInRoom = (UserPtr)userList->at( count++ )) != NULL )
  {
    // Don't overwrite buffer with names; wrap names near the end of the 
    // screen.
    //
    if( strlen( tempBuff ) > (user->getCols() - COMMON_NAME_LEN) )
    {
      toSay.setMessage( tempBuff );
      user->displaySay( toSay );

      // Indent
      //
      strcpy( tempBuff, "  " );
    }

    // Display everybody EXCEPT user, and invisible users
    //
    if( userInRoom != user )
    {
      if( userInRoom->getState( INVISIBLE ) == FALSE )
      {
        strcat( tempBuff, userInRoom->getName() );
        strcat( tempBuff, ", " );

        userCount++;
      }
      else
      if( user->getLevel() > 2 )
      {
        strcat( tempBuff, userInRoom->getName() );
        strcat( tempBuff, " *, " );

        userCount++;
      }
    }
  }

  // Were there any other users in the room?
  //
  if( userCount == 0 )
    strcat( tempBuff, handyStr->getString( NO_OTHER_USERS ) );
  else
    // Remove the last two characters from the end of tempBuff (the comma
    // and space)
    //
    tempBuff[ strlen( tempBuff ) - 2 ] = 0;

  toSay.setMessage( tempBuff );
  user->displaySay( toSay );

  return( userCount );
}

// Returns TRUE if the user can wipe messages off the given room's message
// board.  Indicates an error message otherwise, and returns FALSE.
//
boolean Command::userInteractRoom( UserPtr user, RoomPtr room,
                                   iLevel l, stringConst usrNum )
{
  Say toSay;

  if( user->getLevel() >= l )
    return( TRUE );

  handyStr->setRoomName( room->getName() );
  handyStr->setUserName( user->getName() );
  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );
  toSay.setMessage( handyStr->getString( usrNum ) );
  user->sendString( toSay );
  return( FALSE );
}

// Display a message (msgNum) to "user" of a specific type (ISUNIGNORABLE,
// ISFEEDBACK, etc.).
//
// Returns FALSE because it's typically used for error messages.
//
boolean Command::userMsg( UserPtr user, stringConst msgNum,
                          sayValue type, iColour colour )
{
  handyStr->setUserName( user->getName() );
  userMsgStr( user, handyStr->getString( msgNum ), type, colour );
  return( FALSE );
}

boolean Command::userMsgStr( UserPtr user, char *str,
                             sayValue type, iColour colour )
{
  Say toSay;

  toSay.setColour( colour );
  toSay.setSayType( type );
  toSay.setMessage( str );
  user->sendString( toSay );

  return( TRUE );
}

    //////             //////
   //                     //
  // Command Definitions //
 //                     //
//////             //////

// User requested to change one of her abbreviations, or list them
//
boolean Command::abbrUser( UserPtr user, char *cmd )
{
  int  count = 0;
  Say  toSay;
  char tempBuff[ SAY_LENGTH + 1 ];

  toSay.setSayType( ISUNIGNORABLE );

  if( strlen( cmd ) == 0 )
  {
    toSay.setColour( CYAN + BOLD );
    toSay.setMessage( handyStr->getString( USER_VIEW_ABBR ) );
    user->sendString( toSay );

    toSay.setColour( CYAN );

    for( count = 0; count < ABBR_MAX; count++ )
    {
      if( user->getAbbr( count ) == 0 )
        continue;

      sprintf( tempBuff, "%c %s", user->getAbbr( count ),
                                  user->getAbbr( user->getAbbr( count ) ) );
      toSay.setMessage( tempBuff );
      user->sendString( toSay );
    }

    toSay.setMessage( "" );
    return( user->sendString( toSay ) );
  }

  splitCommand( cmd );

  // Ensure that the first character isn't a "." -- for the users' safety
  //
  if( (getCommand()[ 0 ] == '.') || isalnum( getCommand()[ 0 ] ) )
    return( userMsg( user, USER_ABBR_INVALID, ISUNIGNORABLE, CYAN ) );

  if( strcmp( "", getParam() ) == 0 )
  {
    if( user->removeAbbr( getCommand()[ 0 ] ) )
    {
      userMsg( user, USER_ABBR_DELETE, ISFEEDBACK, CYAN );
      return( user->saveAbbrs() );
    }

    return( userMsg( user, USER_ABBR_BAD_DELETE, ISFEEDBACK, CYAN ) );
  }

  sprintf( tempBuff, "%c %s", getCommand()[ 0 ], getParam() );

  user->addAbbr( getCommand()[ 0 ], getParam() );

  toSay.setColour( CYAN );
  toSay.setMessage( handyStr->getString( USER_CHANGE_ABBR ) );
  user->sendString( toSay );
  toSay.setMessage( tempBuff );
  user->sendString( toSay );
  return( user->saveAbbrs() );
}

// The user has gone away from keyboard.
//
boolean Command::afkUser( UserPtr user, char *msg )
{
  char tempBuff[ AFK_LENGTH + 1 ],
       afkBuff[ AFK_LENGTH + 1 ];

  strncpy( afkBuff, handyStr->getString( USER_AFK_END ), SAY_LENGTH );

  if( strcmp( msg, "" ) == 0 )
    strcpy( tempBuff, handyStr->getString( USER_AFK_DEFAULT ) );
  else
    strncpy( tempBuff, msg, AFK_LENGTH - strlen( afkBuff ) );

  strcat( tempBuff, afkBuff );

  user->setAfk( TRUE );
  user->setAfkMessage( tempBuff );

  return( sayUserType( user, tempBuff, ISEMOTE ) );
}

// Adds a site to the banned sites list.
//
boolean Command::bansiteUser( UserPtr user, char *cmd )
{
  char tempCmd[ SAY_LENGTH + 1 ];
  Say  toSay;
  int  count = talker->getNumBannedSites();

  // Did the user wish to display all banned sites, remove a banned site?
  //
  splitCommand( cmd );

  strcpy( tempCmd, getCommand() );
  lowerCase( tempCmd );

  // Show a list of all banned sites
  //
  if( strcmp( tempCmd, "" ) == 0 )
  {
    toSay.setSayType( ISUNIGNORABLE );
    toSay.setColour( CYAN + BOLD );
    toSay.setMessage( handyStr->getString( BANNED_START ) );
    user->sendString( toSay );

    toSay.setColour( CYAN );

    while( count > 0 )
    {
      toSay.setMessage( talker->getBannedSite( count-- ) );
      user->sendString( toSay );
    }

    toSay.setColour( CYAN + BOLD );
    toSay.setMessage( handyStr->getString( BANNED_END ) );
    user->sendString( toSay );
    return( TRUE );
  }

  // Must have a minimal amount of data in order to ban a site;
  // localhost may not be banned.
  //
  if( strlen( tempCmd ) < BAN_SITE_MIN_LEN )
    return( userMsg( user, BAN_MINIMUM_NAME, ISUNIGNORABLE, CYAN ) );

  if( strncmp( "localhost", tempCmd, strlen( tempCmd ) ) == 0 )
    return( userMsg( user, BAN_LOCALHOST_FAILED, ISUNIGNORABLE, CYAN ) );

  // Did the user wish to remove a banned site?
  //
  if( strncmp( "remove", tempCmd, strlen( tempCmd ) ) == 0 )
  {
    if( talker->unBanSite( getParam() ) == FALSE )
      return( userMsg( user,
                       BAN_SITE_REMOVE_FAILED,
                       ISUNIGNORABLE,
                       RED + BOLD ) );

    return( userMsg( user, BAN_SITE_REMOVED, ISFEEDBACK, CYAN ) );
  }

  if( talker->addBannedSite( getCommand() ) == TRUE )
  {
    talker->saveBannedSites();
    return( userMsg( user, BAN_SITE_ADDED, ISFEEDBACK, CYAN ) );
  }

  return( userMsg( user, BAN_SITE_FAILED, ISUNIGNORABLE, RED + BOLD ) );
}

boolean Command::beepUser( UserPtr user, char *msg )
{
  char tempBuff[ BIG_SAY_LENGTH + 1 ];

  sprintf( tempBuff, "%s%c", msg, BEEP_CHAR );
  setParam( tempBuff );
  return( tellUserType( user, tempBuff, ISTELL ) );
}

// Given a Bot name, try to register that user as a Bot
//
boolean Command::botUser( UserPtr user, char *botName )
{
  iLevel  level   = 0;
  UserPtr botUser;

  if( strcmp( botName, "" ) == 0 )
    return( FALSE );

  if( (botUser = talker->findUser( botName )) == NULL )
    return( FALSE );
    /*return( userMsg( ... ) );*/

  // Try to make the Bot the level required to do a .tell
  //
  if( (level = commandParser.getCommandLevel( "tell" )) < 0 )
    return( FALSE );

  // Tell "user" that botUser is now a Bot
  //

  botUser->setLevel( level );
  return( botUser->setState( ISBOT, TRUE ) );
}

// Bring the user specified by "userAlias" to user
//
boolean Command::bringUser( UserPtr user, char *userAlias )
{
  Say     toSay;
  RoomPtr roomTo      = NULL,
          roomFrom    = NULL;
  UserPtr userToBring = NULL;

  // No alias means the user needs feedback.
  //
  if( strcmp( userAlias, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  if( (userToBring = isMultipleUser( user, userAlias )) == NULL )
    return( FALSE );

  // Where are the two users located?
  //
  roomTo   = user->getRoom();
  roomFrom = userToBring->getRoom();

  if( (userToBring->getLevel() > user->getLevel()) ||
        ((userToBring->getLevel() < roomTo->getLevel()) &&
         (user->getLevel() < roomTo->getLevel())) )
    return( userMsg( user, NOT_ENOUGH_ACCESS, ISFEEDBACK, CYAN ) );

  handyStr->setRoomName( roomTo->getName() );
  handyStr->setCrewName( user->getName() );
  handyStr->setUserName( userToBring->getName() );

  toSay.setColour( CYAN );
  toSay.setSayType( ISFEEDBACK );

  if( roomTo == roomFrom )
  {
    toSay.setMessage( handyStr->getString( ALREADY_IN_ROOM ) );
    user->sendString( toSay );
    return( FALSE );
  }

  sayUserType( userToBring, handyStr->getString( USER_BEGIN_MOVED ),
               ISEMOTE, FALSE );

  moveUser( userToBring, roomFrom, roomTo );

  handyStr->setUserName( userToBring->getName() );
  handyStr->setCrewName( user->getName() );
  handyStr->setRoomName( roomFrom->getName() );

  sayUserType( userToBring, handyStr->getString( USER_END_MOVED ), ISEMOTE );
  return( TRUE );
}

// Display a message to all users
//
boolean Command::broadcastMsg( UserPtr useless, char *msg )
{
  Say  toSay;
  char tempBuff[ SAY_LENGTH + 1 ];

  sprintf( tempBuff, "^5*** ^6%s ^5***^", msg );
  toSay.setSayType( ISBROADCAST );
  toSay.setMessage( tempBuff );

  return( broadcastSay( toSay ) );
}

boolean Command::clearBufferUser( UserPtr user, char *useless )
{
  Say toSay;

  user->getRoom()->clearReviewBuffer();

  handyStr->setUserName( user->getName() );
  handyStr->setRoomName( user->getRoom()->getName() );
  toSay.setColour( GREEN );
  toSay.setSayType( ISFEEDBACK );
  toSay.setMessage( handyStr->getString( BUFFER_CLEARED ) );
  return( user->sendString( toSay ) );
}

boolean Command::clsUser( UserPtr user, char *useless )
{
  Say toSay;
  int count = 0;

  toSay.setMessage( "" );
  toSay.setSayType( ISUNIGNORABLE );

  while( count++ < SCREEN_ROWS + 1 )
    user->displaySay( toSay );

  toSay.setMessage( handyStr->getString( SUCCESSFUL_COMMAND ) );
  return( user->displaySay( toSay ) );
}

boolean Command::colourUser( UserPtr user, char *useless )
{
  return( userMsgStr( user,
    "^11 Blue, ^22 Green, ^33 Red, ^44 Magenta, ^55 Cyan, ^66 Yellow, ^77 White, ^88 Random",
    ISUNIGNORABLE, CYAN ) );
}

boolean Command::cmailUser( UserPtr user, char *cmd )
{
  Message  message;
  Say      toSay;
  char     tempBuff[ SAY_LENGTH + 1 ];
  FileName fileName;
  int      deleted = 0;

  if( user->getState( JAILED ) == TRUE )
    return( userMsg( user, USER_CANNOT_SMAIL, ISUNIGNORABLE, CYAN ) );

  if( strlen( cmd ) == 0 )
    return( userMsg( user, DELETE_WHAT_SMAIL, ISUNIGNORABLE, CYAN ) );

  strncpy( tempBuff, cmd, SAY_LENGTH );
  tempBuff[ SAY_LENGTH ] = 0;
  lowerCase( tempBuff );

  fileName.setFileName( USER_DIR, user->getName(), SMAIL_EXTENSION );

  // "all" will delete all the smail
  //
  if( strncmp( tempBuff, "all", 3 ) == 0 )
  {
    unlink( fileName.getFileName() );
    userMsg( user, DELETED_ALL_SMAIL, ISFEEDBACK, CYAN );
    return( TRUE );
  }

  if( (deleted = message.removeLines( cmd, fileName.getFileName() )) == -1 )
    return( userMsg( user, USER_NO_SMAIL, ISFEEDBACK, CYAN ) );

  sprintf( tempBuff, "^5%d^", deleted );
  handyStr->setNumber( tempBuff );
  toSay.setMessage( handyStr->getString( USER_SMAIL_DELETE ) );
  toSay.setColour( CYAN );
  toSay.setSayType( ISUNIGNORABLE );
  return( user->sendString( toSay ) );
}

boolean Command::rmailUser( UserPtr user, char *cmd )
{
  Message  message;
  char     tempBuff[ FILE_NAME_LENGTH + 1 ];
  FileName fileName;
  Say      toSay;

  if( user->getState( JAILED ) == TRUE )
    return( userMsg( user, USER_CANNOT_SMAIL, ISUNIGNORABLE, CYAN ) );

  fileName.setFileName( USER_DIR, user->getName(), SMAIL_EXTENSION );

  user->setState( NEWSMAIL, FALSE );

  handyStr->setUserName( user->getName() );
  toSay.setColour( CYAN + BOLD );
  toSay.setSayType( ISUNIGNORABLE );
  toSay.setMessage( handyStr->getString( USER_SMAIL_BEGIN ) );
  user->sendString( toSay );

  message.readFile( user, cmd, "", fileName.getFileName() );

  handyStr->setUserName( user->getName() );
  toSay.setMessage( handyStr->getString( USER_SMAIL_END ) );
  user->sendString( toSay );

  return( TRUE );
}

boolean Command::smailUser( UserPtr user, char *cmd )
{
  boolean  online      = FALSE;
  UserPtr  userToSmail = NULL;
  FileName fileName;
  Say      toSay;
  Message  message;

  if( user->getState( JAILED ) == TRUE )
    return( userMsg( user, USER_CANNOT_SMAIL, ISUNIGNORABLE, CYAN ) );

  // Who is the smail going to = getCommand(); what smail is = getParam()
  //
  splitCommand( cmd );

  if( strcmp( getCommand(), "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );
  handyStr->setUserName( getCommand() );

  if( strcmp( getParam(), "" ) == 0 )
  {
    toSay.setMessage( handyStr->getString( NEED_SMAIL ) );
    user->sendString( toSay );
    return( FALSE );
  }

  if( (userToSmail = getExactUser( getCommand(), &online )) == NULL )
  {
    toSay.setMessage( handyStr->getString( USER_NOT_EXIST ) );
    user->sendString( toSay );
    return( FALSE );
  }

  fileName.setFileName( USER_DIR, userToSmail->getName(), SMAIL_EXTENSION );

  if( message.appendMessage( user, getParam(), fileName.getFileName() ) == FALSE )
  {
    userToSmail->setSaveUser( FALSE );

    if( online == FALSE )
      delete( userToSmail );

    return( FALSE );
  }

  userToSmail->setState( NEWSMAIL, TRUE );
  userToSmail->setSaveUser( TRUE );

  // Tell user that userToSmail received it
  //
  handyStr->setUserName( userToSmail->getName() );
  userMsgStr( user, handyStr->getString( USER_RECEIVED_SMAIL ),
              ISFEEDBACK, GREEN );

  if( online == FALSE )
  {
    delete( userToSmail );
    return( TRUE );
  }

  showSmail( userToSmail );
  return( TRUE );
}

boolean Command::demoteUser( UserPtr user, char *userAlias )
{
  Say     toSay;
  UserPtr userToDemote = NULL;
  boolean online       = FALSE;

  if( strcmp( userAlias, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  toSay.setColour( CYAN );
  toSay.setSayType( ISUNIGNORABLE );

  if( (userToDemote = getUser( userAlias, &online )) == NULL )
  {
    handyStr->setUserName( userAlias );
    toSay.setMessage( handyStr->getString( USER_NOT_EXIST ) );
    user->sendString( toSay );
    return( FALSE );
  }

  if( userToDemote->getLevel() == getHighLevel() )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  if( userToDemote->getLevel() == 1 )
  {
    if( online == FALSE )
      delete( userToDemote );

    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );
  }

  userToDemote->setLevel( userToDemote->getLevel() - 1 );

  if( online == FALSE )
  {
    userToDemote->setSaveUser( TRUE );
    delete( userToDemote );
  }

  handyStr->setUserName( userToDemote->getName() );
  handyStr->setCrewName( user->getName() );
  handyStr->setLevelName( getLevelName( userToDemote->getLevel() ) );
  toSay.setMessage( handyStr->getString( DEMOTE ) );
  toSay.setSayType( ISBROADCAST );
  toSay.setColour( CYAN + BOLD );
  return( broadcastSay( toSay ) );
}

// If no description was given, then show current description,
// otherwise change the description.
//
boolean Command::descUser( UserPtr user, char *desc )
{
  handyStr->setUserName( user->getName() );

  // No parameter was given, show the current description
  //
  if( strcmp( desc, "" ) == 0 )
  {
    handyStr->setUserDesc( user->getDescription() );
    return( userMsgStr( user, handyStr->getString( USER_CURRENT_DESC ),
                        ISUNIGNORABLE, CYAN ) );
  }

  user->setDescription( desc );

  // Inform the user her description has been changed.
  //
  handyStr->setUserDesc( user->getDescription() );
  return( userMsgStr( user, handyStr->getString( USER_CHANGED_DESC ),
                      ISFEEDBACK, CYAN ) );
}

boolean Command::downUser( UserPtr user, char *cmd )
{
  Say     toSay;
  UserPtr userToDemote = NULL;

  if( strcmp( cmd, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  if( (userToDemote = isMultipleUser( user, cmd )) == NULL )
    return( FALSE );

  if( userToDemote->getLevel() != 2 )
  {
    handyStr->setLevelName( getLevelName( 2 ) );
    handyStr->setUserName( user->getName() );
    toSay.setColour( CYAN );
    toSay.setSayType( ISFEEDBACK );
    toSay.setMessage( handyStr->getString( USER_CANNOT_DOWN ) );
    user->sendString( toSay );
    return( FALSE );
  }

  return( demoteUser( user, cmd ) );
}

boolean Command::echoUser( UserPtr user, char *msg )
{
  Say toSay;

  toSay.setMessage( msg );

  if( strcmp( msg, "" ) == 0 )
    return( userMsg( user, ECHO_WHAT, ISFEEDBACK, CYAN ) );

  toSay.setSayType( ISECHO );
  toSay.setUserFrom( user->getName() );

  return( sayToRoom( user, toSay ) );
}

boolean Command::emoteCrew( UserPtr user, char *cmd )
{
  return( sayWizType( user, cmd, ISCREWEMOTE ) );
}

boolean Command::emoteUser( UserPtr user, char *cmd )
{
  return( sayUserType( user, cmd, ISEMOTE ) );
}

// Examine a user's profile ...
//
boolean Command::examineUser( UserPtr user, char *userAlias )
{
  Say     toSay;
  UserPtr userToExam = NULL;
  boolean online     = FALSE;        // Search offline as well

  if( strcmp( userAlias, "" ) == 0 )
  {
    userToExam = user;
    online     = TRUE;
  }
  else if( (userToExam = getExactUser( userAlias, &online )) == NULL )
  {
    handyStr->setUserName( userAlias );
    toSay.setSayType( ISUNIGNORABLE );
    toSay.setColour( CYAN );
    toSay.setMessage( handyStr->getString( USER_NOT_EXIST ) );
    user->sendString( toSay );
    return( FALSE );
  }

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( BLUE + BOLD );
  toSay.setMessage( handyStr->getString( PROFILE_START ) );
  user->sendString( toSay );

  examineUser( user, userToExam, FALSE );

  toSay.setMessage( handyStr->getString( PROFILE_END ) );

  // Don't really want to delete a user that's online ...
  //
  if( online == FALSE )
    delete( userToExam );

  return( user->sendString( toSay ) );
}

boolean Command::freeUser( UserPtr user, char *userAlias )
{
  UserPtr userToFree = NULL;
  RoomPtr roomFrom, roomTo;
  Say     toSay;

  if( strcmp( userAlias, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  if( (userToFree = isMultipleUser( user, userAlias )) == NULL )
    return( FALSE );

  if( userToFree->getState( JAILED ) == FALSE )
    return( userMsg( user, ALREADY_FREE, ISUNIGNORABLE, CYAN ) );

  handyStr->setUserName( userToFree->getName() );
  sayWizType( user, handyStr->getString( USER_FREE ), ISCREWEMOTE );

  userToFree->setState( JAILED, FALSE );
  roomFrom = userToFree->getRoom();
  roomTo   = talker->findRoom( MAIN_ROOM );

  if( userToFree->getRoom() != roomTo )
    moveUser( userToFree, roomFrom, roomTo );

  handyStr->setRoomName( roomFrom->getName() );
  sayUserType( userToFree, handyStr->getString( ENTRANCE_USER ), ISEMOTE );
  return( TRUE );
}

// A user may move herself from the current room to an adjoining one
//
// Higher level access can call moveUser directly.
//
boolean Command::goUser( UserPtr user, char *roomName )
{
  Say     toSay;

  RoomPtr roomTo   = NULL,
          roomFrom = NULL;

  char    tempBuff[ SAY_LENGTH + 1 ],
          roomBuff[ SAY_LENGTH + 1 ];

  if( user->getState( JAILED ) == TRUE )
    return( userMsg( user, USER_CANNOT_GO, ISUNIGNORABLE, CYAN ) );

  strncpy( roomBuff, roomName, COMMON_NAME_LEN );
  roomName[ COMMON_NAME_LEN ] = 0;
  lowerCase( roomBuff );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  // No parameter means to go back to the main room
  //
  if( strcmp( roomBuff, "" ) == 0 )
    roomTo = talker->findRoom( MAIN_ROOM );
  else if( (roomTo = talker->findRoom( roomBuff )) == NULL )
  {
    handyStr->setRoomName( roomName );
    handyStr->setUserName( user->getName() );
    toSay.setMessage( handyStr->getString( INVALID_ROOM ) );
    user->sendString( toSay );
    return( FALSE );
  }

  roomFrom = user->getRoom();

  handyStr->setRoomName( roomTo->getName() );
  handyStr->setUserName( user->getName() );

  if( roomTo == roomFrom )
  {
    toSay.setMessage( handyStr->getString( ALREADY_IN_ROOM ) );
    user->sendString( toSay );
    return( FALSE );
  }

  // Make sure that user is allowed to go to that room!
  //
  if( (user->getLevel() < roomTo->getLevel()) &&
      (user->isInvited( roomTo ) == FALSE) )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  // If the room isn't adjoined, and the user doesn't have enough access
  // to warp into the room, indicate such.
  //
  if( (talker->areAdjoined( roomFrom, roomTo ) == FALSE) &&
      (user->getLevel() < roomTo->getWarpRank()) &&
      (user->isInvited( roomTo ) == FALSE) )
  {
    toSay.setMessage( handyStr->getString( NOT_ADJOINED ) );
    user->sendString( toSay );
    return( FALSE );
  }

  // If the room is private indicate such.
  //
  if( (roomTo->getLocked() == TRUE) &&
      (user->isInvited( roomTo ) == FALSE) )
  {
    toSay.setMessage( handyStr->getString( ROOM_PRIVATE ) );
    user->sendString( toSay );
    return( FALSE );
  }

  if( user->getLevel() > 2 )
    strcpy( tempBuff, handyStr->getString( CREW_LEAVES ) );
  else
    strcpy( tempBuff, handyStr->getString( USER_LEAVES ) );

  // Tell everybody that the user is leaving (don't display it to the
  // user herself)
  //
  sayUserType( user, tempBuff, ISEMOTE, FALSE );

  moveUser( user, roomFrom, roomTo );

  handyStr->reset();
  handyStr->setRoomName( roomFrom->getName() );

  if( user->getLevel() > 2 )
    strcpy( tempBuff, handyStr->getString( ENTRANCE_CREW ) );
  else
    strcpy( tempBuff, handyStr->getString( ENTRANCE_USER ) );

  // Tell everybody that the user has arrived
  //
  return( sayUserType( user, tempBuff, ISEMOTE ) );
}

boolean Command::helpUser( UserPtr user, char *helpOn )
{
  FILE     *fp;
  FileName fileName;
  Say      toSay;
  char     tempBuff[ SAY_LENGTH + 1 ],        // Line read in from help file
           tempFName[ FILE_NAME_LENGTH + 1 ];

  if( user->getState( JAILED ) == TRUE )
    return( userMsg( user, USER_CANNOT_HELP, ISUNIGNORABLE, CYAN ) );

  // Figure out the real file name that the user requested help on
  // (there is a 1 to 1 correlation between the file name and
  // the name of the command).
  //
  commandParser.getHelp( helpOn, tempBuff );

  // All help files end with HELP_EXTENSION
  //
  fileName.setFileName( HELP_DIR, &tempBuff[ 1 ], HELP_EXTENSION );

  fp = fopen( fileName.getFileName(), "r" );

  // Help file doesn't exist yet ...
  //
  if( fp == NULL )
    return( userMsg( user, NO_HELP_AVAILABLE, ISUNIGNORABLE, CYAN ) );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  while( fgets( tempBuff, SAY_LENGTH, fp ) != NULL )
  {
    // Don't want no steenkin' newline character.
    //
    tempBuff[ strlen( tempBuff ) - 1 ] = 0;

    toSay.setMessage( tempBuff );
    user->sendString( toSay );
  }

  fclose( fp );

  return( TRUE );
}

boolean Command::hideUser( UserPtr user, char *userAlias )
{
  UserPtr userToHide = NULL;

  // If no parameter was given, un/hide "user"
  //
  if( strcmp( userAlias, "" ) == 0 )
  {
    user->setState( INVISIBLE, !user->getState( INVISIBLE ) );
    userToHide = user;
  }
  else
  {
    if( (userToHide = isMultipleUser( user, userAlias )) == NULL )
      return( FALSE );

    // Toggle hide state
    //
    if( user->getLevel() >= userToHide->getLevel() )
      userToHide->setState( INVISIBLE, !userToHide->getState( INVISIBLE ) );
    else
      return( userMsg( user, NOT_ENOUGH_ACCESS, ISFEEDBACK, CYAN ) );
  }

  // User was JUST hidden?
  //
  if( userToHide->getState( INVISIBLE ) == TRUE )
    return( sayUserType( userToHide,
                         handyStr->getString( USER_BEGIN_INVISIBLE ),
                         ISEMOTE ) );

  return( sayUserType( userToHide,
                       handyStr->getString( USER_END_INVISIBLE ),
                       ISEMOTE ) );
}

boolean Command::inviteUser( UserPtr user, char *userAlias )
{
  Say     toSay;
  RoomPtr room          = user->getRoom(),
          userAliasRoom = NULL;         // Room of the user to invite
  UserPtr userToInvite  = NULL;

  // Show who is on the invite list ...
  //
  if( strcmp( userAlias, "" ) == 0 )
  {
  //  ListPtr invitedUsers  = NULL;
  //  int     userCount     = 0;

    return( TRUE );
  }

  if( (userToInvite = isMultipleUser( user, userAlias )) == NULL )
    return( FALSE );

  toSay.setColour( CYAN );
  toSay.setSayType( ISUNIGNORABLE );

  // If the user is already in the room, don't send another invite.
  //
  userAliasRoom = userToInvite->getRoom();

  handyStr->setUserName( user->getName() );
  handyStr->setRoomName( room->getName() );

  if( room == userAliasRoom )
  {
    handyStr->setUserName( userToInvite->getName() );
    toSay.setMessage( handyStr->getString( ALREADY_IN_ROOM ) );
    user->sendString( toSay );
    return( FALSE );
  }

  userToInvite->invite( room );

  // Tell userTo she has been invited into the room
  //
  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( GREEN + BOLD );
  toSay.setMessage( handyStr->getString( USER_INVITE ) );
  userToInvite->sendString( toSay );

  handyStr->setUserName( userToInvite->getName() );
  toSay.setMessage( handyStr->getString( USER_INVITED ) );
  return( user->sendString( toSay ) );
}

boolean Command::jailUser( UserPtr user, char *userAlias )
{
  UserPtr userToJail = NULL;
  Say     toSay;

  if( strcmp( userAlias, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  if( (userToJail = isMultipleUser( user, userAlias )) == NULL )
    return( FALSE );

  if( userToJail->getState( JAILED ) == TRUE )
    return( userMsg( user, ALREADY_JAILED, ISUNIGNORABLE, CYAN ) );

  if( userToJail->getLevel() >= user->getLevel() )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  userToJail->setLevel( 1 );
  userToJail->setState( JAILED, TRUE );
  userToJail->setState( MUZZLED, TRUE );
  userToJail->setDescription( handyStr->getString( JAILED_DESC ) );
  moveUser( userToJail, JAIL_ROOM );

  handyStr->setUserName( userToJail->getName() );
  handyStr->setCrewName( user->getName() );
  handyStr->setRoomName( userToJail->getRoom()->getName() );
  toSay.setSayType( ISBROADCAST );
  toSay.setMessage( handyStr->getString( USER_JAILED ) );
  toSay.setColour( CYAN + BOLD );
  broadcastSay( toSay );

  return( TRUE );
}

boolean Command::joinUser( UserPtr user, char *userAlias )
{
  Say     toSay;
  RoomPtr roomTo     = NULL,
          roomFrom   = NULL;
  UserPtr userToJoin = NULL;
  char    tempBuff[ SAY_LENGTH + 1 ];

  // No alias means the user needs feedback.
  //
  if( strcmp( userAlias, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  if( (userToJoin = isMultipleUser( user, userAlias )) == NULL )
    return( FALSE );

  // Where are the two users located?
  //
  roomFrom = user->getRoom();
  roomTo   = userToJoin->getRoom();

  toSay.setColour( CYAN );
  toSay.setSayType( ISFEEDBACK );

  handyStr->setUserName( user->getName() );
  handyStr->setRoomName( roomTo->getName() );

  if( roomTo == roomFrom )
  {
    toSay.setMessage( handyStr->getString( ALREADY_IN_ROOM ) );
    user->sendString( toSay );
    return( FALSE );
  }

  // Can't join a user into a room whose warp level is than that of the
  // user who wants to join
  //
  if( (user->getLevel() < roomTo->getWarpRank()) &&
      (user->isInvited( roomTo ) == FALSE) )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  // Is the room locked?  Is the user invited?
  //
  if( (roomTo->getLocked() == TRUE) &&
      (user->isInvited( roomTo ) == FALSE ) )
  {
    userMsgStr( user, handyStr->getString( ROOM_PRIVATE ), ISFEEDBACK, CYAN );
    return( FALSE );
  }

  if( user->getLevel() > 2 )
    strcpy( tempBuff, handyStr->getString( CREW_LEAVES ) );
  else
    strcpy( tempBuff, handyStr->getString( USER_LEAVES ) );

  // Tell everybody that the user is leaving
  //
  sayUserType( user, tempBuff, ISEMOTE, FALSE );
  moveUser( user, roomFrom, roomTo );

  handyStr->setUserName( userToJoin->getName() );
  handyStr->setCrewName( user->getName() );
  handyStr->setRoomName( roomFrom->getName() );

  if( user->getLevel() > 2 )
    strcpy( tempBuff, handyStr->getString( ENTRANCE_CREW ) );
  else
    strcpy( tempBuff, handyStr->getString( ENTRANCE_USER ) );

  // Tell everybody that the user has arrived
  //
  return( sayUserType( user, tempBuff, ISEMOTE ) );
}

boolean Command::killUser( UserPtr user, char *userAlias )
{
  Say     toSay;
  UserPtr userToKill = NULL;

  if( strcmp( userAlias, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  if( (userToKill = isMultipleUser( user, userAlias )) == NULL )
    return( FALSE );

  if( (userToKill->getLevel() > 2) &&
      (user->getLevel() < getHighLevel()) )
    return( userMsg( user, CREW_NO_KILL, ISUNIGNORABLE, CYAN ) );

  handyStr->setUserName( userToKill->getName() );
  handyStr->setCrewName( user->getName() );
  toSay.setMessage( handyStr->getString( USER_KILLED ) );
  toSay.setColour( CYAN + BOLD );
  toSay.setSayType( ISUNIGNORABLE );

  // Broadcast the killed message to everybody
  //
  broadcastSay( toSay );
  quitUser( userToKill, "useless" );
  userToKill = NULL;
  return( TRUE );
}

boolean Command::knockUser( UserPtr user, char *roomName )
{
  ListPtr userList   = NULL;
  UserPtr userInRoom = NULL;
  int     userCount  = 0;

  RoomPtr room;
  Say     toSay;

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  handyStr->setUserName( user->getName() );

  if( (room = talker->findRoom( roomName )) == NULL )
  {
    handyStr->setRoomName( roomName );
    toSay.setMessage( handyStr->getString( INVALID_ROOM ) );
    user->sendString( toSay );
    return( FALSE );
  }

  handyStr->setRoomName( room->getName() );

  if( user->getRoom() == room )
  {
    toSay.setMessage( handyStr->getString( ALREADY_IN_ROOM ) );
    user->sendString( toSay );
    return( FALSE );
  }

  // See if the user is already invited
  //
  if( user->isInvited( room ) == TRUE )
  {
    toSay.setMessage( handyStr->getString( USER_ALREADY_INVITED ) );
    user->sendString( toSay );
    return( FALSE );
  }

  userList = room->getUserList();

  // Inform everybody in that room that "user" wants to be invited
  //
  toSay.setSayType( ISINVITE );
  toSay.setMessage( handyStr->getString( USER_KNOCKS ) );

  while( (userInRoom = (UserPtr)userList->at( userCount++ )) != NULL )
    userInRoom->sendString( toSay );

  toSay.setSayType( ISUNIGNORABLE );
  handyStr->setRoomName( room->getName() );
  toSay.setMessage( handyStr->getString( USER_REQUESTS_INVITE ) );
  user->sendString( toSay );

  return( TRUE );
}

boolean Command::loadUser( UserPtr user, char *options )
{
  char tempBuff[ SAY_LENGTH + 1 ];

  lowerCase( options );
  splitCommand( options );

  if( strncmp( "rooms", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    Say toSay;

    if( talker->loadRoomLinks( tempBuff ) )
      return( userMsg( user, ROOMS_LOAD_SUCCESSFUL, ISFEEDBACK, GREEN ) );

    handyStr->setRoomName( tempBuff );
    toSay.setMessage( handyStr->getString( ROOM_LOAD_FAILED ) );
    toSay.setColour( CYAN );
    toSay.setSayType( ISUNIGNORABLE );
    user->sendString( toSay );
  }
  else if( strncmp( "strings", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    if( handyStr->load() )
      return( userMsg( user, STRINGS_LOAD_SUCCESSFUL, ISFEEDBACK, GREEN ) );
  }
  else if( strncmp( "banned", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    if( talker->loadBannedSites() )
      return( userMsg( user, BANNED_LOAD_SUCCESSFUL, ISFEEDBACK, GREEN ) );
  }
  else if( strncmp( "levels", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    if( loadLevels() )
      return( userMsg( user, LEVEL_LOAD_SUCCESSFUL, ISFEEDBACK, GREEN ) );
  }
  else if( strncmp( "commands", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    if( commandParser.loadCommands() )
      return( userMsg( user, COMMAND_LOAD_SUCCESSFUL, ISFEEDBACK, GREEN ) );
  }
  else if( strncmp( "macros", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    if( talker->loadMacros() )
      return( userMsg( user, MACRO_LOAD_SUCCESSFUL, ISFEEDBACK, GREEN ) );
  }

  return( userMsg( user, LOAD_FAILED, ISUNIGNORABLE, CYAN ) );
}

boolean Command::logonUser( UserPtr user, char *cmd )
{
  TTime aTime;
  Say   toSay;
  char  enterBuffer[ SAY_LENGTH + 1 ];

  toSay.setSayType( ISLOGONMSG );

  switch( user->getLogonState() )
  {
    // Just starting to log on; the command is an alias (try to load it).
    //
    case 0:
      cmd[ COMMON_NAME_LEN ] = 0;

      if( isInvalidAlias( cmd ) )
      {
//        sprintf( enterBuffer, "%s\n", handyStr->getString( INVALID_ALIAS ) );
        toSay.setMessage( handyStr->getString( INVALID_ALIAS ) );
        user->sendString( toSay );

        toSay.setLineFeed( FALSE );
        toSay.setMessage( handyStr->getString( LOGON_PROMPT ) );
        user->sendString( toSay );
        return( FALSE );
      }

      strcpy( enterBuffer, cmd );
      lowerCase( enterBuffer );

      if( strncmp( enterBuffer, "quit", 4 ) == 0 )
      {
        toSay.setLineFeed( TRUE );
        toSay.setMessage( handyStr->getString( USER_QUIT_LOGON ) );
        user->sendString( toSay );
        user->setSaveUser( FALSE );
        quitUser( user, "" );
        return( FALSE );
      }

      // If the user can be loaded, then she exists (duh!)
      //
      if( user->load( cmd ) == TRUE )
      {
        handyStr->setUserName( user->getName() );
        toSay.setLineFeed( FALSE );
        toSay.setColour( GREEN + BOLD );
        toSay.setMessage( handyStr->getString( PASSWORD_PROMPT ) );
        user->sendString( toSay );

        if( user->getLevel() > getHighLevel() )
          user->setLevel( getHighLevel() );

        // Oldie logging on
        //
        return( user->setLogonState( 3 ) );
      }

      // If the *new* user is banned, remove them.
      //
      if( talker->isBannedSite( user->getSite() ) )
      {
        userMsgStr( user, "", ISUNIGNORABLE, CYAN + BOLD );
        userMsg( user, USER_BANNED, ISUNIGNORABLE, CYAN + BOLD );
        user->setSaveUser( FALSE );
        quitUser( user, "" );
        return( USER_QUIT );
      }

      // MAJOR Security hole is fixed by this line (the other two just
      // clean up)
      //
      user->secureReset();
      user->initMacros();
      user->profile.clear();

      // Newbie logging on
      //
      handyStr->setUserName( user->getName() );
      sprintf( enterBuffer, "\n%s", handyStr->getString( NEWBIE_WELCOME ) );
      toSay.setMessage( enterBuffer );
      toSay.setLineFeed( TRUE );
      user->sendString( toSay );

      handyStr->setUserName( user->getName() );
      toSay.setLineFeed( FALSE );
      toSay.setMessage( handyStr->getString( NEWBIE_PASSWORD_PROMPT ) );
      user->sendString( toSay );

      return( user->setLogonState( 1 ) );

    // User is a newbie (cmd is the new password)
    //
    case 1:
      handyStr->setUserName( user->getName() );

      // Ensure the password isn't too short
      //
      if( strlen( cmd ) < MIN_PASSWORD_LEN )
      {
        toSay.setMessage( handyStr->getString( PASSWORD_TOO_SHORT ) );
        user->sendString( toSay );

        sprintf( enterBuffer, "\n%s", handyStr->getString( LOGON_PROMPT ) );
        toSay.setLineFeed( FALSE );
        toSay.setMessage( enterBuffer );
        user->sendString( toSay );

        // Start all over ...
        //
        return( user->setLogonState( 0 ) );
      }

      // Set the newbie's password; ask for verification
      //
      user->setPassword( cmd );

      toSay.setLineFeed( FALSE );
      toSay.setMessage( handyStr->getString( VERIFY_PASSWORD_PROMPT ) );
      user->sendString( toSay );

      // Set up to do a newbie password verification
      //
      return( user->setLogonState( 2 ) );

    // User is a newbie (cmd is the verify password)
    //
    case 2:
      // Lower case password comparison
      //
      lowerCase( cmd );

      // Scramble the password
      //
      //handyStr->scramble( cmd );

      handyStr->setUserName( user->getName() );

      // If the passwords are the same, log the user in!
      //
      if( strcmp( cmd, user->getPassword() ) == 0 )
      {
        UserPtr multiUser = NULL;  // Check for multiple logons

        // If there was another user by the same name online, get
        // rid of her
        //
        if( (multiUser = talker->findUserExact( user->getName() )) != NULL )
          if( multiUser != user )
            quitUser( multiUser, "useless" );

        handyStr->setLevelName( getLevelName( user->getLevel() ) );
        user->setDescription( handyStr->getString( NEWBIE_DESC ) );

        user->setLoggingOn( FALSE );
        toSay.setMessage( "" );
        user->sendString( toSay );

        handyStr->setUserName( user->getName() );
        handyStr->setUserDesc( user->getDescription() );
        strcpy( enterBuffer, handyStr->getString( USER_ENTER_TALKER ) );

        // So Bots know who is logging off ...
        //
        toSay.setUserFrom( user->getName() );

        toSay.setMessage( enterBuffer );
        toSay.setColour( GREEN );
        toSay.setSayType( ISLOGON );
        broadcastSay( toSay );

        user->setSaveUser( TRUE );
        user->setTimeOn( aTime.getCurrentTime() );
        moveUser( user, MAIN_ROOM );

        showSmail( user );

        return( TRUE );
      }
      else
      {
        // Tell the user that the passwords are different
        //
        toSay.setMessage( handyStr->getString( PASSWORDS_DIFFERENT ) );
        user->sendString( toSay );

        sprintf( enterBuffer, "\n%s", handyStr->getString( LOGON_PROMPT ) );

        toSay.setLineFeed( FALSE );
        toSay.setMessage( enterBuffer );
        user->sendString( toSay );

        // Start all over ...
        //
        return( user->setLogonState( 0 ) );
      }

    // User is an oldie (cmd is the password)
    //
    case 3:
      // Lower case password comparison
      //
      lowerCase( cmd );

      // Scramble the password
      //
      //handyStr->scramble( cmd );

      handyStr->setUserName( user->getName() );

      // If the passwords are the same, log the user in!
      //
      if( strcmp( cmd, user->getPassword() ) == 0 )
      {
        UserPtr multiUser = NULL;  // Check for multiple logons

        // If there was another user by the same name online, get
        // rid of her
        //
        if( (multiUser = talker->findUserExact( user->getName() )) != NULL )
          if( multiUser != user )
            quitUser( multiUser, "useless" );

        user->setSaveUser( TRUE );
        user->setTimeOn( aTime.getCurrentTime() );

        user->setLoggingOn( FALSE );
        toSay.setMessage( "" );
        user->sendString( toSay );

        handyStr->setUserName( user->getName() );
        handyStr->setUserDesc( user->getDescription() );
        strcpy( enterBuffer, handyStr->getString( USER_ENTER_TALKER ) );

        // So Bots know who is logging off ...
        //
        toSay.setUserFrom( user->getName() );

        toSay.setMessage( enterBuffer );
        toSay.setColour( GREEN );
        toSay.setSayType( ISLOGON );
        broadcastSay( toSay );

        if( user->getState( JAILED ) == TRUE )
          moveUser( user, JAIL_ROOM );
        else
          moveUser( user, MAIN_ROOM );

        showSmail( user );

        return( TRUE );
      }
      else
      {
        // Tell the user that the password is not right
        //
        toSay.setMessage( handyStr->getString( PASSWORD_INCORRECT ) );
        user->sendString( toSay );

        sprintf( enterBuffer, "\n%s", handyStr->getString( LOGON_PROMPT ) );
        toSay.setLineFeed( FALSE );
        toSay.setMessage( enterBuffer );
        user->sendString( toSay );

        // Start all over ...
        //
        return( user->setLogonState( 0 ) );
      }
  }

  return( FALSE );
}

// Look around the room (description of the room and people in it)
//
boolean Command::lookUser( UserPtr user, char *roomName )
{
  RoomPtr room = NULL;           // Room the user is in
  int     count = 0;
  Say     toSay;
  char    tempBuff[ SAY_LENGTH + 1 ];

  if( (room = peekRoom( user, roomName )) == NULL )
    return( FALSE );

  // ISROOMINFO will make sure this line gets displayed when the user is
  // logging in
  //
  if( user->getLoggingOn() == TRUE )
    toSay.setSayType( ISROOMINFO );
  else
    toSay.setSayType( ISUNIGNORABLE );

  toSay.setMessage( "" );
  user->displaySay( toSay );

  handyStr->setRoomName( room->getName() );
  toSay.setColour( BLUE + BOLD );

  if( user->getRoom() == room )
    toSay.setMessage( handyStr->getString( ROOM_ENTRANCE ) );
  else
    toSay.setMessage( handyStr->getString( REMOTE_ROOM_ENTRANCE ) );

  user->displaySay( toSay );

  // Display the room's description, if the user isn't ignoring them
  //
  if( user->getState( BRIEF ) == FALSE )
  {
    int speed = room->profile.getNumLines();

    toSay.setSayType( ISROOMINFO );
    toSay.setColour( CYAN );
    toSay.setMessage( "" );
    user->displaySay( toSay );

    while( count < speed )
    {
      toSay.setMessage( room->profile.getLine( count++ ) );
      user->displaySay( toSay );
    }
  }

  toSay.setMessage( "" );
  user->displaySay( toSay );

  showAdjoinedRooms( user, room );

  // Display the current topic
  //
  handyStr->setRoomName( room->getName() );
  sprintf( tempBuff, "%s%s", handyStr->getString( CHECK_TOPIC ),
                             room->getTopic() );
  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( GREEN );
  toSay.setMessage( tempBuff );
  user->displaySay( toSay );

  handyStr->setRoomName( room->getName() );

  if( room->getLocked() == FALSE )
    toSay.setMessage( handyStr->getString( ROOM_UNLOCKED ) );
  else
    toSay.setMessage( handyStr->getString( ROOM_LOCKED ) );

  toSay.setLineFeed( FALSE );
  user->displaySay( toSay );

  sprintf( tempBuff, "^5%d^", room->getNumMessages() );
  handyStr->setRoomName( room->getName() );
  handyStr->setNumber( tempBuff );
  toSay.setMessage( handyStr->getString( ROOM_NUMBER_MESSAGES ) );
  toSay.setLineFeed( TRUE );
  user->displaySay( toSay );

  toSay.setMessage( "" );
  user->displaySay( toSay );

  showUsersInRoom( user, room );

  toSay.setMessage( "" );
  return( user->displaySay( toSay ) );
}

boolean Command::macroUser( UserPtr user, char *macro )
{
  Say toSay;

  char tempCmd[ SAY_LENGTH + 1 ],
       macroToAdd[ SAY_LENGTH + 1 ];

  int  count = 0;

  // First thing to do is split the macro into two parts:
  //   1) The macro name
  //   2) The macro's action
  //
  splitCommand( macro );

  // Make the macro name (or command) lower case
  //
  strncpy( tempCmd, getCommand(), SAY_LENGTH );
  tempCmd[ SAY_LENGTH ] = 0;
  lowerCase( tempCmd );

  // If no name was specified, the user wants to view all her macros
  //
  if( strcmp( tempCmd, "" ) == 0 )
  {
    toSay.setSayType( ISUNIGNORABLE );
    toSay.setMessage( handyStr->getString( CURRENT_MACROS_BEGIN ) );
    toSay.setColour( CYAN + BOLD );
    user->sendString( toSay );

    toSay.setColour( CYAN );

    // Display the macros
    //
    while( count < MACRO_MAX )
    {
      if( strcmp( user->getMacroAt( count ), "" ) != 0 )
      {
        toSay.setMessage( user->getMacroAt( count ) );
        user->sendString( toSay );
      }

      count++;
    }

    toSay.setMessage( handyStr->getString( CURRENT_MACROS_END ) );
    toSay.setColour( CYAN + BOLD );

    return( user->sendString( toSay ) );
  }

  // Did the user specify a macro?
  //
  if( strcmp( getParam(), "" ) == 0 )
    return( userMsg( user, MACRO_NEED_MACRO, ISUNIGNORABLE, CYAN ) );

  // Did the user wish to delete a macro?
  //
  if( strncmp( "delete", tempCmd, strlen( tempCmd ) ) == 0 )
  {
    // Figure out which macro the user wanted to delete
    //
    if( user->deleteMacro( getParam() ) == TRUE )
    {
      userMsg( user, MACRO_DEL_SUCCESSFUL, ISFEEDBACK, GREEN );
      user->saveMacros();
    }
    else
      userMsg( user, MACRO_DEL_FAILED, ISUNIGNORABLE, CYAN );

    return( TRUE );
  }

  // Strip out any non-alphanumeric characters
  //
  count = 0;

  while( !isalnum( tempCmd[ count ] ) )
    count++;

  sprintf( macroToAdd, ".%s %s", &tempCmd[ count ], getParam() );

  // Delete the macro if it already exists
  //
  user->deleteMacro( tempCmd );

  if( user->addMacro( macroToAdd ) == TRUE )
  {
    userMsg( user, MACRO_ADDED_SUCCESSFUL, ISFEEDBACK, GREEN );
    user->saveMacros();
  }
  else
    return( userMsg( user, MACRO_ADDED_FAILED, ISUNIGNORABLE, CYAN ) );

  return( TRUE );
}

boolean Command::mapUser( UserPtr user, char *useless )
{
  FILE *fp;
  Say  toSay;
  char fileName[ FILE_NAME_LENGTH + 1 ],
       mapLine[ SAY_LENGTH + 1 ];

  sprintf( fileName, "%s%s", DATA_DIR, MAP_FILENAME );

  fp = fopen( fileName, "r" );

  if( fp == NULL )
  {
    // No map file ...
    //
    return( FALSE );
  }

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  while( fgets( mapLine, SAY_LENGTH, fp ) != NULL )
  {
    // Don't want no steenkin' newline character.
    //
    mapLine[ strlen( mapLine ) - 1 ] = 0;

    toSay.setMessage( mapLine );
    user->sendString( toSay );
  }

  fclose( fp );
  return( TRUE );
}

// Move a user from where they are to a specified room
//
boolean Command::moveUser( UserPtr user, char *cmd )
{
  char tempBuff[ SAY_LENGTH + 1 ];

  strncpy( tempBuff, cmd, SAY_LENGTH );
  tempBuff[ SAY_LENGTH ] = 0;
  lowerCase( tempBuff );

  splitCommand( tempBuff );

  return( moveUser( user, getCommand(), getParam() ) );
}

boolean Command::muzzleUser( UserPtr user, char *userAlias )
{
  Say     toSay;
  UserPtr userToMuzzle;

  if( (userToMuzzle = isMultipleUser( user, userAlias )) == NULL )
    return( FALSE );

  if( (userToMuzzle->getLevel() > getWizLevel()) &&
      (user->getLevel() < getHighLevel()) )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  userToMuzzle->setState( MUZZLED, !userToMuzzle->getState( MUZZLED ) );

  toSay.setSayType( ISUNIGNORABLE );

  if( userToMuzzle->getState( MUZZLED ) == TRUE )
  {
    handyStr->setUserName( user->getName() );
    toSay.setColour( CYAN + BOLD );
    toSay.setMessage( handyStr->getString( USER_MUZZLED ) );

    handyStr->setUserName( userToMuzzle->getName() );
    sayWizType( user, handyStr->getString( CREW_MUZZLED_USER ), ISCREWEMOTE );
  }
  else
  {
    handyStr->setUserName( user->getName() );
    toSay.setColour( GREEN + BOLD );
    toSay.setMessage( handyStr->getString( USER_UNMUZZLED ) );

    handyStr->setUserName( userToMuzzle->getName() );
    sayWizType( user, handyStr->getString( CREW_UNMUZZLED_USER ), ISCREWEMOTE );
  }

  return( userToMuzzle->sendString( toSay ) );
}

boolean Command::nukeUser( UserPtr user, char *userAlias )
{
  Say      toSay;
  UserPtr  userToNuke = NULL;
  boolean  online     = FALSE;
  FileName fileName;
  char     baseName[ FILE_NAME_LENGTH + 1 ],
           userFile[ FILE_NAME_LENGTH + 1 ];

  if( strcmp( userAlias, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  // See if the user is online
  //
  if( (userToNuke = talker->findUserExact( userAlias )) == NULL )
  {
    userToNuke = new User( userAlias );
    userToNuke->setSaveUser( FALSE );

    // Offline then?
    //
    if( userToNuke->load() == FALSE )
    {
      handyStr->setUserName( userAlias );
      toSay.setSayType( ISUNIGNORABLE );
      toSay.setColour( CYAN );
      toSay.setMessage( handyStr->getString( USER_NOT_EXIST ) );
      user->sendString( toSay );
      return( FALSE );
    }
  }
  else
  {
    online = TRUE;
    killUser( user, userToNuke->getName() );
  }

  fileName.setFileName( USER_DIR, userToNuke->getName(), "" );

  // Here is where all the user's files get toasted.  This is irreversable
  // in *nix systems.
  //
  sprintf( userFile, "%s%s", fileName.getFileName(), USER_EXTENSION );
  unlink( userFile );
  sprintf( userFile, "%s%s", fileName.getFileName(), MACRO_EXTENSION );
  unlink( userFile );
  sprintf( userFile, "%s%s", fileName.getFileName(), PROFILE_EXTENSION );
  unlink( userFile );
  sprintf( userFile, "%s%s", fileName.getFileName(), SMAIL_EXTENSION );
  unlink( userFile );
  sprintf( userFile, "%s%s", fileName.getFileName(), ABBR_EXTENSION );
  unlink( userFile );

  handyStr->setUserName( userToNuke->getName() );
  handyStr->setCrewName( user->getName() );
  toSay.setMessage( handyStr->getString( USER_NUKED ) );
  toSay.setColour( BOLD + CYAN );
  toSay.setSayType( ISBROADCAST );
  broadcastSay( toSay );

  if( online == FALSE )
    delete( userToNuke );

  return( TRUE );
}

boolean Command::passwordUser( UserPtr user, char *password )
{
  Say  aSay;
  char tempBuff[ SAY_LENGTH + 1 ];

  if( strcmp( password, "" ) == 0 )
    // User didn't type anything; give a message.
    //
    strcpy( tempBuff, handyStr->getString( USER_PASSWORD ) );
  else
  {
    splitCommand( password );

    // Must verify the old password ...
    //
    if( strcmp( getCommand(), user->getPassword() ) != 0 )
      return( userMsg( user, PASSWORD_INCORRECT, ISUNIGNORABLE, RED + BOLD ) );

    if( strcmp( getParam(), "" ) == 0 )
      return( userMsg( user, USER_PASSWORD, ISUNIGNORABLE, CYAN ) );

    user->setPassword( getParam() );
    user->save();

    sprintf( tempBuff, "%s%s",
             handyStr->getString( USER_CHANGE_PWD ),
             user->getPassword() );
  }

  aSay.setColour( CYAN );
  aSay.setMessage( tempBuff );
  aSay.setSayType( ISUNIGNORABLE );

  return( user->sendString( aSay ) );
}

boolean Command::pemoteUser( UserPtr user, char *cmd )
{
  return( tellUserType( user, cmd, ISPEMOTE ) );
}

// Put the user in profile edit mode, display the first line number.
//
boolean Command::profileUser( UserPtr user, char *lineNum )
{
  Say  toSay;
  int  lNum = atoi( lineNum ) - 1;   // Add 1 when displaying the number
  char lowerBuff[ 4 + 1 ];

  if( strcmp( lineNum, "" ) == 0 )
  {
    // This will shut off all incoming messages except ISUNIGNORABLEs.
    //
    user->setEditing( TRUE );

    userMsg( user, LONG_LINE_WARNING, ISUNIGNORABLE, CYAN + BOLD );
    return( userMsgStr( user, "Line 1:", ISUNIGNORABLE, CYAN ) );
  }

  strncpy( lowerBuff, lineNum, 4 );
  lowerBuff[ 4 ] = 0;

  if( strncmp( "list", lowerBuff, 4 ) == 0 )
    return( examineUser( user, user, TRUE ) );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  // User has requested to delete lines from her profile
  //
  if( lNum < 0 )
  {
    // -15 goes to 14 (delete from line 14; 0 based)
    //
    lNum = ~atoi( lineNum );

    handyStr->setNumber( user->profile.deleteFrom( lNum ) );
    return( userMsg( user, DEL_PROFILE_LINE, ISUNIGNORABLE, CYAN ) );
  }

  // Since lNum is used by itself, it is important to not to increase its
  // value ...
  //
  if( lNum == user->profile.validateLineNum( lNum ) )
  {
    handyStr->setNumber( lNum + 1 );
    handyStr->setUserName( user->getName() );

    splitCommand( lineNum );

    if( strcmp( getParam(), "" ) == 0 )
    {
      if( strcmp( user->profile.getLine( lNum ), "" ) == 0 )
        return( userMsg( user, BLANK_PROFILE_LINE, ISUNIGNORABLE, CYAN ) );

      toSay.setMessage( handyStr->getString( VIEW_PROFILE_LINE ) );
      user->sendString( toSay );
      toSay.setMessage( user->profile.getLine( lNum ) );
      user->sendString( toSay );
      return( TRUE );
    }

    user->profile.setLine( getParam(), lNum );
    user->saveProfile();

    toSay.setMessage( handyStr->getString( CHANGE_PROFILE_LINE ) );
    user->sendString( toSay );
    toSay.setMessage( user->profile.getLine( lNum ) );
    user->sendString( toSay );
    return( TRUE );
  }

  handyStr->setNumber( lNum );
  return( userMsg( user, INVALID_PROFILE_LINE, ISUNIGNORABLE, CYAN ) );
}

boolean Command::promoteUser( UserPtr user, char *userAlias )
{
  Say     toSay;
  UserPtr userToPromote;
  boolean online = FALSE;

  if( strcmp( userAlias, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  toSay.setColour( CYAN );
  toSay.setSayType( ISUNIGNORABLE );

  if( (userToPromote = getUser( userAlias, &online )) == NULL )
  {
    handyStr->setUserName( userAlias );
    toSay.setMessage( handyStr->getString( USER_NOT_EXIST ) );
    user->sendString( toSay );
    return( FALSE );
  }

  if( userToPromote->getLevel() == getHighLevel() )
  {
    if( online == FALSE )
      delete( userToPromote );

    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );
  }

  userToPromote->setLevel( userToPromote->getLevel() + 1 );

  if( online == FALSE )
  {
    userToPromote->setSaveUser( TRUE );
    delete( userToPromote );
  }

  handyStr->setUserName( userToPromote->getName() );
  handyStr->setLevelName( getLevelName( userToPromote->getLevel() ) );
  handyStr->setCrewName( user->getName() );
  toSay.setMessage( handyStr->getString( PROMOTE ) );
  toSay.setSayType( ISBROADCAST );
  toSay.setColour( GREEN + BOLD );
  return( broadcastSay( toSay ) );
}

boolean Command::privateUser( UserPtr user, char *useless )
{
  RoomPtr room       = user->getRoom();
  ListPtr userList   = NULL;
  UserPtr userInRoom = NULL;
  int     count      = 0,
          hiddenUser = FALSE;
  Say     toSay,
          hiddenUserSay;

  handyStr->setRoomName( room->getName() );
  handyStr->setUserName( user->getName() );
  handyStr->setLevelName( getLevelName( user->getLevel() ) );

  toSay.setSayType( ISFEEDBACK );
  toSay.setColour( CYAN );

  // If the room can not be locked by that user, then don't lock it!
  //
  if( user->getLevel() < room->getLockRank() )
  {
    toSay.setMessage( handyStr->getString( CANNOT_LOCK_ROOM ) );
    user->sendString( toSay );
    return( FALSE );
  }

  userList = room->getUserList();

  // Must be at least two users in the room before it can be locked
  // 0 = one user; 1 = two users
  //
  if( (userList->at( 1 ) == NULL) && (user->getLevel() < 2) )
  {
    toSay.setMessage( handyStr->getString( NEED_MORE_USERS ) );
    user->sendString( toSay );
    return( TRUE );
  }

  room->setLocked( !room->getLocked() );

  if( room->getLocked() == FALSE )
    toSay.setMessage( handyStr->getString( USER_UNLOCKED_ROOM ) );
  else
    toSay.setMessage( handyStr->getString( USER_LOCKED_ROOM ) );

  // Look for hidden users
  //
  while( ((userInRoom = (UserPtr)userList->at( count++ )) != NULL) &&
         !hiddenUser )
    hiddenUser = userInRoom->getState( INVISIBLE );

  handyStr->setRoomName( room->getName() );

  hiddenUserSay.setSayType( ISUNIGNORABLE );
  hiddenUserSay.setColour( CYAN );
  hiddenUserSay.setMessage( handyStr->getString( PRIVATE_USER_INVISIBLE ) );

  count = 0;

  // Tell everybody the room was locked; inform each if there is an
  // invisible user in the room, too.
  //
  while( (userInRoom = (UserPtr)userList->at( count++ )) != NULL )
  {
    userInRoom->sendString( toSay );

    if( hiddenUser == TRUE )
      userInRoom->sendString( hiddenUserSay );
  }

  return( TRUE );
}

// A user is allowed to quit (or forced with killUser)
//
boolean Command::quitUser( UserPtr user, char *useless )
{
  RoomPtr room;
  TTime aTime;

  // Not sure if this is really needed, but it does no harm.
  //
  if( user == NULL )
    return( FALSE );

  room = user->getRoom();

  // If a user "quits" from the Logon room, don't give a logoff message
  //
  // May wish to add an || (user->loggingOn() == FALSE)
  //
  if( room != talker->findRoom( LOGON_ROOM ) )
  {
    Say toSay;

    handyStr->setUserName( user->getName() );
    handyStr->setUserDesc( user->getDescription() );

    // So Bots know who is logging off ...
    //
    toSay.setUserFrom( user->getName() );

    toSay.setMessage( handyStr->getString( USER_EXIT_TALKER ) );
    toSay.setColour( CYAN );
    toSay.setSayType( ISLOGOFF );

    user->setBufferNextSay( FALSE );
    broadcastSay( toSay );
  }

  // Set the user's logout time
  //
  user->setTimeOn( aTime.getCurrentTime() );
  room->removeUser( user );

  // Save the user's data upon logging out (this could be because they
  // were kicked off due to lag)
  //
  delete( user );
  user = NULL;
  return( USER_QUIT );
}

// Display the rank of everybody online
//
boolean Command::rankUser( UserPtr user, char *userAlias )
{
  ListPtr roomList   = NULL,
          userList   = NULL;
  RoomPtr room       = NULL;
  UserPtr userInRoom = NULL;
  Say     toSay;
  int     roomCount  = 0,
          userCount  = 0;
  char    tempBuff[ SCREEN_COLUMNS + 1 ];

  lowerCase( userAlias );

  // Display the header for user list
  // 16 = Alias Length + 1
  //
  sprintf( tempBuff, "%-16s %s",
                     "User Name",
                     "Rank" );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setMessage( tempBuff );
  toSay.setColour( CYAN + BOLD );
  user->sendString( toSay );

  roomList = talker->getRoomList();

  toSay.setColour( CYAN );

  // For all the rooms on the talker ...
  //
  while( (room = (RoomPtr)roomList->at( roomCount++ ) ) != NULL )
  {
    userList  = room->getUserList();
    userCount = 0;

    while( (userInRoom = (UserPtr)userList->at( userCount++ )) != NULL )
    {
      sprintf( tempBuff, "%-16s %s", userInRoom->getName(),
                                     getLevelName( userInRoom->getLevel() ) );

      toSay.setMessage( tempBuff );
      user->sendString( toSay );
    }
  }

  toSay.setMessage( "" );
  return( user->sendString( toSay ) );
}

boolean Command::reviewBufferUser( UserPtr user, char *roomName )
{
  RoomPtr room    = NULL;
  SayPtr  wasSaid = NULL;
  Say     toSay;
  int     count   = 1;
  boolean saysOn;
  ListPtr sayList;

  if( (room = peekRoom( user, roomName )) == NULL )
    return( FALSE );

  if( user->getLevel() < room->getReviewRank() )
    return( userMsg( user, NOT_ENOUGH_ACCESS, ISUNIGNORABLE, CYAN ) );

  if( (room->getLocked() == TRUE) && (user->getRoom() != room) )
  {
    handyStr->setRoomName( room->getName() );
    handyStr->setUserName( user->getName() );
    toSay.setSayType( ISUNIGNORABLE );
    toSay.setColour( CYAN );
    toSay.setMessage( handyStr->getString( ROOM_PRIVATE ) );
    user->sendString( toSay );
    return( FALSE );
  }

  if( (user->getRoom() != room) &&
      (room->getLockRank() != getHighLevel()) )
    return( userMsg( user, USER_CANNOT_REVIEW, ISUNIGNORABLE, CYAN ) );

  sayList = room->getReviewBuffer();
  saysOn  = user->getState( SAYS );

  user->setState( SAYS, TRUE );
  userMsg( user, REVIEW_BEGIN, ISUNIGNORABLE, CYAN + BOLD );

  while( (wasSaid = (SayPtr)sayList->at( sayList->howMany() - count )) != NULL )
  {
    user->sendString( *wasSaid );
    count++;
  }

  userMsg( user, REVIEW_END, ISUNIGNORABLE, CYAN + BOLD );
  user->setState( SAYS, saysOn );

  return( TRUE );
}

// To change the look of the rooms display the user gets when viewing
// all the rooms, change this method.
//
// Since the header and information are tightly bound, there is no
// good reason to make the string here a "handy string."
//
boolean Command::roomsUser( UserPtr user, char *param )
{
  ListPtr roomList = NULL;
  RoomPtr room     = NULL;
  Say     toSay;
  int     count        = 0,
          speed        = getHighLevel(),
          passResult   = -1,      // Result from checking level names
          changeAspect = 0;       // 0 signifies no aspect to change
  boolean done         = FALSE,
          passOne      = TRUE;    // For checking level names

  char tempBuff[ SAY_LENGTH + 1 ],
       levelBuff[ SAY_LENGTH + 1 ];

  if( strcmp( param, "" ) == 0 )
  {
    roomList = talker->getRoomList();

    // Display the header for rooms
    //
    sprintf( tempBuff, "%-16s %-4s %-4s %s",
                       "Room Name",
                       "Msgs",
                       "Open",
                       "Topic" );

    toSay.setSayType( ISUNIGNORABLE );
    toSay.setMessage( tempBuff );
    toSay.setColour( CYAN + BOLD );
    user->sendString( toSay );

    // For all the rooms on the talker ...
    //
    while( (room = (RoomPtr)roomList->at( count++ ) ) != NULL )
      if( user->getLevel() >= room->getViewRank() )
      {
        if( room->getLocked() )
          sprintf( tempBuff, "%-16s %4d %4s ",
                   room->getName(),
                   room->getNumMessages(),
                   "No" );
        else
          sprintf( tempBuff, "%-16s %4d %4s ",
                   room->getName(),
                   room->getNumMessages(),
                   "Yes" );

        strncat( tempBuff, room->getTopic(),
                 user->getCols() - strlen( tempBuff ) );
        toSay.setMessage( tempBuff );
        toSay.setColour( CYAN );
        user->sendString( toSay );
      }

    toSay.setMessage( "" );
    user->sendString( toSay );

    return( TRUE );
  }

  // Changing the room's parameters is reserved for Captain level users
  //
  if( user->getLevel() < getHighLevel() )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  strcpy( tempBuff, param );
  lowerCase( tempBuff );

  // Split the parameter into two parts
  //
  splitCommand( tempBuff );

  room = user->getRoom();

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( CYAN );

  // Show the room's current settings
  //
  if( strncmp( "settings", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    // Hard coded stuff here; easily enough changed, and no really good
    // reason for making these things all constants (they won't be
    // changed often enough; nor would a user want to change them).
    //
    handyStr->setRoomName( room->getName() );
    toSay.setMessage( handyStr->getString( ROOMS_SETTINGS ) );
    toSay.setColour( CYAN + BOLD );
    user->sendString( toSay );

    toSay.setColour( CYAN );
    sprintf( tempBuff, "Access Level  : %s",
                       getLevelName( room->getLevel() ) );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    sprintf( tempBuff, "Lock Level    : %s",
                       getLevelName( room->getLockRank() ) );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    sprintf( tempBuff, "Visible Level : %s",
                       getLevelName( room->getViewRank() ) );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    sprintf( tempBuff, "Read Level    : %s",
                       getLevelName( room->getReadRank() ) );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    sprintf( tempBuff, "Review Level  : %s",
                       getLevelName( room->getReviewRank() ) );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    sprintf( tempBuff, "Warp Level    : %s",
                       getLevelName( room->getWarpRank() ) );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    sprintf( tempBuff, "Wipe Level    : %s",
                       getLevelName( room->getWipeRank() ) );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    sprintf( tempBuff, "Write Level   : %s",
                       getLevelName( room->getWriteRank() ) );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    toSay.setMessage( "" );
    return( user->sendString( toSay ) );
  }
  else if( strncmp( "access", getCommand(), strlen( getCommand() ) ) == 0 )
    changeAspect = 1;
  else if( strncmp( "lock", getCommand(), strlen( getCommand() ) ) == 0 )
    changeAspect = 2;
  else if( strncmp( "visible", getCommand(), strlen( getCommand() ) ) == 0 )
    changeAspect = 3;
  else if( strncmp( "warp", getCommand(), strlen( getCommand() ) ) == 0 )
    changeAspect = 4;
  else if( strncmp( "review", getCommand(), strlen( getCommand() ) ) == 0 )
    changeAspect = 5;
  else if( strncmp( "read", getCommand(), strlen( getCommand() ) ) == 0 )
    changeAspect = 6;
  else if( strncmp( "wipe", getCommand(), strlen( getCommand() ) ) == 0 )
    changeAspect = 7;
  else if( strncmp( "write", getCommand(), strlen( getCommand() ) ) == 0 )
    changeAspect = 8;
  else
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  if( strcmp( "", getParam() ) == 0 )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  strncpy( tempBuff, getParam(), LEVEL_NAME_LENGTH );
  tempBuff[ LEVEL_NAME_LENGTH ] = 0;
  lowerCase( tempBuff );

  handyStr->setRoomName( room->getName() );

  while( (count <= speed) && (done == FALSE) )
  {
    strcpy( levelBuff, getLevelName( count ) );
    handyStr->setLevelName( levelBuff );
    lowerCase( levelBuff );

    if( passOne )
      passResult = strcmp( levelBuff, tempBuff );
    else
      passResult = strncmp( levelBuff, tempBuff, strlen( tempBuff ) );

    if( passResult == 0 )
    {
      done = TRUE;
      toSay.setSayType( ISFEEDBACK );

      switch( changeAspect )
      {
        case 1:
          toSay.setMessage( handyStr->getString( ROOM_LEVEL ) );
          room->setLevel( count );
          break;

        case 2:
          toSay.setMessage( handyStr->getString( ROOM_LOCK_LEVEL ) );
          room->setLockRank( count );
          break;

        case 3:
          toSay.setMessage( handyStr->getString( ROOM_VIEW_LEVEL ) );
          room->setViewRank( count );
          break;

        case 4:
          toSay.setMessage( handyStr->getString( ROOM_WARP_LEVEL ) );
          room->setWarpRank( count );
          break;

        case 5:
          toSay.setMessage( handyStr->getString( ROOM_REVIEW_LEVEL ) );
          room->setReviewRank( count );
          break;

        case 6:
          toSay.setMessage( handyStr->getString( ROOM_READ_LEVEL ) );
          room->setReadRank( count );
          break;

        case 7:
          toSay.setMessage( handyStr->getString( ROOM_WIPE_LEVEL ) );
          room->setWipeRank( count );
          break;

        case 8:
          toSay.setMessage( handyStr->getString( ROOM_WRITE_LEVEL ) );
          room->setWriteRank( count );
          break;

        default:
          done = FALSE;
      }
    }

    count++;

    // For checking "Dragon" against "Dragon" & "Dragon Rider" (so to speak)
    //
    if( (passOne == TRUE) && (count > speed) )
    {
      passOne = FALSE;
      count = 0;
    }
  }

  if( !done )
    return( userMsg( user, INVALID_COMMAND, ISUNIGNORABLE, CYAN ) );

  room->save();
  return( user->sendString( toSay ) );
}

boolean Command::sayCrew( UserPtr user, char *cmd )
{
  return( sayWizType( user, cmd, ISCREWSAY ) );
}

boolean Command::sayUser( UserPtr user, char *cmd )
{
  return( sayUserType( user, cmd, ISNORMAL ) );
}

// The user has typed: ".set [option] [state1] [state2] ... [state n]"
//
boolean Command::setUser( UserPtr user, char *option )
{
  Say  toSay;
  char tempBuff[ SAY_LENGTH + 1 ],
       savedCase[ SAY_LENGTH + 1 ];

  // If no option was given then display the user's current settings
  //
  if( strcmp( option, "" ) == 0 )
  {
    toSay.setSayType( ISUNIGNORABLE );

    // Most of this code is implementation specific.  Different Talkers
    // might want to totally rewrite how this stuff gets displayed
    //
    toSay.setMessage( handyStr->getString( USER_SETTINGS ) );
    toSay.setColour( CYAN + BOLD );
    user->sendString( toSay );

    toSay.setColour( CYAN );

    sprintf( tempBuff, "Full alias   : %s %s",
                       user->getName(),
                       user->getDescription() );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    sprintf( tempBuff, "User Level   : %s", getLevelName( user->getLevel() ) );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    strcpy( tempBuff, "Visible      : " );

    if( user->getState( INVISIBLE ) == FALSE )
      strcat( tempBuff, "Yes" );
    else
      strcat( tempBuff, "No" );

    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    sprintf( tempBuff, "Online from  : %s", user->getSite() );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    if( user->getState( SAYS ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( SAYS_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( TELLS ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( TELLS_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( SHOUTS ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( SHOUTS_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( LOGONS ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( LOGONS_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( HIGHLIGHTS ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( HIGHLIGHTS_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( ANSI ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( COLOUR_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( BEEPS ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( BEEPS_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( BRIEF ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( BRIEF_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( FEEDBACK ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( FEEDBACK_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( PICTURES ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( PICTURES_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( ENEMIES ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( ENEMIES_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( SOS ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( SOS_ON ) );
      user->sendString( toSay );
    }

    if( user->getState( MUZZLED ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( USER_IS_MUZZLED ) );
      user->sendString( toSay );
    }

    if( user->getState( JAILED ) == TRUE )
    {
      toSay.setMessage( handyStr->getString( USER_IS_JAILED ) );
      user->sendString( toSay );
    }

    toSay.setMessage( "" );
    return( user->sendString( toSay ) );
  }

  // Parse out what option the user wants to change to which state
  //
  splitCommand( option );

  strncpy( tempBuff, getCommand(), SAY_LENGTH );
  lowerCase( tempBuff );
  setCommand( tempBuff );

  // Save the case of the homepage or e-mail address
  //
  strcpy( savedCase, option );

  // Allow the user to change the case of her alias
  //
  if( strncmp( "alias", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    char buffAliasFrom[ SAY_LENGTH + 1 ],
         buffAliasTo[ SAY_LENGTH + 1 ];

    toSay.setColour( CYAN );
    toSay.setSayType( ISFEEDBACK );

    strncpy( buffAliasTo, getParam(), COMMON_NAME_LEN );
    strncpy( buffAliasFrom, user->getName(), COMMON_NAME_LEN );

    lowerCase( buffAliasTo );
    lowerCase( buffAliasFrom );

    // If the aliases match each other, then change the alias
    //
    if( strcmp( buffAliasFrom, buffAliasTo ) == 0 )
    {
      user->setName( getParam() );
      handyStr->setUserName( user->getName() );
      toSay.setMessage( handyStr->getString( USER_CHANGED_ALIAS ) );
      return( user->sendString( toSay ) );
    }

    // User didn't specify any alias
    //
    if( strcmp( buffAliasTo, " " ) )
    {
      handyStr->setUserName( user->getName() );
      toSay.setMessage( handyStr->getString( USER_DISPLAY_ALIAS ) );
      return( user->sendString( toSay ) );
    }

    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );
  }

  // Make the parameters lower case
  //
  strcpy( tempBuff, option );
  lowerCase( tempBuff );
  setParam( tempBuff );

  if( strncmp( "normal", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    uState nastyStates = user->getState() &
                         (MUZZLED | BANNED | STUNNED | JAILED);

    user->setState( SAYS | TELLS | SHOUTS | PICTURES | ENEMIES | LOGONS |
                    FIXCR | HIGHLIGHTS | BROADCASTS | CREWCASTS | FEEDBACK |
                    nastyStates );

    userMsg( user, USER_RESET_STATE, ISFEEDBACK, GREEN + BOLD );
    return( TRUE );
  }
  else
  if( setUserOption( user, "beeps", BEEPS, BEEPS_ON, BEEPS_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "tells", TELLS, TELLS_ON, TELLS_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "shouts", SHOUTS, SHOUTS_ON, SHOUTS_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "says", SAYS, SAYS_ON, SAYS_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "invites", INVITES, INVITES_ON, INVITES_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "logons", LOGONS, LOGONS_ON, LOGONS_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "sos", SOS, SOS_ON, SOS_OFF, getWizLevel() ) )
    return( TRUE );
  else
  if( setUserOption( user, "broadcasts", BROADCASTS, BROADCASTS_ON, BROADCASTS_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "pictures", PICTURES, PICTURES_ON, PICTURES_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "monitors", MONITOR, MONITOR_ON, MONITOR_OFF,
                     getWizLevel() ) )
    return( TRUE );
  else
  if( setUserOption( user, "colour", ANSI, COLOUR_ON, COLOUR_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "brief", BRIEF, BRIEF_ON, BRIEF_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "wrap", WORDWRAP, WORDWRAP_ON, WORDWRAP_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "wizcasts", CREWCASTS, CREWCASTS_ON, CREWCASTS_OFF,
      getWizLevel() ) )
    return( TRUE );
  else
  if( setUserOption( user, "fixcrs", FIXCR, CR_ON, CR_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "forward", FORWARDING, FORWARD_ON, FORWARD_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "feedback", FEEDBACK, FEEDBACK_ON, FEEDBACK_OFF ) )
    return( TRUE );
  else
  if( setUserOption( user, "bold", HIGHLIGHTS, HIGHLIGHTS_ON, HIGHLIGHTS_OFF ) )
    return( TRUE );
  else if( strncmp( "rows", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    int rows = atoi( getParam() );

    user->setRows( rows );
    handyStr->setNumber( user->getRows() );
    handyStr->setUserName( user->getName() );
    return( userMsgStr( user, handyStr->getString( USER_SET_ROWS ),
                        ISUNIGNORABLE, CYAN ) );
  }
  else if( strncmp( "columns", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    int cols = atoi( getParam() );

    user->setCols( cols );
    handyStr->setNumber( user->getCols() );
    handyStr->setUserName( user->getName() );
    return( userMsgStr( user, handyStr->getString( USER_SET_COLS ),
                        ISUNIGNORABLE, CYAN ) );
  }
  else if( strncmp( "email", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    toSay.setSayType( ISUNIGNORABLE );
    toSay.setColour( CYAN );

    if( strcmp( savedCase, "" ) == 0 )
    {
      if( strcmp( user->getEmail(), "" ) == 0 )
      {
        toSay.setMessage( handyStr->getString( USER_NO_EMAIL ) );
        return( user->sendString( toSay ) );
      }
      else
        toSay.setMessage( handyStr->getString( USER_VIEW_EMAIL ) );
    }
    else
    {
      toSay.setColour( GREEN );
      toSay.setMessage( handyStr->getString( USER_CHANGE_EMAIL ) );
      user->setEmail( savedCase );
      user->save();
    }

    user->sendString( toSay );

    toSay.setMessage( user->getEmail() );
    return( user->sendString( toSay ) );
  }
  else if( strncmp( "homepage", getCommand(), strlen( getCommand() ) ) == 0 )
  {
    toSay.setSayType( ISUNIGNORABLE );
    toSay.setColour( CYAN );

    if( strcmp( savedCase, "" ) == 0 )
    {
      if( strcmp( user->getHomepage(), "" ) == 0 )
      {
        toSay.setMessage( handyStr->getString( USER_NO_HOMEPAGE ) );
        return( user->sendString( toSay ) );
      }
      else
        toSay.setMessage( handyStr->getString( USER_VIEW_HOMEPAGE ) );
    }
    else
    {
      toSay.setColour( GREEN );
      toSay.setMessage( handyStr->getString( USER_CHANGE_HOMEPAGE ) );
      user->setHomepage( savedCase );
      user->save();
    }

    user->sendString( toSay );

    toSay.setMessage( user->getHomepage() );
    return( user->sendString( toSay ) );
  }

  return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );
}

boolean Command::searchUser( UserPtr user, char *toSearch )
{
  Say     toSay;
  Message message;
  RoomPtr room = user->getRoom();
  char    tempBuff[ SAY_LENGTH + 1 ];

  splitCommand( toSearch );

  // Have to specify something to search for ...
  //
  if( (strcmp( getCommand(), "" ) == 0) && (strcmp( getParam(), "" ) == 0) )
    return( userMsg( user, USER_EMPTY_SEARCH, ISUNIGNORABLE, CYAN ) );

  toSay.setColour( CYAN + BOLD );
  toSay.setSayType( ISUNIGNORABLE );

  strcpy( tempBuff, getCommand() );
  lowerCase( tempBuff );

  if( strncmp( "smail", tempBuff, strlen( tempBuff ) ) == 0 )
  {
    handyStr->setUserName( user->getName() );
    toSay.setMessage( handyStr->getString( USER_SMAIL_BEGIN ) );
    user->sendString( toSay );

    sprintf( tempBuff, "%s%s%s", USER_DIR, user->getName(), SMAIL_EXTENSION );
    message.searchFile( user, getParam(), tempBuff );

    handyStr->setUserName( user->getName() );
    toSay.setMessage( handyStr->getString( USER_SMAIL_END ) );
    return( user->sendString( toSay ) );
  }

  handyStr->setUserName( user->getName() );
  handyStr->setRoomName( room->getName() );
  toSay.setMessage( handyStr->getString( USER_READ_BEGIN ) );
  user->sendString( toSay );

  sprintf( tempBuff, "%s%s%s", ROOM_DIR, room->getName(), ROOM_BOARD_EXT );
  message.searchFile( user, getCommand(), tempBuff );

  handyStr->setUserName( user->getName() );
  handyStr->setRoomName( room->getName() );
  toSay.setMessage( handyStr->getString( USER_READ_END ) );
  return( user->sendString( toSay ) );
}

boolean Command::semoteUser( UserPtr user, char *cmd )
{
  return( shoutUserType( user, cmd, ISSEMOTE ) );
}

boolean Command::shoutUser( UserPtr user, char *cmd )
{
  return( shoutUserType( user, cmd, ISSHOUT ) );
}

boolean Command::shutDown( UserPtr user, char *useless )
{
  Say toSay;

  handyStr->setUserName( user->getName() );
  toSay.setMessage( handyStr->getString( TALKER_SHUTDOWN ) );
  toSay.setColour( MAGENTA + BOLD );
  toSay.setSayType( ISUNIGNORABLE );
  broadcastSay( toSay );

  return( talker->setRunning( FALSE ) );
}

boolean Command::siteUser( UserPtr user, char *search )
{
  Say     toSay;
  ListPtr roomList   = NULL,
          userList   = NULL;
  RoomPtr room       = NULL;
  UserPtr userInRoom = NULL;
  int     roomCount  = 0,
          userCount  = 0;
  char    tempBuff[ SCREEN_COLUMNS + 1 ],
          lowerBuff[ SCREEN_COLUMNS + 1 ];

  lowerCase( search );

  // Display the header for user list
  // 16 = Alias Length + 1
  //
  sprintf( tempBuff, "%-16s %s",
                     "User Name",
                     "Site" );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setMessage( tempBuff );
  toSay.setColour( CYAN + BOLD );
  user->sendString( toSay );

  roomList = talker->getRoomList();

  toSay.setColour( CYAN );

  // For all the rooms on the talker ...
  //
  while( (room = (RoomPtr)roomList->at( roomCount++ ) ) != NULL )
  {
    userList  = room->getUserList();
    userCount = 0;

    while( (userInRoom = (UserPtr)userList->at( userCount++ )) != NULL )
      if( (userInRoom->getState( INVISIBLE ) == FALSE) ||
          (user->getLevel() > userInRoom->getLevel()) ||
          (user->getLevel() > 2) )
      {
        sprintf( tempBuff, "%-16s %s",
                           userInRoom->getName(),
                           userInRoom->getSite() );

        strcpy( lowerBuff, tempBuff );
        lowerCase( lowerBuff );

        // If the user wanted to search for a particular string, only
        // display tempBuffs which contain "search".
        //
        if( ((strcmp( search, "" ) != 0) &&
             (strstr( lowerBuff, search ) != NULL)) ||
             (strcmp( search, "" ) == 0) )
        {
          toSay.setMessage( tempBuff );
          user->sendString( toSay );
        }
      }
  }

  toSay.setMessage( "" );
  user->sendString( toSay );

  return( TRUE );
}

// Send a message only to the wizzes
//
boolean Command::sosUser( UserPtr user, char *message )
{

}

boolean Command::stunUser( UserPtr user, char *userAlias )
{
  UserPtr userToStun;

  if( (userToStun = isMultipleUser( user, userAlias )) == NULL )
    return( FALSE );

  return( userMsgStr( userToStun,
                      "You haven't really been stunned.",
                      ISUNIGNORABLE, CYAN ) );
}

boolean Command::suggestUser( UserPtr user, char *suggest )
{
  FileName fileName;
  Message  message;
  Say      toSay;

  toSay.setColour( BOLD + CYAN );
  toSay.setSayType( ISUNIGNORABLE );

  fileName.setFileName( DATA_DIR, SUGGESTION_BOARD, "" );

  if( strcmp( suggest, "" ) == 0 )
  {
    toSay.setMessage( handyStr->getString( SUGGESTION_BEGIN ) );
    user->sendString( toSay );

    // Read the suggestion board
    //
    message.readSuggest( user, fileName.getFileName() );
    
    toSay.setMessage( handyStr->getString( SUGGESTION_END ) );
    return( user->sendString( toSay ) );
  }

  // Should probably show an error message some day ...
  //
  if( message.suggestMessage( suggest, fileName.getFileName() ) == FALSE )
    return( FALSE );

  userMsg( user, USER_SUGGEST, ISUNIGNORABLE, CYAN );
  return( TRUE );
}

boolean Command::tellUser( UserPtr user, char *cmd )
{
  return( tellUserType( user, cmd, ISTELL ) );
}

boolean Command::timeUser( UserPtr user, char *useless )
{
  Say   toSay;
  TTime aTime;
  char  timeBuff[ MAX_TIME_LENGTH + 1 ];

  aTime.difference( talker->getBootTime(), aTime.getCurrentTime(), timeBuff );

  toSay.setColour( CYAN );
  toSay.setSayType( ISUNIGNORABLE );
  handyStr->setSayMsg( timeBuff );
  toSay.setMessage( handyStr->getString( TALKER_BOOTED ) );
  return( user->displaySay( toSay ) );
}

boolean Command::topicUser( UserPtr user, char *topic )
{
  RoomPtr room = NULL;
  Say     toSay;

  char    tempBuff[ SAY_LENGTH + 1 ];

  room = user->getRoom();

  // Tell the user what the current topic is
  //
  if( strcmp( topic, "" ) == 0 )
  {
    handyStr->setRoomName( room->getName() );
    strcpy( tempBuff, handyStr->getString( CHECK_TOPIC ) );
    strcat( tempBuff, room->getTopic() );

    return( userMsgStr( user, tempBuff, ISUNIGNORABLE, GREEN ) );
  }

  toSay.setSayType( ISTOPIC );
  toSay.setUserFrom( user->getName() );
  toSay.setInvisible( user->getState( INVISIBLE ) );
  toSay.setMessage( topic );
  toSay.setColour( CYAN );

  room->setTopic( topic );

  return( sayToRoom( user, toSay ) );
}

boolean Command::upUser( UserPtr user, char *cmd )
{
  Say     toSay;
  UserPtr userToPromote = NULL;

  if( strcmp( cmd, "" ) == 0 )
    return( userMsg( user, NEED_USER_NAME, ISFEEDBACK, CYAN ) );

  if( (userToPromote = isMultipleUser( user, cmd )) == NULL )
    return( FALSE );

  if( userToPromote->getLevel() != 1 )
  {
    handyStr->setLevelName( getLevelName( 1 ) );
    handyStr->setUserName( user->getName() );
    toSay.setColour( CYAN );
    toSay.setSayType( ISFEEDBACK );
    toSay.setMessage( handyStr->getString( USER_CANNOT_UP ) );
    user->sendString( toSay );
    return( FALSE );
  }

  return( promoteUser( user, cmd ) );
}

boolean Command::versionUser( UserPtr user, char *useless )
{
  userMsgStr( user, TALKER_VERSION, ISUNIGNORABLE, GREEN + BOLD );
  return( TRUE );
}

// Look up the status of another user
//
boolean Command::viewUser( UserPtr user, char *userAlias )
{
  Say     toSay;
  UserPtr userToView = NULL;
  boolean online     = FALSE;
  TTime   aTime;
  char    lastOnBuff[ MAX_TIME_LENGTH + 1 ],
          tempBuff[ SAY_LENGTH + 1 ];

  // No alias means the user wishes to view herself
  //
  if( strcmp( userAlias, "" ) == 0 )
    strcpy( userAlias, user->getName() );

  toSay.setSayType( ISUNIGNORABLE );

  if( (userToView = getExactUser( userAlias, &online )) == NULL )
  {
    handyStr->setUserName( userAlias );
    toSay.setColour( CYAN );
    toSay.setMessage( handyStr->getString( USER_NOT_EXIST ) );
    user->sendString( toSay );
    return( FALSE );
  }

  toSay.setColour( BLUE + BOLD );
  toSay.setMessage( handyStr->getString( PROFILE_START ) );
  user->sendString( toSay );

  toSay.setColour( CYAN );

  sprintf( tempBuff, "%s %s",
                     userToView->getName(),
                     userToView->getDescription() );

  toSay.setMessage( tempBuff );
  user->sendString( toSay );

  if( online == FALSE )
  {
    aTime.difference( userToView->getTimeOn(),
                      aTime.getCurrentTime(),
                      lastOnBuff );
    sprintf( tempBuff, "Last on : %s ago", lastOnBuff );
  }
  else
    strcpy( tempBuff, handyStr->getString( USER_CURRENTLY_ON ) );

  toSay.setMessage( tempBuff );
  user->sendString( toSay );

  // Display site and rank information
  //
  if( user->getLevel() > 2 )
  {
    sprintf( tempBuff, "From    : %s", userToView->getSite() );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );

    sprintf( tempBuff, "Rank    : %s", getLevelName( userToView->getLevel() ) );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );
  }

  if( (userToView->getEmail()[ 0 ] != '!') || (user->getLevel() > 2) )
  {
    sprintf( tempBuff, "E-Mail  : %s", userToView->getEmail() );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );
  }

  if( (userToView->getHomepage()[ 0 ] != '!') || (user->getLevel() > 2) )
  {
    sprintf( tempBuff, "Homepage: %s", userToView->getHomepage() );
    toSay.setMessage( tempBuff );
    user->sendString( toSay );
  }

  userMsg( user, PROFILE_END, ISUNIGNORABLE, BLUE + BOLD );

  // Don't really want to delete a user that's online ...
  //
  if( online == FALSE )
    delete( userToView );

  return( TRUE );
}

boolean Command::readUser( UserPtr user, char *toRead )
{
  FileName fileName;
  Message  message;
  Say      toSay;
  RoomPtr  room = user->getRoom();

  if( user->getLevel() < room->getReadRank() )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  fileName.setFileName( ROOM_DIR, room->getName(), ROOM_BOARD_EXT );

  handyStr->setRoomName( room->getName() );
  handyStr->setUserName( user->getName() );
  toSay.setMessage( handyStr->getString( USER_READ_BEGIN ) );
  toSay.setColour( CYAN + BOLD );
  toSay.setSayType( ISUNIGNORABLE );
  user->sendString( toSay );

  message.readFile( user, toRead, "", fileName.getFileName() );

  handyStr->setRoomName( room->getName() );
  handyStr->setUserName( user->getName() );
  toSay.setMessage( handyStr->getString( USER_READ_END ) );
  return( user->sendString( toSay ) );
}

// To change the look of the who display the user gets when viewing
// who is online, change this method.
//
// Since the header and information are tightly bound, there is no
// good reason to make the string here a "handy string."
//
boolean Command::whoUser( UserPtr user, char *search )
{
  ListPtr roomList   = NULL,
          userList   = NULL;
  RoomPtr room       = NULL;
  UserPtr userInRoom = NULL;
  Say     toSay;
  TTime   aTime;
  time_t  currentTime = aTime.getCurrentTime();

  uint    timeOn    = 0,
          idleTime  = 0;
  int     roomCount = 0,
          hideUsers = 0,
          totalUser = 0,
          userCount = 0;

  char    tempBuff[ SCREEN_COLUMNS + 1 ],
          convertBuff[ NUMBER_LENGTH + 1 ],
          lowerBuff[ SCREEN_COLUMNS + 1 ],
          fullAlias[ SCREEN_COLUMNS + 1 ],
          searchBuff[ SCREEN_COLUMNS + 1 ];

  strncpy( searchBuff, search, SCREEN_COLUMNS );
  searchBuff[ SCREEN_COLUMNS ] = 0;
  lowerCase( searchBuff );

  // Display the header for user list
  // 46 = Alias Length + Description Length + 1
  //
  sprintf( tempBuff, "%c%-46s %-15s %-5s %-5s %-3s",
                     HIGHLIGHT_CHAR,
                     "User Name",
                     "Location",
                     "Time",
                     "Idle",
                     "WAH" );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setMessage( tempBuff );
  toSay.setColour( CYAN );
  user->sendString( toSay );

  roomList = talker->getRoomList();

  // For all the rooms on the talker ...
  //
  while( (room = (RoomPtr)roomList->at( roomCount++ ) ) != NULL )
  {
    userList  = room->getUserList();
    userCount = 0;

    // A Crew may see invisible Passengers & Landlovers
    // A Captain may see everything save invisible Captains
    //
    while( (userInRoom = (UserPtr)userList->at( userCount++ )) != NULL )
    {
      if( userInRoom->getState( INVISIBLE ) == TRUE )
        hideUsers++;

      if( (userInRoom->getState( INVISIBLE ) == FALSE) ||
          (user->getLevel() > userInRoom->getLevel()) )
      {
        // Provide a means to compensate the effects BOLD has on strings
        // AFTER the description.  (See User.sendString())
        //
        sprintf( fullAlias, "%s %s%c", userInRoom->getName(),
                                       userInRoom->getDescription(),
                                       HIGHLIGHT_STOP );

        timeOn   = aTime.getMinDifference( userInRoom->getTimeOn(),
                                           currentTime );
        idleTime = aTime.getMinDifference( userInRoom->getIdleTime(),
                                           currentTime );

        if( user->getLevel() >= room->getViewRank() )
          sprintf( tempBuff, "%-47s %-15s %4u ",
                   fullAlias,
                   room->getName(),
                   timeOn );
        else
          sprintf( tempBuff, "%-47s %-15s %4u ",
                   fullAlias,
                   handyStr->getString( ROOM_INVISIBLE_NAME ),
                   timeOn );

        sprintf( convertBuff, "%5u  ", idleTime );
        strcat( tempBuff, convertBuff );

        if( userInRoom->getEditing() == TRUE )
          strcat( tempBuff, "BSY" );
        else if( userInRoom->getState( ISBOT ) == TRUE )
          strcat( tempBuff, "bot" );
        else
        {
          // W, A, H = Wiz, AFK, Hidden
          // *  !  ~
          //
          if( userInRoom->getLevel() > 2 )
            strcat( tempBuff, "*" );
          else
            strcat( tempBuff, " " );

          if( userInRoom->getAfk() == TRUE )
            strcat( tempBuff, "!" );
          else
            strcat( tempBuff, " " );

          if( userInRoom->getState( INVISIBLE ) == TRUE )
            strcat( tempBuff, "~" );
          else
            strcat( tempBuff, " " );
        }

        strcpy( lowerBuff, tempBuff );
        lowerCase( lowerBuff );

        // If the user wanted to search for a particular string, only
        // display tempBuffs which contain "search".
        //
        if( ((strcmp( searchBuff, "" ) != 0) &&
             (strstr( lowerBuff, searchBuff ) != NULL)) ||
             (strcmp( searchBuff, "" ) == 0) )
        {
          toSay.setMessage( tempBuff );
          user->sendString( toSay );
        }
      }
    }

    totalUser += (userCount - 1);
  }

  sprintf( tempBuff, "^5%d^%s^5%d^",
                     hideUsers,
                     handyStr->getString( WHO_INVISIBLE ),
                     totalUser );
  toSay.setMessage( tempBuff );
  user->sendString( toSay );

  toSay.setMessage( "" );
  user->sendString( toSay );

  return( TRUE );
}

boolean Command::withUser( UserPtr user, char *search )
{
  RoomPtr room = talker->findUserRoom( search );

  if( room != NULL )
    return( whoUser( user, room->getName() ) );

  return( whoUser( user, user->getRoom()->getName() ) );
}

// Display a BIG message to all the users in the room.
//
boolean Command::woohooUser( UserPtr user, char *useless )
{
  ListPtr userList   = user->getRoom()->getUserList();
  UserPtr userInRoom = NULL;
  int     userCount  = 0,
          woohoo     = 0;

  Say     toSay;
  char    *woohooText[ 6 ] = {
   "^7 __      __                 ___ ___              ^1._.",
  "^7/  \\    /  \\^2____   ^3____^7    /   |   \\  ^4____   ^5____^1| |",
  "^7\\   \\/\\/   ^2/  _ \\ ^3/  _ \\^7  /    ~    \\^4/  _ \\ ^5/  _ \\^1 |",
  "^7 \\        ^2(  <_> |^3  <_> )^7 \\    Y    ^4(  <_> ^5|  <_> )^1|",
  "^7  \\__/\\  / ^2\\____/ ^3\\____/^7   \\___|_  / ^4\\____/ ^5\\____/^1__",
  "^7       \\/                        \\/               ^1\\/" };

  toSay.setColour( CYAN );
  toSay.setSayType( ISPICTURE );
  toSay.setUserFrom( user->getName() );

  while( (userInRoom = (UserPtr)userList->at( userCount++ )) != NULL )
    for( woohoo = 0; woohoo < 6; woohoo++ )
    {
      toSay.setMessage( woohooText[ woohoo ] );
      userInRoom->sendString( toSay );
    }

  return( TRUE );
}

boolean Command::writeUser( UserPtr user, char *toWrite )
{
  FileName fileName;
  Message  message;
  Say      toSay;
  RoomPtr  room = user->getRoom();

  if( user->getLevel() < room->getWriteRank() )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  if( strcmp( toWrite, "" ) == 0 )
    return( userMsg( user, USER_WRITE_NOTHING, ISUNIGNORABLE, CYAN ) );

  fileName.setFileName( ROOM_DIR, room->getName(), ROOM_BOARD_EXT );

  // Should probably show an error message some day ...
  //
  if( message.appendMessage( user, toWrite, fileName.getFileName() ) == FALSE )
    return( FALSE );

  toSay.setSayType( ISUNIGNORABLE );
  toSay.setColour( GREEN );
  handyStr->setUserName( user->getName() );
  handyStr->setRoomName( room->getName() );
  toSay.setMessage( handyStr->getString( USER_WRITE_MESSAGE ) );
  sayToRoom( user, toSay );

  room->setNumMessages( room->getNumMessages() + 1 );
  return( TRUE );
}

boolean Command::wipeUser( UserPtr user, char *toWipe )
{
  FileName fileName;
  Message  message;
  Say      toSay;

  RoomPtr room    = user->getRoom();
  int     deleted = 0;
  char    tempBuff[ FILE_NAME_LENGTH + 1 ];

  if( user->getLevel() < room->getWipeRank() )
    return( userMsg( user, DENIED_COMMAND, ISUNIGNORABLE, CYAN ) );

  fileName.setFileName( ROOM_DIR, room->getName(), ROOM_BOARD_EXT );

  if( (deleted = message.removeLines( toWipe, fileName.getFileName() )) == -1 )
    return( userMsg( user, NO_BOARD_MESSAGES, ISFEEDBACK, CYAN ) );

  sprintf( tempBuff, "^5%d^", deleted );
  handyStr->setNumber( tempBuff );
  handyStr->setUserName( user->getName() );
  toSay.setMessage( handyStr->getString( USER_WIPE_MESSAGE ) );
  toSay.setColour( CYAN );
  toSay.setSayType( ISUNIGNORABLE );
  sayToRoom( user, toSay );

  room->setNumMessages( room->getNumMessages() - deleted );
  return( TRUE );
}

