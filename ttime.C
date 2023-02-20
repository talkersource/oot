/////////////////////////////////////////////////////////////////////////
//
/*
   Copyright (C) 1996 by Dave Jarvis and Ken Savage

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
// The TTime class handles time manipulations, such as parsing
// out how many days, hours, minutes (and seconds), etc. a
// given time_t has elapsed.
//
// This class is used to manipulate time ...
//
/////////////////////////////////////////////////////////////////////////

#include "ttime.h"

    //////                                               //////
   //                                                       //
  // Class Constructors, Destructors, and Initializations  //
 //                                                       //
//////                                               //////

TTime::TTime( void )
{
  updateTime();
}

    //////               //////
   //                       //
  // Setter/Getter Methods //
 //                       //
//////               //////

time_t TTime::getCurrentTime( void )
{
  return( time( NULL ) );
}

// Calculate the difference in minutes between two times
//
uint TTime::getMinDifference( time_t time1, time_t time2 )
{
  return( (uint)((time2 - time1) / SECONDS_PER_MIN) );
}

boolean TTime::setTime( time_t newTime )
{
  keepTime = newTime;

  return( TRUE );
}

time_t TTime::getTime( void )
{
  return( keepTime );
}

    //////                 //////
   //                         //
  // Called by other Objects //
 //                         //
//////                 //////

// Calculate the differences between two times (result is a string)
//
// Return TRUE if there are 0 days
//
boolean TTime::difference( time_t time1, time_t time2, char *timeStr )
{
  time_t deltaT;
  long   days, hrs, min, sec;

  deltaT  = time2 - time1;
  days    = deltaT / SECONDS_PER_DAY;
  deltaT %= SECONDS_PER_DAY;
  hrs     = deltaT / SECONDS_PER_HOUR;
  deltaT %= SECONDS_PER_HOUR;
  min     = deltaT / SECONDS_PER_MIN;
  deltaT %= SECONDS_PER_MIN;
  sec     = deltaT;

  sprintf( timeStr, TIME_FORMAT, days, hrs, min );
  return( (days > 0) ? FALSE : TRUE );
}

    //////       //////
   //               //
  // Misc. Methods //
 //               //
//////       //////

boolean TTime::updateTime( void )
{
  return( setTime( getCurrentTime() ) );
}
