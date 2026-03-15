#include "Merchant.h"
#include "ItemDatabase.h"
#include <random>

namespace DungeonGame {

    static std::mt19937 rng{ std::random_device{}() };

    static std::vector<std::string> makeFrames(const std::string& path, int count) {
        std::vector<std::string> frames;
        for (int i = 1; i <= count; ++i)
            frames.push_back(path + std::to_string(i) + ".png");
        return frames;
    }

    Merchant::Merchant(int x, int y, int floor)
        : Entity(x, y, '~', EntityType::Merchant, "Merchant") {
        generateStock(floor);

        m_animator.addClip(AnimationState::IdlePassive, {
            makeFrames("assets/animations/merchant/merchant_idle_", 12), true, 12.f });
        m_animator.addClip(AnimationState::Interact, {
            makeFrames("assets/animations/merchant/merchant_interact_", 12), false, 12.f }); //for interact - not implemented
        m_animator.setState(AnimationState::IdlePassive);
        m_animator.setReturnState(AnimationState::IdlePassive);
    }

    void Merchant::generateStock(int floor) {
        m_stock.clear();
        int count = std::uniform_int_distribution<int>(6, 12)(rng);
        for (int i = 0; i < count; ++i)
            m_stock.push_back(ItemDatabase::get().randomItem(floor));
    }

    void Merchant::removeStockItem(int index) {
        if (index < 0 || index >= (int)m_stock.size()) return;
        m_stock.erase(m_stock.begin() + index);
        if (m_selectedIndex >= (int)m_stock.size() && m_selectedIndex > 0)
            --m_selectedIndex;
    }

}