//-----------------------------------------------------------------------------
// Created on: 07 March 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Kiselev
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

#ifndef asiAlgo_WriteSVG_h
#define asiAlgo_WriteSVG_h

// asiAlgo includes
#include <asiAlgo_Optional.h>

// Active Data includes
#include <ActAPI_IProgressNotifier.h>
#include <ActAPI_IPlotter.h>

// OCCT includes
#include <TCollection_AsciiString.hxx>
#include <TopTools_HSequenceOfShape.hxx>

//! \ingroup ASI_INTEROP
//!
//! Services to save a drawing as an SVG file.
namespace asiAlgo_WriteSVG
{
  //! Auxiliary structure to control drawing settings. 
  struct t_drawingStyle
  {
    int    CanvasPadding       = 25;
    double PaddingScaleCoeff   = 0.01f;  //!< Coeff controlling a ratio between padding and canvas size.
    double LineWidthScaleCoeff = 0.3f;   //!< Coeff controlling a line width which is defined as a ratio between image and canvas sizes
    double DiscrCurveLinDefl   = 0.001f;
    double DiscrCurveAngDefl   = (float)(0.5 * M_PI / 180.0);

    tl::optional<double> CanvasMaxDim; //!< Max canvas dimension (optional).
  };

  //! Saves the passed data to SVG after preprocessing it with HLR algorithm.
  //! \param[in] shape   the shape to dump to SVG.
  //! \param[in] dir     the projection direction.
  //! \param[in] path    the SVG path to save to.
  //! \param[in] tol     the discretization tolerance for edges.
  //! \param[in] style   the drawing settings.
  //! \param[in] plotter imperative plotter.
  //! \return true if SVG file was saved, false -- otherwise.
  asiAlgo_EXPORT bool
    WriteWithHLR(const TopoDS_Shape&            shape,
                 const gp_Dir&                  dir,
                 const TCollection_AsciiString& path,
                 const double                   tol,
                 const t_drawingStyle&          style    = t_drawingStyle(),
                 ActAPI_ProgressEntry           progress = nullptr,
                 ActAPI_PlotterEntry            plotter  = nullptr);

  //! Saves the passed data to SVG.
  //! The given shape is expected to be planar and located in XOY plane. If not,
  //! make sure to relocate the drawing to XOY before using this function.
  //!
  //! \param[in] shape   the shape to dump to SVG.
  //! \param[in] path    the SVG path to save to.
  //! \param[in] tol     the discretization tolerance for edges.
  //! \param[in] style   the drawing settings.
  //! \param[in] plotter imperative plotter.
  //! \return true if SVG file was saved, false -- otherwise.
  asiAlgo_EXPORT bool
    Write(const TopoDS_Shape&            shape,
          const TCollection_AsciiString& path,
          const double                   tol,
          const t_drawingStyle&          style   = t_drawingStyle(),
          ActAPI_PlotterEntry            plotter = nullptr);

}

#endif // asiAlgo_WriteSVG_h
