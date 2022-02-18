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

#ifndef ActTest_MeshAttr_HeaderFile
#define ActTest_MeshAttr_HeaderFile

// Active Data unit tests
#include <ActTest_DataFramework.h>

// Active Data includes
#include <ActData_MeshAttr.h>

//! \ingroup AD_TEST
//!
//! Base class providing common functionality for unit tests on
//! Mesh Attribute.
class ActTest_MeshAttrBase : public ActTest_DataFramework
{
public:

  static Standard_Real    NODES[][3];       //!< Collection of nodal co-ordinates.
  static Standard_Integer TRIANGLES[][3];   //!< Collection of triangles.
  static Standard_Integer QUADRANGLES[][4]; //!< Collection of quadrangles.

  static Standard_Integer NB_NODES;       //!< Number of mesh nodes.
  static Standard_Integer NB_TRIANGLES;   //!< Number of triangle mesh elements.
  static Standard_Integer NB_QUADRANGLES; //!< Number of quadrangle mesh elements.

protected:

  //! Type definition for collection of mesh element IDs.
  typedef NCollection_Sequence<Standard_Integer> DatumIdList;

protected:

  static bool
    initializeMeshAttr(const Handle(TDocStd_Document)& doc,
                       TDF_Label& meshLab,
                       const Standard_Boolean isImplictTrans = Standard_True);

  static outcome
    populateMeshData(const Handle(TDocStd_Document)& doc,
                     TDF_Label& meshLab,
                     DatumIdList& NODE_IDS,
                     DatumIdList& TRIANGLE_IDS,
                     DatumIdList& QUADRANGLE_IDS,
                     const Standard_Boolean isImplictTrans = Standard_True);

  static outcome
    populateMeshNodes(TDF_Label& meshLab,
                      DatumIdList& NODE_IDS);

  static outcome
    populateMeshTriangles(TDF_Label& meshLab,
                          DatumIdList& TRIANGLE_IDS);

  static outcome
    populateMeshQuadrangles(TDF_Label& meshLab,
                            DatumIdList& QUADRANGLE_IDS);

};

//! \ingroup AD_TEST
//!
//! Test suite for Active Data.
//! This class performs unit testing of Mesh CAF Attribute class in its
//! transient use cases. This includes creation of a new CAF Label and
//! manipulation with the attached data in a single transaction.
class ActTest_MeshAttrBean : public ActTest_MeshAttrBase
{
public:

  //! Returns Test Case ID.
  //! \return ID of the Test Case.
  static int ID()
  {
    return CaseID_MeshAttrBean;
  }

  //! Returns filename for the description.
  //! \return filename for the description of the Test Case.
  static std::string DescriptionFn()
  {
    return "ActTest_MeshAttrBean";
  }

  //! Returns Test Case description directory.
  //! \return description directory for the Test Case.
  static std::string DescriptionDir()
  {
    return "Mesh";
  }

  //! Returns pointers to the Test Functions to launch.
  //! \param functions [out] output collection of pointers.
  static void Functions(asiTestFunctions& functions)
  {
    functions << &meshBeanTest;
  }

// Test functions:
private:

  static outcome meshBeanTest(const int funcID);

};

//! \ingroup AD_TEST
//!
//! Test suite for Active Data.
//! This class performs unit testing of Mesh CAF Attribute class in its
//! transactional perspective.
class ActTest_MeshAttrTransactional : public ActTest_MeshAttrBase
{
public:

  //! Returns Test Case ID.
  //! \return ID of the Test Case.
  static int ID()
  {
    return CaseID_MeshAttrTransactional;
  }

  //! Returns filename for the description.
  //! \return filename for the description of the Test Case.
  static std::string DescriptionFn()
  {
    return "ActTest_MeshAttrTransactional";
  }

  //! Returns Test Case description directory.
  //! \return description directory for the Test Case.
  static std::string DescriptionDir()
  {
    return "Mesh";
  }

  //! Returns pointers to the Test Functions to launch.
  //! \param functions [out] output collection of pointers.
  static void Functions(asiTestFunctions& functions)
  {
    functions << &meshTransUndoRedoTest1
              << &meshTransAbortTest1
              << &meshTransAbortTest2;
  }

// Test functions:
private:

  static outcome meshTransUndoRedoTest1 (const int funcID);
  static outcome meshTransAbortTest1    (const int funcID);
  static outcome meshTransAbortTest2    (const int funcID);

};

//! \ingroup AD_TEST
//!
//! Test suite for Active Data.
//! This class performs unit testing of Mesh CAF Attribute class in its
//! persistence perspective.
class ActTest_MeshAttrPersistent : public ActTest_MeshAttrBase
{
public:

  //! Returns Test Case ID.
  //! \return ID of the Test Case.
  static int ID()
  {
    return CaseID_MeshAttrPersistent;
  }

  //! Returns filename for the description.
  //! \return filename for the description of the Test Case.
  static std::string DescriptionFn()
  {
    return "ActTest_MeshAttrPersistent";
  }

  //! Returns Test Case description directory.
  //! \return description directory for the Test Case.
  static std::string DescriptionDir()
  {
    return "Mesh";
  }

  //! Returns pointers to the Test Functions to launch.
  //! \param functions [out] output collection of pointers.
  static void Functions(asiTestFunctions& functions)
  {
    functions << &meshSaveOpenTest;
  }

// Test functions:
private:

  static outcome meshSaveOpenTest(const int funcID);

};

#endif
