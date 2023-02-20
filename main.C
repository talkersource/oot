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
// This is the main program which handles the interactions of users.
//
/////////////////////////////////////////////////////////////////////////

#include "consts.h"
#include "talker.h"
#include "user.h"
#include "room.h"
#include "handystr.h"
#include "command.h"

void killSignals( void );

int main( int argc, char *argv[] )
{
  TalkerPtr   talker;
  SocketsPtr  sockets;
  CommandPtr  command;
  HandyStrPtr handyStr;

  UserPtr     userLogon,         // For logging on
              user;              // For cycling through all users in a room
  RoomPtr     room;              // For cycling through all the rooms
  ListPtr     roomList,
              userList;

  Say toSay;

  int         roomCount = 0,
              userCount = 0,
              socketVal = 0;     // Result from reading a socket

  char        tempSite[ SITE_LENGTH + 1 ],
              aCommand[ SAY_LENGTH + 1 ];
  boolean     doneCommand = FALSE,
              success     = TRUE,
              stillMore   = FALSE;     // Non-socket input to be parsed?

  talker   = new Talker;
  handyStr = new HandyStr;
  command  = new Command;

  killSignals();

  if( --argc == 1 )
    sockets = new Sockets( atoi( argv[ argc ] ) );
  else
    sockets = new Sockets( TALKER_PORT );

  if( success && (command->initCommands() == FALSE) )
  {
    success = FALSE;
    sprintf( aCommand, "Could not read command and/or levels file" );
  }

  if( success && (sockets->initListen() == FALSE) )
  {
    success = FALSE;
    sprintf( aCommand, "Could not listen on %d", sockets->getPort() );
  }

  if( success && (command->registerHandyStr( handyStr ) == FALSE) )
  {
    // Register the strings with the Command class
    //
    success = FALSE;
    sprintf( aCommand, "Could not find %s%s", DATA_DIR, HANDY_NAME );
  }

  if( success )
  {
    command->registerTalker( talker );
    command->registerCmdWithParser( command );
  }

  // Run in background automatically
  //
  switch( fork() )
  {
    case -1:
      success = FALSE;
      strcpy( aCommand, "Could not fork process" );
      break;

    // Child becomes the server
    //
    case 0:
      break;

    // Parent ... dies
    //
    default:
      sleep( 1 );
      return( TRUE );
  }

  if( success && (talker->getRunning() == FALSE) )
  {
    success = FALSE;
    strcpy( aCommand, "Could not load properly." );
  }

  if( success == FALSE )
  {
    delete talker;
    delete handyStr;
    delete command;
    delete sockets;

    printf( "\nOOT Fatal Error: %s!\n\n", aCommand );
    return( FALSE );
  }

  printf( "\nListening on %d ...\n", sockets->getPort() );

  while( talker->getRunning() )
  {
    // Set up all the sockets for reading
    //
    sockets->reset();

    roomCount = 0;
    roomList  = talker->getRoomList();
    stillMore = FALSE;

    while( (room = (RoomPtr)roomList->at( roomCount++ )) != NULL )
    {
      userList  = room->getUserList();
      userCount = 0;

      while( (user = (UserPtr)userList->at( userCount++ )) != NULL )
      {
        sockets->addSocketToCheck( user->getSocket() );
        stillMore = user->bufferedInput() || stillMore;
      }
    }

    // Execute all commands that are left over from previous socket reads
    //
    if( !stillMore )
      sockets->waitForInput();

    // Is somebody logging on?
    //
    if( sockets->isNewConnection() == TRUE )
    {
      userLogon = new User;

      userLogon->registerHandyStr( handyStr );
      userLogon->registerSocket( sockets );

      // Give the user a new socket; get her site
      //
      userLogon->setSocket( sockets->acceptSocket( tempSite ) );
      userLogon->setSite( tempSite );

      // Make sure it is a valid socket
      //
      if( userLogon->getSocket() != -1 )
      {
        // Move the user into logon limbo (first screen user sees)
        //
        command->moveUser( userLogon, LOGON_ROOM );

        // Prompt the user for her alias
        //
        toSay.setSayType( ISLOGONMSG );
        toSay.setLineFeed( FALSE );
        toSay.setMessage( handyStr->getString( LOGON_PROMPT ) );
        userLogon->sendString( toSay );
      }
      else
        delete userLogon;
    }

    // The room/user lists may change dynamically at runtime
    //
    roomList    = talker->getRoomList();
    roomCount   = 0;
    doneCommand = FALSE;

    // See which user typed something in, then issue the command
    //
    while( ((room = (RoomPtr)roomList->at( roomCount++ )) != NULL) &&
            !doneCommand && talker->getRunning() )
    {
      userList  = room->getUserList();
      userCount = 0;

      while( ((user = (UserPtr)userList->at( userCount++ )) != NULL) &&
              !doneCommand && talker->getRunning() )
      {
        socketVal = FALSE;

        // Read from a socket a string.  The string will be empty if
        // nothing was read, so we need to check if the user had any
        // typing in her buffer that had not been parsed out.
        //
        if( user->bufferedInput() == FALSE )
        {
          socketVal = sockets->readSocket( user->getSocket(), aCommand );

          if( socketVal == -2 )
          {
            command->quitUser( user, "" );
            doneCommand = TRUE;
            user = NULL;

            // Ensure that "user" doesn't get accessed any longer (see next
            // if() statement: user->bufferedInput() ...)
            //
            continue;
          }
        }

        // "user" is never NULL at this point due to the previous "continue"
        // statement.
        //
        if( !doneCommand && ((socketVal == TRUE) || user->bufferedInput()) )
        {
          int temp = FALSE;

          if( user->bufferedInput() )
            temp = TRUE;
          else if( !user->moreSocketInput( aCommand ) )
            temp = TRUE;

          if( temp == TRUE )
          {
            user->getSocketInput( aCommand );

            if( command->issueCommand( user, aCommand ) == USER_QUIT )
              user = NULL;

            doneCommand = TRUE;
          }
        }  // If( socketVal ... )
      }  // While User
    }  // While Room
  }  // While Talker

  delete talker;
  delete handyStr;
  delete command;
  delete sockets;

  return( TRUE );
}

void killSignals( void )
{
  // MUA HA HA HA HA ... Never will it die!  *Reepicheep!*
  //
  signal( SIGILL,  SIG_IGN );
  signal( SIGTRAP, SIG_IGN );
  signal( SIGIOT,  SIG_IGN );
  signal( SIGCONT, SIG_IGN );
  signal( SIGHUP,  SIG_IGN );
  signal( SIGINT,  SIG_IGN );
  signal( SIGQUIT, SIG_IGN );
  signal( SIGABRT, SIG_IGN );
  signal( SIGFPE,  SIG_IGN );
  signal( SIGTERM, SIG_IGN );
  signal( SIGURG,  SIG_IGN );
  signal( SIGPIPE, SIG_IGN );
  signal( SIGTTIN, SIG_IGN );
  signal( SIGTTOU, SIG_IGN );
  signal( SIGXCPU, SIG_IGN );

//  signal( SIGSEGV, SIG_IGN );
//  signal( SIGBUS,  SIG_IGN );
}

