/*
  ==============================================================================

    SoftClipper.h
    Created: 10 Jul 2025 6:09:42pm
    Author:  DovahKiin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>


class SoftClipper {
public:
    SoftClipper() = default;

    void setParameters(float curveParam, float thresholdDB) {
        curve = curveParam;
		thresholdLinear = juce::Decibels::decibelsToGain(thresholdDB);
	}

    float processSample(float x) noexcept {
        return full_process(x, curve, thresholdLinear);
    }

private:
    float curve = 0.000001f;
    float thresholdLinear = 1.0f;

    float sigmoid(float x) noexcept
    {
        return 1.0f / (1.0f + std::exp(-x));
    }

    float f2(float x) noexcept
    {
        if (x < -1.0f)
        {
            return x;
        }
        else
        {
            return 2.0f * sigmoid(2.0f * x + 2.0f) - 2.0f;
        }
    }

    float full_process(float x, float  k, float  R) noexcept
    {
        float sign = (x < 0.0f) ? -1.0f : 1.0f;
        float absX = std::abs(x);
        float inner = (absX - 1.0f) / k;
        float process = k * f2(inner) + 1.0f;

        return sign * process * R;
    }
};