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

#ifndef T_ROOM_H
#define T_ROOM_H

#include "consts.h"
#include "filename.h"
#include "user.h"
#include "listbase.h"
#include "cmdparse.h"
#include "filename.h"
#include "common.h"

class   Room;
typedef Room* RoomPtr;

class Room : public Common, public ListBase
{
  private:
    char topic[ TOPIC_LENGTH + 1 ];

    // Levels needed to ...
    //
    iLevel enterRank,   // ... get into the room
           lockRank,    // ... lock the room
           viewRank,    // ... see the room
           warpRank,    // ... warp to it
           reviewRank,  // ... review the buffer
           readRank,    // ... read the room's message board
           writeRank,   // ... write to the room's message board
           wipeRank;    // ... wipe messages from the room's message board

    boolean locked;     // TRUE if locked (default = FALSE)

    int  numMessages;   // Number of messages on the message board

    List attachedRooms; // A list of all rooms accessible from this one

    // Every room has a list of users and a review buffer
    //
    List userList,
         sayList;

  public:
    Room( char *title );
   ~Room( void );

    void    init( char *title );
    int     countMessages( void );

    boolean clearReviewBuffer( void );
    boolean removeAttached( void );

    boolean addAttachedRoom( RoomPtr room );
    ListPtr getAttachedRooms( void );
    boolean setLevel( iLevel l );
    iLevel  getLevel( void );
    boolean setLocked( boolean lockIt );
    boolean getLocked( void );
    boolean setLockRank( iLevel l );
    iLevel  getLockRank( void );
    boolean setNumMessages( int newNumber );
    int     getNumMessages( void );
    boolean setReadRank( iLevel l );
    iLevel  getReadRank( void );
    boolean addToReviewBuffer( Say aSay );
    ListPtr getReviewBuffer( void );
    SayPtr  getReviewBufferAt( int index );
    boolean setReviewRank( iLevel l );
    boolean getReviewRank( void );
    boolean setTopic( char *top );
    char    *getTopic( void );
    ListPtr getUserList( void );
    boolean setViewRank( iLevel l );
    iLevel  getViewRank( void );
    boolean setWarpRank( iLevel l );
    iLevel  getWarpRank( void );
    boolean setWipeRank( iLevel l );
    iLevel  getWipeRank( void );
    boolean setWriteRank( iLevel l );
    iLevel  getWriteRank( void );
    void    addUser( UserPtr user );
    void    removeUser( UserPtr user );

    boolean isAdjoined( RoomPtr room );

    UserPtr findUser( char *userName );
    boolean findUser( UserPtr user );
    int     findUserMatches( char *userName );
    UserPtr findUserExactMatch( char *userName );

    boolean load( void );
    boolean load( char *title );
    boolean save( void );

    int     countUsers( void );
};

#endif
