/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2017 (git@dga.me.uk)
 */
#pragma once

#include "ontology/Entity.hpp"

// ReSharper disable CppUnusedIncludeDirective
#include "ecs/Component.h"
// ReSharper restore CppUnusedIncludeDirective

namespace dw {
/// Entity identifier.
using EntityId = Ontology::Entity::ID;

/// Entity object. Currently implemented as an Ontology::Entity wrapper.
class Entity : public Object {
public:
    DW_OBJECT(Entity);

    explicit Entity(Context* context, Ontology::Entity& entity);
    virtual ~Entity() = default;

    /// Accesses a component contained within this entity.
    /// @tparam T Component type.
    /// @return The data for this component type.
    template <typename T> T* component() const;

    /// Determines whether this entity contains a component type.
    /// @tparam T Component type.
    template <typename T> bool hasComponent() const;

    /// Initialises and adds a new component to the entity.
    /// @tparam T Component type.
    /// @tparam Args Component constructor argument types.
    /// @param args Component constructor arguments.
    template <typename T, typename... Args> Entity& addComponent(Args... args);

    /// Returns the identifier of this entity.
    EntityId id() const;

private:
    Ontology::Entity& internal_entity_;
};

inline Entity::Entity(Context* context, Ontology::Entity& entity)
    : Object{context}, internal_entity_{entity} {
}

inline EntityId Entity::id() const {
    return internal_entity_.getID();
}

template <typename T> T* Entity::component() const {
    return internal_entity_.getComponentPtr<T>();
}

template <typename T> bool Entity::hasComponent() const {
    return component<T>() != nullptr;
}

template <typename T, typename... Args> Entity& Entity::addComponent(Args... args) {
    internal_entity_.addComponent<T, Args...>(std::forward<Args>(args)...);
    return *this;
}
}  // namespace dw