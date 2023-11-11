#pragma once

enum class EditorState {
    Editing,
    Playing,
};

class EditorStateManager {
  public:
    [[nodiscard]] EditorState get_state() const { return m_state; }

    void set_state(EditorState state) {
        for (const auto& func : m_state_changed_callback)
            func(state);

        m_state = state;
    }

    void subscribe(const std::function<void(EditorState)>& func) { m_state_changed_callback.push_back(func); }

  private:
    EditorState m_state;

    std::vector<std::function<void(EditorState)>> m_state_changed_callback;
};