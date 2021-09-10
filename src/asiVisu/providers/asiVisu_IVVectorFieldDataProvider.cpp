//-----------------------------------------------------------------------------
// Created on: 10 September 2021
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

// Own include
#include <asiVisu_IVVectorFieldDataProvider.h>

//-----------------------------------------------------------------------------

//! Ctor.
asiVisu_IVVectorFieldDataProvider::asiVisu_IVVectorFieldDataProvider(const Handle(asiData_IVVectorFieldNode)& node)
: asiVisu_VectorsDataProvider(node)
{}

//-----------------------------------------------------------------------------

//! \return points where vectors are located.
Handle(asiAlgo_BaseCloud<double>) asiVisu_IVVectorFieldDataProvider::GetPointsd()
{
  return Handle(asiData_IVVectorFieldNode)::DownCast(m_source)->GetPoints();
}

//-----------------------------------------------------------------------------

//! \return vectors to visualize.
Handle(asiAlgo_BaseCloud<double>) asiVisu_IVVectorFieldDataProvider::GetVectorsd()
{
  return Handle(asiData_IVVectorFieldNode)::DownCast(m_source)->GetVectors();
}

//-----------------------------------------------------------------------------

//! \return max modulus for a vector.
double asiVisu_IVVectorFieldDataProvider::GetMaxVectorModulus() const
{
  return 1.0;
}

//-----------------------------------------------------------------------------

//! Enumerates Data Parameters playing as sources for DOMAIN -> VTK
//! translation process.
//! \return source Parameters.
Handle(ActAPI_HParameterList) asiVisu_IVVectorFieldDataProvider::translationSources() const
{
  ActParamStream out;

  out << m_source->Parameter(asiData_IVVectorFieldNode::PID_Points)
      << m_source->Parameter(asiData_IVVectorFieldNode::PID_Vectors);

  return out;
}
