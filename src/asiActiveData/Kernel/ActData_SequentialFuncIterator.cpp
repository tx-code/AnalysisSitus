//-----------------------------------------------------------------------------
// Created on: November 2013
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
#include <ActData_SequentialFuncIterator.h>

// Active Data includes
#include <ActData_BaseTreeFunction.h>
#include <ActData_TreeFunctionPriority.h>

// OCCT includes
#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>
#include <TDF_MapIteratorOfLabelMap.hxx>
#include <TFunction_DoubleMapIteratorOfDoubleMapOfIntegerLabel.hxx>
#include <TFunction_GraphNode.hxx>
#include <TFunction_IFunction.hxx>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

//! Default constructor.
ActData_SequentialFuncIterator::ActData_SequentialFuncIterator()
{}

//! Complete constructor.
//! \param theAccess [in] key object to access Data Model internals.
ActData_SequentialFuncIterator::ActData_SequentialFuncIterator(const TDF_Label& theAccess)
{
  this->Init(theAccess);
}

//! Initializes iterator with the key object to access Data Model. As a result,
//! iterator contains the first portion of independent Functions which do not
//! have any predecessors.
//! \param theAccess [in] key object to access Data Model internals.
void ActData_SequentialFuncIterator::Init(const TDF_Label& theAccess)
{
  /* ~~~~~~~~~~~~~~~~~~~~
   *  Initialize members
   * ~~~~~~~~~~~~~~~~~~~~ */

  m_currentFunctions.Clear();

  // Get the scope of Functions
  m_scope = TFunction_Scope::Set(theAccess);

  /* ~~~~~~~~~~~~~~~~~~~~~
   *  Find root Functions
   * ~~~~~~~~~~~~~~~~~~~~~ */

  // Two collections for low- and high-priority Functions
  TDF_LabelList normalFunctions, highFunctions;

  // Find the roots
  TFunction_DoubleMapIteratorOfDoubleMapOfIntegerLabel fit( m_scope->GetFunctions() );
  for ( ; fit.More(); fit.Next() )
  {
    const TDF_Label& L = fit.Key2();

    TFunction_IFunction iFunction(L);

#if defined COUT_DEBUG
    Handle(ActData_BaseTreeFunction) funcBase =
      Handle(ActData_BaseTreeFunction)::DownCast( Handle(ActData_TreeFunctionDriver)::DownCast( iFunction.GetDriver() )->GetFunction() );
    //
    std::cout << ">>> Next function: " << funcBase->DynamicType()->Name() << std::endl;
#endif

    Handle(TFunction_GraphNode) graphNode = iFunction.GetGraphNode();
    TFunction_ExecutionStatus status = graphNode->GetStatus();

    // Check whether the Function is a root Function
    if ( !graphNode->GetPrevious().IsEmpty() )
      continue;

    // We consider only "not executed" Functions
    if ( status != TFunction_ES_NotExecuted )
      continue;

    // Check priority
    if ( this->isHighPriority(L) )
      highFunctions.Append(L);
    else
      normalFunctions.Append(L);
  }

  /* ~~~~~~~~~~~~~~
   *  Finalization
   * ~~~~~~~~~~~~~~ */

  m_currentFunctions.Append(highFunctions);
  m_currentFunctions.Append(normalFunctions);
}

//! Switches iterator to the next list of current Functions.
void ActData_SequentialFuncIterator::Next()
{
  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *  Each current has some successor -> we need to get it
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

  TDF_LabelMap nextCurrentsNormal, nextCurrentsHigh;
  for ( TDF_ListIteratorOfLabelList lit(m_currentFunctions); lit.More(); lit.Next() )
  {
    const TDF_Label& L = lit.Value();
    TFunction_IFunction iFunction(L);

#if defined COUT_DEBUG
    {
      Handle(ActData_BaseTreeFunction) funcBase =
        Handle(ActData_BaseTreeFunction)::DownCast( Handle(ActData_TreeFunctionDriver)::DownCast( iFunction.GetDriver() )->GetFunction() );
      //
      std::cout << ">>> Current function: " << funcBase->DynamicType()->Name() << std::endl;
    }
#endif

    Handle(TFunction_GraphNode) graphNode = iFunction.GetGraphNode();
    const TColStd_MapOfInteger& next      = graphNode->GetNext();
    TFunction_ExecutionStatus   status    = graphNode->GetStatus();

    // Not all current Functions have been iterated yet
    if ( status == TFunction_ES_NotExecuted || status == TFunction_ES_Executing )
    {
      // Check priority
      if ( this->isHighPriority(L) )
        nextCurrentsHigh.Add(L);
      else
        nextCurrentsNormal.Add(L);

      continue;
    }
    else if ( status == TFunction_ES_WrongDefinition || status == TFunction_ES_Failed )
      continue;

    /* ~~~~~~~~~~~~~~~~~~~~
     *  Add next Functions
     * ~~~~~~~~~~~~~~~~~~~~ */

    for ( TColStd_MapIteratorOfMapOfInteger nit(next); nit.More(); nit.Next() )
    {
      const Standard_Integer IDNext = nit.Key();
      const TDF_Label& LNext = m_scope->GetFunctions().Find1(IDNext);

      // A previous Function is "succeeded", check status of next Functions and
      // all other previous Functions of the next Functions

      // Check status, it should be "not executed"
      TFunction_IFunction iNextFunction(LNext);

#if defined COUT_DEBUG
      {
        Handle(ActData_BaseTreeFunction) funcBase =
          Handle(ActData_BaseTreeFunction)::DownCast( Handle(ActData_TreeFunctionDriver)::DownCast( iNextFunction.GetDriver() )->GetFunction() );
        //
        std::cout << ">>>\t Next function: " << funcBase->DynamicType()->Name() << std::endl;
      }
#endif

      Handle(TFunction_GraphNode) nextGraphNode = iNextFunction.GetGraphNode();
      TFunction_ExecutionStatus nextStatus = nextGraphNode->GetStatus();
      if ( nextStatus != TFunction_ES_NotExecuted && nextStatus != TFunction_ES_Executing )
        continue;

      // Check all previous functions: all of them should be "succeeded"
      Standard_Boolean isPrevSucceeded = Standard_True;
      const TColStd_MapOfInteger& prevOfNext = nextGraphNode->GetPrevious();
      for ( TColStd_MapIteratorOfMapOfInteger pit(prevOfNext); pit.More(); pit.Next())
      {
        const Standard_Integer IDPrevOfNext = pit.Key();
        const TDF_Label& LPrevOfNext = m_scope->GetFunctions().Find1(IDPrevOfNext);

        Handle(TFunction_GraphNode) GPrevOfNext;
        LPrevOfNext.FindAttribute(TFunction_GraphNode::GetID(), GPrevOfNext);
        TFunction_ExecutionStatus prevStatus = GPrevOfNext->GetStatus();
        if ( prevStatus != TFunction_ES_Succeeded )
        {
          isPrevSucceeded = Standard_False;
          break;
        }
      }

      if ( !isPrevSucceeded )
        continue;

      // Check priority
      if ( this->isHighPriority(LNext) )
        nextCurrentsHigh.Add(LNext);
      else
        nextCurrentsNormal.Add(LNext);
    }
  }

  /* ~~~~~~~~~~~~~~
   *  Finalization
   * ~~~~~~~~~~~~~~ */

  m_currentFunctions.Clear();
  for ( TDF_MapIteratorOfLabelMap mit(nextCurrentsHigh); mit.More(); mit.Next() )
    m_currentFunctions.Append( mit.Key() );
  for ( TDF_MapIteratorOfLabelMap mit(nextCurrentsNormal); mit.More(); mit.Next() )
    m_currentFunctions.Append( mit.Key() );
}

//! Returns true if there is some remaining Tree Function to execute.
//! \return true/false.
Standard_Boolean ActData_SequentialFuncIterator::More() const
{
  TFunction_DoubleMapIteratorOfDoubleMapOfIntegerLabel fit( m_scope->GetFunctions() );
  for ( ; fit.More(); fit.Next() )
  {
    const TDF_Label& L = fit.Key2();
    if ( this->GetStatus(L) == TFunction_ES_NotExecuted )
      return Standard_True;
  }

  return Standard_False;
}

//! Returns the list of current Functions to be executed.
//! \return list of current Functions.
const TDF_LabelList& ActData_SequentialFuncIterator::Current() const
{
  return m_currentFunctions;
}

//! Accessor for the execution status of the Function settled on the given
//! Label.
//! \param theFunc [in] Function Label.
//! \return execution status.
TFunction_ExecutionStatus
  ActData_SequentialFuncIterator::GetStatus(const TDF_Label& theFunc) const
{
  TFunction_IFunction iFunction(theFunc);
  return iFunction.GetGraphNode()->GetStatus();
}

//! Sets the passed execution status for the Tree Function settled on the given
//! Label.
//! \param theFunc [in] Function Label.
//! \param theStatus [in] status to set.
void ActData_SequentialFuncIterator::SetStatus(const TDF_Label& theFunc,
                                               const TFunction_ExecutionStatus theStatus) const
{
  TFunction_IFunction iFunction(theFunc);
  iFunction.GetGraphNode()->SetStatus(theStatus);
}

//! Checks whether the passed Label represents Tree Function of high priority.
//! \param theFunc [in] root Label for Function.
//! \return true/false.
Standard_Boolean ActData_SequentialFuncIterator::isHighPriority(const TDF_Label& theFunc) const
{
  TFunction_IFunction iFunction(theFunc);

  // Access Tree Function
  Handle(ActData_TreeFunctionDriver) FuncDriver =
    Handle(ActData_TreeFunctionDriver)::DownCast( iFunction.GetDriver() );
  Handle(ActData_BaseTreeFunction) Func =
    Handle(ActData_BaseTreeFunction)::DownCast( FuncDriver->GetFunction() );

  // Check priority
  return Func->Priority() == TreeFunctionPriority_High;
}
