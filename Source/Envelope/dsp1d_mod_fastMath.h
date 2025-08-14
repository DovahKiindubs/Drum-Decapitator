/**
 * This file is part of the Three-Body Technology code base.
 * Copyright(C), 2017-2027, Three-Body Technology, all rights reserved.
 * 
 * This file is protected by the copyright law of the people's Republic of China.
 * Permission to use this file is only granted for the employee of Three-Body
 * Technology, or other company/group/individual who has expressly authorized by
 * Three-Body Technology.
 * 
 * 此文件属于Three-Body Technology公司代码库.
 * 版权所有 2017-2027, Three-Body Technology
 * 
 * 此文件受中华人民共和国软件著作权法保护, 使用者仅限Three-Body Technology公司员工,
 * 以及由Three-Body Technology公司明确授权的公司/组织/个人.
 * 
 * @author 萌克
 * @date   2022年7月15日17点32分
 */


#pragma once


#include "dsp1d_mod_header.h"


namespace dsp1d {


template< typename T >
inline T mod_fast_tan( T x ) noexcept
{
    auto x2 = x * x;
    auto numerator = x * ( -135135 + x2 * ( 17325 + x2 * ( -378 + x2 ) ) );
    auto denominator = -135135 + x2 * ( 62370 + x2 * ( -3150 + 28 * x2 ) );
    return numerator / denominator;
}


} // end of namespace dsp1d

