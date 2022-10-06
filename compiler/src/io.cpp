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
#include <peelo/unicode/encoding/utf8.hpp>

#if !defined(BUFSIZ)
# define BUFSIZ 1024
#endif

namespace masiina::compiler::io
{
  std::optional<std::string>
  read_file_contents(const std::string& path)
  {
    FILE* input = std::fopen(path.c_str(), "rb");
    std::string result;
    char buffer[BUFSIZ];

    if (!input)
    {
      return std::nullopt;
    }
    for (;;)
    {
      const auto read = std::fread(
        static_cast<void*>(buffer),
        1,
        BUFSIZ,
        input
      );

      if (read > 0)
      {
        result.append(buffer, read);
      } else {
        break;
      }
    }
    std::fclose(input);

    return result;
  }

  void
  write_uint16(std::vector<unsigned char>& output, std::uint16_t number)
  {
    output.push_back(static_cast<unsigned char>((number >> 0) & 0xff));
    output.push_back(static_cast<unsigned char>((number >> 8) & 0xff));
  }

  void
  write_uint32(FILE* output, std::uint32_t number)
  {
    std::fputc((number >> 0) & 0xff, output);
    std::fputc((number >> 8) & 0xff, output);
    std::fputc((number >> 16) & 0xff, output);
    std::fputc((number >> 24) & 0xff, output);
  }

  void
  write_uint32(std::vector<unsigned char>& output, std::uint32_t number)
  {
    output.push_back(static_cast<unsigned char>((number >> 0) & 0xff));
    output.push_back(static_cast<unsigned char>((number >> 8) & 0xff));
    output.push_back(static_cast<unsigned char>((number >> 16) & 0xff));
    output.push_back(static_cast<unsigned char>((number >> 24) & 0xff));
  }

  void
  write_string(FILE* output, const std::u32string& str)
  {
    const auto encoded_str = peelo::unicode::encoding::utf8::encode(str);

    write_uint32(output, static_cast<std::uint32_t>(encoded_str.length()));
    for (const auto& c : encoded_str)
    {
      std::fputc(static_cast<unsigned char>(c), output);
    }
  }

  void
  write_string(std::vector<unsigned char>& output, const std::u32string& str)
  {
    const auto encoded_str = peelo::unicode::encoding::utf8::encode(str);

    write_uint32(output, static_cast<std::uint32_t>(encoded_str.length()));
    for (const auto& c : encoded_str)
    {
      output.push_back(static_cast<unsigned char>(c));
    }
  }
}
