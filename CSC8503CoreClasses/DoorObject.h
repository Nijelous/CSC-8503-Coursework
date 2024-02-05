#pragma once
#include "GameObject.h"
namespace NCL::CSC8503 {
    class DoorObject : public GameObject {
    public:
        DoorObject(int flag) { button = flag; name = "Door"; }
        ~DoorObject() {}
        void UpdateObject(float dt, GameWorld& world) override;
        bool IsOpen() { return opened; }
        void SetButton(int id) { button = abs(id%4); }
        int GetButton() { return button; }
        void SetHinge(GameObject* hinge) { if (!this->hinge) this->hinge = hinge; }
        GameObject* GetHinge() { return hinge; }
    protected:
        int button;
        bool opened = false;
        GameObject* hinge = nullptr;
    };
}

