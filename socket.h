/////////////////////////////////////////////////////////////////////////
//
/*
   Copyright (C) 1996 by Dave Jarvis and Ken Savage

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

#ifndef T_SOCKETS_H
#define T_SOCKETS_H

#include "consts.h"

// This class will open new sockets to those who want one.  Returns
// the socket on success, FALSE on failure.  Remember to close the
// socket when finished with it.  closeSocket returns FALSE if the
// socket wasn't open.
//
// It may be used to read (non-blocking) on a given socket for data,
// returning the data as a string, or NULL if no data available.
//
// It may be used to write data (non-blocking) on a given socket,
// returning TRUE if successful.
//
class Sockets
{
  private:
    int port,
        on,
        listenSock;

    fd_set readMask;

    struct sockaddr_in bindAddr;

  public:
    Sockets( int portNum );
   ~Sockets( void );

    boolean initListen( void );

    boolean setPort( int portNum );
    int     getPort( void );

    boolean waitForInput( void );
    boolean isNewConnection( void );

    iSocket acceptSocket( char *site );
    boolean addSocketToCheck( iSocket s );
    boolean closeSocket( iSocket s );
    boolean readSocket( iSocket s, char *sockString );
    boolean reset( void );
    boolean writeSocket( iSocket s, char *toWrite );
};

typedef Sockets *SocketsPtr;

#endif
