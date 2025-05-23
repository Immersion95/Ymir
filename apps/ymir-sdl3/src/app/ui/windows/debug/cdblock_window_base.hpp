#pragma once

#include <app/ui/window_base.hpp>

#include <app/debug/cdblock_tracer.hpp>

namespace app::ui {

class CDBlockWindowBase : public WindowBase {
public:
    CDBlockWindowBase(SharedContext &context);
    virtual ~CDBlockWindowBase() = default;

protected:
    ymir::cdblock::CDBlock &m_cdblock;
    CDBlockTracer &m_tracer;
};

} // namespace app::ui
