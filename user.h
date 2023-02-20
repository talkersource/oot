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

#ifndef T_USER_H
#define T_USER_H

#include "consts.h"
#include "filename.h"
#include "socket.h"
#include "listbase.h"
#include "say.h"
#include "handystr.h"
#include "colour.h"
#include "ttime.h"
#include "profile.h"
#include "common.h"

// A user knows which room she's in, and a room knows which users are in
// it, so this works around the recursion.
//
class User;
class Room;

typedef User *UserPtr;
typedef Room *RoomPtr;

class User : public Common, public ListBase
{
  protected:
    char password[ PASSWORD_LENGTH + 1 ],
         description[ DESCRIPTION_LEN + 1 ],
         site[ SITE_LENGTH + 1 ],
         afkMessage[ SAY_LENGTH + 1 ],
         email[ EMAIL_LENGTH + 1 ],
         homepage[ HOMEPAGE_LENGTH + 1 ],
         abbrs[ ABBR_MAX ][ COMMAND_NAME_LEN + ABBR_LENGTH + 1 ],
         macros[ MACRO_MAX ][ MACRO_LENGTH + 1 ];

    String cmdToIssue;

    int editLine,       // Line number that a user is editing
        logonState,     // Can have values from 0..3 (for logging on)
        rows,           // Number of rows on the screen
        cols;           // Number of columns on the screen

    boolean logon,      // TRUE - user is logging on
            editing,    // TRUE - user is editing information
            afk,        // TRUE - user is away from keyboard
            reading,    // TRUE - user is reading a bunch of says
            wrap,       // TRUE - \n's should be placed at end of "cols"
            shouldSave, // TRUE - user data should be saved before deleted
            bufferSays; // TRUE - each call to sendString should be buffered

    time_t timeOn,    // Time the user logged on
           idleTime,  // Has been idle for ...
           lastOn;    // Last time the user logged on

    List tellList,    // List of .tells
         inviteList,  // List of rooms invited to
         bufferedSays;// Buffer for reading lengthy stuff (who, msg board)

    RoomPtr room;     // Room that the user is in

    iLevel      level;
    uState      state;
    iSocket     sock;
    SocketsPtr  socket;
    HandyStrPtr handyStr;

  public:
    User( void );
    User( char *name );
   ~User( void );

    void    init( char *name );
    boolean initMacros( void );
    boolean initAbbrs( void );

    void    secureReset( void );

    boolean clearBufferedSays( void );
    boolean clearTells( void );

    boolean setAfk( boolean toSet );
    boolean getAfk( void );
    boolean setAfkMessage( char *buffer );
    char    *getAfkMessage( void );
    boolean setBufferNextSay( boolean toSet );
    boolean getBufferNextSay( void );
    boolean setCmdToIssue( char *cmdStr );
    String  getCmdToIssue( void );
    boolean setCols( int newCols );
    int     getCols( void );
    boolean setDescription( char *desc );
    char    *getDescription( void );
    void    setEchoOn( void );
    void    setEchoOff( void );
    boolean setEditing( boolean toSet );
    boolean getEditing( void );
    boolean setEditingLine( int lineNum );
    int     getEditingLine( void );
    boolean setEmail( char *newAddy );
    char    *getEmail( void );
    boolean setHomepage( char *newURL );
    char    *getHomepage( void );
    boolean setIdleTime( time_t idledAt );
    time_t  getIdleTime( void );
    boolean setLevel( iLevel l );
    iLevel  getLevel( void );
    boolean setLoggingOn( boolean toSet );
    boolean getLoggingOn( void );
    boolean setLogonState( int newState );
    int     getLogonState( void );
    boolean setPassword( char *pwd );
    char    *getPassword( void );
    boolean setRoom( RoomPtr newRoom );
    RoomPtr getRoom( void );
    boolean setRows( int newRows );
    int     getRows( void );
    boolean setSaveUser( boolean saveStatus );
    boolean getSaveUser( void );
    boolean setSite( char *newSite );
    char    *getSite( void );
    boolean setSocket( iSocket s );
    iSocket getSocket( void );
    boolean setState( uState s );
    boolean setState( uState s, boolean setIt );
    boolean getState( uState s );
    uState  getState( void );
    boolean addToTellList( Say wasSaid );
    ListPtr getTellList( void );
    boolean setTimeOn( time_t startedAt );
    time_t  getTimeOn( void );
    boolean setReading( boolean toSet );
    boolean getReading( void );
    boolean invite( RoomPtr theRoom );
    boolean uninvite( RoomPtr theRoom );

    boolean bufferedInput( void );
    boolean isEnemy( SayPtr aSay );
    boolean isFriend( SayPtr aSay );
    boolean isInvited( RoomPtr theRoom );
    boolean isListening( SayPtr aSay );

    boolean addMacro( char *macro );
    boolean deleteMacro( char *macro );
    char    *getMacroAt( int macroNum );

    boolean addAbbr( char abbr, char *cmdName );
    char    *getAbbr( char abbr );
    char    getAbbr( int index );
    boolean removeAbbr( char abbr );

    boolean registerHandyStr( HandyStrPtr h );
    boolean registerSocket( SocketsPtr sock );

    boolean updateEdit( char *toAdd );

    boolean sendString( Say say );
    boolean displaySays( void );
    int     displaySay( Say say );

    boolean load( void );
    boolean load( char *name );
    boolean loadAbbrs( void );
    boolean loadData( void );
    boolean loadMacros( void );
    boolean loadProfile( void );

    boolean resetAbbrs( void );
    boolean save( void );
    boolean saveAbbrs( void );
    boolean saveMacros( void );
    boolean saveProfile( void );

    boolean getSocketInput( char *tempBuff );
    boolean moreSocketInput( char *input );
};

#endif
