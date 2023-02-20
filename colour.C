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
// Colour class handles all ANSI.  It has methods for parsing out a
// string which has talker-colour-codes (^1 for blue, ^2 for green,
// etc.).
//
// Also handles word wrap (soft & hard)
//
/////////////////////////////////////////////////////////////////////////

#include "colour.h"

    //////                                               //////
   //                                                       //
  // Class Constructors, Destructors, and Initializations  //
 //                                                       //
//////                                               //////

Colour::Colour( void )
{
  // Initialize the normal and bold ANSI sequences
  //
  ansiSeq[ 0 ] = ESC;
  ansiSeq[ 1 ] = '[';
  ansiSeq[ 2 ] = '3';
  ansiSeq[ 4 ] = 'm';

  ansiSeq[ 5 ] = 0;

  // 3 is the index you change for selecting the colour
  //
  ansiSeq[ 3 ] = (char)(LIGHTGREY + '0');

  ansiSeqBold[ 0 ] = ESC;
  ansiSeqBold[ 1 ] = '[';
  ansiSeqBold[ 3 ] = 'm';

  ansiSeqBold[ 4 ] = 0;

  // This is the index you change to turn bold on or off
  //
  ansiSeqBold[ 2 ] = BOLD_ON;

  setBaseColour( -1 );
}

Colour::~Colour( void )
{
}

    //////         //////
   //                 //
  // Private Methods //
 //                 //
//////         //////

char *Colour::getAnsiSeq( void )
{
  return( ansiSeq );
}

char *Colour::getAnsiSeqBold( void )
{
  return( ansiSeqBold );
}

// Return TRUE if the character passed belongs in the set of the users'
// colour control codes (1, 2, 3, ... 8)
//
boolean Colour::isColourCode( char c )
{
  return( ((c >= '1') && (c <= '8')) ? TRUE : FALSE );
//        (strchr( "BGRMCYW", c ) != NULL) );
}

    //////               //////
   //                       //
  // Setter/Getter Methods //
 //                       //
//////               //////

boolean Colour::setBold( char boldChar )
{
  ansiSeqBold[ 2 ] = boldChar;

  return( TRUE );
}

boolean Colour::getBold( char boldChar, char *buff )
{
  setBold( boldChar );
  strcpy( buff, getAnsiSeqBold() );

  return( TRUE );
}

boolean Colour::setBaseColour( iColour c )
{
  baseColour = c;

  return( TRUE );
}

iColour Colour::getBaseColour( void )
{
  return( baseColour );
}

boolean Colour::setColour( iColour c )
{
  ansiSeq[ 3 ] = (char)(c + '0');

  return( TRUE );
}

boolean Colour::getColour( iColour c, char *buff )
{
  buff[ 0 ] = 0;

  if( c >= BOLD )
  {
    getBold( BOLD_ON, buff );
    c -= BOLD;
  }
  else
    setBold( BOLD_OFF );

  setColour( c );
  strcat( buff, getAnsiSeq() );

  return( TRUE );
}

    //////         //////
   //                 //
  // Parsing Methods //
 //                 //
//////         //////

// Given a string of the following format:
//
// "Words Words ^bold^ Words ^bold ^1Blue ^2Green^ Words ^^ Words ^"
//
// It yields the result of:
//
// "Words Words <BOLD>bold</BOLD> Words <BOLD>bold <BLUE>Blue
//  <GREEN>Green</GREEN></BOLD> Words ^ Words"
//
// Note  : result should be about 10 times the size of toParse
// Note 2: toBold indicates if bold should be added
//
boolean Colour::parseColourIn( char *toParse, char *result, boolean toBold )
{
  char tempBuff[ BIG_SAY_LENGTH + 1 ],
       tempColour[ ANSI_COLOUR_LEN + 1 ],
       *backBuffPtr,
       *tempBuffPtr;

  iColour colourToAdd, baseCol;

  boolean boldOn = FALSE;

  strcpy( tempBuff, toParse );
  stripEnd( tempBuff );

  // Start checking for highlight characters at the start of the string
  //
  tempBuffPtr = tempBuff;

  // After a caret is found, backBuffPtr will be pointing to the word(s)
  // before the caret
  //
  backBuffPtr = tempBuff;

  // Null terminate the result string
  //
  result[ 0 ] = 0;

  if( (baseCol = getBaseColour()) != -1 )
    getColour( baseCol, result );
  else
  {
//    getColour( LIGHTGREY, tempColour );
//    strcpy( result, tempColour );
    getBold( BOLD_OFF, tempColour );
    strcat( result, tempColour );
  }

  // Look up to the next caret
  //
  while( (tempBuffPtr = strchr( backBuffPtr, HIGHLIGHT_CHAR )) != NULL )
  {
    // Null out where the caret was found, so that copying from
    // backBuffPtr will stop before the caret.
    //
    *tempBuffPtr = 0;

    // Copy whatever words were from the backBuffPtr into the result
    //
    strcat( result, backBuffPtr );

    // Found a caret--what's the next character?
    //
    tempBuffPtr++;

    switch( *tempBuffPtr )
    {
      case '1':
//      case 'B':
        colourToAdd = BLUE;
        break;
      case '2':
//      case 'G':
        colourToAdd = GREEN;
        break;
      case '3':
//      case 'R':
        colourToAdd = RED;
        break;
      case '4':
//      case 'M':
        colourToAdd = MAGENTA;
        break;
      case '5':
//      case 'C':
        colourToAdd = CYAN;
        break;
      case '6':
//      case 'Y':
        colourToAdd = YELLOW;
        break;
      case '7':
//      case 'W':
        colourToAdd = LIGHTGREY;
        break;

      // Pseudo-random colour (7 represents the # of colours)
      //
      case '8':
        colourToAdd = 1 + (int)((7 * 1.0) * rand() / (RAND_MAX + 1.0));
        break;

      // If the next character was a caret, add a single ^ to the string
      //
      case HIGHLIGHT_CHAR:
        colourToAdd = -1;
        backBuffPtr = tempBuffPtr + 1;

        strcat( result, "^" );
        break;

      // If the bold is on, shut it off (and vice-versa)
      //
      default:
        colourToAdd = -1;
        backBuffPtr = tempBuffPtr;

        if( boldOn )
        {
          getBold( BOLD_OFF, tempColour );
          strcat( result, tempColour );

          if( baseCol != -1 )
            getColour( baseCol, tempColour );
        }
        else
        if( toBold == TRUE )
          getBold( BOLD_ON, tempColour );

        boldOn = !boldOn;

        strcat( result, tempColour );
        break;
    }

    // Insert a colour sequence
    //
    if( colourToAdd > -1 )
    {
      boldOn = TRUE;

      // Point beyond the colour indicator
      //
      backBuffPtr = tempBuffPtr + 1;

      if( toBold == TRUE )
        getColour( colourToAdd + BOLD, tempColour );
      else
        getColour( colourToAdd, tempColour );

      strcat( result, tempColour );
    }
  }

  // Copy the rest of the buffer into the result
  //
  strcat( result, backBuffPtr );

  getBold( BOLD_OFF, tempColour );
  strcat( result, tempColour );

  return( TRUE );
}

boolean Colour::parseColourOut( char *toParse, char *result, boolean toBold )
{
  boolean boldOn = FALSE;

  char tempBuff[ BIG_SAY_LENGTH + 1 ],
       tempColour[ ANSI_COLOUR_LEN + 1 ],
       *backBuffPtr,
       *tempBuffPtr;

  strcpy( tempBuff, toParse );
  stripEnd( tempBuff );

  // Start checking for highlight characters at the start of the string
  //
  tempBuffPtr = tempBuff;

  // After a caret is found, backBuffPtr will be pointing to the word(s)
  // before the caret
  //
  backBuffPtr = tempBuff;

  // Null terminate the result string
  //
  result[ 0 ] = 0;

  // Look up to the next caret
  //
  while( (tempBuffPtr = strchr( backBuffPtr, HIGHLIGHT_CHAR )) != NULL )
  {
    // Null out where the caret was found, so that copying from
    // backBuffPtr will stop before the caret.
    //
    *tempBuffPtr = 0;

    // Copy whatever words were from the backBuffPtr into the result
    //
    strcat( result, backBuffPtr );

    // Found a caret--what's the next character?
    //
    tempBuffPtr++;

    if( *tempBuffPtr == 0 )
    {
      backBuffPtr = tempBuffPtr;
      break;
    }

    // Skip past the colour indicating characters
    //
    if( isColourCode( *tempBuffPtr ) )
    {
      if( toBold )
      {
        boldOn = TRUE;

        getBold( BOLD_ON, tempColour );
        strcat( result, tempColour );
      }

      tempBuffPtr++;
    }
    else
    if( toBold )
    {
      if( boldOn )
        getBold( BOLD_OFF, tempColour );
      else
        getBold( BOLD_ON, tempColour );

      boldOn = !boldOn;

      strcat( result, tempColour );
    }
    else
    if( *tempBuffPtr == HIGHLIGHT_CHAR )
    {
      strcat( result, "^" );

      tempBuffPtr++;
    }

    backBuffPtr = tempBuffPtr;
  }

  if( boldOn )
  {
    getBold( BOLD_OFF, tempColour );
    strcat( result, tempColour );
  }

  strcat( result, backBuffPtr );

  return( TRUE );
}

// Count the number of highlight characters.  For each highlight
// character, insert a space beyond the HIGHLIGHT_STOP character
// (if it exists).  Also remove the HIGHLIGHT_STOP character.
//
boolean Colour::parseHighlightStop( char *result )
{
  int     highlightCount = 0;
  boolean boldOn         = FALSE;

  char *tempBuffPtr,
       *aheadBuff,
       tempBuff[ SAY_LENGTH + 1 ];

  // If there isn't a highlight stop character, then don't parse
  //
  if( strchr( result, (int)HIGHLIGHT_STOP ) == NULL )
    return( FALSE );

  tempBuffPtr = result;

  while( *tempBuffPtr != HIGHLIGHT_STOP )
  {
    if( *tempBuffPtr == HIGHLIGHT_CHAR )
    {
      highlightCount++;

      boldOn    = !boldOn;
      aheadBuff = tempBuffPtr + 1;

      if( isColourCode( *aheadBuff ) )
      {
        highlightCount++;
        boldOn = TRUE;
      }
      else
      if( *aheadBuff == HIGHLIGHT_CHAR )
      {
        tempBuffPtr++;
        boldOn = !boldOn;
      }
    }

    tempBuffPtr++;
  }

  // Get rid of the HIGHLIGHT_STOP character
  //
  *tempBuffPtr = 0;

  // Point to the remainder of the string and make a copy of it
  //
  tempBuffPtr++;

  // Get rid of the BOLD COLOUR (if any, and the user can see it)
  //
  if( boldOn )
    sprintf( tempBuff, "%s%c%*c%s",
             result, HIGHLIGHT_CHAR, highlightCount, ' ', tempBuffPtr );
  else if( highlightCount > 0 )
    sprintf( tempBuff, "%s%*c%s", result, highlightCount, ' ', tempBuffPtr );
  else
    sprintf( tempBuff, "%s%s", result, tempBuffPtr );

  strcpy( result, tempBuff );

  return( TRUE );
}

    //////      //////
   //              //
  // Wrap Methods //
 //              //
//////      //////

boolean Colour::wordWrap( char *wrapBuffer, boolean fixCr, int columns )
{
  int  count     = 1,
       copyCount = 1,
       colourCnt = 1,
       len       = strlen( wrapBuffer );

  char tempBuff[ BIG_SAY_LENGTH + 1 ],
       ch;

  // Assigning the first character allows us to check (count % columns)
  // by itself, without having to worry about checking (count > 0) (a slight
  // optimisation).
  //
  tempBuff[ 0 ] = wrapBuffer[ 0 ];

  for( count = 1; count < len; count++ )
  {
    ch = wrapBuffer[ count ];

    // This should, in theory, strip out colour sequences from the count
    //
    if( (ch < '0') || (ch > '9') )
    {
      if( ch != HIGHLIGHT_CHAR )
        colourCnt++;
    }
    else if( wrapBuffer[ count - 1 ] != HIGHLIGHT_CHAR )
      colourCnt++;

    // If count has reached the end of the screen, then insert a \n, and
    // a \r if the user has fixed her cr's.
    //
    if( (colourCnt % columns) == 0 )
    {
      tempBuff[ copyCount ] = '\n';

      if( fixCr )
        tempBuff[ ++copyCount ] = '\r';

      copyCount++;
    }

    tempBuff[ copyCount ] = ch;
    copyCount++;
  }

  tempBuff[ copyCount ] = 0;

  strncpy( wrapBuffer, tempBuff, BIG_SAY_LENGTH );
  wrapBuffer[ BIG_SAY_LENGTH ] = 0;

  return( TRUE );
}

// Has a few bugs which prevented it from being used.
//
boolean Colour::softWordWrap( char *wrapBuffer, boolean fixCr, int columns )
{
  int  count     = 0,
       oldCount  = 0,
       len       = strlen( wrapBuffer ),
       copyCount = 0;

  char tempBuff[ BIG_SAY_LENGTH + 1 ];

  for( count = 0; count < len; count++ )
  {
    // If count has reached the end of the screen, then insert a \n, and
    // a \r if the user has fixed her cr's.
    //
    if( count && ((count % columns) == 0) )
    {
      oldCount = count;

      // Count back until a space, or the beginning of the line.  If the
      // beginning of the line is reached, reset count to where it was
      // left off.
      //
      while( (count > 0) && !isspace( wrapBuffer[ count ] ) )
        count--;

      // If we reached the start, then just insert a \n (\r) as needed,
      // otherwise, we want to insert a \n (\r) at the previous space and
      // recopy from there.
      //
      if( count == 0 )
        count = oldCount;
      else
      {
        copyCount -= oldCount - count;
        count++;
      }

      tempBuff[ copyCount ] = '\n';

      if( fixCr )
        tempBuff[ ++copyCount ] = '\r';

      while( count != oldCount )
      {
        copyCount++;
        tempBuff[ copyCount ] = wrapBuffer[ count ];
        count++;
      }

      copyCount++;
    }

    tempBuff[ copyCount ] = wrapBuffer[ count ];
    copyCount++;
  }

  tempBuff[ copyCount ] = 0;

  strncpy( wrapBuffer, tempBuff, BIG_SAY_LENGTH );
  wrapBuffer[ BIG_SAY_LENGTH ] = 0;

  return( TRUE );
}

    //////       //////
   //               //
  // Misc. Methods //
 //               //
//////       //////

// Take off the trailing spaces
//
boolean Colour::stripEnd( char *result )
{
  int len = strlen( result );

  // Strip off any highlight characters and/or spaces from the end of the
  // string before converting
  //
  while( isspace( result[ len ] ) && (len != 0) )
    len--;

  result[ len ] = 0;

  return( TRUE );
}

