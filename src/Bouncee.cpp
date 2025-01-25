#include "Bouncee.h"
#include "math.h"

float PI = 3.14159265359f;

float Bouncee::linear(float t)
{
    return t;
}

float Bouncee::spike_linear(float t)
{
    t = t <= 0.5f ? 2 * linear(t) : 2 * linear((1 - t));
    return t;
}

float Bouncee::in_sine(float t)
{
    t = 1 - cos(t * 0.5f);
    return t;
}

float Bouncee::out_sine(float t)
{
    t = invert(in_sine(invert(t)));
    return t;
}

float Bouncee::in_out_sine(float t)
{
    t = in_sine(2 * t) / 2;
    return t;
}

float Bouncee::spike_sine(float t)
{
    t = t <= 0.5f ? in_sine(2 * t) : 2 - in_sine(2 * t);
    return t;
}

float Bouncee::in_quad(float t)
{
    t = t * t;
    return t;
}

float Bouncee::out_quad(float t)
{
    t = invert(in_quad(invert(t)));
    return t;
}

float Bouncee::in_out_quad(float t)
{
    t = t <= 0.5f ? 2 * in_quad(t) : 2 * out_quad(t) - 1;
    return t;
}

float Bouncee::spike_quad(float t)
{
    t = t <= 0.5f ? 4 * in_quad(t) : 4 * in_quad(t - 1);
    return t;
}

float Bouncee::in_cub(float t)
{
    t = t * t * t;
    return t;
}

float Bouncee::out_cub(float t)
{
    t = invert(in_cub(invert(t)));
    return t;
}

float Bouncee::in_out_cub(float t)
{
    t = t <= 0.5 ? 4 * in_cub(t) : 4 * out_cub(t) - 3;
    return t;
}

float Bouncee::spike_cub(float t)
{
    t = t <= 0.5f ? 8 * in_cub(t) : - 8 * in_cub(t - 1);
    return t;
}

float Bouncee::in_quart(float t)
{
    t = t * t * t * t;
    return t;
}

float Bouncee::out_quart(float t)
{
    t = invert(in_quart(invert(t)));
    return t;
}

float Bouncee::in_out_quart(float t)
{
    t = t <= 0.5f ? 8 * in_quart(t) : 8 * out_quart(t) - 7;
    return t;
}

float Bouncee::spike_quart(float t)
{
    t = t <= 0.5f ? 16 * in_quart(t) : 16 * in_quart(t - 1);
    return t;
}

float Bouncee::in_quint(float t)
{
    t = t * t * t * t * t;
    return t;
}

float Bouncee::out_quint(float t)
{
    t = invert(in_quint(invert(t)));
    return t;
}

float Bouncee::in_out_quint(float t)
{
    t = t <= 0.5f ? 16 * in_quint(t) : 16 * out_quint(t) - 15;
    return t;
}

float Bouncee::spike_quint(float t)
{
    t = t <= 0.5f ? 32 * in_quint(t) : - 32 * in_quint(t - 1);
    return t;
}

float Bouncee::in_expo(float t)
{
    t = 1 - sqrt(1 - t);
    return t;
}

float Bouncee::out_expo(float t)
{
    t = sqrt(t);
    return t;
}

float Bouncee::in_out_expo(float t)
{
    t = t <= 0.5f ? 0.5f - sqrt(1 - 2 * t) / 2 : sqrt(2 * t - 1) / 2 + 0.5f;
    return t;
}

float Bouncee::spike_expo(float t)
{
    t = t <= 0.5f ? 1 - sqrt(1 - 2 * t) : 1 - sqrt(2 * t - 1);
    return t;
}

float Bouncee::in_circular(float t)
{
    t = 1 - sqrt(1 - t * t);
    return t;
}

float Bouncee::out_circular(float t)
{
    t = sqrt(1 - (t - 1) * (t - 1));
    return t;
}

float Bouncee::in_out_circular(float t)
{
    t = t < 0.5f ? 0.5f - sqrt(0.25f - t * t) : sqrt(0.25f - (t - 1) * (t - 1)) + 0.5f;
    return t;
}

float Bouncee::spike_circular(float t)
{
    t = t < 0.5f ? 1 - sqrt(1 - 4 * t * t) : 1 - sqrt(1 - (2 * t - 2) * (2 * t - 2));
    return t;
}

float Bouncee::in_bounce(float t)
{
    t = invert(out_bounce(invert(t)));
    return t;
}

float Bouncee::out_bounce(float t)
{
    float offset = 2.75f;
    float scalar = 7.5625f;

    if (t < 1 / offset)
    {
        t = scalar * t * t;
    }
    else if (t < 2 / offset)
    {
        t = scalar * in_quad(t - 1.5f / offset) + 0.75f;
    }
    else if (t < 2.5f / offset)
    {
        t = scalar * in_quad(t - 2.25f / offset) + 0.9375f;
    }
    else
    {
        t = scalar * in_quad(t - 2.625f / offset) + 0.984375f;
    }
    return t;
}

float Bouncee::in_out_bounce(float t)
{
    t = t < 0.5f ? invert(out_bounce(1 - 2 * t)) / 2 : out_bounce(2 * t - 1) / 2 + 0.5f;
    return t;
}

float Bouncee::spike_bounce(float t)
{
    t = t < 0.5f ? invert(out_bounce(1 - 2 * t)) : invert(out_bounce( 2 * t - 1));
    return t;
}

float Bouncee::in_elastic(float t, float amplitude, float period)
{
    if (t <= 0.0f)
    {
        return 0;
    }
    if (t >= 1.0f)
    {
        return 1;
    }
    t = - in_quad(t * amplitude) * sin((t - 0.75f) * PI * 2 * period);
    return t;
}

float Bouncee::in_elastic(float t)
{
    t = in_elastic(t, 1.0f, 3.0f);
    return t;
}

float Bouncee::out_elastic(float t)
{
    t = invert(in_elastic(invert(t)));
    return t;
}

float Bouncee::in_out_elastic(float t)
{
    float amplitude = 1.0f * 2.0f;
    float period = 3.0f * 1.645f;
    t = t <= 0.5f ? in_elastic(t, amplitude, period) / 2 : invert(in_elastic(invert(t), amplitude, period)) / 2 + 0.5f;
    return t;
}

float Bouncee::spike_elastic(float t)
{
    float amplitude = 1.0f * 2.0f;
    float period = 3.0f * 1.645f;
    t = t <= 0.5f ? in_elastic(t, amplitude, period) :  in_elastic(invert(t), amplitude, period);
    return t;
}

float Bouncee::in_back(float t)
{
    float a = 1.70158f;
    t = (a + 1) * in_cub(t) - a * t * t;
    return t;
}

float Bouncee::out_back(float t)
{
    t = invert(in_back(invert(t)));
    return t;
}

float Bouncee::in_out_back(float t)
{
    float p = 1.75f;
    t = t <= 0.5f ? in_back(p * t) : invert(in_back(p * invert(t)));
    return t;
}

float Bouncee::spike_back(float t)
{
    float p = 2.0f;
    t = t <= 0.5f ? in_back(p * t) : in_back(p * invert(t));
    return t;
}

//Helper functions
float Bouncee::invert(float t)
{
    return 1 - t;
}