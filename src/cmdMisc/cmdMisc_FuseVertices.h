//-----------------------------------------------------------------------------
// Created on: 12 December 2020
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

#ifndef cmdMisc_FuseVertices_h
#define cmdMisc_FuseVertices_h

// cmdMisc includes
#include <cmdMisc.h>

//! Fuses vertices with the given compound of edges.
class cmdMisc_FuseVertices
{
public:

  //! Ctor with optional tolerance.
  cmdMisc_EXPORT
    cmdMisc_FuseVertices(const double tol = Precision::Confusion());

public:

  //! Merges vertices.
  //! \param[in] shape the input shape.
  //! \return the result of fuse.
  cmdMisc_EXPORT TopoDS_Shape
    operator()(const TopoDS_Shape& shape) const;

public:

  //! Sets tolerance for merging verticess.
  //! \param[in] tol tolerance to merge verticess.
  void SetTolerance(const double tol)
  {
    m_fToler = tol;
  }

  //! \return used tolerance.
  double GetTolerance() const
  {
    return m_fToler;
  }

protected:

  double m_fToler; //!< Fusing tolerance.

};

#endif
