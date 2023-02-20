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
/////////////////////////////////////////////////////////////////////////

#ifndef T_TIME_H
#define T_TIME_H

#include "consts.h"

// Days; Hours; Minutes (and Seconds, but not used)
//
#define TIME_FORMAT  "^2%3d ^5%02d^1h^5%02d^1m^"

#define MAX_TIME_LENGTH   32

#define SECONDS_PER_DAY   86400L
#define SECONDS_PER_HOUR  3600L
#define SECONDS_PER_MIN   60L

class TTime
{
  private:
    time_t keepTime;

  public:
    TTime( void );

    time_t  getCurrentTime( void );
    uint    getMinDifference( time_t time1, time_t time2 );
    boolean setTime( time_t newTime );
    time_t  getTime( void );

    boolean difference( time_t time1, time_t time2, char *timeStr );
    boolean updateTime( void );
};

// A pointer to Time ... how poetic
//
typedef TTime *TTimePtr;

#endif
