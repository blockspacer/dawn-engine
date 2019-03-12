/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2019 (git@dga.me.uk)
 */
#pragma once

#include "renderer/Renderable.h"
#include "renderer/VertexBuffer.h"
#include "renderer/IndexBuffer.h"

namespace dw {
enum class BillboardType { Point, Directional };

class BillboardSet : public Renderable, public Object {
public:
    DW_OBJECT(BillboardSet);

    BillboardSet(Context* ctx, u32 particle_count, const Vec2& particle_size);

    void resize(u32 particle_count);

    void setBillboardType(BillboardType type);

    void setParticleVisible(u32 particle_id, bool visible);
    void setParticlePosition(u32 particle_id, const Vec3& position);
    void setParticleSize(u32 particle_id, const Vec2& size);
    void setParticleDirection(u32 particle_id, const Vec3& direction);

    void draw(Renderer* renderer, uint view, detail::Transform& camera, const Mat4&,
              const Mat4& view_projection_matrix) override;

private:
    Vec2 particle_size_;
    BillboardType type_;

    struct ParticleData {
        bool visible;
        Vec3 position;
        Vec2 size;
        Vec3 direction;
    };
    Vector<ParticleData> particles_;
    struct ParticleVertex {
        Vec3 position;
        Vec2 uv;
    };
    Vector<ParticleVertex> vertex_data_;
    Vector<u32> index_data_;

    SharedPtr<VertexBuffer> vb_;
    SharedPtr<IndexBuffer> ib_;
    uint particle_count_;

    void update(detail::Transform& camera_transform);
    void calculateAxes(detail::Transform& camera_transform, const ParticleData& data, Vec3& axis_x,
                       Vec3& axis_y);
};
}  // namespace dw
