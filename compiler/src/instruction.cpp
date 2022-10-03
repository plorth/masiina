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
#include <masiina/compiler/instruction.hpp>
#include <masiina/compiler/io.hpp>
#include <masiina/opcode.hpp>

namespace masiina::compiler::instruction
{
  base::base() {}

  push_block::push_block(enum type type, const container_type& elements)
    : m_type(type)
    , m_elements(elements) {}

  void
  push_block::write(FILE* output) const
  {
    std::fputc(
      m_type == type::array ? opcode::push_array : opcode::push_quote,
      output
    );
    io::write_uint32(output, static_cast<std::uint32_t>(m_elements.size()));
    for (const auto& element : m_elements)
    {
      element->write(output);
    }
  }

  push_string::push_string(const std::u32string& value)
    : m_value(value) {}

  void
  push_string::write(FILE* output) const
  {
    std::fputc(opcode::push_string, output);
    io::write_string(output, m_value);
  }

  push_string_const::push_string_const(std::uint32_t index)
    : m_index(index) {}

  void
  push_string_const::write(FILE* output) const
  {
    std::fputc(opcode::push_string_const, output);
    io::write_uint32(output, m_index);
  }

  push_symbol::push_symbol(
    const std::u32string& id,
    std::uint32_t filename_index,
    std::uint16_t line,
    std::uint16_t column
  )
    : m_id(id)
    , m_filename_index(filename_index)
    , m_line(line)
    , m_column(column) {}

  void
  push_symbol::write(FILE* output) const
  {
    std::fputc(opcode::push_symbol, output);
    io::write_string(output, m_id);
    io::write_uint32(output, m_filename_index);
    io::write_uint16(output, m_line);
    io::write_uint16(output, m_column);
  }

  push_symbol_const::push_symbol_const(
    std::uint32_t index,
    std::uint32_t filename_index,
    std::uint16_t line,
    std::uint16_t column
  )
    : m_index(index)
    , m_filename_index(filename_index)
    , m_line(line)
    , m_column(column) {}

  void
  push_symbol_const::write(FILE* output) const
  {
    std::fputc(opcode::push_symbol_const, output);
    io::write_uint32(output, m_index);
    io::write_uint32(output, m_filename_index);
    io::write_uint16(output, m_line);
    io::write_uint16(output, m_column);
  }

  push_object::push_object(const container_type& properties)
    : m_properties(properties) {}

  void
  push_object::write(FILE* output) const
  {
    std::fputc(opcode::push_object, output);
    io::write_uint32(output, static_cast<std::uint32_t>(m_properties.size()));
    for (const auto& property : m_properties)
    {
      property.first->write(output);
      property.second->write(output);
    }
  }

  declare_word::declare_word(
    const std::shared_ptr<base>& symbol,
    const std::shared_ptr<push_block>& quote
  )
    : m_symbol(symbol)
    , m_quote(quote) {}

  void
  declare_word::write(FILE* output) const
  {
    std::fputc(opcode::declare_word, output);
    m_symbol->write(output);
    m_quote->write(output);
  }
}
