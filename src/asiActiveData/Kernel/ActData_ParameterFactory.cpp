//-----------------------------------------------------------------------------
// Created on: February 2012
//-----------------------------------------------------------------------------
// Copyright (c) 2012-present, OPEN CASCADE SAS
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
#include <ActData_ParameterFactory.h>

// ActData includes
#include <ActData_NodeFactory.h>
#include <ActData_UserExtParameter.h>

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TDataStd_Integer.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>

//! Default constructor.
ActData_ParameterFactory::ActData_ParameterFactory()
{}

//! Factory method performing actual allocation of DETACHED Parameter instance
//! of the given type.
//! \param theParamType [in]  requested Parameter type.
//! \param isUndefined  [out] indicates whether the Parameter of the requested
//!                           type is undefined in the Factory. This normally
//!                           happens if the Parameter is of a non-standard
//!                           type (i.e., its type is declared externally to
//!                           Active Data).
//! \return new DETACHED Parameter instance.
Handle(ActAPI_IUserParameter)
  ActData_ParameterFactory::NewParameterDetached(const Standard_Integer theParamType,
                                                 Standard_Boolean&      isUndefined)
{
  isUndefined = Standard_False;
  Handle(ActAPI_IUserParameter) aResult;

  ActAPI_ParameterType aParamType = static_cast<ActAPI_ParameterType>(theParamType);
  switch ( aParamType )
  {
    case Parameter_Bool:
      aResult = ActData_BoolParameter::Instance();
      break;

    case Parameter_BoolArray:
      aResult = ActData_BoolArrayParameter::Instance();
      break;

    case Parameter_Int:
      aResult = ActData_IntParameter::Instance();
      break;

    case Parameter_Real:
      aResult = ActData_RealParameter::Instance();
      break;

    case Parameter_RealArray:
      aResult = ActData_RealArrayParameter::Instance();
      break;

    case Parameter_Shape:
      aResult = ActData_ShapeParameter::Instance();
      break;

    case Parameter_TreeFunction:
      aResult = ActData_TreeFunctionParameter::Instance();
      break;

    case Parameter_TreeNode:
      aResult = ActData_TreeNodeParameter::Instance();
      break;

    case Parameter_AsciiString:
      aResult = ActData_AsciiStringParameter::Instance();
      break;

    case Parameter_ComplexArray:
      aResult = ActData_ComplexArrayParameter::Instance();
      break;

    case Parameter_IntArray:
      aResult = ActData_IntArrayParameter::Instance();
      break;

    case Parameter_Name:
      aResult = ActData_NameParameter::Instance();
      break;

    case Parameter_StringArray:
      aResult = ActData_StringArrayParameter::Instance();
      break;

    case Parameter_ReferenceList:
      aResult = ActData_ReferenceListParameter::Instance();
      break;

    case Parameter_Group:
      aResult = ActData_GroupParameter::Instance();
      break;

    case Parameter_Mesh:
      aResult = ActData_MeshParameter::Instance();
      break;

    case Parameter_Reference:
      aResult = ActData_ReferenceParameter::Instance();
      break;

    case Parameter_Selection:
      aResult = ActData_SelectionParameter::Instance();
      break;

    case Parameter_TimeStamp:
      aResult = ActData_TimeStampParameter::Instance();
      break;

    case Parameter_Triangulation:
      aResult = ActData_TriangulationParameter::Instance();
      break;

    default:
      isUndefined = Standard_True;
      aResult     = ActData_UserExtParameter::Instance();
      break;
  }

  return aResult;
}

//! Checks whether the given Label represents Parameter data or not.
//! \param theLabel [in] raw Label to check.
//! \return true/false.
Standard_Boolean
  ActData_ParameterFactory::IsUserParameter(const TDF_Label& theLabel)
{
  /* =====================================
   *  Check if TYPE Attribute is in place
   * ===================================== */

  if ( theLabel.IsNull() )
    return Standard_False;

  TDF_Label aParamTypeLab =
    theLabel.FindChild(ActData_UserParameter::DS_ParamType, Standard_False);

  if ( aParamTypeLab.IsNull() )
    return Standard_False;

  Handle(TDataStd_Integer) aParamTypeAttr;
  if ( !aParamTypeLab.FindAttribute(TDataStd_Integer::GetID(), aParamTypeAttr) )
    return Standard_False;

  /* =========================================
   *  Check if IS_VALID Attribute is in place
   * ========================================= */

  TDF_Label aParamValidityLab =
    theLabel.FindChild(ActData_UserParameter::DS_IsValid, Standard_False);

  if ( aParamValidityLab.IsNull() )
    return Standard_False;

  Handle(TDataStd_Integer) aParamValidityAttr;
  if ( !aParamValidityLab.FindAttribute(TDataStd_Integer::GetID(), aParamValidityAttr) )
    return Standard_False;

  return Standard_True;
}

//! Checks whether the given Label represents User Parameter or not.
//! \param theLabel     [in]  raw Label to check.
//! \param theMetaParam [out] settled User Parameter or NULL.
//! \param isUndefined  [out] indicates whether the Parameter of the requested
//!                           type is undefined in the Factory. This normally
//!                           happens if the Parameter is of a non-standard
//!                           type (i.e., its type is declared externally to
//!                           Active Data).
//! \return true/false.
Standard_Boolean
  ActData_ParameterFactory::IsUserParameter(const TDF_Label&               theLabel,
                                            Handle(ActAPI_IUserParameter)& theUserParam,
                                            Standard_Boolean&              isUndefined)
{
  // Create User Parameter interface to use its IsWellFormed() checker
  Handle(ActAPI_IUserParameter) dao = NewParameterSettle(theLabel, isUndefined);
  //
  if ( dao.IsNull() )
    return Standard_False;

  // NewParameterSettle check the type attribute. So the only remaining
  // thing to check here is the validity attribute.

  TDF_Label aParamValidityLab =
    theLabel.FindChild(ActData_UserParameter::DS_IsValid, Standard_False);

  if ( aParamValidityLab.IsNull() )
    return Standard_False;

  Handle(TDataStd_Integer) aParamValidityAttr;
  if ( !aParamValidityLab.FindAttribute(TDataStd_Integer::GetID(), aParamValidityAttr) )
    return Standard_False;

  theUserParam = dao;

  return Standard_True;
}

//! Checks whether the given Label represents META Parameter or not.
//! \param theLabel     [in]  raw Label to check.
//! \param theMetaParam [out] settled META Parameter or NULL.
//! \return true/false.
Standard_Boolean
  ActData_ParameterFactory::IsMetaParameter(const TDF_Label&               theLabel,
                                            Handle(ActData_MetaParameter)& theMetaParam)
{
  if ( theLabel.IsNull() )
    return Standard_False;

  // Create META Parameter interface to use its IsWellFormed() checker
  Handle(ActData_MetaParameter) dao = ActData_MetaParameter::Instance();
  //
  dao->settleOn(theLabel);

  // Check Parameter itself
  const bool isParamOk = dao->IsWellFormed();
  //
  if ( !isParamOk )
    return Standard_False;

  theMetaParam = dao;

  // Attempt to settle the Node
  return ActData_NodeFactory::IsNode( theLabel.Father() );
}

//! READ-ONLY method settling down the Parameter Data Cursor of the given type
//! to the given CAF Label.
//! \param theParamType [in]  type of the Parameter to settle.
//! \param theLabel     [in]  target Label to get data access to.
//! \param isUndefined  [out] indicates whether the Parameter of the requested
//!                           type is undefined in the Factory. This normally
//!                           happens if the Parameter is of a non-standard
//!                           type (i.e., its type is declared externally to
//!                           Active Data).
//! \return new Parameter instance.
Handle(ActAPI_IUserParameter)
  ActData_ParameterFactory::NewParameterSettle(const Standard_Integer theParamType,
                                               const TDF_Label&       theLabel,
                                               Standard_Boolean&      isUndefined)
{
  Handle(ActData_UserParameter) aResult =
    Handle(ActData_UserParameter)::DownCast( NewParameterDetached(theParamType, isUndefined) );

  if ( !aResult.IsNull() )
    aResult->settleOn(theLabel);

  return aResult;
}

//! Method EXPANDING the Parameter Data Cursor of the given type on the given
//! CAF Label.
//! \param theParamType [in]  type of the Parameter to expand.
//! \param theLabel     [in]  target Label.
//! \param isUndefined  [out] indicates whether the Parameter of the requested
//!                           type is undefined in the Factory. This normally
//!                           happens if the Parameter is of a non-standard
//!                           type (i.e., its type is declared externally to
//!                           Active Data).
//! \return new Parameter instance.
Handle(ActAPI_IUserParameter)
  ActData_ParameterFactory::NewParameterExpand(const Standard_Integer theParamType,
                                               const TDF_Label&       theLabel,
                                               Standard_Boolean&      isUndefined)
{
  Handle(ActData_UserParameter) aResult =
    Handle(ActData_UserParameter)::DownCast( NewParameterDetached(theParamType, isUndefined) );

  if ( !aResult.IsNull() )
    aResult->expandOn(theLabel);

  return aResult;
}

//! READ-ONLY method settling down the Parameter Data Cursor to the given
//! CAF Label.
//! \param theLabel    [in]  target Label to get data access to.
//! \param isUndefined [out] indicates whether the Parameter of the requested
//!                          type is undefined in the Factory. This normally
//!                          happens if the Parameter is of a non-standard
//!                          type (i.e., its type is declared externally to
//!                          Active Data).
//! \return new Parameter instance.
Handle(ActAPI_IUserParameter)
  ActData_ParameterFactory::NewParameterSettle(const TDF_Label&  theLabel,
                                               Standard_Boolean& isUndefined)
{
  TDF_Label aTypeLab = theLabel.FindChild(ActData_UserParameter::DS_ParamType, Standard_False);

  if ( aTypeLab.IsNull() )
    return NULL;

  Handle(TDataStd_Integer) aTypeAttr;
  if ( !aTypeLab.FindAttribute(TDataStd_Integer::GetID(), aTypeAttr) )
    return NULL; // Type Attribute not found

  return NewParameterSettle(aTypeAttr->Get(), theLabel, isUndefined);
}

//! Constructs a list of Parameters by the list of the corresponding
//! TDF Labels.
//! \param theLabels [in] list of TDF Labels to pack.
//! \return high-level list of Parameters.
Handle(ActAPI_HParameterList)
  ActData_ParameterFactory::ParamsByLabelsSettle(const TDF_LabelList& theLabels)
{
  ActAPI_ParameterStream aPStream;
  TDF_ListIteratorOfLabelList aLabelIt(theLabels);
  for ( ; aLabelIt.More(); aLabelIt.Next() )
  {
    Standard_Boolean isUndefined;
    TDF_Label aLab = aLabelIt.Value();
    aPStream << ActData_ParameterFactory::NewParameterSettle(aLab, isUndefined);
  }

  return aPStream;
}

//! Attempts to find a Parameter instance owning the passed Label as one
//! of its children (any level is allowed).
//! \param theLabel    [in]  Label to recover a Parameter for.
//! \param isUndefined [out] indicates whether the Parameter of the requested
//!                          type is undefined in the Factory. This normally
//!                          happens if the Parameter is of a non-standard
//!                          type (i.e., its type is declared externally to
//!                          Active Data).
//! \return Parameter instance or NULL if nothing appropriate was found.
Handle(ActAPI_IUserParameter)
  ActData_ParameterFactory::ParamByChildLabelSettle(const TDF_Label&  theLabel,
                                                    Standard_Boolean& isUndefined)
{
  if ( IsUserParameter(theLabel) )
    return NewParameterSettle(theLabel, isUndefined);

  TDF_Label aFather = theLabel.Father();
  if ( aFather.IsNull() )
    return NULL;

  return ParamByChildLabelSettle(aFather, isUndefined);
}

//! Attempts to find a META Parameter instance owning the passed Label.
//! \param theLabel [in] Label to recover a META Parameter for.
//! \return Parameter instance or NULL if nothing appropriate was found.
Handle(ActData_MetaParameter)
  ActData_ParameterFactory::MetaParamByLabelSettle(const TDF_Label& theLabel)
{
  Handle(ActData_MetaParameter) theResult;
  //
  if ( IsMetaParameter(theLabel, theResult) )
    return theResult;

  return NULL;
}
