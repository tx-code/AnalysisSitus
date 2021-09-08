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

// Own include
#include <ActData_Utils.h>

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_LogBook.h>
#include <ActData_ParameterFactory.h>

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TColStd_HArray1OfByte.hxx>
#include <TDataStd_AsciiString.hxx>
#include <TDataStd_BooleanArray.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_IntPackedMap.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_Real.hxx>
#include <TDF_Tool.hxx>
#include <TFunction_Scope.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_NamedShape.hxx>
#include <TNaming_UsedShapes.hxx>

#define RET_DISPATCHED_ARRAY2MX_BEGIN(MxType, NbRows, NbCols, DataSource) \
  Handle(MxType) aResult = new MxType(0, NbRows - 1, 0, NbCols - 1); \
  Standard_Integer aRowIndx = -1, aColIndx = -1; \
  for ( Standard_Integer i = 0; i < DataSource->Length(); ++i ) \
  { \
    if ( i % NbCols == 0 ) \
    { \
      aRowIndx++; \
      aColIndx = 0; \
    }

#define RET_DISPATCHED_ARRAY2MX_END_SIMPLE(DataSource, NbRows, NbCols) \
    if ( aRowIndx >= NbRows || aColIndx >= NbCols ) \
      break; \
    aResult->SetValue( aRowIndx, aColIndx++, DataSource->Value(i) ); \
  } \
  return aResult;

#define RET_DISPATCHED_ARRAY2MX_END_COMPLEX(ArrRealAttr, ArrImagAttr, NbRows, NbCols) \
    if ( aRowIndx >= NbRows || aColIndx >= NbCols ) \
      break; \
    ComplexNumber aVal( ArrRealAttr->Value(i), \
                         ArrImagAttr->Value(i) ); \
    aResult->SetValue(aRowIndx, aColIndx++, aVal); \
  } \
  return aResult;

#define RET_DISPATCHED_ARRAY2MX(MxType, NbRows, NbCols, DataSource) \
  RET_DISPATCHED_ARRAY2MX_BEGIN(MxType, NbRows, NbCols, DataSource) \
  RET_DISPATCHED_ARRAY2MX_END_SIMPLE(DataSource, NbRows, NbCols)

#define RET_DISPATCHED_ARRAYS2COMPLEXMX(MxType, NbRows, NbCols, DataSourceReal, DataSourceImag) \
  RET_DISPATCHED_ARRAY2MX_BEGIN(MxType, NbRows, NbCols, DataSourceReal) \
  RET_DISPATCHED_ARRAY2MX_END_COMPLEX(DataSourceReal, DataSourceImag, NbRows, NbCols)

#define INIT_ARRAY_FROM_ARRAY_BEGIN(TypeArray, Lab, Array) \
  Handle(TypeArray) aDataAttr; \
  if ( Lab.FindAttribute(TypeArray::GetID(), aDataAttr) ) \
  { \
    Lab.ForgetAttribute( TypeArray::GetID() ); \
  } \
  aDataAttr = new TypeArray(); \
  aDataAttr->SetID( TypeArray::GetID() ); \
  if ( !Array.IsNull() ) \
    aDataAttr->Init( 0, Array->Upper() ); \
  Lab.AddAttribute(aDataAttr); \
  Standard_Integer anIndex = 0; \
  if ( !Array.IsNull() ) \
    for ( Standard_Integer i = Array->Lower(); i <= Array->Upper(); ++i ) \
    {

#define INIT_ARRAY_FROM_ARRAY_END_SIMPLE(Array) \
      aDataAttr->SetValue( anIndex++, Array->Value(i) ); \
    }

#define INIT_ARRAY_FROM_ARRAY_END_COMPLEX(Array, IsRealPart) \
    aDataAttr->SetValue( anIndex++, (IsRealPart ? Array->Value(i).Re : Array->Value(i).Im) ); \
  }

#define INIT_ARRAY_FROM_ARRAY(TypeArray, Lab, Array) \
  INIT_ARRAY_FROM_ARRAY_BEGIN(TypeArray, Lab, Array) \
  INIT_ARRAY_FROM_ARRAY_END_SIMPLE(Array)

#define INIT_ARRAY_FROM_ARRAY_COMPLEX(TypeArray, Lab, Array, IsRealPart) \
  INIT_ARRAY_FROM_ARRAY_BEGIN(TypeArray, Lab, Array) \
  INIT_ARRAY_FROM_ARRAY_END_COMPLEX(Array, IsRealPart)

#define INIT_ARRAY_FROM_MX_BEGIN(TypeArray, Lab, Mx) \
  Handle(TypeArray) aDataAttr; \
  if ( Lab.FindAttribute(TypeArray::GetID(), aDataAttr) ) \
  { \
    Lab.ForgetAttribute( TypeArray::GetID() ); \
  } \
  aDataAttr = new TypeArray(); \
  aDataAttr->SetID( TypeArray::GetID() ); \
  aDataAttr->Init( 0, Mx->RowLength() * Mx->ColLength() - 1 ); \
  Lab.AddAttribute(aDataAttr); \
  Standard_Integer aFlatIndx = 0; \
  for ( Standard_Integer i = Mx->LowerRow(); i <= Mx->UpperRow(); ++i ) \
    for ( Standard_Integer j = Mx->LowerCol(); j <= Mx->UpperCol(); ++j ) \
    {

#define INIT_ARRAY_FROM_MX_END_SIMPLE(Mx) \
    aDataAttr->SetValue( aFlatIndx++, Mx->Value(i, j) ); \
  }

#define INIT_ARRAY_FROM_MX_END_COMPLEX(Mx, IsRealPart) \
    aDataAttr->SetValue( aFlatIndx++, (IsRealPart ? Mx->Value(i, j).Re : Mx->Value(i, j).Im) ); \
  }

#define INIT_ARRAY_FROM_MX(TypeArray, Lab, Mx) \
  INIT_ARRAY_FROM_MX_BEGIN(TypeArray, Lab, Mx) \
  INIT_ARRAY_FROM_MX_END_SIMPLE(Mx)

#define INIT_ARRAY_FROM_MX_COMPLEX(TypeArray, Lab, Mx, IsRealPart) \
  INIT_ARRAY_FROM_MX_BEGIN(TypeArray, Lab, Mx) \
  INIT_ARRAY_FROM_MX_END_COMPLEX(Mx, IsRealPart)

#define FLAT_MX_INDEX(RowIndex, ColIndex, NbCols) \
  RowIndex * NbCols + ColIndex;

//-----------------------------------------------------------------------------
// Common functionality for CAF-based procedures
//-----------------------------------------------------------------------------

TCollection_AsciiString ActData_Utils::GetEntry(const TDF_Label& theLabel)
{
  TCollection_AsciiString aEntry;
  TDF_Tool::Entry(theLabel, aEntry);
  return aEntry;
}

void ActData_Utils::GetEntry(const TDF_Label&         theLabel,
                             TCollection_AsciiString& theEntry)
{
  TDF_Tool::Entry(theLabel, theEntry);
}

ActAPI_ParameterGID ActData_Utils::ConvertToGID(const TDF_Label&       theLabel,
                                                const Standard_Boolean isInternal)
{
  Standard_Integer PID = theLabel.Tag();
  TDF_Label NodeRoot;

  if ( isInternal )
    NodeRoot = theLabel.Father()  // Sub-group (EVALUATORS)
                       .Father()  // Group (META)
                       .Father(); // Root Label for Node
  else
    NodeRoot = theLabel.Father()  // Group (USER)
                       .Father(); // Root Label for Node

  return ActAPI_ParameterGID( GetEntry(NodeRoot), PID );
}

Standard_Boolean
  ActData_Utils::CheckLabelAttr(const TDF_Label&       theLabel,
                                const Standard_Integer theLabTag,
                                const Standard_GUID&   theAttrGUID)
{
  if ( theLabel.IsNull() )
    return Standard_False;

  TDF_Label aLabelToCheck;

  if ( theLabTag == -1 )
    aLabelToCheck = theLabel;
  else
  {
    aLabelToCheck = ChooseLabelByTag(theLabel, theLabTag, Standard_False);
    if ( aLabelToCheck.IsNull() )
      return Standard_False;
  }

  if ( theAttrGUID != Standard_GUID() ) // Empty GUID not taken into account
  {
    Handle(TDF_Attribute) anAttr;
    if ( !aLabelToCheck.FindAttribute(theAttrGUID, anAttr) )
      return Standard_False;
  }

  return Standard_True;
}

void ActData_Utils::RemoveWithReferences(const TDF_Label&       theLabel,
                                         const Standard_Boolean doAffectChildren)
{
  if ( theLabel.IsNull() )
    return;

  // Remove TFunction records from global scope
  Handle(TFunction_Scope) aFuncScope = TFunction_Scope::Set(theLabel);
  if ( aFuncScope->HasFunction(theLabel) )
    aFuncScope->RemoveFunction(theLabel);

  // Clean up records from LogBook
  TDF_Label aLogBookSection =
    theLabel.Root().FindChild(ActData_BaseModel::StructureTag_LogBook);
  ActData_LogBook(aLogBookSection).ClearReferencesFor(theLabel);

  // Clean up direct attributes
  theLabel.ForgetAllAttributes(doAffectChildren);
}

Standard_Boolean
  ActData_Utils::ReplaceEvaluationString(const Handle(ActAPI_IUserParameter)& theParam,
                                         const TCollection_AsciiString&       theWhat,
                                         const TCollection_AsciiString&       theWith,
                                         const Standard_Boolean               isCompleteErase)
{
  TCollection_AsciiString anOldEvalStr = theParam->GetEvalString();
  TCollection_AsciiString aEvalStr = theParam->GetEvalString();
  Standard_Integer aStart = -1, aEnd = -1;
      
  if ( isCompleteErase && ActData_StringAux::IsLexeme(aEvalStr, theWhat, aStart, aEnd) )
    theParam->SetEvalString(TCollection_AsciiString(), MT_Touched);
  else {
    ActData_StringAux::ReplaceRecursive(aEvalStr, theWhat, theWith);
    theParam->SetEvalString(aEvalStr, MT_Touched);
  }

  return !anOldEvalStr.IsEqual(aEvalStr);
}

//-----------------------------------------------------------------------------
// TIMESTAMP Values
//-----------------------------------------------------------------------------

void ActData_Utils::SetTimeStampValue(const TDF_Label&       theLab,
                                      const Standard_Integer theSubTag)
{
  Handle(ActAux_TimeStamp) aTS = ActAux_TimeStampTool::Generate();
  Handle(HIntArray) aTSChunked = ActAux_TimeStampTool::AsChunked(aTS);
  InitIntegerArray(theLab, theSubTag, aTSChunked);
}

Handle(ActAux_TimeStamp)
  ActData_Utils::GetTimeStampValue(const TDF_Label&       theLab,
                                   const Standard_Integer theSubTag)
{
  Handle(HIntArray) aTSChunked = GetIntegerArray(theLab, theSubTag);
  Handle(ActAux_TimeStamp) aResult = ActAux_TimeStampTool::FromChunked(aTSChunked);
  return aResult;
}

//-----------------------------------------------------------------------------
// INTEGER Values
//-----------------------------------------------------------------------------

void ActData_Utils::SetIntegerValue(const TDF_Label&       theLab,
                                    const Standard_Integer theSubTag,
                                    const Standard_Integer theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);
  TDataStd_Integer::Set(aDataLab, theValue);
}

Standard_Boolean
  ActData_Utils::GetIntegerValue(const TDF_Label&       theLab,
                                 const Standard_Integer theSubTag,
                                 Standard_Integer&      theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  if ( aDataLab.IsNull() )
    return Standard_False;

  Handle(TDataStd_Integer) aValueAttr;
  if ( !aDataLab.FindAttribute(TDataStd_Integer::GetID(), aValueAttr) )
    return Standard_False;

  theValue = aValueAttr->Get();
  return Standard_True;
}

//-----------------------------------------------------------------------------
// REAL Values
//-----------------------------------------------------------------------------

void ActData_Utils::SetRealValue(const TDF_Label&       theLab,
                                 const Standard_Integer theSubTag,
                                 const Standard_Real    theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);
  TDataStd_Real::Set(aDataLab, theValue);
}

Standard_Boolean
  ActData_Utils::GetRealValue(const TDF_Label&       theLab,
                              const Standard_Integer theSubTag,
                              Standard_Real&         theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  if ( aDataLab.IsNull() )
    return Standard_False;

  Handle(TDataStd_Real) aValueAttr;
  if ( !aDataLab.FindAttribute(TDataStd_Real::GetID(), aValueAttr) )
    return Standard_False;

  theValue = aValueAttr->Get();
  return Standard_True;
}

//-----------------------------------------------------------------------------
// STRING Values
//-----------------------------------------------------------------------------

void ActData_Utils::SetAsciiStringValue(const TDF_Label&               theLab,
                                        const Standard_Integer         theSubTag,
                                        const TCollection_AsciiString& theString)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);
  TDataStd_AsciiString::Set(aDataLab, theString);
}

TCollection_AsciiString
  ActData_Utils::GetAsciiStringValue(const TDF_Label&       theLab,
                                     const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  if ( aDataLab.IsNull() )
    return TCollection_AsciiString();

  Handle(TDataStd_AsciiString) aValueAttr;
  if ( !aDataLab.FindAttribute(TDataStd_AsciiString::GetID(), aValueAttr) )
    return TCollection_AsciiString();

  return aValueAttr->Get();
}

void ActData_Utils::SetExtStringValue(const TDF_Label&                  theLab,
                                      const Standard_Integer            theSubTag,
                                      const TCollection_ExtendedString& theString)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);
  TDataStd_Name::Set(aDataLab, theString);
}

TCollection_ExtendedString
  ActData_Utils::GetExtStringValue(const TDF_Label&       theLab,
                                   const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  if ( aDataLab.IsNull() )
    return TCollection_ExtendedString();

  Handle(TDataStd_Name) aValueAttr;
  if ( !aDataLab.FindAttribute(TDataStd_Name::GetID(), aValueAttr) )
    return TCollection_ExtendedString();

  return aValueAttr->Get();
}

//-----------------------------------------------------------------------------
// SHAPE Values
//-----------------------------------------------------------------------------

void ActData_Utils::SetShapeValue(const TDF_Label&       theLab,
                                  const Standard_Integer theSubTag,
                                  const TopoDS_Shape&    theShape)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);

  if ( theShape.IsNull() )
  {
    TopoDS_Shape prevShape = GetShapeValue(theLab, theSubTag);

    if ( !prevShape.IsNull() )
    {
      // Remove from the global UsedShapes attribute.
      const TDF_Label& root = aDataLab.Root();
      Handle(TNaming_UsedShapes) usedShapesAttr;
      //
      if ( root.FindAttribute(TNaming_UsedShapes::GetID(), usedShapesAttr) )
        usedShapesAttr->Map().UnBind(prevShape);
    }
  }

  TNaming_Builder aNBuilder(aDataLab);
  aNBuilder.Generated(theShape);
}

TopoDS_Shape ActData_Utils::GetShapeValue(const TDF_Label&       theLab,
                                          const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);
  //
  if ( aDataLab.IsNull() )
    return TopoDS_Shape();

  Handle(TNaming_NamedShape) aShapeAttr;
  aDataLab.FindAttribute(TNaming_NamedShape::GetID(), aShapeAttr);
  //
  if ( aShapeAttr.IsNull() )
    return TopoDS_Shape();

  return aShapeAttr->Get();
}

//-----------------------------------------------------------------------------
// MESH data sets
//-----------------------------------------------------------------------------

Handle(ActData_MeshAttr)
  ActData_Utils::AccessMeshAttr(const TDF_Label&       theLab,
                                const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);
  return ActData_MeshAttr::Set(aDataLab);
}

void ActData_Utils::SetMesh(const TDF_Label&            theLab,
                            const Standard_Integer      theSubTag,
                            const Handle(ActData_Mesh)& theMesh)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);
  Handle(ActData_MeshAttr) aMeshAttr = ActData_MeshAttr::Set(aDataLab);
  aMeshAttr->SetMesh(theMesh);
}

Handle(ActData_Mesh)
  ActData_Utils::GetMesh(const TDF_Label&       theLab,
                         const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  if ( aDataLab.IsNull() )
    return NULL;

  Handle(ActData_MeshAttr) aMeshAttr;
  if ( !aDataLab.FindAttribute(ActData_MeshAttr::GUID(), aMeshAttr) )
    return NULL;

  return aMeshAttr->GetMesh();
}

Handle(TDataXtd_Triangulation)
  ActData_Utils::AccessTriangulationAttr(const TDF_Label&       theLab,
                                         const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);
  return TDataXtd_Triangulation::Set(aDataLab);
}

void
  ActData_Utils::SetTriangulation(const TDF_Label&                  theLab,
                                  const Standard_Integer            theSubTag,
                                  const Handle(Poly_Triangulation)& theTriangulation)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);
  Handle(TDataXtd_Triangulation) anAttr = TDataXtd_Triangulation::Set(aDataLab);
  anAttr->Set(theTriangulation);
}

Handle(Poly_Triangulation)
  ActData_Utils::GetTriangulation(const TDF_Label&       theLab,
                                  const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  if ( aDataLab.IsNull() )
    return NULL;

  Handle(TDataXtd_Triangulation) anAttr;
  if ( !aDataLab.FindAttribute(TDataXtd_Triangulation::GetID(), anAttr) )
    return NULL;

  return anAttr->Get();
}

//-----------------------------------------------------------------------------
// PACKED MAP OF INTEGER
//-----------------------------------------------------------------------------

void
  ActData_Utils::SetIntPackedMap(const TDF_Label&                           theLab,
                                 const Standard_Integer                     theSubTag,
                                 const Handle(TColStd_HPackedMapOfInteger)& theHMap)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);
  Handle(TDataStd_IntPackedMap) aMapAttr = TDataStd_IntPackedMap::Set(aDataLab);
  aMapAttr->ChangeMap(theHMap);
}

Handle(TColStd_HPackedMapOfInteger)
  ActData_Utils::GetIntPackedMap(const TDF_Label&       theLab,
                                 const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  if ( aDataLab.IsNull() )
    return NULL;

  Handle(TDataStd_IntPackedMap) aMapAttr;
  if ( !aDataLab.FindAttribute(TDataStd_IntPackedMap::GetID(), aMapAttr) )
    return NULL;

  return aMapAttr->GetHMap();
}

Standard_Boolean
  ActData_Utils::AddIntPackedMapValue(const TDF_Label&       theLab,
                                      const Standard_Integer theSubTag,
                                      const Standard_Integer theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  if ( aDataLab.IsNull() )
    Standard_ProgramError::Raise("Data Label is NULL");

  Handle(TDataStd_IntPackedMap) aMapAttr;
  if ( !aDataLab.FindAttribute(TDataStd_IntPackedMap::GetID(), aMapAttr) )
    Standard_ProgramError::Raise("Data Attribute NOT FOUND");

  return aMapAttr->Add(theValue);
}

Standard_Boolean
  ActData_Utils::RemoveIntPackedMapValue(const TDF_Label&       theLab,
                                         const Standard_Integer theSubTag,
                                         const Standard_Integer theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  if ( aDataLab.IsNull() )
    Standard_ProgramError::Raise("Data Label is NULL");

  Handle(TDataStd_IntPackedMap) aMapAttr;
  if ( !aDataLab.FindAttribute(TDataStd_IntPackedMap::GetID(), aMapAttr) )
    Standard_ProgramError::Raise("Data Attribute NOT FOUND");

  return aMapAttr->Remove(theValue);
}

Standard_Boolean
  ActData_Utils::HasIntPackedMapValue(const TDF_Label&       theLab,
                                      const Standard_Integer theSubTag,
                                      const Standard_Integer theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  if ( aDataLab.IsNull() )
    Standard_ProgramError::Raise("Data Label is NULL");

  Handle(TDataStd_IntPackedMap) aMapAttr;
  if ( !aDataLab.FindAttribute(TDataStd_IntPackedMap::GetID(), aMapAttr) )
    Standard_ProgramError::Raise("Data Attribute NOT FOUND");

  return aMapAttr->Contains(theValue);
}

//-----------------------------------------------------------------------------
// INTEGER Arrays
//-----------------------------------------------------------------------------

void ActData_Utils::InitIntegerArray(const TDF_Label&         theLab,
                                     const Standard_Integer   theSubTag,
                                     const Handle(HIntArray)& theArray)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);

  INIT_ARRAY_FROM_ARRAY(TDataStd_IntegerArray, aDataLab, theArray)
}

void ActData_Utils::InitIntegerArray(const TDF_Label&          theLab,
                                     const Standard_Integer    theSubTag,
                                     const Handle(HIntMatrix)& theMx)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);

  INIT_ARRAY_FROM_MX(TDataStd_IntegerArray, aDataLab, theMx)
}

Handle(HIntMatrix)
  ActData_Utils::DispatchIntegerMatrix(const Handle(TDataStd_IntegerArray)& theArrayAttr,
                                       const Standard_Integer               theNbRows,
                                       const Standard_Integer               theNbCols)
{
  RET_DISPATCHED_ARRAY2MX(HIntMatrix, theNbRows, theNbCols, theArrayAttr)
}

void
  ActData_Utils::SetIntegerMatrixElem(const Handle(TDataStd_IntegerArray)& theArrayAttr,
                                      const Standard_Integer               theRowIndex,
                                      const Standard_Integer               theColIndex,
                                      const Standard_Integer               theNbCols,
                                      const Standard_Integer               theValue)
{
  Standard_Integer aFlatIndex = FLAT_MX_INDEX(theRowIndex, theColIndex, theNbCols);
  theArrayAttr->SetValue(aFlatIndex, theValue);
}

Standard_Integer
  ActData_Utils::GetIntegerMatrixElem(const Handle(TDataStd_IntegerArray)& theArrayAttr,
                                      const Standard_Integer               theRowIndex,
                                      const Standard_Integer               theColIndex,
                                      const Standard_Integer               theNbCols)
{
  Standard_Integer aFlatIndex = FLAT_MX_INDEX(theRowIndex, theColIndex, theNbCols);
  return theArrayAttr->Value(aFlatIndex);
}

Handle(HIntArray)
  ActData_Utils::GetIntegerArray(const TDF_Label&       theLab,
                                 const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);
  if ( aDataLab.IsNull() )
    return NULL;

  Handle(TDataStd_IntegerArray) aDataAttr;
  if ( aDataLab.FindAttribute(TDataStd_IntegerArray::GetID(), aDataAttr) )
    return aDataAttr->Array();

  return NULL;
}

void ActData_Utils::BackupIntegerArray(const TDF_Label&       theLab,
                                       const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_IntegerArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_IntegerArray::GetID(), aDataAttr);

  aDataAttr->Backup();
}

void ActData_Utils::SetIntegerArrayElem(const TDF_Label&       theLab,
                                        const Standard_Integer theSubTag,
                                        const Standard_Integer theIndex,
                                        const Standard_Integer theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_IntegerArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_IntegerArray::GetID(), aDataAttr);
  aDataAttr->SetValue(theIndex, theValue);
}

Standard_Integer
  ActData_Utils::GetIntegerArrayElem(const TDF_Label&       theLab,
                                     const Standard_Integer theSubTag,
                                     const Standard_Integer theIndex)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_IntegerArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_IntegerArray::GetID(), aDataAttr);
  return aDataAttr->Value(theIndex);
}

//-----------------------------------------------------------------------------
// REAL Arrays
//-----------------------------------------------------------------------------

void ActData_Utils::InitRealArray(const TDF_Label&          theLab,
                                  const Standard_Integer    theSubTag,
                                  const Handle(HRealArray)& theArray)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);

  INIT_ARRAY_FROM_ARRAY(TDataStd_RealArray, aDataLab, theArray);
}

void ActData_Utils::InitRealArray(const TDF_Label&           theLab,
                                  const Standard_Integer     theSubTag,
                                  const Handle(HRealMatrix)& theMx)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);

  INIT_ARRAY_FROM_MX(TDataStd_RealArray, aDataLab, theMx);
}

void ActData_Utils::InitRealArray(const TDF_Label&             theLab,
                                  const Standard_Integer       theSubTag,
                                  const Handle(HComplexArray)& theArray,
                                  const Standard_Boolean       isReal)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);

  INIT_ARRAY_FROM_ARRAY_COMPLEX(TDataStd_RealArray, aDataLab, theArray, isReal);
}

void ActData_Utils::InitRealArray(const TDF_Label&              theLab,
                                  const Standard_Integer        theSubTag,
                                  const Handle(HComplexMatrix)& theMx,
                                  const Standard_Boolean        isReal)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);

  INIT_ARRAY_FROM_MX_COMPLEX(TDataStd_RealArray, aDataLab, theMx, isReal);
}

Handle(HRealMatrix)
  ActData_Utils::DispatchRealMatrix(const Handle(TDataStd_RealArray)& theArrayAttr,
                                    const Standard_Integer            theNbRows,
                                    const Standard_Integer            theNbCols)
{
  RET_DISPATCHED_ARRAY2MX(HRealMatrix, theNbRows, theNbCols, theArrayAttr)
}

Handle(HComplexMatrix)
    ActData_Utils::DispatchRealMatrices(const Handle(TDataStd_RealArray)& theArrayRealAttr,
                                        const Handle(TDataStd_RealArray)& theArrayImagAttr,
                                        const Standard_Integer            theNbRows,
                                        const Standard_Integer            theNbCols)
{
  RET_DISPATCHED_ARRAYS2COMPLEXMX(HComplexMatrix, theNbRows, theNbCols,
                                  theArrayRealAttr, theArrayImagAttr)
}

void
  ActData_Utils::SetRealMatrixElem(const Handle(TDataStd_RealArray)& theArrayAttr,
                                   const Standard_Integer            theRowIndex,
                                   const Standard_Integer            theColIndex,
                                   const Standard_Integer            theNbCols,
                                   const Standard_Real               theValue)
{
  Standard_Integer aFlatIndex = FLAT_MX_INDEX(theRowIndex, theColIndex, theNbCols);
  theArrayAttr->SetValue(aFlatIndex, theValue);
}

Standard_Real
  ActData_Utils::GetRealMatrixElem(const Handle(TDataStd_RealArray)& theArrayAttr,
                                   const Standard_Integer            theRowIndex,
                                   const Standard_Integer            theColIndex,
                                   const Standard_Integer            theNbCols)
{
  Standard_Integer aFlatIndex = FLAT_MX_INDEX(theRowIndex, theColIndex, theNbCols);
  return theArrayAttr->Value(aFlatIndex);
}

Handle(HRealArray)
  ActData_Utils::GetRealArray(const TDF_Label&       theLab,
                              const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);
  if ( aDataLab.IsNull() )
    return NULL;

  Handle(TDataStd_RealArray) aDataAttr;
  if ( aDataLab.FindAttribute(TDataStd_RealArray::GetID(), aDataAttr) )
    return aDataAttr->Array();

  return NULL; // Null (EMPTY) array
}

void ActData_Utils::BackupRealArray(const TDF_Label&       theLab,
                                    const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_RealArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_RealArray::GetID(), aDataAttr);

  aDataAttr->Backup();
}

void ActData_Utils::SetRealArrayElem(const TDF_Label&       theLab,
                                     const Standard_Integer theSubTag,
                                     const Standard_Integer theIndex,
                                     const Standard_Real    theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_RealArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_RealArray::GetID(), aDataAttr);
  aDataAttr->SetValue(theIndex, theValue);
}

Standard_Real
  ActData_Utils::GetRealArrayElem(const TDF_Label&       theLab,
                                  const Standard_Integer theSubTag,
                                  const Standard_Integer theIndex)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_RealArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_RealArray::GetID(), aDataAttr);
  return aDataAttr->Value(theIndex);
}

//-----------------------------------------------------------------------------
// BOOLEAN Arrays
//-----------------------------------------------------------------------------

void ActData_Utils::InitBooleanArray(const TDF_Label&          theLab,
                                     const Standard_Integer    theSubTag,
                                     const Handle(HBoolArray)& theArray)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);

  INIT_ARRAY_FROM_ARRAY(TDataStd_BooleanArray, aDataLab, theArray);
}

Handle(HBoolArray)
  ActData_Utils::GetBooleanArray(const TDF_Label&       theLab,
                                 const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_BooleanArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_BooleanArray::GetID(), aDataAttr);

  if ( aDataAttr.IsNull() || aDataAttr->InternalArray().IsNull() )
    return NULL;

  Handle(HBoolArray) aResult = new HBoolArray(0, aDataAttr->Length() - 1);
  Standard_Integer aIndx = 0;
  for ( Standard_Integer i = aDataAttr->Lower(); i <= aDataAttr->Upper(); i++ )
  {
    aResult->SetValue( aIndx++, aDataAttr->Value(i) );
  }

  return aResult;
}

void ActData_Utils::BackupBooleanArray(const TDF_Label&       theLab,
                                       const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_BooleanArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_BooleanArray::GetID(), aDataAttr);

  aDataAttr->Backup();
}

void ActData_Utils::SetBooleanArrayElem(const TDF_Label&       theLab,
                                        const Standard_Integer theSubTag,
                                        const Standard_Integer theIndex,
                                        const Standard_Boolean theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_BooleanArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_BooleanArray::GetID(), aDataAttr);
  aDataAttr->SetValue(theIndex, theValue);
}

Standard_Boolean
  ActData_Utils::GetBooleanArrayElem(const TDF_Label&       theLab,
                                     const Standard_Integer theSubTag,
                                     const Standard_Integer theIndex)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_BooleanArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_BooleanArray::GetID(), aDataAttr);
  return aDataAttr->Value(theIndex);
}

//-----------------------------------------------------------------------------
// STRING Arrays
//-----------------------------------------------------------------------------

void ActData_Utils::InitStringArray(const TDF_Label&            theLab,
                                    const Standard_Integer      theSubTag,
                                    const Handle(HStringArray)& theArray)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);

  INIT_ARRAY_FROM_ARRAY(TDataStd_ExtStringArray, aDataLab, theArray);
}

void ActData_Utils::InitStringArray(const TDF_Label&             theLab,
                                    const Standard_Integer       theSubTag,
                                    const Handle(HStringMatrix)& theMx)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag, Standard_True);

  INIT_ARRAY_FROM_MX(TDataStd_ExtStringArray, aDataLab, theMx);
}

Handle(HStringMatrix)
  ActData_Utils::DispatchStringMatrix(const Handle(TDataStd_ExtStringArray)& theArrayAttr,
                                      const Standard_Integer                 theNbRows,
                                      const Standard_Integer                 theNbCols)
{
  RET_DISPATCHED_ARRAY2MX(HStringMatrix, theNbRows, theNbCols, theArrayAttr)
}

void
  ActData_Utils::SetStringMatrixElem(const Handle(TDataStd_ExtStringArray)& theArrayAttr,
                                     const Standard_Integer                 theRowIndex,
                                     const Standard_Integer                 theColIndex,
                                     const Standard_Integer                 theNbCols,
                                     const TCollection_ExtendedString&      theValue)
{
  Standard_Integer aFlatIndex =
    FLAT_MX_INDEX(theRowIndex, theColIndex, theNbCols);
  theArrayAttr->SetValue(aFlatIndex, theValue);
}

TCollection_ExtendedString
  ActData_Utils::GetStringMatrixElem(const Handle(TDataStd_ExtStringArray)& theArrayAttr,
                                     const Standard_Integer                 theRowIndex,
                                     const Standard_Integer                 theColIndex,
                                     const Standard_Integer                 theNbCols)
{
  Standard_Integer aFlatIndex =
    FLAT_MX_INDEX(theRowIndex, theColIndex, theNbCols);
  return theArrayAttr->Value(aFlatIndex);
}

Handle(HStringArray)
  ActData_Utils::GetStringArray(const TDF_Label&       theLab,
                                const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);
  if ( aDataLab.IsNull() )
    return NULL;

  Handle(TDataStd_ExtStringArray) aDataAttr;
  if ( aDataLab.FindAttribute(TDataStd_ExtStringArray::GetID(), aDataAttr) )
    return aDataAttr->Array();

  return NULL;
}

void ActData_Utils::BackupStringArray(const TDF_Label&       theLab,
                                      const Standard_Integer theSubTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_ExtStringArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_ExtStringArray::GetID(), aDataAttr);

  aDataAttr->Backup();
}

void ActData_Utils::SetStringArrayElem(const TDF_Label&                  theLab,
                                       const Standard_Integer            theSubTag,
                                       const Standard_Integer            theIndex,
                                       const TCollection_ExtendedString& theValue)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_ExtStringArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_ExtStringArray::GetID(), aDataAttr);
  aDataAttr->SetValue(theIndex, theValue);
}

TCollection_ExtendedString
  ActData_Utils::GetStringArrayElem(const TDF_Label&       theLab,
                                    const Standard_Integer theSubTag,
                                    const Standard_Integer theIndex)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theSubTag);

  Handle(TDataStd_ExtStringArray) aDataAttr;
  aDataLab.FindAttribute(TDataStd_ExtStringArray::GetID(), aDataAttr);
  return aDataAttr->Value(theIndex);
}

//-----------------------------------------------------------------------------
// Tree Function support
//-----------------------------------------------------------------------------

//! Gathers all Tree Function Parameters which have PENDING outputs in scope
//! of the passed Data Node.
//! \param theNode [in] Node to gather Tree Function Parameters for.
//! \return list of Tree Function Parameters.
Handle(ActAPI_HParameterList)
  ActData_Utils::PendingFunctionParams(const Handle(ActAPI_INode)& theNode)
{
  // Iterate over the Nodal Parameters
  Handle(ActAPI_HParameterList) PFuncRoots = new ActAPI_HParameterList;
  for ( Handle(ActAPI_IParamIterator) pit = theNode->GetParamIterator(); pit->More(); pit->Next() )
  {
    const Handle(ActAPI_IUserParameter)& P = pit->Value();
    if ( P->GetParamType() != Parameter_TreeFunction )
      continue; // Skip non-functional Parameters

    Handle(ActData_TreeFunctionParameter) PFunc = ActParamTool::AsTreeFunction(P);

    if ( !PFunc->IsHeavyFunction() )
      continue; // Skip lightweight Tree Functions

    if ( PFunc->HasPendingResults() )
      PFuncRoots->Append(PFunc);
  }
  return PFuncRoots;
}

//-----------------------------------------------------------------------------
// References
//-----------------------------------------------------------------------------

//! Checks whether the given list contains the passed Label of interest.
//! \param theList      [in] list to check.
//! \param theTargetLab [in] OCAF Label to check.
//! \return index of the target or 0 if nothing was found.
Standard_Integer ActData_Utils::HasTarget(const TDF_LabelList& theList,
                                          const TDF_Label&     theTargetLab)
{
  Standard_Integer aTargetIdx = 0;
  for ( TDF_ListIteratorOfLabelList anIt(theList); anIt.More(); anIt.Next() )
  {
    aTargetIdx++;
    TDF_Label& aLab = anIt.Value();
    if ( aLab == theTargetLab )
      return aTargetIdx;
  }
  return 0; // Not found
}

//! Attempts to treat each Label in the given list as a root Label of some
//! Data Cursor. If it is possible for ALL (!) items, the collection of
//! Data Cursors is returned. If not, an exception is thrown.
//! \param theList [in] list of OCAF Labels to settle Data Cursors to.
//! \return list of Data Cursors.
Handle(ActAPI_HDataCursorList) ActData_Utils::ConvertToCursors(const TDF_LabelList& theList)
{
  Handle(ActAPI_HDataCursorList) aResult = new ActAPI_HDataCursorList();
  //
  for ( TDF_ListIteratorOfLabelList anIt(theList); anIt.More(); anIt.Next() )
  {
    TDF_Label aTargetRoot = anIt.Value();

    if ( ActData_ParameterFactory::IsUserParameter(aTargetRoot) ) // NODE
    {
      Standard_Boolean isUndefinedType;
      aResult->Append( ActData_ParameterFactory::NewParameterSettle(aTargetRoot, isUndefinedType) );
    }
    else if ( ActData_NodeFactory::IsNode(aTargetRoot) ) // PARAMETER
    {
      aResult->Append( ActData_NodeFactory::NodeSettle(aTargetRoot) );
    }
    else
    {
      std::cout << "Data inconsistency: bad referenced target"
                << ActData_Utils::GetEntry(aTargetRoot).ToCString() << std::endl;
    }
  }
  //
  return aResult;
}

//! Attempts to access Reference List attribute for the given (sub-)Label.
//! \param theLab [in] target OCAF Label.
//! \param theTag [in] tag for the sub-Label (or -1 if no sub-Label is to be used).
//! \return Reference List attribute.
Handle(TDataStd_ReferenceList)
  ActData_Utils::GetReferenceList(const TDF_Label&       theLab,
                                  const Standard_Integer theTag)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theTag);
  //
  if ( aDataLab.IsNull() )
    return NULL;

  Handle(TDataStd_ReferenceList) aRefList;
  aDataLab.FindAttribute(TDataStd_ReferenceList::GetID(), aRefList);
  return aRefList;
}

//! Assuming that the root Label (the first passed one) contains Reference List
//! attribute, this function attempts to put the given target Label at the
//! beginning of this list. If such an attribute does not exist, it is
//! created.
//! \param theLab       [in] owning Label.
//! \param theTag       [in] tag to access sub-Label (if needed).
//! \param theTargetLab [in] OCAF Label to put at the beginning.
void ActData_Utils::PrependReference(const TDF_Label&       theLab,
                                     const Standard_Integer theTag,
                                     const TDF_Label&       theTargetLab)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theTag);
  //
  if ( aDataLab.IsNull() )
    return;

  Handle(TDataStd_ReferenceList) aRefsAttr;
  if ( !aDataLab.FindAttribute(TDataStd_ReferenceList::GetID(), aRefsAttr) )
    aRefsAttr = TDataStd_ReferenceList::Set(aDataLab);
  //
  aRefsAttr->Prepend(theTargetLab);
}

//! Assuming that the root Label (the first passed one) contains Reference List
//! attribute, this function attempts to put the given target Label at the
//! end of this list. If such an attribute does not exist, it is
//! created.
//! \param theLab       [in] owning Label.
//! \param theTag       [in] tag to access sub-Label (if needed).
//! \param theTargetLab [in] OCAF Label to put at the end.
void ActData_Utils::AppendReference(const TDF_Label&       theLab,
                                    const Standard_Integer theTag,
                                    const TDF_Label&       theTargetLab)
{
  TDF_Label aDataLab = ChooseLabelByTag(theLab, theTag, Standard_True);
  //
  if ( aDataLab.IsNull() )
    return;

  Handle(TDataStd_ReferenceList) aRefsAttr;
  if ( !aDataLab.FindAttribute(TDataStd_ReferenceList::GetID(), aRefsAttr) )
    aRefsAttr = TDataStd_ReferenceList::Set(aDataLab);
  //
  aRefsAttr->Append(theTargetLab);
}

//-----------------------------------------------------------------------------
// Tree Nodes
//-----------------------------------------------------------------------------

//! Finds or creates Tree Node attribute for the given Label.
//! \param theLab   [in] target OCAF Label.
//! \param toCreate [in] indicates whether to create a Tree Node attribute in
//!                      case if it does not exist by the moment of access.
//! \return Tree Node attribute.
Handle(TDataStd_TreeNode)
  ActData_Utils::AccessTreeNode(const TDF_Label&       theLab,
                                const Standard_Boolean toCreate)
{
  if ( theLab.IsNull() )
    return NULL;

  Handle(TDataStd_TreeNode) aTreeNodeAttr;
  if ( !theLab.FindAttribute(TDataStd_TreeNode::GetDefaultTreeID(), aTreeNodeAttr) &&
        toCreate )
    aTreeNodeAttr = TDataStd_TreeNode::Set(theLab);

  return aTreeNodeAttr;
}

//! Appends one Tree Node attribute as a child to another.
//! \param theParent [in] parent Tree Node.
//! \param theChild  [in] child Tree Node.
void ActData_Utils::AppendChild(const Handle(TDataStd_TreeNode)& theParent,
                                const Handle(TDataStd_TreeNode)& theChild)
{
  if ( theChild->HasFather() )
    theChild->Remove();

  theParent->Append(theChild);
}

//! Removes the second Tree Node from the list of children of the first one.
//! \param theParent [in] parent Tree Node.
//! \param theChild  [in] child Tree Node.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_Utils::RemoveChild(const Handle(TDataStd_TreeNode)& theParent,
                                            const Handle(TDataStd_TreeNode)& theChild)
{
  if ( theChild->Father() != theParent )
    return Standard_False; // The passed Tree Node Parameter is not a child
                           // for this one, so lets return false

  // Remove the child Tree Node from this one
  return theChild->Remove();
}

//-----------------------------------------------------------------------------
// Internal methods
//-----------------------------------------------------------------------------

TDF_Label ActData_Utils::ChooseLabelByTag(const TDF_Label&       theLab,
                                          const Standard_Integer theTag,
                                          const Standard_Boolean toCreate)
{
  TDF_Label aResult =
    ( theTag == -1 ? theLab : theLab.FindChild(theTag, toCreate) );

  return aResult;
}

//-----------------------------------------------------------------------------
// Namespace: ActData_StringAux
//-----------------------------------------------------------------------------

//! Checks whether the passed character is not a special one.
//! \param theChar [in] character to test.
//! \return true/false.
Standard_Boolean ActData_StringAux::IsNotSpecial(const Standard_Character Char)
{
  return (Char == 95) ||               // Underscore
         (Char >= 48 && Char <= 57) || // Numbers
         (Char >= 65 && Char <= 90) || // Upper letters
         (Char >= 97 && Char <= 122);  // Lower letters
}

//! Checks whether the passed Source string contains the Piece string as a
//! sub-string. If so, the Start and End indices are returned as output
//! parameters.
//! \param Source [in] source string to find the piece string in.
//! \param Piece [in] piece to find.
//! \param Start [out] starting index.
//! \param End [out] ending index.
//! \return true if the given sub-string was found, false -- otherwise.
Standard_Boolean ActData_StringAux::IsLexeme(const TCollection_AsciiString& Source,
                                             const TCollection_AsciiString& Piece,
                                             Standard_Integer&              Start,
                                             Standard_Integer&              End)
{
  Standard_Integer aPos = Source.Search(Piece);

  if ( aPos == -1 )
    return Standard_False;

  Standard_Boolean isMarginOnLeft, isMarginOnRight;

  if ( aPos == 1 || !IsNotSpecial( Source.Value(aPos - 1) ) )
    isMarginOnLeft = Standard_True;
  else
    isMarginOnLeft = Standard_False;

  Standard_Integer aPieceLen = Piece.Length();
  Standard_Integer aLastPos = aPos + aPieceLen - 1;

  if ( aLastPos == Source.Length() || !IsNotSpecial( Source.Value(aLastPos + 1) ) )
    isMarginOnRight = Standard_True;
  else
    isMarginOnRight = Standard_False;

  if ( !isMarginOnLeft || !isMarginOnRight )
    return Standard_False;
    
  Start = aPos;
  End = aLastPos;
  return Standard_True;
}

//! Replaces all characters from the Start till the End indices in the
//! given string by the passed Target sub-string. All characters in the
//! specified range are removed. The resulting string is grown so that
//! to accommodate the Target sub-string without truncation.
//! \param String [in/out] string to modify.
//! \param Target [in] sub-string to replace the characters with.
//! \param Start [in] first character in the replacement range.
//! \param End [in] last character in the replacement range.
Standard_Boolean ActData_StringAux::Replace(TCollection_AsciiString&       String,
                                            const TCollection_AsciiString& Target,
                                            const Standard_Integer         Start,
                                            const Standard_Integer         End)
{
  String.Remove(Start, End - Start + 1);

  if ( Start > 1 )
    String.InsertAfter(Start - 1, Target);
  else
    String.Prepend(Target);

  return Standard_True;
}

//! Performs multiple-pass replacement of the 'What' sub-string with the 'With'
//! sub-string in the given 'Source' string traversing the initial 'Source' string
//! from left to right till every occurrence of 'What' is exchanged with 'With'.
//! \param Source [in/out] source string to modify.
//! \param What [in] what to find & replace.
//! \param With [in] replacement sub-string.
void ActData_StringAux::ReplaceRecursive(TCollection_AsciiString&       Source,
                                         const TCollection_AsciiString& What,
                                         const TCollection_AsciiString& With)
{
  Standard_Integer aStart = -1, aEnd = -1;
  if ( IsLexeme(Source, What, aStart, aEnd) )
  {
    Replace(Source, With, aStart, aEnd);
    ReplaceRecursive(Source, What, With);
  }
}
