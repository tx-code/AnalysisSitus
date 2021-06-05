//-----------------------------------------------------------------------------
// Created on: 19 April 2020
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
#include <asiUI_BatchFacilities.h>

// asiUI includes
#include <asiUI_IV.h>
#include <asiUI_Logger.h>

// asiVisu includes
#include <asiVisu_PrsManager.h>

//-----------------------------------------------------------------------------

asiUI_BatchFacilities::asiUI_BatchFacilities(const bool initBatch,
                                             const bool initInterp,
                                             const bool overrideTclChannels)
//
: Standard_Transient()
{
  // Create Data Model.
  this->Model = new asiEngine_Model;
  if ( !Model->NewEmpty() )
  {
    Standard_ProgramError::Raise("Cannot create Data Model");
  }
  //
  this->Model->DisableTransactions();
  {
    this->Model->Populate();
  }
  this->Model->EnableTransactions();

  if ( initBatch )
  {
    // Initialize logger.
    this->Logger = new asiAlgo_Logger;
    this->Logger->AppendStream(&std::cout);

    // Initialize progress notifier.
    this->Progress = ActAPI_ProgressEntry( new asiUI_BatchNotifier(this->Logger) );

    // Prepare presentation manager for offscreen rendering.
    vtkSmartPointer<asiVisu_PrsManager>
            prsMgr3d = vtkSmartPointer<asiVisu_PrsManager>::New();
    //
    prsMgr3d->Initialize(nullptr, true); // Offscreen rendering mode.

    // Resize.
    prsMgr3d->GetRenderWindow()->SetSize(1024, 1024);

    // Initialize imperative plotter.
    this->Plotter = new asiUI_IV(this->Model, prsMgr3d, nullptr, nullptr);

    // Construct the interpreter
    if ( initInterp )
    {
      this->Interp = new asiTcl_Interp;
      this->Interp->Init(overrideTclChannels);
      this->Interp->SetModel(this->Model);
      this->Interp->SetProgress(this->Progress);
      this->Interp->SetPlotter(this->Plotter);
    }
  }
}

//-----------------------------------------------------------------------------

Handle(asiUI_BatchFacilities)
  asiUI_BatchFacilities::Instance(const bool initBatch,
                                  const bool initInterp,
                                  const bool overrideTclChannels)
{
  static Handle(asiUI_BatchFacilities)
    ref = new asiUI_BatchFacilities(initBatch, initInterp, overrideTclChannels);

  return ref;
}
