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
#include <masiina/compiler/io.hpp>
#include <masiina/compiler/symbol-map.hpp>

namespace masiina::compiler
{
  symbol_map::symbol_map() {}

  symbol_map::symbol_map(const symbol_map& that)
    : m_list(that.m_list)
    , m_map(that.m_map) {}

  symbol_map&
  symbol_map::operator=(const symbol_map& that)
  {
    m_list = that.m_list;
    m_map = that.m_map;

    return *this;
  }

  std::uint32_t
  symbol_map::add(const std::u32string& str)
  {
    const auto iterator = m_map.find(str);
    std::uint32_t index;

    if (iterator != std::end(m_map))
    {
      return iterator->second;
    }
    index = static_cast<std::uint32_t>(m_list.size());
    m_list.push_back(str);
    m_map[str] = index;

    return index;
  }

  void
  symbol_map::write(FILE* output) const
  {
    io::write_uint32(output, static_cast<std::uint32_t>(m_list.size()));
    for (const auto& str : m_list)
    {
      io::write_string(output, str);
    }
  }
}
