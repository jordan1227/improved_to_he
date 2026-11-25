#pragma once

class CEffect_Rain;
class CBackend;
// struct Fsphere;

#include "../../xrCore/_sphere.h"

class IRainRender
{
public:
    virtual ~IRainRender() { ; }
    virtual void Copy(IRainRender& _in) = 0;

    virtual void Render(CBackend& cmd_list, CEffect_Rain& owner, bool scope) = 0; // NLC: scope = draw only, no particle step
    virtual void Calculate(CEffect_Rain& owner) = 0;

    virtual const Fsphere& GetDropBounds() const = 0;
};
