#pragma once

#include "core.h"

#include <imgui.h>
#include <imgui_internal.h>

class IImGuiPanel {
  public:
    virtual ~IImGuiPanel() = default;

    virtual void on_imgui_render() = 0;

  protected:
    std::string m_name;
};
