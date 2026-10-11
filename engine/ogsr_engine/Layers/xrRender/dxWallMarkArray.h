#pragma once

#include "../../Include/xrRender/WallMarkArray.h"

class dxWallMarkArray : public IWallMarkArray
{
public:
    virtual ~dxWallMarkArray();
    virtual void Copy(IWallMarkArray& _in);

    virtual void AppendMark(LPCSTR s_textures);
    virtual void clear();
    virtual bool empty();
    virtual wm_shader GenerateWallmark();

    // NLC: soft = the same texture with the shader for surfaces that soak blood in (blood marks only)
    ref_shader* dxGenerateWallmark(bool soft = false);
    bool has_soft() const { return !m_CollideMarksSoft.empty(); }

private:
    DEFINE_VECTOR(ref_shader, ShaderVec, ShaderIt);

    ShaderVec m_CollideMarks;
    ShaderVec m_CollideMarksSoft; // NLC: parallel to m_CollideMarks, empty for non-blood arrays
};
