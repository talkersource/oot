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
// ListNode is a class to encapsulate the behaviour of a single node
// in this "linked list" (treated as a stack).
//
// List is the actual list of ListNodes.
//
// Last modified: April 8, 1997 by Dave Jarvis
//
// Added (April 8, 1997)
//  * GNU General Public License documentation
//
/////////////////////////////////////////////////////////////////////////

#include "listbase.h"

ListNode::ListNode( void )
{
  Next = NULL;
}

ListNode::~ListNode( void )
{
  Next = NULL;
}

List::List( void )
{
  Head        = NULL;
  CurrentNode = NULL;

  numElements = 0;
}

List::~List( void )
{
  while( Head )
  {
    CurrentNode = Head->Next;
    delete( Head );
    Head = CurrentNode;
  }
}

ListNodePtr List::findNode( ListBase &toFind )
{
  ListNodePtr searchNode = Head;

  // If none found, searchNode will be NULL
  //
  while( searchNode )
  {
    if( searchNode->Element == &toFind )
      break;
    else
      searchNode = searchNode->Next;
  }

  return( searchNode );
}

ListNodePtr List::FindParentNode( ListBase &ToFind )
{
  ListNodePtr parent = NULL,
              child  = Head;

  while( child )
  {
    if( child->Element == &ToFind )
      break;
    else
    {
      parent = child;
      child  = child->Next;
    }
  }

  return( parent );
}

// If a node exists, or no memory left return FALSE
//
boolean List::operator += ( ListBase &AddToList )
{
  ListNodePtr aNode = findNode( AddToList );

  if( aNode )
    return( FALSE );

  if( (aNode = new( ListNode )) == NULL )
    return( FALSE );

  aNode->Element = &AddToList;
  aNode->Next    = Head;
  Head           = aNode;

  numElements++;

  return( TRUE );
}

// If no node was found to delete, return FALSE
//
boolean List::operator -= ( ListBase &DelFromList )
{
  ListNodePtr Parent, ToDelete = findNode( DelFromList );

  if( ToDelete == NULL )
    return( FALSE );

  Parent = FindParentNode( DelFromList );

  if( Parent == NULL )
    Head = Head->Next;
  else
    Parent->Next = ToDelete->Next;

  if( ToDelete == CurrentNode )
    CurrentNode = NULL;

  delete( ToDelete );
  ToDelete = NULL;

  numElements--;

  return( TRUE );
}

// This method was added so the list would perform more like a dynamic
// array.
//
ListBasePtr List::at( int index )
{
  int count;

  if( index < 0 )
    return( NULL );

  CurrentNode = Head;

  for( count = 0; count != index; count++ )
    if( CurrentNode )
      CurrentNode = CurrentNode->Next;
    else
      return( NULL );

  // If the CurrentNode is NULL, then return NULL
  //
  if( !CurrentNode )
    return( NULL );

  return( CurrentNode->Element );
}

int List::howMany( void )
{
  return( numElements );
}
