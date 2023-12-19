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

#ifndef asiAlgo_JsonDict_HeaderFile
#define asiAlgo_JsonDict_HeaderFile

// Filename extension.
#define asiRef_Ext "ref"

// Consts
#define asiPropVal_OS_Win                                 "win"
#define asiPropVal_OS_Lin                                 "lin"
#define asiPropVal_Type_Unrecognized                      "UNRECOGNIZED"
#define asiPropVal_Type_FlatShape                         "SHEET_METAL_FLAT"
#define asiPropVal_Type_FoldedSheetMetal                  "SHEET_METAL_FOLDED"
#define asiPropVal_Type_RectTube                          "TUBE_RECTANGULAR"
#define asiPropVal_Type_CylTube                           "TUBE_ROUND"
#define asiPropVal_Type_Profile                           "PROFILE"
#define asiPropVal_Type_OtherTube                         "TUBE_OTHER"
#define asiPropVal_Type_CncMilling                        "CNC_MILLING"
#define asiPropVal_Type_CncLathe                          "CNC_LATHE"
#define asiPropVal_Type_CncLatheMilling                   "CNC_LATHE_MILLING"
#define asiPropVal_Type_Unknown                           "UNKNOWN"

// Common props
#define asiPropName_Sdk                                   "sdk"
#define asiPropName_SdkVersion                            "sdkVersion"
#define asiPropName_SdkHash                               "sdkHash"
#define asiPropName_ProcessingTime                        "processingTime"
#define asiPropName_UnitScaleFactor                       "unitScaleFactor"
#define asiPropName_OriginalUnitString                    "originalUnitString"
#define asiPropName_Bad                                   "bad"
#define asiPropName_Ignore                                "ignore"
#define asiPropName_File                                  "file"
#define asiPropName_Parts                                 "parts"
#define asiPropName_Id                                    "id"
#define asiPropName_Name                                  "name"
#define asiPropName_Quantity                              "numOccurrences"
#define asiPropName_Bodies                                "bodies"
#define asiPropName_Type                                  "type"
#define asiPropName_Volume                                "volume"
#define asiPropName_CuttingLength                         "cuttingLength"
#define asiPropName_NextSegment                           "nextSegment"
#define asiPropName_AngleToNextSegment                    "angleToNextSegment"
#define asiPropName_Radius                                "radius"
#define asiPropName_Angle                                 "angle"
#define asiPropName_Segments                              "segments"
#define asiPropName_FaceIds                               "faceIds"

// JSON properties
#define asiPropName_ExtrasCanRecSummary                   "canrecSummary"
#define asiPropName_ExtrasCanRecSurfBezier                "nbSurfBezier"
#define asiPropName_ExtrasCanRecSurfSpl                   "nbSurfSpl"
#define asiPropName_ExtrasCanRecSurfConical               "nbSurfConical"
#define asiPropName_ExtrasCanRecSurfCyl                   "nbSurfCyl"
#define asiPropName_ExtrasCanRecSurfOffset                "nbSurfOffset"
#define asiPropName_ExtrasCanRecSurfSph                   "nbSurfSph"
#define asiPropName_ExtrasCanRecSurfLinExtr               "nbSurfLinExtr"
#define asiPropName_ExtrasCanRecSurfOfRevol               "nbSurfOfRevol"
#define asiPropName_ExtrasCanRecSurfToroidal              "nbSurfToroidal"
#define asiPropName_ExtrasCanRecSurfPlane                 "nbSurfPlane"
#define asiPropName_ExtrasCanRecCurveBezier               "nbCurveBezier"
#define asiPropName_ExtrasCanRecCurveSpline               "nbCurveSpline"
#define asiPropName_ExtrasCanRecCurveCircle               "nbCurveCircle"
#define asiPropName_ExtrasCanRecCurveEllipse              "nbCurveEllipse"
#define asiPropName_ExtrasCanRecCurveHyperbola            "nbCurveHyperbola"
#define asiPropName_ExtrasCanRecCurveLine                 "nbCurveLine"
#define asiPropName_ExtrasCanRecCurveOffset               "nbCurveOffset"
#define asiPropName_ExtrasCanRecCurveParabola             "nbCurveParabola"
#define asiPropName_ExtrasCanRecIsValid                   "isValidAfterConversion"

// Scene tree
#define asiPropName_SceneTree                             "sceneTree"
#define asiPropName_SceneRootsIds                         "roots"
#define asiPropName_ScenePrototypes                       "prototypes"
#define asiPropName_SceneChildId                          "id"
#define asiPropName_SceneChildName                        "name"
#define asiPropName_SceneAssembliesName                   "assemblies"
#define asiPropName_SceneAssembliesAssemblyChildInstances "childInstances"
#define asiPropName_SceneInstancesName                    "instances"
#define asiPropName_SceneInstancesInstancePrototype       "prototype"
#define asiPropName_SceneInstancesAssemblyItemId          "assemblyItemId"
#define asiPropName_SceneInstancesRotation                "rotation"
#define asiPropName_SceneInstancesTranslation             "translation"
#define asiPropName_ScenePartsName                        "parts"
#define asiPropName_ScenePartsPersistentId                "persistentId"
#define asiPropName_ScenePartsRepresentation              "shape"

#endif
