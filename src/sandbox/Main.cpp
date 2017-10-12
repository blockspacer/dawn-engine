/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2017 (git@dga.me.uk)
 */
#include "DawnEngine.h"
#include "ecs/EntityManager.h"
#include "ecs/Component.h"
#include "ecs/System.h"
#include "renderer/Program.h"
#include "renderer/MeshBuilder.h"
#include "resource/ResourceCache.h"
#include "scene/CameraController.h"
#include "scene/Transform.h"
#include "scene/Universe.h"
#include "renderer/BillboardSet.h"
#include "renderer/Mesh.h"
#include "ui/Imgui.h"

using namespace dw;

class Planet;

/*
 * Terrain patch:
 *
 *       0
 *   0-------1
 *   |       |
 * 3 |       | 1
 *   |       |
 *   3-------2
 *       2
 */
class PlanetTerrainPatch {
public:
    struct Vertex {
        Vec3 p;
        Vec3 n;
        Vec2 tc;
        static VertexDecl createDecl() {
            return VertexDecl{}
                .begin()
                .add(VertexDecl::Attribute::Position, 3, VertexDecl::AttributeType::Float)
                .add(VertexDecl::Attribute::Normal, 3, VertexDecl::AttributeType::Float)
                .add(VertexDecl::Attribute::TexCoord0, 2, VertexDecl::AttributeType::Float)
                .end();
        }
    };

    PlanetTerrainPatch(Planet* planet, PlanetTerrainPatch* parent, const Array<Vec3, 4>& corners,
                       int level);

    void setupAdjacentPatches(const Array<PlanetTerrainPatch*, 4>& adjacent);

    bool hasChildren() const;

    void updatePatch(const Vec3& offset);
    void generateGeometry(Vector<Vertex>& vertex_data, Vector<u32>& index_data);

private:
    Planet* planet_;
    PlanetTerrainPatch* parent_;
    Array<PlanetTerrainPatch*, 4> children_;
    Array<PlanetTerrainPatch*, 4> adjacent_;
    Array<Vec3, 4> corners_;
    Vec3 centre_;

    int level_;

    void split();
    void combine();
};

class Planet : public Object {
public:
    DW_OBJECT(Planet);

    Planet(Context* ctx, float radius, Entity* camera)
        : Object{ctx},
          camera_{camera},
          planet_{nullptr},
          radius_{radius},
          patch_split_distance_{radius * 2.0f},
          terrain_dirty_{false},
          terrain_patches_{} {
        auto em = subsystem<EntityManager>();
        auto rc = subsystem<ResourceCache>();

        // Set up material.
        auto material = makeShared<Material>(
            context(), makeShared<Program>(context(), rc->get<VertexShader>("space/planet.vs"),
                                           rc->get<FragmentShader>("space/planet.fs")));
        material->setTextureUnit(rc->get<Texture>("space/planet.jpg"));
        material->setUniform("light_direction", Vec3{1.0f, 0.0f, 0.0f});
        material->setUniform("surface_sampler", 0);

        // Set up renderable.
        setupTerrainRenderable();
        custom_mesh_renderable_->setMaterial(material);

        planet_ = &em->createEntity(Position::origin, Quat::identity)
                       .addComponent<RenderableComponent>(custom_mesh_renderable_);
    }

    Position& position() const {
        return planet_->transform()->position();
    }

    float radius() const {
        return radius_;
    }

    void update(float dt) {
        const Position& camera_position = camera_->transform()->position();
        Vec3 offset = camera_position.getRelativeTo(planet_->transform()->position());
        for (auto& patch : terrain_patches_) {
            if (patch) {  // TODO: remove once patches are initialised properly.
                patch->updatePatch(offset);
            }
        }

        // If we detected a change in geometry, regenerate.
        if (terrain_dirty_) {
            terrain_dirty_ = false;
            regenerateTerrain();
        }
    }

private:
    Entity* camera_;

    Entity* planet_;
    float radius_;

    // TERRAIN.
    SharedPtr<CustomMeshRenderable> custom_mesh_renderable_;

    float patch_split_distance_;
    bool terrain_dirty_;

    PlanetTerrainPatch* allocatePatch(PlanetTerrainPatch* parent, const Array<Vec3, 4>& corners,
                                      int level) {
        return new PlanetTerrainPatch(this, parent, corners, level);
    }

    void freePatch(PlanetTerrainPatch* patch) {
        delete patch;
    }

    void setupTerrainRenderable() {
        // Setup renderable.
        auto vertex_decl = PlanetTerrainPatch::Vertex::createDecl();
        int default_vertex_count = 36;
        int default_index_count = 20;
        custom_mesh_renderable_ = makeShared<CustomMeshRenderable>(
            context(),
            makeShared<VertexBuffer>(context(), nullptr,
                                     default_vertex_count * vertex_decl.stride(),
                                     default_vertex_count, vertex_decl, BufferUsage::Dynamic),
            makeShared<IndexBuffer>(context(), nullptr, default_index_count * sizeof(u32),
                                    IndexBufferType::U32, BufferUsage::Dynamic));

        // Setup patches.
        float offset = math::Sqrt((radius_ * radius_) / 3.0f);
        Array<Vec3, 8> corners = {
            Vec3{-offset, offset, offset}, Vec3{offset, offset, offset},
            Vec3{offset, -offset, offset}, Vec3{-offset, -offset, offset},
        };
        terrain_patches_ = {
            allocatePatch(nullptr, {corners[0], corners[1], corners[2], corners[3]}, 0),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr};
        regenerateTerrain();
    }

    void regenerateTerrain() {
        Vector<PlanetTerrainPatch::Vertex> vertices;
        Vector<u32> indices;
        for (auto patch : terrain_patches_) {
            if (patch) {  // TODO: remove once patches are initialised properly.
                patch->generateGeometry(vertices, indices);
            }
        }

        // Upload to GPU.
        custom_mesh_renderable_->vertexBuffer()->update(
            vertices.data(), sizeof(PlanetTerrainPatch::Vertex) * vertices.size(), vertices.size(),
            0);
        custom_mesh_renderable_->indexBuffer()->update(indices.data(), sizeof(u32) * indices.size(),
                                                       0);
    }

    // Patches: +z, +x, -z, -x, +y, -y
    Array<PlanetTerrainPatch*, 6> terrain_patches_;

    friend class PlanetTerrainPatch;
};

// Careful to value-initialize children_ and adjacent_ in the initialiser list by giving them an
// empty initializer ({}).
PlanetTerrainPatch::PlanetTerrainPatch(Planet* planet, PlanetTerrainPatch* parent,
                                       const Array<Vec3, 4>& corners, int level)
    : planet_{planet}, parent_{parent}, children_{}, adjacent_{}, corners_(corners), level_{level} {
    // Compute centre position.
    centre_ = Vec3::zero;
    for (auto& c : corners_) {
        centre_ += c;
    }
    centre_ *= 0.25f;
}

void PlanetTerrainPatch::setupAdjacentPatches(const Array<PlanetTerrainPatch*, 4>& adjacent) {
    adjacent_ = adjacent;
}

bool PlanetTerrainPatch::hasChildren() const {
    return children_[0] != nullptr;
}

void PlanetTerrainPatch::updatePatch(const Vec3& offset) {
    if (hasChildren()) {
        // Try combine.
        float combine_threshold = planet_->patch_split_distance_ / level_;
        combine_threshold *= combine_threshold;
        if (offset.DistanceSq(centre_) >= combine_threshold) {
            combine();
        } else {
            for (auto& child : children_) {
                child->updatePatch(offset);
            }
        }
    } else {
        float split_threshold = planet_->patch_split_distance_ / (level_ + 1);
        split_threshold *= split_threshold;
        if (offset.DistanceSq(centre_) <= split_threshold) {
            split();
            for (auto& child : children_) {
                child->updatePatch(offset);
            }
            return;
        }
    }
}

void PlanetTerrainPatch::generateGeometry(Vector<PlanetTerrainPatch::Vertex>& vertex_data,
                                          Vector<u32>& index_data) {
    if (hasChildren()) {
        for (auto& child : children_) {
            child->generateGeometry(vertex_data, index_data);
        }
        return;
    }

    /*
     * Terrain patch geometry:
     *
     *       0
     *   0-------1
     *   | \     |
     * 3 |   \   | 1
     *   |     \ |
     *   3-------2
     *       2
     */
    size_t vertex_start = vertex_data.size();
    for (auto& corner : corners_) {
        Vertex v;
        v.p = corner;
        v.n = corner.Normalized();
        v.tc = {0.0f, 0.0f};
        vertex_data.emplace_back(v);
    }

    // Generate indices.
    index_data.emplace_back(vertex_start);
    index_data.emplace_back(vertex_start + 3);
    index_data.emplace_back(vertex_start + 1);
    index_data.emplace_back(vertex_start + 1);
    index_data.emplace_back(vertex_start + 3);
    index_data.emplace_back(vertex_start + 2);
}

void PlanetTerrainPatch::split() {
    planet_->terrain_dirty_ = true;

    // Allocate child patches.
    Vec3 mid01 = (corners_[0] + corners_[1]) * 0.5f;
    Vec3 mid12 = (corners_[1] + corners_[2]) * 0.5f;
    Vec3 mid23 = (corners_[2] + corners_[3]) * 0.5f;
    Vec3 mid30 = (corners_[3] + corners_[0]) * 0.5f;
    int child_level = level_ + 1;
    children_ = {planet_->allocatePatch(this, {corners_[0], mid01, centre_, mid30}, child_level),
                 planet_->allocatePatch(this, {mid01, corners_[1], mid12, centre_}, child_level),
                 planet_->allocatePatch(this, {centre_, mid12, corners_[2], mid23}, child_level),
                 planet_->allocatePatch(this, {mid30, centre_, mid23, corners_[3]}, child_level)};

    // Setup child adjacent patches.
    for (int i = 0; i < 4; ++i) {
        auto child = children_[i];
        // Internal.
        child->adjacent_[(i + 1) % 4] = children_[(i + 1) % 4];
        child->adjacent_[(i + 2) % 4] = children_[(i + 3) % 4];
        // External.
        if (adjacent_[i]) {
            child->adjacent_[i] = adjacent_[i]->children_[(i + 2) % 4];
        }
        if (adjacent_[(i + 3) % 4]) {
            child->adjacent_[(i + 3) % 4] = adjacent_[(i + 3) % 4]->children_[(i + 1) % 4];
        }
    }

    // Update adjacent child adjacent patches.
    // TODO.
}

void PlanetTerrainPatch::combine() {
}

class Sandbox : public App {
public:
    DW_OBJECT(Sandbox);

    SharedPtr<CameraController> camera_controller;
    SharedPtr<Planet> planet_;

    void init(int argc, char** argv) override {
        auto rc = subsystem<ResourceCache>();
        assert(rc);
        rc->addResourceLocation("../media/base");
        rc->addResourceLocation("../media/sandbox");

        const float radius = 1000.0f;

        // Create a camera.
        auto& camera = subsystem<EntityManager>()
                           ->createEntity(Position{0.0f, 0.0f, radius * 2}, Quat::identity)
                           .addComponent<Camera>(0.1f, 10000.0f, 60.0f, 1280.0f / 800.0f);
        camera_controller = makeShared<CameraController>(context(), 300.0f);
        camera_controller->possess(&camera);

        // Create a planet.
        planet_ = makeShared<Planet>(context(), radius, &camera);
    }

    void update(float dt) override {
        // Calculate distance to planet and adjust acceleration accordingly.
        auto& a = camera_controller->possessed()->transform()->position();
        auto& b = planet_->position();
        float altitude = a.getRelativeTo(b).Length() - planet_->radius();
        camera_controller->setAcceleration(altitude);

        camera_controller->update(dt);
        planet_->update(dt);
    }

    void render() override {
        subsystem<Renderer>()->setViewClear(0, {0.0f, 0.0f, 0.2f, 1.0f});

        // Calculate average FPS.
        float current_fps = 1.0 / engine_->frameTime();
        static const int FPS_HISTORY_COUNT = 100;
        static float fps_history[FPS_HISTORY_COUNT];
        for (int i = 1; i < FPS_HISTORY_COUNT; ++i) {
            fps_history[i - 1] = fps_history[i];
        }
        fps_history[FPS_HISTORY_COUNT - 1] = current_fps;
        float average_fps = 0.0f;
        for (int i = 0; i < FPS_HISTORY_COUNT; ++i) {
            average_fps += fps_history[i] / FPS_HISTORY_COUNT;
        }

        // Update displayed FPS information every 100ms.
        static double accumulated_time = 0.0;
        static float displayed_fps = 60.0f;
        accumulated_time += engine_->frameTime();
        if (accumulated_time > 1.0f / 30.0f) {
            accumulated_time = 0;
            displayed_fps = average_fps;
        }

        // Display FPS information.
        ImGui::SetNextWindowPos({10, 10});
        ImGui::SetNextWindowSize({140, 40});
        if (!ImGui::Begin("FPS", nullptr, {0, 0}, 0.5f,
                          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::End();
            return;
        }
        ImGui::Text("FPS:   %.1f", displayed_fps);
        ImGui::Text("Frame: %.4f ms", 1000.0f / displayed_fps);
        ImGui::End();
    }

    void shutdown() override {
    }

    String gameName() override {
        return "Sandbox";
    }

    String gameVersion() override {
        return "1.0.0";
    }
};

DW_IMPLEMENT_MAIN(Sandbox);
