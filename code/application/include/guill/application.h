#pragma once
#include <functional>
#include <memory>
#include "absl/strings/string_view.h"

struct SDL_Window;
union SDL_Event;
namespace guill {
class Application {
 public:
  using InitCallback = std::function<void(Application *app)>;
  using DelayShutdownCallback = std::function<bool(Application *app)>;
  using ShutdownCallback = std::function<void(Application *app)>;
  using RenderCallback = std::function<void(SDL_Window *)>;
  using EventCallback = std::function<void(const SDL_Event &)>;
  using TickCallback = std::function<void(void)>;

  struct Config {
    absl::string_view window_name;
    InitCallback init_callback;
    DelayShutdownCallback delay_shutdown_callback;
    ShutdownCallback shutdown_callback;
    RenderCallback render_callback;
    EventCallback event_callback;
    TickCallback tick_callback;
    int suggested_width = 0;
    int suggested_height = 0;
  };

  explicit Application(const Config &config);
  ~Application();

  void RunUntilQuit();

  void Tick();
  void Quit();
  bool HasQuit() const;

  SDL_Window &GetWindow();
  void *GetGLContext();

 private:
  class Internal;
  std::unique_ptr<Internal> internal_;
};
}  // namespace guill
