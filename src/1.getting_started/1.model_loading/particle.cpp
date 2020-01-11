#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
class Particle 
{ 
    // Access specifier 
    public:
        glm::vec3 pos, speed;
        glm::vec4 color;
        float size, angle, weight;
        float life;
        float cameradistance;

        Particle():pos(0.0f), speed(0.0f), color(1.0f), life(-1.0f), size(10.0f) { }

        bool operator<(const Particle& that) const {
            // Sort in reverse order : far particles drawn first.
            return this->cameradistance > that.cameradistance;
        }
}; 