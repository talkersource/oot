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
// CommandList is a class which encapsulates the list of pointers to
// methods.
//
// CommandParser is a class which encapsulates the loading, indexing,
// retrieving, and so forth, of the CommandList.  It interacts with
// the Command class (see command.C).
//
/////////////////////////////////////////////////////////////////////////

#include "cmdparse.h"
#include "command.h"

    //////               //////
   //                       //
  // Setter/Getter Methods //
 //                       //
//////               //////

void CommandList::setName( char *newName )
{
  strncpy( cmdName, newName, COMMAND_NAME_LEN );
}

char *CommandList::getName( void )
{
  return( cmdName );
}

void CommandList::setLevel( iLevel newLevel )
{
  minUserLevel = newLevel;
}

iLevel CommandList::getLevel( void )
{
  return( minUserLevel );
}

    //////                                               //////
   //                                                       //
  // Class Constructors, Destructors, and Initializations  //
 //                                                       //
//////                                               //////

CommandParser::CommandParser( void )
{
  commandNames[ 0 ] = 0;
}

CommandParser::~CommandParser( void )
{
}

    //////         //////
   //                 //
  // Private Methods //
 //                 //
//////         //////

boolean CommandParser::addCommand( char *name, iLevel userLevel,
        boolean (Command::*method)( UserPtr user, char *cmdLine ) )
{
  char           tempBuff[ COMMAND_NAME_LEN + 1 ];
  CommandListPtr cmdListPtr = new( CommandList );

//  cmdListPtr = new( CommandList );

  // Put a "." to the start of the command
  //
  sprintf( tempBuff, ".%s", name );
  addCommandName( tempBuff );

  cmdListPtr->setName( tempBuff );
  cmdListPtr->setLevel( userLevel );

  cmdListPtr->cmdMethod = method;

  return( cmdList += *cmdListPtr );
}

boolean CommandParser::addCommandName( char *name )
{
  char tempBuff[ (NUMBER_OF_COMMANDS * COMMAND_NAME_LEN) + 1 ];

  // Add said command, plus the appropriate amount of padded spaces
  // to the start of the internal command list.  To be used for "instant"
  // lookup and execution of commands.
  //
  sprintf( tempBuff, "%-*s%s", COMMAND_NAME_LEN, name, commandNames );
  strcpy( commandNames, tempBuff );
  return( TRUE );
}

int CommandParser::whichCommand( char *cmdStr )
{
  int index;
  char *tempBuff;

  tempBuff = strstr( commandNames, cmdStr );

  if( tempBuff == NULL )
    return( -2 );

  // Pointer arithmetic to give us index within cmdName.
  // Note that it is legal ONLY to subtract pointers.
  //
  index = (tempBuff - commandNames) / COMMAND_NAME_LEN;

  return( index );
}

    //////          //////
   //                  //
  // Clean Up Methods //
 //                  //
//////          //////

boolean CommandParser::removeCommands( void )
{
  CommandListPtr cmdListPtr;

  strcpy( commandNames, "" );

  while( (cmdListPtr = (CommandListPtr)cmdList.at( 0 )) != NULL )
  {
    cmdList -= *cmdListPtr;
    delete( cmdListPtr );
  }

  return( TRUE );
}

    //////                 //////
   //                         //
  // Called by other Objects //
 //                         //
//////                 //////

// Given the name of a command, figure out it's level ... This may
// change to look into the commands.dat file and get the name of the
// command, as well.  Make sure commandName is lowercase (for now).
//
iLevel CommandParser::getCommandLevel( char *commandName )
{
  CommandListPtr cmdListPtr = NULL;
  int            cmdCount   = 0;

  // So ... what command did the user try to type, eh?  If it wasn't
  // found, return FALSE to indicate the command may be a macro.
  //
  if( (cmdCount = whichCommand( commandName )) == -2 )
    return( FALSE );

  // cmdCount now indicates which command was issued.
  //
  cmdListPtr = (CommandListPtr)cmdList.at( cmdCount );

  return( cmdListPtr->getLevel() );
}

// Make an easy way to figure out which help file the user requested
// to view.
//
boolean CommandParser::getHelp( char *helpCmd, char *fileName )
{
  CommandListPtr cmdListPtr = NULL;
  int            cmdCount   = 0;
  char           lowerCmd[ COMMAND_LENGTH + 1 ];
 
  // Lower case comparisons on the command that help is wanted on
  //
  sprintf( lowerCmd, ".%s", helpCmd );
  lowerCase( lowerCmd );

  if( (strcmp( lowerCmd, "." ) == 0) ||
      (strncmp( ".commands", lowerCmd, strlen( lowerCmd ) ) == 0) )
  {
    strcpy( fileName, ".commands" );
    return( TRUE );
  }

  // So ... what command did the user try to get help on, eh?
  //
  if( (cmdCount = whichCommand( lowerCmd )) == -2 )
  {
    // If no command was found, try to give help on what the user typed
    //
    strcpy( fileName, lowerCmd );
    return( TRUE );
  }

  // cmdCount now indicates the command name--look it up
  //
  cmdListPtr = (CommandListPtr)cmdList.at( cmdCount );

  strcpy( fileName, cmdListPtr->getName() );
  return( TRUE );
}

boolean CommandParser::issueCommand( UserPtr user, char *cmd, char *param )
{
  CommandListPtr cmdListPtr = NULL;
  int            cmdCount   = 0;
  char           lowerCmd[ BIG_SAY_LENGTH + 1 ];

  // Lower case comparisons on the command that was issued.
  //
  strncpy( lowerCmd, cmd, BIG_SAY_LENGTH );
  lowerCmd[ BIG_SAY_LENGTH ] = 0;
  lowerCase( lowerCmd );

  // So ... what command did the user try to type, eh?  If it wasn't
  // found, return FALSE to indicate the command may be a macro.
  //
  if( (cmdCount = whichCommand( lowerCmd )) == -2 )
    return( FALSE );

  // cmdCount now indicates which command was issued.
  //
  cmdListPtr = (CommandListPtr)cmdList.at( cmdCount );

  // See if the user has enough access to do the command ...
  //
  if( user->getLevel() < cmdListPtr->getLevel() )
    return( -2 );

  // This line here ... yeah, right.  This calls the method associated
  // with the command entered.  Just trust it (we do).  ;-)
  //
  if( (commandPtr->*(cmdListPtr->cmdMethod))( user, param ) == USER_QUIT )
    return( USER_QUIT );

  // The command existed (might not have been successful)
  //
  return( TRUE );
}

    //////                         //////
   //                                 //
  // Registration with other Objects //
 //                                 //
//////                         //////

boolean CommandParser::registerCommand( CommandPtr cmdPtr )
{
  commandPtr = cmdPtr;

  return( TRUE );
}

    //////             //////
   //                     //
  // Command Persistence //
 //                     //
//////             //////

// Here is where the commands get added.
//
boolean CommandParser::loadCommands( void )
{
  FILE   *fp;
  char   tempBuff[ SAY_LENGTH + 1 ],
         *cmdName, *cmdLevel, *cmdMethod,
         *buffPtr;
  Common common;
  int    methodNum = 0,
         cmdLvl    = 0;

  removeCommands();

  sprintf( tempBuff, "%s%s", DATA_DIR, CMDS_FILENAME );

  // This is also a fairly bad thing to happen ...
  //
  if( (fp = fopen( tempBuff, "r" )) == NULL )
    return( FALSE );

  // The method numbers are statically mapped to the commands in
  // memory.  One should never change them ... This may be a bit slow,
  // however it allows users to easily change the name and level of
  // each command.
  //
  while( fgets( tempBuff, SAY_LENGTH, fp ) != NULL )
  {
    // Ignore lines that start with a #, or are blank
    //
    if( (tempBuff[ 0 ] == '#') || (tempBuff[ 0 ] == '\n') )
      continue;

    // Get rid of the newline character
    //
    if( tempBuff[ strlen( tempBuff ) - 1 ] == '\n' )
      tempBuff[ strlen( tempBuff ) - 1 ] = 0;

    lowerCase( tempBuff );

    // Split out the name, level, and method for the command
    //
    cmdName   = common.splitString( tempBuff, '=' );
    cmdLevel  = common.splitString( cmdName, '=' );
    cmdMethod = common.splitString( cmdLevel, '=' );

    cmdLvl    = atoi( cmdLevel );
    methodNum = atoi( cmdMethod );

    // Trim off whitespace for the name of the command (the level will
    // be converted w/atoi(), and the cmdMethod comparison uses strncmp())
    //
    buffPtr = cmdName;

    while( !isspace( *buffPtr ) && (*buffPtr != 0) )
      buffPtr++;

    *buffPtr = 0;

    // I put these on one line to conserve screen space (and to make it
    // easier to edit).  Don't forget the break; at the end of each line!
    //
    switch( methodNum )
    {
      case 620: addCommand( cmdName, cmdLvl, &Command::woohooUser ); break;
      case 610: addCommand( cmdName, cmdLvl, &Command::writeUser ); break;
      case 600: addCommand( cmdName, cmdLvl, &Command::wipeUser ); break;
      case 590: addCommand( cmdName, cmdLvl, &Command::withUser ); break;
      case 580: addCommand( cmdName, cmdLvl, &Command::whoUser ); break;
      case 570: addCommand( cmdName, cmdLvl, &Command::versionUser ); break;
      case 560: addCommand( cmdName, cmdLvl, &Command::examineUser ); break;
      case 550: addCommand( cmdName, cmdLvl, &Command::upUser ); break;
      case 540: addCommand( cmdName, cmdLvl, &Command::topicUser ); break;
      case 535: addCommand( cmdName, cmdLvl, &Command::timeUser ); break;
      case 530: addCommand( cmdName, cmdLvl, &Command::tellUser ); break;
      case 525: addCommand( cmdName, cmdLvl, &Command::suggestUser ); break;
      case 520: addCommand( cmdName, cmdLvl, &Command::stunUser ); break;
      case 515: addCommand( cmdName, cmdLvl, &Command::sosUser ); break;
      case 510: addCommand( cmdName, cmdLvl, &Command::smailUser ); break;
      case 500: addCommand( cmdName, cmdLvl, &Command::siteUser ); break;
      case 490: addCommand( cmdName, cmdLvl, &Command::shutDown ); break;
      case 480: addCommand( cmdName, cmdLvl, &Command::shoutUser ); break;
      case 475: addCommand( cmdName, cmdLvl, &Command::searchUser ); break;
      case 470: addCommand( cmdName, cmdLvl, &Command::semoteUser ); break;
      case 460: addCommand( cmdName, cmdLvl, &Command::setUser ); break;
      case 450: addCommand( cmdName, cmdLvl, &Command::sayUser ); break;
      case 440: addCommand( cmdName, cmdLvl, &Command::roomsUser ); break;
      case 430: addCommand( cmdName, cmdLvl, &Command::rmailUser ); break;
      case 420: addCommand( cmdName, cmdLvl, &Command::rankUser ); break;
      case 410: addCommand( cmdName, cmdLvl, &Command::readUser ); break;
      case 400: addCommand( cmdName, cmdLvl, &Command::reviewBufferUser ); break;
      case 390: addCommand( cmdName, cmdLvl, &Command::quitUser ); break;
      case 380: addCommand( cmdName, cmdLvl, &Command::passwordUser ); break;
      case 370: addCommand( cmdName, cmdLvl, &Command::promoteUser ); break;
      case 360: addCommand( cmdName, cmdLvl, &Command::privateUser ); break;
      case 350: addCommand( cmdName, cmdLvl, &Command::pemoteUser ); break;
      case 340: addCommand( cmdName, cmdLvl, &Command::nukeUser ); break;
      case 330: addCommand( cmdName, cmdLvl, &Command::muzzleUser ); break;
      case 320: addCommand( cmdName, cmdLvl, &Command::moveUser ); break;
      case 310: addCommand( cmdName, cmdLvl, &Command::mapUser ); break;
      case 300: addCommand( cmdName, cmdLvl, &Command::macroUser ); break;
      case 290: addCommand( cmdName, cmdLvl, &Command::loadUser ); break;
      case 280: addCommand( cmdName, cmdLvl, &Command::lookUser ); break;
      case 270: addCommand( cmdName, cmdLvl, &Command::killUser ); break;
      case 265: addCommand( cmdName, cmdLvl, &Command::knockUser ); break;
      case 260: addCommand( cmdName, cmdLvl, &Command::jailUser ); break;
      case 250: addCommand( cmdName, cmdLvl, &Command::joinUser ); break;
      case 240: addCommand( cmdName, cmdLvl, &Command::inviteUser ); break;
      case 230: addCommand( cmdName, cmdLvl, &Command::profileUser ); break;
      case 220: addCommand( cmdName, cmdLvl, &Command::hideUser ); break;
      case 210: addCommand( cmdName, cmdLvl, &Command::helpUser ); break;
      case 200: addCommand( cmdName, cmdLvl, &Command::goUser ); break;
      case 190: addCommand( cmdName, cmdLvl, &Command::freeUser ); break;
      case 180: addCommand( cmdName, cmdLvl, &Command::viewUser ); break;
      case 170: addCommand( cmdName, cmdLvl, &Command::emoteCrew ); break;
      case 160: addCommand( cmdName, cmdLvl, &Command::echoUser ); break;
      case 150: addCommand( cmdName, cmdLvl, &Command::emoteUser ); break;
      case 140: addCommand( cmdName, cmdLvl, &Command::downUser ); break;
      case 130: addCommand( cmdName, cmdLvl, &Command::demoteUser ); break;
      case 120: addCommand( cmdName, cmdLvl, &Command::descUser ); break;
      case 110: addCommand( cmdName, cmdLvl, &Command::sayCrew ); break;
      case 100: addCommand( cmdName, cmdLvl, &Command::cmailUser ); break;
      case 90:  addCommand( cmdName, cmdLvl, &Command::clsUser ); break;
      case 80:  addCommand( cmdName, cmdLvl, &Command::clearBufferUser ); break;
      case 70:  addCommand( cmdName, cmdLvl, &Command::colourUser ); break;
      case 60:  addCommand( cmdName, cmdLvl, &Command::broadcastMsg ); break;
      case 50:  addCommand( cmdName, cmdLvl, &Command::bringUser ); break;
      case 40:  addCommand( cmdName, cmdLvl, &Command::botUser ); break;
      case 30:  addCommand( cmdName, cmdLvl, &Command::bansiteUser ); break;
      case 20:  addCommand( cmdName, cmdLvl, &Command::beepUser ); break;
      case 15:  addCommand( cmdName, cmdLvl, &Command::abbrUser ); break;
      case 10:  addCommand( cmdName, cmdLvl, &Command::afkUser ); break;
    }
  }

  fclose( fp );
  return( TRUE );
}
