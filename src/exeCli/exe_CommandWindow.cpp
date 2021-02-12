//-----------------------------------------------------------------------------
// Created on: 02 February 2021
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
#include <exe_CommandWindow.h>

//-----------------------------------------------------------------------------
// Construction and destruction
//-----------------------------------------------------------------------------

//! Ctor.
//! \param[in] queue command queue.
//! \param[in] cf    common facilities.
exe_CommandWindow::exe_CommandWindow(const Handle(exe_CommandQueue)& queue)
: m_queue(queue)
{}

//! Destructor.
exe_CommandWindow::~exe_CommandWindow()
{
}

//! Creates new Console Window.
//! \param[in] allocConsole indicates whether to allocate console
//!                         and override stream.
//! \return true in case of success, false -- otherwise.
bool exe_CommandWindow::Create(const bool allocConsole)
{
  if ( allocConsole )
  {
    if ( AllocConsole() )
    {
      FILE *stream;
      SetConsoleTitleA("Analysis Situs >>> Terminal");

      freopen_s(&stream, "CONIN$", "r", stdin);
      freopen_s(&stream, "CONOUT$", "wb", stdout);
      freopen_s(&stream, "CONOUT$", "wb", stderr);

      return true;
    }

    return false;
  }

  return true;
}

//! Starts message loop.
void exe_CommandWindow::StartMessageLoop()
{
  bool stopPrompt = false;
  do
  {
    // Get next command from user.
    std::string inputStr;
    std::getline(std::cin, inputStr);

    // Put to the queue.
    if ( !inputStr.empty() )
    {
      Handle(exe_BaseCmd)
        newCommand = new exe_BaseCmd( inputStr.c_str() );

      // Push command to the shared queue.
      m_queue->Push(newCommand);
    }
    else
      std::cout << AS_CMD_PROMPT;
  }
  while ( !stopPrompt );
}
