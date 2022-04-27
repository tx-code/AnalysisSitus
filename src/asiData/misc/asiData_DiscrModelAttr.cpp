//-----------------------------------------------------------------------------
// Created on: 27 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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

// Own include
#include <asiData_DiscrModelAttr.h>

// OCCT includes
#include <Standard_GUID.hxx>

//-----------------------------------------------------------------------------
// Construction & settling-down routines
//-----------------------------------------------------------------------------

//! Default ctor.
asiData_DiscrModelAttr::asiData_DiscrModelAttr()
: TDF_Attribute()
{}

//! Settles down new discrete model Attribute to the given OCAF Label.
//! \param[in] label TDF Label to settle down the new Attribute to.
//! \return newly created Attribute settled down onto the target Label.
Handle(asiData_DiscrModelAttr)
  asiData_DiscrModelAttr::Set(const TDF_Label& label)
{
  Handle(asiData_DiscrModelAttr) A;
  //
  if ( !label.FindAttribute(GUID(), A) )
  {
    A = new asiData_DiscrModelAttr();
    label.AddAttribute(A);
  }
  return A;
}

//-----------------------------------------------------------------------------
// Accessors for Attribute's GUID
//-----------------------------------------------------------------------------

//! Returns statically defined GUID for discrete model Attribute.
//! \return statically defined GUID.
const Standard_GUID& asiData_DiscrModelAttr::GUID()
{
  static Standard_GUID AttrGUID("1395BED6-B154-4A9B-A33F-76FEDD1B499B");
  return AttrGUID;
}

//! Accessor for GUID associated with this kind of OCAF Attribute.
//! \return GUID of the OCAF Attribute.
const Standard_GUID& asiData_DiscrModelAttr::ID() const
{
  return GUID();
}

//-----------------------------------------------------------------------------
// Attribute's kernel methods:
//-----------------------------------------------------------------------------

//! Creates new instance of the Attribute which is not initially populated
//! with any data structures.
//! \return new instance of the Attribute.
Handle(TDF_Attribute) asiData_DiscrModelAttr::NewEmpty() const
{
  return new asiData_DiscrModelAttr();
}

//! Performs data transferring from the given OCAF Attribute to this one.
//! This method is mainly used by OCAF Undo/Redo kernel as a part of
//! backup functionality.
//! \param[in] mainAttr OCAF Attribute to copy data from.
void asiData_DiscrModelAttr::Restore(const Handle(TDF_Attribute)& mainAttr)
{
  Handle(asiData_DiscrModelAttr)
    fromCasted = Handle(asiData_DiscrModelAttr)::DownCast(mainAttr);
  //
  m_discrModel = fromCasted->GetDiscrModel();
}

//! Supporting method for Copy/Paste functionality. Performs full copying of
//! the underlying data.
//! \param[in] into       where to paste.
//! \param[in] relocTable relocation table.
void asiData_DiscrModelAttr::Paste(const Handle(TDF_Attribute)&       into,
                                   const Handle(TDF_RelocationTable)& asiData_NotUsed(relocTable)) const
{
  Handle(asiData_DiscrModelAttr)
    intoCasted = Handle(asiData_DiscrModelAttr)::DownCast(into);
  intoCasted->SetDiscrModel(m_discrModel);
}

//-----------------------------------------------------------------------------
// Accessors for domain-specific data
//-----------------------------------------------------------------------------

//! Sets discrete model to store.
//! \param[in] model discrete model to store.
void asiData_DiscrModelAttr::SetDiscrModel(const Handle(asiAlgo::discr::Model)& model)
{
  this->Backup();

  m_discrModel = model;
}

//! Returns the stored discrete model.
//! \return stored discrete model.
const Handle(asiAlgo::discr::Model)& asiData_DiscrModelAttr::GetDiscrModel() const
{
  return m_discrModel;
}
