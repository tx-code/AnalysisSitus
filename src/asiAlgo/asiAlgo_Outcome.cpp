//-----------------------------------------------------------------------------
// Created on: 03 May 2021
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
#include <asiAlgo_Outcome.h>

// OpenCascade includes
#include <OSD_Timer.hxx>

using namespace asi;

//-----------------------------------------------------------------------------

Outcome::Outcome()
: Ok             (false),
  ElapsedTimeSec (-1),
  m_timer        (nullptr)
{
  this->startTimer();
}

//-----------------------------------------------------------------------------

Outcome::Outcome(const std::string& _name)
: Name           (_name),
  Ok             (false),
  ElapsedTimeSec (-1),
  m_timer        (nullptr)
{
  this->startTimer();
}

//-----------------------------------------------------------------------------

Outcome::Outcome(const bool _ok)
: Ok             (_ok),
  ElapsedTimeSec (-1),
  m_timer        (nullptr)
{
  this->startTimer();
}

//-----------------------------------------------------------------------------

Outcome::Outcome(const std::string& _name,
                 const bool         _ok)
: Name           (_name),
  Ok             (_ok),
  ElapsedTimeSec (-1),
  m_timer        (nullptr)
{
  this->startTimer();
}

//-----------------------------------------------------------------------------

Outcome::Outcome(const std::string& _name,
                 const bool         _ok,
                 const double       _time)
: Name           (_name),
  Ok             (_ok),
  ElapsedTimeSec (_time),
  m_timer        (nullptr)
{
  this->startTimer();
}

//-----------------------------------------------------------------------------

Outcome::~Outcome()
{
  delete m_timer;
}

//-----------------------------------------------------------------------------

const Outcome& Outcome::Failure()
{
  this->stopTimer();
  this->Ok = false;
  return *this;
}

//-----------------------------------------------------------------------------

const Outcome& Outcome::Success()
{
  this->stopTimer();
  this->Ok = true;
  return *this;
}

//-----------------------------------------------------------------------------

const Outcome& Outcome::Status(const bool _ok)
{
  this->stopTimer();
  this->Ok = _ok;
  return *this;
}

//-----------------------------------------------------------------------------

void Outcome::Dump(std::ostream& out) const
{
  out << "outcome for "                <<  this->Name.c_str()           << "\n";
  out << "ok: "                        << (this->Ok ? "true" : "false") << "\n";
  out << "elapsed time (wall clock): " <<  this->ElapsedTimeSec << " s" << "\n";
}

//-----------------------------------------------------------------------------

void Outcome::startTimer()
{
  if ( m_timer == nullptr )
    m_timer = new OSD_Timer;

  ( (OSD_Timer*) m_timer)->Start();
}

//-----------------------------------------------------------------------------

void Outcome::stopTimer()
{
  if ( m_timer == nullptr )
    return;

  OSD_Timer* casted = (OSD_Timer*) m_timer;
  casted->Stop();

  this->ElapsedTimeSec = casted->ElapsedTime();
}
