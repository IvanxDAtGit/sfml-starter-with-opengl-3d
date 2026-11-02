<<<<<<< Updated upstream
#include <glad/gl.h> // Must be first!
=======
#include <glad/gl.h>
>>>>>>> Stashed changes
#include <SFML/Window.hpp>
#include <SFML/System/Clock.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "mesh.h"

<<<<<<< Updated upstream
static constexpr float pi = 3.14159265358979323846f;

namespace{
    struct CylinderGeometry{
        std::vector<Vertex> verticies;
        std::vector<unsigned int> indicies;
    };

    void rotate(Mesh &mesh, float radians){
        mesh.applyTransform(glm::rotate(glm::mat4(1.0f),
                                        radians,
                                        glm::vec3(3.0f, 1.0f, 0.0f)));}

    // Most of the code for cylinder mirrors a sphere, we just dont curve inwards at the ends, but stack straight up
    CylinderGeometry makeCyliner(unsigned int slices, unsigned int stacks){
        CylinderGeometry cylinder;
        cylinder.verticies.reserve(static_cast<std::size_t>(stacks + 1) * (slices + 1));
        cylinder.indicies.reserve(static_cast<std::size_t>(stacks) * slices * 6);
        for (unsigned int slice = 0; slice <= slices; ++slice){
            // v represents which slice we are working on
            const float v = static_cast<float>(slice) / static_cast<float>(slices);
            const float radians = v * 2 * pi;
            const float x = std::cos(radians);
            const float z = std::sin(radians);

            // Since we are doing a cylinder, we can treat the normal the same as a circle (= (x,y)/||x,y)|| at x,y)
            const glm::vec3 normal(x, 0, z);

            for (unsigned int stack = 0; stack <= stacks; ++stack){
                Vertex vertex;
                const float y = static_cast<float>(stack) / static_cast<float>(stacks);
                const glm::vec3 pos(x, 2*pi*y, z); //2pi to account for a square needing height = width(circumference), counter the stretch

                vertex.position = pos;
                vertex.normal = normal;

                // Change this to change how many times the image repeats, v and y have range [0,1]
                vertex.texCoord = glm::vec2(v, y);
                cylinder.verticies.push_back(vertex);
            }
        }

        const unsigned int height = stacks + 1;

        for (unsigned int slice = 0; slice < slices; ++slice){
            const unsigned int leftSide = slice * height;
            const unsigned int rightSide = leftSide + height;
            for (unsigned int stack = 0; stack < stacks; ++stack){
                const unsigned int bottomLeft = leftSide + stack;
                const unsigned int bottomRight = rightSide + stack;
                const unsigned int topLeft = bottomLeft + 1;
                const unsigned int topRight = bottomRight + 1;

                cylinder.indicies.insert(cylinder.indicies.end(),
                                         {bottomLeft, bottomRight, topRight,
                                          bottomLeft, topRight, topLeft});
            }
        }
        return cylinder;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: cylinder <square-texture-image>\n";
        return 2;
    }

    try
    {
        sf::ContextSettings settings;
        settings.majorVersion = 4;
        settings.minorVersion = 0;
        settings.attributeFlags = sf::ContextSettings::Core;
        settings.depthBits = 24;

        sf::Window window(sf::VideoMode({800, 800}),
                          "Textured cylinder",
                          sf::Style::Default,
                          sf::State::Windowed,
                          settings);

        const sf::ContextSettings actualSettings = window.getSettings();
        if (actualSettings.majorVersion < 4)
        {
            throw std::runtime_error("OpenGL 4.0 Core is required, but the created context is " +
                                     std::to_string(actualSettings.majorVersion) + "." +
                                     std::to_string(actualSettings.minorVersion));
        }

        if (!gladLoadGL(sf::Context::getFunction))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return 1;
        }

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        CylinderGeometry cylinder = makeCyliner(64, 32);
        const std::filesystem::path shaderDirectory =
            std::filesystem::absolute(argv[0]).parent_path() / "shaders";
        Material material;
        material.lightingEnabled = false;
        Mesh mesh(cylinder.verticies,
                  cylinder.indicies,
                  (shaderDirectory / "mesh.vert").string(),
                  (shaderDirectory / "mesh.frag").string(),
                  argv[1],
                  material);

        constexpr float fieldOfView = glm::radians(45.0f);
        constexpr float cylinderRadius = 1.0f;
        const glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
        const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
        sf::Clock frameClock;
        const float rotationSpeed = glm::radians(30.0f);

        while (window.isOpen())
        {
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window.close();
                }
                else if (const auto *resized = event->getIf<sf::Event::Resized>())
                {
                    glViewport(0,
                               0,
                               static_cast<GLsizei>(resized->size.x),
                               static_cast<GLsizei>(resized->size.y));
                }
                else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    if (keyPressed->code == sf::Keyboard::Key::Left)
                    {
                        rotate(mesh, glm::radians(5.0f));
                    }
                    else if (keyPressed->code == sf::Keyboard::Key::Right)
                    {
                        rotate(mesh, glm::radians(-5.0f));
                    }
                    else if (keyPressed->code == sf::Keyboard::Key::Escape)
                    {
                        window.close();
                    }
                }
            }

            if (!window.isOpen())
            {
                break;
            }

            const float dt = frameClock.restart().asSeconds();
            rotate(mesh, rotationSpeed * dt);

            const sf::Vector2u size = window.getSize();
            glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
            const float aspectRatio = size.x == 0 || size.y == 0
                                          ? 1.0f
                                          : static_cast<float>(size.x) / static_cast<float>(size.y);
            const float limitingAspect = std::min(aspectRatio, 1.0f);
            const float limitingHalfFov = std::atan(
                std::tan(fieldOfView * 0.5f) * limitingAspect);
            const float cameraDistance = cylinderRadius / std::sin(limitingHalfFov) * 5.0f;
            const glm::mat4 view = glm::lookAt(cameraTarget + glm::vec3(0.0f,
                                                                        0.0f,
                                                                        cameraDistance),
                                               cameraTarget,
                                               worldUp);
            const glm::mat4 projection = glm::perspective(fieldOfView,
                                                          aspectRatio,
                                                          0.1f,
                                                          cameraDistance + cylinderRadius * 2.0f);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            mesh.draw(view, projection);
            window.display();
        }
    }
    catch (const std::exception &error)
    {
        std::cerr << "Application failed: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
=======
namespace {
struct CylinderGeometry {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};


}
static constexpr float pi = 3.14159265358979323846f;
>>>>>>> Stashed changes
