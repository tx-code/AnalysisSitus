//-----------------------------------------------------------------------------
// Created on: 06 February 2021
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
#include <exe_CommandServer.h>

// asiTcl includes
#include <asiTcl_Plugin.h>

// Qt includes
#pragma warning(push, 0)
#include <QDir>
#include <QNetworkDatagram>
#include <QUdpSocket>
#pragma warning(pop)

//-----------------------------------------------------------------------------

#define EXE_LOAD_MODULE(name) \
{ \
  Handle(asiUI_BatchFacilities) __cf = asiUI_BatchFacilities::Instance();\
  \
  asiTcl_Plugin::Status status = asiTcl_Plugin::Load(__cf->Interp, __cf, name); \
  if ( status == asiTcl_Plugin::Status_Failed ) \
    __cf->Progress.SendLogMessage(LogErr(Normal) << "Cannot load %1 commands." << name); \
  else if ( status == asiTcl_Plugin::Status_OK ) \
    __cf->Progress.SendLogMessage(LogInfo(Normal) << "Loaded %1 commands." << name); \
}

//-----------------------------------------------------------------------------

exe_CommandServer::exe_CommandServer(const Handle(asiEngine_Model)& model,
                                     ActAPI_ProgressEntry           progress,
                                     ActAPI_PlotterEntry            plotter,
                                     const QHostAddress&            hostAddress,
                                     const int                      port)
: m_pSocket  (nullptr),
  m_model    (model),
  m_progress (progress),
  m_plotter  (plotter),
  m_hostAddr (hostAddress),
  m_iPort    (port),
  m_bReady   (false),
  m_pThread  (nullptr)
{}

//-----------------------------------------------------------------------------

exe_CommandServer::~exe_CommandServer()
{}

//-----------------------------------------------------------------------------

void exe_CommandServer::Run()
{
  m_pThread = new QThread;
  this->moveToThread(m_pThread);

  QObject::connect( m_pThread, SIGNAL( started() ), this, SLOT( onStarted() ) );

  m_pThread->start();
}

//-----------------------------------------------------------------------------

void exe_CommandServer::onStarted()
{
  m_interp = new asiTcl_Interp;
  m_interp->Init(true);
  m_interp->SetModel(m_model);
  m_interp->SetProgress(m_progress);
  m_interp->SetPlotter(m_plotter);

  asiUI_BatchFacilities::Instance()->Interp = m_interp;

  // Load default commands.
  EXE_LOAD_MODULE("cmdMisc")
  EXE_LOAD_MODULE("cmdEngine")
  EXE_LOAD_MODULE("cmdRE")
  EXE_LOAD_MODULE("cmdDDF")
  EXE_LOAD_MODULE("cmdAsm")

  //---------------------------------------------------------------------------
  // Load plugins
  //---------------------------------------------------------------------------

  // Lookup for custom plugins and try to load them.
  QDir pluginDir( QDir::currentPath() + "/asi-plugins" );
  TCollection_AsciiString pluginDirStr = pluginDir.absolutePath().toLatin1().data();
  //
  std::cout << "Looking for plugins at "
            << pluginDirStr.ToCString() << "..." << std::endl;
  //
  QStringList cmdLibs = pluginDir.entryList(QStringList() << "*.dll", QDir::Files);
  //
  foreach ( QString cmdLib, cmdLibs )
  {
    TCollection_AsciiString cmdLibName = cmdLib.section(".", 0, 0).toLatin1().data();
    //
    m_progress.SendLogMessage(LogNotice(Normal) << "Detected %1 as a custom plugin's library."
                                                << cmdLibName);

    EXE_LOAD_MODULE(cmdLibName);
  }

  // Initialize socket.
  this->initSocket();
}

//-----------------------------------------------------------------------------

void exe_CommandServer::readPendingDatagrams()
{
  while ( m_pSocket->hasPendingDatagrams() )
  {
    QNetworkDatagram datagram = m_pSocket->receiveDatagram();
    this->processDatagram(&datagram);
  }
}

//-----------------------------------------------------------------------------

void exe_CommandServer::initSocket()
{
  m_pSocket = new QUdpSocket(this);

  QObject::connect(m_pSocket, SIGNAL( readyRead() ), this, SLOT( readPendingDatagrams() ));

  if ( m_pSocket->state() != m_pSocket->BoundState )
  {
    m_bReady = m_pSocket->bind(m_hostAddr, m_iPort);

    if ( m_bReady )
      std::cout << "Socket is ready." << std::endl;
  }
}

//-----------------------------------------------------------------------------

void exe_CommandServer::processDatagram(QNetworkDatagram* pDatagram)
{
  QByteArray data = pDatagram->data();
  TCollection_AsciiString cmdString = QString::fromLatin1(data).toLatin1().data();

  std::cout << "Received datagram: " << cmdString.ToCString() << std::endl;

  if ( !m_interp.IsNull() )
  {
    m_interp->Eval(cmdString);
  }
}
