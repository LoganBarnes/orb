#include <sim-driver/OpenGLSimulation.hpp>
#include <sim-driver/renderers/RendererHelper.hpp>
#include <sim-driver/renderers/MeshRenderer.hpp>
#include <sim-driver/meshes/MeshFunctions.hpp>
#include <glm/gtc/constants.hpp>

namespace
{

//std::vector<float>& orb()
//{
//    static std::vector<float> h{
//        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//        3, 3, 3, 3, 3, 3, 3, 3, 3, 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//    };
//    return h;
//}

sim::PosNormTexVertex create_sphere_vertex(glm::vec3 p, glm::vec2 t)
{
    return {{p.x, p.y, p.z},
            {0,   1,   0},
            {t.x, t.y}};
}

void build_ibo(sim::PosNormTexData &data, unsigned w, unsigned h)
{
    for (unsigned yi = 0; yi < h; ++yi)
    {
        unsigned bottom = (h - yi) * 2 * (w * 2 + 1);
        unsigned top = bottom - (w * 2 + 1);

        unsigned t = top;
        unsigned b = bottom;
        for (unsigned xi = 0; xi < w; ++xi)
        {
            data.ibo.emplace_back(t++);
            data.ibo.emplace_back(b++);
            data.ibo.emplace_back(t);
            data.ibo.emplace_back(b++);
            data.ibo.emplace_back(t++);
            data.ibo.emplace_back(b);
        }

        b = t;
        t = b - (w * 2 + 1);
        for (unsigned xi = 0; xi < w; ++xi)
        {
            data.ibo.emplace_back(b--);
            data.ibo.emplace_back(t--);
            data.ibo.emplace_back(b);
            data.ibo.emplace_back(t--);
            data.ibo.emplace_back(b--);
            data.ibo.emplace_back(t);
        }
        data.ibo.emplace_back(b);
        data.ibo.emplace_back(sim::primitiveRestart());
    }
}

sim::PosNormTexData create_sphere_mesh_data(int u_divisions, int v_divisions)
{
    sim::PosNormTexData data{};

    unsigned w = 20;
    unsigned h = 10;

    for (unsigned zi = 0; zi < h; ++zi)
    {
        float v = (zi + 0.5f) / h;
        float z = zi - (h * 0.5f);

        for (unsigned i = 0; i < 2; ++i)
        {
            z += i * 0.5f;
            for (unsigned xi = 0; xi < w; ++xi)
            {
                float u = (xi + 0.5f) / w;
                float x = xi - (w * 0.5f);

                data.vbo.emplace_back(create_sphere_vertex({x, 0, z}, {u, v}));
                data.vbo.emplace_back(create_sphere_vertex({x + 0.5f, 0, z}, {u, v}));
            }
            data.vbo.emplace_back(create_sphere_vertex({w - (w * 0.5f), 0, z}, {1, v}));
        }
    }

    float z = h - (h * 0.5f);
    float v = 1.0f;
    for (unsigned xi = 0; xi < w; ++xi)
    {
        float u = (xi + 0.5f) / w;
        float x = xi - (w * 0.5f);

        data.vbo.emplace_back(create_sphere_vertex({x, 0, z}, {u, v}));
        data.vbo.emplace_back(create_sphere_vertex({x + 0.5f, 0, z}, {u, v}));
    }
    data.vbo.emplace_back(create_sphere_vertex({w - (w * 0.5f), 0, z}, {1, v}));

    build_ibo(data, w, h);

    data.vaoElements = sim::posNormTexVaoElements();
    return data;
}

} // namespace


class Simulator
{
public:
    explicit Simulator(int, int, sim::SimData *pSimData)
        : renderer_{sim::PosNormTexMesh(create_sphere_mesh_data)},
          simData_{*pSimData}
    {
        simData_.cameraMover.setUsingOrbitMode(true);
        simData_.cameraMover.setOrbitOrigin({0, 0, 0});
        simData_.cameraMover.setOrbitOffsetDistance(15);

        prevCam_ = simData_.camera();
    }

    void onRender(int, int, double alpha)
    {
        renderer_.render(static_cast<float>(alpha), simData_.camera());
    }

    void onGuiRender(int, int)
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        if (ImGui::Begin("Window", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Framerate: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
            renderer_.configureGui();
        }
        ImGui::End();
    }

private:
    sim::MeshRenderer renderer_;
    sim::SimData &simData_;
    sim::Camera prevCam_;
};

int main()
{
    try
    {
        sim::SimInitData initData;
        initData.title = "Mesh Sim";
        sim::OpenGLSimulation<Simulator>(initData).runNoFasterThanRealTimeLoop();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Program failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}