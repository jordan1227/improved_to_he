#include "stdafx.h"
#include "dxWallMarkArray.h"

#include "dxUIShader.h"

void dxWallMarkArray::Copy(IWallMarkArray& _in) { *this = *smart_cast<dxWallMarkArray*>(&_in); }

dxWallMarkArray::~dxWallMarkArray()
{
    for (auto& m_CollideMark : m_CollideMarks)
        m_CollideMark.destroy();
    for (auto& m_CollideMark : m_CollideMarksSoft)
        m_CollideMark.destroy();
}

void dxWallMarkArray::AppendMark(LPCSTR s_textures)
{
    ref_shader s;
    s = GetCachedShader(strstr(s_textures, "wm_blood_") ? "effects\\wallmark_blood" : "effects\\wallmark", s_textures);
    m_CollideMarks.push_back(s);

    // NLC: blood marks also get the variant for soft ground (multiplied into the ground, no gloss)
    if (strstr(s_textures, "wm_blood_"))
        m_CollideMarksSoft.push_back(GetCachedShader("effects\\wallmark_blood_soft", s_textures));
}

void dxWallMarkArray::clear()
{
    m_CollideMarksSoft.clear();
    return m_CollideMarks.clear();
}

bool dxWallMarkArray::empty() { return m_CollideMarks.empty(); }

wm_shader dxWallMarkArray::GenerateWallmark()
{
    wm_shader res;
    if (!m_CollideMarks.empty())
        smart_cast<dxUIShader*>(&*res)->hShader = m_CollideMarks[::Random.randI(0, m_CollideMarks.size())];
    return res;
}

ref_shader* dxWallMarkArray::dxGenerateWallmark(bool soft)
{
    if (m_CollideMarks.empty())
        return nullptr;
    const size_t i = ::Random.randI(0, m_CollideMarks.size());
    if (soft && i < m_CollideMarksSoft.size())
        return &m_CollideMarksSoft[i];
    return &m_CollideMarks[i];
}