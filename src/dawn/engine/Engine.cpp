/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2017 (git@dga.me.uk)
 */
#include "Common.h"
#include "engine/App.h"
#include "core/Timer.h"
#include "DawnEngine.h"

// Required for getBasePath/getPrefPath.
#if DW_PLATFORM == DW_WIN32
#include <Psapi.h>
#elif DW_PLATFORM == DW_MACOS
#include <CoreFoundation/CoreFoundation.h>
#define MAX_PATH 256
#elif DW_PLATFORM == DW_LINUX
#include <unistd.h>
#endif

namespace dw {

Engine::Engine(const String& game, const String& version)
    : Object{nullptr},
      initialised_{false},
      running_{true},
      save_config_on_exit_{true},
      game_name_{game},
      game_version_{version},
      frame_time_{0.0f},
      log_file_{"engine.log"},
      config_file_{"engine.cfg"} {
}

Engine::~Engine() {
    shutdown();
}

void Engine::setup(int argc, char** argv) {
    assert(!initialised_);

    // Process command line arguments.
    Set<String> flags;
    Map<String, String> arguments;
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') {
            // Look ahead to next arg. If flag then add this arg as flag, otherwise add argument
            // pair
            if (i < argc - 1) {
                if (argv[i + 1][0] == '-') {
                    flags.insert(String{argv[i]});
                } else {
                    arguments[String{argv[i]}] = String{argv[i + 1]};
                    i++;  // skip argument value
                }
            } else {
                flags.insert(String{argv[i]});
            }
        }
    }

    // Create context.
    context_ = new Context(basePath(), "");

    // Initialise file system.
    context_->addSubsystem<FileSystem>();

    // Initialise logging.
    context_->addSubsystem<Logger>();
// TODO(david): Add a file logger to prefPath + log_file_
#ifdef DW_DEBUG
    log().warn("NOTE: This is a debug build!");
#endif

    // Update working directory.
    context_->subsystem<FileSystem>()->setWorkingDir(context_->basePath());

    // Print info.
    log().info("Initialising engine " DW_VERSION_STR);
    printSystemInfo();
    if (flags.size() > 0) {
        log().info("Flags:");
        for (auto& flag : flags) {
            log().info("\t%s", flag);
        }
    }
    if (arguments.size() > 0) {
        log().info("Arguments:");
        for (auto& arg : arguments) {
            log().info("\t%s %s", arg.first, arg.second);
        }
    }

    // Build window title.
    String window_title{game_name_};
    window_title += " ";
    window_title += game_version_;
#ifdef DW_DEBUG
    window_title += " (debug)";
#endif

    // Low-level subsystems
    context_->addSubsystem<EventSystem>();

    // Load configuration
    if (context_->subsystem<FileSystem>()->fileExists(config_file_)) {
        log().info("Loading configuration from %s", config_file_);
        context_->loadConfig(config_file_);
    } else {
        log().info("Configuration does not exist, creating %s", config_file_);
        context_->setDefaultConfig();
    }

    // Initialise the Lua VM first so bindings can be defined in Constructors
    context_->addSubsystem<LuaState>();
    // TODO(David): bind engine services to lua?

    // Create the engine subsystems.
    auto* renderer = context_->addSubsystem<Renderer>();
    if (flags.find("-norenderer") == flags.end()) {
        renderer->init(RendererType::OpenGL, context_->config().at("window_width").get<u16>(),
                       context_->config().at("window_height").get<u16>(), window_title, true);
        context_->addSubsystem<Input>();
    } else {
        renderer->init(RendererType::Null, context_->config().at("window_width").get<u16>(),
                       context_->config().at("window_height").get<u16>(), window_title, false);
    }
    context_->addSubsystem<UserInterface>();
    context_->addSubsystem<ResourceCache>();
    context_->addSubsystem<SystemManager>();
    context_->addSubsystem<Universe>();
    // mAudio = new Audio;
    context_->addSubsystem<PhysicsSystem>();
    // mStarSystem = new StarSystem(mRenderer, mPhysicsWorld);
    context_->addSubsystem<GameFramework>();

    auto* net = context_->addSubsystem<NetSystem>();
    if (arguments.find("-host") != arguments.end()) {
        net->listen(std::stoi(arguments["-host"]), 32);
    }
    else if (arguments.find("-join") != arguments.end()) {
        String ip = arguments["-join"];
        u16 port = std::stoi(arguments["-p"]);
        net->connect(ip, port);
    }

    // Set up built in entity systems.
    auto& sm = *context_->subsystem<SystemManager>();
    sm.addSystem<EntityRenderer>();

    // Set input viewport size
    /*
    subsystem<Input>()->setViewportSize(subsystem<Renderer>()->getViewportSize());

    // Enumerate available video modes
    Vector<SDL_DisplayMode> displayModes = subsystem<Renderer>()->getDeviceDisplayModes();
    LOG << "Available video modes:";
    for (auto i = displayModes.begin(); i != displayModes.end(); i++) {
        LOG << "\t" << (*i).w << "x" << (*i).h << "@" << (*i).refresh_rate << "Hz"
            << " - Pixel Format: " << SDL_GetPixelFormatName((*i).format);
    }

    // TODO: move this to UI system
    SDL_StartTextInput();
     */

    // Display startup info
    log().info("Current Working Directory: %s", subsystem<FileSystem>()->workingDir());

    // The engine is now initialised
    initialised_ = true;
    log().info("Engine initialised. Starting %s %s", game_name_, game_version_);

    // Register delegate.
    addEventListener<ExitEvent>(makeEventDelegate(this, &Engine::onExit));
}

void Engine::shutdown() {
    if (!initialised_) {
        return;
    }

    // Save config.
    if (save_config_on_exit_) {
        context_->saveConfig(config_file_);
    }

    // Remove subsystems.
    context_->removeSubsystem<NetSystem>();
    context_->removeSubsystem<GameFramework>();
    context_->removeSubsystem<UserInterface>();
    context_->removeSubsystem<ResourceCache>();
    context_->removeSubsystem<SystemManager>();
    context_->removeSubsystem<Universe>();
    context_->clearSubsystems();

    // The engine is no longer initialised.
    initialised_ = false;
}

void Engine::run(EngineTickCallback tick_callback, EngineRenderCallback render_callback) {
    // Initialise the ECS dependency graph.
    context_->subsystem<SystemManager>()->beginMainLoop();

    // Start the main loop.
	float time_per_update = 1.0f / 60.0f;
    time::TimePoint previous_time = time::beginTiming();
    double accumulated_time = 0.0;
	while (running_)
	{
		time::TimePoint current_time = time::beginTiming();
		frame_time_ = time::elapsed(previous_time, current_time);
		previous_time = current_time;
		accumulated_time += frame_time_;

        // Update game logic.
		while (accumulated_time >= time_per_update)
		{
			update(time_per_update);
			tick_callback(time_per_update);
			accumulated_time -= time_per_update;
		}

        // Render a frame.
		float interpolation = accumulated_time / time_per_update;
        preRender(nullptr);
		context_->subsystem<SystemManager>()->getSystem<EntityRenderer>()->render(interpolation);
        render_callback(interpolation);
        postRender();
        context_->subsystem<Renderer>()->frame();
    }

    context_->subsystem<GameFramework>()->setGameMode(nullptr);
}

double Engine::frameTime() const {
    return frame_time_;
}

void Engine::printSystemInfo() {
#if DW_PLATFORM == DW_WIN32
    String platform = "Windows";
#elif DW_PLATFORM == DW_MACOS
    String platform = "macOS";
#elif DW_PLATFORM == DW_LINUX
    String platform = "Linux";
#endif
    log().info("Platform: %s", platform);
    log().info("Base Path: %s", context()->basePath());
    log().info("Pref Path: %s", context()->prefPath());
    // TODO: more system info
}

#if DW_PLATFORM == DW_LINUX
static String readSymLink(const String& path) {
    char* retval = nullptr;
    size_t len = 64;
    ssize_t rc;

    while (true) {
        if (retval) {
            delete[] retval;
        }
        retval = new char[len + 1];
        rc = readlink(path.c_str(), retval, len);
        if (rc == -1) {
            break; /* not a symlink, i/o error, etc. */
        } else if (rc < static_cast<ssize_t>(len)) {
            retval[rc] = '\0'; /* readlink doesn't null-terminate. */
            String result{retval};
            delete[] retval;
            return result; /* we're good to go. */
        }

        len *= 2; /* grow buffer, try again. */
    }

    delete[] retval;
    return {};
}
#endif

String Engine::basePath() const {
#if DW_PLATFORM == DW_WIN32
    u32 buffer_length = 128;
    char* path_array = NULL;
    u32 length = 0;

    // Get module filename.
    while (true) {
        path_array = new char[buffer_length];
        if (!path_array) {
            delete[] path_array;
            // TODO: Out of memory
            return "";
        }

        length = GetModuleFileNameEx(GetCurrentProcess(), NULL, path_array, buffer_length);
        if (length != buffer_length) {
            break;
        }

        // Buffer too small? Try again.
        buffer_length *= 2;
    }

    // If we failed to locate the exe, bail.
    if (length == 0) {
        delete[] path_array;
        // WIN_SetError("Couldn't locate our .exe");
        return "";
    }

    // Trim off the filename.
    String path{path_array};
    auto last_backslash = path.find_last_of('\\');
    assert(last_backslash != path.npos);
    return path.substr(0, last_backslash + 1);
#elif DW_PLATFORM == DW_MACOS
    CFBundleRef main_bundle = CFBundleGetMainBundle();
    CFURLRef resources_url = CFBundleCopyResourcesDirectoryURL(main_bundle);
    char path[MAX_PATH];
    if (!CFURLGetFileSystemRepresentation(resources_url, TRUE, (UInt8*)path, MAX_PATH)) {
        // TODO: error
    }
    CFRelease(resources_url);
    String str_path{path};
    str_path += "/";

    // For debugging, move from Resources folder to bin.
    str_path += "../../../";
    return str_path;
#elif DW_PLATFORM == DW_LINUX
    String executable_path;

    // Is a Linux-style /proc filesystem available?
    if (access("/proc", F_OK) != 0) {
        // error.
        return "";
    }
    executable_path = readSymLink("/proc/self/exe");
    if (executable_path == "") {
        // Older kernels don't have /proc/self, try PID version.
        executable_path =
            readSymLink(tinyformat::format("/proc/%llu/exe", (unsigned long long)getpid()));
    }

    // Chop off filename.
    auto len = executable_path.find_last_of('/');
    executable_path = executable_path.substr(0, len);
    return executable_path;
#endif
}

void Engine::update(float dt) {
    // log().debug("Frame Time: %f", dt);

	// Receive network packets.
    context_->subsystem<NetSystem>()->update(dt);

	// Trigger events.
    context_->subsystem<EventSystem>()->update(dt);

	// Tick the current universe. (currently no-op).
    context_->subsystem<Universe>()->update(dt);

	// Tick the physics simulation.
	context_->subsystem<PhysicsSystem>()->update(dt, nullptr);

	// Tick the current game mode.
	context_->subsystem<GameFramework>()->update(dt);

	// Update gameplay systems.
    context_->subsystem<SystemManager>()->update(dt);

	// Update user interface.
    context_->subsystem<UserInterface>()->update(dt);
}

void Engine::preRender(Camera_OLD*) {
}

void Engine::postRender() {
    context_->subsystem<UserInterface>()->render();
}

void Engine::onExit(const ExitEvent&) {
    running_ = false;
}
}  // namespace dw
