#pragma once
#include <memory>

namespace dae
{
    class GameObject;

    class PookaState
    {
    public:
        virtual ~PookaState() = default;
        virtual void OnEnter(GameObject*) {}
        virtual std::unique_ptr<PookaState> Update(GameObject* owner) = 0;
        virtual void OnExit(GameObject*) {}
    };
}