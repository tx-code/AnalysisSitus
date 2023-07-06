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

// Own include
#include <asiAlgo_FindBridge.h>

//-----------------------------------------------------------------------------

void asiAlgo_FindBridge::
  Init(const std::vector<std::vector<std::pair<int, int>>>& graph)
{
  m_graph = graph;
  m_timer = 0;
  m_tin.clear();
  m_fup.clear();
  m_used.clear();
  m_bridges.clear();
}

//-----------------------------------------------------------------------------

bool asiAlgo_FindBridge::Perform()
{
  findBridges();

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_FindBridge::dfs(const int vertex,
                             const int ancestor)
{
  m_used[vertex] = true;
  m_tin[vertex] = m_fup[vertex] = m_timer++;
  for (size_t i = 0; i < m_graph[vertex].size(); ++i)
  {
    int to = m_graph[vertex][i].first;
    if (to == ancestor)
    {
      continue;
    }

    if (m_used[to])
    {
      m_fup[vertex] = std::min(m_fup[vertex], m_tin[to]);
    }
    else
    {
      dfs(to, vertex);
      m_fup[vertex] = std::min(m_fup[vertex], m_fup[to]);
      if (m_fup[to] > m_tin[vertex])
      {
        if (isBridge(vertex, to))
        {
          bool isFound = false;
          std::vector<std::pair<int, int>>::const_iterator itB = m_bridges.cbegin();
          for (; itB != m_bridges.cend(); ++itB)
          {
            if ((*itB).first == vertex && (*itB).second == to ||
                (*itB).first == to && (*itB).second == vertex)
            {
              isFound = true;
              break;
            }
          }
          if (!isFound)
          {
            m_bridges.push_back(std::pair<int, int>(vertex, to));
          }
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_FindBridge::findBridges()
{
  m_timer = 0;
  for (int index = 0; index < (int)m_graph.size(); ++index)
  {
    m_used[index] = false;
  }
  for (int index = 0; index < (int)m_graph.size(); ++index)
  {
    if (!m_used[index])
    {
      dfs(index);
    }
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_FindBridge::isBridge(const int firstVertex,
                                  const int secondVertex)
{
  bool isBr = true;

  const std::vector<std::pair<int, int>>& nextFV = m_graph[firstVertex];
  std::vector<std::pair<int, int>>::const_iterator itNV = nextFV.cbegin();
  for (; itNV != nextFV.cend(); ++itNV)
  {
    if ((*itNV).first == secondVertex)
    {
      if ((*itNV).second > 1)
      {
        isBr = false;
      }

      break;
    }
  }

  if (isBr)
  {
    const std::vector<std::pair<int, int>>& nextSV = m_graph[secondVertex];
    itNV = nextSV.cbegin();
    for (; itNV != nextSV.cend(); ++itNV)
    {
      if ((*itNV).first == firstVertex)
      {
        if ((*itNV).second > 1)
        {
          isBr = false;
        }

        break;
      }
    }
  }

  return isBr;
}
