#include "GameObject.h"
#include "Renderer.h"

namespace dae
{
    GameObject::~GameObject()
    {
        if (m_pParent)
        {
            m_pParent->RemoveChild(this);
        }

        for (auto* child : m_pChildren)
        {
            child->m_pParent = nullptr;
        }

        m_pComponents.clear();
        m_pDeleteComponents.clear();
    }

    void GameObject::FixedUpdate(float fixed_time_step)
    {
        for (const auto& component : m_pComponents)
        {
            component->FixedUpdate(fixed_time_step);
        }
    }

    void GameObject::Update()
    {
        for (const auto& component : m_pComponents)
        {
            component->Update();
        }
    }

    void GameObject::LateUpdate()
    {
        for (const auto& deleted : m_pDeleteComponents)
        {
            auto it = std::remove_if(m_pComponents.begin(), m_pComponents.end(),
                [&](const std::unique_ptr<BaseComponent>& comp) {
                    return comp.get() == deleted.get();
                });
            m_pComponents.erase(it, m_pComponents.end());
        }
        m_pDeleteComponents.clear();

        for (const auto& component : m_pComponents)
        {
            component->LateUpdate();
        }
    }

    void GameObject::Render() const
    {
        for (const auto& component : m_pComponents)
        {
            component->Render();
        }
    }

    void GameObject::SetLocalPosition(float x, float y)
    {
        m_transform.SetPosition(x, y, 0.0f);
        SetPositionDirty();
    }

    void GameObject::SetLocalPosition(const glm::vec3& position)
    {
        m_transform.SetPosition(position);
        SetPositionDirty();
    }

    const glm::vec3& GameObject::GetWorldPosition()
    {
        if (m_PositionIsDirty)
        {
            if (m_pParent)
            {
                m_worldPosition = m_pParent->GetWorldPosition() + m_transform.GetPosition();
            }
            else
            {
                m_worldPosition = m_transform.GetPosition();
            }

            m_PositionIsDirty = false;
        }
        return m_worldPosition;
    }

    void GameObject::SetPositionDirty()
    {
        m_PositionIsDirty = true;
        for (auto* child : m_pChildren)
        {
            child->SetPositionDirty();
        }
    }

    void GameObject::SetParent(GameObject* parent, bool keepWorldPosition)
    {
        if (parent == this || m_pParent == parent || IsChild(parent))
        {
            return;
        }

        if (parent == nullptr)
        {
            SetLocalPosition(GetWorldPosition());
        }
        else if (keepWorldPosition)
        {
            SetLocalPosition(GetWorldPosition() - parent->GetWorldPosition());
        }

        SetPositionDirty();

        if (m_pParent)
        {
            m_pParent->RemoveChild(this);
        }

        m_pParent = parent;

        if (m_pParent)
        {
            m_pParent->AddChild(this);
        }
    }

    GameObject* GameObject::GetChild(int index) const
    {
        if (index < 0 || index >= static_cast<int>(m_pChildren.size()))
        {
            return nullptr;
        }
        return m_pChildren[index];
    }

    void GameObject::AddChild(GameObject* child)
    {
        m_pChildren.emplace_back(child);
    }

    void GameObject::RemoveChild(GameObject* child)
    {
        auto it = std::remove(m_pChildren.begin(), m_pChildren.end(), child);
        if (it != m_pChildren.end())
        {
            m_pChildren.erase(it, m_pChildren.end());
            if (child->m_pParent == this)
            {
				child->m_pParent = nullptr;
            }
        }
    }

    bool GameObject::IsChild(GameObject* child) const
    {
        for (auto* c : m_pChildren)
        {
            if (c == child) 
            { 
                return true; 
            }
            if (c->IsChild(child)) 
            {
                return true;
            }
        }
        return false;
    }

    void GameObject::RemoveComponent(BaseComponent* pComponent)
    {
        auto it = std::find_if(m_pComponents.begin(), m_pComponents.end(),
            [pComponent](const std::unique_ptr<BaseComponent>& comp) {
                return comp.get() == pComponent;
            });

        if (it != m_pComponents.end())
        {
            m_pDeleteComponents.push_back(std::move(*it));
        }
    }
}