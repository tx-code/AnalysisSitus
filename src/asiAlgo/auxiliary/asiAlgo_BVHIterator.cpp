//-----------------------------------------------------------------------------
// Created on: 23 September 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

// Own include
#include <asiAlgo_BVHIterator.h>

//-----------------------------------------------------------------------------

asiAlgo_BVHIterator::asiAlgo_BVHIterator(const opencascade::handle<BVH_Tree<double, 3> >& bvh)
: m_bvh         (bvh),
  m_stackHead   (-1),
  m_currentNode (0) // First index.
{
  for ( int i = 0; i < STACK_DEPTH; ++i )
    m_stack[i] = 0;

  m_blocked[0] = m_blocked[1] = false;
  m_current    = m_bvh->NodeInfoBuffer()[m_currentNode];
  m_isLeaf     = (m_current.x() != 0);
}

//-----------------------------------------------------------------------------

void asiAlgo_BVHIterator::Next(const bool rightFirst)
{
  // The purpose of this method is to set the value of m_currentNode properly.

  if ( m_isLeaf )
  {
    m_currentNode = (m_stackHead < 0) ? -1 : m_stack[m_stackHead--]; // Return to parent.
  }
  else
  {
    const int leftChild        = m_current.y();
    const int rightChild       = m_current.z();
    int       items2Process[2] = { -1, -1 };
    int       itemsCount       = 0;

    // Check if children are not blocked externally.
    if ( !m_blocked[0] )
      items2Process[itemsCount++] = leftChild;
    //
    if ( !m_blocked[1] )
      items2Process[itemsCount++] = rightChild;

    if ( itemsCount == 2 && rightFirst )
      std::swap(items2Process[0], items2Process[1]);

    // Set current node to one of the children. It can be -1 if
    // there is nothing to iterate over.
    m_currentNode = items2Process[0]; // Left item is iterated first (because it is left ;).

    // Fill stack to be able to return to non-traversed items.
    if ( itemsCount == 2 )
      m_stack[++m_stackHead] = items2Process[1];

    // If there is no any allowed child, move back in the stack.
    if ( !itemsCount && m_stackHead >= 0 )
      m_currentNode = m_stack[m_stackHead--];
  }

  m_blocked[0] = false;
  m_blocked[1] = false;

  if ( m_currentNode != -1 )
  {
    m_current = BVH::Array<int, 4>::Value(m_bvh->NodeInfoBuffer(), m_currentNode);
    m_isLeaf  = m_current.x() != 0;
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_BVHIterator::More() const
{
  if ( m_currentNode == -1 )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

const BVH_Vec4i& asiAlgo_BVHIterator::Current() const
{
  return m_current;
}

//-----------------------------------------------------------------------------

int asiAlgo_BVHIterator::CurrentIndex() const
{
  return m_currentNode;
}

//-----------------------------------------------------------------------------

bool asiAlgo_BVHIterator::IsLeaf() const
{
  return m_isLeaf;
}

//-----------------------------------------------------------------------------

void asiAlgo_BVHIterator::BlockLeft()
{
  m_blocked[0] = true;
}

//-----------------------------------------------------------------------------

void asiAlgo_BVHIterator::BlockRight()
{
  m_blocked[1] = true;
}
