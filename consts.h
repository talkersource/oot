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
// Most of the constants that OOT uses are defined in this file.
//
/////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <String.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>

#ifndef T_CONSTS_H
#define T_CONSTS_H

#ifndef RAND_MAX
#define RAND_MAX        2147483647
#endif

#define TALKER_VERSION    "OOT v1.0 is Copyright (C) 1996 by Dave Jarvis and Ken Savage."

#define FALSE            0
#define TRUE             !FALSE

// Since "boolean" values can sometimes return -2 (for MAJOR wrong-o's)
// we do a bit of substitution.  (Clever hack--some compilers have
// defined boolean themselves.)
//
#define boolean          int

typedef int              iLevel;
typedef int              iSocket;
typedef unsigned int     uint;
typedef unsigned long    ulong;
typedef int              iColour;
typedef ulong            uState;
typedef uint             uSay;

#define SCREEN_COLUMNS    80    // Default user columns
#define SCREEN_ROWS       24    // Default user rows

#define PROFILE_LINES      30   // Max lines for a profile (room/user)
#define PROFILE_COLUMNS   180   // Max columns for a profile (room/user)
#define NOT_EDITING       -1    // User isn't editing profile
#define USER_QUIT         -3    // User quit/disconnected from the talker

#define TALKER_PORT       5000  // Default talker port

#define REVIEW_BUFFER_LEN 15    // Maximum says a room may buffer
#define MAX_PRIVATE_TELLS 15    // Maximum tells a user may buffer
#define MAX_SHOUTS        15    // Maximum shouts the Talker remembers
#define MAX_CREWCASTS     15    // Maximum crewcasts the Talker remembers

#define NO_MATCHES         0    // Given alias didn't match any user

// Maximum string lengths here
//
#define ABBR_LENGTH          2   // Length of an abbreviation (plus space)
#define AFK_LENGTH         300   // Length of user's .afk message
#define BAN_SITE_MIN_LEN     5   // Can't ban things like site ".com"
#define COMMAND_NAME_LEN    20
#define COMMON_NAME_LEN     15   // Length of name for room/user
#define DESCRIPTION_LEN     30   // Max. length of a user's description
#define EMAIL_LENGTH        65   // Length of a user's E-mail address
#define HOMEPAGE_LENGTH     65   // Length of a user's homepage URL
#define LEVEL_NAME_LENGTH   15   // For the name of a level (e.g., Newbie)
#define MACRO_LENGTH       320   // Max. length of a macro
#define MIN_ALIAS_LENGTH     3
#define MIN_PASSWORD_LEN     3
#define SAY_LENGTH         320   // Max. length of a say/shout/tell/etc.
#define SITE_LENGTH         60   // Name/IP of user's site
#define PASSWORD_LENGTH     15
#define TOPIC_LENGTH        60

#define NUMBER_OF_COMMANDS  75   // Number of commands OOT knows

#define MAX_BANNED_SITES   100
#define WIZ_LEVEL            3   // Level required to be a Wizard
#define MAX_LEVEL           10   // Maximum user levels (10 = God)

// A command + its parameters will be truncated if more than
// COMMAND_LENGTH.  By default, if a command starts with a number
// or letter then a ".say " will be prepended.
//
#define COMMAND_LENGTH     SAY_LENGTH
#define MULTIPLE_ALIAS_LEN SAY_LENGTH
#define ANSI_COLOUR_LEN     6
#define BIG_SAY_LENGTH     ((SAY_LENGTH * ANSI_COLOUR_LEN) + \
                            (MULTIPLE_ALIAS_LEN * ANSI_COLOUR_LEN))

#define ABBR_MAX            20   // Number of abbreviations per user
#define MACRO_MAX           30   // Number of macros per user
#define TALKER_MACRO_MAX   100   // Number of Talker macros

// Elements for linked list; room "locations"
//
#define LOGON_ROOM          0
#define MAIN_ROOM           1
#define JAIL_ROOM           2

// Boolean bit toggles for user states and preferences -- persistent
//
// Check state with: if( user.getState( MUZZLED ) ) { TRUE }
//
#define MUZZLED     0x00000001L  // Can shout?
#define JAILED      0x00000002L  // Arrested?
#define STUNNED     0x00000004L  // Can do any commands?
#define TELLS       0x00000008L  // Ignoring tells?
#define SHOUTS      0x00000010L  // Ignoring shouts?
#define PICTURES    0x00000020L  // Ignoring pictures?
#define ANSI        0x00000040L  // ANSI colour codes?
#define HIGHLIGHTS  0x00000080L  // Highlights on?
#define BRIEF       0x00000100L  // Used for entering rooms quickly!
#define SAYS        0x00000200L  // Ignoring says?
#define ENEMIES     0x00000400L  // Ignoring says/tells on enemy list?
#define INVISIBLE   0x00000800L  // Guess who!
#define LOGONS      0x00001000L  // Ignoring logon & logoffs?
#define BROADCASTS  0x00002000L  // Ignoring ship-wide broadcasts?
#define CREWCASTS   0x00004000L  // Ignoring crew broadcasts?
#define FEEDBACK    0x00008000L  // Ignoring feedback?
#define MONITOR     0x00010000L  // Monitoring other users?
#define NEWSMAIL    0x00020000L  // Has new smail?
#define FRIENDS     0x00040000L  // Being alerted of friends?
#define FIXCR       0x00080000L  // Need carriage return fix?
#define BANNED      0x00100000L  // User is banned from the talker?
#define WORDWRAP    0x00200000L  // User has word wrap on?
#define INVITES     0x00400000L  // User reading .knock requests?
#define FORWARDING  0x00800000L  // Forward smail to E-mail?
#define ISBOT       0x01000000L  // Is the user a Bot?
#define BEEPS       0x02000000L  // User accepting beeps?
#define SOS         0x04000000L  // Listening to SOS messages?

// Some handy dandy constants
//
#define HIGHLIGHT_CHAR   '^'        // For highlighting text
#define HIGHLIGHT_STOP   (char)0xFF // See Command::whoUser
#define MACRO_SUB_CHAR   '*'        // For substituting text in macros
#define BASE_10          10         // For converting numbers

#define BEEP_CHAR        0x07
#define BACKSPACE        0x08
#define TAB              0x09
#define CR               0x0D
#define LF               0x0A
#define ESC              0x1B
#define SPACE            0x20
#define DELETE           0x7F

// Characters which may not be used in a user name
//
#define INVALID_ALIAS_CHARS " !/,^.?><;:*\b\t\\"

// Characters which are used placed flush against a user's alias when
// the user does an emote (e.g., Captain's ship is for everybody!)
//
#define SPECIAL_EMOTE_CHARS ",'?!."

// Maximum characters the string representation of a number is within
// a file (mainly for using fgets to read numbers)
//
#define NUMBER_LENGTH  20

#define FILE_NAME_LENGTH 120       // Maximum name length for any file

// File and directory names
//
#define DATA_DIR         "data/"
#define HELP_DIR         "help/"
#define ROOM_DIR         "rooms/"
#define USER_DIR         "users/"

// The following files are in DATA_DIR
//
#define BAN_FILENAME     "banned"       // Banned site list
#define CMDS_FILENAME    "commands.dat" // List Commands on disk
#define HANDY_NAME       "strings.dat"  // Guess
#define LEVEL_FILENAME   "levels.dat"   // Associate names with levels
#define MACRO_FILENAME   "macros.dat"   // List of Talker Macros
#define MAP_FILENAME     "map.dat"      // Map file
#define ROOM_LINK        "roomlink.dat" // How the rooms are linked
#define SUGGESTION_BOARD "suggest.dat"  // An anonymous suggestion board

#define HELP_EXTENSION    ".hlp"  // On a command's name
#define ROOM_BOARD_EXT    ".msg"  // On a room's name
#define TEMP_EXTENSION    ".tmp"  // For temporary files

#define ABBR_EXTENSION    ".abr"  // On a user's name
#define MACRO_EXTENSION   ".mac"  //   "    "     "
#define PROFILE_EXTENSION ".pro"  //   "    "     "
#define SMAIL_EXTENSION   ".sml"  //   "    "     "
#define USER_EXTENSION    ".dat"  //   "    "     "

#endif
