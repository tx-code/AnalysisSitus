//-----------------------------------------------------------------------------
// Created on: 10 August 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2017-present, Sergey Slyadnev
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
#include <asiAlgo_RelievePointCloud.h>

// OCCT includes
#include <gp_XY.hxx>
#include <NCollection_CellFilter.hxx>

namespace relieve
{
  //! Spatial point enriched with numeric identifier.
  struct t_point
  {
    gp_XYZ Point; //!< Geometric representation.
    int    ID;    //!< Associated ID.
  };

  //! Auxiliary class to search for coincident spatial points.
  class InspectXYZ : public NCollection_CellFilter_InspectorXYZ
  {
  public:

    typedef gp_XYZ Target;

    //! Constructor accepting resolution distance and point.
    InspectXYZ(const double tol, const gp_XYZ& P) : m_fTol(tol), m_bFound(false), m_P(P) {}

    //! \return true/false depending on whether the point was found or not.
    bool IsFound() const { return m_bFound; }

    //! Implementation of inspection method.
    NCollection_CellFilter_Action Inspect(const gp_XYZ& Target)
    {
      m_bFound = ( (m_P - Target).SquareModulus() <= Square(m_fTol) );
      return CellFilter_Keep;
    }

  private:

    gp_XYZ m_P;      //!< Source point.
    bool   m_bFound; //!< Whether two points are coincident or not.
    double m_fTol;   //!< Resolution to check for coincidence.

  };

  //! Auxiliary class to search for coincident tessellation Points.
  class InspectPoint : public InspectXYZ
  {
  public:

    typedef t_point Target;

    //! Constructor accepting resolution distance and point.
    InspectPoint(const double tol, const gp_XYZ& P)
    : InspectXYZ(tol, P)
    {
      found = false;
    }

    //! Implementation of inspection method.
    NCollection_CellFilter_Action Inspect(const t_point& Target)
    {
      InspectXYZ::Inspect(Target.Point);
      if ( IsFound() )
        found = true;
      return CellFilter_Keep;
    }

    bool found;
  };

  //---------------------------------------------------------------------------
  // Auxiliary functions
  //---------------------------------------------------------------------------

  void filterPoint(int&                                  globalPointId,
                   const gp_XYZ&                         xyz,
                   const double                          prec,
                   Handle(asiAlgo_BaseCloud<double>)&    res,
                   NCollection_CellFilter<InspectPoint>& cellFilter)
  {
    InspectPoint Inspect(prec, xyz);
    gp_XYZ XYZ_min = Inspect.Shift( xyz, -prec );
    gp_XYZ XYZ_max = Inspect.Shift( xyz,  prec );

    // Coincidence test with a tolerance.
    cellFilter.Inspect(XYZ_min, XYZ_max, Inspect);
    const bool isFound = Inspect.found;
    //
    if ( !isFound )
    {
      t_point N;
      N.ID    = globalPointId;
      N.Point = xyz;
      //
      cellFilter.Add(N, xyz);
      res->AddElement(xyz.X(), xyz.Y(), xyz.Z());

      // Increment global Point ID.
      ++globalPointId;
    }
  }

  void filterPoint(const int                             localPointId,
                   int&                                  globalPointId,
                   const gp_XYZ&                         xyz,
                   const double                          prec,
                   std::vector<int>&                     res,
                   NCollection_CellFilter<InspectPoint>& cellFilter)
  {
    InspectPoint Inspect(prec, xyz);
    gp_XYZ XYZ_min = Inspect.Shift( xyz, -prec );
    gp_XYZ XYZ_max = Inspect.Shift( xyz,  prec );

    // Coincidence test with a tolerance.
    cellFilter.Inspect(XYZ_min, XYZ_max, Inspect);
    const bool isFound = Inspect.found;
    //
    if ( !isFound )
    {
      t_point N;
      N.ID    = globalPointId;
      N.Point = xyz;
      //
      cellFilter.Add(N, xyz);
      res.push_back(localPointId);

      // Increment global Point ID.
      ++globalPointId;
    }
  }

};

//-----------------------------------------------------------------------------

asiAlgo_RelievePointCloud::asiAlgo_RelievePointCloud() {}

//-----------------------------------------------------------------------------

Handle(asiAlgo_BaseCloud<double>)
  asiAlgo_RelievePointCloud::operator()(const Handle(asiAlgo_BaseCloud<double>)& pc,
                                        const double                             tol) const
{
  // Create result.
  Handle(asiAlgo_BaseCloud<double>) res = new asiAlgo_BaseCloud<double>;

  // Working tools and variables.
  int curID = 0;
  NCollection_CellFilter<relieve::InspectPoint> PointFilter(tol);

  // Iterate over the points and add them one after another to the cell filter.
  double x, y, z;
  for ( int i = 0; i < pc->GetNumberOfElements(); ++i )
  {
    pc->GetElement(i, x, y, z);
    gp_XYZ currP(x, y, z);

    // Add point to the result passing it through the spatial cell filter.
    relieve::filterPoint(curID, currP, tol, res, PointFilter);
  }

  return res;
}

//-----------------------------------------------------------------------------

void asiAlgo_RelievePointCloud::operator()(const std::vector< std::pair<int, gp_Ax1> >& pc,
                                           const double                                 tol,
                                           std::vector< std::pair<int, gp_Ax1> >&       res) const
{
  // Working tools and variables.
  int curID = 0;
  NCollection_CellFilter<relieve::InspectPoint> PointFilter(tol);

  // Iterate over the points and add them one after another to the cell filter.
  int idx = 0;
  std::vector<int> resIds;
  for ( const auto& pt : pc )
  {
    gp_XYZ currP = pt.second.Location().XYZ();

    // Add point to the result passing it through the spatial cell filter.
    relieve::filterPoint(idx, curID, currP, tol, resIds, PointFilter);

    idx++;
  }

  // Recollect the points remaining in the result.
  for ( const auto id : resIds )
    res.push_back(pc[id]);
}
