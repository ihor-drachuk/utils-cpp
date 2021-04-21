#pragma once

#define TIED_OP(STRUCT, OP) \
    inline bool operator OP(const STRUCT& lhs, const STRUCT& rhs) \
    { \
        return lhs.tie() OP rhs.tie(); \
    }

#define TIED_COMPARISONS(STRUCT) \
    TIED_OP(STRUCT, ==) \
    TIED_OP(STRUCT, !=) \
    TIED_OP(STRUCT, <) \
    TIED_OP(STRUCT, <=) \
    TIED_OP(STRUCT, >=) \
    TIED_OP(STRUCT, >)

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
//  TIED_COMPARISONS(My_Struct)
