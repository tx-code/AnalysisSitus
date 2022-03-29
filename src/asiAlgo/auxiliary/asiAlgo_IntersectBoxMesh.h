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

#pragma once

// asiAlgo includes
#include <asiAlgo_RTCD.h>

// asiAlgo includes
#include <asiAlgo_BVHFacets.h>

struct t_interInfo;

//-----------------------------------------------------------------------------

//! Finds collisions between AABBs and meshes.
//!
//! The algorithm is based on the paper
//! [Akenine-Mollser, T. (2001). Fast 3D Triangle-Box Overlap Testing.
//!  Journal of Graphics Tools, 6(1), 29–33. https://doi.org/10.1080/10867651.2001.10487535]
class asiAlgo_IntersectBoxMesh
{
public:

  //! Creates new BVH-based intersection detector.
  asiAlgo_IntersectBoxMesh();

  //! Creates new BVH-based intersection detector.
  asiAlgo_IntersectBoxMesh(const Handle(asiAlgo_BVHFacets)& bvh);

public:

  void SetBVH(const Handle(asiAlgo_BVHFacets)& bvh)
  {
    m_bvh = bvh;
  }

  //! Performs intersection check for a given box.
  bool Perform(const RTCD::AABB& box);

protected:

  bool intersectLeaves(const BVH_Vec4i& leaf,
                       t_interInfo&     box);

protected:

  //! BVH.
  Handle(asiAlgo_BVHFacets) m_bvh;

private:

  asiAlgo_IntersectBoxMesh(const asiAlgo_IntersectBoxMesh&) = delete;
  asiAlgo_IntersectBoxMesh& operator=(const asiAlgo_IntersectBoxMesh&) = delete;

};
