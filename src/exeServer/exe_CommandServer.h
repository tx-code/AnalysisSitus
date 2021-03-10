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

#ifndef exe_CommandServer_HeaderFile
#define exe_CommandServer_HeaderFile

// asiUI includes
#include <asiUI_BatchFacilities.h>

// Qt includes
#pragma warning(push, 0)
#include <QHostAddress>
#include <QThread>
#pragma warning(pop)

//-----------------------------------------------------------------------------

class QUdpSocket;
class QNetworkDatagram;

//-----------------------------------------------------------------------------

#define CLI_HostDefault QHostAddress::LocalHost
#define CLI_PortDefault 7755

//-----------------------------------------------------------------------------

//! Command server for getting UDP datagrams and putting them into the
//! shared command queue.
class exe_CommandServer : public QObject
{
  Q_OBJECT

public:

  //! Ctor.
  //! \param[in] model       the Data Model instance.
  //! \param[in] progress    the progress notifier.
  //! \param[in] plotter     the imperative plotter.
  //! \param[in] hostAddress the host address to use.
  //! \param[in] port        the port number to use.
  exe_CommandServer(const Handle(asiEngine_Model)& model,
                    ActAPI_ProgressEntry           progress,
                    ActAPI_PlotterEntry            plotter,
                    const QHostAddress&            hostAddress = CLI_HostDefault,
                    const int                      port        = CLI_PortDefault);

  //! Dtor.
  virtual ~exe_CommandServer();

public:

  void Run();

protected slots:

  void readPendingDatagrams();
  void onStarted();

protected:

  //! Initializes UPD socket connection.
  void initSocket();

  //! Processes the passed datagram.
  void processDatagram(QNetworkDatagram* pDatagram);

private:

  QUdpSocket*             m_pSocket;  //!< Socket connection.
  Handle(asiEngine_Model) m_model;    //!< Data Model instance.
  ActAPI_ProgressEntry    m_progress; //!< Progress entry.
  ActAPI_PlotterEntry     m_plotter;  //!< Imperative plotter.
  Handle(asiTcl_Interp)   m_interp;   //!< Shared command queue.
  QHostAddress            m_hostAddr; //!< Host address.
  int                     m_iPort;    //!< Port number.
  bool                    m_bReady;   //!< Whether socket is ready or not.

  // Thread.
  QThread* m_pThread;

};

#endif
