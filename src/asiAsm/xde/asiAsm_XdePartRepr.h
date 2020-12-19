//-----------------------------------------------------------------------------
// Created on: 19 December 2020
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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

#ifndef asiAsm_XdePartRepr_h
#define asiAsm_XdePartRepr_h

// asiAsm includes
#include <asiAsm_XdePersistentIds.h>

// OpenCascade includes
#include <TDataXtd_Triangulation.hxx>
#include <TNaming_NamedShape.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! In the XDE framework, there is no such thing as "part representation," so
//! we have to introduce it here. A part representation within our XDE interface
//! is nothing but a usual "data cursor" pointing to the part's attachment
//! TDF_Label. We enumerate some OCAF attributes as "representations," e.g.,
//! TNaming_NamedShape or TDataXtd_Triangulation. You can add more representations,
//! and the only restriction here would be having the unique GUIDs for them all.
//! Since all attributes are attached to the same root label, there is a bit of a
//! mess in the data hierarchy (the representations are mixed up with other
//! "housekeeping" attributes, such as TDataStd_TreeNode). Still, we do not change
//! the storage scheme to stay compatible with the XDE native format.
class asiAsm_XdePartRepr : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAsm_XdePartRepr, Standard_Transient)

public:

  //! \return the corresponding OCAF attribute.
  virtual const Handle(TDF_Attribute)&
    GetAttribute() const = 0;

  //! \return the GUID of the corresponding OCAF attribute.
  virtual const Standard_GUID&
    GetGUID() const = 0;

public:

  //! \return the owner part's ID.
  const asiAsm_XdePartId& GetPartId() const
  {
    return m_partId;
  }

protected:

  //! Default ctor is protected as the base class cannot be
  //! instantiated directly.
  asiAsm_XdePartRepr() = default;

protected:

  asiAsm_XdePartId m_partId; //!< ID of the part.

};

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! The primary representation of a part which is supported natively in the XDE
//! framework is the precise (curved) boundary representation. This representation
//! corresponds to the TNaming_NamedShape attribute of OCAF.
class asiAsm_XdePartBoundaryRepr : public asiAsm_XdePartRepr
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAsm_XdePartBoundaryRepr, asiAsm_XdePartRepr)

public:

  //! \return static GUID.
  static const Standard_GUID& GUID()
  {
    return TNaming_NamedShape::GetID();
  }

public:

  //! Ctor accepting the corresponding OCAF attribute.
  //! \param[in] attr TNaming_NamedShape attribute.
  asiAsm_XdePartBoundaryRepr(const Handle(TNaming_NamedShape)& attr)
  //
  : asiAsm_XdePartRepr (),
    m_attr             (attr)
  {}

public:

  //! \return B-rep shape.
  TopoDS_Shape GetShape() const
  {
    return m_attr->Get();
  }

  //! \return OCAF attribute.
  virtual const Handle(TDF_Attribute)& GetAttribute() const
  {
    return m_attr;
  }

  //! \return static GUID.
  virtual const Standard_GUID& GetGUID() const
  {
    return GUID();
  }

protected:

  Handle(TNaming_NamedShape) m_attr; //!< The corresponding OCAF attribute.

};

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! This part representation corresponds to the TDataXtd_Triangulation OCAF
//! attribute. This representation is a surface triangulation for CAD geometry.
//!
//! The TDataXtd_Triangulation attribute under the part's label is already an
//! extension to the XDE's storage scheme because CAD-agnostic meshes are not
//! natively supported there. However, since the TDataXtd_Triangulation attribute
//! is still a standard OCAF attribute, it could be saved and restored to and from
//! an XDE file (having the "xbf" extension) without any compatibility issues.
//!
//! However, note that the XDE framework's API is not aware of any representations
//! except for TNaming_NamedShape. E.g., you will not capture the mesh subdomains
//! in the triangulation for the sake of attaching any metadata there. That's a
//! limitation of the XDE framework itself that, among with some others, motivates
//! us to develop another data model for CAD assemblies.
class asiAsm_XdePartTriangulationRepr : public asiAsm_XdePartRepr
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAsm_XdePartTriangulationRepr, asiAsm_XdePartRepr)

public:

  //! \return static GUID.
  static const Standard_GUID& GUID()
  {
    return TDataXtd_Triangulation::GetID();
  }

public:

  //! Ctor accepting the TDataXtd_Triangulation OCAF attribute.
  //! \param[in] attr the corresponding TDataXtd_Triangulation attribute.
  asiAsm_XdePartTriangulationRepr(const Handle(TDataXtd_Triangulation)& attr)
  //
  : asiAsm_XdePartRepr (),
    m_attr             (attr)
  {}

public:

  //! \return the stored surface triangulation.
  const Handle(Poly_Triangulation)& GetTriangulation() const
  {
    return m_attr->Get();
  }

  //! \return OCAF attribute.
  virtual const Handle(TDF_Attribute)& GetAttribute() const
  {
    return m_attr;
  }

  //! \return static GUID.
  virtual const Standard_GUID& GetGUID() const
  {
    return GUID();
  }

protected:

  Handle(TDataXtd_Triangulation) m_attr; //!< The corresponding OCAF attribute.

};

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! Factory for part representations. Use this class to construct the representation
//! interfaces and also to check that the attributes you're passing are supported.
class asiAsm_XdePartReprFactory
{
public:

  //! Constructs part representation out of the passed attribute.
  //! \param[in] attr the OCAF attribute in question.
  //! \return new part representation or null handle if the passed attribute's
  //!         type is not supported.
  static Handle(asiAsm_XdePartRepr) New(const Handle(TDF_Attribute)& attr)
  {
    const Handle(Standard_Type)& type = attr->DynamicType();

    if ( type == STANDARD_TYPE(TNaming_NamedShape) )
    {
      return new asiAsm_XdePartBoundaryRepr( Handle(TNaming_NamedShape)::DownCast(attr) );
    }

    if ( type == STANDARD_TYPE(TDataXtd_Triangulation) )
    {
      return new asiAsm_XdePartTriangulationRepr( Handle(TDataXtd_Triangulation)::DownCast(attr) );
    }

    return nullptr;
  }

};

#endif
