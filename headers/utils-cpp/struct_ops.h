#pragma once

#define STRUCT_OP(STRUCT, OP) \
    inline bool operator OP(const STRUCT& lhs, const STRUCT& rhs) \
    { \
        return lhs.tie() OP rhs.tie(); \
    }

#define STRUCT_COMPARISONS(STRUCT) \
    STRUCT_OP(STRUCT, ==) \
    STRUCT_OP(STRUCT, !=) \
    STRUCT_OP(STRUCT, <) \
    STRUCT_OP(STRUCT, <=) \
    STRUCT_OP(STRUCT, >=) \
    STRUCT_OP(STRUCT, >)

//    Example
//  -----------
//
//  struct My_Struct
//  {
//      int a_;
//      double b_;
//      auto tie() const { return std::tie(a_, b_); }
//  };
//
//  STRUCT_COMPARISONS(My_Struct)
