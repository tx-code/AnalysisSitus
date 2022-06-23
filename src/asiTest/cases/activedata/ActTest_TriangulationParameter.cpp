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
#include <ActTest_TriangulationParameter.h>

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------
// Business logic
//-----------------------------------------------------------------------------

//! Performs test on accessing value of TriangulationParameter.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_TriangulationParameter::accessValue(const int funcID)
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  TEST_PRINT_DECOR_L("Create new TRIANGULATION Parameter");

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  doc->NewCommand();
  Handle(ActData_TriangulationParameter)
    param = ActParamTool::AsTriangulation( createParameter(doc, Parameter_Triangulation) );
  doc->CommitCommand();

  TEST_VERIFY( param->IsWellFormed(), DescriptionFn(), funcID ) // Triangulation Node is well-formed even without initialization

  /* ===========================
   *  Initialize Mesh Parameter
   * =========================== */

  static Handle(Poly_Triangulation) TRIS = new Poly_Triangulation(3, 1, 0);
  TRIS->ChangeNode(1) = gp_Pnt(0.0, 0.0, 0.0);
  TRIS->ChangeNode(2) = gp_Pnt(1.0, 0.0, 0.0);
  TRIS->ChangeNode(3) = gp_Pnt(1.0, 1.0, 0.0);
  //
  TRIS->ChangeTriangle(1) = Poly_Triangle(1, 2, 3);

  doc->OpenCommand();
  param->SetTriangulation(TRIS);
  doc->CommitCommand();

  TEST_VERIFY( param->IsWellFormed(), DescriptionFn(), funcID )

  /* ==============================================
   *  Verify Mesh DS retrieved from Mesh Parameter
   * ============================================== */

  Handle(Poly_Triangulation) aMeshFrom = param->GetTriangulation();

  TEST_VERIFY( !aMeshFrom.IsNull(), DescriptionFn(), funcID)
  TEST_VERIFY( aMeshFrom->NbTriangles() == 1, DescriptionFn(), funcID )
  TEST_VERIFY( aMeshFrom->NbNodes() == 3, DescriptionFn(), funcID )

  const gp_Pnt& aNode1 = aMeshFrom->Node(1);
  const gp_Pnt& aNode2 = aMeshFrom->Node(2);
  const gp_Pnt& aNode3 = aMeshFrom->Node(3);

  TEST_VERIFY( aNode1.X() == 0.0, DescriptionFn(), funcID )
  TEST_VERIFY( aNode1.Y() == 0.0, DescriptionFn(), funcID )
  TEST_VERIFY( aNode1.Z() == 0.0, DescriptionFn(), funcID )
  //
  TEST_VERIFY( aNode2.X() == 1.0, DescriptionFn(), funcID )
  TEST_VERIFY( aNode2.Y() == 0.0, DescriptionFn(), funcID )
  TEST_VERIFY( aNode2.Z() == 0.0, DescriptionFn(), funcID )
  //
  TEST_VERIFY( aNode3.X() == 1.0, DescriptionFn(), funcID )
  TEST_VERIFY( aNode3.Y() == 1.0, DescriptionFn(), funcID )
  TEST_VERIFY( aNode3.Z() == 0.0, DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
