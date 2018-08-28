#pragma once

/* In the long-run, at a minimum this header is intended
to be used to augment test writing and using it would
look [roughly] like this:

GENERATOR_CREATE_TYPE( NewType, uint32_t,
  ((unsigned short)(field0)(9))
  ((unsigned char)(field1)(7))
  ((unsigned char)(field2)(7))
  ((unsigned short)(field3)(9))
);

NewType asdf;
NewType::C_style fdsa;

asdf.field0 = fdsa.field0 = 13;
*/

#include <type_traits>
#include <stdint.h>
#include <sys/types.h>

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/arithmetic.hpp>

/* TODO:
  This isn't functional as-is.  It's a re-type from a less general
  implementation and needs some love to get it where I want it.
*/

#define GENERATOR_APPLY( F, ... ) F( __VA_ARGS__ )

#define GENERATOR_CREATE_ELEMENT_( COUNTER, BASE_TYPE, FIELD_TYPE, FIELD_NAME, LENGTH ) \
  static_assert( LENGTH > 0, "Defining field " #FIELD_NAME ); \
  bitpack::Accessor<BASE_TYPE, FIELD_TYPE, BitAssignmentOrder::left_to_right, Offsets::field_##COUNTER - LENGTH, LENGTH>> FIELD_NAME;

#define GENERATOR_CREATE_ELEMENT( x, BASE_TYPE, COUNTER, ARGS ) \
  GENERATOR_APPLY( GENERATOR_CREATE_ELEMENT_, COUNTER, BASE_TYPE, BOOST_PP_SEQ_ENUM( ARGS ) )

#define GENERATOR_CREATE_OLD_BITFIELD_ELEMENT_( FIELD_TYPE, FIELD_NAME, LENGTH ) \
  static_assert( LENGTH > 0, "When defining field " #FIELD_NAME ); \
  FIELD_TYPE FIELD_NAME : LENGTH;

#define GENERATOR_CREATE_OLD_BITFIELD_ELEMENT( x, BASE_TYPE, ARGS ) \
  GENERATOR_APPLY( CREATE_OLD_BITFIELD_ELEMENT_, BOOST_PP_SEQ_ENUM( ARGS ) )

#define GENERATOR_OFFSET_ENUM_ELEMENTS_( COUNTER, COUNTER_MINUS_1, FIELD_TYPE, FIELD_NAME, LENGTH ) \
  BOOST_PP_COMMA_IF( BOOST_PP_NOT_EQUAL( COUNTER, 0 ) ) \
  field_##COUNTER = LENGTH \
    BOOST_PP_IF( \
        BOOST_PP_NOT_EQUAL(COUNTER,0) \
      , + field_##COUNTER_MINUS_1, )

#define GENERATOR_OFFSET_ENUM_ELEMENTS( x, meh, COUNTER, ARGS ) \
  GENERATOR_APPLY( GENERATOR_OFFSET_ENUM_ELEMENTS_, COUNTER, BOOST_PP_SUB(COUNTER,1), BOOST_PP_SEQ_ENUM( ARGS ) )

#define GENERATOR_CREATE_TYPE_( LTR_OR_RTL, TYPE_NAME, BASE_TYPE, ELEMENTS ) \
  static_assert(  std::is_integral<BASE_TYPE>::value && std::is_unsigned<BASE_TYPE>::value \
                , "base type must be an unsigned integral type" ); \
  union TYPE_NAME { \
    using StorageType = BASE_TYPE; \
    \
    TYPE_NAME( StorageType initial_value = 0 ) : data() {} \
    \
    enum BitAssignmentOrder { \
      left_to_right = (LTR_OR_RTL == 0) \
    , right_to_left = !left_to_right \
    }; \
    \
    struct { StorageType data; }; \
    \
    StorageType& get()       { return wrapper.m_value; }; \
    StorageType  get() const { return wrapper.m_value; }; \
    \
    struct C_style { \
      BOOST_PP_SEQ_FOR_EACH( GENERATOR_CREATE_OLD_BITFIELD_ELEMENT, TYPE_NAME::StorageType, ELEMENTS ) \
    }; \
    \
    static_assert( sizeof(StorageType) >= sizeof(C_style), "C-style bitfield's size is larger than new type's size" ); \
    \
    template <typename T> TYPE_NAME& operator=( T t ) = delete; \
    template <typename T> TYPE_NAME& operator=( const T& t ) { \
      static_assert( !std::is_floating_point<T>::value, "Floating point types are not supported as args to this function" ); \
      data = t; return *this; \
    } \
    \
    operator StorageType&()      { return this->get(); } \
    operator StorageType() const { return this->get(); } \
    \
    StorageType* operator&() { return &data; } \
    \
    enum Offsets { \
      BOOST_PP_SEQ_FOR_EACH_I( GENERATOR_OFFSET_ENUM_ELEMENTS, ~, ELEMENTS ) \
    }; \
    \
    BOOST_PP_SEQ_FOR_EACH_I( GENERATOR_CREATE_ELEMENT, TYPE_NAME::StorageType, ELEMENTS ) \
  }

#define GENERATOR_CREATE_LTR_TYPE( ... ) \
  GENERATOR_CREATE_TYPE_( 0, __VA_ARGS__ )

#define GENERATOR_CREATE_RTL_TYPE( TYPE_NAME, BASE_TYPE, ELEMENTS ) \
  GENERATOR_CREATE_TYPE_( 1, TYPE_NAME, BASE_TYPE, BOOST_PP_SEQ_REVERSE( ELEMENTS ) )

}
