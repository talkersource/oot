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

#ifndef T_TALKER_H
#define T_TALKER_H

#include "consts.h"
#include "user.h"
#include "room.h"
#include "listbase.h"
#include "say.h"
#include "ttime.h"

class Talker
{
  private:
    int  bannedIndex,
         numMacros;

    boolean running;  // FALSE - Talker should shut down
    time_t  bootTime; // When the talker was booted

    List roomList,
         shoutList,
         crewcastList;

    char bannedSites[ MAX_BANNED_SITES + 1 ][ SITE_LENGTH + 1 ],
         macros[ TALKER_MACRO_MAX ][ SAY_LENGTH + 1 ];

  public:
    Talker( void );
   ~Talker( void );

    boolean removeRoomLinks( void );

    boolean addMacro( char *newMacro );
    boolean loadMacros( void );
    char    *getMacroAt( int index );

    boolean setBootTime( time_t newBootTime );
    time_t  getBootTime( void );
    char    *getBannedSite( int siteNum );
    boolean setNumBannedSites( int numSites );
    int     getNumBannedSites( void );
    boolean setNumMacros( int newNum );
    int     getNumMacros( void );
    boolean setRunning( boolean toSet );
    boolean getRunning( void );
    boolean addToCrewcastList( Say aCrewcast );
    ListPtr getCrewcastList( void );
    ListPtr getRoomList( void );
    boolean addToShoutList( Say aShout );
    ListPtr getShoutList( void );

    boolean areAdjoined( RoomPtr roomA, RoomPtr roomB );
    boolean isRoomLoaded( char *roomName );
    boolean isBannedSite( char *site );

    RoomPtr findRoom( char *roomName );
    RoomPtr findRoom( int index );
    UserPtr findUser( char *userName );
    UserPtr findUserExact( char *userName );
    int     findUserMatches( char *userName );
    RoomPtr findUserRoom( char *userName );
    RoomPtr findUserRoom( UserPtr user );

    boolean addBannedSite( char *site );
    boolean loadBannedSites( void );
    boolean saveBannedSites( void );
    boolean unBanSite( char *site );

    boolean addRoom( RoomPtr room );
    boolean addRoomNamed( char *name );
    boolean loadRoomLinks( char *badRoom );

    void printInfo( void );
    void validateFile( char *fName );
    boolean validateFiles( void );
};

typedef Talker *TalkerPtr;

#endif
