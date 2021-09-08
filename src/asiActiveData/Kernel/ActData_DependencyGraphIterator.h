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

#ifndef ActData_DependencyGraphIterator_HeaderFile
#define ActData_DependencyGraphIterator_HeaderFile

// Active Data includes
#include <ActData_DependencyGraph.h>

// OCCT includes
#include <NCollection_Sequence.hxx>
#include <TFunction_IFunction.hxx>

//! \ingroup AD_DF
//!
//! Iterator for Dependency Graph. This class can iterate starting from a
//! single graph node in two directions: up and down. Iterating down you
//! traverse all Tree Functions scheduled for execution after yours.
//! Iterating up you traverse all Tree Functions scheduled for execution
//! before yours. The iterator first traverses a single level:
//! only after the complete level traversal it goes deeper. It means that
//! a successor function will be executed only after all its predecessors
//! are done. The diving point is not deterministic.
class ActData_DependencyGraphIterator
{
public:

  ActData_EXPORT
    ActData_DependencyGraphIterator();

  ActData_EXPORT
    ActData_DependencyGraphIterator(const Handle(ActData_DependencyGraph)& theGraph,
                                    const Standard_Integer theRootVertex,
                                    const Standard_Boolean isReverseOrder);

public:

  ActData_EXPORT void
    Init(const Handle(ActData_DependencyGraph)& theGraph,
         const Standard_Integer theRootVertex,
         const Standard_Boolean isReverseOrder);

  ActData_EXPORT Standard_Boolean
    More() const;

  ActData_EXPORT Standard_Integer
    Current() const;

  ActData_EXPORT void
    Next();

protected:

  void
    nextLevel();

  void
    nextLevel(NCollection_Map<Standard_Integer>& theLevel) const;

  NCollection_Sequence<Standard_Integer>
    successorsOrPredecessors() const;

  NCollection_Sequence<Standard_Integer>
    successorsOrPredecessors(const Standard_Integer theVertex) const;

  NCollection_Sequence<Standard_Integer>
    successors() const;

  NCollection_Sequence<Standard_Integer>
    successors(const Standard_Integer theVertex) const;

  NCollection_Sequence<Standard_Integer>
    predecessors() const;

  NCollection_Sequence<Standard_Integer>
    predecessors(const Standard_Integer theVertex) const;

  TFunction_IFunction
    settleIFunc(const Standard_Integer theVertex) const;

protected:

  Handle(ActData_DependencyGraph)   m_graph;             //!< Graph being iterated.
  Standard_Integer                  m_currentID;         //!< ID of the current vertex.
  Standard_Integer                  m_levelRoot;         //!< Root of the current level.
  NCollection_Map<Standard_Integer> m_traversedSiblings; //!< Traversed siblings for current.
  NCollection_Map<Standard_Integer> m_currentSiblings;   //!< All siblings for current.
  Standard_Boolean                  m_bReversed;         //!< Order of traverse.

};

#endif
