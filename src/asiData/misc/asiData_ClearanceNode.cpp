//-----------------------------------------------------------------------------
// Created on: 14 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Quaoar Studio LLC
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
#include <asiData_ClearanceNode.h>

// asiData includes
#include <asiData_MeshParameter.h>

// asiAlgo includes
#include <asiAlgo_MeshField.h>

// OpenCascade includes
#include <Precision.hxx>

#if defined USE_MOBIUS
  #include <mobius/cascade.h>

  using namespace mobius;
#endif

//-----------------------------------------------------------------------------

asiData_ClearanceNode::asiData_ClearanceNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,         PID_Name);
  REGISTER_PARAMETER(IntArray,     PID_ClearanceFieldIds);
  REGISTER_PARAMETER(RealArray,    PID_ClearanceFieldValues);
  REGISTER_PARAMETER(Real,         PID_ScalarMin);
  REGISTER_PARAMETER(Real,         PID_ScalarMax);
  REGISTER_PARAMETER(TreeFunction, PID_CheckClearanceFunc);

  this->registerParameter(PID_Mesh, asiData_MeshParameter::Instance(), false);
}

//-----------------------------------------------------------------------------

void asiData_ClearanceNode::Init()
{
  // Initialize Parameters.
  this->InitParameter(PID_Name,        "Name",        "", ParameterFlag_IsVisible, true);
  this->InitParameter(PID_ScalarMin,   "Min. scalar", "", ParameterFlag_IsVisible, true);
  this->InitParameter(PID_ScalarMax,   "Max. scalar", "", ParameterFlag_IsVisible, true);

  // Set default values.
  this->SetMeshWithScalars( asiAlgo_MeshWithFields() );
  //
  ActParamTool::AsReal( this->Parameter(PID_ScalarMin) ) ->SetValue(-Precision::Infinite() );
  ActParamTool::AsReal( this->Parameter(PID_ScalarMax) ) ->SetValue( Precision::Infinite() );
}

//-----------------------------------------------------------------------------

TCollection_ExtendedString asiData_ClearanceNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_ClearanceNode::SetName(const TCollection_ExtendedString& N)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(N);
}

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS
void asiData_ClearanceNode::SetMesh(const t_ptr<poly_Mesh>& mesh)
{
  Handle(asiData_MeshParameter)::DownCast( this->Parameter(PID_Mesh) )->SetMesh(mesh);
}
#endif

//-----------------------------------------------------------------------------

void asiData_ClearanceNode::SetMeshWithScalars(const asiAlgo_MeshWithFields& mesh)
{
#if defined USE_MOBIUS
  this->SetMesh(mesh.triangulation);

  // Prepare array of ids.
  if ( mesh.fields.size() )
  {
    Handle(asiAlgo_MeshScalarField)
      field = Handle(asiAlgo_MeshScalarField)::DownCast(mesh.fields[0]);

    // Prepare arrays.
    const int numScalars = field->data.Extent();
    //
    Handle(HIntArray)  elemArr   = new HIntArray  (0, numScalars - 1, 0);
    Handle(HRealArray) valuesArr = new HRealArray (0, numScalars - 1, 0.);

    // Populate arrays.
    int idx = 0;
    for ( asiAlgo_MeshScalarField::t_data::Iterator it(field->data);
          it.More(); it.Next(), ++idx )
    {
      const int    elemId = it.Key();
      const double scalar = it.Value();

      elemArr->SetValue(idx, elemId);
      valuesArr->SetValue(idx, scalar);
    }

    // Store data.
    ActParamTool::AsIntArray( this->Parameter(PID_ClearanceFieldIds) )->SetArray(elemArr);
    ActParamTool::AsRealArray( this->Parameter(PID_ClearanceFieldValues) )->SetArray(valuesArr);
  }
  else
  {
    // Nullify arrays.
    ActParamTool::AsIntArray( this->Parameter(PID_ClearanceFieldIds) )->SetArray(nullptr);
    ActParamTool::AsRealArray( this->Parameter(PID_ClearanceFieldValues) )->SetArray(nullptr);
  }
#endif
}
