//-----------------------------------------------------------------------------
// Created on: March 2016
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

#ifndef ActData_MetaParameter_HeaderFile
#define ActData_MetaParameter_HeaderFile

// Active Data includes
#include <ActData_Common.h>

// Active Data (API) includes
#include <ActAPI_IParameter.h>

// OCCT includes
#include <TDataStd_ReferenceList.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDF_LabelList.hxx>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_MetaDTO, Standard_Transient)

//! \ingroup AD_DF
//!
//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Meta Parameter without any OCAF connectivity.
class ActData_MetaDTO : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_MetaDTO, Standard_Transient)

public:

  //! Constructor.
  ActData_MetaDTO() : Standard_Transient() {}

public:

  TCollection_AsciiString        TypeName;       //!< Type of the owning Node.
  ActAPI_DataObjectId            TreeNodeParent; //!< ID of the parent object.
  Standard_Integer               UserFlags;      //!< User flags.
  Handle(ActAPI_HDataCursorList) InputReaders;   //!< Input readers.
  Handle(ActAPI_HDataCursorList) OutputWriters;  //!< Output readers.
  Handle(ActAPI_HDataCursorList) Referrers;      //!< Referrers.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_MetaParameter, ActAPI_IDataCursor)

//! \ingroup AD_DF
//!
//! Meta information associated with each Node. This kind of Parameter is
//! completely internal for the framework.
class ActData_MetaParameter : public ActAPI_IDataCursor
{
friend class ActData_BaseNode;
friend class ActData_CAFConversionCtx;
friend class ActData_ParameterFactory;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_MetaParameter, ActAPI_IDataCursor)

public:

  ActData_EXPORT static Handle(ActData_MetaParameter) Instance();

// Data Cursor API:
public:

  ActData_EXPORT virtual Standard_Boolean
    IsAttached();

  ActData_EXPORT virtual Standard_Boolean
    IsDetached();

  ActData_EXPORT virtual Standard_Boolean
    IsWellFormed();

  ActData_EXPORT virtual Standard_Boolean
    IsValidData();

  ActData_EXPORT virtual Standard_Boolean
    IsPendingData();

  ActData_EXPORT virtual ActAPI_DataObjectId
    GetId() const;

  ActData_EXPORT virtual TDF_Label
    RootLabel() const;

// Setters and getters:
public:

  //---------------------------------------------------------------------------
  // NODE

  ActData_EXPORT Handle(ActAPI_INode)
    GetNode();

  ActData_EXPORT ActAPI_DataObjectId
    GetNodeId() const;

  //---------------------------------------------------------------------------
  // TYPE

  ActData_EXPORT void
    SetTypeName(const TCollection_AsciiString& theTypeName);

  ActData_EXPORT TCollection_AsciiString
    GetTypeName() const;

  //---------------------------------------------------------------------------
  // TREE NODE

  ActData_EXPORT void
    AppendChild(const Handle(ActData_MetaParameter)& theTreeNode);

  ActData_EXPORT Standard_Boolean
    RemoveChild(const Handle(ActData_MetaParameter)& theTreeNode);

  ActData_EXPORT Handle(TDataStd_TreeNode)
    GetCAFTreeNode();

  //---------------------------------------------------------------------------
  // USER FLAGS

  ActData_EXPORT void
    SetUserFlags(const Standard_Integer theUserFlags);

  ActData_EXPORT Standard_Integer
    GetUserFlags() const;

  //---------------------------------------------------------------------------
  // REFERENCES

  ActData_EXPORT Handle(ActAPI_HDataCursorList)
    GetInputReaderCursors();

  ActData_EXPORT Handle(ActAPI_HDataCursorList)
    GetOutputWriterCursors();

  ActData_EXPORT Handle(ActAPI_HDataCursorList)
    GetReferrerCursors();

  ActData_EXPORT TDF_LabelList
    GetInputReaders() const;

  ActData_EXPORT TDF_LabelList
    GetOutputWriters() const;

  ActData_EXPORT TDF_LabelList
    GetReferrers() const;

  ActData_EXPORT Handle(TDataStd_ReferenceList)
    GetInputReadersAttr() const;

  ActData_EXPORT Handle(TDataStd_ReferenceList)
    GetOutputWritersAttr() const;

  ActData_EXPORT Handle(TDataStd_ReferenceList)
    GetReferrersAttr() const;

  //---------//

  ActData_EXPORT Standard_Boolean
    HasInputReader(const TDF_Label& theLab) const;

  ActData_EXPORT Standard_Boolean
    HasOutputWriter(const TDF_Label& theLab) const;

  ActData_EXPORT Standard_Boolean
    HasReferrer(const TDF_Label& theLab) const;

  //---------//

  ActData_EXPORT void
    PrependInputReader(const TDF_Label& theLab);

  ActData_EXPORT void
    PrependOutputWriter(const TDF_Label& theLab);

  ActData_EXPORT void
    PrependReferrer(const TDF_Label& theLab);

  //---------//

  ActData_EXPORT void
    AppendInputReader(const TDF_Label& theLab);

  ActData_EXPORT void
    AppendOutputWriter(const TDF_Label& theLab);

  ActData_EXPORT void
    AppendReferrer(const TDF_Label& theLab);

  //---------//

  ActData_EXPORT Standard_Boolean
    RemoveInputReader(const TDF_Label& theLab);

  ActData_EXPORT Standard_Boolean
    RemoveOutputWriter(const TDF_Label& theLab);

  ActData_EXPORT Standard_Boolean
    RemoveReferrer(const TDF_Label& theLab);

// Manipulation with DTO:
public:

  ActData_EXPORT virtual void
    SetFromDTO(const Handle(ActData_MetaDTO)& theDTO);

  ActData_EXPORT virtual Handle(ActData_MetaDTO)
    GetAsDTO();

// Evaluation mechanism:
public:

  Handle(ActAPI_HIndexedParameterMap)& Evaluators() { return m_evaluators; }

protected:

  ActData_MetaParameter();

// Data Cursor internals:
protected:

  virtual void attach   (const TDF_Label& theLabel);
  virtual void expandOn (const TDF_Label& theLabel);
  virtual void settleOn (const TDF_Label& theLabel);

protected:

  //! Stores a number of reserved tags for future extensions.
  static const Standard_Integer RESERVED_DATUM_RANGE = 100;

  //! Tags for the underlying data chunks (TDF Labels).
  enum Datum
  {
    DS_InputReaders  = 1, //!< Back-references for READER Tree Functions.
    DS_OutputWriters = 2, //!< Back-references for WRITER Tree Functions.
    DS_Referrers     = 3, //!< Back-references for standard references.
    DS_Evaluators    = 4  //!< Optional evaluator Tree Functions.
  };

  //! Parameter status: ATTACHED or DETACHED.
  enum StorageStatus
  {
    SS_Detached,
    SS_Attached
  };

protected:

  //! Root TDF Label for the Parameter.
  TDF_Label m_label;

  //! Current settling status (ATTACHED or DETACHED).
  StorageStatus m_status;

  //! Tree Function evaluation Parameters.
  Handle(ActAPI_HIndexedParameterMap) m_evaluators;

};

#endif
