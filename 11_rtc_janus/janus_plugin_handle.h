#pragma once

#include <cstdint>
#include <string>

struct JanusPluginHandle {
  std::string name;
  int64_t handle_id;
};
