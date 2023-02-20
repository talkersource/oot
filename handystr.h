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

#ifndef T_HANDYSTR_H
#define T_HANDYSTR_H

#include "consts.h"

#define HANDY_STRINGS 300
#define HANDY_STRING_LENGTH 120

// Define a whole _slew_ of constants which have a 1 to 1 mapping with
// the strings in HANDY_NAME.  Only add to the end of HANDY_NAME
// and then make up another constant for the name of that string which
// reflects the comment).
//
// Right now if a distinction is needed between two strings, only make
// "CREW" be special.  If a "USER" and "CREW" can do it, it's a "USER"
// thing.  For example both Crew & Passengers (user) have a SHOUT prefix,
// thus code it as "USER_SHOUTS."  However, Crews & Passangers have
// different exit messages: USER_LEAVES & CREW_LEAVES.
//
// There is no order to them right now; did them as needed.
//
enum stringConst
{
  NEWBIE_DESC = 0,     JAILED_DESC,            ENTRANCE_USER,
  ENTRANCE_CREW,       USER_LEAVES,            CREW_LEAVES,
  USER_BEGIN_MOVED,    USER_KILLED,            USER_AFK_DEFAULT,
  USER_SAYS,           USER_SHOUTS,            LOGON_PROMPT,
  PASSWORD_PROMPT,     NEWBIE_PASSWORD_PROMPT, VERIFY_PASSWORD_PROMPT,
  PASSWORDS_DIFFERENT, PASSWORD_INCORRECT,     NEWBIE_WELCOME,
  INVALID_COMMAND,     DENIED_COMMAND,         USER_TELLS,
  TALKER_SHUTDOWN,     INVALID_ROOM,           INVALID_USER,
  ALREADY_IN_ROOM,     USER_IN_ROOM,           NOT_ADJOINED,
  REVIEW_BEGIN,        REVIEW_END,             SAYS_OFF,
  SAYS_ON,             SHOUTS_OFF,             SHOUTS_ON,
  LOGONS_OFF,          LOGONS_ON,              ROOM_LOCKED,
  ROOM_UNLOCKED,       NEED_MORE_USERS,        ROOM_TOPIC,
  CHECK_TOPIC,         ROOM_INVISIBLE_NAME,    BUFFER_CLEARED,
  ROOM_LEVEL,          ROOM_INVISIBLE_ON,      ROOM_INVISIBLE_OFF,
  USER_CURRENT_DESC,   USER_CHANGED_DESC,      COLOUR_ON,
  COLOUR_OFF,          USERS_IN_ROOM,          NO_OTHER_USERS,
  NO_HELP_AVAILABLE,   ROOM_PRIVATE,           MULTIPLE_USERS,
  USER_IGNORE_TELL,    SUCCESSFUL_COMMAND,     FEEDBACK_ON,
  FEEDBACK_OFF,        ROOMS_SETTINGS,         NEED_USER_NAME,
  NEED_ROOM_NAME,      HIGHLIGHTS_OFF,         HIGHLIGHTS_ON,
  LONG_LINE_WARNING,   PROFILE_SAVED,          PROFILE_UNCHANGED,
  USER_NOT_EXIST,      PROFILE_START,          PROFILE_END,
  ROOM_ENTRANCE,       BRIEF_OFF,              BRIEF_ON,
  PROMOTE,             DEMOTE,                 CREW_NO_KILL,
  USER_CHANGED_ALIAS,  USER_END_MOVED,         INVISIBLE_ALIAS,
  CANNOT_LOCK_ROOM,    USER_BEGIN_INVISIBLE,   USER_END_INVISIBLE,
  USER_SETTINGS,       USER_UNDISTURBABLE,
  SUCCESSFUL_TELL,     SUCCESSFUL_EMOTE,       CR_ON,
  CR_OFF,              MACRO_ADDED_SUCCESSFUL, MACRO_ADDED_FAILED,
  MACRO_DEL_FAILED,    MACRO_DEL_SUCCESSFUL,   CURRENT_MACROS_BEGIN,
  CURRENT_MACROS_END,  RECURSIVE_COMMAND,      STRINGS_LOAD_SUCCESSFUL,
  USER_NOT_AFK,        ROOMS_LOAD_SUCCESSFUL,  TELLS_ON,
  TELLS_OFF,           INVALID_ALIAS,          ENEMIES_ON,
  ENEMIES_OFF,         PICTURES_ON,            PICTURES_OFF,
  USER_RESET_STATE,    BEGIN_REVIEW_TELLS,     END_REVIEW_TELLS,
  BEGIN_VIEW_USER,     END_VIEW_USER,          USER_GRABBED,
  CREW_GRABBED,        USER_MUZZLED,           USER_UNMUZZLED,
  CREW_MUZZLED_USER,   CREW_UNMUZZLED_USER,    USER_EDITING,
  CREWCASTS_ON,        CREWCASTS_OFF,          USER_NUKED,
  ROOM_IS_UNLOCKED,    USER_INVITE,            USER_ALREADY_INVITED,
  INVITED_BEGIN_REVIEW,INVITED_END_REVIEW,     SHOUTS_BEGIN_REVIEW,
  SHOUTS_END_REVIEW,   CREWCAST_BEGIN_REVIEW,  CREWCAST_END_REVIEW,
  MUST_BE_VISIBLE,     USER_INVITED,           USER_NEW_SMAIL,
  USER_RECEIVED_SMAIL, NEED_SMAIL,             USER_SMAIL_BEGIN,
  DELETED_ALL_SMAIL,   DELETE_WHAT_SMAIL,      USER_HAS_SMAIL,
  SMAIL_NOT_FOUND,     NOT_ENOUGH_ACCESS,      MULTI_SUCC_TELL,
  MULTI_SUCC_EMOTE,    BROADCASTS_ON,          BROADCASTS_OFF,
  MONITOR_ON,          MONITOR_OFF,            ECHO_WHAT,
  USER_ENTER_TALKER,   USER_ENTER_END,         USER_EXIT_TALKER,
  USER_EXIT_END,       USER_SMAIL_DELETE,      USER_SMAIL_DEL_END,
  USER_AFK_END,        WHO_INVISIBLE,          PRIVATE_USER_INVISIBLE,
  USER_PASSWORD,       USER_CHANGE_PWD,        BAN_SITE_ADDED,
  BAN_SITE_FAILED,     BAN_SITE_REMOVED,       BAN_SITE_REMOVE_FAILED,
  BAN_LOCALHOST_FAILED,BAN_MINIMUM_NAME,       USER_BANNED,
  MACRO_NEED_MACRO,    PASSWORD_TOO_SHORT,     ADJOINING_ROOMS,
  ROOM_LOCK_LEVEL,     ROOM_VIEW_LEVEL,        ROOM_WARP_LEVEL,
  USER_CANNOT_UP,      USER_CANNOT_DOWN,       USER_IS_MUZZLED,
  NEED_TELL,           USER_JAILED,            USER_CANNOT_GO,
  USER_CANNOT_TELL,    USER_CANNOT_SHOUT,      USER_CANNOT_CREWCAST,
  USER_CANNOT_MSGBOARD,USER_CANNOT_SMAIL,      USER_CANNOT_HELP,
  USER_IS_STUNNED,     USER_IS_JAILED,         ALREADY_JAILED,
  ALREADY_FREE,        USER_FREE,              BANNED_LOAD_SUCCESSFUL,
  BANNED_START,        BANNED_END,             USER_DISPLAY_ALIAS,
  LEVEL_LOAD_SUCCESSFUL,COMMAND_LOAD_SUCCESSFUL,ROOM_REVIEW_LEVEL,
  USER_PRESS_ENTER_Q,  USER_SET_ROWS,          USER_WRITE_NOTHING,
  USER_WIPE_NOTHING,   USER_WRITE_MESSAGE,     USER_WIPE_MESSAGE,
  USER_READ_BEGIN,     USER_READ_END,          ROOM_NUMBER_MESSAGES,
  USER_SMAIL_END,      USER_CANNOT_REVIEW,     USER_LOCKED_ROOM,
  USER_UNLOCKED_ROOM,  USER_NO_SMAIL,          NO_BOARD_MESSAGES,
  REMOTE_ROOM_ENTRANCE,WORDWRAP_ON,            WORDWRAP_OFF,
  LOAD_FAILED,         MACRO_LOAD_SUCCESSFUL,  USER_SET_COLS,
  WIZ_SAYS,            USER_MULTITELLS,        USER_QUIT_LOGON,
  TALKER_BOOTED,       USER_KNOCKS,            INVITES_ON,
  INVITES_OFF,         USER_REQUESTS_INVITE,   FORWARD_ON,
  FORWARD_OFF,         ROOM_LOAD_FAILED,       USER_EMOTE_1,
  USER_EMOTE_2,        USER_SEMOTE_1,          USER_SEMOTE_2,
  USER_PEMOTE_1,       USER_PEMOTE_2,          WIZ_EMOTE_1,
  WIZ_EMOTE_2,         BLANK_PROFILE_LINE,     INVALID_PROFILE_LINE,
  VIEW_PROFILE_LINE,   CHANGE_PROFILE_LINE,    EMPTY_ROOM,
  USER_NO_READ,        USER_NO_WRITE,          USER_NO_WIPE,
  USER_EMPTY_SEARCH,   ROOM_READ_LEVEL,        ROOM_WIPE_LEVEL,
  ROOM_WRITE_LEVEL,    SUGGESTION_BEGIN,       SUGGESTION_END,
  USER_SUGGEST,        BEEPS_ON,               BEEPS_OFF,
  USER_CHANGE_EMAIL,   USER_CHANGE_HOMEPAGE,   USER_VIEW_EMAIL,
  USER_VIEW_HOMEPAGE,  USER_NO_EMAIL,          USER_NO_HOMEPAGE,
  USER_CURRENTLY_ON,   USER_VIEW_ABBR,         USER_CHANGE_ABBR,
  USER_ABBR_INVALID,   USER_ABBR_DELETE,       USER_ABBR_BAD_DELETE,
  USER_SOS_MESSAGE,    SOS_ON,                 SOS_OFF,
  CANNOT_CHANGE_STATE, DEL_PROFILE_LINE
};

// A class to read in a bunch of handy dandy strings.  Think of this
// class as an object whose job is to read and distribute strings to
// any object that wants them.
//
// The user class may well want to support saving these strings in
// a file called "username.str" where "username" is the name of the
// user's alias.  The user will get the predefined strings by default,
// but if any get changed, they can be saved & loaded.
//
class HandyStr
{
  private:
    boolean success;   // TRUE - load worked

    char usefulStr[ HANDY_STRINGS ][ SAY_LENGTH + 1 ],
         tempBuff[ SAY_LENGTH + 1 ],

         userName  [ MULTIPLE_ALIAS_LEN + 1 ],
         roomName  [ COMMON_NAME_LEN    + 1 ],
         levelName [ LEVEL_NAME_LENGTH  + 1 ],
         number    [ NUMBER_LENGTH      + 1 ],
         userDesc  [ DESCRIPTION_LEN    + 1 ],
         crewName  [ MULTIPLE_ALIAS_LEN + 1 ],
         sayMessage[ BIG_SAY_LENGTH     + 1 ];

  private:
    void parseString( char *toParse );

  public:
    HandyStr( void );

    void init( void );

    void reset( void );

    void    setSuccess( boolean worked );
    boolean successful( void );

    boolean setCrewName( char *cName );
    boolean setLevelName( char *lName );
    boolean setNumber( int iNumber );
    boolean setNumber( char *sNumber );
    boolean setRoomName( char *rName );
    boolean setSayMsg( char *sMessage );
    boolean setUserDesc( char *dName );
    boolean setUserName( char *uName );

    boolean load( void );
    boolean load( char *fileName );

    char *getString( int strToGet );
};

typedef HandyStr *HandyStrPtr;

// Shh!  We're cheating here ...
//
void lowerCase( char *str );

#endif
