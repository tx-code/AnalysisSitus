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
#include <ActTest_BaseModel.h>

// asiTestEngine includes
#include <asiTestEngine_Launcher.h>
#include <asiTestEngine_Utils.h>

// Active Data unit tests
#include <ActTest_DummyModel.h>
#include <ActTest_StubANode.h>
#include <ActTest_DummyTreeFunction.h>

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_BaseNode.h>
#include <ActData_BasePartition.h>
#include <ActData_BaseTreeFunction.h>
#include <ActData_DependencyAnalyzer.h>
#include <ActData_FuncExecutionCtx.h>
#include <ActData_IntParameter.h>
#include <ActData_NodeFactory.h>
#include <ActData_ParameterFactory.h>
#include <ActData_ShapeParameter.h>
#include <ActData_TreeFunctionParameter.h>
#include <ActData_Utils.h>
#include <STD/ActData_BoolVarNode.h>
#include <STD/ActData_BoolVarPartition.h>
#include <STD/ActData_IntVarNode.h>
#include <STD/ActData_IntVarPartition.h>
#include <STD/ActData_RealEvaluatorFunc.h>
#include <STD/ActData_RealVarNode.h>
#include <STD/ActData_RealVarPartition.h>
#include <Tools/ActData_GraphToDot.h>

// ACT Algo includes
#include <ActAux_Env.h>

#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------
// PERSISTENCE: Business logic
//-----------------------------------------------------------------------------

//! Performs test on HasOpenCommand method.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelPersistence::testHasOpenCommand(const int asiTestEngine_NotUsed(funcID))
{
  Handle(ActAPI_IModel) M = new ActTest_DummyModel;

  TEST_VERIFY( M->NewEmpty() )
  TEST_VERIFY( !M->HasOpenCommand() )

  M->OpenCommand();
  TEST_VERIFY( M->HasOpenCommand() )
  M->CommitCommand();

  TEST_VERIFY( !M->HasOpenCommand() )

  return outcome().success();
}

//! Performs test on NewEmpty method.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelPersistence::newEmptyModel(const int asiTestEngine_NotUsed(funcID))
{
  Handle(ActAPI_IModel) M = new ActTest_DummyModel;

  TEST_VERIFY( !M->IsInitialized() )
  TEST_VERIFY( M->GetVersionStatus() == ActAPI_IModel::Version_Undefined )
  TEST_VERIFY( M->NewEmpty() )
  TEST_VERIFY( M->IsInitialized() )
  TEST_VERIFY( M->GetVersionStatus() == ActAPI_IModel::Version_Ok )

  return outcome().success();
}

//! Performs test on Open method.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelPersistence::loadModel(const int asiTestEngine_NotUsed(funcID))
{
  Handle(ActAPI_IModel) M = new ActTest_DummyModel;
  TEST_VERIFY( M->NewEmpty() )

  Handle(ActTest_DummyModel) aDummyModel = Handle(ActTest_DummyModel)::DownCast(M);
  
  /* ==================================
   *  Create sample hierarchy of Nodes
   * ================================== */

  // Construct detached Node A
  Handle(ActTest_StubANode)
    aNodeA = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );

  // Construct detached Node B
  Handle(ActTest_StubANode)
    aNodeB = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );

  // Construct detached Node C
  Handle(ActTest_StubANode)
    aNodeC = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );

  aDummyModel->OpenCommand();

  // Attach Nodes to CAF: DETACHED --> ATTACHED
  aDummyModel->StubAPartition()->AddNode(aNodeA);
  aDummyModel->StubAPartition()->AddNode(aNodeB);
  aDummyModel->StubAPartition()->AddNode(aNodeC);

  // Prepare initial data to populate the Nodes
  TopoDS_Shape  aNodeA_ShapeA = asiTestEngine_Utils::RandomShape();
  TopoDS_Shape  aNodeA_ShapeB = asiTestEngine_Utils::RandomShape();
  Standard_Real aNodeA_Val    = asiTestEngine_Utils::RandomReal();
  TopoDS_Shape  aNodeB_ShapeA = asiTestEngine_Utils::RandomShape();
  TopoDS_Shape  aNodeB_ShapeB = asiTestEngine_Utils::RandomShape();
  Standard_Real aNodeB_Val    = asiTestEngine_Utils::RandomReal();
  TopoDS_Shape  aNodeC_ShapeA = asiTestEngine_Utils::RandomShape();
  TopoDS_Shape  aNodeC_ShapeB = asiTestEngine_Utils::RandomShape();
  Standard_Real aNodeC_Val    = asiTestEngine_Utils::RandomReal();

  // Initialize Data Nodes: ATTACHED --> WELL-FORMED
  aNodeA->Init(aNodeA_ShapeA, aNodeA_ShapeB, aNodeA_Val);
  aNodeB->Init(aNodeB_ShapeA, aNodeB_ShapeB, aNodeB_Val);
  aNodeC->Init(aNodeC_ShapeA, aNodeC_ShapeB, aNodeC_Val);

  // Verify Node statuses
  TEST_VERIFY( aNodeA->IsWellFormed() )
  TEST_VERIFY( aNodeB->IsWellFormed() )
  TEST_VERIFY( aNodeC->IsWellFormed() )

  // Prepare primitive USER tree
  aNodeA->AddChildNode(aNodeB);
  aNodeA->AddChildNode(aNodeC);

  aDummyModel->CommitCommand();

  /* ====================
   *  Save Model to file
   * ==================== */

  // Prepare filename
  TCollection_AsciiString
    aFilename = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "loadModel.cbf").c_str();

  // Save the Model
  TEST_VERIFY( aDummyModel->SaveAs(aFilename) )

  // Close the Document
  aDummyModel->Release();

  /* =======================================
   *  Load Model from file to another Model
   * ======================================= */

  // Prepare new Model to load the CBF-file into
  Handle(ActTest_DummyModel) aSecondModel = new ActTest_DummyModel();
  
  // Verify the initial statuses of the Model
  TEST_VERIFY( !aSecondModel->IsInitialized() )
  TEST_VERIFY( aSecondModel->GetVersionStatus() == ActAPI_IModel::Version_Undefined )

  // Model is NOT SAVED initially
  TEST_VERIFY( !aSecondModel->IsSaved() )

  // Load the persistent CAF Document into new Model
  TEST_VERIFY( aSecondModel->Open(aFilename) )

  // Model gets SAVED just after LOAD
  TEST_VERIFY( aSecondModel->IsSaved() )

  // Model is NOT MODIFED just after LOAD
  TEST_VERIFY( !aSecondModel->IsModified() )

  // Verify statuses
  TEST_VERIFY( aSecondModel->IsInitialized() )
  TEST_VERIFY( aSecondModel->GetVersionStatus() == ActAPI_IModel::Version_Undefined )

  /* ==================
   *  Verify hierarchy
   * ================== */

  // Access root Node
  Handle(ActAPI_INode) aRootNode = aSecondModel->GetRootNode();
  TEST_VERIFY( aRootNode->GetParentNode().IsNull() )

  // Basic checks
  TEST_VERIFY( !aRootNode.IsNull() )
  TEST_VERIFY( ActAux::are_equal( aRootNode->DynamicType()->Name(), STANDARD_TYPE(ActTest_StubANode)->Name() ) )
  TEST_VERIFY( aRootNode->IsWellFormed() )

  // Check stored integer Parameter value: we do not check shapes as they
  // are different after retrieval (different TShape handles)
  Handle(ActTest_StubANode) aRootDummy = Handle(ActTest_StubANode)::DownCast(aRootNode);
  TEST_VERIFY( aRootDummy->GetValue() == aNodeA_Val )

  // Verify children
  Standard_Integer aNodeIndex = 0;
  Handle(ActAPI_IChildIterator) aChildIt = aRootNode->GetChildIterator();
  for ( ; aChildIt->More(); aChildIt->Next() )
  {
    aNodeIndex++;
    Handle(ActTest_StubANode)
      aChildDummy = Handle(ActTest_StubANode)::DownCast( aChildIt->Value() );
    if ( aNodeIndex == 1 )
      TEST_VERIFY( aChildDummy->GetValue() == aNodeB_Val )
    else if ( aNodeIndex == 2 )
      TEST_VERIFY( aChildDummy->GetValue() == aNodeC_Val )
  }

  return outcome().success();
}

//! Performs test on Save method.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelPersistence::saveModel(const int asiTestEngine_NotUsed(funcID))
{
  Handle(ActAPI_IModel) M = new ActTest_DummyModel;
  TEST_VERIFY( M->NewEmpty() )

  Handle(ActTest_DummyModel) aDummyModel = Handle(ActTest_DummyModel)::DownCast(M);
  
  /* =====================
   *  Populate Data Model
   * ===================== */

  // Construct detached Node
  Handle(ActTest_StubANode)
    aNode = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );

  aDummyModel->OpenCommand();

  // Attach Node to CAF: DETACHED --> ATTACHED
  aDummyModel->StubAPartition()->AddNode(aNode);

  // Initialize Data Node: ATTACHED --> WELL-FORMED
  aNode->Init( asiTestEngine_Utils::RandomShape(),
               asiTestEngine_Utils::RandomShape(),
               asiTestEngine_Utils::RandomInteger() );

  // Verify Node status
  TEST_VERIFY( aNode->IsWellFormed() )

  aDummyModel->CommitCommand();

  // Model gets MODIFIED just after COMMIT
  TEST_VERIFY( aDummyModel->IsModified() )

  // Model is NOT SAVED initially
  TEST_VERIFY( !aDummyModel->IsSaved() )

  /* ====================
   *  Save Model to file
   * ==================== */

  // Prepare filename
  TCollection_AsciiString
    aFilename = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "saveModel.cbf").c_str();

  // Save the Model
  TEST_VERIFY( aDummyModel->SaveAs(aFilename) )

  // Model gets UNMODIFIED just after SAVE
  TEST_VERIFY( !aDummyModel->IsModified() )

  // Model gets SAVED just after SAVE
  TEST_VERIFY( aDummyModel->IsSaved() )

  return outcome().success();
}

//! Performs test on Release method.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelPersistence::releaseModel(const int asiTestEngine_NotUsed(funcID))
{
  Handle(ActAPI_IModel) M = new ActTest_DummyModel;
  TEST_VERIFY( M->NewEmpty() );

  Handle(ActTest_DummyModel) aDummyModel = Handle(ActTest_DummyModel)::DownCast(M);

  /* =====================
   *  Populate Data Model
   * ===================== */

  // Construct detached Node
  Handle(ActTest_StubANode)
    aNode = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );

  aDummyModel->OpenCommand();

  // Attach Node to CAF: DETACHED --> ATTACHED
  aDummyModel->StubAPartition()->AddNode(aNode);

  // Initialize Data Node: ATTACHED --> WELL-FORMED
  aNode->Init( asiTestEngine_Utils::RandomShape(),
               asiTestEngine_Utils::RandomShape(),
               asiTestEngine_Utils::RandomInteger() );

  // Verify Node status
  TEST_VERIFY( aNode->IsWellFormed() )

  aDummyModel->CommitCommand();

  /* ====================
   *  Release Data Model
   * ==================== */

  aDummyModel->Release();

  TEST_VERIFY( !aDummyModel->IsInitialized() )
  TEST_VERIFY( !aDummyModel->IsModified() )
  TEST_VERIFY( !aDummyModel->IsSaved() )
  TEST_VERIFY( aDummyModel->GetVersionStatus() == ActAPI_IModel::Version_Undefined )

  return outcome().success();
}

//-----------------------------------------------------------------------------
// STRUCTURE MANAGEMENT: Test functions support
//-----------------------------------------------------------------------------

//! Will be called before each test function. If this routine fails,
//! the queued test function will not be executed -- the test will proceed
//! to the next function.
//! \param M        [out] test Model.
//! \param node_IDs [out] IDs of the created Nodes.
void ActTest_BaseModelStructure::init(Handle(ActAPI_IModel)& M,
                                      NCollection_Sequence<ActAPI_DataObjectId>& node_IDs)
{
  /* =======================
   *  Create new Data Model
   * ======================= */

  TEST_PRINT_DECOR_L("Create new Dummy Model instance");
  M = new ActTest_DummyModel();
  M->NewEmpty();

  /* ===================================================
   *  Populate Data Model with the following USER tree:
   * ///////////////////////////////////////////////////
   *
   *                + --> E
   *                |
   *      + --> B --+ --> F
   *      |         |
   *      |         + --> G
   *      |
   *  A --+ --> C
   *      |
   *      |         + --> H
   *      |         |
   *      + --> D --+ --> I
   *                |
   *                + --> J --> K
   *
   * =================================================== */

  TEST_PRINT_DECOR_L("Populate Data Model");

  Handle(ActTest_DummyModel) aDummyModel = Handle(ActTest_DummyModel)::DownCast(M);

  // Create new DETACHED Nodes
  Handle(ActTest_StubANode)
    aNodeA = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeB = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeC = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeD = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeE = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeF = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeG = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeH = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeI = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeJ = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeK = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );

  aDummyModel->OpenCommand();

  // Attach Nodes to CAF Document
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeA) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeB) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeC) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeD) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeE) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeF) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeG) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeH) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeI) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeJ) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeK) );

  // Initialize Node A
  aNodeA->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node B
  aNodeB->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node C
  aNodeC->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node D
  aNodeD->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node E
  aNodeE->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomInteger() );

  // Initialize Node F
  aNodeF->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node G
  aNodeG->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node H
  aNodeH->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node I
  aNodeI->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node J
  aNodeJ->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node D
  aNodeK->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Level 1
  aNodeA->AddChildNode(aNodeB);
  aNodeA->AddChildNode(aNodeC);
  aNodeA->AddChildNode(aNodeD);

  // Level 2
  aNodeB->AddChildNode(aNodeE);
  aNodeB->AddChildNode(aNodeF);
  aNodeB->AddChildNode(aNodeG);
  aNodeD->AddChildNode(aNodeH);
  aNodeD->AddChildNode(aNodeI);
  aNodeD->AddChildNode(aNodeJ);

  // Level 3
  aNodeJ->AddChildNode(aNodeK);

  /* ===================================================
   *  Prepare the following Tree Function graph:
   * ///////////////////////////////////////////////////
   *
   *      + --> E
   *      |
   *      + --> F
   *  C --|
   *      + --> H
   *      |         + --> I
   *      + --> D --|
   *                + --> K
   *
   * =================================================== */

  ////////////////////////////// NODE C ///////////////////////////////////////

  // Using Argument Composer, add INPUT Parameters
  Handle(ActAPI_HParameterList) aINArgsC =
    ActAPI_ParameterStream() << aNodeC->Parameter(ActTest_StubANode::PID_DummyShapeA);

  // Using Argument Composer, add OUTPUT Parameters
  Handle(ActAPI_HParameterList) aOUTArgsC =
    ActAPI_ParameterStream() << aNodeC->Parameter(ActTest_StubANode::PID_DummyShapeB);

  // Connect Tree Function
  aNodeC->ConnectTreeFunction(ActTest_StubANode::PID_TFunc,
                              ActTest_DummyTreeFunction::GUID(),
                              aINArgsC, aOUTArgsC);

  ////////////////////////////// NODE E ///////////////////////////////////////

  // Using Argument Composer, add INPUT Parameters
  Handle(ActAPI_HParameterList) aINArgsE =
    ActAPI_ParameterStream() << aNodeC->Parameter(ActTest_StubANode::PID_DummyShapeB);

  // Using Argument Composer, add OUTPUT Parameters
  Handle(ActAPI_HParameterList) aOUTArgsE =
    ActAPI_ParameterStream() << aNodeE->Parameter(ActTest_StubANode::PID_Real);

  // Connect Tree Function
  aNodeE->ConnectTreeFunction(ActTest_StubANode::PID_TFunc,
                              ActTest_DummyTreeFunction::GUID(),
                              aINArgsE, aOUTArgsE);

  ////////////////////////////// NODE F ///////////////////////////////////////

  // Using Argument Composer, add INPUT Parameters
  Handle(ActAPI_HParameterList) aINArgsF =
    ActAPI_ParameterStream() << aNodeC->Parameter(ActTest_StubANode::PID_DummyShapeB);

  // Using Argument Composer, add OUTPUT Parameters
  Handle(ActAPI_HParameterList) aOUTArgsF =
    ActAPI_ParameterStream() << aNodeF->Parameter(ActTest_StubANode::PID_Real);

  // Bind Function Driver
  aNodeF->ConnectTreeFunction(ActTest_StubANode::PID_TFunc,
                              ActTest_DummyTreeFunction::GUID(),
                              aINArgsF, aOUTArgsF);

  ////////////////////////////// NODE H ///////////////////////////////////////

  // Using Argument Composer, add INPUT Parameters
  Handle(ActAPI_HParameterList) aINArgsH =
    ActAPI_ParameterStream() << aNodeC->Parameter(ActTest_StubANode::PID_DummyShapeB);

  // Using Argument Composer, add OUTPUT Parameters
  Handle(ActAPI_HParameterList) aOUTArgsH =
    ActAPI_ParameterStream() << aNodeH->Parameter(ActTest_StubANode::PID_Real);

  // Connect Tree Function
  aNodeH->ConnectTreeFunction(ActTest_StubANode::PID_TFunc,
                              ActTest_DummyTreeFunction::GUID(),
                              aINArgsH, aOUTArgsH);

  ////////////////////////////// NODE D ///////////////////////////////////////

  // Using Argument Composer, add INPUT Parameters
  Handle(ActAPI_HParameterList) aINArgsD =
    ActAPI_ParameterStream() << aNodeC->Parameter(ActTest_StubANode::PID_DummyShapeB);

  // Using Argument Composer, add OUTPUT Parameters
  Handle(ActAPI_HParameterList) aOUTArgsD =
    ActAPI_ParameterStream() << aNodeD->Parameter(ActTest_StubANode::PID_Real);

  // Connect Tree Function
  aNodeD->ConnectTreeFunction(ActTest_StubANode::PID_TFunc,
                              ActTest_DummyTreeFunction::GUID(),
                              aINArgsD, aOUTArgsD);

  ////////////////////////////// NODE I ///////////////////////////////////////

  // Using Argument Composer, add INPUT Parameters
  Handle(ActAPI_HParameterList) aINArgsI =
    ActAPI_ParameterStream() << aNodeD->Parameter(ActTest_StubANode::PID_Real);

  // Using Argument Composer, add OUTPUT Parameters
  Handle(ActAPI_HParameterList) aOUTArgsI =
    ActAPI_ParameterStream() << aNodeI->Parameter(ActTest_StubANode::PID_Real);

  // Connect Tree Function
  aNodeI->ConnectTreeFunction(ActTest_StubANode::PID_TFunc,
                              ActTest_DummyTreeFunction::GUID(),
                              aINArgsI, aOUTArgsI);

  ////////////////////////////// NODE K ///////////////////////////////////////

  // Using Argument Composer, add INPUT Parameters
  Handle(ActAPI_HParameterList) aINArgsK =
    ActAPI_ParameterStream() << aNodeD->Parameter(ActTest_StubANode::PID_Real);

  // Using Argument Composer, add OUTPUT Parameters
  Handle(ActAPI_HParameterList) aOUTArgsK =
    ActAPI_ParameterStream() << aNodeK->Parameter(ActTest_StubANode::PID_Real);

  // Connect Tree Function
  aNodeK->ConnectTreeFunction(ActTest_StubANode::PID_TFunc,
                              ActTest_DummyTreeFunction::GUID(),
                              aINArgsK, aOUTArgsK);

  /* =====================
   *  Finalize population
   * ===================== */

  aDummyModel->CommitCommand();
}

//-----------------------------------------------------------------------------
// STRUCTURE MANAGEMENT: Business logic
//-----------------------------------------------------------------------------

//! Test function for FindNode method.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelStructure::findNode(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  // Find root Data Node
  Handle(ActAPI_INode) aRootNode = M->FindNode( node_IDs(1) );

  // Verify results
  TEST_VERIFY( !aRootNode.IsNull() )
  TEST_VERIFY( ActAux::are_equal( aRootNode->GetId().ToCString(), node_IDs(1).ToCString() ) )
  TEST_VERIFY( ActAux::are_equal( aRootNode->GetId().ToCString(), M->GetRootNode()->GetId().ToCString() ) )

  return outcome().success();
}

//! Test function for DeleteNode method.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelStructure::deleteRootNode(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  // Prepare filename
  TCollection_AsciiString
    aFilenameBefore = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "deleteRootNode_before.cbf").c_str();

  // Save the Model before modifications
  TEST_VERIFY( M->SaveAs(aFilenameBefore) )

  // Delete root Node A
  M->OpenCommand();
  TEST_VERIFY( M->DeleteNode( node_IDs(1) ) )
  M->CommitCommand();

  Handle(ActAPI_INode) aNewRoot = M->GetRootNode();
  TEST_VERIFY( aNewRoot.IsNull() )

  Handle(ActTest_DummyModel) aDummyModel = Handle(ActTest_DummyModel)::DownCast(M);
  ActTest_StubAPartition::Iterator aPartIt( aDummyModel->StubAPartition() );

  // All Nodes and Partition Iterator can handle this situation by resolving
  // "ghost" Labels as non-Nodal ones
  TEST_VERIFY( !aPartIt.More() )

  return outcome().success();
}

//! Test function for DeleteNode method called on Node D of the
//! initial hierarchy.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelStructure::deleteSubTreeNode_D(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  // Prepare filename
  TCollection_AsciiString
    aFilenameBefore = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "deleteSubTreeNode_D_before.cbf").c_str();

  // Save the Model before modifications
  TEST_VERIFY( M->SaveAs(aFilenameBefore) )

  // Delete Data Node D
  M->OpenCommand();
  TEST_VERIFY( M->DeleteNode( node_IDs(4) ) )
  M->CommitCommand();

  /* ====================================================
   *  Save the Model for observing in external GUI tools
   * ==================================================== */

  // Prepare filename
  TCollection_AsciiString
    aFilename = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "deleteSubTreeNode_D.cbf").c_str();

  Handle(ActTest_DummyModel) aDummyModel = Handle(ActTest_DummyModel)::DownCast(M);

  // Save the Model
  TEST_VERIFY( aDummyModel->SaveAs(aFilename) )

  /* ======================================================================
   *  Verify the deletion results by accessing all Data Nodes in the Model
   * ====================================================================== */

  TEST_VERIFY( !M->FindNode( node_IDs(1) ).IsNull() )  // A is still here
  TEST_VERIFY( !M->FindNode( node_IDs(2) ).IsNull() )  // B is still here
  TEST_VERIFY( !M->FindNode( node_IDs(3) ).IsNull() )  // C is still here
  TEST_VERIFY(  M->FindNode( node_IDs(4) ).IsNull() )  // D deleted (!)
  TEST_VERIFY( !M->FindNode( node_IDs(5) ).IsNull() )  // E is still here
  TEST_VERIFY( !M->FindNode( node_IDs(6) ).IsNull() )  // F is still here
  TEST_VERIFY( !M->FindNode( node_IDs(7) ).IsNull() )  // G is still here
  TEST_VERIFY(  M->FindNode( node_IDs(8) ).IsNull() )  // H deleted (!)
  TEST_VERIFY(  M->FindNode( node_IDs(9) ).IsNull() )  // I deleted (!)
  TEST_VERIFY(  M->FindNode( node_IDs(10) ).IsNull() ) // J deleted (!)
  TEST_VERIFY(  M->FindNode( node_IDs(11) ).IsNull() ) // K deleted (!)

  /* ================================================
   *  Verify Tree Function Parameters after deletion
   * ================================================ */

  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(1) ) )->HasConnectedFunction() )
  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(2) ) )->HasConnectedFunction() )
  TEST_VERIFY(  Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(3) ) )->HasConnectedFunction() )
  TEST_VERIFY(  Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(5) ) )->HasConnectedFunction() )
  TEST_VERIFY(  Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(6) ) )->HasConnectedFunction() )
  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(7) ) )->HasConnectedFunction() )

  return outcome().success();
}

//! Test function for DeleteNode method called on Node D of the
//! initial hierarchy. Introduces additional dependencies via References.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelStructure::deleteSubTreeNode_D_AsReferenced(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  /* =================================================
   *  Add plain references to the Node being removed:
   * -------------------------------------------------
   *  A --+
   *      |
   *      +--> D --> C
   *      |
   *  B --+
   * ================================================= */

  // Any Parameter of D just to make a reference to
  Handle(ActAPI_IUserParameter) D_anyParam =
    M->FindNode( node_IDs(4) )->Parameter(ActTest_StubANode::PID_DummyShapeA);

  // Any Parameter of C just to make a reference to
  Handle(ActAPI_IUserParameter) C_anyParam =
    M->FindNode( node_IDs(3) )->Parameter(ActTest_StubANode::PID_DummyShapeA);

  M->OpenCommand();

  // A & B refer to D
  M->FindNode( node_IDs(1) )->ConnectReference(ActTest_StubANode::PID_Ref, D_anyParam);
  M->FindNode( node_IDs(2) )->ConnectReference(ActTest_StubANode::PID_Ref, D_anyParam);

  // Assure that C has no Referrer observers yet
  Handle(ActData_BaseNode) CBase = Handle(ActData_BaseNode)::DownCast( M->FindNode( node_IDs(3) ) );
  TEST_VERIFY( CBase->GetReferrers()->IsEmpty() )

  // D refers to C
  M->FindNode( node_IDs(4) )->ConnectReference(ActTest_StubANode::PID_Ref, C_anyParam);

  // Dump after setting references
  {
    TCollection_AsciiString
      fn = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "deleteSubTreeNode_D_AsReferenced_init.cbf").c_str();
    //
    TEST_VERIFY( M->SaveAs(fn) )
  }

  // Assure that C has got a Referrer observer
  TEST_VERIFY( !CBase->GetReferrers()->IsEmpty() )

  M->CommitCommand();

  /* ========================
   *  Perform actual removal
   * ======================== */

  // Prepare filename
  TCollection_AsciiString
    aFilenameBefore = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "deleteSubTreeNode_D_AsReferenced_D_before.cbf").c_str();

  // Save the Model before modifications
  TEST_VERIFY( M->SaveAs(aFilenameBefore) )

  // Delete Data Node D
  M->OpenCommand();
  TEST_VERIFY( M->DeleteNode( node_IDs(4) ) )
  M->CommitCommand();

  /* ====================================================
   *  Save the Model for observing in external GUI tools
   * ==================================================== */

  // Prepare filename
  TCollection_AsciiString
    aFilename = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "deleteSubTreeNode_D_AsReferenced_D.cbf").c_str();

  Handle(ActTest_DummyModel) aDummyModel = Handle(ActTest_DummyModel)::DownCast(M);

  // Save the Model
  TEST_VERIFY( aDummyModel->SaveAs(aFilename) )

  /* ======================================================================
   *  Verify the deletion results by accessing all Data Nodes in the Model
   * ====================================================================== */

  TEST_VERIFY( !M->FindNode( node_IDs(1) ).IsNull() )  // A is still here
  TEST_VERIFY( !M->FindNode( node_IDs(2) ).IsNull() )  // B is still here
  TEST_VERIFY( !M->FindNode( node_IDs(3) ).IsNull() )  // C is still here
  TEST_VERIFY(  M->FindNode( node_IDs(4) ).IsNull() )  // D deleted (!)
  TEST_VERIFY( !M->FindNode( node_IDs(5) ).IsNull() )  // E is still here
  TEST_VERIFY( !M->FindNode( node_IDs(6) ).IsNull() )  // F is still here
  TEST_VERIFY( !M->FindNode( node_IDs(7) ).IsNull() )  // G is still here
  TEST_VERIFY(  M->FindNode( node_IDs(8) ).IsNull() )  // H deleted (!)
  TEST_VERIFY(  M->FindNode( node_IDs(9) ).IsNull() )  // I deleted (!)
  TEST_VERIFY(  M->FindNode( node_IDs(10) ).IsNull() ) // J deleted (!)
  TEST_VERIFY(  M->FindNode( node_IDs(11) ).IsNull() ) // K deleted (!)

  /* ============================================
   *  Verify Reference Parameters after deletion
   * ============================================ */

  TEST_VERIFY( !M->FindNode( node_IDs(1) )->HasConnectedReference(ActTest_StubANode::PID_Ref) )
  TEST_VERIFY( !M->FindNode( node_IDs(2) )->HasConnectedReference(ActTest_StubANode::PID_Ref) )

  // Assure that C has lost its Referrer observer D
  TEST_VERIFY( CBase->GetReferrers()->IsEmpty() )

  return outcome().success();
}

//! Test function for DeleteNode method called on Node C of the
//! initial hierarchy.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelStructure::deleteSubTreeNode_C(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  // Prepare filename
  TCollection_AsciiString
    aFilenameBefore = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "deleteSubTreeNode_C_before.cbf").c_str();

  // Save the Model before modifications
  TEST_VERIFY( M->SaveAs(aFilenameBefore) )

  // Delete Data Node
  M->OpenCommand();
  TEST_VERIFY( M->DeleteNode( node_IDs(3) ) )
  M->CommitCommand();

  /* ====================================================
   *  Save the Model for observing in external GUI tools
   * ==================================================== */

  // Prepare filename
  TCollection_AsciiString
    aFilename = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "deleteSubTreeNode_C.cbf").c_str();

  Handle(ActTest_DummyModel) aDummyModel = Handle(ActTest_DummyModel)::DownCast(M);

  // Save the Model
  TEST_VERIFY( aDummyModel->SaveAs(aFilename) )

  /* ======================================================================
   *  Verify the deletion results by accessing all Data Nodes in the Model
   * ====================================================================== */

  TEST_VERIFY( !M->FindNode( node_IDs(1) ).IsNull() )  // A is still here
  TEST_VERIFY( !M->FindNode( node_IDs(2) ).IsNull() )  // B is still here
  TEST_VERIFY(  M->FindNode( node_IDs(3) ).IsNull() )  // C deleted (!)
  TEST_VERIFY( !M->FindNode( node_IDs(4) ).IsNull() )  // D is still here
  TEST_VERIFY( !M->FindNode( node_IDs(5) ).IsNull() )  // E is still here
  TEST_VERIFY( !M->FindNode( node_IDs(6) ).IsNull() )  // F is still here
  TEST_VERIFY( !M->FindNode( node_IDs(7) ).IsNull() )  // G is still here
  TEST_VERIFY( !M->FindNode( node_IDs(8) ).IsNull() )  // H is still here
  TEST_VERIFY( !M->FindNode( node_IDs(9) ).IsNull() )  // I is still here
  TEST_VERIFY( !M->FindNode( node_IDs(10) ).IsNull() ) // J is still here
  TEST_VERIFY( !M->FindNode( node_IDs(11) ).IsNull() ) // K is still here

  /* ================================================
   *  Verify Tree Function Parameters after deletion
   * ================================================ */

  // A did not have Tree Function Parameter
  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(1) ) )->HasConnectedFunction() )

  // B did not have Tree Function Parameter
  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(2) ) )->HasConnectedFunction() )

  // D looses its Tree Function Parameter (!)
  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(4) ) )->HasConnectedFunction() )

  // E looses its Tree Function Parameter (!)
  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(5) ) )->HasConnectedFunction() )

  // F looses its Tree Function Parameter (!)
  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(6) ) )->HasConnectedFunction() )

  // G did not have Tree Function Parameter
  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(7) ) )->HasConnectedFunction() )

  // H looses its Tree Function Parameter (!)
  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(8) ) )->HasConnectedFunction() )

  // I keeps its Tree Function Parameter
  TEST_VERIFY( Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(9) ) )->HasConnectedFunction() )

  // J did not have Tree Function Parameter
  TEST_VERIFY( !Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(10) ) )->HasConnectedFunction() )

  // K keeps its Tree Function Parameter
  TEST_VERIFY( Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(11) ) )->HasConnectedFunction() )

  return outcome().success();
}

//! Test function for accessing Tree Function observers of Node D.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelStructure::accessObservers_D(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActAPI_INode) aNodeD = M->FindNode( node_IDs(4) );
  TEST_VERIFY( aNodeD->IsWellFormed() )

  /* ================
   *  General checks
   * ================ */

  TEST_VERIFY( aNodeD->GetOutputWriters()->IsEmpty() )
  TEST_VERIFY( !aNodeD->GetInputReaders()->IsEmpty() )
  TEST_VERIFY( aNodeD->GetInputReaders()->Length() == 2 )

  /* ======================
   *  Verify INPUT readers
   * ====================== */

  ActAPI_DataObjectId EXPECTED[2] = {node_IDs(9), node_IDs(11)};
  Standard_Integer i = 0;

  Handle(ActAPI_HParameterList) anInputReaders = aNodeD->GetInputReaders();
  ActAPI_ParameterList::Iterator aParamIt( *anInputReaders.operator->() );
  for ( ; aParamIt.More(); aParamIt.Next() )
  {
    const Handle(ActAPI_IUserParameter)& aNextObserver = aParamIt.Value();
    Handle(ActAPI_INode) aDependentNode = ActData_NodeFactory::NodeByParamSettle(aNextObserver);

    TEST_VERIFY( aDependentNode->IsWellFormed() )
    TEST_VERIFY( ActAux::are_equal( aDependentNode->GetId().ToCString(), EXPECTED[i++].ToCString() ) )
  }

  return outcome().success();
}

//-----------------------------------------------------------------------------
// EXPRESSION EVALUATION: Test functions support
//-----------------------------------------------------------------------------

//! Will be called before each test function. If this routine fails,
//! the queued test function will not be executed -- the test will proceed
//! to the next function.
//! \param M [out] sample Data Model.
//! \param node_IDs [out] IDs of the created sample Nodes.
void ActTest_BaseModelEvaluation::init(Handle(ActAPI_IModel)& M,
                                       NCollection_Sequence<ActAPI_DataObjectId>& node_IDs)
{
  /* =======================
   *  Create new Data Model
   * ======================= */

  TEST_PRINT_DECOR_L("Create new Dummy Model instance");
  M = new ActTest_DummyModel();
  M->NewEmpty();

  /* =====================
   *  Populate Data Model
   * ===================== */

  TEST_PRINT_DECOR_L("Populate Data Model");

  Handle(ActTest_DummyModel) aDummyModel = Handle(ActTest_DummyModel)::DownCast(M);

  // Create new DETACHED Dummy Nodes
  Handle(ActTest_StubANode)
    aNodeA = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeB = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  Handle(ActTest_StubANode)
    aNodeC = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );

  // Create new DETACHED Variable Nodes
  Handle(ActData_RealVarNode)
    aRealVarNode1 = Handle(ActData_RealVarNode)::DownCast( ActData_RealVarNode::Instance() );
  Handle(ActData_RealVarNode)
    aRealVarNode2 = Handle(ActData_RealVarNode)::DownCast( ActData_RealVarNode::Instance() );
  Handle(ActData_IntVarNode)
    aIntVarNode1 = Handle(ActData_IntVarNode)::DownCast( ActData_IntVarNode::Instance() );
  Handle(ActData_IntVarNode)
    aIntVarNode2 = Handle(ActData_IntVarNode)::DownCast( ActData_IntVarNode::Instance() );
  Handle(ActData_BoolVarNode)
    aBoolVarNode1 = Handle(ActData_BoolVarNode)::DownCast( ActData_BoolVarNode::Instance() );
  Handle(ActData_BoolVarNode)
    aBoolVarNode2 = Handle(ActData_BoolVarNode)::DownCast( ActData_BoolVarNode::Instance() );

  aDummyModel->OpenCommand();

  // Attach Nodes to CAF Document
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeA) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeB) );
  node_IDs.Append( aDummyModel->StubAPartition()->AddNode(aNodeC) );
  node_IDs.Append( aDummyModel->VariablePartition(ActAPI_IModel::Variable_Real)->AddNode(aRealVarNode1) );
  node_IDs.Append( aDummyModel->VariablePartition(ActAPI_IModel::Variable_Real)->AddNode(aRealVarNode2) );
  node_IDs.Append( aDummyModel->VariablePartition(ActAPI_IModel::Variable_Int)->AddNode(aIntVarNode1) );
  node_IDs.Append( aDummyModel->VariablePartition(ActAPI_IModel::Variable_Int)->AddNode(aIntVarNode2) );
  node_IDs.Append( aDummyModel->VariablePartition(ActAPI_IModel::Variable_Bool)->AddNode(aBoolVarNode1) );
  node_IDs.Append( aDummyModel->VariablePartition(ActAPI_IModel::Variable_Bool)->AddNode(aBoolVarNode2) );

  // Initialize Node A
  aNodeA->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node B
  aNodeB->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Node C
  aNodeC->Init( asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomShape(),
                asiTestEngine_Utils::RandomReal() );

  // Initialize Variable Nodes
  aRealVarNode1->Init( "rX", asiTestEngine_Utils::RandomReal() );
  aRealVarNode2->Init( "rY", asiTestEngine_Utils::RandomReal() );
  aIntVarNode1 ->Init( "iX", asiTestEngine_Utils::RandomInteger() );
  aIntVarNode2 ->Init( "iY", asiTestEngine_Utils::RandomInteger() );
  aBoolVarNode1->Init( "bX", asiTestEngine_Utils::RandomBoolean() );
  aBoolVarNode2->Init( "bY", asiTestEngine_Utils::RandomBoolean() );

  aDummyModel->CommitCommand();
}

//-----------------------------------------------------------------------------
// EXPRESSION EVALUATION: Business logic
//-----------------------------------------------------------------------------

//! Test function for removal of Variable Node playing as a basis for
//! evaluation mechanism.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::removeVariable(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  M->OpenCommand();

  /* ===============================================
   *  Set evaluation strings for each Business Node
   * =============================================== */

  Handle(ActTest_StubANode)
    aNodeA = Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(1) ) );
  Handle(ActTest_StubANode)
    aNodeB = Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(2) ) );
  Handle(ActTest_StubANode)
    aNodeC = Handle(ActTest_StubANode)::DownCast( M->FindNode( node_IDs(3) ) );

  aNodeA->Parameter(ActTest_StubANode::PID_Real)->SetEvalString("rY");
  aNodeB->Parameter(ActTest_StubANode::PID_Real)->SetEvalString("2.0 * rY + iX");
  aNodeC->Parameter(ActTest_StubANode::PID_Real)->SetEvalString("2.0 * rX - iY");

  /* ========================================================
   *  Set evaluation strings for Variable Nodes (optionally)
   * ======================================================== */

  Handle(ActData_RealVarNode)
    aRealVarNode1 = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );
  Handle(ActData_RealVarNode)
    aRealVarNode2 = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(5) ) );

  Handle(ActData_RealParameter) aRealParam1 =
    Handle(ActData_RealParameter)::DownCast( aRealVarNode1->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) aRealParam2 =
    Handle(ActData_RealParameter)::DownCast( aRealVarNode2->Parameter(ActData_RealVarNode::Param_Value) );

  aRealParam1->SetEvalString("iX + iY");
  aRealParam2->SetEvalString("rX");

  /* ===========================================
   *  Set values for the rest of Variable Nodes
   * =========================================== */

  Handle(ActData_IntVarNode)
    aIntVarNode1 = Handle(ActData_IntVarNode)::DownCast( M->FindNode( node_IDs(6) ) );
  Handle(ActData_IntVarNode)
    aIntVarNode2 = Handle(ActData_IntVarNode)::DownCast( M->FindNode( node_IDs(7) ) );

  Handle(ActData_IntParameter) aIntParam1 =
    Handle(ActData_IntParameter)::DownCast( aIntVarNode1->Parameter(ActData_IntVarNode::Param_Value) );
  Handle(ActData_IntParameter) aIntParam2 =
    Handle(ActData_IntParameter)::DownCast( aIntVarNode2->Parameter(ActData_IntVarNode::Param_Value) );

  aIntParam1->SetValue(1);
  aIntParam2->SetValue(10);

  /* =============================
   *  Bind Variables to Variables
   * ============================= */

  // Real Variable 1 depends on Integer Variables 1 & 2
  aRealVarNode1->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                                   ActAPI_ParameterStream() << aIntParam1 << aIntParam2 );

  // Real Variable 2 depends on Real Variable 1
  aRealVarNode2->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                                   ActAPI_ParameterStream() << aRealParam1 );

  /* ==================================
   *  Bind Business Nodes to Variables
   * ================================== */

  // Node A depends on Real Variable 2
  aNodeA->ConnectEvaluator( ActTest_StubANode::PID_Real,
                            ActAPI_ParameterStream() << aRealParam2 );

  // Node B depends on Real Variable 2 & Integer Variable 1
  aNodeB->ConnectEvaluator( ActTest_StubANode::PID_Real,
                            ActAPI_ParameterStream() << aRealParam2 << aIntParam1 );

  // Node C depends on Real Variable 1 & Integer Variable 2
  aNodeC->ConnectEvaluator( ActTest_StubANode::PID_Real,
                            ActAPI_ParameterStream() << aRealParam1 << aIntParam2 );

  ActData_FuncExecutionCtx::UpdateDependencies( Handle(ActData_BaseModel)::DownCast(M) );
  TEST_PRINT( ActData_GraphToDot::Convert(M).ToCString() )

  /* =====================================
   *  Save the Model before modifications
   * ===================================== */

  // Prepare filename
  TCollection_AsciiString
    aFilenameBefore = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "removeVariable_before.cbf").c_str();

  TEST_VERIFY( M->SaveAs(aFilenameBefore) )

  M->CommitCommand();

  /* =================================
   *  Verify observers of variable I1
   * ================================= */

  Handle(ActAPI_HParameterList) anOutputWritersI1 = aIntVarNode1->GetOutputWriters();
  Handle(ActAPI_HParameterList) anInputReadersI1 = aIntVarNode1->GetInputReaders();

  TEST_VERIFY( anOutputWritersI1->IsEmpty() )
  TEST_VERIFY( !anInputReadersI1->IsEmpty() )
  TEST_VERIFY( anInputReadersI1->Length() == 2 )

  Handle(ActAPI_INode)
    aReaderI1_1 = ActData_NodeFactory::NodeByParamSettle( anInputReadersI1->Value(1) );
  Handle(ActAPI_INode)
    aReaderI1_2 = ActData_NodeFactory::NodeByParamSettle( anInputReadersI1->Value(2) );

  TEST_VERIFY( !aReaderI1_1.IsNull() )
  TEST_VERIFY( !aReaderI1_2.IsNull() )

  TEST_VERIFY( ActAux::are_equal( aReaderI1_1->GetId().ToCString(), aRealVarNode1->GetId().ToCString() ) )
  TEST_VERIFY( ActAux::are_equal( aReaderI1_2->GetId().ToCString(), aNodeB->GetId().ToCString() ) )

  /* ====================================================================
   *  Now, remove Integer Variable 1 (I1). As R1 and D2 are dependent
   *  Nodes, their correspondent Tree Function Parameters must switch to
   *  the DISCONNECTED state
   * ==================================================================== */

  M->OpenCommand();

  TEST_VERIFY( M->DeleteNode( aIntVarNode1->GetId() ) )

  M->CommitCommand();

  /* ====================================
   *  Save the Model after modifications
   * ==================================== */

  // Prepare filename
  TCollection_AsciiString
    aFilenameAfter = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "removeVariable_after.cbf").c_str();

  // Save the Model
  TEST_VERIFY( M->SaveAs(aFilenameAfter) )

  /* ======================================================================
   *  Verify the deletion results by accessing all Data Nodes in the Model
   * ====================================================================== */

  TEST_VERIFY( !M->FindNode( node_IDs(1) ).IsNull() ) // A is still here
  TEST_VERIFY( !M->FindNode( node_IDs(2) ).IsNull() ) // B is still here
  TEST_VERIFY( !M->FindNode( node_IDs(3) ).IsNull() ) // C is still here
  TEST_VERIFY( !M->FindNode( node_IDs(4) ).IsNull() ) // R1 is still here
  TEST_VERIFY( !M->FindNode( node_IDs(5) ).IsNull() ) // R2 is still here
  TEST_VERIFY(  M->FindNode( node_IDs(6) ).IsNull() ) // I1 is removed (!)
  TEST_VERIFY( !M->FindNode( node_IDs(7) ).IsNull() ) // I2 is still here
  TEST_VERIFY( !M->FindNode( node_IDs(8) ).IsNull() ) // B1 is still here
  TEST_VERIFY( !M->FindNode( node_IDs(9) ).IsNull() ) // B2 is still here

  /* ================================================
   *  Verify Tree Function Parameters after deletion
   * ================================================ */

  // A still has its Evaluator as it does not depend on I1
  TEST_VERIFY( Handle(ActData_BaseNode)::DownCast( M->FindNode( node_IDs(1) ) )->HasConnectedEvaluator(ActTest_StubANode::PID_Real) )

  // B loses its Evaluator as it depends on I1
  TEST_VERIFY( !Handle(ActData_BaseNode)::DownCast( M->FindNode( node_IDs(2) ) )->HasConnectedEvaluator(ActTest_StubANode::PID_Real) )

  // C still has its Evaluator as it does not depend on I1
  TEST_VERIFY( Handle(ActData_BaseNode)::DownCast( M->FindNode( node_IDs(3) ) )->HasConnectedEvaluator(ActTest_StubANode::PID_Real) )

  // R1 loses its Evaluator as it depends on I1
  TEST_VERIFY( !Handle(ActData_BaseNode)::DownCast( M->FindNode( node_IDs(4) ) )->HasConnectedEvaluator(ActData_RealVarNode::Param_Value) )

  // R2 still has its Evaluator as it does not depend on I1
  TEST_VERIFY( Handle(ActData_BaseNode)::DownCast( M->FindNode( node_IDs(5) ) )->HasConnectedEvaluator(ActData_RealVarNode::Param_Value) )

  // I2 did not have Evaluator
  TEST_VERIFY( !Handle(ActData_BaseNode)::DownCast( M->FindNode( node_IDs(7) ) )->HasConnectedEvaluator(ActData_IntVarNode::Param_Value) )

  // B1 did not have Evaluator
  TEST_VERIFY( !Handle(ActData_BaseNode)::DownCast( M->FindNode( node_IDs(8) ) )->HasConnectedEvaluator(ActData_BoolVarNode::Param_Value) )

  // B2 did not have Evaluator
  TEST_VERIFY( !Handle(ActData_BaseNode)::DownCast( M->FindNode( node_IDs(9) ) )->HasConnectedEvaluator(ActData_BoolVarNode::Param_Value) )

  return outcome().success();
}

//! Test function for performing validation of dependency graph checking
//! if there are some loops in it. Simple loop exists.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::checkLoops1(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActData_BaseModel) BM = Handle(ActData_BaseModel)::DownCast(M);

  Handle(ActData_RealVarNode)
    rX_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );
  Handle(ActData_RealVarNode)
    rY_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(5) ) );

  Handle(ActData_RealParameter) rX_param =
    Handle(ActData_RealParameter)::DownCast( rX_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) rY_param =
    Handle(ActData_RealParameter)::DownCast( rY_node->Parameter(ActData_RealVarNode::Param_Value) );

  M->OpenCommand();

  rX_param->SetEvalString("rY");
  rY_param->SetEvalString("rX");

  // Real Variable 1 depends on Real Variable 2
  rX_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << rY_param );

  // Real Variable 2 depends on Real Variable 1
  rY_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << rX_param );

  ActData_FuncExecutionCtx::UpdateDependencies(BM);

  M->CommitCommand();

  Standard_Integer aState = ActData_FuncExecutionCtx::CheckDependencyGraph(BM);

  // Prepare filename
  TCollection_AsciiString
    aFilenameAfter = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "checkLoops1_after.cbf").c_str();

  TEST_VERIFY( M->SaveAs(aFilenameAfter) )
  TEST_VERIFY( (aState & ActData_DependencyAnalyzer::GraphState_HasLoops) > 0 )

  return outcome().success();
}

//! Test function for performing validation of dependency graph checking
//! if there are some loops in it. No loops exist.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::checkLoops2(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActData_BaseModel) BM = Handle(ActData_BaseModel)::DownCast(M);

  Handle(ActData_RealVarNode)
    rX_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );
  Handle(ActData_RealVarNode)
    rY_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(5) ) );
  Handle(ActData_IntVarNode)
    iX_node = Handle(ActData_IntVarNode)::DownCast( M->FindNode( node_IDs(6) ) );
  Handle(ActData_IntVarNode)
    iY_node = Handle(ActData_IntVarNode)::DownCast( M->FindNode( node_IDs(7) ) );

  Handle(ActData_RealParameter) rX_param =
    Handle(ActData_RealParameter)::DownCast( rX_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) rY_param =
    Handle(ActData_RealParameter)::DownCast( rY_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_IntParameter) iX_param =
    Handle(ActData_IntParameter)::DownCast( iX_node->Parameter(ActData_IntVarNode::Param_Value) );
  Handle(ActData_IntParameter) iY_param =
    Handle(ActData_IntParameter)::DownCast( iX_node->Parameter(ActData_IntVarNode::Param_Value) );

  M->OpenCommand();

  rX_param->SetEvalString("rY");
  rY_param->SetEvalString("iX");
  iX_param->SetEvalString("iY");
  iY_param->SetValue(1);

  // Real Variable 1 depends on Real Variable 2
  rX_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << rY_param );

  // Real Variable 2 depends on Integer Variable 1
  rY_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << iX_param );

  // Integer Variable 1 depends on Integer Variable 2
  iX_node->ConnectEvaluator( ActData_IntVarNode::Param_Value,
                             ActAPI_ParameterStream() << iY_param );

  ActData_FuncExecutionCtx::UpdateDependencies(BM);

  M->CommitCommand();

  Standard_Integer aState = ActData_FuncExecutionCtx::CheckDependencyGraph(BM);

  // Prepare filename
  TCollection_AsciiString
    aFilenameAfter = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "checkLoops2_after.cbf").c_str();

  TEST_VERIFY( M->SaveAs(aFilenameAfter) )
  TEST_VERIFY(aState & ActData_DependencyAnalyzer::GraphState_Ok)

  return outcome().success();
}

//! Test function for performing validation of dependency graph checking
//! if there are some loops in it. One loop exists.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::checkLoops3(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActData_BaseModel) BM = Handle(ActData_BaseModel)::DownCast(M);

  Handle(ActData_RealVarNode)
    rX_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );
  Handle(ActData_RealVarNode)
    rY_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(5) ) );
  Handle(ActData_IntVarNode)
    iX_node = Handle(ActData_IntVarNode)::DownCast( M->FindNode( node_IDs(6) ) );
  Handle(ActData_IntVarNode)
    iY_node = Handle(ActData_IntVarNode)::DownCast( M->FindNode( node_IDs(7) ) );

  Handle(ActData_RealParameter) rX_param =
    Handle(ActData_RealParameter)::DownCast( rX_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) rY_param =
    Handle(ActData_RealParameter)::DownCast( rY_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_IntParameter) iX_param =
    Handle(ActData_IntParameter)::DownCast( iX_node->Parameter(ActData_IntVarNode::Param_Value) );
  Handle(ActData_IntParameter) iY_param =
    Handle(ActData_IntParameter)::DownCast( iY_node->Parameter(ActData_IntVarNode::Param_Value) );

  M->OpenCommand();

  rX_param->SetEvalString("rY");
  rY_param->SetEvalString("iX");
  iX_param->SetEvalString("iY");
  iY_param->SetEvalString("rX");

  // Real Variable 1 depends on Real Variable 2
  rX_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << rY_param );

  // Real Variable 2 depends on Integer Variable 1
  rY_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << iX_param );

  // Integer Variable 1 depends on Integer Variable 2
  iX_node->ConnectEvaluator( ActData_IntVarNode::Param_Value,
                             ActAPI_ParameterStream() << iY_param );

  // Integer Variable 2 depends on Real Variable 1
  iY_node->ConnectEvaluator( ActData_IntVarNode::Param_Value,
                             ActAPI_ParameterStream() << iX_param );

  ActData_FuncExecutionCtx::UpdateDependencies(BM);

  M->CommitCommand();

  Standard_Integer aState = ActData_FuncExecutionCtx::CheckDependencyGraph(BM);

  // Prepare filename
  TCollection_AsciiString
    aFilenameAfter = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "checkLoops3_after.cbf").c_str();

  TEST_VERIFY( M->SaveAs(aFilenameAfter) )
  TEST_VERIFY(aState & ActData_DependencyAnalyzer::GraphState_HasLoops)

  return outcome().success();
}

//! Test function for performing validation of dependency graph checking
//! if there are some loops in it. No loops exist.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::checkLoops4(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActData_BaseModel) BM = Handle(ActData_BaseModel)::DownCast(M);

  Handle(ActData_RealVarNode)
    rX_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );
  Handle(ActData_RealVarNode)
    rY_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(5) ) );
  Handle(ActData_IntVarNode)
    iX_node = Handle(ActData_IntVarNode)::DownCast( M->FindNode( node_IDs(6) ) );

  Handle(ActData_RealParameter) rX_param =
    Handle(ActData_RealParameter)::DownCast( rX_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) rY_param =
    Handle(ActData_RealParameter)::DownCast( rY_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_IntParameter) iX_param =
    Handle(ActData_IntParameter)::DownCast( iX_node->Parameter(ActData_IntVarNode::Param_Value) );

  M->OpenCommand();

  rX_param->SetEvalString("rY");
  rY_param->SetValue(1.0);
  iX_param->SetEvalString("rX");

  // Real Variable 1 depends on Real Variable 2
  rX_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << rY_param );

  // Integer Variable 1 depends on Real Variable 1
  iX_node->ConnectEvaluator( ActData_IntVarNode::Param_Value,
                             ActAPI_ParameterStream() << rX_param );

  ActData_FuncExecutionCtx::UpdateDependencies(BM);

  M->CommitCommand();

  Standard_Integer aState = ActData_FuncExecutionCtx::CheckDependencyGraph(BM);

  // Prepare filename
  TCollection_AsciiString
    aFilenameAfter = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "checkLoops4_after.cbf").c_str();

  TEST_VERIFY( M->SaveAs(aFilenameAfter) )
  TEST_VERIFY(aState & ActData_DependencyAnalyzer::GraphState_Ok)

  return outcome().success();
}

//! Test function for performing validation of dependency graph checking
//! if there are some loops in it. No loops exist.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::checkLoops5(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActData_BaseModel) BM = Handle(ActData_BaseModel)::DownCast(M);

  Handle(ActData_RealVarNode)
    rX_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );
  Handle(ActData_RealVarNode)
    rY_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(5) ) );
  Handle(ActData_IntVarNode)
    iX_node = Handle(ActData_IntVarNode)::DownCast( M->FindNode( node_IDs(6) ) );

  Handle(ActData_RealParameter) rX_param =
    Handle(ActData_RealParameter)::DownCast( rX_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) rY_param =
    Handle(ActData_RealParameter)::DownCast( rY_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_IntParameter) iX_param =
    Handle(ActData_IntParameter)::DownCast( iX_node->Parameter(ActData_IntVarNode::Param_Value) );

  M->OpenCommand();

  rX_param->SetValue(1.0);
  rY_param->SetEvalString("rX");
  iX_param->SetEvalString("rX + rY");

  // Real Variable 2 depends on Real Variable 1
  rY_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << rX_param );

  // Integer Variable 1 depends on Real Variables 1 and 2
  iX_node->ConnectEvaluator( ActData_IntVarNode::Param_Value,
                             ActAPI_ParameterStream() << rX_param << rY_param );

  ActData_FuncExecutionCtx::UpdateDependencies(BM);

  M->CommitCommand();

  Standard_Integer aState = ActData_FuncExecutionCtx::CheckDependencyGraph(BM);

  // Prepare filename
  TCollection_AsciiString
    aFilenameAfter = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "checkLoops5_after.cbf").c_str();

  TEST_VERIFY( M->SaveAs(aFilenameAfter) )
  TEST_VERIFY(aState & ActData_DependencyAnalyzer::GraphState_Ok)

  return outcome().success();
}

//! Test function for performing validation of dependency graph checking
//! if there are some loops in it. Test on infinite recursion detected
//! on the following case:
//! <pre>
//!
//!   +=============+
//!   | VAR | EXPR  |
//!   +-----+-------+
//!   | rX  |   1   |
//!   +-----+-------+
//!   | rY  |  iX   |
//!   +-----+-------+
//!   | iX  | rX*rY |
//!   +=============+
//!
//! </pre>
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::checkLoops6(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActData_BaseModel) BM = Handle(ActData_BaseModel)::DownCast(M);

  Handle(ActData_RealVarNode)
    rX_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );
  Handle(ActData_RealVarNode)
    rY_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(5) ) );
  Handle(ActData_IntVarNode)
    iX_node = Handle(ActData_IntVarNode)::DownCast( M->FindNode( node_IDs(6) ) );

  Handle(ActData_RealParameter) rX_param =
    Handle(ActData_RealParameter)::DownCast( rX_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) rY_param =
    Handle(ActData_RealParameter)::DownCast( rY_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_IntParameter) iX_param =
    Handle(ActData_IntParameter)::DownCast( iX_node->Parameter(ActData_IntVarNode::Param_Value) );

  M->OpenCommand();

  rX_param->SetEvalString("1.0");
  rY_param->SetEvalString("iX");
  iX_param->SetEvalString("rX*rY");

  // Real Variable 2 depends on Real Variable 1
  rY_node->ConnectEvaluator( ActData_BaseVarNode::Param_Value,
                             ActAPI_ParameterStream() << iX_param );

  // Integer Variable 1 depends on Real Variables 1 and 2
  iX_node->ConnectEvaluator( ActData_BaseVarNode::Param_Value,
                             ActAPI_ParameterStream() << rX_param << rY_param );

  ActData_FuncExecutionCtx::UpdateDependencies(BM);

  M->CommitCommand();

  Standard_Integer aState = ActData_FuncExecutionCtx::CheckDependencyGraph(BM);

  // Prepare filename
  TCollection_AsciiString
    aFilenameAfter = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "checkLoops6_after.cbf").c_str();

  TEST_VERIFY( M->SaveAs(aFilenameAfter) )
  TEST_VERIFY(aState & ActData_DependencyAnalyzer::GraphState_HasLoops)

  return outcome().success();
}

//! Test function for renaming functionality for Variable Nodes.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::renameVariable1(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActData_RealVarNode)
    rX_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );
  Handle(ActData_RealVarNode)
    rY_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(5) ) );

  Handle(ActData_RealParameter) rX_param =
    Handle(ActData_RealParameter)::DownCast( rX_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) rY_param =
    Handle(ActData_RealParameter)::DownCast( rY_node->Parameter(ActData_RealVarNode::Param_Value) );

  TCollection_AsciiString INIT_STRING("99.8*rX+sin(rX)");
  TCollection_AsciiString EXPECTED_STRING("99.8*KAPPA+sin(KAPPA)");

  M->OpenCommand();

  rX_param->SetValue(1.0);
  rY_param->SetEvalString(INIT_STRING);

  // Real Variable 2 depends on Real Variable 1
  rY_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << rX_node->Parameter(ActData_RealVarNode::Param_Value) );

  ActData_FuncExecutionCtx::UpdateDependencies( Handle(ActData_BaseModel)::DownCast(M) );
  TEST_PRINT( ActData_GraphToDot::Convert(M).ToCString() )

  // Rename Variable Node 1
  rX_node->RenameConnected("KAPPA");

  M->CommitCommand();

  // Verify results
  TEST_VERIFY( ActAux::are_equal( rY_param->GetEvalString().ToCString(), EXPECTED_STRING.ToCString() ) )

  return outcome().success();
}

//! Test function for renaming functionality for Variable Nodes.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::renameVariable2(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActData_RealVarNode)
    rX_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );
  Handle(ActData_RealVarNode)
    rY_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(5) ) );
  Handle(ActData_IntVarNode)
    iX_node = Handle(ActData_IntVarNode)::DownCast( M->FindNode( node_IDs(6) ) );

  Handle(ActData_RealParameter) rX_param =
    Handle(ActData_RealParameter)::DownCast( rX_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) rY_param =
    Handle(ActData_RealParameter)::DownCast( rY_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_IntParameter) iX_param =
    Handle(ActData_IntParameter)::DownCast( iX_node->Parameter(ActData_IntVarNode::Param_Value) );

  TCollection_AsciiString INIT_STRING1("rX+sin(rX)-rX + rX");
  TCollection_AsciiString INIT_STRING2("sqrt(rX)*rX");
  TCollection_AsciiString EXPECTED_STRING1("KAPPA+sin(KAPPA)-KAPPA + KAPPA");
  TCollection_AsciiString EXPECTED_STRING2("sqrt(KAPPA)*KAPPA");

  M->OpenCommand();

  rX_param->SetValue(1.0);
  rY_param->SetEvalString(INIT_STRING1);
  iX_param->SetEvalString(INIT_STRING2);

  // Real Variable 2 depends on Real Variable 1
  rY_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << rX_node->Parameter(ActData_RealVarNode::Param_Value) );

  // Integer Variable 1 depends on Real Variable 1
  iX_node->ConnectEvaluator( ActData_IntVarNode::Param_Value,
                             ActAPI_ParameterStream() << rX_node->Parameter(ActData_RealVarNode::Param_Value) );

  // Rename Variable Node 1
  rX_node->RenameConnected("KAPPA");

  M->CommitCommand();

  // Verify results
  TEST_VERIFY( ActAux::are_equal( rY_param->GetEvalString().ToCString(), EXPECTED_STRING1.ToCString() ) )
  TEST_VERIFY( ActAux::are_equal( iX_param->GetEvalString().ToCString(), EXPECTED_STRING2.ToCString() ) )

  return outcome().success();
}

//! Test function for renaming functionality for Variable Nodes. This one will
//! test some special characters in Variable names, e.g. "_".
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::renameVariable3(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActData_RealVarNode)
    rX_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );
  Handle(ActData_RealVarNode)
    rY_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(5) ) );

  Handle(ActData_RealParameter) rX_param =
    Handle(ActData_RealParameter)::DownCast( rX_node->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) rY_param =
    Handle(ActData_RealParameter)::DownCast( rY_node->Parameter(ActData_RealVarNode::Param_Value) );

  TCollection_AsciiString INIT_STRING("rX");
  TCollection_AsciiString EXPECTED_STRING("rX_KAPPA");

  M->OpenCommand();

  rX_param->SetValue(1.0);
  rY_param->SetEvalString(INIT_STRING);

  // Real Variable 2 depends on Real Variable 1
  rY_node->ConnectEvaluator( ActData_RealVarNode::Param_Value,
                             ActAPI_ParameterStream() << rX_node->Parameter(ActData_RealVarNode::Param_Value) );

  // Rename Variable Node 1
  rX_node->RenameConnected("rX_KAPPA");

  M->CommitCommand();

  // Verify results
  TEST_VERIFY( ActAux::are_equal( rY_param->GetEvalString().ToCString(), EXPECTED_STRING.ToCString() ) )

  return outcome().success();
}

//! Test for AddVariable functionality on BaseModel.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_BaseModelEvaluation::addVariable(const int asiTestEngine_NotUsed(funcID))
{
  // Create and populate sample Model
  Handle(ActAPI_IModel) M;
  NCollection_Sequence<ActAPI_DataObjectId> node_IDs;
  init(M, node_IDs);

  Handle(ActData_RealVarNode)
    rX_node = Handle(ActData_RealVarNode)::DownCast( M->FindNode( node_IDs(4) ) );

  Handle(ActData_RealParameter) rX_param =
    Handle(ActData_RealParameter)::DownCast( rX_node->Parameter(ActData_RealVarNode::Param_Value) );

  /* =======================================================================
   *  Referenced non-existing variable, so no connections are actually done
   *  and evaluation string remains "dead"
   * ======================================================================= */

  TCollection_AsciiString INV_VARNAME("rX_KAPPA");
  TCollection_AsciiString
    INV_EXPRESSION = TCollection_AsciiString("2*").Cat(INV_VARNAME).Cat("+3.14");

  M->OpenCommand();
  rX_param->SetEvalString(INV_EXPRESSION);
  M->CommitCommand();

  TEST_VERIFY( !rX_node->HasConnectedEvaluator(ActData_BaseVarNode::Param_Value) )

  /* ==============================================================
   *  Now add Variable using comprehensive Data Model abilities to
   *  re-connect "dead" evaluators
   * ============================================================== */

  M->OpenCommand();
  M->AddVariable(ActAPI_IModel::Variable_Real, INV_VARNAME);
  M->CommitCommand();

  /* ================
   *  Verify results
   * ================ */

  // Evaluator must have been connected
  TEST_VERIFY( rX_node->HasConnectedEvaluator(ActData_BaseVarNode::Param_Value) )

  return outcome().success();
}

#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
