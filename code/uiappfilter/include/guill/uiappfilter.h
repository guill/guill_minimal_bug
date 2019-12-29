#pragma once

#include <functional>

#include "guill/application.h"

typedef union SDL_Event SDL_Event;

namespace guill {
namespace UiAppFilter {

struct Config {
  // Arguments: scale, width, height
  std::function<void(float, int, int)> imgui_callback;
  std::function<void(float, int, int)> extra_render_callback;  // opengl stuff
  std::function<void(void)> tick_callback;

  // Return true and Imgui won't process the event
  std::function<bool(const SDL_Event &event)> event_callback;

  float imgui_scale = 1.0f;
};

void Apply(const Config &config, Application::Config *app_config);

}  // namespace UiAppFilter
}  // namespace guill
