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

#ifndef ActData_TreeNodeParameter_HeaderFile
#define ActData_TreeNodeParameter_HeaderFile

// Active Data includes
#include <ActData_UserParameter.h>
#include <ActData_Common.h>

// OCCT includes
#include <TDataStd_TreeNode.hxx>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_TreeNodeDTO, ActData_ParameterDTO)

//! \ingroup AD_DF
//!
//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Tree Node Parameter without any OCAF connectivity.
class ActData_TreeNodeDTO : public ActData_ParameterDTO
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_TreeNodeDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param theGID [in] GID.
  ActData_TreeNodeDTO(const ActAPI_ParameterGID& theGID)
  : ActData_ParameterDTO ( theGID, Parameter_TreeNode ),
    ParentId             ( ActAPI_DataObjectId() ),
    PID                  ( -1 )
  {}

public:

  ActAPI_DataObjectId ParentId; //!< ID of the parent object.
  int                 PID;      //!< PID of the parent Parameter.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_TreeNodeParameter, ActData_UserParameter)

//! \ingroup AD_DF
//!
//! Using this type of Parameter you are free to create your own tree of
//! Nodes which is non-standard. Be aware that Tree Node functionality is
//! already implemented in META header of each Node. So this Parameter is
//! kept only for very specific and not very common use cases.
class ActData_TreeNodeParameter : public ActData_UserParameter
{
friend class ActData_BaseNode;
friend class ActData_BaseChildIterator;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_TreeNodeParameter, ActData_UserParameter)

public:

  ActData_EXPORT static Handle(ActData_TreeNodeParameter)
    Instance();

  ActData_EXPORT void
    AppendChild(const Handle(ActData_TreeNodeParameter)& theTreeNode);

  ActData_EXPORT Standard_Boolean
    RemoveChild(const Handle(ActData_TreeNodeParameter)& theTreeNode);

public:

  ActData_EXPORT Handle(TDataStd_TreeNode)
    GetCAFTreeNode();

protected:

  Handle(TDataStd_TreeNode)
    accessInternalNode(const TDF_Label&       theLab,
                       const Standard_Boolean toCreate = Standard_True);

private:

  ActData_TreeNodeParameter();

private:

  virtual Standard_Boolean isWellFormed() const;
  virtual Standard_Integer parameterType() const;

private:

  virtual void
    setFromDTO(const Handle(ActData_ParameterDTO)& theDTO,
               const ActAPI_ModificationType       theModType      = MT_Touched,
               const Standard_Boolean              doResetValidity = Standard_True,
               const Standard_Boolean              doResetPending  = Standard_True);

  virtual Handle(ActData_ParameterDTO)
    createDTO(const ActAPI_ParameterGID& theGID);

protected:

  //! Tags for the underlying CAF Labels.
  enum Datum
  {
    DS_TreeNode = ActData_UserParameter::DS_DatumLast,
    DS_DatumLast = DS_TreeNode + RESERVED_DATUM_RANGE
  };

};

#endif
