#pragma once

namespace dae
{
    class GameObject;

    class PookaState
    {
    public:
        virtual ~PookaState() = default;
        virtual void OnEnter(GameObject*) {}
        virtual void Update(GameObject* owner) = 0;
        virtual void OnExit(GameObject*) {}
    };
}