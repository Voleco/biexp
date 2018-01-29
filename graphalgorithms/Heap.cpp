/*
 * $Id: Heap.cpp,v 1.6 2006/11/29 17:40:14 nathanst Exp $
 *
 * This file is part of HOG.
 *
 * HOG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * HOG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HOG; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/ 

// HOG File

#include <iostream>
#include "FPUtil.h"
#include "Heap.h"

Heap::Heap(int s)
{
  count = 0;
  _elts.reserve(s);
}

Heap::~Heap()
{
}

unsigned int Heap::size()
{
	return _elts.size();
}

/**
 * Add object into Heap.
 */
void Heap::Add(graph_object *val)
{
  val->key = count;
  _elts.push_back(val);
  count++;
  HeapifyUp(val->key);
}

/**
 * Indicate that the key for a particular object has decreased.
 */
void Heap::DecreaseKey(graph_object *val)
{
  HeapifyUp(val->key);
}

/**
 * Returns true if the object is in the Heap.
 */
bool Heap::IsIn(graph_object *val)
{
  if (val->key < _elts.size() &&
			(_elts[val->key] == val))
		return true;
  return false;
}

/**
 * Remove the item with the lowest key from the Heap & re-heapify.
 */
graph_object *Heap::Remove()
{
  if (Empty())
		return 0;
  count--;
  graph_object *ans = _elts[0];
  _elts[0] = _elts[count];
  _elts[0]->key = 0;
  _elts.pop_back();
  HeapifyDown(0);

  return ans;
}

/**
 * Returns true if no items are in the Heap.
 */
bool Heap::Empty()
{
  return count == 0;
}

void Heap::HeapifyUp(int index)
{
  if (index == 0) return;
  int parent = (index-1)/2;

  if (fgreater(_elts[parent]->GetKey(), _elts[index]->GetKey()))
	{
    graph_object *tmp = _elts[parent];
    _elts[parent] = _elts[index];
    _elts[index] = tmp;
    _elts[parent]->key = parent;
    _elts[index]->key = index;
    HeapifyUp(parent);
  }
}

void Heap::HeapifyDown(int index)
{
  int child1 = index*2+1;
  int child2 = index*2+2;
  int which;
	
  // find smallest child
  if (child1 >= count)
    return;
  else if (child2 >= count)
    which = child1;
  else if (fless(_elts[child1]->GetKey(), _elts[child2]->GetKey()))
    which = child1;
  else
    which = child2;

  if (fless(_elts[which]->GetKey(), _elts[index]->GetKey()))
	{
    graph_object *tmp = _elts[which];
    _elts[which] = _elts[index];
    _elts[index] = tmp;
    _elts[which]->key = which;
    _elts[index]->key = index;
    HeapifyDown(which);
  }
}
