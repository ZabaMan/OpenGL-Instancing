#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


#include "model.h"

#include <algorithm>
#include <iostream>
#include <list>

// Container object for holding all state relevant for a single
// game object entity. Each object in the game likely needs the
// minimal of state as described within GameObject.
class GameObject
{
public:
    // object state
    glm::vec3   Position, Size, Rotation;
    glm::vec3   Color;
    bool        HasParent;
    bool        Destroyed;
    GameObject *Parent;
    GameObject* Child;
    bool        HasChild;
    float       XaxisMin, XaxisMax, YaxisMin, YaxisMax, ZaxisMin, ZaxisMax;
    bool        UseConstraints = false;
    // render state
    Model   model;
    // constructor(s)
    GameObject();
    GameObject(glm::vec3 pos, glm::vec3 size, Model model, glm::vec3 color = glm::vec3(1.0f));
    // draw sprite
    virtual void Draw(Shader& shader);
    glm::mat4 GetModelMatrix();
    void MoveBone(glm::mat4 localTransform, string boneName);
    std::vector<glm::mat4> GetBoneTransforms();
    void SetPosition(glm::vec3 pos);
    void SetRotation(glm::vec3 rot);
    Mesh GetMesh();
    void SendBlendshapeToMesh(Mesh &mesh, int i);
    void MeshReset();
    void SetConstraint(string axis, float min, float max);
    void AddChild(GameObject* GO);
    glm::vec3 GetWorldPos();
    glm::vec3 GetPositionInLocalSpace(glm::vec3 pos);
    void DrawChild(glm::mat4 model_, Shader& shader);
};

#endif