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

#include <masiina/opcode.hpp>
#include <masiina/runtime/io.hpp>
#include <masiina/runtime/parser.hpp>
#include <masiina/version.hpp>

namespace masiina::runtime::parser
{
  using symbol_map = std::unordered_map<std::uint32_t, std::u32string>;

  static bool check_magic_number(FILE*);
  static std::optional<std::string> check_version_number(FILE*);
  static bool parse_symbol_map(FILE*, symbol_map&);
  static std::shared_ptr<plorth::value> parse_instruction(
    FILE*,
    const std::shared_ptr<plorth::runtime>&,
    const symbol_map&
  );
  static std::optional<std::string> parse_module(
    FILE*,
    const std::shared_ptr<plorth::runtime>&,
    const symbol_map&,
    std::vector<std::shared_ptr<module>>&
  );

  result_type
  parse_file(
    const std::shared_ptr<plorth::runtime>& runtime,
    const std::string& path
  )
  {
    FILE* input = std::fopen(path.c_str(), "rb");
    symbol_map symbol_map;
    std::uint32_t module_count;
    std::vector<std::shared_ptr<module>> modules;

    if (!input)
    {
      return result_type::error(
        "Unable to open file `"
        + path
        + "' for reading: "
        + std::strerror(errno)
      );
    }

    if (!check_magic_number(input))
    {
      std::fclose(input);

      return result_type::error("Magic number mismatch.");
    }

    if (const auto error = check_version_number(input))
    {
      std::fclose(input);

      return result_type::error(*error);
    }

    if (!parse_symbol_map(input, symbol_map))
    {
      std::fclose(input);

      return result_type::error("Unable to process symbol table.");
    }

    if (!io::read_uint32(input, module_count))
    {
      std::fclose(input);

      return result_type::error("Unable to determine module count.");
    }

    for (std::uint32_t i = 0; i < module_count; ++i)
    {
      const auto error = parse_module(input, runtime, symbol_map, modules);

      if (error)
      {
        std::fclose(input);

        return result_type::error(*error);
      }
    }

    std::fclose(input);

    return result_type::ok(modules);
  }

  static bool
  check_magic_number(FILE* input)
  {
    char buffer[3];
    const auto read = std::fread(static_cast<void*>(buffer), 3, 1, input);

    if (read != 1)
    {
      return false;
    }

    return buffer[0] == 'R' && buffer[1] == 'j' && buffer[2] == 'L';
  }

  static std::optional<std::string>
  check_version_number(FILE* input)
  {
    char buffer[3];
    const auto read = std::fread(static_cast<void*>(buffer), 3, 1, input);

    if (read != 1)
    {
      return std::make_optional<std::string>("Unable to parse version number.");
    }

    if (buffer[2] > MASIINA_VERSION_MAJOR)
    {
      return std::make_optional<std::string>("Incompatible version number.");
    }

    return std::nullopt;
  }

  static bool
  parse_symbol_map(FILE* input, symbol_map& map)
  {
    std::uint32_t size;

    if (!io::read_uint32(input, size))
    {
      return false;
    }
    for (std::uint32_t i = 0; i < size; ++i)
    {
      std::u32string str;

      if (!io::read_string(input, str))
      {
        return false;
      }
      map[i] = str;
    }

    return true;
  }

  static std::shared_ptr<plorth::array>
  parse_array(
    FILE* input,
    const std::shared_ptr<plorth::runtime>& runtime,
    const symbol_map& symbol_map
  )
  {
    std::uint32_t size;
    std::vector<std::shared_ptr<plorth::value>> elements;

    if (!io::read_uint32(input, size))
    {
      return nullptr;
    }

    elements.reserve(size);

    for (std::uint32_t i = 0; i < size; ++i)
    {
      const auto element = parse_instruction(input, runtime, symbol_map);

      if (!element)
      {
        return nullptr;
      }
      elements.push_back(element);
    }

    return runtime->array(elements.data(), elements.size());
  }

  static std::shared_ptr<plorth::quote>
  parse_quote(
    FILE* input,
    const std::shared_ptr<plorth::runtime>& runtime,
    const symbol_map& symbol_map
  )
  {
    std::uint32_t size;
    std::vector<std::shared_ptr<plorth::value>> children;

    if (!io::read_uint32(input, size))
    {
      return nullptr;
    }

    children.reserve(size);

    for (std::uint32_t i = 0; i < size; ++i)
    {
      const auto child = parse_instruction(input, runtime, symbol_map);

      if (!child)
      {
        return nullptr;
      }
      children.push_back(child);
    }

    return runtime->compiled_quote(children);
  }

  static std::shared_ptr<plorth::object>
  parse_object(
    FILE* input,
    const std::shared_ptr<plorth::runtime>& runtime,
    const symbol_map& symbol_map
  )
  {
    std::uint32_t size;
    std::vector<plorth::object::value_type> properties;

    if (!io::read_uint32(input, size))
    {
      return nullptr;
    }

    for (std::uint32_t i = 0; i < size; ++i)
    {
      const int j = std::fgetc(input);
      std::u32string key;
      std::shared_ptr<plorth::value> value;

      switch (j)
      {
        case opcode::push_string_const:
        {
          std::uint32_t index;

          if (!io::read_uint32(input, index))
          {
            return nullptr;
          }

          const auto k = symbol_map.find(index);

          if (k != std::end(symbol_map))
          {
            key = k->second;
          } else {
            return nullptr;
          }
          break;
        }

        case opcode::push_string:
          if (!io::read_string(input, key))
          {
            return nullptr;
          }
          break;

        default:
          return nullptr;
      }
      if (!(value = parse_instruction(input, runtime, symbol_map)))
      {
        return nullptr;
      }
      properties.push_back({ key, value });
    }

    return runtime->object(properties);
  }

  static std::shared_ptr<plorth::string>
  parse_string(FILE* input, const std::shared_ptr<plorth::runtime>& runtime)
  {
    std::u32string id;

    if (!io::read_string(input, id))
    {
      return nullptr;
    }

    return runtime->string(id);
  }

  static std::shared_ptr<plorth::string>
  parse_string_const(
    FILE* input,
    const std::shared_ptr<plorth::runtime>& runtime,
    const symbol_map& symbol_map
  )
  {
    std::uint32_t index;

    if (io::read_uint32(input, index))
    {
      const auto i = symbol_map.find(index);

      if (i != std::end(symbol_map))
      {
        return runtime->string(i->second);
      }
    }

    return nullptr;
  }

  static bool
  parse_position(
    FILE* input,
    const symbol_map& symbol_map,
    plorth::parser::position& position
  )
  {
    std::uint32_t filename_index;
    symbol_map::const_iterator filename_iterator;
    std::uint16_t line;
    std::uint16_t column;

    if (!io::read_uint32(input, filename_index))
    {
      return false;
    }

    if ((filename_iterator = symbol_map.find(filename_index)) == std::end(symbol_map))
    {
      return false;
    }

    if (!io::read_uint16(input, line))
    {
      return false;
    }

    if (!io::read_uint16(input, column))
    {
      return false;
    }

    position.file = filename_iterator->second;
    position.line = static_cast<int>(line);
    position.column = static_cast<int>(column);

    return true;
  }

  static std::shared_ptr<plorth::symbol>
  parse_symbol(
    FILE* input,
    const std::shared_ptr<plorth::runtime>& runtime,
    const symbol_map& symbol_map
  )
  {
    std::u32string id;
    plorth::parser::position position;

    if (!io::read_string(input, id))
    {
      return nullptr;
    }

    if (!parse_position(input, symbol_map, position))
    {
      return nullptr;
    }

    return runtime->symbol(id, position);
  }

  static std::shared_ptr<plorth::symbol>
  parse_symbol_const(
    FILE* input,
    const std::shared_ptr<plorth::runtime>& runtime,
    const symbol_map& symbol_map
  )
  {
    std::uint32_t index;
    symbol_map::const_iterator iterator;
    plorth::parser::position position;

    if (!io::read_uint32(input, index))
    {
      return nullptr;
    }

    if ((iterator = symbol_map.find(index)) == std::end(symbol_map))
    {
      return nullptr;
    }

    if (!parse_position(input, symbol_map, position))
    {
      return nullptr;
    }

    return runtime->symbol(iterator->second, position);
  }

  static std::shared_ptr<plorth::word>
  parse_word_declaration(
    FILE* input,
    const std::shared_ptr<plorth::runtime>& runtime,
    const symbol_map& symbol_map
  )
  {
    std::shared_ptr<plorth::symbol> symbol;
    std::shared_ptr<plorth::quote> quote;

    switch (std::fgetc(input))
    {
      case opcode::push_symbol:
        symbol = parse_symbol(input, runtime, symbol_map);
        break;

      case opcode::push_symbol_const:
        symbol = parse_symbol_const(input, runtime, symbol_map);
        break;
    }

    if (!symbol)
    {
      return nullptr;
    }

    if (std::fgetc(input) != opcode::push_quote)
    {
      return nullptr;
    }

    if (!(quote = parse_quote(input, runtime, symbol_map)))
    {
      return nullptr;
    }

    return runtime->word(symbol, quote);
  }

  static std::shared_ptr<plorth::value>
  parse_instruction(
    FILE* input,
    const std::shared_ptr<plorth::runtime>& runtime,
    const symbol_map& symbol_map
  )
  {
    int opcode = std::fgetc(input);

    if (opcode == EOF)
    {
      return nullptr;
    }
    switch (static_cast<unsigned char>(opcode))
    {
      case opcode::push_array:
        return parse_array(input, runtime, symbol_map);

      case opcode::push_quote:
        return parse_quote(input, runtime, symbol_map);

      case opcode::push_object:
        return parse_object(input, runtime, symbol_map);

      case opcode::push_string:
        return parse_string(input, runtime);

      case opcode::push_string_const:
        return parse_string_const(input, runtime, symbol_map);

      case opcode::push_symbol:
        return parse_symbol(input, runtime, symbol_map);

      case opcode::push_symbol_const:
        return parse_symbol_const(input, runtime, symbol_map);

      case opcode::declare_word:
        return parse_word_declaration(input, runtime, symbol_map);
    }

    return nullptr;
  }

  static bool
  parse_module_name(
    FILE* input,
    const symbol_map& symbol_map,
    std::u32string& name
  )
  {
    std::uint32_t index;
    symbol_map::const_iterator iterator;

    if (!io::read_uint32(input, index))
    {
      return false;
    }

    if ((iterator = symbol_map.find(index)) == std::end(symbol_map))
    {
      return false;
    }

    name = iterator->second;

    return true;
  }

  static std::optional<std::string>
  parse_module(
    FILE* input,
    const std::shared_ptr<plorth::runtime>& runtime,
    const symbol_map& symbol_map,
    std::vector<std::shared_ptr<module>>& container
  )
  {
    std::u32string name;
    std::uint32_t size;
    module::container_type values;

    if (!parse_module_name(input, symbol_map, name))
    {
      return std::make_optional<std::string>("Unable to import module name.");
    }

    if (!io::read_uint32(input, size))
    {
      return std::make_optional<std::string>("Unable to import module size.");
    }

    for (std::uint32_t i = 0; i < size; ++i)
    {
      const auto value = parse_instruction(input, runtime, symbol_map);

      if (!value)
      {
        return std::make_optional<std::string>("Unable to import module.");
      }
      values.push_back(value);
    }

    container.push_back(std::make_shared<module>(name, values));

    return std::nullopt;
  }
}
