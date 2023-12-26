#pragma once

#include <functional>

enum class EditorState {
    Editing,
    Playing,
};

class EditorStateManager {
  public:
    EditorStateManager() = delete;

    [[nodiscard]] static EditorState get_state() { return m_state; }

    // First updates state, then notifies subscribers
    static void set_state(EditorState state) {
        auto prev = m_state;
        m_state = state;

        for (const auto& func : m_state_changed_callback)
            func(prev, state);
    }

    static void subscribe(const std::function<void(EditorState, EditorState)>& func) {
        m_state_changed_callback.push_back(func);
    }

  private:
    static EditorState m_state;

    static std::vector<std::function<void(EditorState, EditorState)>> m_state_changed_callback;
};

inline EditorState EditorStateManager::m_state = EditorState::Editing;
inline std::vector<std::function<void(EditorState, EditorState)>> EditorStateManager::m_state_changed_callback{};