//-----------------------------------------------------------------------------
// Created on: November 2012
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
#include <ActData_DependencyGraph.h>

// OCCT includes
#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <TDF_Tool.hxx>
#include <TFunction_IFunction.hxx>
#include <TFunction_Scope.hxx>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------
// Construction methods
//-----------------------------------------------------------------------------

//! Default constructor.
ActData_DependencyGraph::ActData_DependencyGraph()
: Standard_Transient()
{
}

//! Complete constructor.
//! \param theModel [in] Data Model to build the Dependency Graph auxiliary
//!        representation for.
ActData_DependencyGraph::ActData_DependencyGraph(const Handle(ActData_BaseModel)& theModel)
: Standard_Transient()
{
  this->Build(theModel);
}

//-----------------------------------------------------------------------------
// Kernel methods
//-----------------------------------------------------------------------------

//! Build graph structure from the Dependency Graph declared by the passed
//! Data Model.
//! \param theModel [in] Data Model to build graph for.
void ActData_DependencyGraph::Build(const Handle(ActData_BaseModel)& theModel)
{
  m_model = theModel;

  // Access global Function Scope
  TDF_Label aRootLab = m_model->RootLabel();
  Handle(TFunction_Scope) aScope;
  if ( !aRootLab.Root().FindAttribute(TFunction_Scope::GetID(), aScope) ) 
    return;

  // Access OCAF-specific graph in form of "Function ID" <-> "TDF Label"
  // association map
  const ActData_Graph& aGraph = aScope->GetFunctions();

  // Iterate over OCAF-specific graph populating all necessary internal
  // structures representing OCAF-independent graph model
  ActData_TFuncGraphIterator aGraphIt(aGraph);
  for ( ; aGraphIt.More(); aGraphIt.Next() )
  {
    // Get next graph node
    Standard_Integer aNextRootID = aGraphIt.Key1();
    const TDF_Label& aNextRootLab = aGraphIt.Key2();

#if defined COUT_DEBUG
    TCollection_AsciiString aNextRootEntry;
    TDF_Tool::Entry(aNextRootLab, aNextRootEntry);
    //
    std::cout << "(id, label) = (" << aNextRootID << ", " << aNextRootEntry.ToCString() << ")" << std::endl;
#endif

    // Settle TFunction interface cursor
    TFunction_IFunction anIRoot(aNextRootLab);

    // Add vertex for current root
    this->registerVertex(aNextRootID, VertexData( anIRoot.GetDriver() ), aNextRootLab);

    // Build graph starting from this node
    this->buildFrom(aNextRootLab, aNextRootID);
  }
}

//! Returns Tree Function Parameter corresponding to the given vertex ID.
//! \param theVertexID [in] vertex ID to return Tree Function Parameter for.
//! \return Tree Function Parameter.
Handle(ActData_TreeFunctionParameter)
  ActData_DependencyGraph::FunctionByVertex(const Standard_Integer theVertexID) const
{
  // Access global Function Scope
  TDF_Label aRootLab = m_model->RootLabel();
  Handle(TFunction_Scope) aScope;
  if ( !aRootLab.Root().FindAttribute(TFunction_Scope::GetID(), aScope) ) 
    return NULL;

  // Access Tree Function Parameter
  Standard_Boolean isUndefinedType;
  const TDF_Label& aFuncRoot = aScope->GetFunction(theVertexID);
  return ActParamTool::AsTreeFunction( ActParamTool::NewParameterSettle(aFuncRoot, isUndefinedType) );
}

//! Returns vertex ID by Tree Function Parameter Label.
//! \param theLab [in] root Label of the Tree Function Parameter.
//! \return vertex index or -1 if nothing is found.
Standard_Integer
  ActData_DependencyGraph::VertexByLabel(const TDF_Label& theLab) const
{
  if ( !m_labelVerts.IsBound(theLab) )
    return -1;

  return m_labelVerts.Find(theLab);
}

//! Returns vertex ID by Tree Function Parameter.
//! \param theParam [in] Tree Function Parameter.
//! \return vertex index or -1 if nothing is found.
Standard_Integer
  ActData_DependencyGraph::VertexByFunction(const Handle(ActData_TreeFunctionParameter)& theParam) const
{
  return this->VertexByLabel( theParam->RootLabel() );
}

//! Returns vertex data for the given vertex ID.
//! \param theVertexID [in] vertex ID.
//! \return vertex data.
ActData_DependencyGraph::VertexData
  ActData_DependencyGraph::DataByVertex(const Standard_Integer theVertexID) const
{
  if ( !m_vertices.IsBound(theVertexID) )
    return VertexData();

  return m_vertices.Find(theVertexID);
}

//! Builds Dependency Graph starting from the passed node.
//! \param theLNode [in] Label of the graph node to start build process from.
//! \param theID [in] ID of the corresponding model graph vertex.
void ActData_DependencyGraph::buildFrom(const TDF_Label& theLNode,
                                        const Standard_Integer theID)
{
  // Access global scope
  Handle(TFunction_Scope) aScope = TFunction_Scope::Set(theLNode);

  // Settle TFunction interface cursor
  TFunction_IFunction anIRoot(theLNode);

  // Iterate over the next graph nodes
  const TColStd_MapOfInteger& aMapOfNexts = anIRoot.GetGraphNode()->GetNext();
  TColStd_MapIteratorOfMapOfInteger aNextsIt(aMapOfNexts);
  for ( ; aNextsIt.More(); aNextsIt.Next() )
  {
    Standard_Integer aNextID = aNextsIt.Key();
    const TDF_Label& aNextDriven = aScope->GetFunction(aNextID);
    TFunction_IFunction aNextIFunc(aNextDriven);

    // Add graph vertex
    this->registerVertex(aNextID, VertexData( aNextIFunc.GetDriver() ), aNextDriven);

    // Add graph edges
    if ( m_edges.Add( OriEdge(theID, aNextID) ) )
      this->buildFrom(aNextDriven, aNextID); // Continue recursively
  }
}

//! Adds vertex to the internal map.
//! \param theID [in] vertex ID.
//! \param theVData [in] vertex data.
//! \param theFuncRoot [in] root OCAF Label of Tree Function Parameter.
void ActData_DependencyGraph::registerVertex(const Standard_Integer theID,
                                             const VertexData& theVData,
                                             const TDF_Label& theFuncRoot)
{
  if ( !m_labelVerts.IsBound(theFuncRoot) )
    m_labelVerts.Bind(theFuncRoot, theID);

  if ( !m_vertices.IsBound(theID) )
    m_vertices.Bind(theID, theVData);
}
