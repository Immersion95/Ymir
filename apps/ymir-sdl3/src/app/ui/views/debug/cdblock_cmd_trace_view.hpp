#pragma once

#include <app/shared_context.hpp>

#include <app/debug/cdblock_tracer.hpp>

namespace app::ui {

class CDBlockCommandTraceView {
public:
    CDBlockCommandTraceView(SharedContext &context);

    void Display();

private:
    SharedContext &m_context;
    CDBlockTracer &m_tracer;
};

} // namespace app::ui
