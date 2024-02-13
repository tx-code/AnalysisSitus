//-----------------------------------------------------------------------------
// Created on: 08 February 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Sergey Slyadnev
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

#ifndef asiAlgo_ConcurrentSet_h
#define asiAlgo_ConcurrentSet_h

// asiAlgo includes
#include <asiAlgo.h>

// Standard includes
#include <mutex>
#include <set>

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
//! Concurrent set to be accessed from parallel worker threads. This class is not
//! efficient as it uses mutex locks. Any TBB version of the same tool is expected
//! to behave better in terms of performance.
template <typename T, typename Compare = std::less<T>>
class asiAlgo_ConcurrentSet
{
private:
  std::set<T, Compare> set_;
  mutable std::mutex mutex_;

public:
  typedef typename std::set<T, Compare>::iterator iterator;

  std::pair<iterator, bool>
  insert(const T& val)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return set_.insert(val);
  }

  size_t size() const
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return set_.size();
  }

  bool contains(const T& val)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return set_.find(val) != set_.end();
  }

  void clear_unsafe()
  {
    set_.clear();
  }
};

#endif
