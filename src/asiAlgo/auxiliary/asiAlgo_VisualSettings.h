//-----------------------------------------------------------------------------
// Created on: 09 December 2021
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

#ifndef asiAlgo_VisualSettings_h
#define asiAlgo_VisualSettings_h

// asiAlgo includes
#include <asiAlgo.h>

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
//! Settings for the visual appearance of parts/assemblies when making snapshots.
struct asiAlgo_VisualSettings
{
  float  LineWidth;
  int    Color;
  bool   HasBackside;
  int    DisplayMode;
  float  Ambient;
  float  Diffuse;
  float  Specular;
  int    SpecularPower;

  //! Default ctor.
  asiAlgo_VisualSettings()
  //
  : LineWidth     (1.5f),
    HasBackside   (false),
    DisplayMode   (0x20),
    Ambient       (0.65f),
    Diffuse       (0.5f),
    Specular      (0.9f),
    SpecularPower (20)
  {
    Color = 255 << 16 | 255 << 8 | 255;
  }
};

#endif
