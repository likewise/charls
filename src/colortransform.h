//
// (C) Jan de Vaan 2007-2010, all rights reserved. See the accompanying "License.txt" for licensed use.
//
#ifndef CHARLS_COLORTRANSFORM
#define CHARLS_COLORTRANSFORM


#include "util.h"


// This file defines simple classes that define (lossless) color transforms.
// They are invoked in processline.h to convert between decoded values and the internal line buffers.
// Color transforms work best for computer generated images, but are outside the official JPEG-LS specifications.

template<typename sample>
struct TransformNoneImpl
{
    typedef sample SAMPLE;

    force_inline Triplet<SAMPLE> operator()(int v1, int v2, int v3) const
    {
        return Triplet<SAMPLE>(v1, v2, v3);
    }
};


template<typename sample>
struct TransformNone : TransformNoneImpl<sample>
{
    typedef struct TransformNoneImpl<sample> INVERSE;
};


template<typename sample>
struct TransformHp1
{
    enum { RANGE = 1 << (sizeof(sample) * 8) };
    typedef sample SAMPLE;

    struct INVERSE
    {
        explicit INVERSE(const TransformHp1&)
        {
        }

        force_inline Triplet<SAMPLE> operator()(int v1, int v2, int v3) const
        {
            return Triplet<SAMPLE>(v1 + v2 - RANGE / 2, v2, v3 + v2 - RANGE / 2);
        }
    };

    force_inline Triplet<SAMPLE> operator()(int R, int G, int B) const
    {
        Triplet<SAMPLE> hp1;
        hp1.v2 = static_cast<SAMPLE>(G);
        hp1.v1 = static_cast<SAMPLE>(R - G + RANGE / 2);
        hp1.v3 = static_cast<SAMPLE>(B - G + RANGE / 2);
        return hp1;
    }
};


template<typename sample>
struct TransformHp2
{
    enum { RANGE = 1 << (sizeof(sample) * 8) };
    typedef sample SAMPLE;

    struct INVERSE
    {
        explicit INVERSE(const TransformHp2&)
        {
        }

        force_inline Triplet<SAMPLE> operator() (int v1, int v2, int v3) const
        {
            Triplet<SAMPLE> rgb;
            rgb.R  = static_cast<SAMPLE>(v1 + v2 - RANGE / 2);                     // new R
            rgb.G  = static_cast<SAMPLE>(v2);                                      // new G
            rgb.B  = static_cast<SAMPLE>(v3 + ((rgb.R + rgb.G) >> 1) - RANGE / 2); // new B
            return rgb;
        }
    };

    force_inline Triplet<SAMPLE> operator()(int R, int G, int B) const
    {
        return Triplet<SAMPLE>(R - G + RANGE / 2, G, B - ((R+G )>>1) - RANGE / 2);
    }
};


template<typename sample>
struct TransformHp3
{
    enum { RANGE = 1 << (sizeof(sample) * 8) };
    typedef sample SAMPLE;

    struct INVERSE
    {
        explicit INVERSE(const TransformHp3&)
        {
        }

        force_inline Triplet<SAMPLE> operator()(int v1, int v2, int v3) const
        {
            const int G = v1 - ((v3 + v2) >> 2) + RANGE / 4;
            Triplet<SAMPLE> rgb;
            rgb.R  = static_cast<SAMPLE>(v3 + G - RANGE / 2); // new R
            rgb.G  = static_cast<SAMPLE>(G);                  // new G
            rgb.B  = static_cast<SAMPLE>(v2 + G - RANGE / 2); // new B
            return rgb;
        }
    };

    force_inline Triplet<SAMPLE> operator() (int R, int G, int B) const
    {
        Triplet<SAMPLE> hp3;
        hp3.v2 = static_cast<SAMPLE>(B - G + RANGE / 2);
        hp3.v3 = static_cast<SAMPLE>(R - G + RANGE / 2);
        hp3.v1 = static_cast<SAMPLE>(G + ((hp3.v2 + hp3.v3)>>2)) - RANGE / 4;
        return hp3;
    }
};


// Transform class that shifts bits towards the high bit when bit count is not 8 or 16
// needed to make the HP color transforms work correctly.
template<typename TRANSFORM>
struct TransformShifted
{
    typedef typename TRANSFORM::SAMPLE SAMPLE;

    struct INVERSE
    {
        explicit INVERSE(const TransformShifted& transform) :
            _shift(transform._shift),
            _inverseTransform(transform._colortransform)
        {
        }

        force_inline Triplet<SAMPLE> operator() (int v1, int v2, int v3)
        {
            Triplet<SAMPLE> result = _inverseTransform(v1 << _shift, v2 << _shift, v3 << _shift);
            return Triplet<SAMPLE>(result.R >> _shift, result.G >> _shift, result.B >> _shift);
        }

        force_inline Quad<SAMPLE> operator() (int v1, int v2, int v3, int v4)
        {
            Triplet<SAMPLE> result = _inverseTransform(v1 << _shift, v2 << _shift, v3 << _shift);
            return Quad<SAMPLE>(result.R >> _shift, result.G >> _shift, result.B >> _shift, v4);
        }

        int _shift;
        typename TRANSFORM::INVERSE _inverseTransform;
    };


    explicit TransformShifted(int shift) :
        _shift(shift)
    {
    }

    force_inline Triplet<SAMPLE> operator() (int R, int G, int B)
    {
        Triplet<SAMPLE> result = _colortransform(R << _shift, G << _shift, B << _shift);
        return Triplet<SAMPLE>(result.R >> _shift, result.G >> _shift, result.B >> _shift);
    }

    force_inline Quad<SAMPLE> operator() (int R, int G, int B, int A)
    {
        Triplet<SAMPLE> result = _colortransform(R << _shift, G << _shift, B << _shift);
        return Quad<SAMPLE>(result.R >> _shift, result.G >> _shift, result.B >> _shift, A);
    }

    int _shift;
    TRANSFORM _colortransform;
};


#endif
