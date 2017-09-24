#include <orb/OrbRenderer.hpp>
#include <sim-driver/OpenGLHelper.hpp>
#include <sim-driver/Camera.hpp>
#include <orb/ShaderConfig.hpp>
#include <sim-driver/ShaderConfig.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

//#define NORMAL_RENDER

namespace sim
{

namespace
{

sim::PosNormTexVertex create_sphere_vertex(glm::vec3 p, glm::vec2 t)
{
    return {{p.x, p.y, p.z},
            {p.x, p.y, p.z},
            {t.x, t.y}};
}

sim::PosNormTexData create_sphere_mesh_data(int u_divisions, int v_divisions)
{
    sim::PosNormTexData data{};

    data.vbo.reserve(static_cast<unsigned>((u_divisions + 2) * (v_divisions + 2)));

    for (int thetai = 0; thetai < (u_divisions + 2); ++thetai) {
        float u = float(thetai) / (u_divisions + 1);
        float theta = glm::two_pi<float>() * u;

        for (int phii = 0; phii < (v_divisions + 2); ++phii) {
            float v = float(phii) / (v_divisions + 1);
            float phi = glm::pi<float>() * v;
            glm::vec3 p{glm::cos(theta) * glm::sin(phi),
                        glm::cos(phi),
                        glm::sin(theta) * glm::sin(phi)};
            data.vbo.emplace_back(create_sphere_vertex(p, glm::vec2{1.0f - u, v}));
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

    data.vaoElements = sim::posNormTexVaoElements();
    return data;
}
}

OrbRenderer::OrbRenderer(sim::Orb orb)
    : orb(std::move(orb)),
      renderer_(sim::shader_path() + "shader.vert"),
      mesh_(create_sphere_mesh_data)
{
//    program_ = sim::OpenGLHelper::createProgram(sim::shader_path() + "shader.vert", sim::frag_shader_file());

    renderer_.setDisplayMode(3);
    renderer_.setUsingWireframe(true);
    renderer_.setShowNormals(true);

    renderer_.setDataFun([&]
                         {
                             return mesh_.getMeshData();
                         });
}

void OrbRenderer::render(float alpha, const Camera &camera) const
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

} // namespace sim
