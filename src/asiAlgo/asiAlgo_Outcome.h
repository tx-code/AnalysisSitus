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

#ifndef asiAlgo_Outcome_h
#define asiAlgo_Outcome_h

// asiAlgo includes
#include <asiAlgo.h>

// Standard includes
#include <memory>
#include <string>

//! \ingroup ASI_CORE
//!
namespace asi {

//! \ingroup ASI_CORE
//!
//! Defines a class for holding the results of API functions. This class
//! contains a Boolean flag to indicate success or failure status of the
//! API routine, together with additional information on function execution.
//!
//! Usage of outcome objects instead of primitive Boolean types is inspired
//! by ACIS modeling kernel. See (Corney, J.R. and Lim, T. 2001. 3D Modelling with ACIS.)
//! for details.
class Outcome
{
public:

  std::string Name;           //!< Function name.
  bool        Ok;             //!< Success/failure.
  double      ElapsedTimeSec; //!< Elapsed (wall) time in seconds.

public:

  //! Default ctor.
  asiAlgo_EXPORT explicit
    Outcome();

  //! Ctor accepting function name.
  //! \param[in] _name function name.
  asiAlgo_EXPORT explicit
    Outcome(const std::string& _name);

  //! Ctor accepting success/failure flag.
  //! \param[in] _ok Boolean value to set as execution status.
  asiAlgo_EXPORT explicit
    Outcome(const bool _ok);

  //! Ctor accepting function name and success/failure flag.
  //! \param[in] _name function name.
  //! \param[in] _ok   Boolean value to set as execution status.
  asiAlgo_EXPORT explicit
    Outcome(const std::string& _name,
            const bool         _ok);

  //! Ctor accepting function name, success/failure flag and execution time.
  //! \param[in] _name function name.
  //! \param[in] _ok   Boolean value to set as execution status.
  //! \param[in] _time execution time in seconds.
  asiAlgo_EXPORT explicit
    Outcome(const std::string& _name,
            const bool         _ok,
            const double       _time);

  //! Dtor.
  asiAlgo_EXPORT
    ~Outcome();

public:

  //! \return failure outcome.
  asiAlgo_EXPORT const Outcome&
    Failure();

  //! \return success outcome.
  asiAlgo_EXPORT const Outcome&
    Success();

  //! Modifies this outcome to have the passed status.
  //! \param[in] _ok status to set.
  //! \return outcome with the given status.
  asiAlgo_EXPORT const Outcome&
    Status(const bool _ok);

public:

  //! Dumps outcome to the passed output stream.
  //! \param[in,out] out target output stream.
  asiAlgo_EXPORT void
    Dump(std::ostream& out) const;

protected:

  //! Starts internal timer.
  asiAlgo_EXPORT void
    startTimer();

  //! Stops internal timer.
  asiAlgo_EXPORT void
    stopTimer();

protected:

  //! Automatic timer. The pointer here is shared to allow for passing the ownership to
  //! the copied outcome objects. In the invocations like `api().Ok`, where `api()` is a
  //! function returning an outcome object by value (and that is the as-designed way of
  //! returning outcomes), the outcome object will be destroyed once the function is done,
  //! and then copied to the caller function's [temporary] variable for accessing the
  //! outcome status like `.Ok`. We want to be able to access just the same timer, so we
  //! make it shared and let the copy outcome object destroy it.
  std::shared_ptr<void> m_timer;

};

} // asiAlgo namespace.

#endif
