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

#ifndef asiAlgo_ConvertCanonical_h
#define asiAlgo_ConvertCanonical_h

// asiAlgo includes
#include <asiAlgo_ConvertCanonicalSummary.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

class ShapeBuild_ReShape;

//-----------------------------------------------------------------------------

//! Converts all freeform surfaces and curves of the given shape to
//! a canonical form.
class asiAlgo_ConvertCanonical : public ActAPI_IAlgorithm
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_ConvertCanonical, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_ConvertCanonical(ActAPI_ProgressEntry progress = nullptr,
                             ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Performs the conversion.
  //! \param[in] shape           the shape to convert.
  //! \param[in] tol             the recognition tolerance.
  //! \param[in] convertSurfaces the surface conversion mode (on/off).
  //! \param[in] convertCurves   the curve conversion mode (on/off).
  //! \return the converted shape.
  asiAlgo_EXPORT TopoDS_Shape
    Perform(const TopoDS_Shape& shape,
            const double        tol,
            const bool          convertSurfaces = true,
            const bool          convertCurves   = true);

public:

  //! \return the conversion summary.
  const asiAlgo_ConvertCanonicalSummary& GetSummary() const
  {
    return m_summary;
  }

protected:

  //! Applies a bunch of post-treatment fixes to the faces of the
  //! converted shape.
  void fixFaces(const TopoDS_Shape&         result,
                Handle(ShapeBuild_ReShape)& context,
                const double                tol);

  //! Applies a bunch of post-treatment fixes to the edges of the
  //! converted shape.
  void fixEdges(const TopoDS_Shape& result);

protected:

  //! Conversion summary.
  asiAlgo_ConvertCanonicalSummary m_summary;

};

#endif // asiAlgo_ConvertCanonical_h
