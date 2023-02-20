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
// The Sockets class encapsulates the cumbersome task of using sockets.
// In effect, it hides reading, writing, and stripping of non-ASCII
// characters.
//
/////////////////////////////////////////////////////////////////////////

#include "socket.h"
#include "user.h"
#include "room.h"

    //////                                               //////
   //                                                       //
  // Class Constructors, Destructors, and Initializations  //
 //                                                       //
//////                                               //////

// Set up TCP listening socket on TALKER_PORT
//
Sockets::Sockets( int portNum )
{
  setPort( portNum );
}

Sockets::~Sockets( void )
{
  closeSocket( listenSock );
}

// Try to initialize a listening socket.  Return TRUE on sucess,
// FALSE if a listen socket couldn't be created, or the port could
// not be bound.
//
boolean Sockets::initListen( void )
{
  int size;

  size = sizeof( struct sockaddr_in );

  // Could a listening socket be opened?
  //
  if( (listenSock = socket( AF_INET, SOCK_STREAM, 0 )) == -1 )
    return( FALSE );

  setsockopt( listenSock,
              SOL_SOCKET,
              SO_REUSEADDR,
              (char *)&on,
              sizeof( on ) );

  bindAddr.sin_family      = AF_INET;
  bindAddr.sin_addr.s_addr = INADDR_ANY;
  bindAddr.sin_port        = htons( getPort() );

  if( bind( listenSock, (struct sockaddr *)&bindAddr, size ) == -1 )
    return( FALSE );

  if( listen( listenSock, 20 ) == -1 )
    return( FALSE );

  // Set socket to non-blocking. Not needed but it does no harm.
  //
  fcntl( listenSock, F_SETFL, O_NDELAY );

  return( TRUE );
}

    //////               //////
   //                       //
  // Setter/Getter Methods //
 //                       //
//////               //////

boolean Sockets::setPort( int portNum )
{
  port = portNum;

  return( TRUE );
}

int Sockets::getPort( void )
{
  return( port );
}

    //////       //////
   //               //
  // Query Methods //
 //               //
//////       //////

// Return TRUE if there is input waiting to be read
//
boolean Sockets::waitForInput( void )
{
  return( select( FD_SETSIZE, &readMask, 0, 0, 0 ) > 0 );
}

// Return TRUE if a new connection is waiting to be accepted on the
// listening socket.  The select() function must be called before this
// method will return proper values.
//
boolean Sockets::isNewConnection( void )
{
  return( FD_ISSET( listenSock, &readMask ) > 0 );
}

    //////              //////
   //                      //
  // Socket Comm. Methods //
 //                      //
//////              //////

// Returns a new socket (always check "isNewConnection" first).
// Also "returns" the site the user is coming in from
//
iSocket Sockets::acceptSocket( char *site )
{
#ifndef __linux__
  struct sockaddr_in accAddr;
  struct hostent *host;
  int    accSocket,
         size = sizeof( struct sockaddr_in );
  char   siteIP[ SITE_LENGTH + 1 ];

  uint   addr;

  accSocket = accept( listenSock, (struct sockaddr *)&accAddr, &size );

  strcpy( siteIP, inet_ntoa( accAddr.sin_addr ) );

  // See if the IP has a name address
  //
  addr = inet_addr( siteIP );

  if( (host = gethostbyaddr( (char *)&addr, 4, AF_INET ) ) )
    strncpy( site, host->h_name, SITE_LENGTH );
  else
    strcpy( site, siteIP );
#else
#define uchar unsigned char

  // Something goes wrong with inet_ntoa(...) on Linux, so this hack
  // figures out the IP address for itself.  Future version may try and
  // figure out the name of the machine, but for now ...
  //
  struct in_addr *accAddr;
  struct sockaddr addr;
  int    accSocket = -1,
         len       = sizeof( struct sockaddr );
  char   siteIP[ SITE_LENGTH + 1 ];

  accSocket = accept( listenSock, &addr, &len );
  accAddr   = &(((struct sockaddr_in *)&addr)->sin_addr);

  memcpy( siteIP, (char *)accAddr, len );
  sprintf( site, "%d.%d.%d.%d", (uchar)siteIP[ 0 ], (uchar)siteIP[ 1 ],
                                (uchar)siteIP[ 2 ], (uchar)siteIP[ 3 ] );
#endif

  return( (accSocket >= 0) ? accSocket : -1 );
}

boolean Sockets::addSocketToCheck( iSocket s )
{
  FD_SET( s, &readMask );
}

boolean Sockets::closeSocket( iSocket s )
{
  return( close( s ) != -1 );
}

// Returns TRUE if something was actually read
//
boolean Sockets::readSocket( iSocket s, char *sockString )
{
  int len       = 0,
      count     = 0,
      tempCount = 0;

  char input[ SAY_LENGTH + 1 ],
       tempBuff[ SAY_LENGTH + 1 ],
       backspcBuff[ 10 ];

  // Terminate the string, in case nothing was read
  //
  sockString[ 0 ] = 0;

  // See if any data is on the socket
  //
  if( !FD_ISSET( s, &readMask ) )
    return( FALSE );

  // Has client closed socket?
  //
  if( (len = read( s, input, SAY_LENGTH )) < 1 )
    return( (errno == EAGAIN) ? FALSE : -2 );

  input[ len ] = 0;

  // Strip out nasty characters
  //
  while( (count != len) && (count < SAY_LENGTH) )
  {
    // Might want to check telnet commands ...
    //
    //if( input[ count ] == 255 )
    //skip the next two characters, or read them if not enough

    if( (input[ count ] == BACKSPACE) || (input[ count ] == DELETE) )
    {
      sprintf( backspcBuff, "%c %c", input[ count ], input[ count ] );
      writeSocket( s, backspcBuff );
    }
    else if( isprint( input[ count ] ) || isascii( input[ count ] ) )
      if( (input[ count ] > 31) || (input[ count ] == '\n') ||
          (input[ count ] == BEEP_CHAR) )
        tempBuff[ tempCount++ ] = input[ count ];

    count++;
  }

  tempBuff[ tempCount ] = 0;

  // The buffer was read successfully.
  //
  strncpy( sockString, tempBuff, SAY_LENGTH );

  sockString[ tempCount ] = 0;

  return( TRUE );
}

boolean Sockets::reset( void )
{
  FD_ZERO( &readMask );

  FD_SET( listenSock, &readMask );
  return( TRUE );
}

boolean Sockets::writeSocket( iSocket s, char *toWrite )
{
  boolean done = FALSE;
  int     numWritten = 0;

  while( !done )
    if( (numWritten = write( s, toWrite, strlen( toWrite ) )) == -1 )
      done = (errno == EPIPE) || (errno == EBADF);
    else
      done = TRUE;

  return( numWritten != -1 );
}

