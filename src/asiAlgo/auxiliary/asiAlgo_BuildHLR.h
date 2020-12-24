//-----------------------------------------------------------------------------
// Created on: 24 December 2020
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

#ifndef asiAlgo_BuildHLR_h
#define asiAlgo_BuildHLR_h

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

//-----------------------------------------------------------------------------

//! Performs hidden line removal for the input shape. The result is returned
//! as a compound of edges representing the extracted feature lines.
class asiAlgo_BuildHLR : public ActAPI_IAlgorithm
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_BuildHLR, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] shape    B-rep shape of a CAD part to analyze.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_BuildHLR(const TopoDS_Shape&  shape,
                     ActAPI_ProgressEntry progress = nullptr,
                     ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Performs HLR.
  //! \param[in] projectionDir the direction of projection to use.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const gp_Dir& projectionDir);

  //! \return the extracted feature lines.
  asiAlgo_EXPORT const TopoDS_Shape&
    GetResult() const;

protected:

  //! Runs precise HLR.
  //! \param[in] projectionDir the direction of projection to use.
  asiAlgo_EXPORT bool
    performPrecise(const gp_Dir& projectionDir);

  //! Build 3Ds curves out of the 2D curves constructed by HLR.
  //! \param[in] shape the input shape.
  //! \return the shape with reconstructed 3D curves.
  asiAlgo_EXPORT const TopoDS_Shape&
    build3dCurves(const TopoDS_Shape& shape);

protected:

  TopoDS_Shape m_input;  //!< Input shape.
  TopoDS_Shape m_result; //!< Result shape.

};

#endif
