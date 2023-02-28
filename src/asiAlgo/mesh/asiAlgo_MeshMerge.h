//-----------------------------------------------------------------------------
// Created on: 23 May 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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

#ifndef asiAlgo_MeshMerge_h
#define asiAlgo_MeshMerge_h

// asiAlgo includes
#include <asiAlgo_MeshLink.h>

// Active Data includes
#include <ActData_Mesh.h>

// Mobius includes
#if defined USE_MOBIUS
#include <mobius/poly_Mesh.h>
#endif

// OCCT includes
#include <Poly_CoherentTriangulation.hxx>
#include <TColStd_PackedMapOfInteger.hxx>
#include <TopoDS_Face.hxx>

// Standard includes
#include <vector>

//-----------------------------------------------------------------------------

//! Given a shape with initialized tessellations inside, assembles all
//! triangles in a monolithic structure. Use this tool to assemble a single
//! tessellation from series of tessellations distributed by several faces.
//! The boundary information is preserved by means of a dedicated collection.
class asiAlgo_MeshMerge
{
public:

  typedef NCollection_DataMap< TopoDS_Face, NCollection_Vector<int> > t_faceElems;

  //! Conversion mode.
  enum Mode
  {
    Mode_PolyCoherentTriangulation,
    Mode_Mesh,
    Mode_MobiusMesh
  };

public:

  //! Puts all the passed triangulations into a single data structure
  //! with duplicated nodes.
  //! \param[in] tris the triangulations to put together.
  //! \return the united triangulation.
  asiAlgo_EXPORT static Handle(Poly_Triangulation)
    PutTogether(const std::vector<Handle(Poly_Triangulation)>& tris);

  //! Puts all triangulations from the passed shape into a single data structure
  //! with duplicated nodes.
  //! \param[in]  shape   the shape whose triangulations are to be put together.
  //! \param[out] history the mapping for face indices vs facet indices.
  //! \return the united triangulation.
  asiAlgo_EXPORT static Handle(Poly_Triangulation)
    PutTogether(const TopoDS_Shape& shape,
                t_faceElems&        history);

public:

  //! Ctor.
  //! \param[in] body         the CAD model to extract triangulation patches from.
  //! \param[in] mode         the conversion mode.
  //! \param[in] storeFaceIds the Boolean flag indicating whether to store face IDs
  //!                         in the mesh elements.
  asiAlgo_EXPORT
    asiAlgo_MeshMerge(const TopoDS_Shape& body,
                      const Mode          mode = Mode_PolyCoherentTriangulation,
                      const bool          storeFaceIds = true);

  //! Ctor.
  //! \param[in] triangulations the list of triangulations to merge into one.
  //! \param[in] mode           the conversion mode.
  asiAlgo_EXPORT
    asiAlgo_MeshMerge(const std::vector<Handle(Poly_Triangulation)>& triangulations,
                      const Mode                                     mode = Mode_PolyCoherentTriangulation);

public:

  //! \return result.
  const Handle(Poly_CoherentTriangulation)& GetResultPoly() const
  {
    return m_resultPoly;
  }

  //! \return result as Poly_Triangulation.
  Handle(Poly_Triangulation) GetResultTris() const
  {
    return m_resultPoly->GetTriangulation();
  }

  //! \return result.
  const Handle(ActData_Mesh)& GetResultMesh() const
  {
    return m_resultMesh;
  }

  //! \return map of indices of mesh triagnles for face.
  const t_faceElems& GetFaceElems() const
  {
  	return m_faceElems;
  }

  //! Converts merging tool to triangulation object.
  //! \return triangulation.
  operator Handle(Poly_Triangulation)()
  {
    return m_resultPoly->GetTriangulation();
  }

#if defined USE_MOBIUS
  //! \return the resulting mesh data structure of Mobius kernel.
  const mobius::t_ptr<mobius::t_mesh>& GetMobiusMesh() const
  {
    return m_resultMobMesh;
  }
#endif

protected:

  //! Merges multiple triangulations into a single one.
  //! \param[in] body         the CAD model to extract triangulation from.
  //! \param[in] mode         the conversion mode.
  //! \param[in] storeFaceIds the Boolean flag indicating whether to store face IDs
  //!                         in the mesh elements.
  void build(const TopoDS_Shape& body,
             const Mode          mode,
             const bool          storeFaceIds);

  //! Merges multiple triangulations into a single one.
  //! \param[in] triangulations the triangulations to merge.
  //! \param[in] mode           the conversion mode.
  void build(const std::vector<Handle(Poly_Triangulation)>& triangulations,
             const Mode                                     mode);

// OUTPUTS:
protected:

  Handle(Poly_CoherentTriangulation) m_resultPoly; //!< Result tessellation.
  Handle(ActData_Mesh)               m_resultMesh; //!< Result mesh.
  t_faceElems                        m_faceElems;  //!< Map to store indices of mesh triangles for face.

#if defined USE_MOBIUS
  //! Result Mobius mesh.
  mobius::t_ptr<mobius::t_mesh> m_resultMobMesh;
#endif

};

#endif
