//-----------------------------------------------------------------------------
// Created on: May 2012
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
#include <ActData_DependencyAnalyzer.h>

// Active Data includes
#include <ActData_ParameterFactory.h>
#include <ActData_Utils.h>

// OCCT includes
#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TFunction_IFunction.hxx>
#include <TFunction_Scope.hxx>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------
// Construction methods
//-----------------------------------------------------------------------------

//! Constructor accepting Data Model.
//! \param theModel [in] Data Model instance.
ActData_DependencyAnalyzer::ActData_DependencyAnalyzer(const Handle(ActData_BaseModel)& theModel)
: Standard_Transient(), m_model(theModel)
{}

//-----------------------------------------------------------------------------
// Interface methods for topological analysis
//-----------------------------------------------------------------------------

void ActData_DependencyAnalyzer::DetectLoops(const LoopStrategy theStrategy)
{
  /* =====================================================
   *  Access dependency graph and perform initializations
   * ===================================================== */

  m_resultState = GraphState_Undefined;

  TDF_Label aRootLab = m_model->RootLabel();
  Handle(TFunction_Scope) aScope = TFunction_Scope::Set(aRootLab);
  const ActData_Graph& aGraph = aScope->GetFunctions();

  if ( aGraph.IsEmpty() )
  {
    m_resultState = GraphState_NoGraph;
    return;
  }

  /* ==============================================
   *  Depending on the chosen strategy, launch the
   *  appropriate algorithm
   * ============================================== */

  switch ( theStrategy )
  {
    case LoopStrategy_AD_DFS:
      this->performAD_DFS(aGraph);
      break;

    case LoopStrategy_Tarjan:
      this->performTarjan(aGraph);
      break;

    default:
      Standard_ProgramError::Raise("Unexpected detection strategy");
  }
}

//-----------------------------------------------------------------------------
// Interface methods for accessing the results of analysis
//-----------------------------------------------------------------------------

//! Accessor for the resulting status.
//! \return status.
Standard_Integer ActData_DependencyAnalyzer::GetResultStatus() const
{
  return m_resultState;
}

//! Accessor for the detected cyclic Parameters. If isEnriched flag is set as
//! TRUE (default value), all children graph nodes for all cyclic Parameters
//! are also included into the resulting collection. Such behavior is actually
//! desirable in most cases, as we normally want our dependency graph to remain
//! consistent after removal of cyclical groups. Indeed, when you simply reject
//! strongly connected components from a graph, you can easily break its
//! connectivity and obtain a topological structure including non-calculatable
//! nodes. E.g:
//! <pre>
//!                                  ###################################
//! X -> Y -> Z -> X         --->    ###      LOOP IS REJECTED       ###
//!           |              --->    ### Item A cannot be calculated ###
//!           |              --->    ###################################
//!           + -> A -> B            ... ? ... -> A -> B
//! </pre>
//! Here we have {X, Y, Z} forming a loop and detected as cyclic so. However,
//! not enumerated node A cannot be kept in the execution graph as it depends
//! on node Z which in-turn cannot be calculated due to its presence in a
//! loop. Similarly, we cannot keep node B and, generally speaking, any
//! successors of any cyclic node. Including these additional nodes into the
//! resulting collection is what enriching functionality actually does.
//! \param isEnriched [in] indicates whether to perform enriching.
//! \return collection of cyclic Parameters optionally enriched by the
//!         dependent non-cyclic ones.
Handle(ActAPI_HParameterList)
  ActData_DependencyAnalyzer::CyclicParameters(const Standard_Boolean isEnriched) const
{
  /* =====================
   *  Check prerequisites
   * ===================== */

  if ( !this->HasLoops() )
    return new ActAPI_HParameterList;

  /* =======================================================
   *  Complete collection of Parameter ID in ENRICHING mode
   * ======================================================= */

  TColStd_PackedMapOfInteger anEntireIDs; anEntireIDs = m_cyclicParamIDs;
  if ( isEnriched )
  {
    TColStd_MapIteratorOfPackedMapOfInteger aCyclicIt(m_cyclicParamIDs);
    for ( ; aCyclicIt.More(); aCyclicIt.Next() )
    {
      Standard_Integer aNextCyclicID = aCyclicIt.Key();
      this->cumulateChildParameters(aNextCyclicID, anEntireIDs);
    }
  }

  Handle(TFunction_Scope) aScope = TFunction_Scope::Set( m_model->RootLabel() );
  const ActData_Graph& aGraph = aScope->GetFunctions();

  /* =======================================================
   *  Dispatch unique IDs to their corresponding Parameters
   * ======================================================= */

  Handle(ActAPI_HParameterList) aParamList = new ActAPI_HParameterList;
  TColStd_MapIteratorOfPackedMapOfInteger aMapIt(anEntireIDs);
  for ( ; aMapIt.More(); aMapIt.Next() )
  {
    Standard_Boolean isUndefinedType;
    Standard_Integer anID = aMapIt.Key();
    TDF_Label aLab = aGraph.Find1(anID);
    aParamList->Append( ActData_ParameterFactory::NewParameterSettle(aLab, isUndefinedType) );
  }

  return aParamList;
}

//! Returns true if analysis tool is initialized, false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_DependencyAnalyzer::IsInitialized() const
{
  return !(m_resultState & GraphState_Undefined);
}

//! Returns true if any algorithms has been performed. If nothing was
//! actually done (due to missing DetectXXX invocation, empty graph or
//! whatever else), returns false.
//! \return true/false.
Standard_Boolean ActData_DependencyAnalyzer::IsDone() const
{
  return !(m_resultState & GraphState_Undefined) &&
         !(m_resultState & GraphState_NoGraph);
}

//! Returns true if the dependency graph is topologically OK, i.e. no problems
//! were detected.
//! \return true/false.
Standard_Boolean ActData_DependencyAnalyzer::IsOk() const
{
  return (m_resultState & GraphState_Ok) > 0;
}

//! Returns true if the dependency graph contains any non-trivial (more than
//! one element in a group) Strongly Connected Components, i.e. if item Y
//! can be reached from X, than item X can be reached from Y. Therefore,
//! your dependency graph contains loops and cannot be executed as-is,
//! because it will lead to infinite iterations over the dependent Tree
//! Functions.
//! \return true/false.
Standard_Boolean ActData_DependencyAnalyzer::HasLoops() const
{
  return (m_resultState & GraphState_HasLoops) > 0;
}

//-----------------------------------------------------------------------------
// Internal methods for topological analysis
//-----------------------------------------------------------------------------

//! Performs AD_DFS algorithm for searching of the Strongly Connected Components
//! in the initial oriented dependency graph.
//! \param theGraph [in] dependency graph to process.
void ActData_DependencyAnalyzer::performAD_DFS(const ActData_Graph& theGraph)
{
  FuncIDStackList aLoopSeq;
  ActData_TFuncGraphIterator aGraphIt(theGraph);
  for ( ; aGraphIt.More(); aGraphIt.Next() )
  {
    /* =============================================================
     *  Access current root node (all nodes will play a root's role
     *  in an unpredictable order when use GraphIterator) and
     *  prepare the corresponding IFunction interface (Data Cursor)
     *  shipped with OCCT
     * ============================================================= */

    Standard_Integer aNextRootID = aGraphIt.Key1();
    const TDF_Label& aNextRootLab = aGraphIt.Key2();
    TFunction_IFunction aNextIRoot(aNextRootLab);

    /* =================================================================
     *  Prepare a working stack which will store the actually traversed
     *  items for each recursive step of the AD_DFS-algorithm
     * ================================================================= */

    // Create a new stack for the next path being checked
    FuncIDStack aCumulStack;
    aCumulStack.Prepend(aNextRootID); // Push the starting graph node in the stack

    this->checkLoopRecursiveAD_DFS(aNextIRoot.GetGraphNode(), aCumulStack, aLoopSeq);

    aCumulStack.RemoveFirst(); // Just for symmetry ;) -- may be useful in future
                               // if we want to continue working with the stack somehow
  }

  /* ===============================================
   *  Populate the list of cyclic Parameters if any
   * =============================================== */

  // Collect unique IDs only
  FuncIDStackList::Iterator aLoopSeqIt(aLoopSeq);
  for ( ; aLoopSeqIt.More(); aLoopSeqIt.Next() )
  {
    FuncIDStack aNextStack = aLoopSeqIt.Value();

#if defined ACT_DEBUG && defined COUT_DEBUG
    std::cout << "=====> Next stack:" << std::endl;
#endif

    FuncIDStack::Iterator aStackIt(aNextStack);
    for ( ; aStackIt.More(); aStackIt.Next() )
    {
      m_cyclicParamIDs.Add( aStackIt.Value() );

#if defined ACT_DEBUG && defined COUT_DEBUG
      TDF_Label aLab = theGraph.Find1( aStackIt.Value() );
      TCollection_AsciiString anEntry = ActData_Utils::GetEntry(aLab);
      std::cout << "Next Entry: " << anEntry << std::endl;
#endif
    }
  }

  /* ======================
   *  Set resulting status
   * ====================== */

  if ( aLoopSeq.IsEmpty() && !this->IsInitialized() )
    m_resultState = GraphState_Ok;
  else
    m_resultState |= GraphState_HasLoops;
}

//! Performs Tarjan's algorithms for searching of the Strongly Connected
//! Components in the initial oriented dependency graph.
//! \param theGraph [in] dependency graph to process.
void ActData_DependencyAnalyzer::performTarjan(const ActData_Graph& ActData_NotUsed(theGraph))
{
  // TODO: NYI Tarjan's algorithm of SCC searching
  Standard_ProgramError::Raise("Not yet implemented");
}

//! Internal routine providing recursive analysis of the given dependency web
//! starting from the passed graph node. This is a kernel of AD_DFS algorithm.
//! Once a loop is detected (next traversed item is found in the traversal
//! stack), this routine will register the actual loop stack in the output
//! collection of loops.
//! \param theNode [in] graph node to check.
//! \param theStack [in/out] stack of currently traversed Functions. This
//!        stack we check on repetitions on each iteration.
//! \param theLoopSeq [out] resulting collection of stacks representing the
//!        actual loops.
void ActData_DependencyAnalyzer::checkLoopRecursiveAD_DFS(const Handle(TFunction_GraphNode)& theNode,
                                                       FuncIDStack& theStack,
                                                       FuncIDStackList& theLoopSeq) const
{
  if ( theNode.IsNull() )
    return;

  Handle(TFunction_Scope) aScope = TFunction_Scope::Set( theNode->Label() );

  // Iterate over the NEXT Labels of the passed Graph Node
  const TColStd_MapOfInteger& aMapOfNexts = theNode->GetNext();
  TColStd_MapIteratorOfMapOfInteger aNextsIt(aMapOfNexts);
  for ( ; aNextsIt.More(); aNextsIt.Next() )
  {
    Standard_Integer aNext = aNextsIt.Key();

    // Prepare a stack for the actual loop
    FuncIDStack aHBStack;

    // Check if Next function is already in the stack. It means that
    // we have found the loop
    FuncIDStack::Iterator aStackIt(theStack);
    for ( ; aStackIt.More(); aStackIt.Next() )
    {
      Standard_Integer aStackedID = aStackIt.Value();
      aHBStack.Prepend(aStackedID);
      if ( aStackedID == aNext )
      {
        theLoopSeq.Append(aHBStack); // Push stack to the output
        return;
      }
    }

    // Loop is not found at this stage, so push new function to the stack
    theStack.Prepend(aNext);
    
    if ( !aScope->HasFunction(aNext) )
      continue;

    // Continue analysis recursively
    const TDF_Label& aNextDriven = aScope->GetFunction(aNext);
    TFunction_IFunction anIFunc(aNextDriven);
    this->checkLoopRecursiveAD_DFS(anIFunc.GetGraphNode(), theStack, theLoopSeq);

    // Rollback the stack in case when no loops are found
    theStack.RemoveFirst();
  }
}

void ActData_DependencyAnalyzer::cumulateChildParameters(const Standard_Integer theRootID,
                                                         TColStd_PackedMapOfInteger& theChildIDs) const
{
  Handle(TFunction_Scope) aScope = TFunction_Scope::Set( m_model->RootLabel() );
  const ActData_Graph& aGraph = aScope->GetFunctions();
  TDF_Label aBaseLab = aGraph.Find1(theRootID);

  TFunction_IFunction anIFunc(aBaseLab);
  Handle(TFunction_GraphNode) aGraphNode = anIFunc.GetGraphNode();

  const TColStd_MapOfInteger& aMapOfNexts = aGraphNode->GetNext();
  TColStd_MapIteratorOfMapOfInteger aNextsIt(aMapOfNexts);
  for ( ; aNextsIt.More(); aNextsIt.Next() )
  {
    Standard_Integer aNextID = aNextsIt.Key();
    if ( theChildIDs.Contains(aNextID) )
      continue;

    theChildIDs.Add(aNextID);
    this->cumulateChildParameters(aNextID, theChildIDs);
  }
}
