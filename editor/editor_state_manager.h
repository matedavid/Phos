#pragma once

enum class EditorState {
    Editing,
    Playing,
};

struct EditorStateManager {
    EditorState state;
};