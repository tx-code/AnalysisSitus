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

#ifndef ActData_ParameterFactory_HeaderFile
#define ActData_ParameterFactory_HeaderFile

// Active Data includes
#include <ActData_AsciiStringParameter.h>
#include <ActData_BoolArrayParameter.h>
#include <ActData_BoolParameter.h>
#include <ActData_Common.h>
#include <ActData_ComplexArrayParameter.h>
#include <ActData_GroupParameter.h>
#include <ActData_IntArrayParameter.h>
#include <ActData_IntParameter.h>
#include <ActData_MeshParameter.h>
#include <ActData_MetaParameter.h>
#include <ActData_NameParameter.h>
#include <ActData_ReferenceListParameter.h>
#include <ActData_ReferenceParameter.h>
#include <ActData_RealParameter.h>
#include <ActData_RealArrayParameter.h>
#include <ActData_SelectionParameter.h>
#include <ActData_ShapeParameter.h>
#include <ActData_StringArrayParameter.h>
#include <ActData_TimeStampParameter.h>
#include <ActData_TreeFunctionParameter.h>
#include <ActData_TreeNodeParameter.h>
#include <ActData_TriangulationParameter.h>

// Active Data (API) includes
#include <ActAPI_IParameter.h>

// OCCT includes
#include <TDF_Label.hxx>

#define Parameter_SafeDownCast_Macro(Type) \
  inline static Handle(ActData_##Type##Parameter) As##Type(const Handle(ActAPI_IUserParameter)& theParam) \
  { \
    return Handle(ActData_##Type##Parameter)::DownCast(theParam); \
  }

//! Convenience short-cut which is useful to reduce the amount of code
//! to be written. E.g. ActData_ParameterFactory::AsReal(...) becomes
//! ActParamTool::AsReal(...)
#define ActParamTool ActData_ParameterFactory

DEFINE_STANDARD_HANDLE(ActData_ParameterFactory, Standard_Transient)

//! \ingroup AD_DF
//!
//! Factory class for all existing types of Active Data Parameters.
class ActData_ParameterFactory : public Standard_Transient
{
friend class ActData_TreeFunctionDriver;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_ParameterFactory, Standard_Transient)

// Settling/Expanding:
public:

  ActData_EXPORT static Handle(ActAPI_IUserParameter)
    NewParameterDetached(const Standard_Integer theParamType,
                         Standard_Boolean&      isUndefined);

  ActData_EXPORT static Standard_Boolean
    IsUserParameter(const TDF_Label& theLabel);

  ActData_EXPORT static Standard_Boolean
    IsUserParameter(const TDF_Label&               theLabel,
                    Handle(ActAPI_IUserParameter)& theUserParam,
                    Standard_Boolean&              isUndefined);

  ActData_EXPORT static Standard_Boolean
    IsMetaParameter(const TDF_Label&               theLabel,
                    Handle(ActData_MetaParameter)& theMetaParam);

  ActData_EXPORT static Handle(ActAPI_IUserParameter)
    NewParameterSettle(const Standard_Integer theParamType,
                       const TDF_Label&       theLabel,
                       Standard_Boolean&      isUndefined);

  ActData_EXPORT static Handle(ActAPI_IUserParameter)
    NewParameterExpand(const Standard_Integer theParamType,
                       const TDF_Label&       theLabel,
                       Standard_Boolean&      isUndefined);

  ActData_EXPORT static Handle(ActAPI_IUserParameter)
    NewParameterSettle(const TDF_Label&  theLabel,
                       Standard_Boolean& isUndefined);

  ActData_EXPORT static Handle(ActAPI_HParameterList)
    ParamsByLabelsSettle(const TDF_LabelList& theLabels);

  ActData_EXPORT static Handle(ActAPI_IUserParameter)
    ParamByChildLabelSettle(const TDF_Label&  theLabel,
                            Standard_Boolean& isUndefined);

public:

  ActData_EXPORT static Handle(ActData_MetaParameter)
    MetaParamByLabelSettle(const TDF_Label& theLabel);

// Safe Conversion methods:
public:

  Parameter_SafeDownCast_Macro(AsciiString)
  Parameter_SafeDownCast_Macro(BoolArray)
  Parameter_SafeDownCast_Macro(Bool)
  Parameter_SafeDownCast_Macro(ComplexArray)
  Parameter_SafeDownCast_Macro(Group)
  Parameter_SafeDownCast_Macro(IntArray)
  Parameter_SafeDownCast_Macro(Int)
  Parameter_SafeDownCast_Macro(Mesh)
  Parameter_SafeDownCast_Macro(Name)
  Parameter_SafeDownCast_Macro(RealArray)
  Parameter_SafeDownCast_Macro(Real)
  Parameter_SafeDownCast_Macro(Reference)
  Parameter_SafeDownCast_Macro(ReferenceList)
  Parameter_SafeDownCast_Macro(Selection)
  Parameter_SafeDownCast_Macro(Shape)
  Parameter_SafeDownCast_Macro(StringArray)
  Parameter_SafeDownCast_Macro(TimeStamp)
  Parameter_SafeDownCast_Macro(TreeFunction)
  Parameter_SafeDownCast_Macro(TreeNode)
  Parameter_SafeDownCast_Macro(Triangulation)

private:

  ActData_ParameterFactory();

};

#endif
