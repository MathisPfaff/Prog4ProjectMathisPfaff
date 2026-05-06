#pragma once
#include "Command.h"
#include <glm/glm.hpp>

namespace dae
{
    class GridComponent;

    class MoveCommand final : public GameObjectCommand
    {
    public:
        MoveCommand(GameObject* pGameObject, glm::vec2 direction,
                    GameObject* pGridObject);

        void Execute() override;

    private:
        glm::vec2   m_Direction{};
        GameObject* m_pGridObject{};
    };
}