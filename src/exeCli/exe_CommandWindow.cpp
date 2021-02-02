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

// STD includes
#include <strsafe.h>
#include <stdio.h>
#include <string>

//-----------------------------------------------------------------------------
// Some predefined commands
//-----------------------------------------------------------------------------

#define QR_CMD_PROMPT   "AnalysisSitus> "
#define QR_CMD_BUF_SIZE 255

//-----------------------------------------------------------------------------
// Construction and destruction
//-----------------------------------------------------------------------------

//! Ctor.
//! \param[in] queue command queue.
//! \param[in] cf    common facilities.
exe_CommandWindow::exe_CommandWindow(const Handle(exe_CommandQueue)&      queue,
                                     const Handle(asiUI_BatchFacilities)& cf)
: m_queue(queue), m_cf(cf)
{}

//! Destructor.
exe_CommandWindow::~exe_CommandWindow()
{
}

//! Creates new Console Window.
//! \return true in case of success, false -- otherwise.
bool exe_CommandWindow::Create()
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

//! Starts message loop.
void exe_CommandWindow::StartMessageLoop()
{
  bool stopPrompt = false;
  do
  {
    Handle(exe_BaseCmd) LastCommand = m_queue->Last();
    //bool canProceed = false;
    //do
    //{
    //  LastCommand = m_queue->Last();
    //  if ( LastCommand.IsNull() )
    //    canProceed = true;
    //  else
    //  {
    //    // Check command type: we can proceed only with console ones
    //    visu_ConsoleCmd* CmdPtr = dynamic_cast<visu_ConsoleCmd*>( LastCommand.Access() );
    //    if ( CmdPtr )
    //      canProceed = true;
    //  }
    //}
    //while ( !canProceed );

    // If there is something to proceed, let us do it
    if ( !LastCommand.IsNull() )
    {
      // Check if it is a standard 'exit' command
      /*visu_ExitCmd* ExitPtr = dynamic_cast<visu_ExitCmd*>( LastCommand.Access() );
      if ( ExitPtr )
        stopPrompt = true;*/

      // Remove command from queue and execute it.
      m_queue->Pop();
      //
      if ( m_cf->Interp->Eval(LastCommand->Cmd) == TCL_ERROR )
        DisplayMessage("Console", "Command failed!", false);

      // Has 'exit' command been pushed?
      if ( stopPrompt )
        continue;
    }

    // Get next command from user
    std::cout << QR_CMD_PROMPT;
    std::string inputStr;
    std::getline(std::cin, inputStr);

    Handle(exe_BaseCmd) newCommand = new exe_BaseCmd;
    newCommand->Cmd = inputStr.c_str();

    // Push command to the shared queue
    m_queue->Push(newCommand);
  }
  while ( !stopPrompt );
}

//! Prints message from the given client (its user-friendly name should be
//! provided).
//! \param From [in] client name.
//! \param Message [in] message to display.
//! \param newPrompt [in] indicates whether to ask for a new prompt.
void exe_CommandWindow::DisplayMessage(const std::string& From,
                                       const std::string& Message,
                                       const bool         newPrompt)
{
  std::cout << "`" << From.c_str() << "` says: \"" << Message.c_str() << "\"" << std::endl;
  if ( newPrompt )
    std::cout << QR_CMD_PROMPT;
}
