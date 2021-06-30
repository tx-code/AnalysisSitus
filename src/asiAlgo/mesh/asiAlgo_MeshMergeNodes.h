//-----------------------------------------------------------------------------
// Created on: 30 June 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Alexander Malyshev
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

#ifndef asiAlgo_MeshMergeNodes_h
#define asiAlgo_MeshMergeNodes_h

// asiAlgo includes
#include <asiAlgo.h>

// OCCT includes
#include <gp_XY.hxx>
#include <gp_XYZ.hxx>
#include <gp_Pnt.hxx>
#include <NCollection_CellFilter.hxx>
#include <NCollection_IndexedDataMap.hxx>
#include <Precision.hxx>
#include <TColStd_ListOfInteger.hxx>

// STL includes
#include <vector>

//! Merges coincident mesh nodes.
class asiAlgo_MeshMergeNodes
{
public:

  asiAlgo_EXPORT
    asiAlgo_MeshMergeNodes(const double precision = Precision::Confusion());

public:

  asiAlgo_EXPORT void
    Clear();

  asiAlgo_EXPORT void
    Perform(const std::vector<gp_Pnt>& data);

  asiAlgo_EXPORT bool
    Perform(const gp_Pnt& pnt,
            int&          resIdx);

public:

  const std::vector<gp_Pnt>& GetResultingArray() const
  { return m_resPoints; }

  int GetResultingNode(const int origNode) const
  { return m_inpOutMap[origNode - 1]; }

  const std::vector<int>& GetMergingMap() const
  { return m_inpOutMap; }

protected:

  int GetNodeId(const gp_XYZ& pnt);

private:

  class NodesMerger_VertexInspector : public NCollection_CellFilter_InspectorXYZ
  {
  public:
    typedef int Target;
    //! Constructor; remembers the tolerance
    NodesMerger_VertexInspector(const double tol)
    : mySqTol(tol*tol)
    {}

    //! Clear the list of adjacent points
    void ClearResList()
    { myResInd.Clear(); }

    //! Set current point to search for coincidence
    void SetPntVec(std::vector<gp_Pnt>* pntVec)
    { myPoints = pntVec; }

    //! Set current point to search for coincidence
    void SetCurrent(const gp_XYZ& curPnt)
    { myCurrent = curPnt; }

    //! Get list of indexes of points adjacent with the current
    const TColStd_ListOfInteger& ResInd() const
    { return myResInd; }

    //! Implementation of inspection method
    NCollection_CellFilter_Action Inspect(const int target)
    {
      const gp_Pnt& aPnt = (*myPoints)[target - 1];
      if (aPnt.SquareDistance (gp_Pnt (myCurrent)) <= mySqTol)
        myResInd.Append (target);

      return CellFilter_Keep;
    }

  private:
    double                mySqTol;
    TColStd_ListOfInteger myResInd;
    std::vector<gp_Pnt>*  myPoints;
    gp_XYZ                myCurrent;
  };

  //! Resulting nodes.
  std::vector<gp_Pnt> m_resPoints;

  //! Merging tolerance.
  double m_mergingTol;

  //! Filter.
  NCollection_CellFilter<NodesMerger_VertexInspector> m_filter;

  //! Is new point.
  bool m_isNew;

  //! Vector storing resulting indexes.
  std::vector<int> m_inpOutMap;
};

#endif
