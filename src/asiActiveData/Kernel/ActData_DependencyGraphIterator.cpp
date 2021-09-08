//-----------------------------------------------------------------------------
// Created on: January 2013
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

// Own include
#include <ActData_DependencyGraphIterator.h>

// OCCT includes
#include <TFunction_Scope.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>

//! Default constructor.
ActData_DependencyGraphIterator::ActData_DependencyGraphIterator()
: m_currentID(-1),
  m_levelRoot(-1),
  m_bReversed(Standard_False)
{
}

//! Complete constructor.
//! \param theGraph [in] Dependency Graph to iterate through.
//! \param theRootVertex [in] root vertex to start iteration from.
//! \param isReverseOrder [in] indicates whether the traverse should be
//!        done from next to previous (true) or from previous to next (false).
ActData_DependencyGraphIterator::ActData_DependencyGraphIterator(const Handle(ActData_DependencyGraph)& theGraph,
                                                                 const Standard_Integer theRootVertex,
                                                                 const Standard_Boolean isReverseOrder)
{
  this->Init(theGraph, theRootVertex, isReverseOrder);
}

//-----------------------------------------------------------------------------
// API
//-----------------------------------------------------------------------------

//! Initializes Iterator with Dependency Graph instance.
//! \param theGraph [in] Dependency Graph to iterate through.
//! \param theRootVertex [in] root vertex to start iteration from.
//! \param isReverseOrder [in] indicates whether the traverse should be
//!        done from next to previous (true) or from previous to next (false).
void ActData_DependencyGraphIterator::Init(const Handle(ActData_DependencyGraph)& theGraph,
                                           const Standard_Integer theRootVertex,
                                           const Standard_Boolean isReverseOrder)
{
  m_bReversed = isReverseOrder;
  m_graph     = theGraph;
  m_currentID = theRootVertex;
  m_levelRoot = -1;

  m_traversedSiblings.Clear();
  m_traversedSiblings.Add(theRootVertex);

  m_currentSiblings.Clear();
  m_currentSiblings.Add(theRootVertex);
}

//! Checks if there are any items to iterate.
//! \return true/false.
Standard_Boolean ActData_DependencyGraphIterator::More() const
{
  /* ================
   *  Check siblings
   * ================ */

  // Primitive check that all them have been traversed. If not, there
  // is still something to continue on in a sibling list
  if ( m_currentSiblings.Extent() != m_traversedSiblings.Extent() )
    return Standard_True;

  /* ==================
   *  Check next level
   * ================== */

  // Get all vertices which can be reached from the current siblings. This
  // logic works when all siblings are already traversed, so we cannot
  // miss any reachable node
  NCollection_Map<Standard_Integer> aNextLevel;
  this->nextLevel(aNextLevel);

  // Check size
  return aNextLevel.Extent() > 0;
}

//! Returns current ID.
//! \return ID of the current vertex.
Standard_Integer ActData_DependencyGraphIterator::Current() const
{
  return m_currentID;
}

//! Moves iterator to the next item. Iteration goes deeper only if no
//! siblings exist for the current vertex.
void ActData_DependencyGraphIterator::Next()
{
  if ( m_levelRoot == -1 )
  {
    this->nextLevel();
  }
  else // Check siblings
  {
    // Try to find any non-traversed sibling
    Standard_Integer aNextSibling = -1;
    for ( NCollection_Map<Standard_Integer>::Iterator sit(m_currentSiblings); sit.More(); sit.Next() )
    {
      Standard_Integer S = sit.Value();
      if ( !m_traversedSiblings.Contains(S) )
      {
        aNextSibling = S;
        break;
      }
    }

    if ( aNextSibling != -1 ) // We found a non-traversed sibling
    {
      m_traversedSiblings.Add(aNextSibling);
      m_currentID = aNextSibling;
    }
    else // All siblings have been traversed --> go deeper from current
    {
      this->nextLevel();
    }
  }
}

//-----------------------------------------------------------------------------
// Internal routines
//-----------------------------------------------------------------------------

//! Switches current to the first item on the next level.
void ActData_DependencyGraphIterator::nextLevel()
{
  // Get successors or predecessors for each (!) current sibling
  NCollection_Map<Standard_Integer> aNextSet;
  this->nextLevel(aNextSet);

  // Check if there is something at all
  if ( aNextSet.IsEmpty() )
  {
    m_currentSiblings.Clear();
    m_currentID = -1;
    return;
  }

  // Get any successor or predecessor
  const Standard_Integer
    aNext = NCollection_Map<Standard_Integer>::Iterator(aNextSet).Value();

  // Change state
  m_traversedSiblings.Clear();
  m_traversedSiblings.Add(aNext);
  m_currentSiblings = aNextSet;
  m_levelRoot       = m_currentID;
  m_currentID       = aNext;
}

//! Returns all vertices which can be reached from all the accumulated
//! siblings.
//! \param theLevel [out] result set.
void ActData_DependencyGraphIterator::nextLevel(NCollection_Map<Standard_Integer>& theLevel) const
{
  // Make sure that the set is initially empty
  theLevel.Clear();

  // Get successors or predecessors for each (!) sibling
  for ( NCollection_Map<Standard_Integer>::Iterator sit(m_currentSiblings); sit.More(); sit.Next() )
  {
    const Standard_Integer aSibling = sit.Value();

    // Get all vertices which can be reached from the current sibling
    NCollection_Sequence<Standard_Integer>
      aNext4Sibling = this->successorsOrPredecessors(aSibling);

    // Add vertices to the result set. Notice that uniqueness in maintained
    for ( Standard_Integer s = 1; s <= aNext4Sibling.Length(); ++s )
      theLevel.Add( aNext4Sibling(s) );
  }
}

//! Returns the list of successors or predecessors (depending on the iteration
//! mode) for the current vertex.
//! \return list of successors or predecessors for the current vertex.
NCollection_Sequence<Standard_Integer>
  ActData_DependencyGraphIterator::successorsOrPredecessors() const
{
  return m_bReversed ? this->predecessors(m_currentID) : this->successors(m_currentID);
}

//! Returns the list of successors or predecessors (depending on the iteration
//! mode) for the given vertex.
//! \param theVertex [in] vertex to get successors or predecessors for.
//! \return list of successors or predecessors for the current vertex.
NCollection_Sequence<Standard_Integer>
  ActData_DependencyGraphIterator::successorsOrPredecessors(const Standard_Integer theVertex) const
{
  return m_bReversed ? this->predecessors(theVertex) : this->successors(theVertex);;
}

//! Returns the list of successors for the current vertex.
//! \return list of successors for the current vertex.
NCollection_Sequence<Standard_Integer> ActData_DependencyGraphIterator::successors() const
{
  return this->successors(m_currentID);
}

//! Returns the list of successors for the given vertex.
//! \param theVertex [in] vertex to get successors for.
//! \return list of successors for the current vertex.
NCollection_Sequence<Standard_Integer>
  ActData_DependencyGraphIterator::successors(const Standard_Integer theVertex) const
{
  NCollection_Sequence<Standard_Integer> aResult;

  // Access current Function node
  TFunction_IFunction CurrentIFunc = this->settleIFunc(theVertex);

  // Access next Functions
  TDF_LabelList aNextLabs;
  CurrentIFunc.GetNext(aNextLabs);

  for ( TDF_ListIteratorOfLabelList lit(aNextLabs); lit.More(); lit.Next() )
  {
    Standard_Integer aVertexID = m_graph->VertexByLabel( lit.Value() );
    if ( aVertexID != -1 )
      aResult.Append(aVertexID);
  }

  return aResult;
}

//! Returns the list of predecessors for the current vertex.
//! \return list of predecessors for the current vertex.
NCollection_Sequence<Standard_Integer> ActData_DependencyGraphIterator::predecessors() const
{
  return this->predecessors(m_currentID);
}

//! Returns the list of predecessors for the given vertex.
//! \param theVertex [in] vertex to get predecessors for.
//! \return list of predecessors for the current vertex.
NCollection_Sequence<Standard_Integer>
  ActData_DependencyGraphIterator::predecessors(const Standard_Integer theVertex) const
{
  NCollection_Sequence<Standard_Integer> aResult;

  // Access current Function node
  TFunction_IFunction CurrentIFunc = this->settleIFunc(theVertex);

  // Access previous Functions
  TDF_LabelList aNextLabs;
  CurrentIFunc.GetPrevious(aNextLabs);

  for ( TDF_ListIteratorOfLabelList lit(aNextLabs); lit.More(); lit.Next() )
  {
    Standard_Integer aVertexID = m_graph->VertexByLabel( lit.Value() );
    if ( aVertexID != -1 )
      aResult.Append(aVertexID);
  }

  return aResult;
}

//! Settles IFunction interface for the passed vertex ID.
//! \param theVertex [in] vertex ID to source data for.
//! \return IFunction tool.
TFunction_IFunction ActData_DependencyGraphIterator::settleIFunc(const Standard_Integer theVertex) const
{
  // Access TFunction Scope
  TDF_Label aRootLab = m_graph->Model()->RootLabel();
  Handle(TFunction_Scope) aScope;
  if ( !aRootLab.Root().FindAttribute(TFunction_Scope::GetID(), aScope) )
    return TFunction_IFunction();

  // Access root Label of the next Function
  const TDF_Label& aLabel = aScope->GetFunction(theVertex);

  // Settle IFunction Data Cursor
  TFunction_IFunction IFunc(aLabel);
  return IFunc;
}
