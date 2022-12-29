 #include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "filesystem.h"
#include"camera.h"
#include"shader.h"
#include <iostream>
#include <iostream>
#include <map>
#include <string>
#define STB_IMAGE_IMPLEMENTATION 
// functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void renderSphere();
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void GetDesktopResolution(float& horizontal, float& vertical);
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);

// constants and variables
float SCR_WIDTH = 900;
float SCR_HEIGHT = 700;
unsigned int VAO, VBO;
int nrRows = 1;
int nrColumns = 1;
float spacing = 1;
float speed = .1f;
GLfloat rotateY = 0.0f;
GLfloat rotateX = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 point = glm::vec3(0.0f, 0.0f, 0.0f);


struct Character 
{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;


int main()
{
    GetDesktopResolution(SCR_WIDTH, SCR_HEIGHT);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Quote of the Day", NULL, NULL);
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    std::string font_name = FileSystem::getPath("resources/fonts/Antonio-Bold.ttf");
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {

        FT_Set_Pixel_Sizes(face, 0, 48);


        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned char c = 0; c < 128; c++)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    // ** Text Rendering **//

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // init shaders
    Shader specShader("specular.vs", "specular.fs");
    Shader shader("text.vs", "text.fs");

    
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));

    shader.use();
   
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glm::vec3 pointLightPositions[] = {
        glm::vec3(2.0f, 1.0f, 2.0f),
        glm::vec3(1.0f, 2.0f, -3.0f),
        glm::vec3(-3.0f, 1.0f, -2.0f),
        glm::vec3(1.0f, 1.0f, 2.0f),
        glm::vec3(-0.0f, 2.0f, -2.0f),
        glm::vec3(2.0f, 2.0f, 3.0f),
        glm::vec3(0.0f, 3.0f, -2.0f),
        glm::vec3(-2.0f, 1.0f, -2.0f),
        glm::vec3(1.0f, 2.0f, -3.0f),
        glm::vec3(2.0f, 1.0f, 2.0f),
        glm::vec3(3.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 1.0f, 2.0f),
        glm::vec3(2.0f, 1.0f, -2.0f),
        glm::vec3(-1.0f, 2.0f, 2.0f),
        glm::vec3(2.0f, 1.0f, 2.0f),
        glm::vec3(3.0f, 3.0f, -5.0f), 
        glm::vec3(0.0f, 2.0f, 2.0f),
        glm::vec3(2.0f, 2.0f, 3.0f),
    };

    //TEXT VBO VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int diffuseeffect = loadTexture("resources/textures/space/color.png");
    unsigned int speculareffect = loadTexture("resources/textures/space/dwarf.png");
    unsigned int sphereTexture = loadTexture("resources/textures/space/10.jpg");
    unsigned int sphere2Texture = loadTexture("resources/textures/space/35.jpg");
    unsigned int sphere4Texture = loadTexture("resources/textures/space/36.jpg");
    unsigned int sphere5Texture = loadTexture("resources/textures/space/blue.jpg");
    unsigned int sphere6Texture = loadTexture("resources/textures/space/glitter.jpg");
    unsigned int sphere3Texture = loadTexture("resources/textures/space/34.jpg");


    std::vector<std::string> faces
    {
        "resources/textures/skybox2/right.png", // right   or rightright
        "resources/textures/skybox2/left.png",//left     or left
        "resources/textures/skybox2/top.png", //top   
        "resources/textures/skybox2/bottom.png", //bottom
        "resources/textures/skybox2/front.png", //front  or right
        "resources/textures/skybox2/back.png",  //back or leftleft
    };
    
    unsigned int cubemapTexture = loadCubemap(faces);
    shader.use();
    shader.setInt("texture1", 0);
    specShader.use();
    specShader.setInt("material.diffuse", 0);
    specShader.setInt("material.specular", 1);

    // render while loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
       
        
        // ** TEXT REND  **//
        processInput(window);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // ** TEXT REND  **//

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        // define world
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
       

     
        // init shader
        specShader.use();
        specShader.setMat4("projection", projection);
        specShader.setMat4("view", view);
        // init shader
        specShader.setVec3("viewPos", camera.Position);
        specShader.setFloat("material.shininess", 32.0f);
        // directional light
        specShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        specShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        specShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        specShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        specShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        specShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        specShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        specShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        specShader.setFloat("pointLights[0].constant", 1.0f);
        specShader.setFloat("pointLights[0].linear", 0.09f);
        specShader.setFloat("pointLights[0].quadratic", 0.032f);
        // point light 2
        specShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        specShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        specShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        specShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        specShader.setFloat("pointLights[1].constant", 1.0f);
        specShader.setFloat("pointLights[1].linear", 0.09f);
        specShader.setFloat("pointLights[1].quadratic", 0.032f);
        // point light 3
        specShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        specShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        specShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        specShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        specShader.setFloat("pointLights[2].constant", 1.0f);
        specShader.setFloat("pointLights[2].linear", 0.09f);
        specShader.setFloat("pointLights[2].quadratic", 0.032f);
        // point light 4
        specShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        specShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        specShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        specShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        specShader.setFloat("pointLights[3].constant", 1.0f);
        specShader.setFloat("pointLights[3].linear", 0.09f);
        specShader.setFloat("pointLights[3].quadratic", 0.032f);
        // spotLight
        specShader.setVec3("spotLight.position", camera.Position);
        specShader.setVec3("spotLight.direction", camera.Front);
        specShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        specShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        specShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        specShader.setFloat("spotLight.constant", 1.0f);
        specShader.setFloat("spotLight.linear", 0.09f);
        specShader.setFloat("spotLight.quadratic", 0.032f);
        specShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        specShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // init textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseeffect);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, speculareffect);

        model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            specShader.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                // on direct lighting.
                specShader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    -2.0f
                ));

                float xx = sin(glfwGetTime() * speed * 0.55f) * 100.0f * 4.0f * 1.3f;
                float zz = cos(glfwGetTime() * speed * 0.55f) * 100.0f * 4.0f * 1.3f;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, sphereTexture);
                model = glm::translate(model, point);
                model = glm::rotate(model, glm::radians(rotateY), glm::vec3(1.0f, -1.0f, -13.0f));
                model = glm::rotate(model, glm::radians(rotateX), glm::vec3(0.0f, 0.0f, -1.0f));
                //  model_earth = glm::translate(model_earth, glm::vec3(xx, 0.0f, zz));
                glm::vec3 xpoint = glm::vec3(xx, 0.0f, zz);
                model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
                model = glm::rotate(model, glm::radians(-33.25f), glm::vec3(0.0f, 1.0f, 0.f));
                model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(-33.25f) * 2.0f, glm::vec3(0.0f, 0.0f, 1.f));
                specShader.setMat4("model", model);
                renderSphere();
            }
        }

        glActiveTexture(GL_TEXTURE0);
        model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            specShader.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                specShader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    -2.0f
                ));
                float xx = sin(glfwGetTime() * speed * 0.55f) * -100.0f * 4.0f * 1.3f;
                float zz = cos(glfwGetTime() * speed * 0.55f) * 100.0f * 4.0f * 1.3f;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, sphere2Texture);
                model = glm::translate(model, point);
                model = glm::rotate(model, glm::radians(rotateY), glm::vec3(1.0f, -1.0f, -13.0f));
                model = glm::rotate(model, glm::radians(rotateX), glm::vec3(0.0f, 0.0f, -1.0f));
                //  model_earth = glm::translate(model_earth, glm::vec3(xx, 0.0f, zz));
                glm::vec3 xpoint = glm::vec3(xx, 0.0f, zz);
                model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(1.0f, 0.0f, 0.f));
                model = glm::rotate(model, glm::radians(33.25f), glm::vec3(0.0f, 1.0f, 0.f));
                model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(-33.25f) * 2.0f, glm::vec3(0.0f, 0.0f, 1.f));
                specShader.setMat4("model", model);
                unsigned int sphereVAO = 0;
                unsigned int indexCount;
                renderSphere();
            }
        }

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sphere3Texture);
        model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            specShader.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                specShader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    -2.0f
                ));

                float xx = sin(glfwGetTime() * speed * 0.55f) * -100.0f * 4.0f * 1.3f;
                float zz = cos(glfwGetTime() * speed * 0.55f) * 100.0f * 4.0f * 1.3f;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, sphere3Texture);
                model = glm::translate(model, point);
                model = glm::rotate(model, glm::radians(rotateY), glm::vec3(1.0f, -1.0f, -13.0f));
                model = glm::rotate(model, glm::radians(rotateX), glm::vec3(0.0f, 0.0f, -1.0f));
                //  model_earth = glm::translate(model_earth, glm::vec3(xx, 0.0f, zz));
                glm::vec3 xpoint = glm::vec3(xx, 0.0f, zz);
                model = glm::rotate(model, glm::radians(-120.0f), glm::vec3(1.0f, 0.0f, 0.f));
                model = glm::rotate(model, glm::radians(-23.25f), glm::vec3(0.0f, 1.0f, 0.f));
                model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(-33.25f) * 2.0f, glm::vec3(0.0f, 0.0f, 1.f));
                specShader.setMat4("model", model);
                unsigned int sphereVAO = 0;
                unsigned int indexCount;
                renderSphere();
            }
        }
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sphere3Texture);
        model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            specShader.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                specShader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    -2.0f
                ));

                float xx = sin(glfwGetTime() * speed * 0.55f) * -100.0f * 4.0f * 1.3f;
                float zz = cos(glfwGetTime() * speed * 0.55f) * 100.0f * 4.0f * 1.3f;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, sphere4Texture);
                model = glm::translate(model, point);
                model = glm::rotate(model, glm::radians(rotateY), glm::vec3(1.0f, -1.0f, -13.0f));
                model = glm::rotate(model, glm::radians(rotateX), glm::vec3(0.0f, 0.0f, -1.0f));
                //  model_earth = glm::translate(model_earth, glm::vec3(xx, 0.0f, zz));
                glm::vec3 xpoint = glm::vec3(xx, 0.0f, zz);
                model = glm::rotate(model, glm::radians(120.0f), glm::vec3(1.0f, 0.0f, 0.f));
                model = glm::rotate(model, glm::radians(60.25f), glm::vec3(0.0f, 1.0f, 0.f));
                model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(-33.25f) * 2.0f, glm::vec3(0.0f, 0.0f, 1.f));
                specShader.setMat4("model", model);
                unsigned int sphereVAO = 0;
                unsigned int indexCount;
                renderSphere();
            }
        }

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sphere3Texture);
        model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            specShader.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                specShader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    -2.0f
                ));

                float xx = sin(glfwGetTime() * speed * 0.55f) * -50.0f * 4.0f * 1.3f;
                float zz = cos(glfwGetTime() * speed * 0.55f) * 100.0f * 4.0f * 1.3f;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, sphere5Texture);
                model = glm::translate(model, point);
                model = glm::rotate(model, glm::radians(rotateY), glm::vec3(1.0f, -1.0f, -13.0f));
                model = glm::rotate(model, glm::radians(rotateX), glm::vec3(0.0f, 0.0f, -1.0f));
                //  model_earth = glm::translate(model_earth, glm::vec3(xx, 0.0f, zz));
                glm::vec3 xpoint = glm::vec3(xx, 0.0f, zz);
                model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(1.0f, 0.0f, 0.f));
                model = glm::rotate(model, glm::radians(80.25f), glm::vec3(0.0f, 1.0f, 0.f));
                model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(-33.25f) * 2.0f, glm::vec3(0.0f, 0.0f, 1.f));
                specShader.setMat4("model", model);
                unsigned int sphereVAO = 0;
                unsigned int indexCount;
                renderSphere();
            }
        }

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sphere3Texture);
        model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            specShader.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                specShader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    -2.0f
                ));

                float xx = sin(glfwGetTime() * speed * 0.55f) * -100.0f * 4.0f * 1.3f;
                float zz = cos(glfwGetTime() * speed * 0.55f) * 50.0f * 4.0f * 1.3f;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, sphere6Texture);
                model = glm::translate(model, point);
                model = glm::rotate(model, glm::radians(rotateY), glm::vec3(1.0f, -1.0f, -13.0f));
                model = glm::rotate(model, glm::radians(rotateX), glm::vec3(0.0f, 0.0f, -1.0f));
                //  model_earth = glm::translate(model_earth, glm::vec3(xx, 0.0f, zz));
                glm::vec3 xpoint = glm::vec3(xx, 0.0f, zz);
                model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.f));
                model = glm::rotate(model, glm::radians(10.25f), glm::vec3(0.0f, 1.0f, 0.f));
                model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(-33.25f) * 2.0f, glm::vec3(0.0f, 0.0f, 1.f));
                specShader.setMat4("model", model);
                unsigned int sphereVAO = 0;
                unsigned int indexCount;
                renderSphere();
            }
        }

        shader.setMat4("model", model);
        model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            for (int col = 0; col < nrColumns; ++col)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    -2.0f
                ));
                float xx = sin(glfwGetTime() * speed * 0.55f) * -100.0f * 4.0f * 1.3f;
                float zz = cos(glfwGetTime() * speed * 0.55f) * 100.0f * 4.0f * 1.3f;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, sphere2Texture);
                model = glm::translate(model, point);
                model = glm::rotate(model, glm::radians(rotateY), glm::vec3(1.0f, -1.0f, -13.0f));
                model = glm::rotate(model, glm::radians(rotateX), glm::vec3(0.0f, 0.0f, -1.0f));
                //  model_earth = glm::translate(model_earth, glm::vec3(xx, 0.0f, zz));
                glm::vec3 xpoint = glm::vec3(xx, 0.0f, zz);
                model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(1.0f, 0.0f, 0.f));
                model = glm::rotate(model, glm::radians(33.25f), glm::vec3(0.0f, 1.0f, 0.f));
                model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(-33.25f) * 2.0f, glm::vec3(0.0f, 0.0f, 1.f));
                shader.setMat4("model", model);
                RenderText(shader, "- Ralph Waldo Emerson", 180.0f, 125.0f, 1.4f, glm::vec3(0.5, 0.8f, 0.2f));
                RenderText(shader, "\"To be yourself in a world that is constantly trying", 720.0f, 950.0f, 1.1f, glm::vec3(0.3, 0.7f, 0.9f));   // vec 1 is position x, y and size,  last vec is color
                RenderText(shader, "to make you something else is the greatest accomplishment.\"", 600.0f, 870.0f, 1.1f, glm::vec3(0.5, 0.4f, 0.9f));
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
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
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

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


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()        
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);
        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;
        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI)*.45 + 0;            
                float yPos = std::cos(ySegment * PI) * .45 + 2.7; 
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI)*.45 + .25;

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;

    for (unsigned int i = 0; i < faces.size(); i++)
    {

        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
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

    return textureID;
}


unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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


void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
       
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale; 
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

}

void GetDesktopResolution(float& horizontal, float& vertical)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}