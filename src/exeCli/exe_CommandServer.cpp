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

// Qt includes
#include <QNetworkDatagram>
#include <QUdpSocket>

//! Ctor.
//! \param[in] queue command queue.
exe_CommandServer::exe_CommandServer(const Handle(exe_CommandQueue)& queue)
: m_queue(queue)
{
  this->initSocket();
}

//! Destructor.
exe_CommandServer::~exe_CommandServer()
{
}

//! Starts message loop.
void exe_CommandServer::StartMessageLoop()
{
  while ( 1 )
  {
    if ( m_pSocket->hasPendingDatagrams() )
    {
      QNetworkDatagram datagram = m_pSocket->receiveDatagram();

      QByteArray data = datagram.data();
      TCollection_AsciiString cmdString = QStr2AsciiStr( QString::fromLatin1(data) );

      std::cout << "Received datagram: " << cmdString.ToCString() << std::endl;

      m_queue->Push( new exe_BaseCmd(cmdString) );
    }
  }
}

void exe_CommandServer::initSocket()
{
  m_pSocket = new QUdpSocket(this);
  m_pSocket->bind(QHostAddress::LocalHost, 7755);
}
