#pragma once
#include <unordered_map>
#include <vector>
#include <string>

namespace DungeonGame {

    enum class AnimationState {
        IdlePassive,
        IdleCombat,
        Attack,
        Hit,
        Interact,
        Opening
    };

    struct AnimationClip {
        std::vector<std::string> frames;
        bool loop = true;
        float fps = 12.f;
    };

    class Animator {
    public:
        
        void addClip(AnimationState state, AnimationClip clip);
        void setState(AnimationState state);
        void setReturnState(AnimationState state);
        void update(float dt);

        const std::string& getCurrentFrame() const;
        AnimationState getState() const { return m_currentState; }
        bool isComplete() const { return m_complete; }

    private:
        std::unordered_map<AnimationState, AnimationClip> m_clips;

        AnimationState m_currentState = AnimationState::IdlePassive;
        AnimationState m_returnState = AnimationState::IdlePassive;

        int   m_frameIndex = 0;
        float m_elapsed = 0.f;
        bool  m_complete = false;

        static const std::string k_emptyFrame;
    };

}