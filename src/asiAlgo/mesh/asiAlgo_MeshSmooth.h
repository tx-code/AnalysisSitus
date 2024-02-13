//-----------------------------------------------------------------------------
// Created on: 30 June 2021
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

#ifndef asiAlgo_MeshSmooth_h
#define asiAlgo_MeshSmooth_h

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IProgressNotifier.h>

// OpenCascade includes
#include <Poly_Triangulation.hxx>

//! \ingroup ASI_MODELING
//!
//! Smoothing services.
namespace asiAlgo_MeshSmooth
{
#ifdef USE_VTK
  //! Runs VTK Laplacian smoothing algorithm.
  //! \param[in]  source   mesh to smooth.
  //! \param[in]  nbIter   number of iterations.
  //! \param[out] result   smoothed mesh.
  //! \param[in]  notifier progress notifier.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    DoVTK(const Handle(Poly_Triangulation)& source,
          const int                         nbIter,
          Handle(Poly_Triangulation)&       result,
          ActAPI_ProgressEntry              notifier = NULL);
#endif
};

#endif
