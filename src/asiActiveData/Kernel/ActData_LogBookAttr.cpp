//-----------------------------------------------------------------------------
// Created on: September 2019
//-----------------------------------------------------------------------------
// Copyright (c) 2019, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

// Own include
#include <ActData_LogBookAttr.h>

// OCCT includes
#include <Standard_GUID.hxx>

//-----------------------------------------------------------------------------

Handle(ActData_LogBookAttr) ActData_LogBookAttr::Set(const TDF_Label& label)
{
  Handle(ActData_LogBookAttr) A;
  //
  if ( !label.FindAttribute(GUID(), A) )
  {
    A = new ActData_LogBookAttr();
    label.AddAttribute(A);
  }
  return A;
}

//-----------------------------------------------------------------------------

const Standard_GUID& ActData_LogBookAttr::GUID()
{
  static Standard_GUID AttrGUID("A4533C1E-7DA0-49CE-9597-87B70A92DCE7");
  return AttrGUID;
}

//-----------------------------------------------------------------------------

const Standard_GUID& ActData_LogBookAttr::ID() const
{
  return GUID();
}

//-----------------------------------------------------------------------------

Handle(TDF_Attribute) ActData_LogBookAttr::NewEmpty() const
{
  return new ActData_LogBookAttr();
}

//-----------------------------------------------------------------------------

void ActData_LogBookAttr::Restore(const Handle(TDF_Attribute)& from)
{
  Handle(ActData_LogBookAttr) fromLog = Handle(ActData_LogBookAttr)::DownCast(from);

  // Copy map.
  m_map = fromLog->GetMap();
}

//-----------------------------------------------------------------------------

void ActData_LogBookAttr::Paste(const Handle(TDF_Attribute)& into,
                                const Handle(TDF_RelocationTable)&) const
{
  // Copy map.
  Handle(ActData_LogBookAttr) intoLog = Handle(ActData_LogBookAttr)::DownCast(into);
  intoLog->m_map = m_map;
}

//-----------------------------------------------------------------------------

void ActData_LogBookAttr::LogLabel(const TDF_Label& label)
{
  this->Backup();

  m_map.Add(label);
}

//-----------------------------------------------------------------------------

Standard_Boolean ActData_LogBookAttr::IsLogged(const TDF_Label& label) const
{
  return m_map.Contains(label);
}

//-----------------------------------------------------------------------------

void ActData_LogBookAttr::ReleaseLogged()
{
  m_map.Clear();
}

//-----------------------------------------------------------------------------

void ActData_LogBookAttr::ReleaseLogged(const TDF_Label& label)
{
  m_map.Remove(label);
}
