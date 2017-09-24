#pragma once

#include <sim-driver/OpenGLTypes.hpp>
#include <orb/Orb.hpp>
#include <sim-driver/renderers/RendererHelper.hpp>
#include <sim-driver/meshes/MeshHelper.hpp>

namespace sim
{

class OrbRenderer
{
public:
    explicit OrbRenderer(sim::Orb heightMap);

    void render(float alpha, const Camera &camera) const;

    void configureGui();

    void resize(int width, int height);

    sim::Orb orb;

private:
//    std::shared_ptr<GLuint> program_;
    sim::PosNormTexRenderer renderer_;
    sim::PosNormTexMesh mesh_;
};

} // namespace sim
