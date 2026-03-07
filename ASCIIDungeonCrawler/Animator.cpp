#include "Animator.h"

namespace DungeonGame {

    const std::string Animator::k_emptyFrame = "";

    void Animator::addClip(AnimationState state, AnimationClip clip) {
        m_clips[state] = std::move(clip);
    }

    void Animator::setState(AnimationState state) {
        // quietly ignore states with no registered clip
        if (m_clips.find(state) == m_clips.end()) return;

        // don't interrupt a non-looping animation mid-play
        if (!m_complete) {
            auto it = m_clips.find(m_currentState);
            if (it != m_clips.end() && !it->second.loop)
                return;
        }

        if (state == m_currentState && !m_complete) return;

        m_currentState = state;
        m_frameIndex = 0;
        m_elapsed = 0.f;
        m_complete = false;
    }

    void Animator::update(float dt) {
        auto it = m_clips.find(m_currentState);
        if (it == m_clips.end()) return;

        const AnimationClip& clip = it->second;
        if (clip.frames.empty()) return;

        m_elapsed += dt;
        float frameDuration = 1.f / clip.fps;

        if (m_elapsed >= frameDuration) {
            m_elapsed -= frameDuration;
            m_frameIndex++;

            if (m_frameIndex >= (int)clip.frames.size()) {
                if (clip.loop) {
                    m_frameIndex = 0;
                }
                else {
                    // hold last frame, mark complete, auto-return to idle
                    m_frameIndex = (int)clip.frames.size() - 1;
                    m_complete = true;
                    m_currentState = m_returnState;
                    m_frameIndex = 0;
                    m_elapsed = 0.f;
                }
            }
        }
    }

    const std::string& Animator::getCurrentFrame() const {
        auto it = m_clips.find(m_currentState);
        if (it == m_clips.end() || it->second.frames.empty())
            return k_emptyFrame;
        return it->second.frames[m_frameIndex];
    }

    void Animator::setReturnState(AnimationState state) {
        if (m_clips.find(state) == m_clips.end()) return;
        m_returnState = state;
    }
}