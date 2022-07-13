//-----------------------------------------------------------------------------
// Created on: 13 July 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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

// Own include
#include <asiEngine_CurvatureCombsFunc.h>

// asiData includes
#include <asiData_AAGParameter.h>
#include <asiData_CurvatureCombsNode.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// Active Data includes
#include <ActData_ParameterFactory.h>
#include <ActData_UserExtParameter.h>
#include <ActAux_ArrayUtils.h>

//-----------------------------------------------------------------------------

int asiEngine_CurvatureCombsFunc::execute(const Handle(ActAPI_HParameterList)& inputs,
                                          const Handle(ActAPI_HParameterList)& outputs,
                                          const Handle(Standard_Transient)&) const
{
  ActAPI_ProgressEntry progress = this->GetProgressNotifier();

  Handle(ActData_IntParameter)     numPtsParam       = Handle(ActData_IntParameter)::DownCast(inputs->Value(1));
  //Handle(ActData_RealParameter)    scaleFactorParam  = Handle(ActData_RealParameter)::DownCast(inputs->Value(2));
  Handle(ActData_RealParameter)    amplFactorParam   = Handle(ActData_RealParameter)::DownCast(inputs->Value(3));
  Handle(ActData_UserExtParameter) aagParam         = Handle(ActData_UserExtParameter)::DownCast(inputs->Value(4));
  Handle(ActData_IntParameter)     selectedEdgeParam = Handle(ActData_IntParameter)::DownCast(inputs->Value(5));
  if ( numPtsParam.IsNull()     || //scaleFactorParam.IsNull() ||
       amplFactorParam.IsNull() || aagParam.IsNull()         ||
       selectedEdgeParam.IsNull() )
  {
    return 1;
  }

  Handle(ActAPI_INode) ownerNode = aagParam->GetNode();
  Handle(asiAlgo_AAG) aag = Handle(asiData_AAGParameter)::DownCast(ownerNode->Parameter(aagParam->GetParamId()))->GetAAG();
  const TopTools_IndexedMapOfShape& subShapes = aag->RequestMapOfSubShapes();
  const TopoDS_Shape& edgeShape = subShapes(selectedEdgeParam->GetValue());
  if ( edgeShape.ShapeType() != TopAbs_EDGE )
  {
    progress.SendLogMessage(LogErr(Normal) << "Unexpected topological type of the selected edge.");
    return 1;
  }

  double f, l;
  Handle(Geom_Curve) curve = BRep_Tool::Curve(TopoDS::Edge(edgeShape), f, l);

  double amplFactor = amplFactorParam->GetValue();
  int    numPts     = numPtsParam->GetValue();

  std::vector<gp_Pnt> points;
  std::vector<double> params;
  std::vector<double> curvatures;
  std::vector<gp_Vec> combs;
  std::vector<bool>   combsOk;

  if ( !asiAlgo_Utils::CalculateCurvatureCombs(curve, f, l, numPts, amplFactor,
                                               points, params, curvatures, combs,
                                               combsOk) )
  {
    progress.SendLogMessage(LogErr(Normal) << "Cannot calculate curvature combs.");
    return 1;
  }

  Handle(HRealArray) pointsRealArray;
  if ( points.size() != 0 )
    ActAux_ArrayUtils::ToCoords3d<gp_Pnt>(points, pointsRealArray);
  //
  Handle(HBoolArray) combsOkFlag;
  if ( combsOk.size() != 0 )
  {
    combsOkFlag = new HBoolArray(0, int(combsOk.size()) - 1);
    for ( int i = 0; i < combsOkFlag->Length(); ++i )
      combsOkFlag->ChangeValue(i) = combsOk[i];
  }
  //
  Handle(HRealArray) paramsRealArray;
  if ( params.size() != 0 )
    ActAux_ArrayUtils::ToRealArray(params, paramsRealArray);
  //
  Handle(HRealArray) curvaturesRealArray;
  if ( curvatures.size() != 0 )
    ActAux_ArrayUtils::ToRealArray(curvatures, curvaturesRealArray);
  //
  Handle(HRealArray) combsRealArray;
  if ( combs.size() != 0 )
    ActAux_ArrayUtils::ToCoords3d<gp_Vec>(combs, combsRealArray);


  // Set the extreme thickness values.
  ActParamTool::AsRealArray(outputs->Value(1))->SetArray(pointsRealArray);
  ActParamTool::AsBoolArray(outputs->Value(2))->SetArray(combsOkFlag);
  ActParamTool::AsRealArray(outputs->Value(3))->SetArray(paramsRealArray);
  ActParamTool::AsRealArray(outputs->Value(4))->SetArray(curvaturesRealArray);
  ActParamTool::AsRealArray(outputs->Value(5))->SetArray(combsRealArray);

  return 0; // Success.
}

//-----------------------------------------------------------------------------

ActAPI_ParameterTypeStream
  asiEngine_CurvatureCombsFunc::inputSignature() const
{
  return ActAPI_ParameterTypeStream() << Parameter_Int  // Number of points.
                                      << Parameter_Real // Scale factor.
                                      << Parameter_Real // Amplification factor.
                                      << Parameter_AAG  // AAG.
                                      << Parameter_Int  // Selected edge.

  ;
}

//-----------------------------------------------------------------------------

ActAPI_ParameterTypeStream
  asiEngine_CurvatureCombsFunc::outputSignature() const
{
  return ActAPI_ParameterTypeStream() << Parameter_RealArray // Points.
                                      << Parameter_BoolArray // Points statuses.
                                      << Parameter_RealArray // Parameters.
                                      << Parameter_RealArray // Curvatures.
                                      << Parameter_RealArray // Combs.
    ;
}
