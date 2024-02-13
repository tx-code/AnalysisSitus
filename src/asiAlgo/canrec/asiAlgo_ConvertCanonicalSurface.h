//-----------------------------------------------------------------------------
// Created on: 29 October 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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

#ifndef asiAlgo_ConvertCanonicalSurface_HeaderFile
#define asiAlgo_ConvertCanonicalSurface_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <Geom_Surface.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Utility to recognize B-spline and Bezier surfaces as canonical surfaces,
//! such as planes, cylinders, cones, spheres, toruses.
class asiAlgo_ConvertCanonicalSurface
{
public:

  //! Ctor accepting the surface to convert.
  //! \param[in] S the surface to convert.
  asiAlgo_EXPORT
    asiAlgo_ConvertCanonicalSurface(const Handle(Geom_Surface)& S);

public:

  //! \return the max deviation of the converted surface
  //!         from the original surface.
  asiAlgo_EXPORT double
    GetFitError() const;

  //! Performs conversion.
  asiAlgo_EXPORT Handle(Geom_Surface)
    Perform(const double tol);

private:

  Handle(Geom_Surface) m_surf; //!< Surface to convert.
  double               m_fGap; //!< Fitting error.

};

#endif
