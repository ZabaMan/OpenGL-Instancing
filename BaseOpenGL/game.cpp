#include "game.h"
#include "resource_manager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/compatibility.hpp>

#include "camera.h"
#include "game_object.h"
#include <queue>
#include <random>
#include <iomanip>

#include <stdlib.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include <string>

unsigned int grassAmount = 1000000;
unsigned int rockAmount = 1000;
glm::vec3* rockPos = new glm::vec3[rockAmount];
vector<glm::mat4> rock_lod0;
vector<glm::mat4> rock_lod1;
unsigned int treeAmount = 1000;

Camera camera(glm::vec3(0.0f, 5.0f, 20.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90, 0);

GameObject* Terrain;
Model Grass;
Model Tree;
Model Rock;
Model tRock;
unsigned int rockVBO;
unsigned int tRockVBO;
int grassTexture;
int rockTexture;
int barkTexture;
int leafTexture;
int noiseTexture;
int groundTexture;

vector<vector<int>> noise;

bool menu = false;

string prefix = "C:/Users/Aaron/source/repos/Instancing/BaseOpenGL/resources/";

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_MENU), Keys(), Width(width), Height(height), LightPos(1.2f, 20.0f, 2.0f)
{

}

Game::~Game()
{

}

void Game::Init()
{

    // build and compile shaders
    // -------------------------
    ResourceManager::LoadShader("grass_Instanced.vert", "grass_Instanced.frag", nullptr, "Grass");
    ResourceManager::LoadShader("rock_Instanced.vert", "rock_Instanced.frag", nullptr, "Rock");
    ResourceManager::LoadShader("bark_Instanced.vert", "bark_Instanced.frag", nullptr, "Bark");
    ResourceManager::LoadShader("leaf_Instanced.vert", "leaf_Instanced.frag", nullptr, "Leaf");
    ResourceManager::LoadShader("terrainTexture.vert", "terrainTexture.frag", nullptr, "Terrain");
    Shader shader = ResourceManager::GetShader("Terrain");
    shader.Use();
    shader.SetInteger("heightmap", 0);
    shader.SetInteger("texture", 1);

    //Models
    Terrain = new GameObject(glm::vec3(125.0f, -22.5f, 125.0f), glm::vec3(250.0f, 250.0f, 250.0f),
        Model("Plane.obj"), glm::vec3(1.0f, 1.0f, 1.0f));
    Terrain->Rotation.y = -90.0f;
    noiseTexture = loadTexture((prefix + "noise256.png").c_str(), false);
    groundTexture = loadTexture((prefix + "ground.jpg").c_str(), true);

    Grass = Model("PlaneVertical.obj");
    grassTexture = loadTexture((prefix + "grass2.png").c_str(), false);
    Rock = Model("rock/rock.OBJ");
    tRock = Model("rock/cube.obj");
    rockTexture = loadTexture((prefix + "rock/rock.png").c_str(), true);
    stbi_set_flip_vertically_on_load(false);
    Tree = Model("tree/tree3.obj");
    barkTexture = loadTexture((prefix + "tree/texture_laubbaum.png").c_str(), true);
    leafTexture = loadTexture((prefix + "tree/SampleLeaves_2.tga").c_str(), true);


    LoadNoise((prefix + "noise256.txt").c_str());
    InitializeInstances(grassAmount, Grass, 0.5f, 3.0f, 1);
    InitializeInstances(rockAmount, Rock, 1.0f, 2.0f, 23, false, 1);
    InitializeInstances(rockAmount, tRock, 1.0f, 2.0f, 23, false, 2);
    InitializeInstances(treeAmount, Tree, 1.0f, 2.0f, 89, true);

    cout << Tree.meshes.size() << endl;
}

glm::mat4* rmodelMatrices;

void Game::InitializeInstances(int amount, Model model, float scaleMin, float scaleMax, int randomizer, bool tree, int rock)
{
    glm::mat4* modelMatrices;
    if (rock != 2) {
        modelMatrices = new glm::mat4[amount];
        srand(glfwGetTime() * randomizer); // initialize random seed	
        float size = 256.0;
        int freq = 7;
        for (unsigned int i = 0; i < amount; i++)
        {

            glm::mat4 model = glm::mat4(1.0f);
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            float x = RandomFloat(0, 250);
            float z = RandomFloat(0, 250);
            float y = (float)(noise[x][z] - 150) / freq;
            model = glm::translate(model, glm::vec3(x, y, z));
            if(rock == 1)
            {
                rockPos[i] = glm::vec3(x, y, z);
            }

            // 2. scale: Scale between 0.05 and 0.25f
            float scale = RandomFloat(scaleMin, scaleMax);
            model = glm::scale(model, glm::vec3(scale));

            // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
            if (tree)
            {
                //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                float rotAngle = (rand() % 360);
                model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            else {
                model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                float rotAngle = (rand() % 360);
                model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.0f, 0.0f, 1.0f));
            }
            //model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 1.0f, 0.0f));

            // 4. now add to list of matrices
            modelMatrices[i] = model;
        }
    }
    else
    {
        for(int i = 0; i < rockAmount; i++)
        {
            float distance = glm::distance(camera.Position, rockPos[i]);
            distance = glm::pow(distance, 2);
            if(distance > 1000)
            {
                rock_lod1.push_back(rmodelMatrices[i]);
            }
        }
        modelMatrices = rock_lod1.data();
        amount = rock_lod1.size();
    }

    if (rock == 1) 
    {
        rmodelMatrices = modelMatrices;
        for (int i = 0; i < rockAmount; i++)
        {
            float distance = glm::distance(camera.Position, rockPos[i]);
            distance = glm::pow(distance, 2);
            if (distance < 1000)
            {
                rock_lod0.push_back(rmodelMatrices[i]);
            }
        }
        modelMatrices = rock_lod0.data();
        amount = rock_lod0.size();
    }

    // configure instanced array
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    if(rock > 0)
		glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_DRAW);
    else
		glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    if (rock == 1)
        rockVBO = buffer;
    else if (rock == 2)
        tRockVBO = buffer;

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
    // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
    // -----------------------------------------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < model.meshes.size(); i++)
    {
        unsigned int VAO = model.meshes[i].VAO;
        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }
}


float t_terrainHeight = -20.1f;
float t_terrainNoiseFreq = 8.1;

void Game::Update(float dt)
{
    {
        ImGui::Begin("Settings");

        float lightPos[3] = { LightPos.x, LightPos.y, LightPos.z };
        ImGui::SliderFloat("Terrain Height", &t_terrainHeight, -0.0f, -50.0f);
        ImGui::SliderFloat("Terrain Noise Freq", &t_terrainNoiseFreq, 3.0f, 20.0f);

        if (ImGui::CollapsingHeader("Light"))
        {
            ImGui::SliderFloat3("Position", lightPos, -20.0f, 20.0f);
            LightPos.x = lightPos[0];
            LightPos.y = lightPos[1];
            LightPos.z = lightPos[2];
        }


        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();
    }

    Terrain->Position.y = t_terrainHeight;

    
}

vector<glm::mat4> Game::CalculateLOD(Model model, bool greaterThan)
{
    vector<glm::mat4> lod;
    if (greaterThan) {
        for (int i = 0; i < rockAmount; i++)
        {
            float distance = glm::distance(camera.Position, rockPos[i]);
            distance = glm::pow(distance, 2);
            if (distance > 1000)
            {
                lod.push_back(rmodelMatrices[i]);
            }
        }
    }
    else
    {
        for (int i = 0; i < rockAmount; i++)
        {
            float distance = glm::distance(camera.Position, rockPos[i]);
            distance = glm::pow(distance, 2);
            if (distance < 1000)
            {
                lod.push_back(rmodelMatrices[i]);
            }
        }
    }
    glm::mat4* modelMatrices = lod.data();
    int amount = lod.size();

    // configure instanced array
    // -------------------------
    if (greaterThan) {
        glBindBuffer(GL_ARRAY_BUFFER, tRockVBO);
        glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_DRAW);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, rockVBO);
        glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_DRAW);
    }

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
    // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
    // -----------------------------------------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < model.meshes.size(); i++)
    {
        unsigned int VAO = model.meshes[i].VAO;
        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

    return lod;
}

bool keyDown = false;

void Game::ProcessInput(float dt, GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    bool fast = false;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        fast = true;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, dt, fast);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, dt, fast);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, dt, fast);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, dt, fast);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        rock_lod0 = CalculateLOD(Rock, false);
        rock_lod1 = CalculateLOD(tRock, true);
    }


    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !keyDown)
    {
        if (!menu)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        keyDown = true;
        menu = !menu;
    }
    else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE && keyDown)
    {
        keyDown = false;
    }
}


void Game::Render()
{
    ResourceManager::GetShader("Terrain").Use();
    Shader shader = ResourceManager::GetShader("Terrain");
    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)this->Width / this->Height, 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.SetMatrix4("projection", projection);
    shader.SetMatrix4("view", view);
    shader.SetVector3f("lightPos", this->LightPos);
    shader.SetVector3f("viewPos", camera.Position);
    shader.SetFloat("noiseFreq", t_terrainNoiseFreq);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, groundTexture);

    Terrain->Draw(shader);

    ResourceManager::GetShader("Grass").Use();
    shader = ResourceManager::GetShader("Grass");
    // view/projection transformations
    shader.SetMatrix4("projection", projection);
    shader.SetMatrix4("view", view);
    shader.SetVector3f("lightPos", this->LightPos);
    shader.SetVector3f("viewPos", camera.Position);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    for (unsigned int i = 0; i < Grass.meshes.size(); i++)
    {
        glBindVertexArray(Grass.meshes[i].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, Grass.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, grassAmount);
        glBindVertexArray(0);
    }
    
    ResourceManager::GetShader("Rock").Use();
    shader = ResourceManager::GetShader("Rock");
    // view/projection transformations
    shader.SetMatrix4("projection", projection);
    shader.SetMatrix4("view", view);
    shader.SetVector3f("lightPos", this->LightPos);
    shader.SetVector3f("viewPos", camera.Position);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rockTexture);

    for (unsigned int i = 0; i < Rock.meshes.size(); i++)
    {
        glBindVertexArray(Rock.meshes[i].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, Rock.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, rock_lod0.size());
        glBindVertexArray(0);
    }

    for (unsigned int i = 0; i < tRock.meshes.size(); i++)
    {
        glBindVertexArray(tRock.meshes[i].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, tRock.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, rock_lod1.size());
        glBindVertexArray(0);
    }

    ResourceManager::GetShader("Leaf").Use();
    shader = ResourceManager::GetShader("Leaf");
    // view/projection transformations
    shader.SetMatrix4("projection", projection);
    shader.SetMatrix4("view", view);
    shader.SetVector3f("lightPos", this->LightPos);
    shader.SetVector3f("viewPos", camera.Position);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, barkTexture);

    for (unsigned int i = 0; i < Tree.meshes.size(); i++)
    {
        glBindVertexArray(Tree.meshes[i].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, Tree.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, treeAmount);
        glBindVertexArray(0);
    }

    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
}

void Game::ProcessMouseMovement(float xoffset, float yoffset)
{
    if (!menu)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

void Game::ProcessMouseScroll(float yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void Game::LoadNoise(char const* path)
{
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();

    string temp;
    int index = 0;
    while (std::getline(buffer, temp, '\n')) {
        string word = "";
        vector<int> tempFloats;
        for (auto x : temp)
        {
            if (x == ' ')
            {
                tempFloats.push_back(std::stoi(word));
                word = "";
            }
            else {
                word = word + x;
            }
            
        }
        tempFloats.push_back(std::stoi(word));
        word = "";
        noise.push_back(tempFloats);
        index++;
    }
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int Game::loadTexture(char const* path, bool repeat)
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 7);
        glGenerateMipmap(GL_TEXTURE_2D);

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
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

float Game::RandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}