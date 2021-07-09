//-----------------------------------------------------------------------------
// Created on: 03 July 2021
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

// Own include
#include <cmdMobius_Mesh.h>

#if defined USE_MOBIUS

using namespace mobius;

//-----------------------------------------------------------------------------

cmdMobius_Mesh::cmdMobius_Mesh(const t_ptr<poly_Mesh>& mesh)
: asiTcl_Variable()
{
  this->SetMesh(mesh);
}

#endif

//-----------------------------------------------------------------------------

cmdMobius_Mesh::~cmdMobius_Mesh()
{
}

#if defined USE_MOBIUS

//-----------------------------------------------------------------------------

void cmdMobius_Mesh::SetMesh(const t_ptr<poly_Mesh>& mesh)
{
  m_mesh = mesh;
}

//-----------------------------------------------------------------------------

const t_ptr<poly_Mesh>& cmdMobius_Mesh::GetMesh() const
{
  return m_mesh;
}

#endif

//-----------------------------------------------------------------------------

std::string cmdMobius_Mesh::WhatIs() const
{
  return "Mesh";
}

//-----------------------------------------------------------------------------

void cmdMobius_Mesh::Dump(std::ostream& out) const
{
  (void) out; // Do nothing.
}
