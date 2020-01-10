#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/shader_m.h>

#include "particle.cpp"
const int MaxParticles = 100000;

class ParticleContainer {
    public:
        
        Particle ParticlesContainer[MaxParticles];
        int LastUsedParticle = 0;
        GLuint billboard_vertex_buffer;
        GLuint particles_position_buffer;
        GLuint particles_color_buffer;
        GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
        GLubyte* g_particule_color_data         = new GLubyte[MaxParticles * 4];
        Shader shader;

        ParticleContainer(Shader shader) : shader("particle.vs", "particle.fs") {
            this->shader = shader;
        }

        void initLifeCamera() {
            for(int i=0; i < MaxParticles; i++){
                this->ParticlesContainer[i].life = -1.0f;
                this->ParticlesContainer[i].cameradistance = -1.0f;
            }
        }

        void generateParticles(float delta) {

            // Generate 10 new particule each millisecond,
            // but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
            // newparticles will be huge and the next frame even longer.
            int newparticles = (int)(delta*MaxParticles);

            if (newparticles > (int)(0.016f*MaxParticles)) {
                newparticles = (int)(0.016f*MaxParticles);   
            }

            for(int i=0; i < newparticles; i++){
                int particleIndex = this->FindUnusedParticle();
                this->ParticlesContainer[particleIndex] = Particle();
                this->ParticlesContainer[particleIndex].life = 5.0f; // This particle will live 5 seconds.
                this->ParticlesContainer[particleIndex].pos = glm::vec3(0,0,0.0f);

                float spread = 1.5f;
                glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);

                glm::vec3 randomdir = glm::vec3(
                    (rand()%2000 - 1000.0f)/1000.0f,
                    (rand()%2000 - 1000.0f)/1000.0f,
                    (rand()%2000 - 1000.0f)/1000.0f
                );
                
                this->ParticlesContainer[particleIndex].speed = maindir + randomdir * spread;


                // Very bad way to generate a random color
                this->ParticlesContainer[particleIndex].r = rand() % 256;
                this->ParticlesContainer[particleIndex].g = rand() % 256;
                this->ParticlesContainer[particleIndex].b = rand() % 256;
                this->ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

                this->ParticlesContainer[particleIndex].size = (rand()%1000)/2000.0f + 0.1f;
            }
        }

        void initBuffers() {

            const GLfloat g_vertex_buffer_data[] = {
                -0.5f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f,
                -0.5f, 0.5f, 0.0f,
                0.5f, 0.5f, 0.0f,
            };

            glGenBuffers(1, &this->billboard_vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, this->billboard_vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

            // The VBO containing the positions and sizes of the particles
            glGenBuffers(1, &this->particles_position_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, this->particles_position_buffer);
            // Initialize with empty (NULL) buffer : it will be updated later, each frame.
            glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

            // The VBO containing the colors of the particles
            glGenBuffers(1, &this->particles_color_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, this->particles_color_buffer);
            // Initialize with empty (NULL) buffer : it will be updated later, each frame.
            glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
        }

        // Finds a Particle in ParticlesContainer which isn't used yet.
        // (i.e. life < 0);
        int FindUnusedParticle(){

            for(int i=this->LastUsedParticle; i < MaxParticles; i++){
                if (this->ParticlesContainer[i].life < 0){
                    this->LastUsedParticle = i;
                    return i;
                }
            }

            for(int i=0; i < this->LastUsedParticle; i++){
                if (this->ParticlesContainer[i].life < 0){
                    this->LastUsedParticle = i;
                    return i;
                }
            }

            return 0; // All particles are taken, override the first one
        }

        int simulateParticles(double delta, glm::vec3 CameraPosition) {
            // Simulate all particles
            int ParticlesCount = 0;
            for(int i=0; i < MaxParticles; i++){

                Particle& p = this->ParticlesContainer[i]; // shortcut

                if(p.life > 0.0f){

                    // Decrease life
                    p.life -= delta;
                    if (p.life > 0.0f){

                        // Simulate simple physics : gravity only, no collisions
                        p.speed += glm::vec3(0.0f,-9.81f, 0.0f) * (float)delta * 0.5f;
                        p.pos += p.speed * (float)delta;
                        p.cameradistance = glm::length( p.pos - CameraPosition);
                        //ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

                        // Fill the GPU buffer
                        this->g_particule_position_size_data[4*ParticlesCount+0] = p.pos.x;
                        this->g_particule_position_size_data[4*ParticlesCount+1] = p.pos.y;
                        this->g_particule_position_size_data[4*ParticlesCount+2] = p.pos.z;
                                                    
                        this->g_particule_position_size_data[4*ParticlesCount+3] = p.size;
                                                    
                        this->g_particule_color_data[4*ParticlesCount+0] = p.r;
                        this->g_particule_color_data[4*ParticlesCount+1] = p.g;
                        this->g_particule_color_data[4*ParticlesCount+2] = p.b;
                        this->g_particule_color_data[4*ParticlesCount+3] = p.a;

                    }else{
                        // Particles that just died will be put at the end of the buffer in SortParticles();
                        p.cameradistance = -1.0f;
                    }

                    ParticlesCount++;

                }
            }

            this->SortParticles();

            return ParticlesCount;
        }

        void draw(int ParticlesCount, unsigned int Texture, glm::vec3 CameraRight, glm::vec3 CameraUp, glm::mat4 ViewProjectionMatrix) {

            // Clear the screen

            glBindBuffer(GL_ARRAY_BUFFER, this->particles_position_buffer);
            glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning
            glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, this->g_particule_position_size_data);

            glBindBuffer(GL_ARRAY_BUFFER, this->particles_color_buffer);
            glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning
            glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, this->g_particule_color_data);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Use our shader
            this->shader.use();

            // Bind our texture in Texture Unit 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture);
            this->shader.setInt("myTextureSampler", 0);
            this->shader.setVec3("CameraRight_worldspace", CameraRight);
            this->shader.setVec3("CameraUp_worldspace", CameraUp);
            this->shader.setMat4("VP", ViewProjectionMatrix);

            // 1rst attribute buffer : vertices
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, this->billboard_vertex_buffer);
            glVertexAttribPointer(
                0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
            );
            
            // 2nd attribute buffer : positions of particles' centers
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, this->particles_position_buffer);
            glVertexAttribPointer(
                1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                4,                                // size : x + y + z + size => 4
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                0,                                // stride
                (void*)0                         // array buffer offset
            );

            // 3rd attribute buffer : particles' colors
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, this->particles_color_buffer);
            glVertexAttribPointer(
                2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                4,                                // size : r + g + b + a => 4
                GL_UNSIGNED_BYTE,                 // type
                GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
                0,                                // stride
                (void*)0                        // array buffer offset
            );

            // These functions are specific to glDrawArrays*Instanced*.
            // The first parameter is the attribute buffer we're talking about.
            // The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
            // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
            glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
            glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
            glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

            // Draw the particules !
            // This draws many times a small triangle_strip (which looks like a quad).
            // This is equivalent to :
            // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
            // but faster.
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);
        }

        void deleteBuffers() {
            delete[] this->g_particule_position_size_data;

            // Cleanup VBO and shader
            glDeleteBuffers(1, &this->particles_color_buffer);
            glDeleteBuffers(1, &this->particles_position_buffer);
            glDeleteBuffers(1, &this->billboard_vertex_buffer);
        }

        void SortParticles(){
            std::sort(&this->ParticlesContainer[0], &this->ParticlesContainer[MaxParticles]);
        }
};