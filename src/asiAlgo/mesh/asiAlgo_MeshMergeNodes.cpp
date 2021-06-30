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

// Own include
#include <asiAlgo_MeshMergeNodes.h>

//-----------------------------------------------------------------------------

int asiAlgo_MeshMergeNodes::GetNodeId(const gp_XYZ& theP)
{
  int index = -1;

  //! Inspector.
  NodesMerger_VertexInspector anInspector(m_mergingTol);
  anInspector.SetPntVec(&m_resPoints);
  anInspector.SetCurrent(theP);

  gp_XYZ minp = anInspector.Shift(theP, -m_mergingTol);
  gp_XYZ maxp = anInspector.Shift(theP, +m_mergingTol);
  m_filter.Inspect(minp, maxp, anInspector);
  const TColStd_ListOfInteger& indices = anInspector.ResInd();
  if (indices.IsEmpty() == false)
  {
    // Node is found.
    index = indices.First();
    m_isNew = false;
  }
  else
  {
    // Node is not found - add it to the filter.
    m_resPoints.push_back(theP);
    index = (int)m_resPoints.size();
    m_filter.Add(index, theP); // Add point to filter with correct index.
    m_isNew = true;
  }

  return index;
}

//-----------------------------------------------------------------------------

asiAlgo_MeshMergeNodes::asiAlgo_MeshMergeNodes(const double precision)
{
  m_mergingTol = precision;
  m_filter.Reset(m_mergingTol);
  m_isNew = false;
}

//-----------------------------------------------------------------------------

void asiAlgo_MeshMergeNodes::Clear()
{
  m_resPoints.clear();
  m_inpOutMap.clear();
  m_filter.Reset(m_mergingTol);
  m_isNew = false;
}

//-----------------------------------------------------------------------------

void asiAlgo_MeshMergeNodes::Perform(const std::vector<gp_Pnt>& data)
{
  Clear();
  m_inpOutMap.resize(data.size());

  for (int i = 0; i < (int) data.size(); ++i)
  {
    const gp_Pnt& aP = data[i];
    int aNewNodeId = GetNodeId(aP.XYZ());
    m_inpOutMap[i] = aNewNodeId;
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_MeshMergeNodes::Perform(const gp_Pnt& pnt,
                                     int&          resIdx)
{
  int aNewNodeId = GetNodeId(pnt.XYZ());
  m_inpOutMap.push_back(aNewNodeId);
  resIdx = aNewNodeId;

  return m_isNew;
}
