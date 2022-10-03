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
#include <masiina/runtime/io.hpp>
#include <peelo/unicode/encoding/utf8.hpp>

namespace masiina::runtime::io
{
  bool
  read_uint16(FILE* input, std::uint16_t& number)
  {
    char buffer[2];
    const auto read = std::fread(static_cast<void*>(buffer), 2, 1, input);

    if (read != 1)
    {
      return false;
    }

    number = static_cast<std::uint16_t>(
      (buffer[0] << 0)
      + (buffer[1] << 8)
    );

    return true;
  }

  bool
  read_uint32(FILE* input, std::uint32_t& number)
  {
    char buffer[4];
    const auto read = std::fread(static_cast<void*>(buffer), 4, 1, input);

    if (read != 1)
    {
      return false;
    }

    number = static_cast<std::uint32_t>(
      (buffer[0] << 0)
      + (buffer[1] << 8)
      + (buffer[2] << 16)
      + (buffer[3] << 24)
    );

    return true;
  }

  bool
  read_string(FILE* input, std::u32string& str)
  {
    std::uint32_t length;

    if (!read_uint32(input, length))
    {
      return false;
    }

    if (length > 0)
    {
      char buffer[length];
      const auto read = std::fread(
        static_cast<void*>(buffer),
        length,
        1,
        input
      );

      if (read != 1)
      {
        return false;
      }
      str.assign(peelo::unicode::encoding::utf8::decode(buffer, length));
    } else {
      str.clear();
    }

    return true;
  }
}
