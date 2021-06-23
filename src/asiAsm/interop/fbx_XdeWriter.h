//-----------------------------------------------------------------------------
// Created on: 06 March 2021
// Created by: Sergey SLYADNEV
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

#ifndef fbx_XdeWriter_h
#define fbx_XdeWriter_h

// asiAsm includes
#include <asiAsm.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <Poly_Triangulation.hxx>
#include <TCollection_AsciiString.hxx>
#include <TDF_Label.hxx>

// STL include
#include <map>

//-----------------------------------------------------------------------------

namespace asiAsm {
namespace xde {

struct t_fbxState;
class Doc;

//! Writes the passed XDE document to FBX format.
class fbxWriter: public ActAPI_IAlgorithm
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(fbxWriter, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] filename the full path to the file to write.
  //! \param[in] notifier the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAsm_EXPORT
    fbxWriter(const TCollection_AsciiString& filename,
              ActAPI_ProgressEntry           notifier = nullptr,
              ActAPI_PlotterEntry            plotter  = nullptr);

  //! Dtor.
  asiAsm_EXPORT virtual
    ~fbxWriter();

public:

  //! Saves model to file.
  //! \param[in] doc the XDE document to save.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT bool
    Perform(const Handle(Doc)& doc);

protected:

  asiAsm_EXPORT void
    clearState();

protected:

  TCollection_AsciiString m_filename;  //!< Target filename.
  t_fbxState*             m_pFbxState; //!< State of FBX SDK.

};

} // xde
} // asiAsm

#endif
