/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2017 (git@dga.me.uk)
 */
#pragma once

#include <json.hpp>
#include "ontology/World.hpp"
#include "core/Collections.h"
#include "math/StringHash.h"

namespace dw {

class Subsystem;
class ConfigNode;
class Entity;

using Json = nlohmann::json;

/// The Context is a class which acts as a container of subsystems, which are low level engine
/// services such as the resource manager, renderer, filesystem, entity manager, etc. The context
/// also contains the engine and game configuration encoded as JSON.
class DW_API Context {
public:
    Context(const String& base_path, const String& pref_path);
    Context(Context& other) = delete;
    ~Context();

    /// Non-copyable.
    Context& operator=(const Context& other) = delete;

    /// Accesses a subsystem by type hash. Requires a downcast.
    /// @param subsystem_type Subsystem type hash.
    /// @return A pointer to the subsystem instance, or nullptr otherwise.
    Subsystem* subsystem(StringHash subsystem_type) const;

    /// Adds a subsystem to this context.
    /// @param subsystem Subsystem instance.
    Subsystem* addSubsystem(UniquePtr<Subsystem> subsystem);

    /// Removes a subsystem contained within the context, calling the subsystems
    /// deconstructor.
    /// @param subsystem_type Subsystem type hash.
    void removeSubsystem(StringHash subsystem_type);

    /// Clears all subsystems from this context. Equivalent to calling removeSubsystem on
    /// every subsystem.
    void clearSubsystems();

    /// Accesses a subsystem by type.
    /// @tparam T Subsystem type.
    /// @return A pointer to the subsystem instance, or nullptr otherwise.
    template <typename T> T* subsystem() const;

    /// Constructs a new subsystem and adds it to the context.
    /// @tparam T Subsystem type.
    /// @tparam Args Argument types.
    /// @param args Arguments to the subsystem constructor.
    template <typename T, typename... Args> T* addSubsystem(Args... args);

    /// Removes a subsystem contained within this context, calling the subsystems
    /// deconstructor.
    /// @tparam T Subsystem type.
    template <typename T> void removeSubsystem();

    /// Access the config root.
    Json& config();

    /// Access the config root.
    const Json& config() const;

    /// Load the configuration.
    void loadConfig(const String& configFile);

    /// Save the configuration.
    void saveConfig(const String& configFile);

    /// Set the default configuration.
    void setDefaultConfig();

    /// Get the base path of the application.
    const String& basePath() const;

    /// Get the preferences path of the application.
    const String& prefPath() const;

private:
    HashMap<StringHash, UniquePtr<Subsystem>> subsystems_;
    Vector<StringHash> subsystem_init_order_;

    // Engine configuration.
    Json config_;

    // File paths.
    String base_path_;
    String pref_path_;

    // Ontology world.
    Ontology::World ontology_world_;
    friend class SystemManager;
    friend class Universe;
};

template <typename T> T* Context::subsystem() const {
    return static_cast<T*>(subsystem(T::typeStatic()));
}

template <typename T, typename... Args> T* Context::addSubsystem(Args... args) {
    return static_cast<T*>(addSubsystem(makeUnique<T>(this, std::forward<Args>(args)...)));
}

template <typename T> void Context::removeSubsystem() {
    removeSubsystem(T::typeStatic());
}
}  // namespace dw
