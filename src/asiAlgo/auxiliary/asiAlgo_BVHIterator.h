//-----------------------------------------------------------------------------
// Created on: 23 September 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2016-present, Sergey Slyadnev, Alexander Malyshev
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
  const static int STACK_DEPTH = 64;

public:

  //! Ctor. Constructs the iterator for visiting a BVH tree.
  //! \param[in] bvh the BVH structure to traverse.
  asiAlgo_EXPORT
    asiAlgo_BVHIterator(const opencascade::handle<BVH_Tree<double, 3>>& bvh);

public:

  //! Moves iterator to the next position.
  //! \param [in] rightFirst indicates that the right sub-tree should be
  //!                        inspected before the left one.
  asiAlgo_EXPORT void
    Next(const bool rightFirst = false);

  //! \return true if there is still something to iterate over,
  //!         false -- otherwise.
  asiAlgo_EXPORT bool
    More() const;

  //! \return current item.
  asiAlgo_EXPORT const BVH_Vec4i&
    Current() const;

  //! \return current node's index.
  asiAlgo_EXPORT int
    CurrentIndex() const;

  //! \return true if the current BVH node is leaf, false -- otherwise.
  asiAlgo_EXPORT bool
    IsLeaf() const;

  //! Prevents iterator from visiting left child of the current node.
  asiAlgo_EXPORT void
    BlockLeft();

  //! Prevents iterator from visiting right child of the current node.
  asiAlgo_EXPORT void
    BlockRight();

protected:

  opencascade::handle<BVH_Tree<double, 3>> m_bvh; //!< Structure to iterate over.

  // Iteration variables.
  int     m_stack[STACK_DEPTH]; //!< Non-traversed nodes to return.
  int     m_stackHead;          //!< Pointer to the stack head.
  int     m_currentNode;        //!< Current node.
  bool    m_blocked[2];         //!< Nodes to stop traverse (their children will be skipped).

  // Internal variables to speedup calculation.
  BVH_Vec4i m_current;         //!< current node.
  bool      m_isLeaf;          //!< flag indicating whether the current node is leaf or not.

};

#endif
