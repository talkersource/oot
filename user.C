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
// The User Class is where you'll find "displaySay"--the code that
// actually displays (and parses) out the Say class to display its
// contents to the user.
//
// It also stores their information, macros, data, etc. on destruction,
// and loads it on creation.  The Init( ... ) method sets all the
// defaults.
//
/////////////////////////////////////////////////////////////////////////

#include "user.h"
#include "room.h"

    //////                                               //////
   //                                                       //
  // Class Constructors, Destructors, and Initializations  //
 //                                                       //
//////                                               //////

// Must satisfy the linked list by having a void constructor
//
User::User( void )
{
  init( "Unknown User" );
}

User::User( char *name )
{
  init( name );
}

User::~User( void )
{
  // Try to save the user's information
  //
  save();

  // Clean up used memory
  //
  clearTells();
  clearBufferedSays();

  // Close the socket
  //
  socket->closeSocket( getSocket() );
}

void User::init( char *name )
{
  TTime aTime;

  // This is a must.  The rest are for initalization purposes
  //
  setName( name );
  setPassword( "" );
  setSite( "" );
  setEmail( "" );
  setHomepage( "" );

  setSocket( -1 );
  setEditing( FALSE );
  setAfk( FALSE );
  setAfkMessage( "" );
  setReading( FALSE );
  setBufferNextSay( FALSE );

  secureReset();

  // Indicate that the user data is not to be saved (until they log
  // in as a newbie, or oldie)
  //
  setSaveUser( FALSE );

  // Start the logon sequence:
  //   0 = logging on (w/alias)
  //   1 = newbie (w/password)
  //   2 = newbie (w/verification password)
  //   3 = oldie (w/password)
  //
  setLogonState( 0 );
  setLoggingOn( TRUE );

  initMacros();
  initAbbrs();

  // Initialize the socket input
  //
  setCmdToIssue( "" );

  // Just logged on (and just started to idle) now
  //
  setTimeOn( aTime.getCurrentTime() );
  setIdleTime( aTime.getCurrentTime() );
}

boolean User::initMacros( void )
{
  int count = 0;

  while( count < MACRO_MAX )
    macros[ count++ ][ 0 ] = 0;

  return( TRUE );
}

boolean User::initAbbrs( void )
{
  int count = 0;

  while( count < ABBR_MAX )
    abbrs[ count++ ][ 0 ] = 0;

  // Put in the default abbreviations
  //
  addAbbr( ';', ".emote" );
  addAbbr( '/', ".pemote" );
  addAbbr( '&', ".semote" );
  addAbbr( '!', ".shout" );
  addAbbr( ',', ".tell" );
  addAbbr( '>', ".tell" );
//  addAbbr( '}', ".ewizcast" );       // Add these on promotion to Wizard
//  addAbbr( ']', ".wizcast" );
  return( TRUE );
}

// Make the user a Newbie, with all commands, rights, and priviledges given
// to a Newbie.
//
void User::secureReset( void )
{
  // This gets loaded from the strings.dat file after the user logs in
  // (while logging in just give it something nondescript).
  //
  setDescription( "- Unknown Description" );

  setLevel( 1 );
  setRows( SCREEN_ROWS );
  setCols( SCREEN_COLUMNS );

  // Default settings ... blammo!
  //
  setState( SAYS | TELLS | SHOUTS | PICTURES | ENEMIES | LOGONS | INVITES |
            FIXCR | HIGHLIGHTS | BROADCASTS | CREWCASTS | FEEDBACK );
}

    //////          //////
   //                  //
  // Clean Up Methods //
 //                  //
//////          //////

boolean User::clearBufferedSays( void )
{
  SayPtr toSay = NULL;

  while( (toSay = (SayPtr)bufferedSays.at( 0 )) != NULL )
  {
    bufferedSays -= *toSay;
    delete( toSay );
  }

  return( TRUE );
}

boolean User::clearTells( void )
{
  SayPtr aSay = NULL;

  // Get rid of the all tells that was sent
  //
  while( (aSay = (SayPtr)tellList.at( 0 )) != NULL )
  {
    tellList -= *aSay;
    delete( aSay );
  }

  return( TRUE );
}

    //////               //////
   //                       //
  // Setter/Getter Methods //
 //                       //
//////               //////

boolean User::setAfk( boolean toSet )
{
  afk = toSet;
  return( TRUE );
}

boolean User::getAfk( void )
{
  return( afk );
}

boolean User::setAfkMessage( char *buffer )
{
  strncpy( afkMessage, buffer, AFK_LENGTH );
  afkMessage[ AFK_LENGTH ] = 0;
  return( TRUE );
}

char *User::getAfkMessage( void )
{
  return( afkMessage );
}

boolean User::setBufferNextSay( boolean toSet )
{
  bufferSays = toSet;
  return( TRUE );
}

boolean User::getBufferNextSay( void )
{
  return( bufferSays );
}

boolean User::setCmdToIssue( char *cmdStr )
{
  cmdToIssue = cmdStr;
  return( TRUE );
}

String User::getCmdToIssue( void )
{
  return( cmdToIssue );
}

boolean User::setCols( int newCols )
{
  if( (newCols < 80) || (newCols > 255) )
    return( FALSE );

  cols = newCols;
  return( TRUE );
}

int User::getCols( void )
{
  return( cols );
}

boolean User::setDescription( char *desc )
{
  strncpy( description, desc, DESCRIPTION_LEN );
  description[ DESCRIPTION_LEN ] = 0;
  return( TRUE );
}

char *User::getDescription( void )
{
  return( description );
}

void User::setEchoOn( void )
{
  char tempBuff[ SAY_LENGTH + 1 ];

  sprintf( tempBuff, "%c%c%c", 255, 252, 1 );
  socket->writeSocket( getSocket(), tempBuff );
}

void User::setEchoOff( void )
{
  char tempBuff[ SAY_LENGTH + 1 ];

  sprintf( tempBuff, "%c%c%c", 255, 251, 1 );
  socket->writeSocket( getSocket(), tempBuff );
}

boolean User::setEditing( boolean toSet )
{
  if( toSet == TRUE )
    setEditingLine( 0 );
  else
    setEditingLine( NOT_EDITING );

  editing = toSet;
  return( TRUE );
}

boolean User::getEditing( void )
{
  return( editing );
}

boolean User::setEditingLine( int lineNum )
{
  editLine = lineNum;
  return( TRUE );
}

int User::getEditingLine( void )
{
  return( editLine );
}

boolean User::setEmail( char *newAddy )
{
  strncpy( email, newAddy, EMAIL_LENGTH );
  email[ EMAIL_LENGTH ] = 0;
  return( TRUE );
}

char *User::getEmail( void )
{
  return( email );
}

boolean User::setHomepage( char *newURL )
{
  strncpy( homepage, newURL, HOMEPAGE_LENGTH );
  homepage[ HOMEPAGE_LENGTH ] = 0;
  return( TRUE );
}

char *User::getHomepage( void )
{
  return( homepage );
}

boolean User::setIdleTime( time_t idledAt )
{
  idleTime = idledAt;
  return( TRUE );
}

time_t User::getIdleTime( void )
{
  return( idleTime );
}

boolean User::setLevel( iLevel l )
{
  level = l;
  return( TRUE );
}

iLevel User::getLevel( void )
{
  return( level );
}

boolean User::setLoggingOn( boolean toSet )
{
  logon = toSet;
  return( TRUE );
}

boolean User::getLoggingOn( void )
{
  return( logon );
}

boolean User::setLogonState( int newState )
{
  logonState = newState;
  return( TRUE );
}

int User::getLogonState( void )
{
  return( logonState );
}

boolean User::setPassword( char *pwd )
{
  strncpy( password, pwd, PASSWORD_LENGTH );
  password[ PASSWORD_LENGTH ] = 0;
  lowerCase( password );
  return( TRUE );
}

char *User::getPassword( void )
{
  return( password );
}

boolean User::setRoom( RoomPtr newRoom )
{
  room = newRoom;
  return( TRUE );
}

RoomPtr User::getRoom( void )
{
  return( room );
}

boolean User::setRows( int newRows )
{
  if( (newRows < 10) || (newRows > 255) )
    return( FALSE );

  rows = newRows;
  return( TRUE );
}

int User::getRows( void )
{
  return( rows );
}

boolean User::setSaveUser( boolean saveStatus )
{
  shouldSave = saveStatus;
  return( TRUE );
}

boolean User::getSaveUser( void )
{
  return( shouldSave );
}

boolean User::setSite( char *newSite )
{
  strncpy( site, newSite, SITE_LENGTH );
  site[ SITE_LENGTH ] = 0;
  return( TRUE );
}

char *User::getSite( void )
{
  return( site );
}

boolean User::setSocket( iSocket s )
{
  sock = s;
  return( TRUE );
}

iSocket User::getSocket( void )
{
  return( sock );
}

// Blammo!
//
boolean User::setState( uState s )
{
  state = s;
  return( TRUE );
}

// Set the state ON or OFF depending on the
// passed boolean.
//
boolean User::setState( uState s, boolean setIt )
{
  if( setIt == TRUE )
    state |= s;
  else
    state &= ~s;

  return( TRUE );
}

// Returns TRUE if a given state is ON
//
boolean User::getState( uState s )
{
  return( (getState() & s) == s );
}

// Returns the state ... blammo!
//
uState User::getState( void )
{
  return( state );
}

// Add a tell to the user's private tell list
//
boolean User::addToTellList( Say aSay )
{
  SayPtr wasSaid = NULL,
         buffer  = NULL;

  wasSaid = new Say( aSay.getMessage(),
                     aSay.getUserFrom(),
                     aSay.getSayType() );

  wasSaid->setInvisible( aSay.getInvisible() );

  // Get rid of the "first" tell that was sent
  //
  if( tellList.at( MAX_PRIVATE_TELLS ) != NULL )
  {
    buffer = (SayPtr)tellList.at( MAX_PRIVATE_TELLS );
    tellList -= *buffer;
    delete buffer;
  }

  // Add the newest tell to the end of the buffer.
  //
  return( tellList += *wasSaid );
}

ListPtr User::getTellList( void )
{
  return( &tellList );
}

boolean User::setTimeOn( time_t startedAt )
{
  timeOn = startedAt;
  return( TRUE );
}

time_t User::getTimeOn( void )
{
  return( timeOn );
}

boolean User::setReading( boolean toSet )
{
  reading = toSet;
  return( reading );
}

boolean User::getReading( void )
{
  return( reading );
}

boolean User::invite( RoomPtr room )
{
  if( isInvited( room ) )
    return( FALSE );

  inviteList += *room;
  return( TRUE );
}

boolean User::uninvite( RoomPtr room )
{
  inviteList -= *room;
  return( TRUE );
}

    //////       //////
   //               //
  // Query Methods //
 //               //
//////       //////

// Returns TRUE if the user has some buffered input (commands) that have
// yet to be parsed (issued) out.
//
boolean User::bufferedInput( void )
{
  return( cmdToIssue.index( "\n" ) > 0 );
}

boolean User::isEnemy( SayPtr aSay )
{
  return( FALSE );
}

boolean User::isFriend( SayPtr aSay )
{
  return( TRUE );
}

boolean User::isInvited( RoomPtr room )
{
  return( (inviteList.findNode( *room ) == NULL) ? FALSE : TRUE );
}

// Returns TRUE if the user is listening the type of Say that was passed.
// This will eventually take into account isEnemy() and isFriend().
//
boolean User::isListening( SayPtr aSay )
{
  switch( aSay->getSayType() )
  {
    case ISNORMAL:
    case ISEMOTE:
    case ISTOPIC:
    case ISECHO:
      return( getState( SAYS ) );
      break;

    case ISSHOUT:
    case ISSEMOTE:
      return( getState( SHOUTS ) );
      break;

    case ISCREWSAY:
    case ISCREWEMOTE:
      return( getState( CREWCASTS ) );
      break;

    case ISTELL:
    case ISPEMOTE:
    case ISMULTITELL:
    case ISMULTIPEMOTE:
    case ISNEWSMAIL:
      return( getState( TELLS ) );
      break;

    case ISLOGON:
    case ISLOGOFF:
      return( getState( LOGONS ) );
      break;

    case ISINVITE:
      return( getState( INVITES ) );
      break;

    case ISPICTURE:
      return( getState( PICTURES ) );
      break;

    case ISBROADCAST:
      return( getState( BROADCASTS ) );
      break;

    case ISROOMINFO:
      return( getState( BRIEF ) ? FALSE : TRUE );
      break;

    case ISFEEDBACK:
      return( getState( FEEDBACK ) );
      break;

    // Guess the user is listening to it ... (or that type has yet to be
    // defined)
    //
    default:
      return( TRUE );
  }
}

    //////       //////
   //               //
  // Macro Methods //
 //               //
//////       //////

boolean User::addMacro( char *macro )
{
  int count = 0;

  // Changed how macros work ... This could cause problems?
  //
  while( count < MACRO_MAX )
  {
    if( macros[ count ][ 0 ] == 0 )
    {
      strncpy( macros[ count ], macro, MACRO_LENGTH );
      macros[ count ][ MACRO_LENGTH ] = 0;
      return( TRUE );
    }

    count++;
  }

  return( FALSE );
}

boolean User::deleteMacro( char *macro )
{
  int count = 0;

  do
  {
    // All stored macros start with a ".", so the 1 skips over that "."
    //
    if( strncmp( &macros[ count ][ 1 ], macro, strlen( macro ) ) == 0 )
    {
      macros[ count ][ 0 ] = 0;
      break;
    }
  }
  while( ++count != MACRO_MAX );

  return( (count == MACRO_MAX) ? FALSE : TRUE );
}

char *User::getMacroAt( int macroNum )
{
  if( macroNum >= MACRO_MAX )
    return( NULL );

  return( macros[ macroNum ] );
}

    //////              //////
   //                      //
  // Abbreviation Methods //
 //                      //
//////              //////

boolean User::addAbbr( char abbr, char *cmdName )
{
  boolean done  = FALSE;
  int     count = 0;

  // If abbr already exists, then replace it ... otherwise put it into the
  // first blank spot available
  //
  while( count < ABBR_MAX )
  {
    if( (abbrs[ count ][ 0 ] == abbr) || (abbrs[ count ][ 0 ] == 0) )
    {
      sprintf( abbrs[ count ], "%c ", abbr );
      strncat( abbrs[ count ], cmdName, COMMAND_NAME_LEN );
      abbrs[ count ][ COMMAND_NAME_LEN + ABBR_LENGTH ] = 0;
      return( TRUE );
    }

    count++;
  }

  return( FALSE );
}

char *User::getAbbr( char abbr )
{
  int count = 0;

  while( count < ABBR_MAX )
  {
    if( abbrs[ count ][ 0 ] == abbr )
      return( &abbrs[ count ][ 2 ] );

    count++;
  }

  return( NULL );
}

char User::getAbbr( int index )
{
  if( index > ABBR_MAX )
    return( 0 );

  return( abbrs[ index ][ 0 ] );
}

boolean User::removeAbbr( char abbr )
{
  int index = 0;

  while( index < ABBR_MAX )
  {
    if( abbrs[ index ][ 0 ] == abbr )
    {
      abbrs[ index ][ 0 ] = 0;
      return( TRUE );
    }

    index++;
  }

  return( FALSE );
}

    //////                         //////
   //                                 //
  // Registration with other Objects //
 //                                 //
//////                         //////

boolean User::registerSocket( SocketsPtr sock )
{
  socket = sock;
  return( TRUE );
}

boolean User::registerHandyStr( HandyStrPtr h )
{
  handyStr = h;
  return( TRUE );
}

// Add a string in the user's information area.  FALSE indicates that
// the user has finished (or aborted) entering the profile.
//
boolean User::updateEdit( char *toAdd )
{
  Say  toSay;
  char tempBuff[ SCREEN_COLUMNS + 1 ];

  // If the user has just started editing, but leaves a lone '.' on
  // the first line, abort.
  //
  if( (getEditingLine() == 0) && (strcmp( toAdd, "." ) == 0) )
  {
    setEditing( FALSE );

    toSay.setSayType( ISFEEDBACK );
    toSay.setColour( YELLOW + BOLD );
    toSay.setMessage( handyStr->getString( PROFILE_UNCHANGED ) );
    sendString( toSay );

    return( FALSE );
  }

  // If the user has just started editing, then set things up.
  //
  if( getEditingLine() == 0 )
  {
    profile.clear();
    setEditingLine( 0 );
  }

  // If the user didn't type anything, then insert a space.
  //
  if( strcmp( toAdd, "" ) == 0 )
    strcpy( toAdd, " " );

  // If the user typed a dot by itself, then stop editing and save.
  //
  if( (toAdd[ 0 ] == '.') || (toAdd[ 1 ] == 0) )
    setEditingLine( PROFILE_LINES );
  else
  {
    // Insert the line that was typed into the profile.
    //
    profile.addLine( toAdd );
    setEditingLine( getEditingLine() + 1 );
  }

  // If the EditingLine has reached the max then stop editing, return FALSE
  // to indicate that editing has finished
  //
  if( getEditingLine() >= PROFILE_LINES )
  {
    setEditing( FALSE );
    saveProfile();

    toSay.setSayType( ISFEEDBACK );
    toSay.setMessage( handyStr->getString( PROFILE_SAVED ) );
    toSay.setColour( GREEN + BOLD );
    sendString( toSay );
    return( FALSE );
  }

  sprintf( tempBuff, "Line %d:", getEditingLine() + 1 );

  // Display the next line number to edit (1 has already been displayed).
  //
  toSay.setColour( CYAN );
  toSay.setSayType( ISUNIGNORABLE );
  toSay.setMessage( tempBuff );
  return( displaySay( toSay ) );
}

    //////            //////
   //                    //
  // Output to the User //
 //                    //
//////            //////

boolean User::sendString( Say aSay )
{
  SayPtr bufferedSay = NULL;

  if( getReading() || getBufferNextSay() )
  {
    if( (bufferedSay = new Say( aSay.getMessage(), aSay.getUserFrom(),
                                aSay.getSayType(), aSay.getColour() )) == NULL )
    {
      displaySay( aSay );
      return( FALSE );
    }

    bufferedSay->setUserTo( aSay.getUserTo() );
    bufferedSay->setLineFeed( aSay.getLineFeed() );
    bufferedSay->setInvisible( aSay.getInvisible() );

    return( bufferedSays += *bufferedSay );
  }

  return( displaySay( aSay ) > 0 );
}

// Returns TRUE if there are more says to display.
//
boolean User::displaySays( void )
{
  int    count = 0;
  SayPtr toSay = NULL;

  while( ((toSay = (SayPtr)bufferedSays.at( bufferedSays.howMany() - 1 )) != NULL)
         && (count < getRows()) )
  {
    count        += displaySay( *toSay );
    bufferedSays -= *toSay;
    delete( toSay );
  }

  return( bufferedSays.at( 0 ) == NULL ? FALSE : TRUE );
}

// Returns the number of rows that the say took up on the screen.
//
int User::displaySay( Say say )
{
  sayValue sayType;
  Colour   colour;
  char     tempBuff[ BIG_SAY_LENGTH + 1 ],
           colourBuff[ BIG_SAY_LENGTH + 1 ];

  // Should colour/bold be taken out when calculating the number of rows in
  // the Say?
  //
  boolean needToParse = TRUE;

  sayType = say.getSayType();

  // The only thing a user may see when logging on is a room's description
  //
  if( (getLoggingOn() == TRUE) &&
     ((sayType != ISLOGONMSG) && (sayType != ISROOMINFO)) )
    return( FALSE );

  // A user may NOT be disturbed when editing a profile (as it is very
  // annoying).  Except for ISUNIGNORABLE messages, of course.
  //
  if( (getEditing() == TRUE) && (sayType != ISUNIGNORABLE) )
    return( FALSE );

  // If the user is a Bot, send a specially formatted string:
  //   !User !CommandType !Message
  //
  // Note that !User may actually be blank in some cases, yielding:
  //   ! !CommandType !Message
  //
  // Additionally, ONLY PRIVATE TELLS coming in from a Bot get displayed
  // without any formatting.  That is, if a Bot sends a private message to
  // the user Jane, she sees EXACTLY what the Bot sent.  It is up to the Bot
  // to format this in any way appropriate.  It is common courtesy to inform
  // the user somewhere that the text is coming from the Bot.
  //
  // ** THIS IS THE STANDARD OOT (and JOOT) BOT PROTOCOL **
  //
  // The idea is to maintain compatibility across ALL OOT and JOOT talkers.
  // In effect, you write the Bot _ONCE_ and can then be guaranteed that it
  // will function on ALL OTHER OOT (and JOOT) TALKERS.
  //
  // There are several advantages to devising a protocol instead of building
  // a Bot into the talker code itself:
  //
  // 1) Language Independence.  The Bots can be written in any language
  //    that is able to read/write sockets.  (C, C++, Perl, Java, Smalltalk,
  //    and numerous others.)
  //
  // 2) Talker Stability.  Since a Bot is treated as a regular user (for
  //    the most part), if the Bot crashes the corresponding "user" merely
  //    logs out (idles out/socket closes, etc.).  It does not hurt the Talker.
  //
  // 3) No Rebooting.  If the Bot were tightly coupled with the code, you
  //    would need to reboot to instigate the new Bot code.
  //
  // 4) Lots of Bots.  Since the protcol is now public, some people can
  //    focus on Bot development with the knowledge their code will run
  //    on more than just one particular talker.
  //
  // 5) Other Talker Codes.  Not likely, granted, but it would be a boon.
  //
  // 6) Computer Independence.  The Bot (or Bots) that logs on your talker
  //    might actually be on a different computer half-way across the world.
  //    If the Bot is CPU intensive, it doesn't affect the talker.
  //
  // 7) Security.  Since the Sysop must execute the ".bot" command on any
  //    user account that will be a Bot, there is no room for security
  //    breaches.
  //
  // ** PLEASE Do Not Change This Protocol. **
  //
  if( (getState( ISBOT ) == TRUE) && (sayType != ISLOGONMSG) )
  {
    char commandName[ SAY_LENGTH + 1 ];

    // Extract the message part of the command, and strip it of bold/colour
    //
    strcpy( tempBuff, say.getMessage() );
    colour.parseColourOut( tempBuff, colourBuff, FALSE );

    // colourBuff now contains the plaintext message itself; format the string
    // properly.
    //
    switch( sayType )
    {
      case ISNORMAL:
        strcpy( commandName, "say" );
        break;
      case ISTELL:
        strcpy( commandName, "tell" );
        break;
      case ISSHOUT:
        strcpy( commandName, "shout" );
        break;
      case ISCREWSAY:
        strcpy( commandName, "wizcast" );
        break;

      case ISEMOTE:
        strcpy( commandName, "emote" );
        break;
      case ISPEMOTE:
        strcpy( commandName, "pemote" );
        break;
      case ISSEMOTE:
        strcpy( commandName, "semote" );
        break;
      case ISCREWEMOTE:
        strcpy( commandName, "ewizcast" );
        break;

      case ISLOGON:
        say.setUserFrom( say.getUserFrom() );
        strcpy( commandName, "logon" );
        break;
      case ISLOGOFF:
        say.setUserFrom( say.getUserFrom() );
        strcpy( commandName, "logoff" );
        break;

      case ISECHO:
        strcpy( commandName, "echo" );
        break;
      case ISBROADCAST:
        strcpy( commandName, "broadcast" );
        break;
      case ISTOPIC:
        strcpy( commandName, "topic" );
        break;
      case ISNEWSMAIL:
        strcpy( commandName, "smail" );
        break;
      case ISROOMINFO:
        strcpy( commandName, "roominfo" );
        break;

      case ISROOMLIST:
        strcpy( commandName, "roomlist" );
        break;
      case ISUSERLIST:
        strcpy( commandName, "userlist" );
        break;

      case ISUNIGNORABLE:
        strcpy( commandName, "unignorable" );
        break;
      case ISFEEDBACK:
        strcpy( commandName, "feedback" );
        break;

      default:
        strcpy( commandName, "undefined" );
        break;
    }

    // No ANSI.  Just plain text, followed by a newline character.
    //
    sprintf( tempBuff, "!%s !%s !%s\n",
             say.getUserFrom(), commandName, colourBuff );

    // Write the special string out to the socket, and return
    //
    return( socket->writeSocket( getSocket(), tempBuff ) );
  }

  // Ensure that the user is both listening to that specific type of
  // Say, and that whoever said the Say is on/off the enemy/friends list.
  //
  if( isListening( &say ) == FALSE )
    return( FALSE );

  // If the say is supposed to be hidden, but the user is monitoring, then
  // dress it up nicely.  Otherwise make the userFrom to be hidden.
  //
  if( say.getInvisible() == TRUE )
  {
    if( getState( MONITOR ) == TRUE )
    {
      sprintf( tempBuff, "[^%s^] %s",
                         say.getUserFrom(),
                         handyStr->getString( INVISIBLE_ALIAS ) );

      say.setUserFrom( tempBuff );
    }
    else
      say.setUserFrom( handyStr->getString( INVISIBLE_ALIAS ) );
  }

  tempBuff[ 0 ]   = 0;
  colourBuff[ 0 ] = 0;

  handyStr->setUserName( say.getUserFrom() );
  handyStr->setSayMsg( say.getMessage() );

  // Format the say properly, according to its type
  //
  switch( sayType )
  {
    case ISNORMAL:
      strcpy( tempBuff, handyStr->getString( USER_SAYS ) );
      break;

    case ISEMOTE:
      if( strchr( SPECIAL_EMOTE_CHARS, (int)say.getMessage()[ 0 ] ) != NULL )
        strcpy( tempBuff, handyStr->getString( USER_EMOTE_2 ) );
      else
        strcpy( tempBuff, handyStr->getString( USER_EMOTE_1 ) );
      break;

    case ISCREWSAY:
      strcpy( tempBuff, handyStr->getString( WIZ_SAYS ) );
      break;

    case ISCREWEMOTE:
      if( strchr( SPECIAL_EMOTE_CHARS, (int)say.getMessage()[ 0 ] ) != NULL )
        strcpy( tempBuff, handyStr->getString( WIZ_EMOTE_2 ) );
      else
        strcpy( tempBuff, handyStr->getString( WIZ_EMOTE_1 ) );
      break;

    case ISTOPIC:
      strcpy( tempBuff, handyStr->getString( ROOM_TOPIC ) );
      break;

    case ISECHO:
      if( getState( MONITOR ) == TRUE )
        sprintf( tempBuff, "[^%s^] %s",
                           say.getUserFrom(),
                           say.getMessage() );
      else
        strcpy( tempBuff, say.getMessage() );

      break;

    case ISATMO:
      return( FALSE );
      break;

    case ISTELL:
      // A bot tell comes through without any formatting
      //
      if( say.getFromBot() )
        strcpy( tempBuff, say.getMessage() );
      else
        strcpy( tempBuff, handyStr->getString( USER_TELLS ) );

      break;

    case ISPEMOTE:
      if( strchr( SPECIAL_EMOTE_CHARS, (int)say.getMessage()[ 0 ] ) != NULL )
        strcpy( tempBuff, handyStr->getString( USER_PEMOTE_2 ) );
      else
        strcpy( tempBuff, handyStr->getString( USER_PEMOTE_1 ) );
      break;

    case ISMULTITELL:
      handyStr->setUserName( say.getUserTo() );
      handyStr->setCrewName( say.getUserFrom() );
      strcpy( tempBuff, handyStr->getString( USER_MULTITELLS ) );
      break;

    case ISMULTIPEMOTE:
      strcpy( colourBuff, say.getMessage() );

      if( strchr( SPECIAL_EMOTE_CHARS, (int)colourBuff[ 0 ] ) != NULL )
        sprintf( tempBuff, "%c -> %s %s%s", HIGHLIGHT_CHAR,
                                            say.getUserTo(),
                                            say.getUserFrom(),
                                            colourBuff );
      else
        sprintf( tempBuff, "%c -> %s %s %s", HIGHLIGHT_CHAR,
                                             say.getUserTo(),
                                             say.getUserFrom(),
                                             colourBuff );
      break;

    case ISSEMOTE:
      if( strchr( SPECIAL_EMOTE_CHARS, (int)say.getMessage()[ 0 ] ) != NULL )
        strcpy( tempBuff, handyStr->getString( USER_SEMOTE_2 ) );
      else
        strcpy( tempBuff, handyStr->getString( USER_SEMOTE_1 ) );
      break;

    case ISSHOUT:
      strcpy( tempBuff, handyStr->getString( USER_SHOUTS ) );
      break;

    case ISPICTURE:
      if( getState( MONITOR ) == TRUE )
        sprintf( tempBuff, "[^%s^] %s", say.getUserFrom(), say.getMessage() );
      else
        strcpy( tempBuff, say.getMessage() );
      break;

    // Checking to see if the user is listening to these types of says
    // (which require no formatting) has already been done, so it is safe
    // to squish them all together like this.
    //
    case ISBOT:
    case ISINVITE:
    case ISNEWSMAIL:
    case ISLOGON:
    case ISLOGOFF:
    case ISROOMINFO:
    case ISROOMLIST:
    case ISUSERLIST:
    case ISFEEDBACK:
    case ISUNIGNORABLE:
    case ISLOGONMSG:
    case ISBROADCAST:

    // May have to take out this default:
    //
    default:
      strcpy( tempBuff, say.getMessage() );
      break;
  }

  colour.parseHighlightStop( tempBuff );
  colour.setBaseColour( say.getColour() );

  if( getState( WORDWRAP ) == TRUE )
    colour.wordWrap( tempBuff, getState( FIXCR ), getCols() );

  if( getState( ANSI ) == TRUE )
  {
    if( getState( HIGHLIGHTS ) == TRUE )
      colour.parseColourIn( tempBuff, colourBuff, TRUE );
    else
      colour.parseColourIn( tempBuff, colourBuff, FALSE );
  }
  else
  {
    if( getState( HIGHLIGHTS ) == TRUE )
      colour.parseColourOut( tempBuff, colourBuff, TRUE );
    else
    {
      needToParse = FALSE;
      colour.parseColourOut( tempBuff, colourBuff, FALSE );
    }
  }

  if( getState( BEEPS ) == FALSE )
    if( colourBuff[ strlen( colourBuff ) - 1 ] == BEEP_CHAR )
      colourBuff[ strlen( colourBuff ) - 1 ] = 0;

  if( say.getLineFeed() == TRUE )
  {
    strcat( colourBuff, "\n" );

    if( getState( FIXCR ) == TRUE )
      strcat( colourBuff, "\r" );
  }

  // Write the data to the user's socket
  //
  socket->writeSocket( getSocket(), colourBuff );

  if( needToParse == TRUE )
    colour.parseColourOut( tempBuff, colourBuff, FALSE );

  return( (strlen( colourBuff ) / getCols()) + 1 );
}

    //////          //////
   //                  //
  // User Persistence //
 //                  //
//////          //////

// A user must always have an alias; this is safe
//
boolean User::load( void )
{
  if( loadData() == FALSE )
    return( FALSE );

  loadAbbrs();
  loadProfile();
  loadMacros();
  return( TRUE );
}

// Load a user by name
//
boolean User::load( char *name )
{
  setName( name );
  return( load() );
}

boolean User::loadAbbrs( void )
{
  FILE     *fp;
  FileName fileName;
  char     tempBuff[ SAY_LENGTH + 1 ];

  fileName.setFileName( USER_DIR, getName(), ABBR_EXTENSION );

  if( (fp = fopen( fileName.getFileName(), "r" )) == NULL )
    return( FALSE );

  resetAbbrs();

  while( fgets( tempBuff, SAY_LENGTH, fp ) != NULL )
  {
    tempBuff[ strlen( tempBuff ) - 1 ] = 0;
    addAbbr( tempBuff[ 0 ], &tempBuff[ 2 ] );
  }

  fclose( fp );
  return( TRUE );
}

boolean User::loadData( void )
{
  FILE     *fp;
  FileName fileName;
  char     tempBuff[ SAY_LENGTH + 1 ];

  ulong tempNum = 0;
  int   rowCols = 0;

  fileName.setFileName( USER_DIR, getName(), USER_EXTENSION );

  if( (fp = fopen( fileName.getFileName(), "r" )) == NULL )
    return( FALSE );

  // Read all the user's information from a file in a known way
  //
  fgets( tempBuff, SAY_LENGTH, fp );
  tempBuff[ strlen( tempBuff ) - 1 ] = 0;
  setName( tempBuff );

  fgets( tempBuff, SAY_LENGTH, fp );
  tempBuff[ strlen( tempBuff ) - 1 ] = 0;
  setPassword( tempBuff );

  fgets( tempBuff, SAY_LENGTH, fp );
  tempBuff[ strlen( tempBuff ) - 1 ] = 0;
  setDescription( tempBuff );

  fscanf( fp, "%ld\n", &tempNum );
  setLevel( (int)tempNum );

  fscanf( fp, "%lu\n", &tempNum );
  setState( tempNum );

  // This is required for looking up a user's last time on (gets changed
  // after a successful logon)
  //
  fscanf( fp, "%lu\n", &tempNum );
  setTimeOn( tempNum );

  fgets( tempBuff, SAY_LENGTH, fp );
  tempBuff[ strlen( tempBuff ) - 1 ] = 0;

  if( getSaveUser() == TRUE )
    setSite( tempBuff );

  fscanf( fp, "%d\n", &rowCols );
  setRows( rowCols );
  fscanf( fp, "%d\n", &rowCols );
  setCols( rowCols );

  if( fgets( tempBuff, SAY_LENGTH, fp ) != NULL )
  {
    tempBuff[ strlen( tempBuff ) - 1 ] = 0;
    setEmail( tempBuff );
  }
  else
    setEmail( "" );

  if( fgets( tempBuff, SAY_LENGTH, fp ) != NULL )
  {
    tempBuff[ strlen( tempBuff ) - 1 ] = 0;
    setHomepage( tempBuff );
  }
  else
    setHomepage( "" );

  fclose( fp );
  return( TRUE );
}

boolean User::loadMacros( void )
{
  FILE     *fp;
  FileName fileName;

  int  count = 0;
  char tempBuff[ SAY_LENGTH + 1 ];

  fileName.setFileName( USER_DIR, getName(), MACRO_EXTENSION );

  if( (fp = fopen( fileName.getFileName(), "r" )) == NULL )
    return( FALSE );

  initMacros();

  while( (fgets( tempBuff, SAY_LENGTH, fp) != NULL) &&
         (count++ < MACRO_MAX) )
  {
    tempBuff[ strlen( tempBuff ) - 1 ] = 0;
    addMacro( tempBuff );
  }

  fclose( fp );

  return( TRUE );
}

boolean User::loadProfile( void )
{
  char tempBuff[ SAY_LENGTH + 1 ];
  int  count = 0;

  FILE     *fp;
  FileName fileName;

  fileName.setFileName( USER_DIR, getName(), PROFILE_EXTENSION );

  if( (fp = fopen( fileName.getFileName(), "r" )) == NULL )
    return( FALSE );

  profile.clear();

  // Load the user's profile
  //
  while( (fgets( tempBuff, SCREEN_COLUMNS, fp ) != NULL) &&
         (count < PROFILE_LINES) )
  {
    tempBuff[ strlen( tempBuff ) - 1 ] = 0;

    if( tempBuff[ 0 ] == 0 )
      break;

    profile.addLine( tempBuff );
  }

  fclose( fp );

  return( TRUE );
}

boolean User::resetAbbrs( void )
{
  int count = 0;

  while( count < ABBR_MAX )
    abbrs[ count++ ][ 0 ] = 0;

  return( TRUE );
}

boolean User::save( void )
{
  FILE     *fp;
  FileName fileName;

  char tempBuff[ SAY_LENGTH + 1 ];

  if( getSaveUser() == FALSE )
    return( FALSE );

  fileName.setFileName( USER_DIR, getName(), USER_EXTENSION );

  if( (fp = fopen( fileName.getFileName(), "w" )) == NULL )
    return( FALSE );

  // Dump all the user's information into a file in a known way
  //
  fprintf( fp, "%s\n%s\n%s\n%u\n%lu\n%lu\n%s\n%d\n%d\n%s\n%s\n",
               getName(),
               getPassword(),
               getDescription(),
               getLevel(),
               getState(),
               getTimeOn(),
               getSite(),
               getRows(),
               getCols(),
               getEmail(),
               getHomepage() );

  fclose( fp );

  return( TRUE );
}

boolean User::saveAbbrs( void )
{
  FILE     *fp;
  FileName fileName;
  int      count = 0;

  fileName.setFileName( USER_DIR, getName(), ABBR_EXTENSION );

  if( (fp = fopen( fileName.getFileName(), "w" )) == NULL )
    return( FALSE );

  while( count < ABBR_MAX )
  {
    if( abbrs[ count ][ 0 ] != 0 )
      fprintf( fp, "%s\n", abbrs[ count ] );

    count++;
  }

  fclose( fp );
  return( TRUE );
}

boolean User::saveMacros( void )
{
  int  count = 0;
  char tempBuff[ SAY_LENGTH + 1 ];

  FILE     *fp;
  FileName fileName;

  fileName.setFileName( USER_DIR, getName(), MACRO_EXTENSION );

  if( (fp = fopen( fileName.getFileName(), "w" )) == NULL )
    return( FALSE );

  while( count < MACRO_MAX )
    fprintf( fp, "%s\n", getMacroAt( count++ ) );

  fclose( fp );
  return( TRUE );
}

boolean User::saveProfile( void )
{
  int  count = 0,
       speed = profile.getNumLines();
  char tempBuff[ SAY_LENGTH + 1 ];

  FILE     *fp;
  FileName fileName;

  fileName.setFileName( USER_DIR, getName(), PROFILE_EXTENSION );

  if( (fp = fopen( fileName.getFileName(), "w" )) == NULL )
    return( FALSE );

  while( count < speed )
    fprintf( fp, "%s\n", profile.getLine( count++ ) );

  fclose( fp );
  return( TRUE );
}

boolean User::getSocketInput( char *tempBuff )
{
  String copyBuff;

  if( cmdToIssue.index( "\r" ) == 0 )
    copyBuff = cmdToIssue.before( "\n" );
  else
    copyBuff = cmdToIssue.before( "\r" );

  // Return as much as possible as the command that should be issued
  //
  strncpy( tempBuff, (const char *)copyBuff, SAY_LENGTH );

  // Store the overflow of the command to issue back INTO cmdToIssue
  //
  copyBuff   = cmdToIssue.after( "\n" );
  cmdToIssue = copyBuff;
  return( TRUE );
}

// Return FALSE if ENTER was found; buffer may not be overflowed anymore
//
boolean User::moreSocketInput( char *input )
{
  // Add "input" to the input (buffered) that may already exist
  //
  cmdToIssue += input;

  // Insert a CR if there isn't already one there
  //
  if( cmdToIssue.index( "\r" ) == -1 )
    cmdToIssue.gsub( "\n", "\r\n" );

  return( (cmdToIssue.index( "\n" ) == -1) ? TRUE : FALSE );
}

