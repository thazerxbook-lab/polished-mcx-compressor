#pragma once
#include <algorithm>
#include <cmath>

// POLISHED MCX - motor de compresion (sin dependencias de JUCE)
namespace polished
{
constexpr double kPi = 3.14159265358979323846;

struct Params
{
    float thresholdDb = -18.f;
    float ratio       = 4.f;
    float attackMs    = 10.f;
    float releaseMs   = 150.f;
    float kneeDb      = 6.f;     // ancho de la rodilla en dB
    float makeupDb    = 0.f;
    float mix         = 1.f;     // 0..1 (compresion paralela)
    float scHz        = 20.f;    // filtro pasa-altos del sidechain (<=21 = apagado)
    float link        = 1.f;     // 0..1 enlace estereo
    bool  opto        = false;   // false = Electro, true = Opto
    bool  arc         = true;    // Auto Release Control
    bool  rms         = false;   // false = Peak, true = RMS
    bool  autoGain    = false;
};

inline float dbToGain (float db) { return std::pow (10.f, db * 0.05f); }
inline float gainToDb (float g)  { return 20.f * std::log10 (std::max (g, 1e-6f)); }

inline float autoMakeupDb (const Params& p)
{
    return p.autoGain ? std::max (0.f, -p.thresholdDb * (1.f - 1.f / p.ratio) * 0.5f) : 0.f;
}

// Cantidad de reduccion (positiva, en dB) para un nivel de entrada, con rodilla suave
inline float reductionDb (float xDb, float T, float R, float W)
{
    const float over = xDb - T;
    const float slope = 1.f - 1.f / R;
    if (W <= 0.01f) return over > 0.f ? over * slope : 0.f;
    if (2.f * over < -W) return 0.f;
    if (2.f * std::abs (over) <= W)
    {
        const float t = over + W * 0.5f;
        return slope * t * t / (2.f * W);
    }
    return over * slope;
}

struct HighPass
{
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void reset() { z1 = z2 = 0; }
    void set (double sr, double f)
    {
        const double w0 = 2.0 * kPi * f / sr, cw = std::cos (w0), alpha = std::sin (w0) / (2.0 * 0.70710678);
        const double a0 = 1.0 + alpha;
        b0 = (1 + cw) / 2 / a0; b1 = -(1 + cw) / a0; b2 = b0;
        a1 = -2 * cw / a0;      a2 = (1 - alpha) / a0;
    }
    float process (float x)
    {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return (float) y;
    }
};

class Compressor
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        rmsCoef = (float) std::exp (-1.0 / (0.010 * sr));   // ventana RMS ~10 ms
        susCoef = (float) std::exp (-1.0 / (0.500 * sr));   // memoria del programa ~0.5 s
        lastSc = -1.f;
        reset();
        setParams (p);
    }

    void reset()
    {
        for (int c = 0; c < 2; ++c) { env[c] = 0.f; sustain[c] = 0.f; ms[c] = 0.f; hp[c].reset(); }
    }

    void setParams (const Params& np)
    {
        p = np;
        totalMakeup = p.makeupDb + autoMakeupDb (p);
        scOn = p.scHz > 21.f;
        if (scOn && p.scHz != lastSc)
        {
            hp[0].set (sr, p.scHz);
            hp[1].set (sr, p.scHz);
            lastSc = p.scHz;
        }
    }

    float grDb() const { return std::max (env[0], env[1]); }

    void process (float& l, float& r)
    {
        float x[2] = { l, r }, d[2];
        for (int c = 0; c < 2; ++c)
        {
            const float s = scOn ? hp[c].process (x[c]) : x[c];
            if (p.rms)
            {
                ms[c] += (s * s - ms[c]) * (1.f - rmsCoef);
                d[c] = gainToDb (std::sqrt (ms[c]) * 1.41421356f);   // seno a fondo de escala = 0 dB
            }
            else
                d[c] = gainToDb (std::abs (s));
        }
        const float linked = std::max (d[0], d[1]);

        for (int c = 0; c < 2; ++c)
        {
            const float det = p.link * linked + (1.f - p.link) * d[c];
            const float target = reductionDb (det, p.thresholdDb, p.ratio, p.kneeDb);

            // memoria del programa (cuanto tiempo/cuanta compresion lleva sostenida)
            const float susTarget = std::clamp (env[c] / 12.f, 0.f, 1.f);
            sustain[c] = susTarget + susCoef * (sustain[c] - susTarget);

            float tauA = std::max (0.05f, p.attackMs);
            float factor = 1.f;
            if (p.opto) { tauA = std::max (tauA, 6.f); factor *= 1.f + 2.f * sustain[c]; }
            if (p.arc)  { factor *= 0.3f + 1.7f * sustain[c]; }
            const float tauR = std::max (1.f, p.releaseMs * factor);

            const float aA = (float) std::exp (-1.0 / (tauA * 0.001 * sr));
            const float aR = (float) std::exp (-1.0 / (tauR * 0.001 * sr));
            env[c] = target + (target > env[c] ? aA : aR) * (env[c] - target);

            const float g = dbToGain (totalMakeup - env[c]);
            x[c] *= (1.f - p.mix) + p.mix * g;
        }
        l = x[0];
        r = x[1];
    }

private:
    Params p;
    double sr = 44100.0;
    float rmsCoef = 0.f, susCoef = 0.f, totalMakeup = 0.f, lastSc = -1.f;
    bool scOn = false;
    float env[2] {}, sustain[2] {}, ms[2] {};
    HighPass hp[2];
};
} // namespace polished
