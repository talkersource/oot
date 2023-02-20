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

#ifndef T_PROFILE_H
#define T_PROFILE_H

#include "consts.h"

class Profile
{
  private:
    char info[ PROFILE_LINES ][ PROFILE_COLUMNS + 1 ];
    int  numLines;

  public:
    Profile();
   ~Profile();

    boolean clear( void );
    int     validateLineNum( int lineNum );

    int     getNumLines( void );
    boolean setNumLines( int lineNum );

    char    *getLine( int lineNum );
    boolean setLine( char *buffer, int lineNum );
    boolean addLine( char *buffer );
    int deleteFrom( int deleteIndex );
};

#endif
