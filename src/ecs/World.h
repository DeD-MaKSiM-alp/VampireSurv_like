#pragma once

#include "ecs/Entity.h"

#include <type_traits>
#include <typeindex>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <memory>

class World
{
public:
    World() = default;
    ~World() = default;

    EntityId createEntity();
    bool isAlive(EntityId entity) const;
    void destroyEntityDeferred(EntityId entity);
    void flushDestroyed();
    std::size_t aliveCount() const { return m_aliveEntities.size(); }

    template <typename Component, typename... Args>
    Component& addComponent(EntityId entity, Args&&... args)
    {
        static_assert(std::is_constructible_v<Component, Args...>,
            "Component must be constructible from provided arguments.");

        auto& storage = storageFor<Component>();
        auto [it, inserted] =
            storage.items.emplace(entity, Component{std::forward<Args>(args)...});

        if (!inserted)
        {
            it->second = Component{std::forward<Args>(args)...};
        }

        return it->second;
    }

    template <typename Component>
    bool hasComponent(EntityId entity) const
    {
        const auto* storage = storageForIfExists<Component>();
        if (!storage)
        {
            return false;
        }

        return storage->items.find(entity) != storage->items.end();
    }

    template <typename Component>
    Component* getComponent(EntityId entity)
    {
        auto* storage = storageForIfExists<Component>();
        if (!storage)
        {
            return nullptr;
        }

        auto it = storage->items.find(entity);
        if (it == storage->items.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    template <typename Component>
    const Component* getComponent(EntityId entity) const
    {
        const auto* storage = storageForIfExists<Component>();
        if (!storage)
        {
            return nullptr;
        }

        auto it = storage->items.find(entity);
        if (it == storage->items.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    template <typename Component>
    void removeComponent(EntityId entity)
    {
        auto* storage = storageForIfExists<Component>();
        if (!storage)
        {
            return;
        }

        storage->items.erase(entity);
    }

    template <typename... Components, typename Callback>
    void forEach(Callback&& callback)
    {
        static_assert(sizeof...(Components) > 0, "forEach requires at least one component type.");

        using FirstComponent = std::tuple_element_t<0, std::tuple<Components...>>;
        auto* first = storageForIfExists<FirstComponent>();

        if (!first)
        {
            return;
        }

        std::vector<EntityId> snapshot;
        snapshot.reserve(first->items.size());
        for (const auto& [entity, _] : first->items)
        {
            snapshot.push_back(entity);
        }

        for (const EntityId entity : snapshot)
        {
            if (!isAlive(entity))
            {
                continue;
            }

            if (!(hasComponent<Components>(entity) && ...))
            {
                continue;
            }

            callback(entity, *getComponent<Components>(entity)...);
        }
    }

private:
    struct IStorage
    {
        virtual ~IStorage() = default;
        virtual void removeEntity(EntityId entity) = 0;
    };

    template <typename Component>
    struct Storage final : IStorage
    {
        std::unordered_map<EntityId, Component> items;

        void removeEntity(EntityId entity) override
        {
            items.erase(entity);
        }
    };

    template <typename Component>
    Storage<Component>& storageFor()
    {
        const std::type_index type = std::type_index(typeid(Component));
        auto it = m_componentStores.find(type);

        if (it == m_componentStores.end())
        {
            auto storage = std::make_unique<Storage<Component>>();
            auto* ptr = storage.get();
            m_componentStores.emplace(type, std::move(storage));
            return *ptr;
        }

        return *static_cast<Storage<Component>*>(it->second.get());
    }

    template <typename Component>
    Storage<Component>* storageForIfExists()
    {
        const std::type_index type = std::type_index(typeid(Component));
        auto it = m_componentStores.find(type);

        if (it == m_componentStores.end())
        {
            return nullptr;
        }

        return static_cast<Storage<Component>*>(it->second.get());
    }

    template <typename Component>
    const Storage<Component>* storageForIfExists() const
    {
        const std::type_index type = std::type_index(typeid(Component));
        auto it = m_componentStores.find(type);

        if (it == m_componentStores.end())
        {
            return nullptr;
        }

        return static_cast<const Storage<Component>*>(it->second.get());
    }

    EntityId m_nextEntityId{1};
    std::unordered_set<EntityId> m_aliveEntities;
    std::vector<EntityId> m_pendingDestroy;
    std::unordered_map<std::type_index, std::unique_ptr<IStorage>> m_componentStores;
};
