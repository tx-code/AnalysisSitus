//-----------------------------------------------------------------------------
// Created on: 22 June 2018
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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

#ifndef asiTest_CaseIDs_HeaderFile
#define asiTest_CaseIDs_HeaderFile

// Tests includes
#include <asiTest_CommonFacilities.h>

// asiTestEngine includes
#include <asiTestEngine.h>

//! IDs for Test Cases.
enum test_CaseID
{
  CaseID_InvertShells = 1,
  CaseID_KEV,
  CaseID_RebuildEdge,
  CaseID_RecognizeBlends,
  CaseID_SuppressBlends,
  CaseID_ConvertCanonical,
  CaseID_ConcatPCurves,

/* ------------------------------------------------------------------------ */

  CaseID_ChangeColor,
  CaseID_DataDictionary,
  CaseID_GenerateFacets,
  CaseID_Utils,

/* ------------------------------------------------------------------------ */

  CaseID_Exchange,
  CaseID_ExchangeAstra,
  CaseID_ExchangeMesh,
  CaseID_ExchangeShape,

/* ------------------------------------------------------------------------ */

  CaseID_AAG,
  CaseID_BuildQuickHull,
  CaseID_ComputeNegativeVolume,
  CaseID_IsContourClosed,
  CaseID_EdgeVexity,
  CaseID_VertexVexity,
  CaseID_FaceGrid,
  CaseID_RecognizeCavities,
  CaseID_RecognizeConvexHull,
  CaseID_RecognizeHoles,

/* ------------------------------------------------------------------------ */

  CaseID_XdeDoc,

/* ------------------------------------------------------------------------ */

  CaseID_AsciiStringParameter,
  CaseID_BaseModelPersistence,
  CaseID_BaseModelStructure,
  CaseID_BaseModelEvaluation,
  CaseID_BoolArrayParameter,
  CaseID_BoolParameter,
  CaseID_ComplexArrayParameter,
  CaseID_ComplexMatrixParameter,
  CaseID_CopyPasteEngine,
  CaseID_GroupParameter,
  CaseID_IntArrayParameter,
  CaseID_IntMatrixParameter,
  CaseID_IntParameter,
  CaseID_MeshAttrBean,
  CaseID_MeshAttrTransactional,
  CaseID_MeshAttrPersistent,
  CaseID_MeshParameter,
  CaseID_ExtTransactionEngine,
  CaseID_NameParameter,
  CaseID_ReferenceParameter,
  CaseID_ReferenceListParameter,
  CaseID_RealArrayParameter,
  CaseID_RealMatrixParameter,
  CaseID_RealParameter,
  CaseID_SelectionParameter,
  CaseID_ShapeArrayParameter,
  CaseID_ShapeParameter,
  CaseID_StringArrayParameter,
  CaseID_StringMatrixParameter,
  CaseID_TimeStamp,
  CaseID_TimeStampParameter,
  CaseID_TreeFunctionParameter,
  CaseID_TreeNodeParameter,
  CaseID_TrsfParameter,
  CaseID_TypeNameParameter,
  CaseID_CAFConversionCtx,
  CaseID_TriangulationParameter,

  CaseID_LAST

};

#endif
