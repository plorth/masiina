/*
 * Copyright (c) 2022, Rauli Laine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include <masiina/runtime/module.hpp>
#include <masiina/runtime/routine.hpp>
#include <plorth/runtime.hpp>

namespace masiina::runtime
{
  class environment : public plorth::module::manager
  {
  public:
    using module_cache_type = std::unordered_map<
      std::u32string,
      std::shared_ptr<plorth::object>
    >;

    explicit environment();

    inline const std::shared_ptr<plorth::runtime>& runtime() const
    {
      return m_runtime;
    }

    void add_imported_module(const std::shared_ptr<module>& module);

    bool is_finished() const;

    void spawn(const std::vector<std::shared_ptr<plorth::value>>& values);

    bool step();

    virtual std::shared_ptr<plorth::object> import_module(
      const std::shared_ptr<plorth::context>& context,
      const std::u32string& path
    );

  private:
    DISALLOW_COPY_AND_ASSIGN(environment);

  private:
    plorth::memory::manager m_memory_manager;
    const std::shared_ptr<plorth::runtime> m_runtime;
    std::unordered_map<std::u32string, std::shared_ptr<module>> m_imported_modules;
    module_cache_type m_module_cache;
    std::vector<std::shared_ptr<routine>> m_routines;
    std::size_t m_routine_offset;
  };
}
