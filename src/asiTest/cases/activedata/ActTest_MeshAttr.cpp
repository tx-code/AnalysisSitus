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
#include <ActTest_MeshAttr.h>

// Active Data includes
#include <ActData_Application.h>
#include <ActData_Utils.h>

// asiTestEngine includes
#include <asiTestEngine_Launcher.h>

// OCCT includes
#include <Standard_ImmutableObject.hxx>

// Mesh includes
#include <ActData_Mesh_ElementsIterator.h>
#include <ActData_Mesh_Node.h>
#include <ActData_Mesh_Quadrangle.h>
#include <ActData_Mesh_Triangle.h>

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------

#define SET_MODIFIED_BY_TIMESTAMP( L ) \
  Handle(ActAux_TimeStamp) aTS = ActAux_TimeStampTool::Generate(); \
  Handle(HIntArray) aTSChunked = ActAux_TimeStampTool::AsChunked(aTS); \
  ActData_Utils::InitIntegerArray(L, -1, aTSChunked);

//-----------------------------------------------------------------------------
// ActTest_MeshAttrBase: construction & static initializers
//-----------------------------------------------------------------------------

// Number of allocations of a single value in a triple slot => 8 nodes possible
Standard_Real ActTest_MeshAttrBase::NODES[][3] =
  { {0.0, 0.0, 0.0},   // Node 1
    {0.0, 0.0, 1.1},   // Node 2
    {0.0, 1.1, 0.0},   // Node 3
    {0.0, 1.1, 1.1},   // Node 4
    {1.1, 0.0, 0.0},   // Node 5
    {1.1, 0.0, 1.1},   // Node 6
    {1.1, 1.1, 0.0},   // Node 7
    {1.1, 1.1, 1.1} }; // Node 8

// Number of combinations from 8 by 3 => 56 triangles possible
Standard_Integer ActTest_MeshAttrBase::TRIANGLES[][3] =
  { {1, 2, 3}, {1, 3, 4}, {1, 4, 5}, {1, 5, 6}, {1, 6, 7}, {1, 7, 8},
    {1, 2, 4}, {1, 3, 5}, {1, 4, 6}, {1, 5, 7}, {1, 6, 8},
    {1, 2, 5}, {1, 3, 6}, {1, 4, 7}, {1, 5, 8},
    {1, 2, 6}, {1, 3, 7}, {1, 4, 8},
    {1, 2, 7}, {1, 3, 8},
    {1, 2, 8},
    {2, 3, 4}, {2, 4, 5}, {2, 5, 6}, {2, 6, 7}, {2, 7, 8},
    {2, 3, 5}, {2, 4, 6}, {2, 5, 7}, {2, 6, 8},
    {2, 3, 6}, {2, 4, 7}, {2, 5, 8},
    {2, 3, 7}, {2, 4, 8},
    {2, 3, 8},
    {3, 4, 5}, {3, 5, 6}, {3, 6, 7}, {3, 7, 8},
    {3, 4, 6}, {3, 5, 7}, {3, 6, 8},
    {3, 4, 7}, {3, 5, 8},
    {3, 4, 8},
    {4, 5, 6}, {4, 6, 7}, {4, 7, 8},
    {4, 5, 7}, {4, 6, 8},
    {4, 5, 8},
    {5, 6, 7}, {5, 7, 8},
    {5, 6, 8},
    {6, 7, 8} };

// Number of combinations from 8 by 4 => 70 quadrangles possible
Standard_Integer ActTest_MeshAttrBase::QUADRANGLES[][4] =
  { {1, 2, 3, 4}, {1, 2, 4, 5}, {1, 2, 5, 6}, {1, 2, 6, 7}, {1, 2, 7, 8},
    {1, 2, 3, 5}, {1, 2, 4, 6}, {1, 2, 5, 7}, {1, 2, 6, 8},
    {1, 2, 3, 6}, {1, 2, 4, 7}, {1, 2, 5, 8},
    {1, 2, 3, 7}, {1, 2, 4, 8},
    {1, 2, 3, 8},
    {1, 3, 4, 5}, {1, 3, 5, 6}, {1, 3, 6, 7}, {1, 3, 7, 8},
    {1, 3, 4, 6}, {1, 3, 5, 7}, {1, 3, 6, 8},
    {1, 3, 4, 7}, {1, 3, 5, 8},
    {1, 3, 4, 8},
    {1, 4, 5, 6}, {1, 4, 6, 7}, {1, 4, 7, 8},
    {1, 4, 5, 7}, {1, 4, 6, 8},
    {1, 4, 5, 8},
    {1, 5, 6, 7}, {1, 5, 7, 8},
    {1, 5, 6, 8},
    {1, 6, 7, 8}, // Reached C from 7 by 3
    {2, 3, 4, 5}, {2, 3, 5, 6}, {2, 3, 6, 7}, {2, 3, 7, 8},
    {2, 3, 4, 6}, {2, 3, 5, 7}, {2, 3, 6, 8},
    {2, 3, 4, 7}, {2, 3, 5, 8},
    {2, 3, 4, 8},
    {2, 4, 5, 6}, {2, 4, 6, 7}, {2, 4, 7, 8},
    {2, 4, 5, 7}, {2, 4, 6, 8},
    {2, 4, 5, 8},
    {2, 5, 6, 7}, {2, 5, 7, 8},
    {2, 5, 6, 8},
    {2, 6, 7, 8}, // Reached C from 6 by 3
    {3, 4, 5, 6}, {3, 4, 6, 7}, {3, 4, 7, 8},
    {3, 4, 5, 7}, {3, 4, 6, 8},
    {3, 4, 5, 8},
    {3, 5, 6, 7}, {3, 5, 7, 8},
    {3, 5, 6, 8},
    {3, 6, 7, 8}, // Reached C from 5 by 3
    {4, 5, 6, 7}, {4, 5, 7, 8},
    {4, 5, 6, 8},
    {4, 6, 7, 8}, // Reached C from 4 by 3
    {5, 6, 7, 8}  // Reached C from 3 by 3
  };

Standard_Integer ActTest_MeshAttrBase::NB_NODES =
  sizeof(ActTest_MeshAttrBase::NODES) / (sizeof(Standard_Real) * 3);

Standard_Integer ActTest_MeshAttrBase::NB_TRIANGLES =
  sizeof(ActTest_MeshAttrBase::TRIANGLES) / (sizeof(Standard_Integer) * 3);

Standard_Integer ActTest_MeshAttrBase::NB_QUADRANGLES =
  sizeof(ActTest_MeshAttrBase::QUADRANGLES) / (sizeof(Standard_Integer) * 4);


//-----------------------------------------------------------------------------
// ActTest_MeshAttrBase: common functionality
//-----------------------------------------------------------------------------

//! Initializes Mesh Attribute by attaching it to some CAF Label.
//! \param doc [in] CAF document.
//! \param meshLab [out] mesh Label.
//! \param isImplictTrans [in] indicates whether this method should open (and
//!        then commit) CAF transaction by itself.
//! \return true in case of success, false -- otherwise.
bool ActTest_MeshAttrBase::initializeMeshAttr(const Handle(TDocStd_Document)& doc,
                                              TDF_Label&                      meshLab,
                                              const Standard_Boolean          isImplictTrans)
{
  if ( isImplictTrans )
    doc->NewCommand();

  meshLab = nextParameterLabel(doc);
  ActData_MeshAttr::Set(meshLab);

  if ( isImplictTrans )
    doc->CommitCommand();

  return true;
}

//! Populates Mesh Attribute with test data.
//! \param doc [in] CAF document.
//! \param meshLab [in/out] mesh Label.
//! \param NODE_IDS [out] node IDs registered in the Mesh DS.
//! \param TRIANGLE_IDS [out] triangle element IDs registered in the Mesh DS.
//! \param QUADRANGLE_IDS [out] quadrangle element IDs registered in the
//!        Mesh DS.
//! \param isImplictTrans [in] indicates whether this method should open (and
//!        then commit) CAF transaction by itself.
//! \return true in case of success, false -- otherwise.
outcome ActTest_MeshAttrBase::populateMeshData(const Handle(TDocStd_Document)& doc,
                                               TDF_Label&                      meshLab,
                                               DatumIdList&                    NODE_IDS,
                                               DatumIdList&                    TRIANGLE_IDS,
                                               DatumIdList&                    QUADRANGLE_IDS,
                                               const Standard_Boolean          isImplictTrans,
                                               const std::string&              nameFunc,
                                               const int                       funcID)
{
  if ( isImplictTrans )
    doc->NewCommand();

  if ( !populateMeshNodes(meshLab, NODE_IDS, nameFunc, funcID).ok ||
       !populateMeshTriangles(meshLab, TRIANGLE_IDS, nameFunc, funcID).ok ||
       !populateMeshQuadrangles(meshLab, QUADRANGLE_IDS, nameFunc, funcID).ok )
    return outcome(nameFunc, funcID).failure();

  if ( isImplictTrans )
    doc->CommitCommand();

  return outcome(nameFunc, funcID).success();
}

//! Populates Mesh Attribute with test nodes.
//! \param meshLab [in/out] mesh Label.
//! \param NODE_IDS [out] node IDs registered in the Mesh DS.
//! \return true in case of success, false -- otherwise.
outcome ActTest_MeshAttrBase::populateMeshNodes(TDF_Label&         meshLab,
                                                DatumIdList&       NODE_IDS,
                                                const std::string& nameFunc,
                                                const int          funcID)
{
  Handle(ActData_MeshAttr) aMeshAttr;
  TEST_VERIFY( meshLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr), nameFunc, funcID )

  SET_MODIFIED_BY_TIMESTAMP( meshLab )

  // Populate Mesh DS with nodes
  for ( Standard_Integer i = 0; i < NB_NODES; i++ )
    NODE_IDS.Append( aMeshAttr->AddNode(NODES[i][0], NODES[i][1], NODES[i][2]) );

  return outcome(nameFunc, funcID).success();
}

//! Populates Mesh Attribute with test triangles.
//! \param meshLab [in/out] mesh Label.
//! \param TRIANGLE_IDS [out] triangle IDs registered in the Mesh DS.
//! \return true in case of success, false -- otherwise.
outcome ActTest_MeshAttrBase::populateMeshTriangles(TDF_Label&         meshLab,
                                                    DatumIdList&       TRIANGLE_IDS,
                                                    const std::string& nameFunc,
                                                    const int          funcID)
{
  Handle(ActData_MeshAttr) aMeshAttr;
  TEST_VERIFY( meshLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr), nameFunc, funcID )

  SET_MODIFIED_BY_TIMESTAMP( meshLab )

  // Populate Mesh DS with triangles
  for ( Standard_Integer i = 0; i < NB_TRIANGLES; i++ )
    TRIANGLE_IDS.Append( aMeshAttr->AddElement(TRIANGLES[i], 3) );

  return outcome(nameFunc, funcID).success();
}

//! Populates Mesh Attribute with test quadrangles.
//! \param meshLab [in/out] mesh Label.
//! \param QUADRANGLE_IDS [out] quadrangle IDs registered in the Mesh DS.
//! \return true in case of success, false -- otherwise.
outcome ActTest_MeshAttrBase::populateMeshQuadrangles(TDF_Label&         meshLab,
                                                      DatumIdList&       QUADRANGLE_IDS,
                                                      const std::string& nameFunc,
                                                      const int          funcID)
{
  Handle(ActData_MeshAttr) aMeshAttr;
  TEST_VERIFY( meshLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr), nameFunc, funcID )

  SET_MODIFIED_BY_TIMESTAMP( meshLab )

  // Populate Mesh DS with quadrangles
  for ( Standard_Integer i = 0; i < NB_QUADRANGLES; i++ )
    QUADRANGLE_IDS.Append( aMeshAttr->AddElement(QUADRANGLES[i], 4) );

  return outcome(nameFunc, funcID).success();
}

//-----------------------------------------------------------------------------
// ActTest_MeshAttrBean: business logic
//-----------------------------------------------------------------------------

//! Performs test on accessing data stored in MeshAttrBean.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_MeshAttrBean::meshBeanTest(const int funcID)
{
  TEST_PRINT_DECOR_L("Create new Mesh Attribute");

  /* =======================================
   *  Attach Mesh Attribute to CAF Document
   * ======================================= */

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  TDF_Label meshLab;

  initializeMeshAttr(doc, meshLab);

  Handle(ActData_MeshAttr) aMeshAttr;
  TEST_VERIFY( meshLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr),
               DescriptionFn(), funcID )

  // Access Mesh DS
  Handle(ActData_Mesh) aActData_Mesh_TRANSIENT = aMeshAttr->GetMesh();

  // Check if Mesh DS is created
  TEST_VERIFY( !aActData_Mesh_TRANSIENT.IsNull(),
               DescriptionFn(), funcID )

  /* ==================================================
   *  Populate transient Mesh Attribute with test data
   * ================================================== */

  // Collection of resulting mesh elements (nodes, triangles, quadrangles)
  DatumIdList NODE_IDS, TRIANGLE_IDS, QUADRANGLE_IDS;

  // Populate in transactional scope
  populateMeshData(doc, meshLab, NODE_IDS, TRIANGLE_IDS, QUADRANGLE_IDS);

  /* ==========================================
   *  Read and verify data from Mesh Attribute
   * ========================================== */

  TEST_VERIFY( aActData_Mesh_TRANSIENT->NbNodes() == NB_NODES, DescriptionFn(), funcID )
  TEST_VERIFY( aActData_Mesh_TRANSIENT->NbFaces() == NB_TRIANGLES + NB_QUADRANGLES,
               DescriptionFn(), funcID )

  // Verify nodes
  for ( Standard_Integer i = 0; i < NB_NODES; i++ )
  {
    Standard_Integer aNodeId = i + 1;
    Handle(ActData_Mesh_Node) aNode = aActData_Mesh_TRANSIENT->FindNode(aNodeId);

    TEST_VERIFY( !aNode.IsNull(), DescriptionFn(), funcID )
    TEST_VERIFY( aNode->X() == NODES[i][0], DescriptionFn(), funcID )
    TEST_VERIFY( aNode->Y() == NODES[i][1], DescriptionFn(), funcID )
    TEST_VERIFY( aNode->Z() == NODES[i][2], DescriptionFn(), funcID )
  }

  // Verify elements: can be TRIANGLES or QUADRANGLES
  Standard_Integer anElemId = 0;
  ActData_Mesh_ElementsIterator aMeshIt(aActData_Mesh_TRANSIENT, ActData_Mesh_ET_Face);
  for ( ; aMeshIt.More(); aMeshIt.Next() )
  {
    anElemId++;
    const Handle(ActData_Mesh_Element)& aMeshElem = aMeshIt.GetValue();
    if ( aMeshElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Triangle) ) )
    {
      Handle(ActData_Mesh_Triangle) aTriElem = Handle(ActData_Mesh_Triangle)::DownCast(aMeshElem);
      Standard_Integer aTriNodeIds[3];
      Standard_Integer aNbNodes;
      aTriElem->GetFaceDefinedByNodes(3, aTriNodeIds, aNbNodes);

      TEST_VERIFY( aTriNodeIds[0] == TRIANGLES[anElemId - 1][0], DescriptionFn(), funcID);
      TEST_VERIFY( aTriNodeIds[1] == TRIANGLES[anElemId - 1][1], DescriptionFn(), funcID);
      TEST_VERIFY( aTriNodeIds[2] == TRIANGLES[anElemId - 1][2], DescriptionFn(), funcID);
    }
    else if ( aMeshElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Quadrangle) ) )
    {
      Handle(ActData_Mesh_Quadrangle) aQuadElem = Handle(ActData_Mesh_Quadrangle)::DownCast(aMeshElem);

      Standard_Integer aQuadNodeIds[4];
      Standard_Integer aNbNodes;
      aQuadElem->GetFaceDefinedByNodes(4, aQuadNodeIds, aNbNodes);

      // Here we take an advantage of mesh DS particularity: it generates
      // IDs of the mesh elements sequentially. So we know that quadrangles
      // are located internally after triangles just in the same order as
      // we pushed them before. That is why their IDs are shifted by
      // NB_TRIS from the beginning and we can easily collate them with
      // the initial test data stored in a plain array
      TEST_VERIFY( aQuadNodeIds[0] == QUADRANGLES[anElemId - NB_TRIANGLES - 1][0], DescriptionFn(), funcID );
      TEST_VERIFY( aQuadNodeIds[1] == QUADRANGLES[anElemId - NB_TRIANGLES - 1][1], DescriptionFn(), funcID );
      TEST_VERIFY( aQuadNodeIds[2] == QUADRANGLES[anElemId - NB_TRIANGLES - 1][2], DescriptionFn(), funcID );
      TEST_VERIFY( aQuadNodeIds[3] == QUADRANGLES[anElemId - NB_TRIANGLES - 1][3], DescriptionFn(), funcID );
    }
  }

  return outcome(DescriptionFn(), funcID).success();
}

//-----------------------------------------------------------------------------
// ActTest_MeshAttrTransactional: Business logic
//-----------------------------------------------------------------------------

//! Performs test of Mesh Attribute by different scenarios containing
//! UNDO and REDO actions.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_MeshAttrTransactional::meshTransUndoRedoTest1(const int funcID)
{
  // Collection of resulting mesh elements (nodes, triangles, quadrangles)
  DatumIdList NODE_IDS, TRIANGLE_IDS, QUADRANGLE_IDS;

  /* ==========================================================
   *  Start a working session which populates the Mesh DS with
   *  nodes (transaction #2), triangles (transaction #3)
   *  and quadrangles (transaction #4)
   * ========================================================== */

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  TDF_Label meshLab;

  // Create Mesh Attribute
  doc->NewCommand();
  initializeMeshAttr(doc, meshLab, Standard_False);
  doc->CommitCommand();

  // Populate mesh with nodes
  doc->NewCommand();
  populateMeshNodes(meshLab, NODE_IDS, DescriptionFn(), funcID);
  doc->CommitCommand();

  // Populate mesh with triangles
  doc->NewCommand();
  populateMeshTriangles(meshLab, TRIANGLE_IDS, DescriptionFn(), funcID);
  doc->CommitCommand();

  // Populate mesh with quadrangles
  doc->NewCommand();
  populateMeshQuadrangles(meshLab, QUADRANGLE_IDS, DescriptionFn(), funcID);
  doc->CommitCommand();

  /* ====================================================================
   *  Verify the contents of the Mesh DS after working session completes
   * ==================================================================== */

  Handle(ActData_MeshAttr) aMeshAttr;
  TEST_VERIFY( meshLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr), DescriptionFn(), funcID )
  const Handle(ActData_Mesh)& aMeshDS = aMeshAttr->GetMesh();
  TEST_VERIFY( aMeshDS->NbNodes() == NB_NODES, DescriptionFn(), funcID )
  TEST_VERIFY( aMeshDS->NbFaces() == NB_TRIANGLES + NB_QUADRANGLES, DescriptionFn(), funcID )

  /* ====================================
   *  Undo changes one-by-one and verify
   * ==================================== */

  doc->Undo();
  TEST_VERIFY( aMeshDS->NbNodes() == NB_NODES, DescriptionFn(), funcID )
  TEST_VERIFY( aMeshDS->NbFaces() == NB_TRIANGLES, DescriptionFn(), funcID ) // Quadrangles disappear

  doc->Undo();
  TEST_VERIFY( aMeshDS->NbNodes() == NB_NODES, DescriptionFn(), funcID )
  TEST_VERIFY( aMeshDS->NbFaces() == 0, DescriptionFn(), funcID ) // Triangles disappear

  doc->Undo();
  TEST_VERIFY( aMeshDS->NbNodes() == 0, DescriptionFn(), funcID ) // Nodes disappear
  TEST_VERIFY( aMeshDS->NbFaces() == 0, DescriptionFn(), funcID )

  doc->Undo();
  Handle(ActData_MeshAttr) aMeshAttr2;
  TEST_VERIFY( !meshLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr2),
               DescriptionFn(), funcID ) // Attribute disappears

  /* ====================================
   *  Redo changes one-by-one and verify
   * ==================================== */

  doc->Redo();
  Handle(ActData_MeshAttr) aMeshAttr3;
  TEST_VERIFY( meshLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr3),
               DescriptionFn(), funcID )

  doc->Redo();
  TEST_VERIFY( aMeshDS->NbNodes() == NB_NODES, DescriptionFn(), funcID )
  TEST_VERIFY( aMeshDS->NbFaces() == 0, DescriptionFn(), funcID )

  doc->Redo();
  TEST_VERIFY( aMeshDS->NbNodes() == NB_NODES, DescriptionFn(), funcID )
  TEST_VERIFY( aMeshDS->NbFaces() == NB_TRIANGLES, DescriptionFn(), funcID )

  doc->Redo();
  TEST_VERIFY( aMeshDS->NbNodes() == NB_NODES, DescriptionFn(), funcID )
  TEST_VERIFY( aMeshDS->NbFaces() == NB_TRIANGLES + NB_QUADRANGLES, DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test of Mesh Attribute with ABORT action.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_MeshAttrTransactional::meshTransAbortTest1(const int funcID)
{
  // Collection of resulting mesh elements (nodes, triangles, quadrangles)
  DatumIdList NODE_IDS, TRIANGLE_IDS, QUADRANGLE_IDS;

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  TDF_Label meshLab;

  // Create & Populate in a single transaction, then ABORT
  doc->NewCommand();
  initializeMeshAttr(doc, meshLab, Standard_False);
  populateMeshData(doc, meshLab, NODE_IDS, TRIANGLE_IDS, QUADRANGLE_IDS, Standard_False);
  doc->AbortCommand();

  // Assure that CAF Document lost the aborted changes as just created
  // Mesh Attribute must have been "Forgotten"
  Handle(ActData_MeshAttr) aMeshAttr;
  TEST_VERIFY( !meshLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr), DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test of Mesh Attribute with ABORT action.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_MeshAttrTransactional::meshTransAbortTest2(const int funcID)
{
  // Collection of resulting mesh elements (nodes, triangles, quadrangles)
  DatumIdList NODE_IDS, TRIANGLE_IDS, QUADRANGLE_IDS;

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  TDF_Label meshLab;

  // Create Mesh Attribute
  doc->OpenCommand();
  initializeMeshAttr(doc, meshLab, Standard_False);
  doc->CommitCommand();

  // Populate Mesh Attribute
  doc->OpenCommand();
  populateMeshData(doc, meshLab, NODE_IDS, TRIANGLE_IDS, QUADRANGLE_IDS, Standard_False);
  doc->AbortCommand();

  // Assure that CAF Document is still here
  Handle(ActData_MeshAttr) aMeshAttr;
  TEST_VERIFY( meshLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr), DescriptionFn(), funcID )
  TEST_VERIFY( aMeshAttr->GetMesh()->NbNodes() == 0, DescriptionFn(), funcID )
  TEST_VERIFY( aMeshAttr->GetMesh()->NbFaces() == 0, DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

//-----------------------------------------------------------------------------
// ActTest_MeshAttrPersistent: business logic
//-----------------------------------------------------------------------------

//! Performs test on saving and restoring Mesh Attribute.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_MeshAttrPersistent::meshSaveOpenTest(const int funcID)
{
  // Collection of resulting mesh elements (nodes, triangles, quadrangles)
  DatumIdList NODE_IDS, TRIANGLE_IDS, QUADRANGLE_IDS;

  /* ==========================================
   *  Initialize Mesh Attribute with test data
   * ========================================== */

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  TDF_Label meshLab;

  doc->OpenCommand();
  initializeMeshAttr(doc, meshLab, Standard_False);
  populateMeshData(doc, meshLab, NODE_IDS, TRIANGLE_IDS, QUADRANGLE_IDS);
  doc->CommitCommand();

  /* =======================
   *  Now save the document
   * ======================= */

  // Prepare filename
  TCollection_AsciiString
    aFilename = (ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ) + "meshSaveOpenTest.cbf").c_str();

  // Save
  PCDM_StoreStatus
    aSaveStat = ActData_Application::Instance()->SaveAs(doc, aFilename);
  TEST_VERIFY( aSaveStat == PCDM_SS_OK, DescriptionFn(), funcID )

  /* ===================================================================
   *  Open the document expecting that Mesh Attribute will be recovered
   * =================================================================== */

  // Preserve tag in order to re-initialize TDF Label after opening
  Standard_Integer anAttrTag = meshLab.Tag();

  // Close the Document first
  ActData_Application::Instance()->Close(doc);

  // Open
  PCDM_ReaderStatus
    anOpenStat = ActData_Application::Instance()->Open(aFilename, doc);
  TEST_VERIFY( anOpenStat == PCDM_RS_OK, DescriptionFn(), funcID )

  // Re-initialize TDF Label
  meshLab = doc->Main().Root().FindChild(anAttrTag, Standard_False);
  TEST_VERIFY( !meshLab.IsNull(), DescriptionFn(), funcID )

  // Access Mesh Attribute
  Handle(ActData_MeshAttr) aMeshAttr;
  TEST_VERIFY( meshLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr), DescriptionFn(), funcID )

  // Access Mesh Data
  Handle(ActData_Mesh) aMeshDS = aMeshAttr->GetMesh();
  TEST_VERIFY( !aMeshDS.IsNull(), DescriptionFn(), funcID )

  /* ==============================================
   *  Verify the contents of the retrieved Mesh DS
   * ============================================== */

  // Verify generalities
  TEST_VERIFY( aMeshDS->NbNodes() == NB_NODES, DescriptionFn(), funcID )
  TEST_VERIFY( aMeshDS->NbFaces() == NB_TRIANGLES + NB_QUADRANGLES, DescriptionFn(), funcID )

  // Verify mesh nodes
  Standard_Integer aNodeIndex = 0;
  ActData_Mesh_ElementsIterator aMeshNodesIt(aMeshDS, ActData_Mesh_ET_Node);
  for ( ; aMeshNodesIt.More(); aMeshNodesIt.Next() )
  {
    // Access next node
    Handle(ActData_Mesh_Node)
      aNode = Handle(ActData_Mesh_Node)::DownCast( aMeshNodesIt.GetValue() );

    // Verify node ID
    TEST_VERIFY( aNode->GetID() == NODE_IDS(aNodeIndex + 1), DescriptionFn(), funcID )

    // Verify nodal co-ordinates
    gp_Pnt aNodeCoords = aNode->Pnt();
    TEST_VERIFY( aNodeCoords.X() == NODES[aNodeIndex][0], DescriptionFn(), funcID )
    TEST_VERIFY( aNodeCoords.Y() == NODES[aNodeIndex][1], DescriptionFn(), funcID )
    TEST_VERIFY( aNodeCoords.Z() == NODES[aNodeIndex][2], DescriptionFn(), funcID )
    aNodeIndex++;
  }

  // Verify mesh elements
  Standard_Integer aTriIndex = 0, aQuadIndex = 0;
  ActData_Mesh_ElementsIterator aMeshElemsIt(aMeshDS, ActData_Mesh_ET_Face);
  for ( ; aMeshElemsIt.More(); aMeshElemsIt.Next() )
  {
    const Handle(ActData_Mesh_Element)& anElem = aMeshElemsIt.GetValue();

    // Proceed with TRIANGLE elements
    if ( anElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Triangle) ) )
    {
      // Access element data
      Handle(ActData_Mesh_Triangle) aTriElem = Handle(ActData_Mesh_Triangle)::DownCast(anElem);

      // Verify element ID
      TEST_VERIFY( aTriElem->GetID() == TRIANGLE_IDS(aTriIndex + 1), DescriptionFn(), funcID )

      Standard_Integer aTriNodeIds[3];
      Standard_Integer aNbNodes;
      aTriElem->GetFaceDefinedByNodes(3, aTriNodeIds, aNbNodes);

      for ( Standard_Integer k = 0; k < 3; k++ )
        TEST_VERIFY( aTriNodeIds[k] == TRIANGLES[aTriIndex][k], DescriptionFn(), funcID )

      aTriIndex++;
    }
    // Proceed with QUADRANGLE elements
    else if ( anElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Quadrangle) ) )
    {
      // Access element data
      Handle(ActData_Mesh_Quadrangle) aQuadElem = Handle(ActData_Mesh_Quadrangle)::DownCast(anElem);

      // Verify element ID
      TEST_VERIFY( aQuadElem->GetID() == QUADRANGLE_IDS(aQuadIndex + 1), DescriptionFn(), funcID )

      Standard_Integer aQuadNodeIds[4];
      Standard_Integer aNbNodes;
      aQuadElem->GetFaceDefinedByNodes(4, aQuadNodeIds, aNbNodes);

      for ( Standard_Integer k = 0; k < 4; k++ )
        TEST_VERIFY( aQuadNodeIds[k] == QUADRANGLES[aQuadIndex][k], DescriptionFn(), funcID )

      aQuadIndex++;
    }
  }

  return outcome(DescriptionFn(), funcID).success();
}

#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
