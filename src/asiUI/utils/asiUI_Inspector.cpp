//-----------------------------------------------------------------------------
// Created on: 21 November 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Natalia Ermolaeva
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
#include <asiUI_Inspector.h>

#ifdef USE_OCCT_INSPECTOR
#include <CDM_Application.hxx>
#include <Message.hxx>
#include <Message_Report.hxx>
#include <inspector/TInspector_Communicator.hxx>

static TInspector_Communicator* m_TCommunicator;
#endif

#include <asiAlgo_Message.h>

//-----------------------------------------------------------------------------

void asiUI_Inspector::showInspector(const NCollection_List<Handle(Standard_Transient)>& parameters)
{
#ifdef USE_OCCT_INSPECTOR
  //Handle(ActData_BaseModel)
  //  M = Handle(ActData_BaseModel)::DownCast( interp->GetModel() );

  if (!m_TCommunicator)
  {
    m_TCommunicator = new TInspector_Communicator();

    //NCollection_List<Handle(Standard_Transient)> parameters;
    //parameters.Append(M->Document()->Application());

    m_TCommunicator->RegisterPlugin("TKDFBrowser");
    m_TCommunicator->Init ("TKDFBrowser", parameters, false);
    //m_TCommunicator->RegisterPlugin("TKVInspector");
    //m_TCommunicator->Init ("TKVInspector", parameters, false);
    m_TCommunicator->RegisterPlugin("TKShapeView");
    m_TCommunicator->Init ("TKShapeView", parameters, false);

    Handle(Message_Report) report = Message::DefaultReport(Standard_False);
    if (report.IsNull())
    {
      report = Message::DefaultReport(Standard_True);
      report->SetLimit(1000);
    }
    m_TCommunicator->RegisterPlugin("TKMessageView");

    m_TCommunicator->Init(parameters);

    //m_TCommunicator->Activate("TKDFBrowser");
    m_TCommunicator->Activate("TKMessageView");

    report->ActivateInMessenger(Standard_True);
    report->UpdateActiveInMessenger();

    //report->SetActiveMetric(Message_MetricType_ProcessCPUUserTime, Standard_True);
    //report->SetActiveMetric(Message_MetricType_ProcessCPUSystemTime, Standard_True);
    //report->SetActiveMetric(Message_MetricType_WallClock, Standard_True);
    ///*report->SetActiveMetric(Message_MetricType_MemPrivate, Standard_True);
    //report->SetActiveMetric(Message_MetricType_MemVirtual, Standard_True);
    //report->SetActiveMetric(Message_MetricType_MemWorkingSet, Standard_True);
    //report->SetActiveMetric(Message_MetricType_MemWorkingSetPeak, Standard_True);
    //report->SetActiveMetric(Message_MetricType_MemSwapUsage, Standard_True);
    //report->SetActiveMetric(Message_MetricType_MemSwapUsagePeak, Standard_True);*/
    //m_TCommunicator->Activate("TKMessageView");

    MESSAGE_INFO_LEVEL("asi messages")
  }
  else
  {
    if (!parameters.IsEmpty())
      m_TCommunicator->Init(parameters, false);

    m_TCommunicator->UpdateContent();
  }

  m_TCommunicator->SetVisible(true);
#endif
}

//-----------------------------------------------------------------------------

void asiUI_Inspector::hideInspector()
{
#ifdef USE_OCCT_INSPECTOR
  if (!m_TCommunicator)
    return;

  m_TCommunicator->SetVisible(false);
#endif
}

//-----------------------------------------------------------------------------

void asiUI_Inspector::updateInspector()
{
#ifdef USE_OCCT_INSPECTOR
  if (!m_TCommunicator)
    return;

  m_TCommunicator->UpdateContent();
#endif
}
