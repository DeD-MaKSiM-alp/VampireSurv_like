#include "ecs/World.h"

EntityId World::createEntity()
{
    const EntityId id = m_nextEntityId++;
    m_aliveEntities.insert(id);
    return id;
}

bool World::isAlive(EntityId entity) const
{
    if (entity == InvalidEntity)
    {
        return false;
    }

    return m_aliveEntities.find(entity) != m_aliveEntities.end();
}

void World::destroyEntityDeferred(EntityId entity)
{
    if (!isAlive(entity))
    {
        return;
    }

    m_pendingDestroy.push_back(entity);
}

void World::flushDestroyed()
{
    for (const EntityId entity : m_pendingDestroy)
    {
        m_aliveEntities.erase(entity);

        for (auto& [_, storage] : m_componentStores)
        {
            storage->removeEntity(entity);
        }
    }

    m_pendingDestroy.clear();
}
