#pragma once

class CActor;

class CActorInputHandler
{
public:
    virtual void reinit();

    virtual void install();
    virtual void install(CActor*);
    virtual void release();

    virtual bool authorized(int cmd) { return true; }
    virtual float mouse_scale_factor() { return 1.f; }
    // NLC: raw mouse counts while a handler is installed (bloodsucker grab struggle)
    virtual void on_mouse_move(int dx, int dy) {}

protected:
    CActor* m_actor{};
};
