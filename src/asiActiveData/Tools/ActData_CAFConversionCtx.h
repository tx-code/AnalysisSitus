//-----------------------------------------------------------------------------
// Created on: March 2013
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

#ifndef ActData_CAFConversionCtx_HeaderFile
#define ActData_CAFConversionCtx_HeaderFile

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_CAFConversionModel.h>
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>
#include <ActData_ParameterFactory.h>

// OCCT includes
#include <Message_ProgressIndicator.hxx>

//! \ingroup AD_DF
//!
//! Conversion Context utility. This tool is used to provide an efficient
//! and consistent way of data modification. At the first stage this
//! tool cumulates all modification requests. Then these requests are
//! applied at once, producing new Data Model instance.
class ActData_CAFConversionCtx
{
public:

  ActData_EXPORT
    ActData_CAFConversionCtx(const TCollection_AsciiString& theDumpPath);

// Auxiliary types and structures:
public:

  //! Class for modification records.
  struct Record
  {
    ActAPI_ParameterGID          GID;  //!< Original GID.
    Handle(ActData_ParameterDTO) Data; //!< Target data.

    //! Default constructor.
    Record() {}

    //! Complete constructor.
    //! \param theGID [in] GID.
    //! \param Data [in] Data.
    Record(const ActAPI_ParameterGID& theGID,
           const Handle(ActData_ParameterDTO)& theData) : GID(theGID), Data(theData) {}

    //! Constructor setting NULL for the passed GID.
    //! \param theGID [in] GID.
    Record(const ActAPI_ParameterGID& theGID) : GID(theGID), Data(NULL) {}

    //! Checks equality of two records.
    //! \param theRec [in] record to compare with.
    //! \return true/false.
    Standard_Boolean IsEqual(const Record& theRec) const
    {
      return ActAPI_ParameterGID::Hasher::IsEqual(GID, theRec.GID);
    }
  };

  //! Type short-cut for handled record instance.
  typedef NCollection_Shared<Record> HRecord;

  //! Structure describing modifications to apply.
  class Modification
  {
  public:

    //! Default constructor.
    Modification() {}

  public:

    inline NCollection_Sequence<Handle(HRecord)>& ListInsert() {return m_listInsert;}
    inline NCollection_Sequence<Handle(HRecord)>& ListDelete() {return m_listDelete;}
    inline NCollection_Sequence<Handle(HRecord)>& ListUpdate() {return m_listUpdate;}

  public:

    //! Cleans up recorded modifications.
    void Clear()
    {
      m_listInsert.Clear();
      m_listDelete.Clear();
      m_listUpdate.Clear();
    }

    //! Adds insertion record.
    //! \param theRec [in] record to add.
    //! \return true in case of success, false -- otherwise.
    Standard_Boolean AddInsert(const Handle(HRecord)& theRec)
    {
      m_listInsert.Append(theRec);
      return Standard_True;
    }

    //! Adds deletion record.
    //! \param theRec [in] record to add.
    //! \return true in case of success, false -- otherwise.
    Standard_Boolean AddDelete(const Handle(HRecord)& theRec)
    {
      if ( this->contains(m_listDelete, theRec) ||
           this->contains(m_listUpdate, theRec) )
        return Standard_False;

      m_listDelete.Append(theRec);
      return Standard_True;
    }

    //! Adds update record.
    //! \param theRec [in] record to add.
    //! \return true in case of success, false -- otherwise.
    Standard_Boolean AddUpdate(const Handle(HRecord)& theRec)
    {
      if ( this->contains(m_listDelete, theRec) ||
           this->contains(m_listUpdate, theRec) )
        return Standard_False;

      m_listUpdate.Append(theRec);
      return Standard_True;
    }

  private:

    //! Checks whether the given collection contains the passed item.
    //! \param theList [in] collection to check.
    //! \param theRec [in] item to check.
    //! \return true/false.
    Standard_Boolean contains(const NCollection_Sequence<Handle(HRecord)>& theList,
                              const Handle(HRecord)& theRec) const
    {
      for ( Standard_Integer i = 1; i <= theList.Length(); ++i )
      {
        if ( theList.Value(i)->IsEqual(*theRec) )
          return Standard_True;
      }
      return Standard_False;
    }

  private:

    NCollection_Sequence<Handle(HRecord)> m_listInsert; //!< List of insertions.
    NCollection_Sequence<Handle(HRecord)> m_listDelete; //!< List of removals.
    NCollection_Sequence<Handle(HRecord)> m_listUpdate; //!< List of modifications.

  };

// Common services:
public:

  ActData_EXPORT void
    Clear();

// Recording:
public:

  ActData_EXPORT Standard_Boolean
    Insert(const Handle(ActData_ParameterDTO)& theParamDTO,
           const ActAPI_ParameterGID& theGIDBefore = ActAPI_ParameterGID());

  ActData_EXPORT Standard_Boolean
    Update(const ActAPI_ParameterGID& theGID,
           const Handle(ActData_ParameterDTO)& theNewParamDTO);

  ActData_EXPORT Standard_Boolean
    Delete(const ActAPI_ParameterGID& theGID);

// Applying:
public:

  ActData_EXPORT Standard_Boolean
    Apply(const Handle(ActAPI_IModel)& theModel,
          const Handle(Message_ProgressIndicator)& theProgress = nullptr);

  ActData_EXPORT const Handle(ActAPI_IModel)&
    Result() const;

// Applying internals:
private:

  Standard_Boolean
    applyNormalization(const Handle(Message_ProgressIndicator)& theProgress);

  Standard_Boolean
    applyModifications(const Handle(Message_ProgressIndicator)& theProgress);

  Standard_Boolean
    applyInsert(const Handle(HRecord)& theRec);

  Standard_Boolean
    applyUpdate(const Handle(HRecord)& theRec);

  Standard_Boolean
    applyRemove(const Handle(HRecord)& theRec);

private:

  Standard_Boolean
    normalizeNodalSection(const TDF_Label&       theNodeRoot,
                          const Standard_Boolean isInternal);

  Standard_Boolean
    normalizeReferenceList(const TDF_Label&       theRefListOwner,
                           const Standard_Boolean isBackRef);

  Handle(ActAPI_IUserParameter)
    paramByGID(const ActAPI_ParameterGID& theGID) const;

  Standard_Boolean
    samplerToOCAF();

  Standard_Boolean
    saveAndRetrieve(const Handle(ActAPI_IModel)& theOldModel,
                    const Handle(ActAPI_IModel)& theNewModel);

  TCollection_AsciiString
    temporaryFilename(const Handle(TDocStd_Document)& theDoc,
                      const Standard_Boolean isCopy = Standard_False) const;

  TDF_Label
    uSectionRoot(const ActAPI_DataObjectId& theNID,
                 const Handle(ActAPI_IModel)& theModel,
                 const Standard_Boolean canCreate) const;

private:

  void dumpModel(const Handle(ActAPI_IModel)& theModel) const;

private:

  //! Modification to apply.
  Modification m_modif;

  //! Directory for dumping of temporary Data Model.
  TCollection_AsciiString m_dumpPath;

  //! Conversion Model (sampler).
  Handle(ActData_CAFConversionModel) m_sampler;

  //! Original Data Model.
  Handle(ActAPI_IModel) m_oriModel;

  //! Resulting Data Model.
  Handle(ActAPI_IModel) m_resModel;

  //! Temporary Model instances. This collection is used to accumulate
  //! all temporary Models in order to release them once conversion is
  //! completed. It is absolutely necessary to release Data Model
  //! manually in order to resolve cyclic dependency between Model and its
  //! internal engine objects!
  NCollection_Sequence<Handle(ActAPI_IModel)> m_tempModels;

};

#endif
