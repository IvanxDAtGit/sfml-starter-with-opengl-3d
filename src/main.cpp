#include <glad/gl.h> // Must be first!
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

static constexpr float pi = 3.14159265358979323846f;

namespace {
	struct part{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
	};

	struct joint : part {
		glm::vec3 center;
		glm::vec3 normal;
	};

    Vertex makeVertex(const glm::vec3& position, const glm::vec2& texCoord)
    {
        Vertex vertex;
        vertex.position = position;
        vertex.texCoord = texCoord;
        return vertex;
    };

    //FROM STARTER CODE/ THE GIVEN GIT HUB REPO
    //NOT MY CODE, TAKEN FROM THE GIVEN STARTING REOP
    part makeCube()
    {
        constexpr glm::vec2 bottomLeft(0.0f, 0.0f);
        constexpr glm::vec2 bottomRight(1.0f, 0.0f);
        constexpr glm::vec2 topRight(1.0f, 1.0f);
        constexpr glm::vec2 topLeft(0.0f, 1.0f);

        part cube;
        cube.vertices = {
            // Front
            makeVertex({ -1.0f, -1.0f,  1.0f }, bottomLeft),
            makeVertex({ 1.0f, -1.0f,  1.0f }, bottomRight),
            makeVertex({ 1.0f,  1.0f,  1.0f }, topRight),
            makeVertex({ -1.0f,  1.0f,  1.0f }, topLeft),
            // Back
            makeVertex({ 1.0f, -1.0f, -1.0f }, bottomLeft),
            makeVertex({ -1.0f, -1.0f, -1.0f }, bottomRight),
            makeVertex({ -1.0f,  1.0f, -1.0f }, topRight),
            makeVertex({ 1.0f,  1.0f, -1.0f }, topLeft),
            // Right
            makeVertex({ 1.0f, -1.0f,  1.0f }, bottomLeft),
            makeVertex({ 1.0f, -1.0f, -1.0f }, bottomRight),
            makeVertex({ 1.0f,  1.0f, -1.0f }, topRight),
            makeVertex({ 1.0f,  1.0f,  1.0f }, topLeft),
            // Left
            makeVertex({ -1.0f, -1.0f, -1.0f }, bottomLeft),
            makeVertex({ -1.0f, -1.0f,  1.0f }, bottomRight),
            makeVertex({ -1.0f,  1.0f,  1.0f }, topRight),
            makeVertex({ -1.0f,  1.0f, -1.0f }, topLeft),
            // Top
            makeVertex({ -1.0f,  1.0f,  1.0f }, bottomLeft),
            makeVertex({ 1.0f,  1.0f,  1.0f }, bottomRight),
            makeVertex({ 1.0f,  1.0f, -1.0f }, topRight),
            makeVertex({ -1.0f,  1.0f, -1.0f }, topLeft),
            // Bottom
            makeVertex({ -1.0f, -1.0f, -1.0f }, bottomLeft),
            makeVertex({ 1.0f, -1.0f, -1.0f }, bottomRight),
            makeVertex({ 1.0f, -1.0f,  1.0f }, topRight),
            makeVertex({ -1.0f, -1.0f,  1.0f }, topLeft)
        };

        cube.indices.reserve(36);
        for (unsigned int face = 0; face < 6; ++face) {
            const unsigned int first = face * 4;
            cube.indices.insert(cube.indices.end(),
                { first, first + 1, first + 2,
                 first, first + 2, first + 3 });
        }

        return cube;
    }
    
    part makeBase(unsigned int wedges) {
        part Base;
        Base.vertices.reserve(wedges + 1);
        Base.indices.reserve(3 * wedges);

        //Centre dot
        Vertex centre;
        glm::vec3 p = glm::vec3(0, 0, 0);
        glm::vec3 n = glm::vec3(0, 1, 0);
        glm::vec2 tC = glm::vec2(0.5f, 0.5f);
        centre.position = p;
        centre.normal = n;
        centre.texCoord = tC;

        Base.vertices.push_back(centre);

        for (unsigned int wedge = 0; wedge < wedges; ++wedge) {
            const float theta = static_cast<float>(wedge) * 2 * pi / static_cast<float>(wedges);
            const float x = std::cos(theta);
            const float z = std::sin(theta);
            Vertex vertex;
            const glm::vec3 pos(x, 0, z);
            const glm::vec3 up(0, 1.0f, 0);

            vertex.position = pos;
            vertex.normal = up;
            vertex.texCoord = glm::vec2(0.5 + x, 0.5 + z);

            Base.vertices.push_back(vertex);


        }

        for (unsigned int wedge = 0; wedge < wedges; ++wedge) {
            if (wedge + 1 < wedges) {
                Base.indices.insert(
                    Base.indices.end(),
                    { 0,  wedge + 1, wedge + 2 });
            }

            else {
                Base.indices.insert(
                    Base.indices.end(),
                    { 0,  wedge + 1, 1 });
            }

        }
        return Base;
    }


    part makeCyliner(unsigned int slices, unsigned int stacks) {
        part cylinder;
        cylinder.vertices.reserve(static_cast<std::size_t>(stacks + 1) * (slices + 1));
        cylinder.vertices.reserve(static_cast<std::size_t>(stacks) * slices * 6);
        for (unsigned int slice = 0; slice <= slices; ++slice) {
            // v represents which slice we are working on
            const float v = static_cast<float>(slice) / static_cast<float>(slices);
            const float radians = v * 2 * pi;
            const float x = std::cos(radians);
            const float z = std::sin(radians);

            // Since we are doing a cylinder, we can treat the normal the same as a circle (= (x,y)/||x,y)|| at x,y)
            const glm::vec3 normal(x, 0, z);

            for (unsigned int stack = 0; stack <= stacks; ++stack) {
                Vertex vertex;
                const float y = static_cast<float>(stack) / static_cast<float>(stacks);
                const glm::vec3 pos(x, 2 * pi * y, z); //2pi to account for a square needing height = width(circumference), counter the stretch

                vertex.position = pos;
                vertex.normal = normal;

                // Change this to change how many times the image repeats, v and y have range [0,1]
                vertex.texCoord = glm::vec2(v, y);
                cylinder.vertices.push_back(vertex);
            }
        }

        const unsigned int height = stacks + 1;

        for (unsigned int slice = 0; slice < slices; ++slice) {
            const unsigned int leftSide = slice * height;
            const unsigned int rightSide = leftSide + height;
            for (unsigned int stack = 0; stack < stacks; ++stack) {
                const unsigned int bottomLeft = leftSide + stack;
                const unsigned int bottomRight = rightSide + stack;
                const unsigned int topLeft = bottomLeft + 1;
                const unsigned int topRight = bottomRight + 1;

                cylinder.indices.insert(cylinder.indices.end(),
                    { bottomLeft, bottomRight, topRight,
                     bottomLeft, topRight, topLeft });
            }
        }
        return cylinder;
    }

    //FROM STARTER CODE/ THE GIVEN GIT HUB REPO
    //NOT MY CODE, TAKEN FROM THE GIVEN STARTING REOP
    part makeSphere(unsigned int slices, unsigned int stacks)
    {
        constexpr float pi = 3.14159265358979323846f;
        part sphere;
        sphere.vertices.reserve(static_cast<std::size_t>(stacks + 1) * (slices + 1));
        sphere.indices.reserve(static_cast<std::size_t>(stacks) * slices * 6);

        for (unsigned int stack = 0; stack <= stacks; ++stack) {
            const float v = static_cast<float>(stack) / static_cast<float>(stacks);
            const float theta = pi * v;
            const float sinTheta = std::sin(theta);
            const float cosTheta = std::cos(theta);

            for (unsigned int slice = 0; slice <= slices; ++slice) {
                const float u = static_cast<float>(slice) / static_cast<float>(slices);
                const float phi = 2.0f * pi * u;
                const glm::vec3 normal(sinTheta * std::cos(phi),
                    cosTheta,
                    sinTheta * std::sin(phi));

                Vertex vertex;
                vertex.position = normal;
                vertex.normal = normal;
                vertex.texCoord = glm::vec2(2 * u, 1.0f - v);
                sphere.vertices.push_back(vertex);
            }
        }

        const unsigned int rowLength = slices + 1;
        for (unsigned int stack = 0; stack < stacks; ++stack) {
            for (unsigned int slice = 0; slice < slices; ++slice) {
                const unsigned int topLeft = stack * rowLength + slice;
                const unsigned int bottomLeft = topLeft + rowLength;
                const unsigned int topRight = topLeft + 1;
                const unsigned int bottomRight = bottomLeft + 1;

                sphere.indices.insert(sphere.indices.end(),
                    { topLeft, topRight, bottomLeft,
                     topRight, bottomRight, bottomLeft });
            }
        }

        return sphere;
    }

    //FROM STARTER CODE/ THE GIVEN GIT HUB REPO
    //NOT MY CODE, TAKEN FROM THE GIVEN STARTING REOP
    void rotate(Mesh& mesh, float radians) {
        mesh.applyTransform(glm::rotate(glm::mat4(1.0f),
            radians,
            glm::vec3(1.0f, 0, 0)));
    }
    //Takes into account previous translations, the previous translation gives a new 

    //We make a affine translate with glm then we apply it to the mesh with mesh.applytransform
    void translate(Mesh& mesh, float x, float y, float z) {
        mesh.applyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)));
    }

}

int main(int argc, char* argv[]) {
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

        sf::Window window(sf::VideoMode({ 800, 800 }),
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
        part cylinder = makeCyliner(64, 32);
        part box = makeCube();
        part base = makeBase(64);
        const std::filesystem::path shaderDirectory =
            std::filesystem::absolute(argv[0]).parent_path() / "shaders";
        Material material;
        material.lightingEnabled = false;

        std::vector<std::unique_ptr<Mesh>> Objects;

        Objects.push_back(std::make_unique<Mesh>(cylinder.vertices,
            cylinder.indices,
            (shaderDirectory / "mesh.vert").string(),
            (shaderDirectory / "mesh.frag").string(),
            argv[1],
            material)
        );

        for (auto& mesh : Objects) {
            translate(*mesh, 0, 1, 0);
        }
        //X is L-R
        //Z is in and out

        Objects.push_back(std::make_unique<Mesh>(box.vertices,
            box.indices,
            (shaderDirectory / "mesh.vert").string(),
            (shaderDirectory / "mesh.frag").string(),
            (std::filesystem::absolute(argv[0]).parent_path().parent_path().parent_path() / "assets" / "marble.png").string(),
            material)
        );


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
                else if (const auto* resized = event->getIf<sf::Event::Resized>())
                {
                    glViewport(0,
                        0,
                        static_cast<GLsizei>(resized->size.x),
                        static_cast<GLsizei>(resized->size.y));
                }
                else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    if (keyPressed->code == sf::Keyboard::Key::Left)
                    {
                    }
                    else if (keyPressed->code == sf::Keyboard::Key::Right)
                    {
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

            for (auto& mesh : Objects) {
                rotate(*mesh, rotationSpeed * dt);
            }

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
            for (auto& mesh : Objects) {
                mesh->draw(view, projection);
            }
            window.display();
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Application failed: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}