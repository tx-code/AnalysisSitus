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

#ifndef asiAlgo_BVHIterator_h
#define asiAlgo_BVHIterator_h

// Analysis Situs includes
#include <asiAlgo.h>

// OCCT includes
#include <BVH_Types.hxx>
#include <BVH_PrimitiveSet.hxx>
#include <NCollection_Handle.hxx>
#include <TColStd_PackedMapOfInteger.hxx>

//-----------------------------------------------------------------------------

namespace
{

// This default hash assumes T can be converted to int implicitly.
// The user can define how to map T to int with template specializations.
template <class T>
struct DefaultHashFunc
{
  inline static int to_int(const T& t)
  {
    return t;
  }
};

// This is a stack-based hash table supporting basic operations:
// - insert an element into the hash
// - see if an element exist
//
// Pros: Fast, no heap memory, no STL header needed. e.g. embeded programming
// Cons: User must anticipate max memory usage. Limited functionality.
// NOTE: if N is not enough to hold all element, error will be reported.
template <class T, int N, class HashFunc = DefaultHashFunc<T> >
class StackHash
{
private:

  struct Node
  {
    T val;
    Node *next;
  };

  Node  buffer[N];
  Node *hash[N] = {nullptr};
  int   buf_i   = 0;

public:

  void Add(const T& t)
  {
    if ( buf_i >= N )
    {
      std::cerr << "error: exceeded capacity!" << std::endl;
      return;
    }
    buffer[buf_i].val = t;

    int hash_value = HashFunc::to_int(t)%N;

    if ( hash[hash_value] == nullptr )
    {
      buffer[buf_i].next = nullptr;
      hash[hash_value] = &buffer[buf_i];
    }
    else
    {
      buffer[buf_i].next = hash[hash_value];
      hash[hash_value] = &buffer[buf_i];
    }
    buf_i++;
  }

  bool Contains(const T& t)
  {
    int hash_value = HashFunc::to_int(t)%N;

    if ( hash[hash_value] == nullptr )
    {
      return false;
    }
    else
    {
      Node *p = hash[hash_value];
      while ( p )
      {
        if ( p->val == t ) return true;
        p = p->next;
      }
    }
    return false;
  }
};

}

//-----------------------------------------------------------------------------

//! Depth-first iterator for BVH structure.
//!
//! Stored data of each BVH node has the following meaning:
//! <pre>
//! ========================================================
//!  coord |         leaf        |        sub-volume       |
//! ========================================================
//!    x   |          !0         |             0           |
//! -------+---------------------+-------------------------+
//!    y   | idx of start grain  | idx of left child node  |
//! -------+---------------------+-------------------------+
//!    z   |  idx of end grain   | idx of right child node |
//! ========================================================
//! </pre>
class asiAlgo_BVHIterator
{
  typedef int BVH_Item; // Local jargon (node in BVH).

public:

  asiAlgo_EXPORT
    asiAlgo_BVHIterator(const opencascade::handle<BVH_Tree<double, 3> >& bvh);

public:

  asiAlgo_EXPORT bool
    More() const;

  asiAlgo_EXPORT const BVH_Vec4i&
    Current() const;

  asiAlgo_EXPORT int
    CurrentIndex() const;

  asiAlgo_EXPORT void
    Next();

  asiAlgo_EXPORT bool
    IsLeaf() const;

  asiAlgo_EXPORT void
    BlockLeft();

  asiAlgo_EXPORT void
    BlockRight();

protected:

  opencascade::handle< BVH_Tree<double, 3> > m_bvh; //!< Structure to iterate over.

  // Iteration state variables
  BVH_Item            m_stack[96];   //!< Non-traversed nodes to return.
  int                 m_iStackHead;  //!< Pointer to the stack head.
  BVH_Item            m_currentNode; //!< Current node.
  StackHash<int, 512> m_blocked;     //!< Nodes to stop traverse (their children will be skipped).

};

#endif
