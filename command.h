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

#ifndef T_COMMAND_H
#define T_COMMAND_H

#include "consts.h"
#include "user.h"
#include "talker.h"
#include "handystr.h"
#include "colour.h"
#include "ttime.h"
#include "cmdparse.h"
#include "say.h"
#include "message.h"
#include "filename.h"

class   Command;
typedef Command *CommandPtr;

class Command
{
  private:
    // Since commands take parameters and parameters can take an
    // arbitrary number of parameters (.set colour on, .jail user),
    // successive calls to "splitCommand" should be used to get at
    // the information.
    //
    // For example, let's say the command is: .set colour on
    // The first call to splitCommand sets userCmd oo ".set" and
    // userParam to "colour on"
    //
    // Since ".set" may take any number of parameters, weeding out the
    // first parameter (colour) is a must.  So another call to the
    // splitCommand method with userParam as the first parameter will
    // set userCmd to "colour" and userParam to "off"
    //
    // Note  : Call parseCommand only ONCE.  parseCommand ensures that
    //   its cmd parameter has a command associated with it, then
    //   immediately calls splitCommand to do the dirty work.
    //
    // Note 2: Don't call splitCommand before any calls to parseCommand
    //   unless you know what you're doing.
    //
    char userCmd[ COMMAND_LENGTH + 1 ],
         userParam[ BIG_SAY_LENGTH + 1 ],
         levelNames[ MAX_LEVEL ][ LEVEL_NAME_LENGTH + 1 ];

    int           highLevel,      // Highest level the talker knows
                  wizLevel;       // Level number assigned to a Wizard
    boolean       recurse;        // TRUE - a macro was used

    CommandParser commandParser;  // Best look at the class itself
    TalkerPtr     talker;         // Handle on the Talker class
    HandyStrPtr   handyStr;       // Handle on the HandyStr class

  public:
    Command( void );

    boolean initCommands( void );

    boolean registerTalker( TalkerPtr t );
    boolean registerHandyStr( HandyStrPtr h );
    boolean registerCmdWithParser( CommandPtr cmdPtr );

    boolean execMacro( UserPtr user, char *macroName, char *macroParam );
    char    *findMacro( UserPtr user, boolean isTalkerMacro, int index );
    boolean issueCommand( UserPtr user, char *cmd );

    int     buildAliasList( ListPtr userList, char *aliasList );
    void    parseCommand( UserPtr user, char *cmd );
    int     parseUserList( UserPtr user, ListPtr list, char *userList,
                           char *theText );
    void    splitCommand( char *cmd );

    boolean setCommand( char *cmd );
    char    *getCommand( void );
    boolean setHighLevel( int newHighLevel );
    int     getHighLevel( void );
    boolean setParam( char *parameters );
    char    *getParam( void );
    boolean setRecurse( boolean toSet );
    boolean getRecurse( void );
    UserPtr getUser( char *alias, boolean *online );
    UserPtr getExactUser( char *alias, boolean *online );
    boolean setWizLevel( iLevel newLevel );
    iLevel  getWizLevel( void );

    boolean isCommandChar( UserPtr user, char ch );
    boolean isInvalidAlias( char *alias );
    boolean isMultipleUser( char *alias );
    UserPtr isMultipleUser( UserPtr user, char *alias );

    boolean addLevel( char *newLevelName );
    char    *getLevelName( int levelToGet );
    boolean loadLevels( void );

    boolean moveUser( UserPtr userToMove, int index );
    boolean moveUser( UserPtr user, char *userAlias, char *roomName );
    boolean moveUser( UserPtr userToMove, RoomPtr roomFrom, RoomPtr roomTo );
    RoomPtr peekRoom( UserPtr user, char *roomName );
    boolean setUserOption( UserPtr user, char *optionStr, uState state,
                           int stateOn, int stateOff, iLevel minOn );
    boolean setUserOption( UserPtr user, char *optionStr, uState state,
                           int stateOn, int stateOff );

    boolean sayUserType( UserPtr user, char *msg, sayValue type );
    boolean sayUserType( UserPtr user, char *msg, sayValue type, boolean toUser );
    boolean sayWizType( UserPtr user, char *msg, sayValue type );
    boolean shoutUserType( UserPtr user, char *msg, sayValue type );
    boolean tellUserType( UserPtr user, char *msg, sayValue type );
    boolean splitMsgCommand( char *toSplit, char **rName,
                             char **rNum, char **rText );

    boolean broadcastSay( Say toSay );
    boolean examineUser( UserPtr user, UserPtr toEx, boolean showLineNum );
    boolean invalidName( UserPtr user, char *name, stringConst usrNum );
    boolean sayToRoom( UserPtr user, Say toSay );
    boolean sayToRoom( UserPtr user, Say toSay, boolean toUser );
    boolean showAdjoinedRooms( UserPtr user, RoomPtr room );
    boolean showSmail( UserPtr user );
    int     showUsersInRoom( UserPtr user, RoomPtr room );
    boolean userInteractRoom( UserPtr user, RoomPtr room,
                              iLevel l, stringConst usrNum );
    boolean userMsg( UserPtr user, stringConst usrNum,
                     sayValue type, iColour colour );
    boolean userMsgStr( UserPtr user, char *str,
                        sayValue type, iColour colour );

    //////////////////////////////////////
    //                                  //
    //  Command Definitions Start Here  //
    //                                  //
    //////////////////////////////////////

    // They must all follow the same parameter definition in order
    // for parseCommand to call them correctly.

    boolean abbrUser( UserPtr user, char *cmd );
    boolean afkUser( UserPtr user, char *msg );

    boolean bansiteUser( UserPtr user, char *cmd );
    boolean beepUser( UserPtr user, char *msg );
    boolean botUser( UserPtr user, char *botName );
    boolean bringUser( UserPtr user, char *userAlias );
    boolean broadcastMsg( UserPtr useless, char *msg );

    boolean clearBufferUser( UserPtr user, char *useless );
    boolean clsUser( UserPtr user, char *useless );
    boolean colourUser( UserPtr user, char *useless );

    boolean cmailUser( UserPtr user, char *cmd );
    boolean rmailUser( UserPtr user, char *cmd );
    boolean smailUser( UserPtr user, char *cmd );

    boolean demoteUser( UserPtr user, char *userAlias );
    boolean descUser( UserPtr user, char *desc );
    boolean downUser( UserPtr user, char *cmd );

    boolean echoUser( UserPtr user, char *msg );
    boolean emoteCrew( UserPtr user, char *cmd );
    boolean emoteUser( UserPtr user, char *cmd );
    boolean examineUser( UserPtr user, char *userAlias );

    boolean freeUser( UserPtr user, char *userAlias );

    boolean goUser( UserPtr user, char *roomName );

    boolean helpUser( UserPtr user, char *helpOn );
    boolean hideUser( UserPtr user, char *userAlias );

    boolean inviteUser( UserPtr user, char *roomName );

    boolean jailUser( UserPtr user, char *userAlias );
    boolean joinUser( UserPtr user, char *userAlias );

    boolean killUser( UserPtr user, char *userAlias );
    boolean knockUser( UserPtr user, char *room );

    boolean loadUser( UserPtr user, char *useless );
    boolean logonUser( UserPtr user, char *cmd );
    boolean lookUser( UserPtr user, char *roomName );

    boolean macroUser( UserPtr user, char *macro );
    boolean mapUser( UserPtr user, char *useless );
    boolean moveUser( UserPtr userToMove, char *cmd );
    boolean muzzleUser( UserPtr user, char *userAlias );

    boolean nukeUser( UserPtr user, char *userAlias );

    boolean passwordUser( UserPtr user, char *password );
    boolean pemoteUser( UserPtr user, char *cmd );
    boolean profileUser( UserPtr user, char *lineNum );
    boolean promoteUser( UserPtr user, char *userAlias );
    boolean privateUser( UserPtr user, char *useless );

    boolean quitUser( UserPtr user, char *useless );

    boolean rankUser( UserPtr user, char *userAlias );
    boolean reviewBufferUser( UserPtr user, char *roomName );
    boolean restrictUser( UserPtr user, char *siteName );
    boolean roomsUser( UserPtr user, char *param );

    boolean sayCrew( UserPtr user, char *cmd );
    boolean sayUser( UserPtr user, char *cmd );
    boolean searchUser( UserPtr user, char *toSearch );
    boolean setUser( UserPtr user, char *option );
    boolean semoteUser( UserPtr user, char *cmd );
    boolean shoutUser( UserPtr user, char *cmd );
    boolean shutDown( UserPtr user, char *useless );
    boolean siteUser( UserPtr user, char *search );
    boolean sosUser( UserPtr user, char *message );
    boolean stunUser( UserPtr user, char *userAlias );
    boolean suggestUser( UserPtr user, char *suggest );

    boolean tellUser( UserPtr user, char *cmd );
    boolean timeUser( UserPtr user, char *useless );
    boolean topicUser( UserPtr user, char *topic );

    boolean upUser( UserPtr user, char *cmd );

    boolean versionUser( UserPtr user, char *useless );
    boolean viewUser( UserPtr user, char *userAlias );

    boolean whoUser( UserPtr user, char *search );
    boolean withUser( UserPtr user, char *search );
    boolean woohooUser( UserPtr user, char *useless );

    boolean readUser( UserPtr user, char *toRead );
    boolean writeUser( UserPtr user, char *toWrite );
    boolean wipeUser( UserPtr user, char *toWipe );
};

#endif
