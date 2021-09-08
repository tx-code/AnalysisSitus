//-----------------------------------------------------------------------------
// Created on: September 2019
//-----------------------------------------------------------------------------
// Copyright (c) 2019, OPEN CASCADE SAS
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

#ifndef ActData_LogBookAttr_HeaderFile
#define ActData_LogBookAttr_HeaderFile

// ActData includes
#include <ActData.h>

// OCCT includes
#include <TDF_Attribute.hxx>
#include <TDF_LabelMap.hxx>

//! \ingroup AD_DF
//!
//! OCAF Attribute representing the logbook. The idea to have a custom attribute
//! which provides fast a lookup of labels through the managed map. Initially,
//! we used TDataStd_ReferenceList attributes to store the references to the
//! affected labels. However, a list is a very inefficient data structure and
//! does not provide sufficient performance for the big enough data models.
//! Therefore, this map is the 
class ActData_LogBookAttr : public TDF_Attribute
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_LogBookAttr, TDF_Attribute)

public:

  //! Default constructor.
  ActData_LogBookAttr() = default;

public:

  //! Settles down new Attribute to the given OCAF Label.
  //! \param[in] label TDF Label to settle down the new Attribute to.
  //! \return newly created Attribute settled down onto the target Label.
  ActData_EXPORT static Handle(ActData_LogBookAttr)
    Set(const TDF_Label& label);

  //! Returns statically defined GUID for the Attribute.
  //! \return statically defined GUID.
  ActData_EXPORT static const Standard_GUID&
    GUID();

// Attribute's core methods:
public:

  //! Accessor for GUID associated with this kind of OCAF Attribute.
  //! \return GUID of the OCAF Attribute.
  ActData_EXPORT virtual const Standard_GUID&
    ID() const;

  //! \return new instance of Attribute.
  ActData_EXPORT virtual Handle(TDF_Attribute)
    NewEmpty() const;

  //! Performs data transferring from the given OCAF Attribute to this one.
  //! This method is mainly used by OCAF Undo/Redo kernel as a part of
  //! backup functionality.
  //! \param[in] from OCAF Attribute to copy data from.
  ActData_EXPORT virtual void
    Restore(const Handle(TDF_Attribute)& from);

  //! Supporting method for Copy/Paste functionality. Performs full copying of
  //! the underlying data.
  //! \param[in] into       where to paste.
  //! \param[in] relocTable relocation table.
  ActData_EXPORT virtual void
    Paste(const Handle(TDF_Attribute)&       into,
          const Handle(TDF_RelocationTable)& relocTable) const;

// Getters/setters:
public:

  //! Adds the passed label to the managed map.
  //! \param[in] label label to add.
  ActData_EXPORT void
    LogLabel(const TDF_Label& label);

  //! Checks if the passed label is logged or not.
  //! \param[in] label label to check.
  //! \return true/false.
  ActData_EXPORT Standard_Boolean
    IsLogged(const TDF_Label& label) const;

  //! Cleans up the logbook.
  ActData_EXPORT void
    ReleaseLogged();

  //! Removes log record for the passed label from the logbook.
  //! \param[in] label label in question.
  ActData_EXPORT void
    ReleaseLogged(const TDF_Label& label);

public:

  //! \return const reference to the managed map of labels.
  const TDF_LabelMap& GetMap() const
  {
    return m_map;
  }

// Member fields:
private:

  TDF_LabelMap m_map; //!< Managed map of labels.

};

#endif
