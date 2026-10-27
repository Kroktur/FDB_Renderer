#ifndef KTR_FNV_1A_F
#define KTR_FNV_1A_F

/**
 * @file KTR_FNV_1A.h
 * @brief Aggregates all FNV-1A hashing implementations.
 *
 * This header provides a single entry point for the default hashing system
 * by including every available FNV-1A specialization. It allows users to
 * access hashing support for integral types, strings, enums, and pointers without
 * including each header individually.
 */

#include "KTR_FNV_1AIntegral.h"
#include "KTR_FNV_1AString.h"
#include "KTR_FNV_1APointer.h"
#include "KTR_FNV_1AEnum.h"

#endif
