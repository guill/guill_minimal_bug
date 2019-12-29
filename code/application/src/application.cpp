#include "guill/application.h"

#include <cassert>

#include "SDL.h"
#include "absl/memory/memory.h"
#include "glad/glad.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#include "emscripten/html5.h"
#define TRACK_WINDOWS 1
#endif

#if TRACK_WINDOWS
#include <unordered_map>
namespace {
std::unordered_map<int, SDL_Window *> windows_by_id;
int window_id_gen = 1;
}  // namespace
#endif

//=============================================================================
//                                    Using                                   =
//=============================================================================
using guill::Application;

//=============================================================================
//                            Application::Internal                           =
//=============================================================================
class Application::Internal {
 public:
  explicit Internal(Application *app, const Config &config);
  ~Internal();
  void Init(Application *app);
  void Shutdown(Application *app);

  void RunUntilQuit();

  void Tick();
  void Quit();
  bool HasQuit() const;
  SDL_Window &GetWindow() { return *window_; }
  void *GetGLContext() { return glcontext_; }

 private:
  InitCallback init_callback_;
  DelayShutdownCallback delay_shutdown_callback_;
  ShutdownCallback shutdown_callback_;
  RenderCallback render_callback_;
  EventCallback event_callback_;
  TickCallback tick_callback_;
  bool has_quit_ = false;
  SDL_GLContext glcontext_;
  SDL_Window *window_;
  Application *app_;
#if TRACK_WINDOWS
  int window_id_ = 0;
#endif
};

#ifdef __EMSCRIPTEN__
EM_JS(int, get_em_window_width, (), { return window.innerWidth; });
EM_JS(int, get_em_window_height, (), { return window.innerHeight; });
EM_JS(int, get_em_consumed_height, (), {
  let debugpanel = document.getElementById("debugpanel");
  if (debugpanel == null) {
    return 0;
  }
  return debugpanel.offsetHeight;
});
#endif

Application::Internal::Internal(Application *app, const Config &config)
    : init_callback_(config.init_callback),
      delay_shutdown_callback_(config.delay_shutdown_callback),
      shutdown_callback_(config.shutdown_callback),
      render_callback_(config.render_callback),
      event_callback_(config.event_callback),
      tick_callback_(config.tick_callback),
      app_(app) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    has_quit_ = true;
    assert(false);
    return;
  }

  int width = config.suggested_width;
  int height = config.suggested_height;

  // Setup window
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#ifdef USING_OPENGL_ES
#ifdef __EMSCRIPTEN__
  // Awful bug, see https://github.com/emscripten-ports/SDL2/issues/59
  // We should remove places we force USE_WEBGL2=1 like imgui
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
#endif

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                      SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif
  SDL_DisplayMode current;
  SDL_GetCurrentDisplayMode(0, &current);

#ifdef __EMSCRIPTEN__
  // Ignore size suggestions for web views. Use the size of the window.
  // IMPORTANT - The "window" here is NOT C++ code.
  width = get_em_window_width();
  height = get_em_window_height() - get_em_consumed_height();
#endif

  window_ = SDL_CreateWindow(config.window_name.data(), SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, width, height,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  assert(window_);
#if TRACK_WINDOWS
  window_id_ = window_id_gen++;
  windows_by_id.insert(std::pair<int, SDL_Window *>(window_id_, window_));
#endif
  glcontext_ = SDL_GL_CreateContext(window_);
  assert(glcontext_);

  SDL_GL_MakeCurrent(window_, glcontext_);

// Initialize OpenGL
#ifdef USING_OPENGL_ES
  gladLoadGLES2Loader(SDL_GL_GetProcAddress);
#else
  gladLoadGLLoader(SDL_GL_GetProcAddress);
#endif

#ifdef __EMSCRIPTEN__
  emscripten_set_resize_callback(
      nullptr, reinterpret_cast<void *>(static_cast<intptr_t>(window_id_)),
      false,
      [](int event_type, const EmscriptenUiEvent *ui_event,
         void *user_data) -> int {
        if (event_type == EMSCRIPTEN_EVENT_RESIZE) {
          intptr_t window_id = reinterpret_cast<intptr_t>(user_data);
          auto it = windows_by_id.find(static_cast<int>(window_id));
          if (it != windows_by_id.end()) {
            SDL_Window *window = it->second;
            SDL_SetWindowSize(
                window, ui_event->windowInnerWidth,
                ui_event->windowInnerHeight - get_em_consumed_height());
            return 1;
          }
        }
        return 0;
      });
#endif
}

Application::Internal::~Internal() {
#if TRACK_WINDOWS
  windows_by_id.erase(window_id_);
#endif
  SDL_GL_DeleteContext(glcontext_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

void Application::Internal::Init(Application *app) {
  if (init_callback_) {
    init_callback_(app);
  }
}
void Application::Internal::Shutdown(Application *app) {
  if (shutdown_callback_) {
    shutdown_callback_(app);
  }
}

void Application::Internal::RunUntilQuit() {
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(
      [](void *user_data) {
        Application::Internal *application =
            static_cast<Application::Internal *>(user_data);
        application->Tick();
      },
      this, 0, 1);
#else
  while (!HasQuit()) {
    Tick();
  }
#endif
}

void Application::Internal::Tick() {
  if (HasQuit()) return;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event_callback_) {
      event_callback_(event);
    }
    if (event.type == SDL_QUIT ||
        (event.type == SDL_WINDOWEVENT &&
         event.window.event == SDL_WINDOWEVENT_CLOSE &&
         event.window.windowID == SDL_GetWindowID(window_))) {
      Quit();
    }
  }

  if (render_callback_) {
    render_callback_(window_);
  }
  SDL_GL_MakeCurrent(window_, glcontext_);
  SDL_GL_SwapWindow(window_);

  if (tick_callback_) {
    tick_callback_();
  }
}

void Application::Internal::Quit() { has_quit_ = true; }

bool Application::Internal::HasQuit() const {
  if (!has_quit_) {
    return false;
  }

  if (!delay_shutdown_callback_) {
    return true;
  }

  return !delay_shutdown_callback_(app_);
}

//=============================================================================
//                                 Application                                =
//=============================================================================
Application::Application(const Config &config)
    : internal_(absl::make_unique<Internal>(this, config)) {
  internal_->Init(this);
}

Application::~Application() { internal_->Shutdown(this); }

void Application::RunUntilQuit() { internal_->RunUntilQuit(); }

void Application::Tick() { internal_->Tick(); }

void Application::Quit() { internal_->Quit(); }

bool Application::HasQuit() const { return internal_->HasQuit(); }

SDL_Window &Application::GetWindow() { return internal_->GetWindow(); }
void *Application::GetGLContext() { return internal_->GetGLContext(); }
