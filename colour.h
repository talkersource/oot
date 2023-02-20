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

#ifndef T_COLOUR_H
#define T_COLOUR_H

#include "consts.h"

// Some colourful constants (in order of ANSI)
//
enum { BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, LIGHTGREY };

// No colour may be greater than 16 right now.
//
#define BOLD      255

#define BOLD_OFF  '0'
#define BOLD_ON   '1'

class Colour
{
  private:
    char ansiSeq[ ANSI_COLOUR_LEN + 1],
         ansiSeqBold[ ANSI_COLOUR_LEN + 1 ];

    iColour baseColour;

  private:
    char    *getAnsiSeq( void );
    char    *getAnsiSeqBold( void );
    boolean isColourCode( char c );

  public:
    Colour( void );
   ~Colour( void );

    boolean setBold( char boldChar );
    boolean getBold( char boldChar, char *buff );
    boolean setBaseColour( iColour c );
    iColour getBaseColour( void );
    boolean setColour( iColour c );
    boolean getColour( iColour c, char *buff );

    boolean parseColourIn( char *toParse, char *result, boolean toBold );
    boolean parseColourOut( char *toParse, char *result, boolean toBold );
    boolean parseHighlightStop( char *result );

    boolean wordWrap( char *wrapBuffer, boolean fixCr, int columns );
    boolean softWordWrap( char *wrapBuffer, boolean fixCr, int columns );

    boolean stripEnd( char *result );
};

#endif
