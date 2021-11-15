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
#include <asiAsm_GLTFIDataSourceProvider.h>

// STL includes
#include <memory>
#include <map>

#include <TColStd_MapOfAsciiString.hxx>

//-----------------------------------------------------------------------------

// Forward declarations from the global namespace.
class TDocStd_Document;
class XCAFDoc_ShapeTool;
class TopoDS_Edge;

//-----------------------------------------------------------------------------

namespace asiAsm {
namespace xde {

//! This class is forked from the open-source OpenCascade kernel
//! to serve as a glTF writer for colorized CAD parts and assemblies.
//! It mimics the data structure the same as XDE in the format appropriate for glTF writer.
//! All B-rep shapes in the XDE document should have undergone meshing to get facets for export.
//! It represents each B-rep shape by a set of its faces where each face is a "primitive" in terms of glTF format.
//! If B-rep shape has sub-shapes of "edge" type in XDE document, then these edges will be output as primitives as well.
//! 
//! The output structure is following:
//!    SOLID       <------ B-rep solid
//!      Face_1    <------ mesh primitive, has the color assigned to a source TopoDS_Face
//!      Face_2
//!      ...
//!      Edge_1    <------ mesh primitive, presents if a colored sub-shape exists
//!      Edge_2
//!      ...
class glTFXdeDataSourceProvider : public glTFIDataSourceProvider
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(glTFXdeDataSourceProvider, glTFIDataSourceProvider)

  typedef NCollection_DataMap<TopoDS_Shape, glTFXdeVisualStyle, TopTools_ShapeMapHasher> t_Shape2Style;
  typedef NCollection_IndexedDataMap<glTFNode*, TDF_Label>                               t_Node2Label;

public:

  //! Ctor.
  gltf_EXPORT
    glTFXdeDataSourceProvider(const Handle(TDocStd_Document)& doc,
                              const TColStd_MapOfAsciiString& filter = TColStd_MapOfAsciiString());

  //! Dtor.
  gltf_EXPORT virtual
    ~glTFXdeDataSourceProvider();

public:

  //! Examines the document to prepare the needed data for glTF writer.
  gltf_EXPORT void Process(ActAPI_ProgressEntry progress = nullptr);

public:

  virtual
    const glTFSceneStructure& GetSceneStructure() const { return m_sceneStructure; }

  virtual
    const t_Meshes2Primitives& GetSceneMeshes() const { return m_meshes; }

protected:

  gltf_EXPORT virtual
    void createSceneStructure(t_Node2Label&        solids,
                              ActAPI_ProgressEntry progress = nullptr);

  gltf_EXPORT virtual
    void processSceneMeshes(t_Node2Label&          solids,
                            ActAPI_ProgressEntry   progress = nullptr);

  gltf_EXPORT 
    bool processFacePrimitive(const TopoDS_Face&    face,
                              glTFPrimitive& facePrimitive);

  gltf_EXPORT
  bool processEdgePrimitive(const TopoDS_Edge&      edge,
                            const t_Shape2Style&    styles,
                            glTFPrimitive&   edgePrimitive);

  //! Reads styles from OCAF document to internal map. As a result, each face
  //! gets an associated style.
  gltf_EXPORT
    void readStyles(const TDF_Label&                label,
                    t_Shape2Style&                  shapeStyles);

protected:

  Handle(TDocStd_Document)  m_doc;
  TColStd_MapOfAsciiString  m_filter;

  glTFSceneStructure        m_sceneStructure;
  t_Meshes2Primitives       m_meshes;
};
} // xde
} // asiAsm

