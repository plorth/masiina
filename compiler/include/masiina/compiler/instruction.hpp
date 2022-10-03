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

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include <masiina/macros.hpp>

namespace masiina::compiler::instruction
{
  class base
  {
  public:
    explicit base();

    virtual void write(FILE* output) const = 0;

  private:
    DISALLOW_COPY_AND_ASSIGN(base);
  };

  class push_block : public base
  {
  public:
    using value_type = std::shared_ptr<base>;
    using container_type = std::vector<value_type>;

    enum class type
    {
      array = '[',
      quote = '(',
    };

    explicit push_block(enum type type, const container_type& elements);

    void write(FILE* output) const;

  private:
    const type m_type;
    const container_type m_elements;
  };

  class push_string : public base
  {
  public:
    explicit push_string(const std::u32string& value);

    void write(FILE* output) const;

  private:
    const std::u32string m_value;
  };

  class push_string_const : public base
  {
  public:
    explicit push_string_const(std::uint32_t index);

    void write(FILE* output) const;

  private:
    const std::uint32_t m_index;
  };

  class push_symbol : public base
  {
  public:
    explicit push_symbol(
      const std::u32string& id,
      std::uint32_t filename_index,
      std::uint16_t line,
      std::uint16_t column
    );

    void write(FILE* output) const;

  private:
    const std::u32string m_id;
    const std::uint32_t m_filename_index;
    const std::uint16_t m_line;
    const std::uint16_t m_column;
  };

  class push_symbol_const : public base
  {
  public:
    explicit push_symbol_const(
      std::uint32_t index,
      std::uint32_t filename_index,
      std::uint16_t line,
      std::uint16_t column
    );

    void write(FILE* output) const;

  private:
    const std::uint32_t m_index;
    const std::uint32_t m_filename_index;
    const std::uint16_t m_line;
    const std::uint16_t m_column;
  };

  class push_object : public base
  {
  public:
    using key_type = std::shared_ptr<base>;
    using mapped_type = std::shared_ptr<base>;
    using value_type = std::pair<key_type, mapped_type>;
    using container_type = std::vector<value_type>;

    explicit push_object(const container_type& properties);

    void write(FILE* output) const;

  private:
    const container_type m_properties;
  };

  class declare_word : public base
  {
  public:
    explicit declare_word(
      const std::shared_ptr<base>& symbol,
      const std::shared_ptr<push_block>& quote
    );

    void write(FILE* output) const;

  private:
    const std::shared_ptr<base> m_symbol;
    const std::shared_ptr<push_block> m_quote;
  };
}
