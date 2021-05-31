#include "game_object.h"
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>


GameObject::GameObject()
    : Position(0.0f, 0.0f, 0.0f), Size(1.0f, 1.0f, 1.0f), Color(1.0f), Rotation(0.0f, 0.0f, 0.0f), Child(), model(), Parent(), HasParent(false), Destroyed(false), HasChild(false), XaxisMin(-90), XaxisMax(90), YaxisMin(-90), YaxisMax(90), ZaxisMin(-90), ZaxisMax(90) { }

GameObject::GameObject(glm::vec3 pos, glm::vec3 size, Model model, glm::vec3 color)
    : Position(pos), Size(size), Color(color), Rotation(0.0f, 0.0f, 0.0f), Child(), model(model), Parent(), HasParent(false), Destroyed(false), HasChild(false), XaxisMin(-90), XaxisMax(90), YaxisMin(-90), YaxisMax(90), ZaxisMin(-90), ZaxisMax(90) { }

void GameObject::Draw(Shader& shader)
{
    glm::mat4 model_ = glm::mat4(1.0f);
    model_ = glm::translate(model_, Position);
    model_ = glm::rotate(model_, glm::radians(Rotation.y), glm::vec3(0, 1, 0));
    model_ = glm::rotate(model_, glm::radians(Rotation.x), glm::vec3(1, 0, 0));
    model_ = glm::rotate(model_, glm::radians(Rotation.z), glm::vec3(0, 0, 1));
	model_ = glm::scale(model_, Size);
    shader.SetMatrix4("model", model_);
	shader.SetVector3f("objectColor", Color.x, Color.y, Color.z);
    model.Draw(shader);

    if (HasChild)
    {
        glm::mat4 model_child = glm::mat4(1.0f);
        model_child = glm::translate(model_child, Child->Position);
        model_child = glm::rotate(model_child, glm::radians(Child->Rotation.y), glm::vec3(0, 1, 0));
        model_child = glm::rotate(model_child, glm::radians(Child->Rotation.x), glm::vec3(1, 0, 0));
        model_child = glm::rotate(model_child, glm::radians(Child->Rotation.z), glm::vec3(0, 0, 1));
        model_child = glm::scale(model_child, Child->Size);

        Child->DrawChild(model_ * model_child, shader);
    }
}

glm::mat4 GameObject::GetModelMatrix()
{
    glm::mat4 model_ = glm::mat4(1.0f);
    model_ = glm::translate(model_, Position);
    model_ = glm::rotate(model_, glm::radians(Rotation.y), glm::vec3(0, 1, 0));
    model_ = glm::rotate(model_, glm::radians(Rotation.x), glm::vec3(1, 0, 0));
    model_ = glm::rotate(model_, glm::radians(Rotation.z), glm::vec3(0, 0, 1));
    model_ = glm::scale(model_, Size);
    return model_;
}


void GameObject::SetPosition(glm::vec3 pos)
{
    Position = pos;
}

void GameObject::SetRotation(glm::vec3 rot)
{
    Rotation = rot;
}

Mesh GameObject::GetMesh()
{
    return model.meshes[0];
}


void GameObject::SetConstraint(string axis, float min, float max)
{
    if (axis == "X")
    {
        XaxisMin = min;
        XaxisMax = max;
    }
    else if (axis == "Y")
    {
        YaxisMin = min;
        YaxisMax = max;
    }
    else if (axis == "Z")
    {
        ZaxisMin = min;
        ZaxisMax = max;
    }
}

void GameObject::AddChild(GameObject* GO)
{
    Child = GO;
    HasChild = true;
    GO->HasParent = true;
    GO->Parent = this;
}

glm::vec3 GameObject::GetWorldPos()
{
    glm::mat4 model_ = GetModelMatrix();

    glm::vec3 WP = glm::vec3(model_[3].x, model_[3].y, model_[3].z);
    return WP;
}

glm::vec3 GameObject::GetPositionInLocalSpace(glm::vec3 pos)
{
    if (HasParent)
        pos = pos - Parent->Position;

    //model_ = model_ * GetModelMatrix();
    //glm::vec3 LP = glm::vec3(model_[3].x, model_[3].y, model_[3].z);
    return pos;
}



void GameObject::DrawChild(glm::mat4 model_, Shader& shader)
{
    shader.SetMatrix4("model", model_);
    shader.SetVector3f("objectColor", Color.x, Color.y, Color.z);
    model.Draw(shader);

    if (HasChild)
    {
        glm::mat4 model_child = glm::mat4(1.0f);
        model_child = glm::translate(model_child, Child->Position);
        model_child = glm::rotate(model_child, glm::radians(Child->Rotation.y), glm::vec3(0, 1, 0));
        model_child = glm::rotate(model_child, glm::radians(Child->Rotation.x), glm::vec3(1, 0, 0));
        model_child = glm::rotate(model_child, glm::radians(Child->Rotation.z), glm::vec3(0, 0, 1));
    	model_child = glm::scale(model_child, Child->Size);

        Child->DrawChild(model_ * model_child, shader);
    }
}
