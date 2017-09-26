#pragma once

#include <sim-driver/OpenGLTypes.hpp>
#include <orb/Orb.hpp>
#include <sim-driver/renderers/RendererHelper.hpp>
#include <sim-driver/meshes/MeshHelper.hpp>

namespace vmp
{

class OrbRenderer
{
public:
    explicit OrbRenderer(vmp::Orb orb);

    void update(double world_time, double scale);
    void render(float alpha, const sim::Camera &camera) const;

    void configureGui();

    void resize(int width, int height);
    sim::PosNormTexData create_sphere_mesh_data(int u_divisions, int v_divisions);

    vmp::Orb orb;
private:
    std::shared_ptr<GLuint> texture_{nullptr};
    sim::PosNormTexRenderer renderer_;
    sim::PosNormTexMesh mesh_;
};

} // namespace vmp
