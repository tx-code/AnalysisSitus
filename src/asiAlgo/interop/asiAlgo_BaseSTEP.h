//-----------------------------------------------------------------------------
// Created on: 24 March 2022
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

#ifndef asiAlgo_BaseSTEP_h
#define asiAlgo_BaseSTEP_h

// asiAlgo includes
#include <asiAlgo_InteropVars.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

class STEPControl_Writer;

//-----------------------------------------------------------------------------

//! \ingroup ASI_INTEROP
//!
//! Base class for STEP interoperability interfaces.
class asiAlgo_BaseSTEP : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_BaseSTEP, ActAPI_IAlgorithm)

public:

  //! Ctor accepting progress notifier and imperative plotter.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_BaseSTEP(ActAPI_ProgressEntry progress,
                   ActAPI_PlotterEntry  plotter = nullptr)
  //
  : ActAPI_IAlgorithm(progress, plotter), m_fLengthScale(1.0) {}

public:

  //! \return the unit string from the STEP file.
  const TCollection_AsciiString& GetUnitString() const
  {
    return m_unitString;
  }

  //! \return length scale.
  double GetLengthScale() const
  {
    return m_fLengthScale;
  }

protected:

  //! Converts the passed unit string to the scaling coefficient.
  //! \param[in] unitStr the unit string.
  asiAlgo_EXPORT double
    fromSiName(const TCollection_AsciiString& unitStr) const;

protected:

  TCollection_AsciiString m_unitString;
  double                  m_fLengthScale;

};

#endif
