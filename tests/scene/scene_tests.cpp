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

    auto entities = scene.get_all_entities();

    REQUIRE(entities.size() == 1);
    REQUIRE(entities[0].uuid() == entity2.uuid());

    scene.destroy_entity(entity2);

    entities = scene.get_all_entities();

    REQUIRE(entities.empty());
}

TEST_CASE("Can get all entities with given component and remove components", "[Scene]") {
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

TEST_CASE("Adding children updates RelationshipComponent accordingly", "[Scene]") {
    auto scene = Phos::Scene("Test scene");

    auto parent = scene.create_entity();
    auto child_1 = scene.create_entity();
    auto child_2 = scene.create_entity();

    parent.add_child(child_1);
    parent.add_child(child_2);

    const auto& parent_component = parent.get_component<Phos::RelationshipComponent>();

    REQUIRE_FALSE(parent_component.parent.has_value());
    REQUIRE(parent_component.children.size() == 2);
    REQUIRE(parent_component.children[0] == child_1.uuid());
    REQUIRE(parent_component.children[1] == child_2.uuid());

    const auto& child_1_component = child_1.get_component<Phos::RelationshipComponent>();
    const auto& child_2_component = child_2.get_component<Phos::RelationshipComponent>();

    REQUIRE(child_1_component.parent.has_value());
    REQUIRE(*child_1_component.parent == parent.uuid());

    REQUIRE(child_2_component.parent.has_value());
    REQUIRE(*child_2_component.parent == parent.uuid());
}

TEST_CASE("Adding parent updates RelationshipComponent accordingly", "[Scene]") {
    auto scene = Phos::Scene("Test scene");

    auto parent = scene.create_entity();
    auto child_1 = scene.create_entity();
    auto child_2 = scene.create_entity();

    child_1.set_parent(parent);
    child_2.set_parent(parent);

    const auto& parent_component = parent.get_component<Phos::RelationshipComponent>();

    REQUIRE_FALSE(parent_component.parent.has_value());
    REQUIRE(parent_component.children.size() == 2);
    REQUIRE(parent_component.children[0] == child_1.uuid());
    REQUIRE(parent_component.children[1] == child_2.uuid());

    const auto& child_1_component = child_1.get_component<Phos::RelationshipComponent>();
    const auto& child_2_component = child_2.get_component<Phos::RelationshipComponent>();

    REQUIRE(child_1_component.parent.has_value());
    REQUIRE(*child_1_component.parent == parent.uuid());

    REQUIRE(child_2_component.parent.has_value());
    REQUIRE(*child_2_component.parent == parent.uuid());
}

TEST_CASE("Adding multi level children updates RelationshipComponent accordingly", "[Scene]") {
    auto scene = Phos::Scene("Test scene");

    auto parent = scene.create_entity();
    auto child_1 = scene.create_entity();
    auto child_2 = scene.create_entity();
    auto child_3 = scene.create_entity();

    parent.add_child(child_1);
    parent.add_child(child_3);
    child_1.add_child(child_2);

    const auto& parent_component = parent.get_component<Phos::RelationshipComponent>();
    const auto& child_1_component = child_1.get_component<Phos::RelationshipComponent>();
    const auto& child_2_component = child_2.get_component<Phos::RelationshipComponent>();
    const auto& child_3_component = child_3.get_component<Phos::RelationshipComponent>();

    REQUIRE(parent_component.children.size() == 2);
    REQUIRE(parent_component.children[0] == child_1.uuid());
    REQUIRE(parent_component.children[1] == child_3.uuid());

    REQUIRE((child_1_component.parent.has_value() && *child_1_component.parent == parent.uuid()));
    REQUIRE(child_1_component.children.size() == 1);
    REQUIRE(child_1_component.children[0] == child_2.uuid());

    REQUIRE((child_2_component.parent.has_value() && *child_2_component.parent == child_1.uuid()));
    REQUIRE(child_2_component.children.empty());

    REQUIRE((child_3_component.parent.has_value() && *child_3_component.parent == parent.uuid()));
    REQUIRE(child_3_component.children.empty());
}

TEST_CASE("Removing entity also removes children", "[Scene]") {
    auto scene = Phos::Scene("Test scene");

    auto parent_1 = scene.create_entity();
    auto parent_2 = scene.create_entity();

    auto child_1 = scene.create_entity();
    auto child_2 = scene.create_entity();
    auto child_3 = scene.create_entity();

    /*
     * parent_1 -> child_1 -> child_2
     *          -> child_3
     * parent_2
     */
    parent_1.add_child(child_1);
    parent_1.add_child(child_3);
    child_1.add_child(child_2);

    scene.destroy_entity(parent_1);

    const auto& entities = scene.get_all_entities();
    REQUIRE(entities.size() == 1);
    REQUIRE(entities[0].uuid() == parent_2.uuid());
}

TEST_CASE("Removing sub-entity removes it's children and updates RelationshipComponent", "[Scene]") {
    auto scene = Phos::Scene("Test scene");

    auto parent = scene.create_entity();

    auto child_1 = scene.create_entity();
    auto child_2 = scene.create_entity();
    auto child_3 = scene.create_entity();

    /*
     * parent_1 -> child_1 -> child_2
     *          -> child_3
     */
    parent.add_child(child_1);
    parent.add_child(child_3);
    child_1.add_child(child_2);

    scene.destroy_entity(child_1);

    const auto& entities = scene.get_all_entities();
    REQUIRE(entities.size() == 2);
    REQUIRE((std::ranges::find(entities, parent) != entities.end() &&
             std::ranges::find(entities, child_3) != entities.end()));

    const auto& parent_component = parent.get_component<Phos::RelationshipComponent>();
    REQUIRE(parent_component.children.size() == 1);
    REQUIRE(parent_component.children[0] == child_3.uuid());
}
