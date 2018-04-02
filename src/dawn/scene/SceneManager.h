/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2017 (git@dga.me.uk)
 */
#pragma once

#include "renderer/Renderable.h"
#include "scene/Entity.h"
#include "scene/Transform.h"

#include <ontology/World.hpp>
#include <ontology/SystemManager.hpp>

#include "scene/System.h"
#include "scene/PhysicsScene.h"

namespace dw {
/// Manages the current game world, including all entities and entity systems.
class DW_API SceneManager : public Module {
public:
    DW_OBJECT(SceneManager);

    SceneManager(Context* context);
    ~SceneManager();

    // TODO: Move this into Universe class.
    /// Set up star system.
    void createStarSystem();

    /// Constructs a new entity system of the specified type.
    /// @tparam T Entity system type.
    /// @tparam Args List of constructor argument types.
    /// @param args Constructor arguments.
    template <typename T, typename... Args> T* addSystem(Args... args);

    /// Looks up an entity system in the context.
    /// @tparam T Entity system type.
    /// @return Instance of the entity system type.
    template <typename T> T* getSystem();

    /// Removes the entity system from the context.
    /// @tparam T Entity system type.
    template <typename T> void removeSystem();

    /// Creates a new empty entity.
    /// @return A newly created entity.
    Entity& createEntity();

    /// Creates a new entity with a transform component.
    /// @param p Initial position.
    /// @param o Initial orientation.
    /// @param parent Parent entity.
    /// @return A newly created entity.
    Entity& createEntity(const Position& p, const Quat& o, Entity* parent);

    /// Creates a new entity with a transform component.
    /// @param p Initial position.
    /// @param o Initial orientation.
    /// @return A newly created entity.
    Entity& createEntity(const Position& p, const Quat& o);

    /// Creates a new empty entity with a previously reserved entity ID.
    /// @return A newly created entity.
    Entity& createEntity(EntityId reserved_entity_id);

    /// Reserve a new entity ID.
    /// @return Unique unused entity ID.
    EntityId reserveEntityId();

    /// Looks up an entity by its ID.
    /// @param id Entity ID.
    /// @return The entity which corresponds to this entity ID.
    Entity* findEntity(EntityId id);

    /// Removes an entity from the engine.
    /// @param entity Entity to remove.
    void removeEntity(Entity* entity);

    /// Begin main loop. Required by Ontology.
    void beginMainLoop();

    /// Updates systems, and calls update on each entity.
    /// @param dt Time elapsed
    void update(float dt);

    /// Returns the root node in the scene graph.
    Transform* rootNode() const;

    /// Returns the physics scene.
    PhysicsScene* physicsScene() const;

    /// Returns the last delta time. Used only by the Ontology wrapper.
    float lastDeltaTime_internal() const;

private:
    float last_dt_;
    Ontology::World ontology_world_;
    HashMap<EntityId, UniquePtr<Entity>> entity_lookup_table_;
    EntityId entity_id_allocator_;

    SharedPtr<Transform> root_node_;
    SharedPtr<RenderableNode> background_renderable_root_;
    Entity* background_entity_;

    UniquePtr<PhysicsScene> physics_scene_;
};

template <typename T, typename... Args> T* SceneManager::addSystem(Args... args) {
    auto system = makeUnique<T>(context(), std::forward(args)...);
    return ontology_world_.getSystemManager()
        .addSystem<OntologySystemAdapter<T>>(std::move(system))
        .system();
}

template <typename T> T* SceneManager::getSystem() {
    return ontology_world_.getSystemManager().getSystem<OntologySystemAdapter<T>>().system();
}

template <typename T> void SceneManager::removeSystem() {
    ontology_world_.getSystemManager().removeSystem<OntologySystemAdapter<T>>();
}
}  // namespace dw
