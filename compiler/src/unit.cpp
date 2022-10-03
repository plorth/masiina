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
#include <cerrno>
#include <cstring>

#include <masiina/compiler/io.hpp>
#include <masiina/compiler/unit.hpp>
#include <masiina/version.hpp>
#include <peelo/unicode/encoding/utf8.hpp>
#include <plorth/parser.hpp>

namespace masiina::compiler
{
  unit::unit() {}

  std::optional<std::string>
  unit::compile_file(const std::string& path)
  {
    const auto decoded_path = peelo::unicode::encoding::utf8::decode(path);
    const auto raw_source = io::read_file_contents(path);
    std::u32string source;
    plorth::parser::position position = { decoded_path, 1, 0 };

    if (!raw_source)
    {
      return std::make_optional<std::string>(
        "Unable to open file `"
        + path
        + "' for reading: "
        + std::strerror(errno)
      );
    }

    if (!peelo::unicode::encoding::utf8::decode_validate(*raw_source, source))
    {
      return std::make_optional<std::string>(
        "Unable to decode contents of `"
        + path
        + "' with UTF-8 character encoding."
      );
    }

    auto begin = std::cbegin(source);
    const auto end = std::cend(source) - 1;
    const auto result = plorth::parser::parse(begin, end, position);

    if (result)
    {
      const auto& tokens = result.value();

      if (tokens)
      {
        compile(decoded_path, *tokens);
      }
    } else {
      const auto& error = result.error();
      std::string message;

      if (error)
      {
        message =
          peelo::unicode::encoding::utf8::encode(error->position.file)
          + ":"
          + std::to_string(error->position.line)
          + ":"
          + std::to_string(error->position.column)
          + ": "
          + peelo::unicode::encoding::utf8::encode(error->message);
      } else {
        message = "Unknown error.";
      }

      return std::make_optional<std::string>(message);
    }

    return std::nullopt;
  }

  std::uint32_t
  unit::add_string_constant(const std::u32string& str)
  {
    const auto index = m_symbol_map.find(str);
    std::uint32_t new_index;

    if (index != std::end(m_symbol_map))
    {
      return index->second;
    }
    new_index = static_cast<std::uint32_t>(m_symbol_list.size());
    m_symbol_list.push_back(str);
    m_symbol_map[str] = new_index;

    return new_index;
  }

  void
  unit::write(FILE* output) const
  {
    // Magic number.
    std::fputs("RjL", output);

    // Version number.
    std::fputc(MASIINA_VERSION_PATCH, output);
    std::fputc(MASIINA_VERSION_MINOR, output);
    std::fputc(MASIINA_VERSION_MAJOR, output);

    // Symbol table.
    io::write_uint32(output, static_cast<std::uint32_t>(m_symbol_list.size()));
    for (const auto& symbol : m_symbol_list)
    {
      io::write_string(output, symbol);
    }

    // All modules contained in the compilation unit.
    io::write_uint32(output, static_cast<std::uint32_t>(m_modules.size()));
    for (const auto& module : m_modules)
    {
      module->write(output);
    }
  }
}
