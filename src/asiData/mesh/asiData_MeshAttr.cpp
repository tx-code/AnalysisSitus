//-----------------------------------------------------------------------------
// Created on: 06 July 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiData_MeshAttr.h>

// OCCT includes
#include <Standard_GUID.hxx>

#if defined USE_MOBIUS
  #include <mobius/poly_Mesh.h>

  using namespace mobius;
#endif

//-----------------------------------------------------------------------------
// Construction & settling-down routines
//-----------------------------------------------------------------------------

//! Default constructor.
asiData_MeshAttr::asiData_MeshAttr() : TDF_Attribute() {}

//! Settles down new Mesh Attribute to the given OCAF Label.
//! \param[in] label TDF Label to settle down the new Attribute to.
//! \return newly created Attribute settled down onto the target Label.
Handle(asiData_MeshAttr) asiData_MeshAttr::Set(const TDF_Label& label)
{
  Handle(asiData_MeshAttr) A;
  //
  if ( !label.FindAttribute(GUID(), A) )
  {
    A = new asiData_MeshAttr();
    label.AddAttribute(A);
  }
  return A;
}

//-----------------------------------------------------------------------------
// Accessors for Attribute's GUID
//-----------------------------------------------------------------------------

//! Returns statically defined GUID for Mesh Attribute.
//! \return statically defined GUID.
const Standard_GUID& asiData_MeshAttr::GUID()
{
  static Standard_GUID AttrGUID("FE3766D5-F3A7-42E2-907D-009FA41CA724");
  return AttrGUID;
}

//! Accessor for GUID associated with this kind of OCAF Attribute.
//! \return GUID of the OCAF Attribute.
const Standard_GUID& asiData_MeshAttr::ID() const
{
  return GUID();
}

//-----------------------------------------------------------------------------
// Attribute's kernel methods:
//-----------------------------------------------------------------------------

//! Creates new instance of Mesh Attribute which is not initially populated
//! with any data structures.
//! \return new instance of Mesh Attribute.
Handle(TDF_Attribute) asiData_MeshAttr::NewEmpty() const
{
  return new asiData_MeshAttr();
}

//! Performs data transferring from the given OCAF Attribute to this one.
//! This method is mainly used by OCAF Undo/Redo kernel as a part of
//! backup functionality.
//! \param[in] mainAttr OCAF Attribute to copy data from.
void asiData_MeshAttr::Restore(const Handle(TDF_Attribute)& mainAttr)
{
  m_mesh.Nullify();

  Handle(asiData_MeshAttr)
    fromCasted = Handle(asiData_MeshAttr)::DownCast(mainAttr);

  if ( !fromCasted->m_mesh.IsNull() )
  {
    t_ptr<poly_Mesh> mesh = fromCasted->m_mesh->DeepCopy();
    m_mesh = mesh;
  }
}

//! Supporting method for Copy/Paste functionality. Performs full copying of
//! the underlying data.
//! \param[in] into       where to paste.
//! \param[in] relocTable relocation table.
void asiData_MeshAttr::Paste(const Handle(TDF_Attribute)&       into,
                             const Handle(TDF_RelocationTable)& asiData_NotUsed(relocTable)) const
{
  Handle(asiData_MeshAttr) intoCasted = Handle(asiData_MeshAttr)::DownCast(into);

  intoCasted->m_mesh.Nullify();

  if ( !m_mesh.IsNull() )
  {
    t_ptr<poly_Mesh> mesh = m_mesh->DeepCopy();
    intoCasted->m_mesh = mesh;
  }
}

//-----------------------------------------------------------------------------
// Accessors for domain-specific data
//-----------------------------------------------------------------------------

//! Sets mesh to store.
//! \param[in] mesh mesh to store.
void asiData_MeshAttr::SetMesh(const t_ptr<poly_Mesh>& mesh)
{
  this->Backup();

  m_mesh = mesh;
}

//! Returns the stored mesh.
//! \return stored mesh.
const t_ptr<poly_Mesh>& asiData_MeshAttr::GetMesh() const
{
  return m_mesh;
}
