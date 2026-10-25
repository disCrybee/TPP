#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {
constexpr unsigned WIDTH = 960;
constexpr unsigned HEIGHT = 720;
constexpr float MU = 900000.f; // G*M, arbitrary simulation units
constexpr float CENTRAL_RADIUS = 24.f;
constexpr float PARTICLE_RADIUS = 7.f;
constexpr float TIME_STEP = 1.f / 240.f;
constexpr sf::Vector2f CENTER{480.f, 360.f};

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
};

float magnitude(sf::Vector2f v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f acceleration(sf::Vector2f position) {
    sf::Vector2f offset = position - CENTER;
    float distance = std::max(magnitude(offset), CENTRAL_RADIUS);
    float scale = -MU / (distance * distance * distance);
    return offset * scale;
}

Particle initialParticle(float radius, float speedMultiplier) {
    float orbitalSpeed = std::sqrt(MU / radius);
    return {{CENTER.x + radius, CENTER.y}, {0.f, -orbitalSpeed * speedMultiplier}};
}

// Velocity Verlet integration: stable for orbital motion with a small fixed step.
void integrate(Particle& particle, float dt) {
    sf::Vector2f oldAcceleration = acceleration(particle.position);
    particle.position += particle.velocity * dt + oldAcceleration * (0.5f * dt * dt);
    sf::Vector2f newAcceleration = acceleration(particle.position);
    particle.velocity += (oldAcceleration + newAcceleration) * (0.5f * dt);
}

std::string titleText(float radius, float multiplier, const Particle& particle,
                      bool running, float elapsed) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2);
    out << "SFML Lab 5 | Variant 15 | " << (running ? "RUNNING" : "PAUSED")
        << " | R=" << std::setprecision(0) << radius
        << " | Speed x" << std::setprecision(2) << multiplier
        << " | Current speed=" << std::setprecision(1) << magnitude(particle.velocity)
        << " | t=" << elapsed << " s";
    return out.str();
}
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "SFML Lab 5 - Variant 15");
    window.setFramerateLimit(60);

    float startRadius = 210.f;
    float startSpeedMultiplier = 1.f;
    Particle particle = initialParticle(startRadius, startSpeedMultiplier);
    std::vector<sf::Vector2f> trail;
    trail.push_back(particle.position);
    bool running = false;
    float elapsed = 0.f;
    float accumulator = 0.f;
    sf::Clock frameClock;

    sf::CircleShape central(CENTRAL_RADIUS);
    central.setOrigin({CENTRAL_RADIUS, CENTRAL_RADIUS});
    central.setPosition(CENTER);
    central.setFillColor(sf::Color(250, 194, 50));

    sf::CircleShape dot(PARTICLE_RADIUS);
    dot.setOrigin({PARTICLE_RADIUS, PARTICLE_RADIUS});
    dot.setFillColor(sf::Color(85, 215, 255));

    sf::CircleShape referenceOrbit(startRadius);
    referenceOrbit.setOrigin({startRadius, startRadius});
    referenceOrbit.setPosition(CENTER);
    referenceOrbit.setFillColor(sf::Color::Transparent);
    referenceOrbit.setOutlineThickness(1.f);
    referenceOrbit.setOutlineColor(sf::Color(55, 65, 92));

    auto reset = [&]() {
        particle = initialParticle(startRadius, startSpeedMultiplier);
        trail.clear();
        trail.push_back(particle.position);
        elapsed = 0.f;
        accumulator = 0.f;
        running = false;
    };

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                switch (key->code) {
                    case sf::Keyboard::Key::Space:
                        running = !running;
                        break;
                    case sf::Keyboard::Key::R:
                        reset();
                        break;
                    case sf::Keyboard::Key::Up:
                        startSpeedMultiplier = std::clamp(startSpeedMultiplier + 0.05f, 0.2f, 1.8f);
                        reset();
                        break;
                    case sf::Keyboard::Key::Down:
                        startSpeedMultiplier = std::clamp(startSpeedMultiplier - 0.05f, 0.2f, 1.8f);
                        reset();
                        break;
                    case sf::Keyboard::Key::Right:
                        startRadius = std::clamp(startRadius + 10.f, 70.f, 290.f);
                        reset();
                        break;
                    case sf::Keyboard::Key::Left:
                        startRadius = std::clamp(startRadius - 10.f, 70.f, 290.f);
                        reset();
                        break;
                    default: break;
                }
            }
        }

        float frameDt = std::min(frameClock.restart().asSeconds(), 0.05f);
        if (running) {
            accumulator += frameDt;
            while (accumulator >= TIME_STEP) {
                integrate(particle, TIME_STEP);
                elapsed += TIME_STEP;
                accumulator -= TIME_STEP;
                if (magnitude(particle.position - CENTER) <= CENTRAL_RADIUS + PARTICLE_RADIUS) {
                    running = false; // collision with the central body
                    accumulator = 0.f;
                    break;
                }
            }
            trail.push_back(particle.position);
            if (trail.size() > 3500) trail.erase(trail.begin());
        }

        referenceOrbit.setRadius(startRadius);
        referenceOrbit.setOrigin({startRadius, startRadius});
        dot.setPosition(particle.position);
        window.setTitle(titleText(startRadius, startSpeedMultiplier, particle, running, elapsed));

        window.clear(sf::Color(10, 14, 28));
        window.draw(referenceOrbit);
        if (trail.size() > 1) {
            sf::VertexArray line(sf::PrimitiveType::LineStrip, trail.size());
            for (std::size_t i = 0; i < trail.size(); ++i) {
                line[i].position = trail[i];
                line[i].color = sf::Color(65, 190, 235);
            }
            window.draw(line);
        }
        window.draw(central);
        window.draw(dot);
        window.display();
    }
}
