#pragma once
#include <tuple>

/*
    Example
  -----------

  struct My_Struct
  {
      int a_;
      double b_;

      auto tie() const { return std::tie(a_, b_); }
  };

  STRUCT_COMPARISONS(My_Struct);
*/

#define STRUCT_OP(STRUCT, OP) \
    inline bool operator OP(const STRUCT& lhs, const STRUCT& rhs) \
    { \
        return lhs.tie() OP rhs.tie(); \
    }

#define STRUCT_MEMBER_OP(STRUCT, OP) \
    inline bool operator OP(const STRUCT& rhs) const \
    { \
        return tie() OP rhs.tie(); \
    }

#define STRUCT_COMPARISONS(STRUCT) \
    STRUCT_OP(STRUCT, ==) \
    STRUCT_OP(STRUCT, !=) \
    STRUCT_OP(STRUCT, <) \
    STRUCT_OP(STRUCT, <=) \
    STRUCT_OP(STRUCT, >=) \
    STRUCT_OP(STRUCT, >)

#define STRUCT_COMPARISONS_ONLY_EQ(STRUCT) \
    STRUCT_OP(STRUCT, ==) \
    STRUCT_OP(STRUCT, !=)

#define STRUCT_COMPARISONS_MEMBER(STRUCT) \
    STRUCT_MEMBER_OP(STRUCT, ==) \
    STRUCT_MEMBER_OP(STRUCT, !=) \
    STRUCT_MEMBER_OP(STRUCT, <) \
    STRUCT_MEMBER_OP(STRUCT, <=) \
    STRUCT_MEMBER_OP(STRUCT, >=) \
    STRUCT_MEMBER_OP(STRUCT, >)

#define STRUCT_COMPARISONS_MEMBER_ONLY_EQ(STRUCT) \
    STRUCT_MEMBER_OP(STRUCT, ==) \
    STRUCT_MEMBER_OP(STRUCT, !=)
