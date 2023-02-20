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

#ifndef T_SAY_H
#define T_SAY_H

#include "consts.h"
#include "listbase.h"
#include "colour.h"

// Each say must have a type defined to it so a user may check to see
// if it is being ignored or not.
//
// ISFEEDBACK should be used for telling the user stuff that a novice
// may need to know but a veteran user wouldn't.  Thus a user should
// have the ability to turn off ISFEEDBACK messages.
//
// Some commands you might assume should be ISUNIGNORABLE might fall
// into the ISFEEDBACK category.
//
enum sayValue
{
  ISNORMAL,     ISTELL,     ISPEMOTE,    ISBROADCAST,   ISSHOUT,
  ISPICTURE,    ISLOGON,    ISLOGOFF,    ISROOMINFO,    ISFEEDBACK,
  ISEMOTE,      ISSEMOTE,   ISATMO,      ISUNIGNORABLE, ISTOPIC,
  ISECHO,       ISCREWSAY,  ISCREWEMOTE, ISMULTITELL,   ISMULTIPEMOTE,
  ISLOGONMSG,   ISINVITE,   ISUSERLIST,  ISROOMLIST,    ISNEWSMAIL,
  ISSOS
};

class Say : public ListBase
{
  private:
    sayValue sayType;
    iColour  colour;
    boolean  invisible,  // TRUE if from an invisible user
             lineFeed,   // TRUE if a \n should be added to the line.
             fromBot;    // TRUE if the message was from a Bot

    char message[ SAY_LENGTH + 1 ],
         userFrom[ MULTIPLE_ALIAS_LEN + 1 ],
         userTo[ MULTIPLE_ALIAS_LEN + 1 ];

  public:
    Say( void );
    Say( char *said, sayValue type );
    Say( char *said, char *user, sayValue type );
    Say( char *said, char *user, sayValue type, iColour c );

    boolean init( char *said, char *user, sayValue type, iColour c );

    boolean  setColour( iColour c );
    iColour  getColour( void );
    boolean  setFromBot( boolean toSet );
    boolean  getFromBot( void );
    boolean  setInvisible( boolean toSet );
    boolean  getInvisible( void );
    boolean  setLineFeed( boolean toSet );
    boolean  getLineFeed( void );
    boolean  setMessage( char *msg );
    char     *getMessage( void );
    boolean  setSayType( sayValue type );
    sayValue getSayType( void );
    boolean  setUserFrom( char *userAlias );
    char     *getUserFrom( void );
    boolean  setUserTo( char *userAliases );
    char     *getUserTo( void );
};

typedef Say *SayPtr;

#endif
