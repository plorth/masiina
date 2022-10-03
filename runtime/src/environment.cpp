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
#include <iostream>

#include <masiina/runtime/environment.hpp>
#include <peelo/unicode/encoding/utf8.hpp>

namespace masiina::runtime
{
  environment::environment()
    : m_memory_manager()
    , m_runtime(plorth::runtime::make(m_memory_manager)) {}

  void
  environment::add_imported_module(const std::shared_ptr<module>& module)
  {
    m_imported_modules[module->name()] = module;
  }

  bool
  environment::is_finished() const
  {
    for (const auto& routine : m_routines)
    {
      if (!routine->is_finished())
      {
        return false;
      }
    }

    return true;
  }

  void
  environment::spawn(const std::vector<std::shared_ptr<plorth::value>>& values)
  {
    m_routines.push_back(std::make_shared<routine>(
      plorth::context::make(m_runtime),
      values
    ));
  }

  bool
  environment::step()
  {
    bool error_occurred = false;

    if (m_routine_offset < m_routines.size())
    {
      const auto& routine = m_routines[m_routine_offset];

      if (!routine->step())
      {
        const auto& context = routine->context();
        const auto& error = context->error();

        if (error)
        {
          const auto& position = error->position();

          error_occurred = true;
          std::cerr << "Error: ";
          if (position && (!position->file.empty() || position->line))
          {
            std::cerr
              << peelo::unicode::encoding::utf8::encode(position->file)
              << ":"
              << position->line
              << ":"
              << position->column
              << ":";
          }
          std::cerr
            << error->code()
            << " - "
            << peelo::unicode::encoding::utf8::encode(error->message());
        } else {
          std::cerr << "Unknown error." << std::endl;
        }
        std::cerr << std::endl;
        context->clear_error();
      }
      if (routine->is_finished())
      {
        m_routines.erase(m_routines.begin() + m_routine_offset);
      }
      ++m_routine_offset;
    } else {
      m_routine_offset = 0;
    }

    return error_occurred;
  }

  std::shared_ptr<plorth::object>
  environment::import_module(
    const std::shared_ptr<plorth::context>& context,
    const std::u32string& path
  )
  {
    const auto cached_module_index = m_module_cache.find(path);

    if (cached_module_index != std::end(m_module_cache))
    {
      return cached_module_index->second;
    }

    const auto imported_module_index = m_imported_modules.find(path);

    if (imported_module_index != std::end(m_imported_modules))
    {
      auto module_context = plorth::context::make(context->runtime());
      std::vector<plorth::object::value_type> result;
      std::shared_ptr<plorth::object> module;

      module_context->filename(path);
      for (const auto& value : imported_module_index->second->values())
      {
        if (!plorth::value::exec(module_context, value))
        {
          const auto error = module_context->error();

          if (error)
          {
            context->error(error);
          }

          return nullptr;
        }
      }

      // Finally convert the module into an object.
      for (const auto& word : module_context->dictionary().words())
      {
        result.push_back({ word->symbol()->id(), word->quote() });
      }

      module = context->runtime()->object(result);
      m_module_cache[path] = module;

      return module;
    }

    // TODO: Implement file system access at some point?

    return nullptr;
  }
}
