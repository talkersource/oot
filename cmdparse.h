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
/////////////////////////////////////////////////////////////////////////

#ifndef T_COMMANDPARSE_H
#define T_COMMANDPARSE_H

#include "consts.h"
#include "user.h"

class Command;
class CommandList;
class CommandParser;

typedef Command *CommandPtr;
typedef CommandList *CommandListPtr;
typedef CommandParser *CommandParserPtr;

// This class defines how an actual command is made up.
//
class CommandList : public ListBase
{
  private:
    char   cmdName[ COMMAND_NAME_LEN + 1 ];
    iLevel minUserLevel;

  public:
    void   setName( char *newName );
    char   *getName( void );
    void   setLevel( iLevel newLevel );
    iLevel getLevel( void );

    boolean (Command::*cmdMethod)( UserPtr user, char *cmdLine );
};

class CommandParser
{
  private:
    List cmdList;
    char commandNames[ (NUMBER_OF_COMMANDS * COMMAND_NAME_LEN) + 1 ];

    CommandPtr commandPtr;

  private:
    boolean addCommand( char *name, iLevel userLevel,
            boolean (Command::*method)( UserPtr user, char *cmdLine ) );
    boolean addCommandName( char *name );
    int     whichCommand( char *name );

  public:
    CommandParser( void );
   ~CommandParser( void );

    boolean removeCommands( void );

    iLevel  getCommandLevel( char *commandName );
    boolean getHelp( char *helpCmd, char *fileName );
    boolean issueCommand( UserPtr user, char *cmd, char *param );

    boolean registerCommand( CommandPtr cmdPtr );

    boolean loadCommands( void );
};

#endif
