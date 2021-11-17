//-----------------------------------------------------------------------------
// Created on: 05 July 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Julia Slyadneva
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

#pragma once

// glTF includes
#include <asiAsm_GlTFEntities.h>
#include <asiAsm_GlTFSceneStructure.h>
#include <asiAsm_GLTFPrimitive.h>

//-----------------------------------------------------------------------------

typedef NCollection_IndexedDataMap <asiAsm::xde::glTFNode*, NCollection_Vector<asiAsm::xde::glTFPrimitive>> t_Meshes2Primitives;

//-----------------------------------------------------------------------------

namespace asiAsm {
namespace xde {

//! This class is used to feed glTF writer with scene structure and definition of meshes.
//! This class is considered to be inherited when a custom scene structure is needed.
class glTFIDataSourceProvider : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(glTFIDataSourceProvider, Standard_Transient)

public:
  //! This method is called to prepare the scene tree and meshes definition.
  virtual 
    void Process(ActAPI_ProgressEntry progress = nullptr) = 0;

  //! Gets the scene tree.
  virtual
    const glTFSceneStructure& GetSceneStructure() const = 0;

  //! Gets the map of meshes nodes with the corresponding 3D definition also known as primitives.
  virtual
    const t_Meshes2Primitives& GetSceneMeshes() const = 0;
};
} // xde
} // asiAsm
