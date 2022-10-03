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
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <masiina/compiler/config.hpp>
#include <masiina/compiler/unit.hpp>
#include <masiina/version.hpp>

#if defined(HAVE_SYSEXITS_H)
# include <sysexits.h>
#endif

#if !defined(EX_USAGE)
# define EX_USAGE 64
#endif

static std::vector<std::string> input_paths;
static std::string output_path;

static void
print_usage(const char* executable)
{
  std::cerr
    << std::endl
    << "Usage: "
    << executable
    << " [switches] <filename...>"
    << std::endl
    << "  -o <path> Where to write the compiled bytecode to." << std::endl
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

    if (*arg == '-')
    {
      if (!arg[1])
      {
        std::cerr << "Unrecognized switch: `-'" << std::endl;
        print_usage(argv[0]);
        std::exit(EX_USAGE);
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
      } else {
        for (int i = 1; arg[i]; ++i)
        {
          switch (arg[i])
          {
            case 'o':
              if (offset < argc)
              {
                output_path = argv[offset++];
              } else {
                std::cerr << "Argument expected for the -o option." << std::endl;
                print_usage(argv[0]);
                std::exit(EX_USAGE);
              }
              break;

            case 'h':
              print_usage(argv[0]);
              std::exit(EXIT_SUCCESS);
              break;
          }
        }
      }
    } else {
      input_paths.push_back(arg);
    }
  }
}

int
main(int argc, char** argv)
{
  FILE* output;
  masiina::compiler::unit unit;

  scan_arguments(argc, argv);

  if (input_paths.empty() || output_path.empty())
  {
    print_usage(argv[0]);
    std::exit(EX_USAGE);
  }

  for (const auto& path : input_paths)
  {
    const auto error = unit.compile_file(path);

    if (error)
    {
      std::cerr << *error << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  if (!(output = std::fopen(output_path.c_str(), "wb")))
  {
    std::cerr
      << "Couldn't open file `"
      << argv[argc - 1]
      << "' for writing: "
      << std::strerror(errno)
      << std::endl;
    std::exit(EXIT_FAILURE);
  }

  unit.write(output);
  std::fclose(output);

  return EXIT_SUCCESS;
}
