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
        unsigned char r,g,b,a;
        float size, angle, weight;
        float life;
        float cameradistance;

        bool operator<(const Particle& that) const {
            // Sort in reverse order : far particles drawn first.
            return this->cameradistance > that.cameradistance;
        }
}; 