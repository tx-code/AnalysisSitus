//-----------------------------------------------------------------------------
// Created on: 21 November 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Natalia Ermolaeva
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

#ifndef asiUI_Inspector_HeaderFile
#define asiUI_Inspector_HeaderFile

// Own includes
#include <asiAlgo.h>

// OCCT includes
#include <NCollection_List.hxx>

//! Class to show/hide OCCT inspector.
//! It provides plugins:
//! - DFBrowser   - OCAF structure
//! - ShapeView   - TopoDS_Shape structure,
//! - VIspector   - AIS_InteractiveContext content,
//! - MessageView - to prepare tree report with debug messages.
class asiUI_Inspector
{
public:
  //! Shows inspector.
  asiAlgo_EXPORT static void showInspector(const NCollection_List<Handle(Standard_Transient)>& parameters = 
                                           NCollection_List<Handle(Standard_Transient)>());

  //! Hides inspector
  asiAlgo_EXPORT static void hideInspector();

  //! Updates content of the inspector.
  asiAlgo_EXPORT static void updateInspector();
};

#endif