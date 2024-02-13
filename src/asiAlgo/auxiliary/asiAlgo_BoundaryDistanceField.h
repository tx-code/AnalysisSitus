//-----------------------------------------------------------------------------
// Created on: 29 March 2022
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

#ifndef asiAlgo_BoundaryDistanceField_HeaderFile
#define asiAlgo_BoundaryDistanceField_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// Poly includes
#include <mobius/poly_BaseDistanceField.h>
#include <mobius/poly_SVO.h>

// Core includes
#include <mobius/core_Ptr.h>

// Active Data includes
#include <ActAPI_IProgressNotifier.h>
#include <ActAPI_IPlotter.h>

//-----------------------------------------------------------------------------

class asiAlgo_BVHFacets;
class asiAlgo_IntersectBoxMesh;

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Distance field represented by voxelization and its associated real
//! function to calculate the distance values.
class asiAlgo_BoundaryDistanceField : public mobius::poly_BaseDistanceField
{
public:

  //! Ctor.
  //! \param[in] bvh       BVH structure.
  //! \param[in] precision distance field approximation precision.
  //! \param[in] isUniform indicates whether uniform voxelization is requested.
  //! \param[in] bndMode   boundary evaluation mode.
  //! \param[in] progress  progress notifier.
  //! \param[in] plotter   imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_BoundaryDistanceField(const Handle(asiAlgo_BVHFacets)& bvh,
                                  const double                     precision,
                                  const bool                       isUniform,
                                  const bool                       bndMode  = false,
                                  ActAPI_ProgressEntry             progress = nullptr,
                                  ActAPI_PlotterEntry              plotter  = nullptr);

  //! Ctor with initialization.
  //! \param[in] bvh       BVH structure.
  //! \param[in] pRoot     octree to handle.
  //! \param[in] precision distance field approximation precision.
  //! \param[in] isUniform indicates whether uniform voxelization is requested.
  //! \param[in] bndMode   boundary evaluation mode.
  //! \param[in] progress  progress notifier.
  //! \param[in] plotter   imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_BoundaryDistanceField(const Handle(asiAlgo_BVHFacets)& bvh,
                                  mobius::poly_SVO*                pRoot,
                                  const double                     precision,
                                  const bool                       isUniform,
                                  const bool                       bndMode  = false,
                                  ActAPI_ProgressEntry             progress = nullptr,
                                  ActAPI_PlotterEntry              plotter  = nullptr);

  //! Dtor.
  //! CAUTION: this dtor does not destroy the octree.
  asiAlgo_EXPORT virtual
    ~asiAlgo_BoundaryDistanceField();

public:

  //! Builds distance field with the specified spatial resolution for the
  //! passed distance function. The method accepts min and max cell sizes
  //! to limit the voxelization granularity. The precision argument is
  //! used to compare the real distance function with its linear approximation.
  //! The adaptive distance field strives to capture the field's bevaior, so
  //! any significant deviation of the distance from its linear approximation
  //! leads to sub-splitting. The latter will not happen if the uniform
  //! voxelization mode is requested.
  //!
  //! \param[in] minCellSize min allowed voxel size.
  //! \param[in] maxCellSize max allowed voxel size.
  //! \param[in] func        driving function.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Build(const double                                minCellSize,
          const double                                maxCellSize,
          const mobius::t_ptr<mobius::poly_RealFunc>& func);

public:

  //! Evaluates the distance field as a conventional trivariate function.
  //! \param[in] x X coordinate of the argument point.
  //! \param[in] y Y coordinate of the argument point.
  //! \param[in] z Z coordinate of the argument point.
  //! \return function value.
  asiAlgo_EXPORT virtual double
    Eval(const double x, const double y, const double z) const;

public:

  //! \return root SVO node.
  mobius::poly_SVO* GetRoot()
  {
    return m_pRoot;
  }

  //! Allows to set a new root node for the distance field. Use with
  //! care as this setter does absolutely nothing in terms of memory
  //! management.
  //! \param[in] pRoot new root SVO node to set.
  void SetRoot(mobius::poly_SVO* pRoot)
  {
    m_pRoot = pRoot;
  }

  //! Enables/disables the boundary evaluation mode. In this mode,
  //! only the zero-crossing voxels are computed precisely. The inner
  //! and the outer voxels are computed as -1 and +1 respectively.
  //! \param[in] on the Boolean value to set.
  void SetBoundaryEvaluationMode(const bool on)
  {
    m_bBndMode = on;
  }

  //! Sets the eforced limit for the octree depth. Use this function
  //! for debugging.
  //! \param[in] limit the limit to set.
  void SetDepthLimit(const int limit)
  {
    m_iDepthLimit = limit;
  }

  //! Sets distance threshold.
  //! \param[in] value the value to set.
  void SetDistanceThreshold(const double value)
  {
    m_fDistThreshold = value;
  }

  //! Creates shallow copy of this distance field.
  //! \return copy of the field pointing to the same octree.
  mobius::t_ptr<asiAlgo_BoundaryDistanceField> ShallowCopy() const
  {
    mobius::t_ptr<asiAlgo_BoundaryDistanceField>
      res = new asiAlgo_BoundaryDistanceField(m_bvh, m_bUniform, m_fPrecision);
    //
    res->SetRoot(m_pRoot);
    return res;
  }

protected:

  Handle(asiAlgo_BVHFacets) m_bvh;            //!< BVH structure.
  double                    m_fPrecision;     //!< Distance field approximation precision.
  bool                      m_bUniform;       //!< Indicates whether uniform mode is enabled.
  mobius::poly_SVO*         m_pRoot;          //!< Root voxel.
  bool                      m_bBndMode;       //!< Boundary evaluation mode.
  asiAlgo_IntersectBoxMesh* m_pBoxMeshInter;  //!< Box-mesh intersector.
  int                       m_iDepthLimit;    //!< Limit for octree depth.
  double                    m_fDistThreshold; //!< Distance threshold to split boundary cells.
  ActAPI_ProgressEntry      m_progress;       //!< Progress notifier.
  ActAPI_PlotterEntry       m_plotter;        //!< Imperative plotter.

};

#endif
