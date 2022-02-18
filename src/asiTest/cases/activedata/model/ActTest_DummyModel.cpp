//-----------------------------------------------------------------------------
// Created on: 17 February 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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
#include <ActTest_DummyModel.h>

// ACT Unit Testing includes
#include <ActTest_DummyTreeFunction.h>
#include <ActTest_StubANode.h>
#include <ActTest_StubBNode.h>
#include <ActTest_StubCNode.h>
#include <ActTest_StubMeshNode.h>

// Active Data includes
#include <ActData_BoolVarNode.h>
#include <ActData_BoolVarPartition.h>
#include <ActData_CAFConverter.h>
#include <ActData_IntVarNode.h>
#include <ActData_IntVarPartition.h>
#include <ActData_NodeFactory.h>
#include <ActData_RealEvaluatorFunc.h>
#include <ActData_RealVarNode.h>
#include <ActData_RealVarPartition.h>
#include <ActData_Utils.h>

//-----------------------------------------------------------------------------
// Register involved Node types
//-----------------------------------------------------------------------------

REGISTER_NODE_TYPE(ActTest_StubANode)
REGISTER_NODE_TYPE(ActTest_StubBNode)
REGISTER_NODE_TYPE(ActTest_StubCNode)
REGISTER_NODE_TYPE(ActTest_StubMeshNode)
REGISTER_NODE_TYPE(ActData_BoolVarNode)
REGISTER_NODE_TYPE(ActData_IntVarNode)
REGISTER_NODE_TYPE(ActData_RealVarNode)

//-----------------------------------------------------------------------------
// Implementation of Data Model for testing purposes
//-----------------------------------------------------------------------------

//! Default constructor.
ActTest_DummyModel::ActTest_DummyModel()
{}

//! Registers user's Partitions in the Data Model. We use smart macro here
//! to make thing simpler.
void ActTest_DummyModel::initPartitions()
{
  // Partition Type <-> Partition internal ID
  REGISTER_PARTITION(ActTest_StubAPartition,    Partition_A);
  REGISTER_PARTITION(ActTest_StubBPartition,    Partition_B);
  REGISTER_PARTITION(ActTest_StubCPartition,    Partition_C);
  REGISTER_PARTITION(ActTest_StubMeshPartition, Partition_Mesh);
  REGISTER_PARTITION(ActData_RealVarPartition,  Partition_RealVar);
  REGISTER_PARTITION(ActData_IntVarPartition,   Partition_IntVar);
  REGISTER_PARTITION(ActData_BoolVarPartition,  Partition_BoolVar);
}

//! Registers user's Tree Functions in the Data Model. You're free to omit
//! Tree Functions at all if you dislike the concept of active Data Model.
//! Anyway, you state the latter fact by keeping this method empty.
void ActTest_DummyModel::initFunctionDrivers()
{
  REGISTER_TREE_FUNCTION(ActTest_DummyTreeFunction);
  REGISTER_TREE_FUNCTION(ActData_RealEvaluatorFunc);
}

//! Performs initial population of the Data Model. This method will be
//! executed just after NewEmpty invocation ensuring that even just
//! created Models do have the application-consistent structure. If you
//! do not need to perform any initializations of Nodal structure, just
//! keep the implementation empty.
//! \return true in case of success, false -- otherwise. Notice, that
//!         resulting value will be returned by NewEmpty basic method
//!         as well, so returning FALSE will be processed as Model
//!         creation failure.
Standard_Boolean ActTest_DummyModel::populate()
{
  return Standard_True;
}

//! Returns a dedicated Partition for Nodal Variables.
//! \param theVarType [in] Variable type to return the dedicated
//!        Partition for.
//! \return Variable Partition.
Handle(ActAPI_IPartition)
  ActTest_DummyModel::getVariablePartition(const VariableType& theVarType) const
{
  switch ( theVarType )
  {
    case Variable_Real:
      return this->Partition(Partition_RealVar);
    case Variable_Int:
      return this->Partition(Partition_IntVar);
    case Variable_Bool:
      return this->Partition(Partition_BoolVar);
    default:
      break;
  }
  return NULL;
}

//! Accessor for the root Data Node of the Model's hierarchy.
//! \return root Data Node.
Handle(ActAPI_INode) ActTest_DummyModel::getRootNode() const
{
  ActTest_StubAPartition::Iterator anIt( this->Partition(Partition_A) );
  return ( anIt.More() ? anIt.Value() : NULL );
}

//! Populates the passed collectons of references to pass out-scope filtering
//! in Copy/Paste operation.
//! \param FuncGUIDs [in/out] Function GUIDs to pass.
//! \param Refs [in/out] Reference Parameters to pass.
void ActTest_DummyModel::invariantCopyRefs(ActAPI_FuncGUIDStream& FuncGUIDs,
                                           ActAPI_ParameterLocatorStream& Refs) const
{
  for ( ActAPI_FuncGUIDList::Iterator it( *m_copyGUIDs.List.operator->() ); it.More(); it.Next() )
    FuncGUIDs << it.Value();

  for ( ActAPI_ParameterLocatorList::Iterator it( *m_copyRefs.List.operator->() ); it.More(); it.Next() )
    Refs << it.Value();
}

//! Returns the version of your custom Data Model. This version number will
//! be stored in the persistent form of your Data Model and retrieved by
//! Load functionality. It allows you to implement your versioning
//! mechanism, i.e. compatibility conversions, etc.
//! \return the current version you define for your Data Model.
Standard_Integer ActTest_DummyModel::actualVersionApp()
{
  return 10;
}

//! Allows you to perform conversion from the older version of Data Model
//! to the actual one. If you are not planning to have any kind of
//! backward compatibility for your Data Model, you should return NULL.
//! \return properly initialized CAF converter or NULL.
Handle(ActData_CAFConverter) ActTest_DummyModel::converterApp()
{
  return NULL;
}
