#include <sim-driver/renderers/RendererHelper.hpp>
#include <orb/OrbRenderer.hpp>
#include <sim-driver/OpenGLSimulation.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <orb/OrbUtil.hpp>

class Simulator
{
public:
    Simulator(int, int, sim::SimData *pSimData)
        : renderer_{vmp::Orb{128}},
          simData_{*pSimData}
    {
        simData_.camera().setNearPlane(0.1f);

        simData_.cameraMover.setUsingOrbitMode(true);
        simData_.cameraMover.setOrbitOrigin({0, 0, 0});
        simData_.cameraMover.setOrbitOffsetDistance(5);

        prevCam_ = simData_.camera();
        renderer_.update(0, scale_);
    }

    void onUpdate(double worldTime, double time_step)
    {
        renderer_.update(worldTime, scale_);
        prevCam_ = simData_.camera();
    }

    void onRender(int, int, double alpha)
    {
        auto a = static_cast<float>(alpha);

        if (simData_.paused) {
            renderer_.render(a, simData_.camera());
        }
        else {
            sim::Camera &currCam = simData_.camera();
            sim::Camera camera;

            glm::vec3 eye{glm::mix(prevCam_.getEyeVector(), currCam.getEyeVector(), a)};
            glm::vec3 look{glm::mix(prevCam_.getLookVector(), currCam.getLookVector(), a)};
            glm::vec3 up{glm::mix(prevCam_.getUpVector(), currCam.getUpVector(), a)};

            float aspect{glm::mix(prevCam_.getAspectRatio(), currCam.getAspectRatio(), a)};

            camera.lookAt(eye, eye + look, up);
            camera.perspective(currCam.getFovYDegrees(), aspect, currCam.getNearPlane(), currCam.getFarPlane());

            renderer_.render(a, camera);
        }
    }

    void onGuiRender(int, int)
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        if (ImGui::Begin("Window", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Framerate: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
            renderer_.configureGui();

            if (ImGui::CollapsingHeader("Orb Settings", "orb_settings", false, true)) {
                ImGui::SliderFloat("Scale", &scale_, 1.0f, 20.0f);
            }
        }
        ImGui::End();
    }

private:
    float scale_{10.0f};

    vmp::OrbRenderer renderer_;
    sim::Camera prevCam_;

    sim::SimData &simData_;
};

int main()
{
    try {
        sim::SimInitData initData;
        initData.title = "Heights Sim";
        sim::OpenGLSimulation<Simulator>(initData).runNoFasterThanRealTimeLoop();
    }
    catch (const std::exception &e) {
        std::cerr << "Program failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}