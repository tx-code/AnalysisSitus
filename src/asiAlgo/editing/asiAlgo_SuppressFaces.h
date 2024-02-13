//-----------------------------------------------------------------------------
// Created on: 16 June 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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

#ifndef asiAlgo_SuppressFaces_h
#define asiAlgo_SuppressFaces_h

// asiAlgo includes
#include <asiAlgo_AAG.h>
#include <asiAlgo_FeatureFaces.h>

// OCCT includes
#include <BRepTools_ReShape.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Utility to delete faces.
class asiAlgo_SuppressFaces : public Standard_Transient
{
  // OCCT RTTI.
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_SuppressFaces, Standard_Transient)

public:

  //! Default ctor.
  asiAlgo_EXPORT
    asiAlgo_SuppressFaces();

  //! Constructor.
  //! \param[in] masterCAD full CAD model.
  asiAlgo_EXPORT
    asiAlgo_SuppressFaces(const TopoDS_Shape& masterCAD);

  //! Constructor.
  //! \param[in] aag attributed adjacency graph.
  asiAlgo_EXPORT
    asiAlgo_SuppressFaces(const Handle(asiAlgo_AAG)& aag);

public:

  //! Removes the given faces from the master model.
  //! \param[in] faceIndices indices faces to suppress.
  //! \param[in] facesOnly   indicates whether to delete faces only. Otherwise,
  //!                        all their belonging sub-shapes will be deleted,
  //!                        thus having impact on the neighbor shapes.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT virtual bool
    Perform(const asiAlgo_Feature& faceIndices,
            const bool             facesOnly);

public:

  //! Sets map of faces. If this method is not used, the map of faces will
  //! be taken from the input shape or from AAG (if provided).
  //! \param[in] faces map to set.
  void SetMapOfFaces(const TopTools_IndexedMapOfShape& faces)
  {
    m_faces = faces;
  }

  //! Sets AAG.
  //! \param[in] aag attributed adjacency graph.
  void SetAAG(const Handle(asiAlgo_AAG)& aag)
  {
    m_master = aag->GetMasterShape();
    m_aag    = aag;
  }

  //! \return result shape.
  const TopoDS_Shape& GetResult() const
  {
    return m_result;
  }

  //! \return instance of Re-Shape utility used for topological reduction.
  const Handle(BRepTools_ReShape)& GetReShape() const
  {
    return m_reShape;
  }

  //! \return history of modification.
  Handle(BRepTools_History) GetHistory() const
  {
    return m_reShape->History();
  }

protected:

  TopoDS_Shape               m_master;  //!< Master model.
  TopoDS_Shape               m_result;  //!< Result.
  Handle(BRepTools_ReShape)  m_reShape; //!< Re-Shape tool.
  Handle(asiAlgo_AAG)        m_aag;     //!< Optional AAG.
  TopTools_IndexedMapOfShape m_faces;   //!< Map of faces.

};

#endif
