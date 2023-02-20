/////////////////////////////////////////////////////////////////////////
//
/*
   Copyright (C) 1996 by Ken Savage

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
// Base-class based list declaration.
//
// Last modified: April 8, 1997
// By           : Dave Jarvis
//
// Added (July 27)
//   * findNode method moved into public access
//
// Added (April 8)
//   * GNU General Public License
//

#ifndef T_LISTBASE_H
#define T_LISTBASE_H

#include "consts.h"

class ListBase;
class ListElement;
class ListNode;
class List;

typedef ListBase    *ListBasePtr;
typedef ListElement *ListElementPtr;
typedef ListNode    *ListNodePtr;
typedef List        *ListPtr;

// Base class for list
//
class ListBase
{
  public:
//    virtual void doNotSubclassListBase( void ) = 0;
};

class ListElement
{
  friend List;

  protected:
    ListBasePtr Element;
};

class ListNode : public ListElement
{
  friend List;

  public:
    ListNode( void );
   ~ListNode( void );

  protected:
    ListNodePtr Next;
};

class List
{
  private:
    int numElements;

  protected:
    ListNodePtr Head;
    ListNodePtr CurrentNode;
    ListNodePtr FindParentNode( ListBase &ToFind );

  public:
    List( void );
   ~List( void );

    boolean operator += ( ListBase &AddToList );
    boolean operator -= ( ListBase &DelFromList );

    ListNodePtr findNode( ListBase &toFind );

    int howMany( void );

    ListBasePtr at( int index );
};

#endif
