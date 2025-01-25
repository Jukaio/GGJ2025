#pragma once

inline float PI = 3.14159265359f;

class Bouncee
{
public:
    static float linear(float t);
    static float spike_linear(float t);
    static float in_sine(float t);
    static float out_sine(float t);
    static float in_out_sine(float t);
    static float spike_sine(float t);
    static float in_quad(float t);
    static float out_quad(float t);
    static float in_out_quad(float t);
    static float spike_quad(float t);
    static float in_cub(float t);
    static float out_cub(float t);
    static float in_out_cub(float t);
    static float spike_cub(float t);
    static float in_quart(float t);
    static float out_quart(float t);
    static float in_out_quart(float t);
    static float spike_quart(float t);
    static float in_quint(float t);
    static float out_quint(float t);
    static float in_out_quint(float t);
    static float spike_quint(float t);
    static float in_expo(float t);
    static float out_expo(float t);
    static float in_out_expo(float t);
    static float spike_expo(float t);
    static float in_circular(float t);
    static float out_circular(float t);
    static float in_out_circular(float t);
    static float spike_circular(float t);
    static float in_bounce(float t);
    static float out_bounce(float t);
    static float in_out_bounce(float t);
    static float spike_bounce(float t);
    static float in_elastic(float t, float amplitude, float period);
    static float in_elastic(float t);
    static float out_elastic(float t);
    static float in_out_elastic(float t);
    static float spike_elastic(float t);
    static float in_back(float t);
    static float out_back(float t);
    static float in_out_back(float t);
    static float spike_back(float t);
    //Helper functions
    static float invert(float t);
};