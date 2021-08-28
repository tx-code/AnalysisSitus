//-----------------------------------------------------------------------------
// Created on: 17 February 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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
#include <asiAlgo_IGES.h>

// OCCT includes
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <Transfer_TransientProcess.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_WorkSession.hxx>
#include <IFSelect_CheckCounter.hxx>

//-----------------------------------------------------------------------------
bool asiAlgo_IGES::Read(const TCollection_AsciiString& filename,
                        TopoDS_Shape&                  result)
{
  IGESControl_Reader reader;
  Handle(XSControl_WorkSession) WS = reader.WS();

  // Read CAD and associated data from file
  try
  {
    // Reading file into memory
    {
      IFSelect_ReturnStatus outcome = reader.ReadFile(filename.ToCString());
      //
      if (outcome != IFSelect_RetDone)
      {
        m_progress.SendLogMessage(LogErr(Normal) << "IGES reader failed (error occurred while reading IGES file).");

        printCheckStats(WS->Model(), WS->ModelCheckList());
        clearSession(WS);
        return false;
      }
      printCheckStats(WS->Model(), WS->ModelCheckList());
    }

    // Translate model into shape
    {
      m_progress.SetMessageKey("Translate loaded file into model.");
      {
        // Transfer all roots into one shape or into several shapes.
        try
        {
          reader.TransferRoots();
        }
        catch (Standard_Failure&)
        {
          m_progress.SendLogMessage(LogErr(Normal) << "Warning: exception occurred during translation.");
        }
        if (reader.NbShapes() <= 0)
        {
          m_progress.SendLogMessage(LogErr(Normal) << "Error: transferring IGES to BREP failed.");
          return false;
        }

        TopoDS_Shape preResult = reader.OneShape();

        // Release memory after translation.
        reader.WS()->NewModel();
        Standard::Purge();

        result = preResult;
        clearSession(WS);
      }
    }
  }
  catch (...)
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Warning: exception occurred during translation.");
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
void asiAlgo_IGES::printCheckStats(const Handle(Interface_InterfaceModel)& model,
                                   const Interface_CheckIterator&          checkIterator)
{
  if (model.IsNull())
    return;

  Handle(IFSelect_CheckCounter) failCounter = new IFSelect_CheckCounter(false);
  failCounter->Analyse(checkIterator, model, true, false);
  failCounter->SetName("IGES Reading");
  int nbFails = failCounter->List()->Size();
  if (nbFails > 0)
  {
    m_progress.SendLogMessage(LogNotice(Normal) << "IGES Reading problems:");
    Handle(TColStd_HSequenceOfHAsciiString) fails = failCounter->List();
    int totalNumber = 0;
    for (int it = 1; it <= fails->Length(); it++)
    {
      Handle(TCollection_HAsciiString) failMessage = fails->Value(it);
      // Exclude useless warning about non-alphabetical order of complex type in STEP.
      int isUseless = failMessage->SearchFromEnd("alphabetic order");
      if (isUseless > 0)
        continue;

      Standard_CString fail = failMessage->ToCString();
      int curNumber = failCounter->NbTimes(fail);
      totalNumber += curNumber;
      m_progress.SendLogMessage(LogNotice(Normal) << "  Count: %1  Message: %2" << curNumber << fail);
    }
    m_progress.SendLogMessage(LogNotice(Normal) << "Total: %1" << totalNumber);
  }
}

//-----------------------------------------------------------------------------
void asiAlgo_IGES::clearSession(const Handle(XSControl_WorkSession)& WS)
{
  if (WS.IsNull())
    return;

  // Clear transient process.
  const Handle(Transfer_TransientProcess)& mapReader = WS->TransferReader()->TransientProcess();
  if (!mapReader.IsNull())
    mapReader->Clear();

  // Clear transfer reader.
  const Handle(XSControl_TransferReader)& transferReader = WS->TransferReader();
  if (!transferReader.IsNull())
    transferReader->Clear(-1);
}

