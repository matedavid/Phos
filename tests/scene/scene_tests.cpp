#include "scene/scene.h"
#include "scene/entity.h"

#include <catch2/catch_all.hpp>

struct ComponentC {
    float x = 10.0f;
};

struct ComponentD {
    float y;
};

TEST_CASE("Can create and destroy entities", "[Scene]") {
    auto scene = Phos::Scene("Test scene");

    auto entity1 = scene.create_entity();
    auto entity2 = scene.create_entity();

    REQUIRE(entity1 != entity2);

    scene.destroy_entity(entity1);
    scene.destroy_entity(entity2);
}

TEST_CASE("Can add and modify components", "[Scene]") {
    auto scene = Phos::Scene("Test scene");

    auto entity1 = scene.create_entity();
    entity1.add_component<ComponentC>();
    entity1.add_component<ComponentD>({.y = -5.0f});

    auto& component_c_1 = entity1.get_component<ComponentC>();
    auto& component_d_1 = entity1.get_component<ComponentD>();

    REQUIRE(component_c_1.x == 10.0f);
    REQUIRE(component_d_1.y == -5.0f);

    component_c_1.x = 4.0f;
    REQUIRE(component_c_1.x == 4.0f);
}

TEST_CASE("Can destroy entities", "[Scene]") {
    auto scene = Phos::Scene("Test scene");

    auto entity1 = scene.create_entity();
    auto entity2 = scene.create_entity();

    scene.destroy_entity(entity1);
    scene.destroy_entity(entity2);
}

TEST_CASE("Can get all entities with given component and remove components ", "[Scene]") {
    auto scene = Phos::Scene("Test scene");

    auto entity1 = scene.create_entity();
    auto entity2 = scene.create_entity();
    auto entity3 = scene.create_entity();

    entity1.add_component<ComponentC>();
    entity2.add_component<ComponentC>();
    entity3.add_component<ComponentC>();

    entity3.add_component<ComponentD>({.y = -3.0f});

    auto entities_c = scene.get_entities_with<ComponentC>();
    REQUIRE(std::ranges::find(entities_c, entity1) != entities_c.end());
    REQUIRE(std::ranges::find(entities_c, entity2) != entities_c.end());
    REQUIRE(std::ranges::find(entities_c, entity3) != entities_c.end());

    auto entities_d = scene.get_entities_with<ComponentD>();
    REQUIRE(std::ranges::find(entities_d, entity1) == entities_d.end());
    REQUIRE(std::ranges::find(entities_d, entity3) != entities_d.end());

    entity1.remove_component<ComponentC>();
    entities_c = scene.get_entities_with<ComponentC>();
    REQUIRE(std::ranges::find(entities_c, entity1) == entities_c.end());
    REQUIRE(std::ranges::find(entities_c, entity2) != entities_c.end());
    REQUIRE(std::ranges::find(entities_c, entity3) != entities_c.end());

    entity3.remove_component<ComponentD>();
    entities_d = scene.get_entities_with<ComponentD>();
    REQUIRE(entities_d.empty());
}