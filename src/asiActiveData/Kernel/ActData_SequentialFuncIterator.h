//-----------------------------------------------------------------------------
// Created on: November 2012
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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

#ifndef ActData_SequentialFuncIterator_HeaderFile
#define ActData_SequentialFuncIterator_HeaderFile

// Active Data includes
#include <ActData.h>

// OCCT includes
#include <TDF_Label.hxx>
#include <TDF_LabelList.hxx>
#include <TFunction_ExecutionStatus.hxx>
#include <TFunction_Scope.hxx>

//! \ingroup AD_DF
//!
//! Iterator returning independent Functions available for execution at each
//! stage of Dependency Graph processing. Note that conceptually this
//! iterator is similar to standard TFunction_Iterator, however, it has
//! one important difference. Unlike the standard one, this iterator is
//! NOT designed for cases when parallel processing is required for the
//! list of currently available Functions. Indeed, this iterator sorts
//! the returned collection so that to have lower-priority Functions
//! preceeding the higher-priority ones. In fact, this iterator implements
//! prioritization mechanism for INDEPENDENT Tree Functions.
//!
//! If you do not need any priorities for your independent Functions (e.g.
//! if you would like to follow the "normal" way of execution when dependencies
//! are always expressed with inputs/outputs correspondence) and if you do not
//! want to loose parallelism possibilities at this level, prefer using
//! the standard TFunction_Iterator instead.
class ActData_SequentialFuncIterator
{
public:

  ActData_EXPORT
    ActData_SequentialFuncIterator();

  ActData_EXPORT
    ActData_SequentialFuncIterator(const TDF_Label& theAccess);

public:

  ActData_EXPORT virtual void
    Init(const TDF_Label& theAccess);

  ActData_EXPORT virtual void
    Next();

  ActData_EXPORT virtual Standard_Boolean
    More() const;

  ActData_EXPORT virtual const TDF_LabelList&
    Current() const;

  ActData_EXPORT virtual TFunction_ExecutionStatus
    GetStatus(const TDF_Label& theFunc) const;

  ActData_EXPORT virtual void
    SetStatus(const TDF_Label& theFunc,
              const TFunction_ExecutionStatus theStatus) const;

protected:

  Standard_Boolean isHighPriority(const TDF_Label& theFunc) const;

protected:

  TDF_LabelList           m_currentFunctions; //!< Current Functions.
  Handle(TFunction_Scope) m_scope;            //!< Function Scope.

};

#endif
