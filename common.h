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

#ifndef T_COMMON_H
#define T_COMMON_H

#include "consts.h"
#include "profile.h"

class Common
{
  protected:
    char name[ COMMON_NAME_LEN + 1 ];

  public:
    Profile profile;

  public:
    Common( void );

    boolean setName( char *newName );
    char    *getName( void );
    char    *splitString( char *strToSplit, char splitAt );
};

#endif
