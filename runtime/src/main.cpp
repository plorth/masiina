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
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <masiina/runtime/config.hpp>
#include <masiina/runtime/environment.hpp>
#include <masiina/runtime/parser.hpp>
#include <masiina/version.hpp>
#include <peelo/unicode/encoding/utf8.hpp>
#include <plorth/runtime.hpp>

#if defined(HAVE_SYSEXITS_H)
# include <sysexits.h>
#endif
#if defined(HAVE_FORK)
# include <unistd.h>
#endif

#if !defined(EX_USAGE)
# define EX_USAGE 64
#endif

static std::string input_path;
static std::vector<std::u32string> arguments;
static bool use_fork = false;

static void
print_usage(const char* executable)
{
  std::cerr
    << std::endl
    << "Usage: "
    << executable
    << " [switches] <filename> [arguments...]"
    << std::endl
    << "  -f        Fork to background before executing program." << std::endl
    << "  --version Print the version." << std::endl
    << "  --help    Display this message." << std::endl;
}

static void
scan_arguments(int argc, char** argv)
{
  int offset = 1;

  while (offset < argc)
  {
    const auto arg = argv[offset++];

    if (!*arg)
    {
      continue;
    }
    else if (*arg != '-')
    {
      if (input_path.empty())
      {
        input_path = arg;
      }
      break;
    }
    else if (!arg[1])
    {
      break;
    }
    else if (arg[1] == '-')
    {
      if (!std::strcmp(arg, "--help"))
      {
        print_usage(argv[0]);
        std::exit(EXIT_SUCCESS);
      }
      else if (!std::strcmp(arg, "--version"))
      {
        std::cout
          << "Masiina "
          << MASIINA_VERSION_MAJOR
          << "."
          << MASIINA_VERSION_MINOR
          << "."
          << MASIINA_VERSION_PATCH
          << std::endl;
        std::exit(EXIT_SUCCESS);
      } else {
        std::cerr << "Unrecognized switch: " << arg << std::endl;
        print_usage(argv[0]);
        std::exit(EX_USAGE);
      }
    }
    for (int i = 1; arg[i]; ++i)
    {
      switch (arg[i])
      {
        case 'f':
          use_fork = true;
          break;

        case 'h':
          print_usage(argv[0]);
          std::exit(EXIT_SUCCESS);
          break;

        default:
          std::cerr << "Unrecognized switch: `" << arg[i] << "'" << std::endl;
          print_usage(argv[0]);
          std::exit(EX_USAGE);
      }
    }
  }

  while (offset < argc)
  {
    arguments.push_back(peelo::unicode::encoding::utf8::decode(argv[offset++]));
  }
}

int
main(int argc, char** argv)
{
  masiina::runtime::environment env;
  std::vector<std::shared_ptr<masiina::runtime::module>> modules;
  std::shared_ptr<masiina::runtime::module> main_module;
  bool error_occurred = false;

  scan_arguments(argc, argv);

  if (input_path.empty())
  {
    print_usage(argv[0]);
    std::exit(EX_USAGE);
  }

  env.runtime()->arguments() = arguments;

  const auto import_result = masiina::runtime::parser::parse_file(
    env.runtime(),
    input_path
  );

  if (import_result)
  {
    const auto& modules = import_result.value();

    if (modules->size() > 0)
    {
      main_module = (*modules)[0];
    }
    for (const auto& module : *modules)
    {
      env.add_imported_module(module);
    }
  } else {
    const auto& error = import_result.error();

    if (error)
    {
      std::cerr << *error << std::endl;
    } else {
      std::cerr << "Unknown error." << std::endl;
    }
    std::exit(EXIT_FAILURE);
  }

  if (main_module)
  {
    env.spawn(main_module->values());
  }

  if (use_fork)
  {
#if defined(HAVE_FORK)
    if (fork())
    {
      std::exit(EXIT_SUCCESS);
    }
#else
    std::cerr << "Forking to background is not supported on this platform." << std::endl;
#endif
  }

  while (!env.is_finished())
  {
    if (!env.step())
    {
      error_occurred = true;
    }
  }

  return error_occurred ? EXIT_FAILURE : EXIT_SUCCESS;
}
