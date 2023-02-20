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
// Telnet class hides away the telnet protcol.
//
/////////////////////////////////////////////////////////////////////////

#include "telnet.h"

// The first character of buffer is IAC.  The rest of the buffer either
// has telnet options (and extended options) or the socket needs to be
// read in order to figure out what the telnet command was.
//
// The sock is a valid (open) socket
//
// The function returns:
//   TRUE if the command was parsed successfully
//   FALSE if the command wasn't parse successfully
//   -2 if the socket was closed prematurely
//
boolean Telnet::telnetCommand( char *buffer, int sock )
{
}

