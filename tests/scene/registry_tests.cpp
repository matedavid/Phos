#include "scene/registry.h"

#include <catch2/catch_all.hpp>

struct ComponentA {
    float x;
};

struct ComponentB {
    float y;
};

TEST_CASE("Can create entities", "[Registry]") {
    auto registry = Phos::Registry();

    auto entity1 = registry.create();
    auto entity2 = registry.create();

    REQUIRE(entity1 != entity2);
    REQUIRE(registry.number_entities() == 2);
}

TEST_CASE("Can destroy entities", "[Registry]") {
    auto registry = Phos::Registry();

    auto entity1 = registry.create();
    auto entity2 = registry.create();

    registry.destroy(entity1);
    REQUIRE(registry.number_entities() == 1);

    registry.destroy(entity2);
    REQUIRE(registry.number_entities() == 0);
}

TEST_CASE("Can add component to entity", "[Registry]") {
    auto registry = Phos::Registry();

    auto entity1 = registry.create();
    registry.add_component<ComponentA>(entity1, {.x = 5.0f});
    REQUIRE(registry.get_component<ComponentA>(entity1).x == 5.0f);

    auto entity2 = registry.create();
    registry.add_component<ComponentA>(entity2, {.x = 20.0f});
    registry.add_component<ComponentB>(entity2, {.y = 15.0f});

    REQUIRE(registry.get_component<ComponentA>(entity2).x == 20.0f);
    REQUIRE(registry.get_component<ComponentB>(entity2).y == 15.0f);
}

TEST_CASE("Can remove component from entity", "[Registry]") {
    auto registry = Phos::Registry();

    auto entity1 = registry.create();
    registry.add_component<ComponentA>(entity1, {.x = -4.0f});
    REQUIRE(registry.get_component<ComponentA>(entity1).x == -4.0f);

    registry.remove_component<ComponentA>(entity1);
}

TEST_CASE("Can modify component from entity", "[Registry]") {
    auto registry = Phos::Registry();

    auto entity1 = registry.create();
    registry.add_component<ComponentA>(entity1, {.x = 2.0f});

    auto& component_a = registry.get_component<ComponentA>(entity1);
    REQUIRE(component_a.x == 2.0f);

    component_a.x = 10.0f;
    REQUIRE(registry.get_component<ComponentA>(entity1).x == 10.0f);
}

TEST_CASE("Can get all entities with given component", "[Registry]") {
    auto registry = Phos::Registry();

    auto entity1 = registry.create();
    auto entity2 = registry.create();
    auto entity3 = registry.create();

    registry.add_component<ComponentA>(entity1, {.x = 1.0f});
    registry.add_component<ComponentA>(entity2, {.x = 2.0f});
    registry.add_component<ComponentA>(entity3, {.x = 3.0f});

    registry.add_component<ComponentB>(entity2, {.y = 4.0f});

    const auto& entities_a = registry.view<ComponentA>();
    REQUIRE(std::ranges::find(entities_a, entity1) != entities_a.end());
    REQUIRE(std::ranges::find(entities_a, entity2) != entities_a.end());
    REQUIRE(std::ranges::find(entities_a, entity3) != entities_a.end());

    const auto& entities_b = registry.view<ComponentB>();
    REQUIRE(std::ranges::find(entities_b, entity1) == entities_b.end());
    REQUIRE(std::ranges::find(entities_b, entity2) != entities_b.end());
    REQUIRE(std::ranges::find(entities_b, entity3) == entities_b.end());

    registry.remove_component<ComponentA>(entity3);

    const auto& entities_a_2 = registry.view<ComponentA>();
    REQUIRE(std::ranges::find(entities_a_2, entity3) == entities_a_2.end());
}
