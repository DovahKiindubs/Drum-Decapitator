#pragma once

#include "dsp1d_mod_fastMath.h"
#include "dsp1d_mod_header.h"

namespace dsp1d {
    namespace mod {

        template <typename T>
        class SmootherKernel {
        public:
            SmootherKernel() : mOldY(T(0)) {}

            void resetBuffer() { mOldY = T(0); }

            T operator()(const T& x, const T& R) noexcept
            {
                mOldY = x + R * (mOldY - x);
                return mOldY;
            }

            const T& getOldY() const noexcept { return mOldY; }

        private:
            T mOldY;
        };

    } // end of namespace mod
} // end of namespace dsp1d