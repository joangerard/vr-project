#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include "particle_container.cpp"

#include <iostream>

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(vector<std::string> faces);

void drawScene(Shader ourShader, Shader metal, Shader glassShader, Shader skyboxShader, Shader lampShader, Shader groundShader, unsigned int skyboxVAO,
 unsigned int cubeVAO, unsigned int planeVAO, unsigned int roofVAO, unsigned int glassWallsVAO, unsigned int cubemapTexture, unsigned int woodTexture, unsigned int marmolTexture,unsigned int woodTableTexture, 
 unsigned int roofTexture, glm::vec3 lightPos[], glm::vec3 lightColor[], Camera camera, Model ship, Model nanoSuitModel,
 Model sphere_mirrow, Model table, Model fountain, Model computer, float fov, float aspectRatio, glm::mat4 lightSpaceMatrix,  unsigned int depthMap, float rotationAngle);

 void drawSceneDepth(Shader shader, unsigned int planeVAO, glm::vec3 lightPos, Model ship, Model nanoSuitModel,
 Model sphere_mirrow, Model table, Model computer, Model fountain, float rotationAngle);

 unsigned int loadCubemap(unsigned int faces);
 unsigned int createEmptyCubemap(int size);
 void switchCam(Camera* cam, int i);
void checkFBOStatus();
unsigned int loadTexture(char const * path);
unsigned int getEmptyTexture(unsigned int width, unsigned int height);
void renderQuad();
void setLights(Shader shader, glm::vec3 lightPositions[], glm::vec3 lightColors[], glm::mat4 lightSpaceMatrix, unsigned int depthMap, int nrLights);
void getLightColors(glm::vec3 *pointLightColors);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

// camera
Camera camera(glm::vec3(6.5f, 2.0f, -6.8f), glm::vec3(0.0f, 1.0f, 0.0f), 135, -20);
// inverted camera to mirrowing
Camera invertedCam(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, -1.0f, 0.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// user to change lighting: mode party, dark, night.
int turn = 0;
bool activateMirrow = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "VR Diskotek", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);  

    // used to rotate nanosut and ship
    int rotationAngle = 0;

    // -------------------------
    // BUILD AND COMPILE SHADERS
    // -------------------------
    Shader ourShader("multiple_lights.vs", "multiple_lights.fs");
    Shader skyboxShader("cubemap.vs", "cubemap.fs");
    Shader lampShader("lamp.vs", "lamp.fs");
    Shader metal("metal.vs", "metal.fs");
    Shader glassShader("glass.vs", "glass.fs");
    Shader screenShader("framebuffers_screen.vs", "framebuffers_screen.fs");
    Shader simpleDepthShader("shadow_mapping_depth.vs", "shadow_mapping_depth.fs");
    Shader debugDepthQuad("debug_quad.vs", "debug_quad.fs");
    Shader groundShader("ground.vs", "ground.fs");
    Shader particleShader("particle.vs", "particle.fs");

    // --------------------
    // PARTICLE SYSTEM INITIALIZATION
    // ---------------------
    ParticleContainer* particleContainer = new ParticleContainer(particleShader, glm::vec3(4.5f, 1.0f, 0));
    particleContainer->initBuffers();

    ParticleContainer* particleContainer2 = new ParticleContainer(particleShader, glm::vec3(-4.5f, 1.0f, 0));
    particleContainer2->initBuffers();

    // load models
    // -----------
    Model ship(FileSystem::getPath("resources/objects/ship/99-intergalactic_spaceship-obj-1/Intergalactic_Spaceship-(Wavefront).obj"));
    Model nanoSuitModel(FileSystem::getPath("resources/objects/nanosuit/nanosuit.obj"));
    Model table(FileSystem::getPath("resources/objects/table/table.obj"));
    Model sphere_mirrow(FileSystem::getPath("resources/objects/ball/13517_Beach_Ball_v2_L3.obj"));
    Model fountain(FileSystem::getPath("resources/objects/angel/angel.obj"));
    Model computer(FileSystem::getPath("resources/objects/notebook/Lowpoly_Notebook_2.obj"));
    
    // load textures
    // -------------
    unsigned int woodTexture = loadTexture(FileSystem::getPath("resources/textures/wood.png").c_str());
    unsigned int waterTexture = loadTexture(FileSystem::getPath("resources/textures/water.png").c_str());
    unsigned int marmolTexture = loadTexture(FileSystem::getPath("resources/textures/marmol.png").c_str());
    unsigned int woodTableTexture = loadTexture(FileSystem::getPath("resources/textures/toy_box_diffuse.png").c_str());
    unsigned int roofTexture = loadTexture(FileSystem::getPath("resources/textures/wall.jpg").c_str());

    // load vertices
    // --------------
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // used for debugging for mirrowing facing.
    float quadVertices[] = { 
        // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates. 
        // NOTE that this plane is now much smaller and at the top of the screen
        // positions   // texCoords
        -0.3f,  1.0f,  0.0f, 1.0f,
        -0.3f,  0.7f,  0.0f, 0.0f,
         0.3f,  0.7f,  1.0f, 0.0f,

        -0.3f,  1.0f,  0.0f, 1.0f,
         0.3f,  0.7f,  1.0f, 0.0f,
         0.3f,  1.0f,  1.0f, 1.0f
    };

    // floor vertices
    // ---------------
    float planeVertices[] = {
        // positions            // normals         // texcoords
         7.0f, -0.5f,  7.0f,  0.0f, 1.0f, 0.0f,  7.0f,  0.0f,
        -7.0f, -0.5f,  7.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -7.0f, -0.5f, -7.0f,  0.0f, 1.0f, 0.0f,   0.0f, 7.0f,

         7.0f, -0.5f,  7.0f,  0.0f, 1.0f, 0.0f,  7.0f,  0.0f,
        -7.0f, -0.5f, -7.0f,  0.0f, 1.0f, 0.0f,   0.0f, 7.0f,
         7.0f, -0.5f, -7.0f,  0.0f, 1.0f, 0.0f,  7.0f, 7.0f
    };

    // Roof and white wall vertices
    float roofVertices[] = {
        // positions            // normals         // texcoords
        //roof
         7.0f, -0.5f,  7.0f,  0.0f, -1.0f, 0.0f,  7.0f,  0.0f,
        -7.0f, -0.5f,  7.0f,  0.0f, -1.0f, 0.0f,   0.0f,  0.0f,
        -7.0f, -0.5f, -7.0f,  0.0f, -1.0f, 0.0f,   0.0f, 7.0f,

         7.0f, -0.5f,  7.0f,  0.0f, -1.0f, 0.0f,  7.0f,  0.0f,
        -7.0f, -0.5f, -7.0f,  0.0f, -1.0f, 0.0f,   0.0f, 7.0f,
         7.0f, -0.5f, -7.0f,  0.0f, -1.0f, 0.0f,  7.0f, 7.0f,

        // right
        7.0f, 0.0f,  7.0f,  -1.0f, .0f, 0.0f,  7.0f,  0.0f,
         7.0f, -7.0f,  7.0f,  -1.0f, .0f, 0.0f,   0.0f,  0.0f,
         7.0f, -7.0f, -7.0f,  -1.0f, .0f, 0.0f,   0.0f, 7.0f,

         7.0f, 0.0f,  7.0f,  -1.0f, .0f, 0.0f,  7.0f,  0.0f,
         7.0f, -7.0f, -7.0f,  -1.0f, .0f, 0.0f,   0.0f, 7.0f,
         7.0f, 0.0f, -7.0f,  -1.0f, .0f, 0.0f,  7.0f, 7.0f,

        //left
         -7.0f, 0.0f, 7.0f,  1.0f, .0f, 0.0f,    7.0f,  0.0f,
         -7.0f, -7.0f,  7.0f,  1.0f, .0f, 0.0f,   0.0f, 0.0f,
         -7.0f, -7.0f, -7.0f,  1.0f, .0f, 0.0f,   0.0f, 7.0f,

         -7.0f, 0.0f,  7.0f,  1.0f, .0f, 0.0f,  7.0f,  0.0f,
         -7.0f, -7.0f, -7.0f,  1.0f, .0f, 0.0f,   0.0f, 7.0f,
         -7.0f, 0.0f, -7.0f,  1.0f, .0f, 0.0f,   7.0f, 7.0f

    };

    // glass wall vertices
    // -------------------
    float glassWallVertices[] = {
        // positions            // normals         // texcoords
         7.0f, 0.0f,  7.0f,  0.0f, .0f, -1.0f,  7.0f,  0.0f,
         7.0f, -7.0f, 7.0f,  0.0f, .0f, -1.0f,   0.0f, 0.0f,
         -7.0f, -7.0f,  7.0f,  0.0f, .0f, -1.0f,  0.0f, 7.0f,

         7.0f, 0.0f,  7.0f,  0.0f, .0f, -1.0f,  7.0f,  0.0f,
         -7.0f, 0.0f, 7.0f,  0.0f, .0f, -1.0f,   0.0f, 7.0f,
         -7.0f, -7.0f,  7.0f,  0.0f, .0f, -1.0f, 7.0f, 7.0f,

         -7.0f, -7.0f, -7.0f,  0.0f, .0f, 1.0f,   7.0f, 0.0f,
         -7.0f, 0.0f, -7.0f,  0.0f, .0f, 1.0f,   0.0f, 0.0f,
         7.0f, -7.0f, -7.0f,  0.0f, .0f, 1.0f,   0.0f, 7.0f,

         7.0f, 0.0f,  -7.0f,  0.0f, .0f, 1.0f,   7.0f,  0.0f,
         7.0f, -7.0f, -7.0f,  0.0f, .0f, 1.0f,   0.0f, 7.0f,
         -7.0f, 0.0f, -7.0f,  0.0f, .0f, 1.0f,   7.0f, 7.0f,

    };

    // cubmap faces
    vector<std::string> faces
    {
        FileSystem::getPath("resources/textures/skybox_maskonaive/right.jpg"),
        FileSystem::getPath("resources/textures/skybox_maskonaive/left.jpg"),
        FileSystem::getPath("resources/textures/skybox_maskonaive/top.jpg"),
        FileSystem::getPath("resources/textures/skybox_maskonaive/bottom.jpg"),
        FileSystem::getPath("resources/textures/skybox_maskonaive/front.jpg"),
        FileSystem::getPath("resources/textures/skybox_maskonaive/back.jpg")
    };

    // mirrow cubemap initialization
    unsigned int cubemapTexture = loadCubemap(faces);
    int cubemapSize = 128;
    unsigned int relativeCubemapTexture = createEmptyCubemap(cubemapSize);
    
    // cube Vao
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    // plane VAO
    unsigned int planeVBO, planeVAO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // roof VAO
    unsigned int roofVBO, roofVAO;
    glGenVertexArrays(1, &roofVAO);
    glGenBuffers(1, &roofVBO);
    glBindVertexArray(roofVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roofVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roofVertices), &roofVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // glass wall VAO
    unsigned int glassVBO, glassVAO;
    glGenVertexArrays(1, &glassVAO);
    glGenBuffers(1, &glassVBO);
    glBindVertexArray(glassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, glassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glassWallVertices), &glassWallVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    // framebuffer configuration
    // -------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    // used for debugging only in mirrowing.
    unsigned int textureColorbuffer;
    // glGenTextures(1, &textureColorbuffer);
    // glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cubemapSize, cubemapSize, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubemapSize, cubemapSize); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
                                                                                                  // now that we actually created the framebuffer and added all 
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ------------------
    // Depth Frame Buffer
    // ------------------
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMap = getEmptyTexture(SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // rotate models
        if (rotationAngle == 360) {
            rotationAngle = 0;
        }
        rotationAngle++;

        // change lighting
        if (turn == 3) {
            turn = 0;
        }

        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ---------------------------------------
        // Configure lighting: color and positions
        // ---------------------------------------
        const float radius = 2.5f;
        glm::vec3 pointLightColors[4];
        getLightColors(pointLightColors);
        glm::vec3 pointLightPos[] = {
            glm::vec3(sin(glfwGetTime()) * radius, 4.0f, cos(glfwGetTime()) * radius),
            glm::vec3(6.8f, 1.0f, -6.8f),
            glm::vec3(0.0f, 1.0, 6.8f),
            glm::vec3(-6.8f, 1.0, -6.8f)
        };

        // input
        // -----
        processInput(window);

        // clear buffer
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // -------------------------------------------------------------
        // SHADOWS: RENDER TO DEPTH BUFFER
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 20.0f;
        
        // notice that ortho is used here instead of perspective.
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(pointLightPos[0], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // render important objects only.
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        drawSceneDepth(simpleDepthShader, planeVAO, pointLightPos[0], ship, nanoSuitModel, sphere_mirrow, table, computer, fountain, glm::radians((float)rotationAngle));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // -------------------------------------------------------------
        // MIRROW: RENDER TO FRAME BUFFER 6 TIMES, 1 FOR EACH FACE
        // --------------------------------------------------------------
        if (activateMirrow) {
            glViewport(0, 0, cubemapSize, cubemapSize);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glEnable(GL_DEPTH_TEST);

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            for (int i = 0; i < 6; i++) {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, relativeCubemapTexture, 0);
                // positionate the camera
                invertedCam.Position = glm::vec3(0.0f, 0.0f, 2.0f);
                // select the correct face direction
                switchCam(&invertedCam, i);
                invertedCam.ProcessMouseMovement(0, 0, false);
                
                // clear buffer
                glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
                glClearDepth(1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // draw scene
                drawScene(ourShader, metal, glassShader, skyboxShader, lampShader, groundShader, skyboxVAO,
                cubeVAO, planeVAO, roofVAO, glassVAO, cubemapTexture, woodTexture, marmolTexture, woodTableTexture, roofTexture, 
                pointLightPos, pointLightColors, invertedCam, ship, nanoSuitModel, 
                sphere_mirrow, table, fountain, computer,
                90, 1, lightSpaceMatrix, depthMap, glm::radians((float)rotationAngle));
            }
        }

        // --------------------------------
        // RENDER IN USER BUFFER
        // --------------------------------

        // reset viewport and clear buffer
        glViewport(0, 0, SCR_WIDTH*2, SCR_HEIGHT*2);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---------------------------------------------
        // DEBUGGING PURPOSES: SHADOWS
        // ---------------------------------------------
        // debugDepthQuad.use();
        // debugDepthQuad.setFloat("near_plane", near_plane);
        // debugDepthQuad.setFloat("far_plane", far_plane);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, depthMap);
        // renderQuad();

        // -------------------------------------------------------------
        // USER RENDER: RENDER FROM THE USER
        // --------------------------------------------------------------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        drawScene(ourShader, metal, glassShader, skyboxShader, lampShader, groundShader, skyboxVAO,
        cubeVAO, planeVAO, roofVAO, glassVAO, cubemapTexture, woodTexture, marmolTexture, woodTableTexture, roofTexture, 
        pointLightPos, pointLightColors, camera, ship, nanoSuitModel, 
        sphere_mirrow, table, fountain, computer,
         camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, lightSpaceMatrix, depthMap, glm::radians((float)rotationAngle));

        // ____________________________________________
        // DRAW TWO SET OF PARTICLES
        // ____________________________________________
        if (!activateMirrow) {
            particleContainer->generateParticles(deltaTime);
            particleContainer->simulateParticles(deltaTime);
            particleContainer->draw(waterTexture, projection, view);

            particleContainer2->generateParticles(deltaTime);
            particleContainer2->simulateParticles(deltaTime);
            particleContainer2->draw(waterTexture, projection, view);
        }

        // ---------------------------------------------------------------------------------
        // USER RENDER: RENDER THE MIRROW SPHERE AND GIVE IT THE DYNAMIC CUBEMAP ENVIRONMENT
        // AS TEXTURE
        // ---------------------------------------------------------------------------------
        glEnable(GL_DEPTH_TEST);
        metal.use();
        glActiveTexture(GL_TEXTURE0);
        if (activateMirrow){
            glBindTexture(GL_TEXTURE_CUBE_MAP, relativeCubemapTexture);
        } else {
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        }
        
        metal.setInt("skybox", 0);
        model = glm::translate(model, glm::vec3(0.0f, 0.3f, 0.0f));
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        metal.setMat4("model", model);
        metal.setMat4("view", view);
        metal.setMat4("projection", projection);
        metal.setVec3("cameraPos", camera.Position);
        sphere_mirrow.Draw(metal);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        // ----------------------------------------------
        // DEBUGGING PURPOSES: FRAMEBUFFER FACING.
        // --------------------------------------------
        // glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // screenShader.use();
        // glBindVertexArray(quadVAO);
        // glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
        // glDrawArrays(GL_TRIANGLES, 0, 6);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // delete buffers after use
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &roofVAO);
    glDeleteBuffers(1, &roofVBO);
    particleContainer->deleteBuffers();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// change the light colors
void getLightColors(glm::vec3 *pointLightColors) {
    // PARTY
    if (turn == 0) {
        pointLightColors[0] = glm::vec3(1.0f, 1.0, 0.0);
        pointLightColors[1] = glm::vec3(1.0f, 0.6f, 0.0f);
        pointLightColors[2] = glm::vec3(1.0f, 0.0f, 0.0f);
        pointLightColors[3] = glm::vec3(0.2f, 0.2f, 1.0f);
    } else if(turn == 1) {
        // MOON
        pointLightColors[0] = glm::vec3(0.2f, 0.2f, 0.6f);
        pointLightColors[1] = glm::vec3(0.3f, 0.3f, 0.7f);
        pointLightColors[2] = glm::vec3(0.0f, 0.0f, 0.3f);
        pointLightColors[3] = glm::vec3(0.4f, 0.4f, 0.4f);
    } else if(turn == 2) {
        // DARKNESS
        pointLightColors[0] = glm::vec3(0.3f, 0.1f, 0.1f);
        pointLightColors[1] = glm::vec3(0.1f, 0.1f, 0.1f);
        pointLightColors[2] = glm::vec3(0.1f, 0.1f, 0.1f);
        pointLightColors[3] = glm::vec3(0.1f, 0.1f, 0.1f);
    }
}

// Method to transmit the light parameters to the shaders
void setLights(Shader shader, glm::vec3 lightPositions[], glm::vec3 pointLightColors[], glm::mat4 lightSpaceMatrix, unsigned int depthMap, int nrLights) {
    // directional light
    shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);	
    shader.setVec3("dirLight.ambient",  0.0f, 0.0f, 0.0f);
    shader.setVec3("dirLight.diffuse", 0.05f, 0.05f, 0.05);
    shader.setVec3("dirLight.specular", 0.2f, 0.2f, 0.2f);

    // point lights
    for(int i = 0; i < nrLights; i++) {
        shader.setVec3("pointLights["+std::to_string(i)+"].position", lightPositions[i].x, lightPositions[i].y, lightPositions[i].z);
        shader.setVec3("pointLights["+std::to_string(i)+"].ambient", pointLightColors[i].x * 0.1, pointLightColors[i].y * 0.1, pointLightColors[i].z * 0.1);
        shader.setVec3("pointLights["+std::to_string(i)+"].diffuse", pointLightColors[i].x, pointLightColors[i].y, pointLightColors[i].z);
        shader.setVec3("pointLights["+std::to_string(i)+"].specular", pointLightColors[i].x, pointLightColors[i].y, pointLightColors[i].z);
        shader.setFloat("pointLights["+std::to_string(i)+"].constant", 1.0f);
        shader.setFloat("pointLights["+std::to_string(i)+"].linear", 0.09f);
        shader.setFloat("pointLights["+std::to_string(i)+"].quadratic", 0.032f);
    }

    shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.setFloat("material.shininess", 32.0f);

    // depthMap: shadows
    shader.setInt("shadowMap", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthMap);
}

// Draw principal scene
void drawScene(Shader ourShader, Shader metal, Shader glassShader, Shader skyboxShader, Shader lampShader, Shader groundShader, unsigned int skyboxVAO,
 unsigned int cubeVAO, unsigned int planeVAO, unsigned int roofVAO, unsigned int glassWallsVAO, unsigned int cubemapTexture, unsigned int woodTexture, unsigned int marmolTexture, unsigned int woodTableTexture,
 unsigned int roofTexture, glm::vec3 lightPos[], glm::vec3 lightColor[], Camera camera, Model ship, Model nanoSuitModel,
 Model sphere_mirrow, Model table, Model fountain, Model computer, float fov, float aspectRatio, glm::mat4 lightSpaceMatrix, unsigned int depthMap, float rotationAngle) {
        
        ourShader.use();
        ourShader.setVec3("viewPos", camera.Position);
        setLights(ourShader, lightPos, lightColor, lightSpaceMatrix, depthMap, 4);

        // // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render ship
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.7f, 4.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, rotationAngle, glm::vec3(0.0, 1.0, 0.0));
        ourShader.setMat4("model", model);
        ship.Draw(ourShader);

        //render nanosut
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, -4.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, rotationAngle, glm::vec3(0.0, 1.0, 0.0));
        ourShader.setMat4("model", model);
        nanoSuitModel.Draw(ourShader);

        //render table
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.5f, -2.0f, 4.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 0.0, 1.0));
        ourShader.setMat4("model", model);
        ourShader.setInt("texture_diffuse1", 0);
        ourShader.setInt("texture_specular1", 0);
        ourShader.setInt("shadowMap", 2);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTableTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        table.Draw(ourShader);

        //render computer
        if (!activateMirrow) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-4.5f, -0.8f, 4.5f)); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
            model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0.0, 1.0, 0.0));
            ourShader.setMat4("model", model);
            ourShader.setInt("texture_diffuse1", 0);
            ourShader.setInt("texture_specular1", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, marmolTexture);
            computer.Draw(ourShader);
        }

        //fountain 1
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.5f, -2.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
        ourShader.setMat4("model", model);
        ourShader.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTableTexture);
        fountain.Draw(ourShader);

        //fountain 2
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.5f, -2.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
        ourShader.setMat4("model", model);
        ourShader.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, marmolTexture);
        fountain.Draw(ourShader);

        // floor
        ourShader.use();
        ourShader.setInt("texture_diffuse1", 0);
        ourShader.setInt("texture_specular1", 0);
        ourShader.setInt("shadowMap", 1);
        // material properties
        ourShader.setFloat("material.shininess", 128.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
        ourShader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        //glass walls
        glassShader.use();
        glassShader.setMat4("projection", projection);
        glassShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
        glassShader.setMat4("model", model);
        glassShader.setInt("skybox", 0);
        glassShader.setVec3("cameraPos", camera.Position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glBindVertexArray(glassWallsVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // roof
        ourShader.use();
        ourShader.setInt("texture_diffuse1", 0);
        // material properties
        ourShader.setFloat("material.shininess", 128.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
        ourShader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, roofTexture);
        glBindVertexArray(roofVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // also draw the lamp objects
        if (!activateMirrow) {
            lampShader.use();
            lampShader.setMat4("projection", projection);
            lampShader.setMat4("view", view);
            for (int i = 0; i < 4; i++) {
                lampShader.setVec4("color", lightColor[i].x, lightColor[i].y, lightColor[i].z, 1.0f);
                model = glm::mat4(1.0f);
                model = glm::translate(model, lightPos[i]);
                model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube
                lampShader.setMat4("model", model);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
        }

        // draw skybox as last
        if (!activateMirrow) {
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            skyboxShader.use();
            view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
            skyboxShader.setMat4("view", view);
            skyboxShader.setMat4("projection", projection);
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS);
        }
}

// Draw scene for getting shadows
void drawSceneDepth(Shader shader, unsigned int planeVAO, glm::vec3 lightPos, Model ship, Model nanoSuitModel,
 Model sphere_mirrow, Model table, Model computer, Model fountain, float rotationAngle) {
 // don't forget to enable shader before setting uniforms
        shader.use();
        // render ship
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.7f, 4.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));	// it's a bit too big for our scene, so scale it down
        shader.setMat4("model", model);
        ship.Draw(shader);

        //render nanosut
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, -4.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", model);
        nanoSuitModel.Draw(shader);

        // floor
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
        shader.setMat4("model", model);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        //render computer
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.5f, -0.8f, 4.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0.0, 1.0, 0.0));
        shader.setMat4("model", model);
        computer.Draw(shader);

         //render table
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.5f, -2.0f, 4.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 0.0, 1.0));
        shader.setMat4("model", model);
        table.Draw(shader);

        //fountain 1
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.5f, -2.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
        shader.setMat4("model", model);
        fountain.Draw(shader);

        //fountain 2
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.5f, -2.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
        shader.setMat4("model", model);
        fountain.Draw(shader);

        // sphere
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.3f, 0.0f));
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        shader.setMat4("model", model);
        sphere_mirrow.Draw(shader);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        turn++;

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        activateMirrow = !activateMirrow;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// This method is used mainly to reserve space in memory for a texture
unsigned int getEmptyTexture(unsigned int width, unsigned int height) {
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return depthMap;
}

// Reserve space in memory for a cubemap
unsigned int createEmptyCubemap(int size) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}

// switch camera for collecting the 6 faces of the cubemap
void switchCam(Camera* cam, int i) {
    //RIGHT
    if (i == 0) {
        cam->Pitch = 0;
        cam->Yaw = 0;
    // LEFT
    } else if (i == 1) {
        cam->Pitch = 0;
        cam->Yaw = 180;
    //TOP
    } else if (i == 2) {
        cam->Pitch = 90;
        cam->Yaw = -90;
    //BOTTOM
    } else if (i == 3) {
        cam->Pitch = -90;
        cam->Yaw = -90;
    //FRONT
    } else if (i == 4) {
        cam->Pitch = 0;
        cam->Yaw = 90;
    //BACK
    } else if (i == 5) {
        cam->Pitch = 0;
        cam->Yaw = -90;
    }
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return textureID;
}

// checks the status of the currently bound frame buffer object
void checkFBOStatus() {
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	switch (status) {
	case GL_FRAMEBUFFER_UNDEFINED: {
		fprintf(stderr,"FBO: undefined.\n");
		break;
	}
	case GL_FRAMEBUFFER_COMPLETE: {
		fprintf(stderr,"FBO: complete.\n");
		break;
	}
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: {
		fprintf(stderr,"FBO: incomplete attachment.\n");
		break;
	}
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: {
		fprintf(stderr,"FBO: no buffers are attached to the FBO.\n");
		break;
	}
	case GL_FRAMEBUFFER_UNSUPPORTED: {
		fprintf(stderr,"FBO: combination of internal buffer formats is not supported.\n");
		break;
	}
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: {
		fprintf(stderr,"FBO: number of samples or the value for ... does not match.\n");
		break;
	}
	}
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
