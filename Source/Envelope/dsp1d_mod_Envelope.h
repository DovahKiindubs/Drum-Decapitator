#pragma once

#include "dsp1d_mod_LogDecayTime.h"
#include "dsp1d_mod_SmootherKernel.h"

namespace dsp1d {
    namespace mod {

        class Envelope {
        public:
            Envelope() {}

            void init(float fs) {
                mAttack.setFs(fs);
                mRelease.setFs(fs);
            }

            void setAttackMs(float ms) { mAttack.setMS(ms); }

            void setReleaseMs(float ms) { mRelease.setMS(ms); }

            inline float process(float x) noexcept {
                auto c = x > mOldY;
                float R =
                    c ? mAttack.getSmoothCoeffForOldY() : mRelease.getSmoothCoeffForOldY();
                mOldY = mFilter(x, R);
                return mOldY;
            }

        private:
            dsp1d::mod::LogDecayTime<float> mAttack;
            dsp1d::mod::LogDecayTime<float> mRelease;
            SmootherKernel<float> mFilter;
            float mOldY = 0;
        };

    } // end of namespace mod
} // end of namespace dsp1d