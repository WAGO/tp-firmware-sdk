// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "program.hpp"

#include <fcntl.h>
#include <glib.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace wago::libswitchconfig {

program::program(program &&other) noexcept {
  this->operator =(::std::move(other));
}

program& program::operator=(program &&other) noexcept {
  result = other.result;

  other.result = -1;

  return *this;
}

program program::execute(std::string &&cmd_line) {
  return execute(cmd_line);
}

program program::execute(std::string &cmd_line) {
  gchar **argv;
  gchar *stdout_data;
  gchar *stderr_data;
  GError *error = nullptr;
  program p;

  // Parse command line into argument array
  if (!g_shell_parse_argv(cmd_line.c_str(), nullptr, &argv, &error)) {
    g_error_free(error);
    return p;
  }

  // Execute with parsed arguments
  if (g_spawn_sync(nullptr, argv, nullptr, G_SPAWN_DEFAULT, nullptr, nullptr, &stdout_data, &stderr_data, &p.result,
                   &error)) {
    p.stdout = stdout_data ? stdout_data : "";
    p.stderr = stderr_data ? stderr_data : "";
    g_free(stdout_data);
    g_free(stderr_data);
  } else {
    g_error_free(error);
  }
  g_strfreev(argv);
  return p;
}

::std::string program::get_stdout() const {
  return stdout;
}

::std::string program::get_stderr() const {
  return stderr;
}

int program::get_result() const {
  return result;
}

}  // namespace wago::libswitchconfig
