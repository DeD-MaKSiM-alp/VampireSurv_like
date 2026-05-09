#include "test_runner.h"

#include "ecs/World.h"

namespace
{
struct Position
{
    float x{0.0f};
    float y{0.0f};
};

struct Tag
{
    int value{0};
};

struct Health
{
    float hp{100.0f};
};
}

TEST_CASE(world_create_and_alive)
{
    World w;
    const EntityId a = w.createEntity();
    const EntityId b = w.createEntity();

    EXPECT_TRUE(w.isAlive(a));
    EXPECT_TRUE(w.isAlive(b));
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(w.isAlive(InvalidEntity));
    EXPECT_FALSE(w.isAlive(9999u));
    EXPECT_EQ(w.aliveCount(), static_cast<std::size_t>(2));
}

TEST_CASE(world_destroy_is_deferred)
{
    World w;
    const EntityId e = w.createEntity();
    w.destroyEntityDeferred(e);

    // Still alive until flushDestroyed.
    EXPECT_TRUE(w.isAlive(e));
    w.flushDestroyed();
    EXPECT_FALSE(w.isAlive(e));
    EXPECT_EQ(w.aliveCount(), static_cast<std::size_t>(0));
}

TEST_CASE(world_destroy_invalid_is_noop)
{
    World w;
    w.destroyEntityDeferred(InvalidEntity);
    w.destroyEntityDeferred(42u);
    w.flushDestroyed();
    EXPECT_EQ(w.aliveCount(), static_cast<std::size_t>(0));
}

TEST_CASE(world_add_and_get_component)
{
    World w;
    const EntityId e = w.createEntity();

    w.addComponent<Position>(e, Position{10.0f, 20.0f});
    EXPECT_TRUE(w.hasComponent<Position>(e));
    auto* p = w.getComponent<Position>(e);
    EXPECT_TRUE(p != nullptr);
    EXPECT_NEAR(p->x, 10.0f, 1e-6);
    EXPECT_NEAR(p->y, 20.0f, 1e-6);

    EXPECT_FALSE(w.hasComponent<Tag>(e));
    EXPECT_TRUE(w.getComponent<Tag>(e) == nullptr);
}

TEST_CASE(world_add_component_overwrites)
{
    World w;
    const EntityId e = w.createEntity();
    w.addComponent<Tag>(e, Tag{1});
    w.addComponent<Tag>(e, Tag{2});

    auto* t = w.getComponent<Tag>(e);
    EXPECT_TRUE(t != nullptr);
    EXPECT_EQ(t->value, 2);
}

TEST_CASE(world_remove_component)
{
    World w;
    const EntityId e = w.createEntity();
    w.addComponent<Tag>(e, Tag{7});
    EXPECT_TRUE(w.hasComponent<Tag>(e));

    w.removeComponent<Tag>(e);
    EXPECT_FALSE(w.hasComponent<Tag>(e));
    EXPECT_TRUE(w.getComponent<Tag>(e) == nullptr);
}

TEST_CASE(world_destroy_removes_components)
{
    World w;
    const EntityId e = w.createEntity();
    w.addComponent<Position>(e, Position{1.0f, 2.0f});
    w.addComponent<Tag>(e, Tag{5});

    w.destroyEntityDeferred(e);
    w.flushDestroyed();

    EXPECT_FALSE(w.hasComponent<Position>(e));
    EXPECT_FALSE(w.hasComponent<Tag>(e));
}

TEST_CASE(world_foreach_filters_by_components)
{
    World w;
    const EntityId a = w.createEntity();
    const EntityId b = w.createEntity();
    const EntityId c = w.createEntity();

    w.addComponent<Position>(a, Position{1.0f, 0.0f});
    w.addComponent<Tag>(a, Tag{10});

    w.addComponent<Position>(b, Position{2.0f, 0.0f});

    w.addComponent<Tag>(c, Tag{30});

    int singleCount = 0;
    w.forEach<Position>([&](EntityId, Position&) { ++singleCount; });
    EXPECT_EQ(singleCount, 2);

    int pairCount = 0;
    int pairTagSum = 0;
    w.forEach<Position, Tag>([&](EntityId, Position&, Tag& t) {
        ++pairCount;
        pairTagSum += t.value;
    });
    EXPECT_EQ(pairCount, 1);
    EXPECT_EQ(pairTagSum, 10);
}

TEST_CASE(world_foreach_safe_destroy_during_iteration)
{
    World w;
    for (int i = 0; i < 10; ++i)
    {
        const EntityId e = w.createEntity();
        w.addComponent<Tag>(e, Tag{i});
    }

    int seen = 0;
    w.forEach<Tag>([&](EntityId entity, Tag&) {
        ++seen;
        w.destroyEntityDeferred(entity);
    });

    // Snapshot taken at start; all 10 should be visited even though we
    // queued destruction along the way.
    EXPECT_EQ(seen, 10);
    EXPECT_EQ(w.aliveCount(), static_cast<std::size_t>(10));
    w.flushDestroyed();
    EXPECT_EQ(w.aliveCount(), static_cast<std::size_t>(0));
}

TEST_CASE(world_foreach_safe_with_added_during_iteration)
{
    World w;
    for (int i = 0; i < 3; ++i)
    {
        const EntityId e = w.createEntity();
        w.addComponent<Tag>(e, Tag{i});
    }

    int seen = 0;
    w.forEach<Tag>([&](EntityId, Tag&) {
        ++seen;
        // Spawn new tagged entity inside the loop. Must NOT be visited in the
        // current iteration (snapshot semantics).
        const EntityId fresh = w.createEntity();
        w.addComponent<Tag>(fresh, Tag{99});
    });
    EXPECT_EQ(seen, 3);
}

TEST_CASE(world_isalive_after_destroy_is_false_immediately_after_flush)
{
    World w;
    const EntityId e = w.createEntity();
    w.destroyEntityDeferred(e);
    w.destroyEntityDeferred(e);  // double-queue, should not crash
    w.flushDestroyed();
    EXPECT_FALSE(w.isAlive(e));
}
