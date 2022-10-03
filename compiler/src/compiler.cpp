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
#include <masiina/compiler/unit.hpp>
#include <plorth/parser/visitor.hpp>

namespace masiina::compiler
{
  namespace
  {
    class compile_visitor : public plorth::parser::ast::visitor<
      module::container_type&,
      unit&
    >
    {
    public:
      void visit_array(
        const std::shared_ptr<plorth::parser::ast::array>& token,
        module::container_type& output,
        class unit& unit
      ) const override
      {
        instruction::push_block::container_type elements;

        for (const auto& element : token->elements())
        {
          visit(element, elements, unit);
        }
        output.push_back(std::make_shared<instruction::push_block>(
          instruction::push_block::type::array,
          elements
        ));
      }

      void visit_quote(
        const std::shared_ptr<plorth::parser::ast::quote>& token,
        module::container_type& output,
        class unit& unit
      ) const override
      {
        instruction::push_block::container_type children;

        for (const auto& child : token->children())
        {
          visit(child, children, unit);
        }
        output.push_back(std::make_shared<instruction::push_block>(
          instruction::push_block::type::quote,
          children
        ));
      }

      void visit_object(
        const std::shared_ptr<plorth::parser::ast::object>& token,
        module::container_type& output,
        class unit& unit
      ) const override
      {
        instruction::push_object::container_type properties;
        module::container_type value_output;

        for (const auto& property : token->properties())
        {
          std::shared_ptr<instruction::base> key;

          if (property.first.length() > 25)
          {
            key = std::make_shared<instruction::push_string>(property.first);
          } else {
            key = std::make_shared<instruction::push_string_const>(
              unit.add_string_constant(property.first)
            );
          }
          value_output.clear();
          visit(property.second, value_output, unit);
          if (value_output.size() > 0)
          {
            properties.push_back({ key, value_output[0] });
          }
        }

        output.push_back(std::make_shared<instruction::push_object>(
          properties
        ));
      }

      void visit_string(
        const std::shared_ptr<plorth::parser::ast::string>& token,
        module::container_type& output,
        class unit& unit
      ) const override
      {
        const auto& value = token->value();

        if (value.length() > 25)
        {
          output.push_back(std::make_shared<instruction::push_string>(value));
        } else {
          output.push_back(std::make_shared<instruction::push_string_const>(
            unit.add_string_constant(value)
          ));
        }
      }

      void visit_symbol(
        const std::shared_ptr<plorth::parser::ast::symbol>& token,
        module::container_type& output,
        class unit& unit
      ) const override
      {
        const auto& id = token->id();
        const auto& position = token->position();

        if (id.length() > 25)
        {
          output.push_back(std::make_shared<instruction::push_symbol>(
            id,
            unit.add_string_constant(position.file),
            static_cast<std::uint16_t>(position.line),
            static_cast<std::uint16_t>(position.column)
          ));
        } else {
          output.push_back(std::make_shared<instruction::push_symbol_const>(
            unit.add_string_constant(id),
            unit.add_string_constant(position.file),
            static_cast<std::uint16_t>(position.line),
            static_cast<std::uint16_t>(position.column)
          ));
        }
      }

      void visit_word(
        const std::shared_ptr<plorth::parser::ast::word>& token,
        module::container_type& output,
        class unit& unit
      ) const override
      {
        const auto& id = token->symbol()->id();
        const auto& position = token->symbol()->position();
        std::shared_ptr<instruction::base> symbol;
        instruction::push_block::container_type children;

        if (id.length() > 25)
        {
          symbol = std::make_shared<instruction::push_symbol>(
            id,
            unit.add_string_constant(position.file),
            static_cast<std::uint16_t>(position.line),
            static_cast<std::uint16_t>(position.column)
          );
        } else {
          symbol = std::make_shared<instruction::push_symbol_const>(
            unit.add_string_constant(id),
            unit.add_string_constant(position.file),
            static_cast<std::uint16_t>(position.line),
            static_cast<std::uint16_t>(position.column)
          );
        }

        for (const auto& child : token->quote()->children())
        {
          visit(child, children, unit);
        }

        output.push_back(std::make_shared<instruction::declare_word>(
          symbol,
          std::make_shared<instruction::push_block>(
            instruction::push_block::type::quote,
            children
          )
        ));
      }
    };
  }

  void
  unit::compile(
    const std::u32string& module_name,
    const std::vector<std::shared_ptr<plorth::parser::ast::token>>& tokens
  )
  {
    compile_visitor visitor;
    instruction::push_block::container_type instructions;

    for (const auto& token : tokens)
    {
      visitor.visit(token, instructions, *this);
    }

    m_modules.push_back(std::make_shared<module>(
      add_string_constant(module_name),
      instructions
    ));
  }
}
