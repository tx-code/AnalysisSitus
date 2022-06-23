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
#include <ActTest_CopyPasteEngine.h>

// Active Data unit tests
#include <ActTest_DummyTreeFunction.h>
#include <ActTest_StubANode.h>
#include <ActTest_StubBNode.h>
#include <ActTest_StubMeshNode.h>

// ACT Test Library includes
#include <asiTestEngine_Utils.h>
#include <asiTestEngine_Launcher.h>

// Active Data includes
#include <ActData_CAFDumper.h>
#include <ActData_RealEvaluatorFunc.h>
#include <ActData_RealVarNode.h>
#include <ActData_Utils.h>

// Mesh includes
#include <ActData_Mesh_Node.h>

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------
// Business logic
//-----------------------------------------------------------------------------

//! Performs test on Copy method.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_PlainToPlain(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_PlainToPlain");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ===================================
   *  Prepare initial data to be copied
   * =================================== */

  TEST_VERIFY( M->NewEmpty(), DescriptionFn(), funcID );

  // Prepare detached data
  Handle(ActTest_StubBNode) aSourceNode_B = Handle(ActTest_StubBNode)::DownCast( ActTest_StubBNode::Instance() );
  Handle(ActTest_StubANode) aTargetNode_A = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  TopoDS_Shape aTargetShapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() };
  Standard_Real aTargetRealVal = asiTestEngine_Utils::RandomReal();
  Standard_Integer aSourceIntVal = asiTestEngine_Utils::RandomInteger();
  Standard_Integer aSourceAltIntVal = -aSourceIntVal;
  Standard_Real aSourceRealVal = asiTestEngine_Utils::RandomReal();
  Standard_Real aSourceAltRealVal = -aSourceRealVal;
  
  // Attach data to the CAF Document
  M->OpenCommand();
  M->StubAPartition()->AddNode(aTargetNode_A); // This will be a ROOT Node
  M->StubBPartition()->AddNode(aSourceNode_B);
  aTargetNode_A->SetName("Parent A");
  aTargetNode_A->Init(aTargetShapes[0], aTargetShapes[1], aTargetRealVal);
  aSourceNode_B->SetName("Source B");
  aSourceNode_B->Init(aSourceIntVal, aSourceRealVal);
  TEST_VERIFY( aSourceNode_B->IsWellFormed(), DescriptionFn(), funcID );
  TEST_VERIFY( aTargetNode_A->IsWellFormed(), DescriptionFn(), funcID );
  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureOnly);

  /* ==============
   *  Perform Copy
   * ============== */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(aSourceNode_B);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  // Access the Data Node which has been just created for buffering purpose.
  // Such Nodes have their own dedicated Label in the Data Model. We can
  // access this Label and settle down a Node onto it in order to check if
  // data is generally Ok
  Handle(ActTest_StubBNode) aBufferNode = Handle(ActTest_StubBNode)::DownCast( tool->GetRootBuffered() );
  TEST_VERIFY( aBufferNode->IsWellFormed(), DescriptionFn(), funcID );

  // Do something with source Node to assure that copy is not affected
  M->OpenCommand();
  aSourceNode_B->Init(aSourceAltIntVal, aSourceAltRealVal);
  M->CommitCommand();

  TEST_VERIFY( aSourceNode_B->GetIntValue()  == aSourceAltIntVal,  DescriptionFn(), funcID );
  TEST_VERIFY( aSourceNode_B->GetRealValue() == aSourceAltRealVal, DescriptionFn(), funcID );

  /* ===============
   *  Perform Paste
   * =============== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  aTargetNode_A->AddChildNode(aSourceCopy);
  M->CommitCommand();

  TEST_VERIFY( !aSourceCopy.IsNull(), DescriptionFn(), funcID );
  TEST_VERIFY( aSourceCopy->IsWellFormed(), DescriptionFn(), funcID );

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureOnly);

  Handle(ActTest_StubBNode) aSourceCopyStub = Handle(ActTest_StubBNode)::DownCast(aSourceCopy);

  // Values should be equal to the initial sources
  TEST_VERIFY( aSourceCopyStub->GetIntValue()  == aSourceIntVal,  DescriptionFn(), funcID );
  TEST_VERIFY( aSourceCopyStub->GetRealValue() == aSourceRealVal, DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on Copy method with the following Nodes:
//! ---------------------------------------------------------------------------
//!
//!  A [ROOT] ---+---> A_1 ---+---> B_1
//!              |            |
//!              |            +---> A_2 ---+---> B_2
//!              |                         |
//!              |                         +---> A_3
//!              |
//!              +---> B_3
//!
//! ---------------------------------------------------------------------------
//! Here A_1 is the sub-tree being copied, while B_3 is the target Node.
//! The expected results are as follows:
//! ---------------------------------------------------------------------------
//!
//!  A [ROOT] ---+---> A_1 ---+---> B_1
//!              |            |
//!              |            +---> A_2 ---+---> B_2
//!              |                         |
//!              |                         +---> A_3
//!              |
//!              +---> B_3 ---+---> A_1 ---+---> B_1
//!                                        |
//!                                        +---> A_2 ---+---> B_2
//!                                                     |
//!                                                     +---> A_3
//!
//! ---------------------------------------------------------------------------
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeToPlain(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeToPlain");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  // Prepare data for tree verification
  NCollection_Sequence<ActAPI_DataObjectId> TreeLevel_1;
  TreeLevel_1.Append( A_1->GetId() );
  NCollection_Sequence<ActAPI_DataObjectId> TreeLevel_2;
  TreeLevel_2.Append( B_1->GetId() );
  TreeLevel_2.Append( A_2->GetId() );
  NCollection_Sequence<ActAPI_DataObjectId> TreeLevel_3;
  TreeLevel_3.Append( B_2->GetId() );
  TreeLevel_3.Append( A_3->GetId() );
  TreeLevelSeq TreeLevels[] = {TreeLevel_1, TreeLevel_2, TreeLevel_3};

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* ======================================================
   *  Now we delete the initial sub-tree in order to check
   *  that the prepared copy is fully independent
   * ====================================================== */

  M->OpenCommand();
  TEST_VERIFY( M->DeleteNode( A_1->GetId() ), DescriptionFn(), funcID );
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =========================================
   *  Now check the cloned hierarchy of Nodes
   * ========================================= */
  
  isOk = Standard_True;
  verifyTree(aSourceCopy, COPY_Reloc, PASTE_Reloc, TreeLevels, 0, 1, isOk);
  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 3.1 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithReferencesToPlain_1(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithReferencesToPlain_1");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* =====================================
   *  Establish Tree Function connections
   * ===================================== */

  M->OpenCommand();

  A_2->ConnectTreeFunction( ActTest_StubANode::PID_TFunc,
                            ActTest_DummyTreeFunction::GUID(),
                            ActAPI_ParameterStream() << B_2->Parameter(ActTest_StubBNode::PID_Int)
                                                     << A_3->Parameter(ActTest_StubANode::PID_Real),
                            ActAPI_ParameterStream() << B_1->Parameter(ActTest_StubBNode::PID_Int) );

  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==========================================
   *  Verify relocation of involved references
   * ========================================== */

  Handle(ActAPI_IUserParameter) A_2_copy_TFunc = A_2_copy->Parameter(ActTest_StubANode::PID_TFunc);

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_2), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_3_copy), DescriptionFn(), funcID ); // !!!
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_2_copy), DescriptionFn(), funcID ); // !!!

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_1_copy), DescriptionFn(), funcID ); // !!!
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_2), DescriptionFn(), funcID );

  // Precise the actual Parameters referred to by Tree Function one
  TEST_VERIFY( ReferenceValidator::TreeFunction(A_2_copy_TFunc).HasAsArgument( A_3_copy->Parameter(ActTest_StubANode::PID_Real) ),
               DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::TreeFunction(A_2_copy_TFunc).HasAsArgument( B_2_copy->Parameter(ActTest_StubBNode::PID_Int) ),
               DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::TreeFunction(A_2_copy_TFunc).HasAsResult( B_1_copy->Parameter(ActTest_StubBNode::PID_Int) ),
               DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 3.2 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithReferencesToPlain_2(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithReferencesToPlain_2");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* =====================================
   *  Establish Tree Function connections
   * ===================================== */

  M->OpenCommand();

  A_2->ConnectTreeFunction( ActTest_StubANode::PID_TFunc,
                            ActTest_DummyTreeFunction::GUID(),
                            ActAPI_ParameterStream() << B_2->Parameter(ActTest_StubBNode::PID_Int)
                                                     << B_3->Parameter(ActTest_StubBNode::PID_Int),
                            ActAPI_ParameterStream() << B_1->Parameter(ActTest_StubBNode::PID_Int) );

  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==========================================
   *  Verify relocation of involved references
   * ========================================== */

  Handle(ActAPI_IUserParameter) A_2_copy_TFunc = A_2_copy->Parameter(ActTest_StubANode::PID_TFunc);

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_2), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_2), DescriptionFn(), funcID );

  TEST_VERIFY( ActData_ParameterFactory::AsTreeFunction(A_2_copy_TFunc)->Arguments().IsNull(), DescriptionFn(), funcID );
  TEST_VERIFY( ActData_ParameterFactory::AsTreeFunction(A_2_copy_TFunc)->Results().IsNull(), DescriptionFn(), funcID );

  /* ==================================================
   *  Verify that the source references is still there
   * ================================================== */

  Handle(ActAPI_IUserParameter) A_2_TFunc = A_2->Parameter(ActTest_StubANode::PID_TFunc);

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_2), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_3), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_3), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_2_copy), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_2).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_3).HasObservers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasObservers(), DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 3.3 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithReferencesToPlain_3(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithReferencesToPlain_3");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* =====================================
   *  Establish Tree Function connections
   * ===================================== */

  M->OpenCommand();

  A_2->ConnectTreeFunction( ActTest_StubANode::PID_TFunc,
                            ActTest_DummyTreeFunction::GUID(),
                            ActAPI_ParameterStream() << B_2->Parameter(ActTest_StubBNode::PID_Int)
                                                     << A_3->Parameter(ActTest_StubANode::PID_Real),
                            ActAPI_ParameterStream() << B_3->Parameter(ActTest_StubBNode::PID_Int) );

  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==========================================
   *  Verify relocation of involved references
   * ========================================== */

  Handle(ActAPI_IUserParameter) A_2_copy_TFunc = A_2_copy->Parameter(ActTest_StubANode::PID_TFunc);

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_2), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_2), DescriptionFn(), funcID );

  TEST_VERIFY( ActData_ParameterFactory::AsTreeFunction(A_2_copy_TFunc)->Arguments().IsNull(), DescriptionFn(), funcID);
  TEST_VERIFY( ActData_ParameterFactory::AsTreeFunction(A_2_copy_TFunc)->Results().IsNull(), DescriptionFn(), funcID);

  /* ==================================================
   *  Verify that the source references is still there
   * ================================================== */

  Handle(ActAPI_IUserParameter) A_2_TFunc = A_2->Parameter(ActTest_StubANode::PID_TFunc);

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_2), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_2), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_3), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_2_copy), DescriptionFn(), funcID );  

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_3).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_2).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_3).HasObservers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasObservers(), DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 3.4 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithReferencesToPlain_4(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithReferencesToPlain_4");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* =====================================
   *  Establish Tree Function connections
   * ===================================== */

  M->OpenCommand();

  B_3->ConnectTreeFunction( ActTest_StubBNode::PID_TFunc,
                            ActTest_DummyTreeFunction::GUID(),
                            ActAPI_ParameterStream() << A_2->Parameter(ActTest_StubANode::PID_Real),
                            ActAPI_ParameterStream() << A_3->Parameter(ActTest_StubANode::PID_Real) );

  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==================================================
   *  Verify that the source references is still there
   * ================================================== */

  Handle(ActAPI_IUserParameter) B_3_TFunc = B_3->Parameter(ActTest_StubBNode::PID_TFunc);

  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(B_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(B_3), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsInputReaderFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(B_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(B_3), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(B_3_TFunc).IsOutputWriterFor(B_2_copy), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_2).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_3).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_3).HasObservers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasObservers(), DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 3.5 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithReferencesToPlain_5(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithReferencesToPlain_5");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* =====================================
   *  Establish Tree Function connections
   * ===================================== */

  M->OpenCommand();

  A_2->ConnectTreeFunction( ActTest_StubANode::PID_TFunc,
                            ActTest_DummyTreeFunction::GUID(),
                            ActAPI_ParameterStream() << A_2->Parameter(ActTest_StubANode::PID_DummyShapeA),
                            ActAPI_ParameterStream() << A_2->Parameter(ActTest_StubANode::PID_Real) );

  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ===================================================
   *  Verify that the source references are still there
   * =================================================== */

  Handle(ActAPI_IUserParameter) A_2_TFunc = A_2->Parameter(ActTest_StubANode::PID_TFunc);

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_3), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsInputReaderFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_3), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_TFunc).IsOutputWriterFor(B_2_copy), DescriptionFn(), funcID );

  /* ==========================
   *  Verify copied references
   * ========================== */

  Handle(ActAPI_IUserParameter) A_2_copy_TFunc = A_2_copy->Parameter(ActTest_StubANode::PID_TFunc);

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_3), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsInputReaderFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_3), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_2), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_3), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_TFunc).IsOutputWriterFor(B_2_copy), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_3).HasObservers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasObservers(), DescriptionFn(), funcID );

  /* ===================
   *  Nano Verification
   * =================== */

  TEST_VERIFY( ReferenceValidator::TreeFunction(A_2_TFunc).HasAsArgument( A_2->Parameter(ActTest_StubANode::PID_DummyShapeA) ),
               DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::TreeFunction(A_2_TFunc).HasAsResult( A_2->Parameter(ActTest_StubANode::PID_Real) ),
               DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::TreeFunction(A_2_copy_TFunc).HasAsArgument( A_2_copy->Parameter(ActTest_StubANode::PID_DummyShapeA) ),
               DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::TreeFunction(A_2_copy_TFunc).HasAsResult( A_2_copy->Parameter(ActTest_StubANode::PID_Real) ),
               DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 4 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithEvalReferencesToPlain(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithEvalReferencesToPlain");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  Handle(ActAPI_INode) VAR_1 = M->FindNode( VARNodeIDs(1) );
  Handle(ActAPI_INode) VAR_2 = M->FindNode( VARNodeIDs(2) );

  Handle(ActData_RealParameter) VAR_1P =
    Handle(ActData_RealParameter)::DownCast( VAR_1->Parameter(ActData_RealVarNode::Param_Value) );
  Handle(ActData_RealParameter) VAR_2P =
    Handle(ActData_RealParameter)::DownCast( VAR_2->Parameter(ActData_RealVarNode::Param_Value) );

  /* ===============================
   *  Connect evaluator to A_2 Node
   * =============================== */

  M->OpenCommand();

  A_2->ConnectEvaluator(ActTest_StubANode::PID_Real,
                        ActAPI_ParameterStream() << VAR_1P << VAR_2P);

  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* ====================================================================
   *  Register Real Evaluator as Tree Function passing out-scoped filter
   * ==================================================================== */

  tool->AccessReferenceFilter().AccessTreeFunctionFilter().PassOutScoped( ActData_RealEvaluatorFunc::GUID() );

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ================================================
   *  Verify that the copied reference is consistent
   * ================================================ */

  Handle(ActAPI_IUserParameter) A_2_copy_Eval = A_2_copy->Evaluator(ActTest_StubANode::PID_Real);

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(A_1),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(A_2),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(A_3),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(B_1),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(B_2),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(B_3),   DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(VAR_1), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(VAR_2), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsInputReaderFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(A_1),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(A_2),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(A_3),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(B_1),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(B_2),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(B_3),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(VAR_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(VAR_2), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_copy_Eval).IsOutputWriterFor(B_2_copy), DescriptionFn(), funcID );

  /* =================================================
   *  Verify that the source reference is still there
   * ================================================= */

  Handle(ActAPI_IUserParameter) A_2_Eval = A_2->Evaluator(ActTest_StubANode::PID_Real);

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(A_1),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(A_2),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(A_3),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(B_1),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(B_2),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(B_3),   DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(VAR_1), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(VAR_2), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsInputReaderFor(B_2_copy), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(A_1),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(A_2),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(A_3),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(B_1),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(B_2),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(B_3),   DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(VAR_1), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(VAR_2), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(A_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(B_1_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::TreeFunction(A_2_Eval).IsOutputWriterFor(B_2_copy), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_3).HasObservers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasObservers(), DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::Node(VAR_1).HasObservers(), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(VAR_2).HasObservers(), DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::Node(VAR_1).InputReadersAre(ActAPI_ParameterStream() << A_2_Eval << A_2_copy_Eval),
               DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(VAR_2).InputReadersAre(ActAPI_ParameterStream() << A_2_Eval << A_2_copy_Eval),
               DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 5.1 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithPlainReferenceToPlain_1(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithPlainReferenceToPlain_1");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* ======================================
   *  Establish plain Reference connection
   * ====================================== */

  M->OpenCommand();
  B_2->ConnectReference(ActTest_StubBNode::PID_Ref, A_2);
  M->CommitCommand();

  TEST_VERIFY( ReferenceValidator::Node(A_2).HasReferrers(), DescriptionFn(), funcID );

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==============================
   *  Verify the copied references
   * ============================== */

  Handle(ActAPI_IUserParameter) B_2_copy_Ref = B_2_copy->Parameter(ActTest_StubBNode::PID_Ref);

  TEST_VERIFY( ReferenceValidator::Reference(B_2_copy_Ref).TargetIs(A_2_copy), DescriptionFn(), funcID );

  /* =================================================
   *  Verify that the source reference is still there
   * ================================================= */

  Handle(ActAPI_IUserParameter) B_2_Ref = B_2->Parameter(ActTest_StubBNode::PID_Ref);

  TEST_VERIFY( ReferenceValidator::Reference(B_2_Ref).TargetIs(A_2), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_3).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_2_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::Node(A_2).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref),
               DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(A_2_copy).ReferrersAre(ActAPI_ParameterStream() << B_2_copy_Ref),
               DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 5.2 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithPlainReferenceToPlain_2(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithPlainReferenceToPlain_2");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* ======================================
   *  Establish plain Reference connection
   * ====================================== */

  M->OpenCommand();
  B_2->ConnectReference(ActTest_StubBNode::PID_Ref, B_3);
  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==============================
   *  Verify the copied references
   * ============================== */

  Handle(ActAPI_IUserParameter) B_2_copy_Ref = B_2_copy->Parameter(ActTest_StubBNode::PID_Ref);

  TEST_VERIFY( ActData_ParameterFactory::AsReference(B_2_copy_Ref)->GetTarget().IsNull(), DescriptionFn(), funcID );

  /* =================================================
   *  Verify that the source reference is still there
   * ================================================= */

  Handle(ActAPI_IUserParameter) B_2_Ref = B_2->Parameter(ActTest_StubBNode::PID_Ref);

  TEST_VERIFY( ReferenceValidator::Reference(B_2_Ref).TargetIs(B_3), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_3).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::Node(B_3).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref), DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 5.3 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithPlainReferenceToPlain_3(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithPlainReferenceToPlain_3");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* ======================================
   *  Establish plain Reference connection
   * ====================================== */

  M->OpenCommand();
  B_2->ConnectReference(ActTest_StubBNode::PID_Ref, B_3);
  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* ==========================================================
   *  Configure Reference Filter to allow out-scoped Reference
   * ========================================================== */

  tool->AccessReferenceFilter().AccessRefParamFilter()
         .PassOutScoped(B_2->GetTypeName(), ActTest_StubBNode::PID_Ref);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==============================
   *  Verify the copied references
   * ============================== */

  Handle(ActAPI_IUserParameter) B_2_copy_Ref = B_2_copy->Parameter(ActTest_StubBNode::PID_Ref);

  TEST_VERIFY( ReferenceValidator::Reference(B_2_copy_Ref).TargetIs(B_3), DescriptionFn(), funcID );

  /* =================================================
   *  Verify that the source reference is still there
   * ================================================= */

  Handle(ActAPI_IUserParameter) B_2_Ref = B_2->Parameter(ActTest_StubBNode::PID_Ref);

  TEST_VERIFY( ReferenceValidator::Reference(B_2_Ref).TargetIs(B_3), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_3).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::Node(B_3).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref << B_2_copy_Ref),
               DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 5.4 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithPlainReferenceToPlain_4(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithPlainReferenceToPlain_4");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* ======================================
   *  Establish plain Reference connection
   * ====================================== */

  M->OpenCommand();
  B_2->ConnectReference(ActTest_StubBNode::PID_Ref, B_3);
  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* ==========================================================
   *  Configure Reference Filter to allow out-scoped Reference
   * ========================================================== */

  tool->AccessReferenceFilter().AccessRefParamFilter()
         .PassOutScoped(B_2->GetTypeName(), ActTest_StubBNode::PID_Ref);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==============================
   *  Verify the copied references
   * ============================== */

  Handle(ActAPI_IUserParameter) B_2_copy_Ref = B_2_copy->Parameter(ActTest_StubBNode::PID_Ref);

  TEST_VERIFY( ReferenceValidator::Reference(B_2_copy_Ref).TargetIs(B_3), DescriptionFn(), funcID );

  /* ==================================
   *  Copy A_1_copy sub-tree to buffer
   * ================================== */

  M->OpenCommand();
  isOk = tool->TransferToBuffer(A_1_copy);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgressNC = dumpPath().Cat(fn_name).Cat("_progress_nextcopy").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgressNC, M);

  ActData_CopyPasteEngine::RelocationTable Dbl_COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* =======================================
   *  Restore A_1_copy sub-tree from buffer
   * ======================================= */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceDblCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceDblCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfterNC = dumpPath().Cat(fn_name).Cat("_after_nextcopy").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterNC, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTNNC = dumpPath().Cat(fn_name).Cat("_after_TN_nextcopy").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTNNC, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable Dbl_PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy_copy = ActData_NodeFactory::NodeSettle( Dbl_PASTE_Reloc.Find1( Dbl_COPY_Reloc.Find1( A_1_copy->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy_copy = ActData_NodeFactory::NodeSettle( Dbl_PASTE_Reloc.Find1( Dbl_COPY_Reloc.Find1( A_2_copy->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy_copy = ActData_NodeFactory::NodeSettle( Dbl_PASTE_Reloc.Find1( Dbl_COPY_Reloc.Find1( A_3_copy->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy_copy = ActData_NodeFactory::NodeSettle( Dbl_PASTE_Reloc.Find1( Dbl_COPY_Reloc.Find1( B_1_copy->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy_copy = ActData_NodeFactory::NodeSettle( Dbl_PASTE_Reloc.Find1( Dbl_COPY_Reloc.Find1( B_2_copy->RootLabel() ) ) );

  /* ==============================
   *  Verify the copied references
   * ============================== */

  Handle(ActAPI_IUserParameter) B_2_copy_copy_Ref = B_2_copy_copy->Parameter(ActTest_StubBNode::PID_Ref);

  TEST_VERIFY( ReferenceValidator::Reference(B_2_copy_copy_Ref).TargetIs(B_3), DescriptionFn(), funcID );

  /* =================================================
   *  Verify that the source reference is still there
   * ================================================= */

  Handle(ActAPI_IUserParameter) B_2_Ref = B_2->Parameter(ActTest_StubBNode::PID_Ref);

  TEST_VERIFY( ReferenceValidator::Reference(B_2_Ref).TargetIs(B_3), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_3).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy_copy).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::Node(B_3).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref << B_2_copy_Ref << B_2_copy_copy_Ref),
               DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 6.1 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithListReferenceToPlain_1(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithListReferenceToPlain_1");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* =====================================
   *  Establish Reference List connection
   * ===================================== */

  M->OpenCommand();
  B_2->ConnectReferenceToList(ActTest_StubBNode::PID_RefList, A_2);
  B_2->ConnectReferenceToList(ActTest_StubBNode::PID_RefList, A_3);
  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==============================
   *  Verify the copied references
   * ============================== */

  Handle(ActAPI_IUserParameter) B_2_copy_Ref = B_2_copy->Parameter(ActTest_StubBNode::PID_RefList);

  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_copy_Ref).HasTarget(A_2_copy), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_copy_Ref).HasTarget(A_3_copy), DescriptionFn(), funcID );

  /* =================================================
   *  Verify that the source reference is still there
   * ================================================= */

  Handle(ActAPI_IUserParameter) B_2_Ref = B_2->Parameter(ActTest_StubBNode::PID_RefList);

  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_Ref).HasTarget(A_2), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_Ref).HasTarget(A_3), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_4).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_2_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_3_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::Node(A_2).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(A_3).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(A_2_copy).ReferrersAre(ActAPI_ParameterStream() << B_2_copy_Ref), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(A_3_copy).ReferrersAre(ActAPI_ParameterStream() << B_2_copy_Ref), DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 6.2 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithListReferenceToPlain_2(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithListReferenceToPlain_2");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* =====================================
   *  Establish Reference List connection
   * ===================================== */

  M->OpenCommand();
  B_2->ConnectReferenceToList(ActTest_StubBNode::PID_RefList, B_3);
  B_2->ConnectReferenceToList(ActTest_StubBNode::PID_RefList, B_4);
  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY(isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==============================
   *  Verify the copied references
   * ============================== */

  Handle(ActAPI_IUserParameter) B_2_copy_Ref = B_2_copy->Parameter(ActTest_StubBNode::PID_RefList);

  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_copy_Ref).IsEmpty(), DescriptionFn(), funcID );

  /* =================================================
   *  Verify that the source reference is still there
   * ================================================= */

  Handle(ActAPI_IUserParameter) B_2_Ref = B_2->Parameter(ActTest_StubBNode::PID_RefList);

  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_Ref).HasTarget(B_3), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_Ref).HasTarget(B_4), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_4).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_3_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::Node(B_3).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(B_4).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref), DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 6.3 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithListReferenceToPlain_3(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithListReferenceToPlain_3");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* =====================================
   *  Establish Reference List connection
   * ===================================== */

  M->OpenCommand();
  B_2->ConnectReferenceToList(ActTest_StubBNode::PID_RefList, B_3);
  B_2->ConnectReferenceToList(ActTest_StubBNode::PID_RefList, A_3);
  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY(isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==============================
   *  Verify the copied references
   * ============================== */

  Handle(ActAPI_IUserParameter) B_2_copy_Ref = B_2_copy->Parameter(ActTest_StubBNode::PID_RefList);

  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_copy_Ref).HasTarget(A_3_copy), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::ReferenceList(B_2_copy_Ref).HasTarget(B_3), DescriptionFn(), funcID );

  /* =================================================
   *  Verify that the source reference is still there
   * ================================================= */

  Handle(ActAPI_IUserParameter) B_2_Ref = B_2->Parameter(ActTest_StubBNode::PID_RefList);

  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_Ref).HasTarget(B_3), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_Ref).HasTarget(A_3), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_4).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_3_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::Node(B_3).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(A_3).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(A_3_copy).ReferrersAre(ActAPI_ParameterStream() << B_2_copy_Ref), DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 6.4 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_TreeWithListReferenceToPlain_4(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_TreeWithListReferenceToPlain_4");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ======================
   *  Prepare initial data
   * ====================== */

  ActAPI_DataObjectIdList ANodeIDs, BNodeIDs, VARNodeIDs;
  populateSampleTree(M, ANodeIDs, BNodeIDs, VARNodeIDs, DescriptionFn(), funcID);

  Handle(ActAPI_INode) A   = M->FindNode( ANodeIDs(1) );
  Handle(ActAPI_INode) A_1 = M->FindNode( ANodeIDs(2) );
  Handle(ActAPI_INode) A_2 = M->FindNode( ANodeIDs(3) );
  Handle(ActAPI_INode) A_3 = M->FindNode( ANodeIDs(4) );

  Handle(ActAPI_INode) B_1 = M->FindNode( BNodeIDs(1) );
  Handle(ActAPI_INode) B_2 = M->FindNode( BNodeIDs(2) );
  Handle(ActAPI_INode) B_3 = M->FindNode( BNodeIDs(3) );
  Handle(ActAPI_INode) B_4 = M->FindNode( BNodeIDs(4) );

  /* =====================================
   *  Establish Reference List connection
   * ===================================== */

  M->OpenCommand();
  B_2->ConnectReferenceToList(ActTest_StubBNode::PID_RefList, B_3);
  B_2->ConnectReferenceToList(ActTest_StubBNode::PID_RefList, A_3);
  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  /* ============================
   *  Configure Reference Filter
   * ============================ */

  tool->AccessReferenceFilter()
         .AccessRefParamFilter()
         .PassOutScoped(B_3->GetTypeName(), ActTest_StubBNode::PID_RefList);

  /* =============================
   *  Copy A_1 sub-tree to buffer
   * ============================= */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(A_1);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  ActData_CopyPasteEngine::RelocationTable COPY_Reloc = tool->GetRelocationTable(Standard_True);

  /* ==================================
   *  Restore A_1 sub-tree from buffer
   * ================================== */

  M->OpenCommand();
  Handle(ActAPI_INode) aSourceCopy = tool->RestoreFromBuffer();
  B_4->AddChildNode(aSourceCopy);
  M->CommitCommand();

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureNodesOnly);

  ActData_CopyPasteEngine::RelocationTable PASTE_Reloc = tool->GetRelocationTable(Standard_False);

  /* =============================
   *  Access all resulting copies
   * ============================= */

  Handle(ActAPI_INode) A_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_1->RootLabel() ) ) );
  Handle(ActAPI_INode) A_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_2->RootLabel() ) ) );
  Handle(ActAPI_INode) A_3_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( A_3->RootLabel() ) ) );

  Handle(ActAPI_INode) B_1_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_1->RootLabel() ) ) );
  Handle(ActAPI_INode) B_2_copy = ActData_NodeFactory::NodeSettle( PASTE_Reloc.Find1( COPY_Reloc.Find1( B_2->RootLabel() ) ) );

  /* ==============================
   *  Verify the copied references
   * ============================== */

  Handle(ActAPI_IUserParameter) B_2_copy_Ref = B_2_copy->Parameter(ActTest_StubBNode::PID_RefList);

  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_copy_Ref).HasTarget(B_3), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_copy_Ref).HasTarget(A_3_copy), DescriptionFn(), funcID );

  /* =================================================
   *  Verify that the source reference is still there
   * ================================================= */

  Handle(ActAPI_IUserParameter) B_2_Ref = B_2->Parameter(ActTest_StubBNode::PID_RefList);

  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_Ref).HasTarget(B_3), DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::ReferenceList(B_2_Ref).HasTarget(A_3), DescriptionFn(), funcID );

  /* =====================================
   *  Verification from Nodal perspective
   * ===================================== */

  TEST_VERIFY( !ReferenceValidator::Node(A_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(B_3).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_4).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( !ReferenceValidator::Node(A_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(A_2_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY(  ReferenceValidator::Node(A_3_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_1_copy).HasReferrers(), DescriptionFn(), funcID );
  TEST_VERIFY( !ReferenceValidator::Node(B_2_copy).HasReferrers(), DescriptionFn(), funcID );

  TEST_VERIFY( ReferenceValidator::Node(B_3).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref << B_2_copy_Ref),
               DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(A_3).ReferrersAre(ActAPI_ParameterStream() << B_2_Ref),
               DescriptionFn(), funcID );
  TEST_VERIFY( ReferenceValidator::Node(A_3_copy).ReferrersAre(ActAPI_ParameterStream() << B_2_copy_Ref),
               DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! See case 7 in UNIT-TESTING section of reference documentation.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_PlainMeshToPlain(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_PlainMeshToPlain");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ===================================
   *  Prepare initial data to be copied
   * =================================== */

  TEST_VERIFY( M->NewEmpty(), DescriptionFn(), funcID );

  // Prepare detached data
  Handle(ActTest_StubMeshNode) MESH = Handle(ActTest_StubMeshNode)::DownCast( ActTest_StubMeshNode::Instance() );
  Handle(ActTest_StubANode) A = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  TopoDS_Shape aTargetShapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() };
  Standard_Real aTargetRealVal = asiTestEngine_Utils::RandomReal();

  // Attach data to the CAF Document
  M->OpenCommand();
  M->StubAPartition()->AddNode(A); // This will be a ROOT Node
  M->StubMeshPartition()->AddNode(MESH);
  A->SetName("Parent A");
  A->Init(aTargetShapes[0], aTargetShapes[1], aTargetRealVal);
  MESH->SetName("Mesh Node");
  MESH->Init(new ActData_Mesh);
  TEST_VERIFY( A->IsWellFormed(), DescriptionFn(), funcID );
  TEST_VERIFY( MESH->IsWellFormed(), DescriptionFn(), funcID );
  M->CommitCommand();

  // Prepare simple test mesh
  M->OpenCommand();
  Handle(ActData_MeshParameter)
    ActData_Mesh_P = ActData_ParameterFactory::AsMesh( MESH->Parameter(ActTest_StubMeshNode::Param_Mesh) );
  Standard_Integer NODES[] = { ActData_Mesh_P->AddNode(0, 0, 0),
                               ActData_Mesh_P->AddNode(0, 0, 1),
                               ActData_Mesh_P->AddNode(0, 1, 0) };
  ActData_Mesh_P->AddElement(NODES, 3);
  M->CommitCommand();

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureOnly);

  /* ==============
   *  Perform Copy
   * ============== */

  M->OpenCommand();
  Standard_Boolean isOk = tool->TransferToBuffer(MESH);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  /* ===============
   *  Perform Paste
   * =============== */

  M->OpenCommand();
  Handle(ActAPI_INode) ActData_Mesh_COPY = tool->RestoreFromBuffer();
  A->AddChildNode(ActData_Mesh_COPY);
  M->CommitCommand();

  TEST_VERIFY( !ActData_Mesh_COPY.IsNull(), DescriptionFn(), funcID );
  TEST_VERIFY( ActData_Mesh_COPY->IsWellFormed(), DescriptionFn(), funcID );

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureOnly);

  /* ======================
   *  Perform verification
   * ====================== */

  Handle(ActTest_StubMeshNode) ActData_Mesh_COPY_STUB = Handle(ActTest_StubMeshNode)::DownCast(ActData_Mesh_COPY);

  Handle(ActData_MeshParameter)
    ActData_Mesh_COPY_P = ActData_ParameterFactory::AsMesh( MESH->Parameter(ActTest_StubMeshNode::Param_Mesh) );

  Handle(ActData_Mesh) aMeshDS_Copy = ActData_Mesh_COPY_P->GetMesh();

  for ( Standard_Integer k = 0; k < 3; k++ )
  {
    TEST_VERIFY( aMeshDS_Copy->FindNode(NODES[k])->X() == ActData_Mesh_P->GetMesh()->FindNode(NODES[k])->X(), DescriptionFn(), funcID );
    TEST_VERIFY( aMeshDS_Copy->FindNode(NODES[k])->Y() == ActData_Mesh_P->GetMesh()->FindNode(NODES[k])->Y(), DescriptionFn(), funcID );
    TEST_VERIFY( aMeshDS_Copy->FindNode(NODES[k])->Z() == ActData_Mesh_P->GetMesh()->FindNode(NODES[k])->Z(), DescriptionFn(), funcID );
  }

  return outcome(DescriptionFn(), funcID).success();
}

//! Attempts to paste simple Node having Tree Function Parameter with
//! outdated argument. This can happen if the problematic argument has
//! been deleted between Copy & Paste invocations.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_PasteWithDEAD_DFunctionArgument(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_PasteWithDEAD_DFunctionArgument");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ===================================
   *  Prepare initial data to be copied
   * =================================== */

  TEST_VERIFY( M->NewEmpty(), DescriptionFn(), funcID );

  // Prepare detached data for root
  Handle(ActTest_StubANode) ROOT = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  TopoDS_Shape aROOTShapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() };
  Standard_Real aROOTRealVal = asiTestEngine_Utils::RandomReal();

  // Prepare detached data to be copied
  Handle(ActTest_StubANode) A = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  TopoDS_Shape aTargetShapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() };
  Standard_Real aTargetRealVal = asiTestEngine_Utils::RandomReal();

  // Prepare detached data to be used as Function arguments
  Handle(ActTest_StubBNode) B = Handle(ActTest_StubBNode)::DownCast( ActTest_StubBNode::Instance() );
  Standard_Integer aBIntVal = asiTestEngine_Utils::RandomInteger();
  Standard_Real aBRealVal = asiTestEngine_Utils::RandomReal();
  
  // Push data to the CAF Document
  {
    M->OpenCommand();

    M->StubAPartition()->AddNode(ROOT); // This will be a ROOT Node
    M->StubAPartition()->AddNode(A);
    M->StubBPartition()->AddNode(B);

    ROOT->SetName("ROOT A");
    A->SetName("Target A");
    B->SetName("Input B");

    ROOT->Init(aROOTShapes[0], aROOTShapes[1], aROOTRealVal);
    A->Init(aTargetShapes[0], aTargetShapes[1], aTargetRealVal);
    B->Init(aBIntVal, aBRealVal);

    TEST_VERIFY( ROOT->IsWellFormed(), DescriptionFn(), funcID );
    TEST_VERIFY( A->IsWellFormed(), DescriptionFn(), funcID );
    TEST_VERIFY( B->IsWellFormed(), DescriptionFn(), funcID );

    ROOT->AddChildNode(A);
    ROOT->AddChildNode(B);

    A->ConnectTreeFunction( ActTest_StubANode::PID_TFunc, ActTest_DummyTreeFunction::GUID(),
                            ActAPI_ParameterStream() << B->Parameter(ActTest_StubBNode::PID_Real),
                            ActAPI_ParameterStream() << A->Parameter(ActTest_StubANode::PID_Real) );

    M->CommitCommand();
  }

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureOnly);

  /* =============================================================
   *  Set up filtering strategy for out-scoped A <-> B connection
   * ============================================================= */

  M->LoadCopyFuncGUIDs( ActAPI_FuncGUIDStream() << ActTest_DummyTreeFunction::GUID() );

  /* ==============
   *  Perform Copy
   * ============== */

  M->OpenCommand();
  Standard_Boolean isOk = M->CopyNode(A);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  /* ================================================================
   *  Delete Node A and then Node B in order to have DEAD references
   *  in the Copy/Paste buffer
   * ================================================================ */

   M->OpenCommand();
   TEST_VERIFY( M->DeleteNode(A), DescriptionFn(), funcID );
   TEST_VERIFY( M->DeleteNode(B), DescriptionFn(), funcID );
   M->CommitCommand();

   // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgressDel = dumpPath().Cat(fn_name).Cat("_progress_del").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgressDel, M);

  /* ===============
   *  Perform Paste
   * =============== */

  M->OpenCommand();
  Handle(ActAPI_INode) A_COPY = M->PasteAsChild(ROOT);
  M->CommitCommand();

  // Some very preliminary verifications which assure us that pasting has
  // been really performed
  TEST_VERIFY( !A_COPY.IsNull(), DescriptionFn(), funcID );
  TEST_VERIFY( A_COPY->IsWellFormed(), DescriptionFn(), funcID );

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureOnly);

  /* ======================
   *  Perform verification
   * ====================== */

  Standard_Integer PasteStatus = M->CopyPasteEngine()->Status();

  TEST_VERIFY(PasteStatus & ActData_CopyPasteEngine::Status_WarnNullFuncArgument, DescriptionFn(), funcID );
  TEST_VERIFY( A_COPY->IsValidData(), DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! Attempts to paste simple Node having Tree Function Parameter with
//! outdated result. This can happen if the problematic result has
//! been deleted between Copy & Paste invocations.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_PasteWithDEAD_DFunctionResult(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_PasteWithDEAD_DFunctionResult");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ===================================
   *  Prepare initial data to be copied
   * =================================== */

  TEST_VERIFY( M->NewEmpty(), DescriptionFn(), funcID );

  // Prepare detached data for root
  Handle(ActTest_StubANode) ROOT = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  TopoDS_Shape aROOTShapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() };
  Standard_Real aROOTRealVal = asiTestEngine_Utils::RandomReal();

  // Prepare detached data to be copied
  Handle(ActTest_StubANode) A = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  TopoDS_Shape aTargetShapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() };
  Standard_Real aTargetRealVal = asiTestEngine_Utils::RandomReal();

  // Prepare detached data to be used as Function arguments
  Handle(ActTest_StubBNode) B = Handle(ActTest_StubBNode)::DownCast( ActTest_StubBNode::Instance() );
  Standard_Integer aBIntVal = asiTestEngine_Utils::RandomInteger();
  Standard_Real aBRealVal = asiTestEngine_Utils::RandomReal();

  // Push data to the CAF Document
  {
    M->OpenCommand();

    M->StubAPartition()->AddNode(ROOT); // This will be a ROOT Node
    M->StubAPartition()->AddNode(A);
    M->StubBPartition()->AddNode(B);

    ROOT->SetName("ROOT A");
    A->SetName("Target A");
    B->SetName("Input B");

    ROOT->Init(aROOTShapes[0], aROOTShapes[1], aROOTRealVal);
    A->Init(aTargetShapes[0], aTargetShapes[1], aTargetRealVal);
    B->Init(aBIntVal, aBRealVal);

    TEST_VERIFY( ROOT->IsWellFormed(), DescriptionFn(), funcID );
    TEST_VERIFY( A->IsWellFormed(), DescriptionFn(), funcID );
    TEST_VERIFY( B->IsWellFormed(), DescriptionFn(), funcID );

    ROOT->AddChildNode(A);
    ROOT->AddChildNode(B);

    A->ConnectTreeFunction( ActTest_StubANode::PID_TFunc, ActTest_DummyTreeFunction::GUID(),
                            ActAPI_ParameterStream() << A->Parameter(ActTest_StubANode::PID_Real),
                            ActAPI_ParameterStream() << B->Parameter(ActTest_StubBNode::PID_Real) );

    M->CommitCommand();
  }

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureOnly);

  /* =============================================================
   *  Set up filtering strategy for out-scoped A <-> B connection
   * ============================================================= */

  M->LoadCopyFuncGUIDs( ActAPI_FuncGUIDStream() << ActTest_DummyTreeFunction::GUID() );

  /* ==============
   *  Perform Copy
   * ============== */

  M->OpenCommand();
  Standard_Boolean isOk = M->CopyNode(A);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID );

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  /* ================================================================
   *  Delete Node A and then Node B in order to have DEAD references
   *  in the Copy/Paste buffer
   * ================================================================ */

   M->OpenCommand();
   TEST_VERIFY( M->DeleteNode(A), DescriptionFn(), funcID );
   TEST_VERIFY( M->DeleteNode(B), DescriptionFn(), funcID );
   M->CommitCommand();

   // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgressDel = dumpPath().Cat(fn_name).Cat("_progress_del").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgressDel, M);

  /* ===============
   *  Perform Paste
   * =============== */

  M->OpenCommand();
  Handle(ActAPI_INode) A_COPY = M->PasteAsChild(ROOT);
  M->CommitCommand();

  // Some very preliminary verifications which assure us that pasting has
  // been really performed
  TEST_VERIFY( !A_COPY.IsNull(), DescriptionFn(), funcID );
  TEST_VERIFY( A_COPY->IsWellFormed(), DescriptionFn(), funcID );

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureOnly);

  /* ======================
   *  Perform verification
   * ====================== */

  Standard_Integer PasteStatus = M->CopyPasteEngine()->Status();

  TEST_VERIFY(PasteStatus & ActData_CopyPasteEngine::Status_WarnNullFuncResult, DescriptionFn(), funcID );
  TEST_VERIFY( A_COPY->IsValidData(), DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//! Attempts to paste simple Node having Reference Parameter with
//! outdated target. This can happen if the problematic target has
//! been deleted between Copy & Paste invocations.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::testCopyPaste_PasteWithDEADReference(const int funcID)
{
  TCollection_AsciiString fn_name("testCopyPaste_PasteWithDEADReference");
  Handle(ActTest_DummyModel) M = new ActTest_DummyModel;
  Handle(ActData_CopyPasteEngine) tool = new ActData_CopyPasteEngine(M);

  /* ===================================
   *  Prepare initial data to be copied
   * =================================== */

  TEST_VERIFY( M->NewEmpty(), DescriptionFn(), funcID );

  // Prepare detached data for root
  Handle(ActTest_StubANode) ROOT = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  TopoDS_Shape aROOTShapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() };
  Standard_Real aROOTRealVal = asiTestEngine_Utils::RandomReal();

  // Prepare detached data to be copied
  Handle(ActTest_StubANode) A = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  TopoDS_Shape aTargetShapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() };
  Standard_Real aTargetRealVal = asiTestEngine_Utils::RandomReal();

  // Prepare detached data to be used as Function arguments
  Handle(ActTest_StubBNode) B = Handle(ActTest_StubBNode)::DownCast( ActTest_StubBNode::Instance() );
  Standard_Integer aBIntVal = asiTestEngine_Utils::RandomInteger();
  Standard_Real aBRealVal = asiTestEngine_Utils::RandomReal();

  // Push data to the CAF Document
  {
    M->OpenCommand();

    M->StubAPartition()->AddNode(ROOT); // This will be a ROOT Node
    M->StubAPartition()->AddNode(A);
    M->StubBPartition()->AddNode(B);

    ROOT->SetName("ROOT A");
    A->SetName("Target A");
    B->SetName("Input B");

    ROOT->Init(aROOTShapes[0], aROOTShapes[1], aROOTRealVal);
    A->Init(aTargetShapes[0], aTargetShapes[1], aTargetRealVal);
    B->Init(aBIntVal, aBRealVal);

    TEST_VERIFY( ROOT->IsWellFormed(), DescriptionFn(), funcID );
    TEST_VERIFY( A->IsWellFormed(), DescriptionFn(), funcID );
    TEST_VERIFY( B->IsWellFormed(), DescriptionFn(), funcID );

    ROOT->AddChildNode(A);
    ROOT->AddChildNode(B);

    A->ConnectReference(ActTest_StubANode::PID_Ref, B);

    M->CommitCommand();
  }

  // Dump the Model before modifications
  TCollection_AsciiString
    aFilenameBefore = dumpPath().Cat(fn_name).Cat("_before").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBefore, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameBeforeTN = dumpPath().Cat(fn_name).Cat("_before_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameBeforeTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureOnly);

  /* =============================================================
   *  Set up filtering strategy for out-scoped A <-> B connection
   * ============================================================= */

  M->CopyPasteEngine()->AccessReferenceFilter()
                             .AccessRefParamFilter()
                             .PassOutScoped(A->GetTypeName(), ActTest_StubANode::PID_Ref);

  /* ==============
   *  Perform Copy
   * ============== */

  M->OpenCommand();
  Standard_Boolean isOk = M->CopyNode(A);
  M->CommitCommand();

  TEST_VERIFY( isOk, DescriptionFn(), funcID);

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgress = dumpPath().Cat(fn_name).Cat("_progress").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgress, M);

  /* ================================================================
   *  Delete Node A and then Node B in order to have DEAD references
   *  in the Copy/Paste buffer
   * ================================================================ */

   M->OpenCommand();
   TEST_VERIFY( M->DeleteNode(A), DescriptionFn(), funcID );
   TEST_VERIFY( M->DeleteNode(B), DescriptionFn(), funcID );
   M->CommitCommand();

  // Dump the Model just after the copying
  TCollection_AsciiString
    aFilenameProgressDel = dumpPath().Cat(fn_name).Cat("_progress_del").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameProgressDel, M);

  /* ===============
   *  Perform Paste
   * =============== */

  M->OpenCommand();
  Handle(ActAPI_INode) A_COPY = M->PasteAsChild(ROOT);
  M->CommitCommand();

  // Some very preliminary verifications which assure us that pasting has
  // been really performed
  TEST_VERIFY( !A_COPY.IsNull(), DescriptionFn(), funcID );
  TEST_VERIFY( A_COPY->IsWellFormed(), DescriptionFn(), funcID );

  // Dump the Model just after the pasting
  TCollection_AsciiString
    aFilenameAfter = dumpPath().Cat(fn_name).Cat("_after").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfter, M);

  // Dump the Model in Tree Node mode
  TCollection_AsciiString
    aFilenameAfterTN = dumpPath().Cat(fn_name).Cat("_after_TN").Cat(".dump");
  ActData_CAFDumper::Dump(aFilenameAfterTN, M,
                          ActData_CAFDumper::Content_TreeNodes,
                          ActData_CAFDumper::Verbosity_StructureOnly);

  /* ======================
   *  Perform verification
   * ====================== */

  Standard_Integer PasteStatus = M->CopyPasteEngine()->Status();

  TEST_VERIFY(PasteStatus & ActData_CopyPasteEngine::Status_NoError, DescriptionFn(), funcID );
  TEST_VERIFY( A_COPY->IsValidData(), DescriptionFn(), funcID );
  TEST_VERIFY( ActData_ParameterFactory::AsReference( A->Parameter(ActTest_StubANode::PID_Ref) )->GetTarget().IsNull(),
               DescriptionFn(), funcID );

  return outcome(DescriptionFn(), funcID).success();
}

//-----------------------------------------------------------------------------
// Auxiliary functions
//-----------------------------------------------------------------------------

//! Prepares non-trivial initial data for Copy/Paste functionality testing.
//! \param M [in] sample Model.
//! \param ANodeIDs [out] collection of Nodal IDs for Data Nodes of type A.
//! \param BNodeIDs [out] collection of Nodal IDs for Data Nodes of type B.
//! \param VARNodeIDs [out] collection of Nodal IDs for REAL Variable Nodes.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CopyPasteEngine::populateSampleTree(const Handle(ActTest_DummyModel)& M,
                                                    ActAPI_DataObjectIdList& ANodeIDs,
                                                    ActAPI_DataObjectIdList& BNodeIDs,
                                                    ActAPI_DataObjectIdList& VARNodeIDs,
                                                    const std::string&       nameFunc,
                                                    const int                funcID)
{
  TEST_VERIFY( M->NewEmpty(), nameFunc, funcID );

  // Prepare detached A Nodes
  Handle(ActTest_StubANode) A   = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() ),
                            A_1 = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() ),
                            A_2 = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() ),
                            A_3 = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );

  // Prepare detached B Nodes
  Handle(ActTest_StubBNode) B_1 = Handle(ActTest_StubBNode)::DownCast( ActTest_StubBNode::Instance() ),
                            B_2 = Handle(ActTest_StubBNode)::DownCast( ActTest_StubBNode::Instance() ),
                            B_3 = Handle(ActTest_StubBNode)::DownCast( ActTest_StubBNode::Instance() ),
                            B_4 = Handle(ActTest_StubBNode)::DownCast( ActTest_StubBNode::Instance() );

  // Prepare VARIABLE Nodes
  Handle(ActData_RealVarNode) VAR_1 = Handle(ActData_RealVarNode)::DownCast( ActData_RealVarNode::Instance() ),
                              VAR_2 = Handle(ActData_RealVarNode)::DownCast( ActData_RealVarNode::Instance() );

  // Prepare data to initialize the involved A Nodes
  TopoDS_Shape A_Shapes[]   = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() },
               A_1_Shapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() },
               A_2_Shapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() },
               A_3_Shapes[] = { asiTestEngine_Utils::RandomShape(), asiTestEngine_Utils::RandomShape() };
  Standard_Real A_Value   = asiTestEngine_Utils::RandomReal(),
                A_1_Value = asiTestEngine_Utils::RandomReal(),
                A_2_Value = asiTestEngine_Utils::RandomReal(),
                A_3_Value = asiTestEngine_Utils::RandomReal();

  // Prepare data to initialize the involved B Nodes
  Standard_Integer B_1_iValue = asiTestEngine_Utils::RandomInteger(),
                   B_2_iValue = asiTestEngine_Utils::RandomInteger(),
                   B_3_iValue = asiTestEngine_Utils::RandomInteger(),
                   B_4_iValue = asiTestEngine_Utils::RandomInteger();
  Standard_Real B_1_fValue = asiTestEngine_Utils::RandomReal(),
                B_2_fValue = asiTestEngine_Utils::RandomReal(),
                B_3_fValue = asiTestEngine_Utils::RandomReal(),
                B_4_fValue = asiTestEngine_Utils::RandomReal();

  // Prepare data for VARIABLE Nodes
  Standard_Real VAR_1_fValue = 1.0,
                VAR_2_fValue = 2.0;

  // Attach data to the CAF Document
  M->OpenCommand();

  ANodeIDs.Append( M->StubAPartition()->AddNode(A) ); // This will be a ROOT Node
  ANodeIDs.Append( M->StubAPartition()->AddNode(A_1) );
  ANodeIDs.Append( M->StubAPartition()->AddNode(A_2) );
  ANodeIDs.Append( M->StubAPartition()->AddNode(A_3) );
  BNodeIDs.Append( M->StubBPartition()->AddNode(B_1) );
  BNodeIDs.Append( M->StubBPartition()->AddNode(B_2) );
  BNodeIDs.Append( M->StubBPartition()->AddNode(B_3) );
  BNodeIDs.Append( M->StubBPartition()->AddNode(B_4) );
  VARNodeIDs.Append( M->VariablePartition(ActAPI_IModel::Variable_Real)->AddNode(VAR_1) );
  VARNodeIDs.Append( M->VariablePartition(ActAPI_IModel::Variable_Real)->AddNode(VAR_2) );

  A  ->SetName("Root A");
  A_1->SetName("A_1");
  A_2->SetName("A_2");
  A_3->SetName("A_3");
  B_1->SetName("B_1");
  B_2->SetName("B_2");
  B_3->SetName("B_3");
  B_4->SetName("B_4");
  VAR_1->SetName("VAR_1");
  VAR_2->SetName("VAR_2");

  A  ->Init(A_Shapes[0],   A_Shapes[1],   A_Value);
  A_1->Init(A_1_Shapes[0], A_1_Shapes[1], A_1_Value);
  A_2->Init(A_2_Shapes[0], A_2_Shapes[1], A_2_Value);
  A_3->Init(A_3_Shapes[0], A_3_Shapes[1], A_3_Value);

  B_1->Init(B_1_iValue, B_1_fValue);
  B_2->Init(B_2_iValue, B_2_fValue);
  B_3->Init(B_3_iValue, B_3_fValue);
  B_4->Init(B_4_iValue, B_4_fValue);

  VAR_1->Init("v1", VAR_1_fValue);
  VAR_2->Init("v2", VAR_2_fValue);

  M->CommitCommand();

  // Check if initial data is OK
  TEST_VERIFY( A_1->IsWellFormed(),   nameFunc, funcID );
  TEST_VERIFY( A_2->IsWellFormed(),   nameFunc, funcID );
  TEST_VERIFY( A_3->IsWellFormed(),   nameFunc, funcID );
  TEST_VERIFY( B_1->IsWellFormed(),   nameFunc, funcID );
  TEST_VERIFY( B_2->IsWellFormed(),   nameFunc, funcID );
  TEST_VERIFY( B_3->IsWellFormed(),   nameFunc, funcID );
  TEST_VERIFY( B_4->IsWellFormed(),   nameFunc, funcID );
  TEST_VERIFY( VAR_1->IsWellFormed(), nameFunc, funcID );
  TEST_VERIFY( VAR_2->IsWellFormed(), nameFunc, funcID );

  // Build parent-child relationship
  M->OpenCommand();
  A->AddChildNode(A_1);
  A->AddChildNode(B_3);
  A->AddChildNode(B_4);
  A->AddChildNode(VAR_1);
  A->AddChildNode(VAR_2);
  A_1->AddChildNode(B_1);
  A_1->AddChildNode(A_2);
  A_2->AddChildNode(B_2);
  A_2->AddChildNode(A_3);
  M->CommitCommand();

  return outcome(nameFunc, funcID).success();
}

//! Verifies the Nodal sub-tree against the passed pattern. Relocation tables
//! are used to retrieve the ultimate source IDs for each target Data Node
//! which is treated as a result of copying operation.
//! \param theRoot [in] root of the Nodal sub-tree to verify.
//! \param COPYReloc [in] relocation table prepared at COPY stage.
//! \param PASTEReloc [in] relocation table prepared at PASTE stage.
//! \param TREELevels [in] pattern to check the actual tree against.
//! \param LevelIndex [in] tree level to check.
//! \param SiblingIndex [in] sibling to check.
//! \param IsOK [out] verification result.
void
  ActTest_CopyPasteEngine::verifyTree(const Handle(ActAPI_INode)& theRoot,
                                      const ActData_CopyPasteEngine::RelocationTable& COPYReloc,
                                      const ActData_CopyPasteEngine::RelocationTable& PASTEReloc,
                                      TreeLevelSeq* TREELevels,
                                      const Standard_Integer LevelIndex,
                                      const Standard_Integer SiblingIndex,
                                      Standard_Boolean& IsOK)
{
  TDF_Label aLabTrg = theRoot->RootLabel();
  TDF_Label aLabSrc = COPYReloc.Find2( PASTEReloc.Find2(aLabTrg) );

  TCollection_AsciiString aSourceID = ActData_Utils::GetEntry(aLabSrc);
  TCollection_AsciiString anExpectedID = TREELevels[LevelIndex].Value(SiblingIndex);

  if ( !::IsEqual(aSourceID, anExpectedID) )
  {
    IsOK = Standard_False;
    return;
  }

  Standard_Integer i = 0;
  for ( Handle(ActAPI_IChildIterator) it1 = theRoot->GetChildIterator(); it1->More(); it1->Next() )
  {
    Handle(ActAPI_INode) aChild = it1->Value();
    verifyTree(aChild, COPYReloc, PASTEReloc, TREELevels, LevelIndex + 1, ++i, IsOK);
  }
}

//! Returns path where temporary files are dumped.
//! \return dumping path.
TCollection_AsciiString ActTest_CopyPasteEngine::dumpPath()
{
  TCollection_AsciiString
    path = ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ).c_str();
  return path;
}

#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
