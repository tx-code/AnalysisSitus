//-----------------------------------------------------------------------------
// Created on: 06 July 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Andrey Voevodin
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

#ifndef asiAlgo_FindBridge_h
#define asiAlgo_FindBridge_h

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// STL includes
#include <map>
#include <vector>

//! \brief Find bridges in graph.
class asiAlgo_FindBridge : public ActAPI_IAlgorithm
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_FindBridge, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] notifier progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_FindBridge(ActAPI_ProgressEntry notifier = nullptr,
                     ActAPI_PlotterEntry  plotter  = nullptr)
  : ActAPI_IAlgorithm(notifier, plotter)
  {}

public:

  // <vID, <vID, multiplicity>>
  asiAlgo_EXPORT void
    Init(const std::vector<std::vector<std::pair<int, int>>>& graph);

  //! Perform.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform();

  const std::vector<std::pair<int, int>>& GetBridges()
  {
    return m_bridges;
  }

private:

  void dfs(const int vertex,
           const int ancestor = -1);

  void findBridges();

  bool isBridge(const int firstVertex,
                const int secondVertex);

private:

  std::vector<std::vector<std::pair<int, int>>> m_graph;
  int                                           m_timer;
  std::map<int, int>                            m_tin;
  std::map<int, int>                            m_fup;
  std::map<int, bool>                           m_used;
  std::vector<std::pair<int, int>>              m_bridges;

};

#endif
