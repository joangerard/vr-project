#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/shader_m.h>
#include <vector>

#include "particle.cpp"

class ParticleContainer {
    public:
        GLuint nr_particles = 5000;
        std::vector<Particle> particles;
        GLuint LastUsedParticle = 0;
        GLuint VAO;
        Shader shader;
        glm::vec3 posInit;

        ParticleContainer(Shader shader, glm::vec3 posInit) : shader("particle.vs", "particle.fs") {
            this->shader = shader;
            this->initBuffers();
            this->posInit = posInit;
            for (GLuint i = 0; i < this->nr_particles; ++i)
                this->particles.push_back(Particle());
        }

        void generateParticles(float delta) {
            // Generate 10 new particule each millisecond,
            // but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
            // newparticles will be huge and the next frame even longer.
            int newparticles = (int)(delta*this->nr_particles);

            if (newparticles > (int)(0.016f*this->nr_particles)) {
                newparticles = (int)(0.016f*this->nr_particles);   
            }

            for(int i=0; i < newparticles; i++){
                int particleIndex = this->FindUnusedParticle();
                this->particles[particleIndex].life = 4.0f; // This particle will live 4 seconds.
                this->particles[particleIndex].pos = this->posInit;

                float spread = 0.2f;
                glm::vec3 maindir = glm::vec3(0.0f, 1.0f, 0.0f);

                glm::vec3 randomdir = glm::vec3(
                    (rand()%2000 - 1000.0f)/1000.0f,
                    (rand()%2000 - 1000.0f)/1000.0f,
                    (rand()%2000 - 1000.0f)/1000.0f
                );
                
                this->particles[particleIndex].speed = maindir + randomdir * spread;

                GLfloat rColor = 0.5 + ((rand() % 100) / 100.0f);
                this->particles[particleIndex].color =  glm::vec4(rColor, rColor, rColor, 1.0f);

                this->particles[particleIndex].size = 10.0f;
            }
        }

        void initBuffers() {
            // Set up mesh and attribute properties
            GLuint VBO;
            GLfloat particle_quad[] = {
                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,

                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f
            }; 
            glGenVertexArrays(1, &this->VAO);
            glGenBuffers(1, &VBO);
            glBindVertexArray(this->VAO);
            // Fill mesh buffer
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
            // Set mesh attributes
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
            glBindVertexArray(0);
        }

        // Finds a Particle in particles which isn't used yet.
        // (i.e. life < 0);
        int FindUnusedParticle(){

            for(int i=this->LastUsedParticle; i < this->nr_particles; i++){
                if (this->particles[i].life < 0){
                    this->LastUsedParticle = i;
                    return i;
                }
            }

            for(int i=0; i < this->LastUsedParticle; i++){
                if (this->particles[i].life < 0){
                    this->LastUsedParticle = i;
                    return i;
                }
            }

            return 0; // All particles are taken, override the first one
        }

        int simulateParticles(double delta) {
            // Simulate all particles
            int ParticlesCount = 0;
            for(int i=0; i < this->nr_particles; i++){

                Particle& p = this->particles[i]; // shortcut

                if(p.life > 0.0f){

                    // Decrease life
                    p.life -= delta;
                    if (p.life > 0.0f){

                        // Simulate simple physics : gravity only, no collisions
                        p.speed += glm::vec3(0.0f,-9.81f, 0.0f) * (float)delta * 0.5f;
                        p.pos += p.speed * (float)delta;
                        // p.color.a -= delta * 2.5;
                    }

                    ParticlesCount++;

                }
            }

            return ParticlesCount;
        }

        void draw(unsigned int Texture, glm::mat4 proj, glm::mat4 view) {
           // Use additive blending to give it a 'glow' effect
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            this->shader.use();
            for (Particle particle : this->particles)
            {
                if (particle.life > 0.0f)
                {
                    this->shader.setVec3("offset", particle.pos);
                    this->shader.setVec4("color", particle.color);
                    this->shader.setMat4("projection", proj);
                    this->shader.setMat4("view", view);
                    this->shader.setInt("sprite", 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, Texture);
                    glBindVertexArray(this->VAO);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glBindVertexArray(0);
                }
            }
            // Don't forget to reset to default blending mode
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        void deleteBuffers() {
            // Cleanup VBO and shader
            glDeleteBuffers(1, &this->VAO);
        }
};