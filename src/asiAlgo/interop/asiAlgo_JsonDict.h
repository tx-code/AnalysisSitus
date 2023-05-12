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
#define Ref_Ext "ref"

// Consts
#define PropVal_OS_Win                            "win"
#define PropVal_OS_Lin                            "lin"
#define PropVal_Type_Unrecognized                 "UNRECOGNIZED"
#define PropVal_Type_FlatShape                    "SHEET_METAL_FLAT"
#define PropVal_Type_FoldedSheetMetal             "SHEET_METAL_FOLDED"
#define PropVal_Type_RectTube                     "TUBE_RECTANGULAR"
#define PropVal_Type_CylTube                      "TUBE_ROUND"
#define PropVal_Type_Profile                      "PROFILE"
#define PropVal_Type_OtherTube                    "TUBE_OTHER"
#define PropVal_Type_CncMilling                   "CNC_MILLING"
#define PropVal_Type_CncLathe                     "CNC_LATHE"
#define PropVal_Type_CncLatheMilling              "CNC_LATHE_MILLING"
#define PropVal_Type_Unknown                      "UNKNOWN"

// Common props
#define PropName_Sdk                              "sdk"
#define PropName_SdkVersion                       "sdkVersion"
#define PropName_ProcessingTime                   "processingTime"
#define PropName_UnitScaleFactor                  "unitScaleFactor"
#define PropName_OriginalUnitString               "originalUnitString"
#define PropName_Bad                              "bad"
#define PropName_Ignore                           "ignore"
#define PropName_File                             "file"
#define PropName_Parts                            "parts"
#define PropName_Id                               "id"
#define PropName_Name                             "name"
#define PropName_Quantity                         "numOccurrences"
#define PropName_Bodies                           "bodies"
#define PropName_Type                             "type"
#define PropName_Volume                           "volume"

// JSON properties
#define asiPropName_ExtrasCanRecSummary            "canrecSummary"
#define asiPropName_ExtrasCanRecSurfBezier         "nbSurfBezier"
#define asiPropName_ExtrasCanRecSurfSpl            "nbSurfSpl"
#define asiPropName_ExtrasCanRecSurfConical        "nbSurfConical"
#define asiPropName_ExtrasCanRecSurfCyl            "nbSurfCyl"
#define asiPropName_ExtrasCanRecSurfOffset         "nbSurfOffset"
#define asiPropName_ExtrasCanRecSurfSph            "nbSurfSph"
#define asiPropName_ExtrasCanRecSurfLinExtr        "nbSurfLinExtr"
#define asiPropName_ExtrasCanRecSurfOfRevol        "nbSurfOfRevol"
#define asiPropName_ExtrasCanRecSurfToroidal       "nbSurfToroidal"
#define asiPropName_ExtrasCanRecSurfPlane          "nbSurfPlane"
#define asiPropName_ExtrasCanRecCurveBezier        "nbCurveBezier"
#define asiPropName_ExtrasCanRecCurveSpline        "nbCurveSpline"
#define asiPropName_ExtrasCanRecCurveCircle        "nbCurveCircle"
#define asiPropName_ExtrasCanRecCurveEllipse       "nbCurveEllipse"
#define asiPropName_ExtrasCanRecCurveHyperbola     "nbCurveHyperbola"
#define asiPropName_ExtrasCanRecCurveLine          "nbCurveLine"
#define asiPropName_ExtrasCanRecCurveOffset        "nbCurveOffset"
#define asiPropName_ExtrasCanRecCurveParabola      "nbCurveParabola"
#define asiPropName_ExtrasCanRecIsValid            "isValidAfterConversion"

#define asiScene_Name                              "graph"
#define asiScene_RootsIds                          "roots"
#define asiScene_AssembliesName                    "assemblies"
#define asiScene_AssembliesAssemblyName            "assemblyName"
#define asiScene_AssembliesAssemblyId              "assemblyId"
#define asiScene_AssembliesAssemblyChildren        "children"

#define asiScene_InstancesName                     "instances"
#define asiScene_InstancesInstanceName             "instanceName"
#define asiScene_InstancesInstanceId               "instanceId"
#define asiScene_InstancesInstanceRef              "instanceRef"
#define asiScene_InstancesAssemblyItemId           "assemblyItemId"
#define asiScene_InstancesRotation                 "rotation"
#define asiScene_InstancesTranslation              "translation"

#define asiScene_PartsName                         "parts"
#define asiScene_PartsPartName                     "partName"
#define asiScene_PartsPartId                       "partId"
#define asiScene_PartsPersistentId                 "persistentId"

#endif
