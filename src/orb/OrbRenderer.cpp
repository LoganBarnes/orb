#include <orb/OrbRenderer.hpp>
#include <sim-driver/OpenGLHelper.hpp>
#include <sim-driver/Camera.hpp>
#include <orb/ShaderConfig.hpp>
#include <sim-driver/ShaderConfig.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <iostream>

namespace vmp
{

namespace
{

const std::vector<sim::VAOElement> &phiThetaElements()
{
    static std::vector<sim::VAOElement> elements{
        {"phis", 3, GL_FLOAT, reinterpret_cast<void *>(offsetof(sim::PosNormTexVertex, position))},
        {"theta", 3, GL_FLOAT, reinterpret_cast<void *>(offsetof(sim::PosNormTexVertex, normal))},
        {"tex_coords", 2, GL_FLOAT, reinterpret_cast<void *>(offsetof(sim::PosNormTexVertex, texCoords))},
    };
    return elements;
}
} // namespace

OrbRenderer::OrbRenderer(vmp::Orb orb)
    : orb(std::move(orb)),
      renderer_(vmp::shader_path() + "orb.vert"),
      mesh_([&](int u_divs, int v_divs)
            { return create_sphere_mesh_data(u_divs, v_divs); })
{
    renderer_.setDisplayMode(3);
    renderer_.setUsingWireframe(true);
    renderer_.setShowNormals(true);
    renderer_.setNormalScale(0.1f);

    auto &vals = orb.get_fft_vals();
    texture_ = sim::OpenGLHelper::createTextureArray(static_cast<int>(vals.size()),
                                                     1,
                                                     vals.data(),
                                                     GL_NEAREST,
                                                     GL_CLAMP_TO_EDGE,
                                                     GL_R32F,
                                                     GL_RED);

    renderer_.setDataFun([&]
                         { return mesh_.getMeshData(); });
}

void OrbRenderer::update(double world_time, double scale)
{
    orb.update(world_time, scale);
    auto &vals = orb.get_fft_vals();
    sim::OpenGLHelper::resetTextureArray(texture_, static_cast<int>(vals.size()), 1, vals.data(), GL_R32F, GL_RED);
    renderer_.setTexture(texture_);
}

void OrbRenderer::render(float alpha, const sim::Camera &camera) const
{
    if (renderer_.isShowNormals()) {
        renderer_.customRender(alpha,
                               &camera,
                               renderer_.getDrawMode(),
                               1,
                               renderer_.getShapeColor(),
                               renderer_.getLightDir(),
                               true,
                               renderer_.getNormalScale());
    }

    renderer_.customRender(alpha,
                           &camera,
                           renderer_.getDrawMode(),
                           renderer_.getDisplayMode(),
                           renderer_.getShapeColor(),
                           renderer_.getLightDir(),
                           false);
}

void OrbRenderer::configureGui()
{
    if (ImGui::CollapsingHeader("Mesh Options", "mesh", false, true)) {
        bool mesh_needs_update = mesh_.configureGui();
        if (mesh_needs_update) {
            renderer_.rebuild_mesh();
        }
    }

    if (ImGui::CollapsingHeader("Render Options", "render", false, true)) {
        renderer_.onGuiRender();
    }
}

void OrbRenderer::resize(int width, int height)
{
    renderer_.onResize(width, height);
}

sim::PosNormTexData OrbRenderer::create_sphere_mesh_data(int u_divisions, int v_divisions)
{
    sim::PosNormTexData data{};

    data.vbo.reserve(static_cast<unsigned>((u_divisions + 2) * (v_divisions + 2)));

    for (int thetai = 0; thetai < (u_divisions + 2); ++thetai) {
        float u = float(thetai) / (u_divisions + 1);

        for (int phii = 0; phii < (v_divisions + 2); ++phii) {
            float v = float(phii) / (v_divisions + 1);
            data.vbo.emplace_back(sim::PosNormTexVertex{{0, v, 0}, {u, 0, 0}, {u, v}});
        }
    }

    assert(data.vbo.size() == data.vbo.capacity());

    data.ibo.reserve(static_cast<unsigned>((u_divisions + 1) * ((v_divisions + 2) * 2 + 1)));

    unsigned index = 0;
    for (int thetai = 0; thetai < (u_divisions + 1); ++thetai) {
        for (int phii = 0; phii < (v_divisions + 2); ++phii) {
            data.ibo.push_back(index);
            data.ibo.push_back(index + v_divisions + 2);
            ++index;
        }
        data.ibo.push_back(sim::primitiveRestart());
    }
    data.ibo.pop_back();

    assert(data.ibo.size() == data.ibo.capacity() - 1);

    data.vaoElements = phiThetaElements();
    return data;
}

} // namespace vmp
