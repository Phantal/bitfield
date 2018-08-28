#pragma once

// This is a heavily modified version of the following:
//
//   https://codereview.stackexchange.com/questions/54342/template-for-endianness-free-code-data-always-packed-as-big-endian
//
// 

#include <stdint.h>
#include <assert.h>

template<typename base_type, uint32_t SZ, uint32_t firstBit, uint32_t bitSize>
class BitfieldMember
{
  using self_t = BitfieldMember<base_type, SZ, firstBit, bitSize>;
  using uchar = unsigned char;

  uchar data[SZ];

  // TODO:
  //   Look at making 'data' use any underlying type to speed it up.
  //
  //   As a consequence these two situations will be a problem due
  //   to bit shifting:
  //      operator base_type():  sizeof(data[0]) -vs- sizeof(base_type)
  //      operator=(arg):        sizeof(data[0]) -vs- sizeof(arg)
  //
  //   Consider making the conversion/assignment operators templated
  //   and do the following (or ensure through static_assert):
  //     * operator=():
  //       * arg is unsigned
  //       * if sizeof(arg) <= sizeof(data[0])
  //           provide a specialization that doesn't loop
  //     * operator base_type():
  //       * if sizeof(base_type) >= sizeof(data[0])
  //           provide a specialization that doesn't loop
  //   
  static constexpr base_type bitsPer              = 8 * sizeof(data[0]);

  static constexpr base_type lastBit              = firstBit + bitSize - 1;
  static constexpr base_type mask                 = (1ULL << bitSize) - 1;
  static constexpr base_type firstBitIndex        = bitsPer - (firstBit & (bitsPer-1));
  static constexpr base_type excessBitsAfterField = (bitsPer-1) - (lastBit & (bitsPer-1));

  BitfieldMember(BitfieldMember&) = delete;
  BitfieldMember(BitfieldMember&&) = delete;
  BitfieldMember* operator&() = delete;

public:
  // TODO:
  //   I'm pretty sure I deleted this in my initial implementation,
  //   but clang 6 won't allow it.  Look into it.
  BitfieldMember() = default;

  inline operator base_type() const {
    base_type ret = 0;
    for( unsigned ii = firstBit / bitsPer; ii <= lastBit / bitsPer; ++ii ) {
      ret = (ret << bitsPer) | data[ii];
    }
    return (ret >> excessBitsAfterField) & mask;
  }

  inline self_t& operator=( base_type m ) {
    static_assert(  mask == ((mask << excessBitsAfterField) >> excessBitsAfterField)
                  , "Need a carry, larger type or rethink this logic" );

    m = (m & mask) << excessBitsAfterField;
    base_type write_mask = mask << excessBitsAfterField;
    // Signed is the right choice due to ii >= 0
    for( int ii = lastBit / bitsPer; ii >= 0 && 0 != write_mask; --ii ) {
      uchar& ref = data[ii];
      ref &= ~write_mask;
      ref |= m;
      m >>= bitsPer;
      write_mask >>= bitsPer;
    }

    return *this;
  }

  // Strictly speaking, this is only necessary under the original
  // implementation where this class has no storage.
  //
  // Without it the 4th line below wouldn't do anything:
  //   TheType a, b;
  //   a.field1 = 1;
  //   b.field1 = 2;
  //   b.field1 = a.field1;
  inline self_t& operator=( const self_t& m ) {
    return *this = static_cast<base_type>( m );
  }

#pragma push_macro("BF_MEMBER_OPERATOR_ASSIGN")
#if defined(BF_MEMBER_OPERATOR_ASSIGN)
#undef BF_MEMBER_OPERATOR_ASSIGN
#endif
#define BF_MEMBER_OPERATOR_ASSIGN(OP) \
  inline self_t& operator OP##=( base_type m ) { \
    *this = *this OP m; \
    return *this; \
  }

  BF_MEMBER_OPERATOR_ASSIGN( +  );
  BF_MEMBER_OPERATOR_ASSIGN( -  );
  BF_MEMBER_OPERATOR_ASSIGN( *  );
  BF_MEMBER_OPERATOR_ASSIGN( /  );
  BF_MEMBER_OPERATOR_ASSIGN( %  );
  BF_MEMBER_OPERATOR_ASSIGN( << );
  BF_MEMBER_OPERATOR_ASSIGN( >> );
  BF_MEMBER_OPERATOR_ASSIGN( |  );
  BF_MEMBER_OPERATOR_ASSIGN( &  );
  BF_MEMBER_OPERATOR_ASSIGN( ^  );

#pragma pop_macro("BF_MEMBER_OPERATOR_ASSIGN")
};

