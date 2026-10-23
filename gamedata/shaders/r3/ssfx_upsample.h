/**
 * @ Description: Depth-aware upsample of reduced-size SSS 23 results ( AO, IL, SSR )
 * @ NLC: 2026-10-13 ( combine_1.ps ), shared 2026-10-24: bilinear across depth edges left dark or bright halos
 *        around thin objects. 2x2 low-res taps, bilinear weights times depth similarity. Needs s_position.
 */

#ifndef SSFX_UPSAMPLE_H
#define SSFX_UPSAMPLE_H

float4 ssfx_upsample(Texture2D t, float2 tc, float z0)
{
    float2 size;
    t.GetDimensions(size.x, size.y);
    float2 f = tc * size - 0.5f;
    float2 base = floor(f);
    float2 fr = f - base;

    float4 sum = 0;
    float wsum = 0;
    [unroll] for (int i = 0; i < 4; i++)
    {
        float2 o = float2(i & 1, i >> 1);
        float2 uv = (base + o + 0.5f) / size;
        float z = s_position.SampleLevel(smp_nofilter, uv, 0).z;
        float wb = (o.x ? fr.x : 1.0f - fr.x) * (o.y ? fr.y : 1.0f - fr.y);
        float w = wb * exp2(-abs(1.0f - z / max(z0, 0.0001f)) * 16.0f) + 0.0001f * wb;
        sum += t.SampleLevel(smp_nofilter, uv, 0) * w;
        wsum += w;
    }
    return sum / max(wsum, 0.00001f);
}

#endif // SSFX_UPSAMPLE_H
