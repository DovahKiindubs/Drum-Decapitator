#pragma once

#include "dsp1d_mod_header.h"
#include "dsp1d_mod_fastMath.h"

namespace dsp1d {
    namespace mod {

        template<typename T>
        class LogDecayTime
        {
        public:
            LogDecayTime() = default;

            void setFs(T fs) noexcept
            {
                mFs = fs;
                setMS(mMS);
            }

            void setMS(T ms) noexcept
            {
                mMS = std::max(T(0), ms);
                mR = getCoeffFromMS(mMS, mFs);
            }

            T getMS() const noexcept
            {
                return mMS;
            }

            const T& getSmoothCoeffForOldY() const noexcept
            {
                return mR;
            }

            T getFs() const noexcept
            {
                return mFs;
            }

        private:
            T mR = 0;
            T mMS = 0;
            T mFs = 44100;

            static T getCoeffFromMS(T ms, T fs) noexcept
            {
                massert(ms >= 0);
                massert(fs > 0);
                ms = std::max(T(0), ms);
                auto coeff = (ms == T(0)) ? T(0) : std::pow(std::exp(T(-1.0)), 1 / (fs * ms / T(1000)));
                return coeff;
            }
        };

    } // end of namespace mod
} // end of namespace dsp1d