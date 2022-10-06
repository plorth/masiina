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
#include <masiina/compiler/module.hpp>
#include <masiina/compiler/symbol-map.hpp>
#include <masiina/opcode.hpp>
#include <plorth/parser/visitor.hpp>

namespace masiina::compiler
{
  static const std::size_t long_symbol_length = 25;

  class compile_visitor : public plorth::parser::ast::visitor<
    symbol_map&,
    std::vector<unsigned char>&
  >
  {
  public:
    void
    visit_array(
      const std::shared_ptr<plorth::parser::ast::array>& token,
      class symbol_map& symbol_map,
      std::vector<unsigned char>& output
    ) const override
    {
      const auto& elements = token->elements();

      output.push_back(static_cast<unsigned char>(opcode::push_array));
      io::write_uint32(output, static_cast<std::uint32_t>(elements.size()));
      for (const auto& element : elements)
      {
        visit(element, symbol_map, output);
      }
    }

    void
    visit_quote(
      const std::shared_ptr<plorth::parser::ast::quote>& token,
      class symbol_map& symbol_map,
      std::vector<unsigned char>& output
    ) const override
    {
      const auto& children = token->children();

      output.push_back(static_cast<unsigned char>(opcode::push_quote));
      io::write_uint32(output, static_cast<std::uint32_t>(children.size()));
      for (const auto& element : children)
      {
        visit(element, symbol_map, output);
      }
    }

    void
    visit_object(
      const std::shared_ptr<plorth::parser::ast::object>& token,
      class symbol_map& symbol_map,
      std::vector<unsigned char>& output
    ) const override
    {
      const auto& properties = token->properties();

      output.push_back(opcode::push_object);
      io::write_uint32(output, static_cast<std::uint32_t>(properties.size()));
      for (const auto& property : properties)
      {
        if (property.first.length() > long_symbol_length)
        {
          output.push_back(opcode::push_string);
          io::write_string(output, property.first);
        } else {
          output.push_back(opcode::push_string_const);
          io::write_uint32(output, symbol_map.add(property.first));
        }
        visit(property.second, symbol_map, output);
      }
    }

    void
    visit_string(
      const std::shared_ptr<plorth::parser::ast::string>& token,
      class symbol_map& symbol_map,
      std::vector<unsigned char>& output
    ) const override
    {
      const auto& value = token->value();

      if (value.length() > long_symbol_length)
      {
        output.push_back(opcode::push_string);
        io::write_string(output, value);
      } else {
        output.push_back(opcode::push_string_const);
        io::write_uint32(output, symbol_map.add(value));
      }
    }

    void
    visit_symbol(
      const std::shared_ptr<plorth::parser::ast::symbol>& token,
      class symbol_map& symbol_map,
      std::vector<unsigned char>& output
    ) const override
    {
      const auto& position = token->position();
      const auto& id = token->id();

      if (id.length() > long_symbol_length)
      {
        output.push_back(opcode::push_symbol);
        io::write_string(output, id);
      } else {
        output.push_back(opcode::push_symbol_const);
        io::write_uint32(output, symbol_map.add(id));
      }
      io::write_uint32(output, symbol_map.add(position.file));
      io::write_uint16(output, static_cast<std::uint16_t>(position.line));
      io::write_uint16(output, static_cast<std::uint16_t>(position.column));
    }

    void
    visit_word(
      const std::shared_ptr<plorth::parser::ast::word>& token,
      class symbol_map& symbol_map,
      std::vector<unsigned char>& output
    ) const override
    {
      output.push_back(opcode::declare_word);
      visit_symbol(token->symbol(), symbol_map, output);
      visit_quote(token->quote(), symbol_map, output);
    }
  };

  module::module(
    const std::u32string& name,
    const container_type& tokens
  )
    : m_name(name)
    , m_tokens(tokens) {}

  module::module(const module& that)
    : m_name(that.m_name)
    , m_tokens(that.m_tokens) {}

  module&
  module::operator=(const module& that)
  {
    m_name = that.m_name;
    m_tokens = that.m_tokens;

    return *this;
  }

  std::vector<unsigned char>
  module::compile(class symbol_map& symbol_map) const
  {
    std::vector<unsigned char> output;
    compile_visitor visitor;

    io::write_uint32(output, symbol_map.add(m_name));
    io::write_uint32(output, static_cast<std::uint32_t>(m_tokens.size()));
    for (const auto& token : m_tokens)
    {
      visitor.visit(token, symbol_map, output);
    }

    return output;
  }
}
