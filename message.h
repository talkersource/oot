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

#ifndef T_MESSAGE_H
#define T_MESSAGE_H

#include "consts.h"
#include "ttime.h"
#include "user.h"
#include "say.h"
#include "filename.h"

class Message : FileName
{
  private:
    boolean parseNumbers( char *lineNumbers, int number );
    boolean appendMessage( UserPtr user, char *msg );
    int     readFile( UserPtr user, char *numbers, char *toSearch );
    int     readSuggest( UserPtr user );
    int     removeLines( char *numbers );
    boolean suggestMessage( char *msg );

  public:
    Message( void );
   ~Message( void );

    boolean appendMessage( UserPtr user, char *msg, char *fName );
    int     readFile( UserPtr user, char *numbers, char *toSearch, char *fName );
    int     readSuggest( UserPtr user, char *fName );
    int     removeLines( char *numbers, char *fName );
    int     searchFile( UserPtr user, char *toSearch, char *fName );
    boolean suggestMessage( char *msg, char *fName );
};

#endif
