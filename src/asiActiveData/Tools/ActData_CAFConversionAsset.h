//-----------------------------------------------------------------------------
// Created on: June 2012
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

#ifndef ActData_CAFConversionAsset_HeaderFile
#define ActData_CAFConversionAsset_HeaderFile

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_Common.h>
#include <ActData_ParameterFactory.h>

//! \ingroup AD_DF
//!
//! Utility class providing a set useful routines performing atomic
//! conversion modifications. This class is intended to assist you in
//! your custom backward compatibility conversion routines.
class ActData_CAFConversionAsset
{
public:

  ActData_EXPORT
    ActData_CAFConversionAsset(const Handle(ActAPI_IModel)& theModel);

// Verifications:
public:

  ActData_EXPORT Standard_Boolean
    HasBadFormedNodes() const;

  ActData_EXPORT Standard_Boolean
    HasUnMTimedParams(Handle(ActAPI_HParameterList)& theParams) const;

// Conversions:
public:

  ActData_EXPORT void
    ActualizeVersions();

  ActData_EXPORT void
    ModifyParams();

  ActData_EXPORT Handle(ActAPI_IUserParameter)
    ChangeParameterType(const Handle(ActAPI_INode)& theNode,
                        const Standard_Integer thePID,
                        const ActAPI_ParameterType theNewType);

private:

  Handle(ActAPI_IUserParameter)
    expandParam(const Handle(ActAPI_INode)& theNode,
                const Standard_Integer thePID,
                const ActAPI_ParameterType theType) const;

  Handle(ActAPI_IUserParameter)
    settleParam(const Handle(ActAPI_INode)& theNode,
                const Standard_Integer thePID) const;

  TDF_Label
    uScopePRoot(const Handle(ActAPI_INode)& theNode,
                const Standard_Integer thePID,
                const Standard_Boolean toCreate) const;

  void
    accessUnMTimedParams(const TDF_Label& theNodeLab,
                         Handle(ActAPI_HParameterList)& theParams) const;

  Handle(ActAPI_HParameterList)
    accessUnMTimedParamsBySection(const TDF_Label& theSectionLab) const;

private:

  //! Data Model being modified.
  Handle(ActData_BaseModel) m_modelBase;

};

#endif
