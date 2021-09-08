//-----------------------------------------------------------------------------
// Created on: February 2012
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

#ifndef ActData_Utils_HeaderFile
#define ActData_Utils_HeaderFile

// Active Data includes
#include <ActData_Common.h>
#include <ActData_MeshAttr.h>

// Active Data (auxiliary) layer includes
#include <ActAux_TimeStamp.h>

// Active Data (API) includes
#include <ActAPI_INode.h>

// OCCT includes
#include <TColStd_HPackedMapOfInteger.hxx>
#include <TDataStd_ExtStringArray.hxx>
#include <TDataStd_IntegerArray.hxx>
#include <TDataStd_RealArray.hxx>
#include <TDataStd_ReferenceList.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataXtd_Triangulation.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelList.hxx>
#include <TopoDS_Shape.hxx>

#define REGISTER_PARAMETER_COMMON(Class, Id, IsExpressible) \
  this->registerParameter(Id, ActData_##Class##Parameter::Instance(), IsExpressible)

#define REGISTER_PARAMETER(Class, Id) \
  REGISTER_PARAMETER_COMMON(Class, Id, Standard_False)

#define REGISTER_PARAMETER_EXPR(Class, Id) \
  REGISTER_PARAMETER_COMMON(Class, Id, Standard_True)

#define REGISTER_PARTITION(Class, Id) \
  this->registerPartition( Id, Class::Instance() )

#define REGISTER_TREE_FUNCTION(Class) \
  this->registerTreeFunction( Class::Instance() )

//! \ingroup AD_DF
//!
//! Low-level utilities facilitating atomic operations on OCAF document.
class ActData_Utils
{
// Common methods:
public:

  ActData_EXPORT static TCollection_AsciiString
    GetEntry(const TDF_Label& theLabel);

  ActData_EXPORT static void
    GetEntry(const TDF_Label&         theLabel,
             TCollection_AsciiString& theEntry);

  ActData_EXPORT static ActAPI_ParameterGID
    ConvertToGID(const TDF_Label&       theLabel,
                 const Standard_Boolean isInternal);

  ActData_EXPORT static Standard_Boolean
    CheckLabelAttr(const TDF_Label&       theLabel,
                   const Standard_Integer theLabTag,
                   const Standard_GUID&   theAttrGUID);

  ActData_EXPORT static void
    RemoveWithReferences(const TDF_Label&       theLabel,
                         const Standard_Boolean doAffectChildren = Standard_True);

  ActData_EXPORT static Standard_Boolean
    ReplaceEvaluationString(const Handle(ActAPI_IUserParameter)& theParam,
                            const TCollection_AsciiString&       theWhat,
                            const TCollection_AsciiString&       theWith,
                            const Standard_Boolean               isCompleteErase);

// Timestamp:
public:

  ActData_EXPORT static void
    SetTimeStampValue(const TDF_Label&       theLab,
                      const Standard_Integer theSubTag);

  ActData_EXPORT static Handle(ActAux_TimeStamp)
    GetTimeStampValue(const TDF_Label&       theLab,
                      const Standard_Integer theSubTag);

// Integer:
public:

  ActData_EXPORT static void
    SetIntegerValue(const TDF_Label&       theLab,
                    const Standard_Integer theSubTag,
                    const Standard_Integer theValue);

  ActData_EXPORT static Standard_Boolean
    GetIntegerValue(const TDF_Label&       theLab,
                    const Standard_Integer theSubTag,
                    Standard_Integer&      theValue);

// Real:
public:

  ActData_EXPORT static void
    SetRealValue(const TDF_Label&       theLab,
                 const Standard_Integer theSubTag,
                 const Standard_Real    theValue);

  ActData_EXPORT static Standard_Boolean
    GetRealValue(const TDF_Label&       theLab,
                 const Standard_Integer theSubTag,
                 Standard_Real&         theValue);

// String:
public:

  ActData_EXPORT static void
    SetAsciiStringValue(const TDF_Label&               theLab,
                        const Standard_Integer         theSubTag,
                        const TCollection_AsciiString& theString);

  ActData_EXPORT static TCollection_AsciiString
    GetAsciiStringValue(const TDF_Label&       theLab,
                        const Standard_Integer theSubTag);

  ActData_EXPORT static void
    SetExtStringValue(const TDF_Label&                  theLab,
                      const Standard_Integer            theSubTag,
                      const TCollection_ExtendedString& theString);

  ActData_EXPORT static TCollection_ExtendedString
    GetExtStringValue(const TDF_Label&       theLab,
                      const Standard_Integer theSubTag);

// Shape:
public:

  ActData_EXPORT static void
    SetShapeValue(const TDF_Label&       theLab,
                  const Standard_Integer theSubTag,
                  const TopoDS_Shape&    theShape);

  ActData_EXPORT static TopoDS_Shape
    GetShapeValue(const TDF_Label&       theLab,
                  const Standard_Integer theSubTag);

// Mesh:
public:

  ActData_EXPORT static Handle(ActData_MeshAttr)
    AccessMeshAttr(const TDF_Label&       theLab,
                   const Standard_Integer theSubTag);

  ActData_EXPORT static void
    SetMesh(const TDF_Label&            theLab,
            const Standard_Integer      theSubTag,
            const Handle(ActData_Mesh)& theMesh);

  ActData_EXPORT static Handle(ActData_Mesh)
    GetMesh(const TDF_Label&       theLab,
            const Standard_Integer theSubTag);

  ActData_EXPORT static Handle(TDataXtd_Triangulation)
    AccessTriangulationAttr(const TDF_Label&       theLab,
                            const Standard_Integer theSubTag);

  ActData_EXPORT static void
    SetTriangulation(const TDF_Label&                  theLab,
                     const Standard_Integer            theSubTag,
                     const Handle(Poly_Triangulation)& theTriangulation);

  ActData_EXPORT static Handle(Poly_Triangulation)
    GetTriangulation(const TDF_Label&       theLab,
                     const Standard_Integer theSubTag);

// Bit mask:
public:

  ActData_EXPORT static void
    SetIntPackedMap(const TDF_Label&                           theLab,
                    const Standard_Integer                     theSubTag,
                    const Handle(TColStd_HPackedMapOfInteger)& theHMap);

  ActData_EXPORT static Standard_Boolean
    AddIntPackedMapValue(const TDF_Label&       theLab,
                         const Standard_Integer theSubTag,
                         const Standard_Integer theValue);

  ActData_EXPORT static Standard_Boolean
    RemoveIntPackedMapValue(const TDF_Label&       theLab,
                            const Standard_Integer theSubTag,
                            const Standard_Integer theValue);

  ActData_EXPORT static Standard_Boolean
    HasIntPackedMapValue(const TDF_Label&       theLab,
                         const Standard_Integer theSubTag,
                         const Standard_Integer theValue);

  ActData_EXPORT static Handle(TColStd_HPackedMapOfInteger)
    GetIntPackedMap(const TDF_Label&       theLab,
                    const Standard_Integer theSubTag);

// Integer array:
public:

  ActData_EXPORT static void
    InitIntegerArray(const TDF_Label&         theLab,
                     const Standard_Integer   theSubTag,
                     const Handle(HIntArray)& theArray);

  ActData_EXPORT static void
    InitIntegerArray(const TDF_Label&          theLab,
                     const Standard_Integer    theSubTag,
                     const Handle(HIntMatrix)& theMx);

  ActData_EXPORT static Handle(HIntMatrix)
    DispatchIntegerMatrix(const Handle(TDataStd_IntegerArray)& theArrayAttr,
                          const Standard_Integer               theNbRows,
                          const Standard_Integer               theNbCols);

  ActData_EXPORT static void
    SetIntegerMatrixElem(const Handle(TDataStd_IntegerArray)& theArrayAttr,
                         const Standard_Integer               theRowIndex,
                         const Standard_Integer               theColIndex,
                         const Standard_Integer               theNbCols,
                         const Standard_Integer               theValue);

  ActData_EXPORT static Standard_Integer
    GetIntegerMatrixElem(const Handle(TDataStd_IntegerArray)& theArrayAttr,
                         const Standard_Integer               theRowIndex,
                         const Standard_Integer               theColIndex,
                         const Standard_Integer               theNbCols);

  ActData_EXPORT static Handle(HIntArray)
    GetIntegerArray(const TDF_Label&       theLab,
                    const Standard_Integer theSubTag);

  ActData_EXPORT static void
    BackupIntegerArray(const TDF_Label&       theLab,
                       const Standard_Integer theSubTag);

  ActData_EXPORT static void
    SetIntegerArrayElem(const TDF_Label&       theLab,
                        const Standard_Integer theSubTag,
                        const Standard_Integer theIndex,
                        const Standard_Integer theValue);

  ActData_EXPORT static Standard_Integer
    GetIntegerArrayElem(const TDF_Label&       theLab,
                        const Standard_Integer theSubTag,
                        const Standard_Integer theIndex);

// Real array:
public:

  ActData_EXPORT static void
    InitRealArray(const TDF_Label&          theLab,
                  const Standard_Integer    theSubTag,
                  const Handle(HRealArray)& theArray);

  ActData_EXPORT static void
    InitRealArray(const TDF_Label&           theLab,
                  const Standard_Integer     theSubTag,
                  const Handle(HRealMatrix)& theMx);

  ActData_EXPORT static void
    InitRealArray(const TDF_Label&             theLab,
                  const Standard_Integer       theSubTag,
                  const Handle(HComplexArray)& theArray,
                  const Standard_Boolean       isReal = Standard_True);

  ActData_EXPORT static void
    InitRealArray(const TDF_Label&              theLab,
                  const Standard_Integer        theSubTag,
                  const Handle(HComplexMatrix)& theMx,
                  const Standard_Boolean        isReal = Standard_True);

  ActData_EXPORT static Handle(HRealMatrix)
    DispatchRealMatrix(const Handle(TDataStd_RealArray)& theArrayAttr,
                       const Standard_Integer            theNbRows,
                       const Standard_Integer            theNbCols);

  ActData_EXPORT static Handle(HComplexMatrix)
    DispatchRealMatrices(const Handle(TDataStd_RealArray)& theArrayRealAttr,
                         const Handle(TDataStd_RealArray)& theArrayImagAttr,
                         const Standard_Integer            theNbRows,
                         const Standard_Integer            theNbCols);

  ActData_EXPORT static void
    SetRealMatrixElem(const Handle(TDataStd_RealArray)& theArrayAttr,
                      const Standard_Integer            theRowIndex,
                      const Standard_Integer            theColIndex,
                      const Standard_Integer            theNbCols,
                      const Standard_Real               theValue);

  ActData_EXPORT static Standard_Real
    GetRealMatrixElem(const Handle(TDataStd_RealArray)& theArrayAttr,
                      const Standard_Integer            theRowIndex,
                      const Standard_Integer            theColIndex,
                      const Standard_Integer            theNbCols);

  ActData_EXPORT static Handle(HRealArray)
    GetRealArray(const TDF_Label&       theLab,
                 const Standard_Integer theSubTag);

  ActData_EXPORT static void
    BackupRealArray(const TDF_Label&       theLab,
                    const Standard_Integer theSubTag);

  ActData_EXPORT static void
    SetRealArrayElem(const TDF_Label&       theLab,
                     const Standard_Integer theSubTag,
                     const Standard_Integer theIndex,
                     const Standard_Real    theValue);

  ActData_EXPORT static Standard_Real
    GetRealArrayElem(const TDF_Label&       theLab,
                     const Standard_Integer theSubTag,
                     const Standard_Integer theIndex);

// Boolean array:
public:

  ActData_EXPORT static void
    InitBooleanArray(const TDF_Label&          theLab,
                     const Standard_Integer    theSubTag,
                     const Handle(HBoolArray)& theArray);

  ActData_EXPORT static Handle(HBoolArray)
    GetBooleanArray(const TDF_Label&       theLab,
                    const Standard_Integer theSubTag);

  ActData_EXPORT static void
    BackupBooleanArray(const TDF_Label&       theLab,
                       const Standard_Integer theSubTag);

  ActData_EXPORT static void
    SetBooleanArrayElem(const TDF_Label&       theLab,
                        const Standard_Integer theSubTag,
                        const Standard_Integer theIndex,
                        const Standard_Boolean theValue);

  ActData_EXPORT static Standard_Boolean
    GetBooleanArrayElem(const TDF_Label&       theLab,
                        const Standard_Integer theSubTag,
                        const Standard_Integer theIndex);

// String array:
public:

  ActData_EXPORT static void
    InitStringArray(const TDF_Label&            theLab,
                    const Standard_Integer      theSubTag,
                    const Handle(HStringArray)& theArray);

  ActData_EXPORT static void
    InitStringArray(const TDF_Label&             theLab,
                    const Standard_Integer       theSubTag,
                    const Handle(HStringMatrix)& theArray);

  ActData_EXPORT static Handle(HStringMatrix)
    DispatchStringMatrix(const Handle(TDataStd_ExtStringArray)& theArrayAttr,
                         const Standard_Integer                 theNbRows,
                         const Standard_Integer                 theNbCols);

  ActData_EXPORT static void
    SetStringMatrixElem(const Handle(TDataStd_ExtStringArray)& theArrayAttr,
                        const Standard_Integer                 theRowIndex,
                        const Standard_Integer                 theColIndex,
                        const Standard_Integer                 theNbCols,
                        const TCollection_ExtendedString&      theValue);

  ActData_EXPORT static TCollection_ExtendedString
    GetStringMatrixElem(const Handle(TDataStd_ExtStringArray)& theArrayAttr,
                        const Standard_Integer                 theRowIndex,
                        const Standard_Integer                 theColIndex,
                        const Standard_Integer                 theNbCols);

  ActData_EXPORT static Handle(HStringArray)
    GetStringArray(const TDF_Label&       theLab,
                   const Standard_Integer theSubTag);

  ActData_EXPORT static void
    BackupStringArray(const TDF_Label&       theLab,
                      const Standard_Integer theSubTag);

  ActData_EXPORT static void
    SetStringArrayElem(const TDF_Label&                  theLab,
                       const Standard_Integer            theSubTag,
                       const Standard_Integer            theIndex,
                       const TCollection_ExtendedString& theValue);

  ActData_EXPORT static TCollection_ExtendedString
    GetStringArrayElem(const TDF_Label&       theLab,
                       const Standard_Integer theSubTag,
                       const Standard_Integer theIndex);

// Tree Function:
public:

  ActData_EXPORT static Handle(ActAPI_HParameterList)
    PendingFunctionParams(const Handle(ActAPI_INode)& theNode);

// Reference:
public:

  ActData_EXPORT static Standard_Integer
    HasTarget(const TDF_LabelList& theList,
              const TDF_Label&     theTargetLab);

  ActData_EXPORT static Handle(ActAPI_HDataCursorList)
    ConvertToCursors(const TDF_LabelList& theList);

  ActData_EXPORT static Handle(TDataStd_ReferenceList)
    GetReferenceList(const TDF_Label&       theLab,
                     const Standard_Integer theTag);

  ActData_EXPORT static void
    PrependReference(const TDF_Label&       theLab,
                     const Standard_Integer theTag,
                     const TDF_Label&       theTargetLab);

  ActData_EXPORT static void
    AppendReference(const TDF_Label&       theLab,
                    const Standard_Integer theTag,
                    const TDF_Label&       theTargetLab);

// Tree Node:
public:

  ActData_EXPORT static Handle(TDataStd_TreeNode)
    AccessTreeNode(const TDF_Label&       theLab,
                   const Standard_Boolean toCreate);

  ActData_EXPORT static void
    AppendChild(const Handle(TDataStd_TreeNode)& theParent,
                const Handle(TDataStd_TreeNode)& theChild);

  ActData_EXPORT static Standard_Boolean
    RemoveChild(const Handle(TDataStd_TreeNode)& theParent,
                const Handle(TDataStd_TreeNode)& theChild);

public:

  ActData_EXPORT static TDF_Label
    ChooseLabelByTag(const TDF_Label&       theLab,
                     const Standard_Integer theTag,
                     const Standard_Boolean toCreate = Standard_False);

};

//! \ingroup AD_DF
//!
//! Auxiliary routines for manipulation with ASCII strings playing as a basis
//! for regexp-like processing on Parameters' evaluation strings.
namespace ActData_StringAux
{
  ActData_EXPORT Standard_Boolean
    IsNotSpecial(const Standard_Character Char);

  ActData_EXPORT Standard_Boolean
    IsLexeme(const TCollection_AsciiString& Source,
             const TCollection_AsciiString& Piece,
             Standard_Integer&              Start,
             Standard_Integer&              End);

  ActData_EXPORT Standard_Boolean
    Replace(TCollection_AsciiString&       String,
            const TCollection_AsciiString& Target,
            const Standard_Integer         Start,
            const Standard_Integer         End);

  ActData_EXPORT void
    ReplaceRecursive(TCollection_AsciiString&       Source,
                     const TCollection_AsciiString& What,
                     const TCollection_AsciiString& With);
}

#endif
