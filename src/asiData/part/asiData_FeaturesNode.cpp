//-----------------------------------------------------------------------------
// Created on: 06 December 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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
#include <asiData_FeaturesNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

// OpenCascade includes
#include <TDataStd_Integer.hxx>

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiData_FeaturesNode::Instance()
{
  return new asiData_FeaturesNode();
}

//-----------------------------------------------------------------------------

asiData_FeaturesNode::asiData_FeaturesNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name, PID_Name);
}

//-----------------------------------------------------------------------------

void asiData_FeaturesNode::Init()
{
  this->InitParameter(PID_Name, "Name");
}

//-----------------------------------------------------------------------------

TCollection_ExtendedString asiData_FeaturesNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_FeaturesNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------

Handle(asiData_FeatureNode)
  asiData_FeaturesNode::FindFeature(const int featureId) const
{
  // Iterate over the existing features to find one for the
  // requested ID.
  Handle(asiData_FeatureNode) feature_n;
  //
  for ( Handle(ActAPI_IChildIterator) cit = this->GetChildIterator(); cit->More(); cit->Next() )
  {
    /*
       Make this accessor as fast as possible by using labels directly and
       not using data cursors. Doing so, we avoid overheads on cursor
       construction and validation thus saving quite a lot of CPU cycles.
     */

    TDF_Label root = cit->ValueLabel();

    // Access label with user Parameters.
    TDF_Label idLab = root.FindChild(ActData_BaseNode::TagUser)
                          .FindChild(asiData_FeatureNode::PID_FeatureId);

    // Get int attribute.
    Handle(TDataStd_Integer) idAttr;
    //
    idLab.FindAttribute(TDataStd_Integer::GetID(), idAttr);
    //
    if ( idAttr.IsNull() )
      continue;

    // Compare feature ID with the passed one.
    if ( idAttr->Get() == featureId )
    {
      feature_n = Handle(asiData_FeatureNode)::DownCast( ActData_NodeFactory::NodeSettle(root) );
      break;
    }
  }

  return feature_n;
}
