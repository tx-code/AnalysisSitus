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

#ifndef cmdMobius_Mesh_h
#define cmdMobius_Mesh_h

// cmdMobius includes
#include <cmdMobius.h>

#ifdef USE_MOBIUS
  #include <mobius/poly_Mesh.h>
#endif

// asiTcl includes
#include <asiTcl_Variable.h>

//-----------------------------------------------------------------------------

//! Mesh model in a Tcl session.
class cmdMobius_Mesh : public asiTcl_Variable
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(cmdMobius_Mesh, asiTcl_Variable)

public:

#if defined USE_MOBIUS

  //! Ctor.
  //! \param[in] mesh the mesh to set.
  cmdMobius_EXPORT
    cmdMobius_Mesh(const mobius::t_ptr<mobius::t_mesh>& mesh = nullptr);

#endif

  //! Dtor.
  cmdMobius_EXPORT virtual
    ~cmdMobius_Mesh();

#if defined USE_MOBIUS

public:

  //! Sets the mesh to store.
  //! \param[in] mesh the mesh to set.
  cmdMobius_EXPORT void
    SetMesh(const mobius::t_ptr<mobius::t_mesh>& mesh);

  //! \return the owned mesh.
  cmdMobius_EXPORT const mobius::t_ptr<mobius::t_mesh>&
    GetMesh() const;

#endif

public:

  //! \return brief description "what is" this object.
  cmdMobius_EXPORT virtual std::string
    WhatIs() const;

  //! Dumps this variable to the passed output stream.
  //! \param[in,out] out the output stream.
  cmdMobius_EXPORT virtual void
    Dump(std::ostream& out) const;

#if defined USE_MOBIUS

protected:

  mobius::t_ptr<mobius::t_mesh> m_mesh; //!< Owned mesh.

#endif

};

#endif
