//-----------------------------------------------------------------------------
// Created on: 30 August 2022
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

#ifndef asiVisu_AxesDataProvider_h
#define asiVisu_AxesDataProvider_h

// asiVisu includes
#include <asiVisu_DataProvider.h>

//! Data provider for axes.
class asiVisu_AxesDataProvider : public asiVisu_DataProvider
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_AxesDataProvider, asiVisu_DataProvider)

public:

  asiVisu_EXPORT
    asiVisu_AxesDataProvider(const Handle(ActAPI_INode)& N);

public:

  asiVisu_EXPORT virtual ActAPI_DataObjectId
    GetNodeID() const;

public:

  virtual gp_Pnt
    GetOrigin() = 0;

  virtual gp_Dir
    GetDX() = 0;

  virtual gp_Dir
    GetDY() = 0;

  virtual gp_Dir
    GetDZ() = 0;

  virtual bool
    HasOrientationTip() const = 0;

  virtual double
    GetScaleCoeff() const = 0;

protected:

  Handle(ActAPI_INode) m_source; //!< Source Node.

};

#endif
