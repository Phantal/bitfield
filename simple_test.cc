#include <assert.h>

#include "bitfieldmember/BitfieldMember.h"

union Asdf {
  struct {
    unsigned char data[4];
  };

  BitfieldMember<unsigned,sizeof(data),0 ,5 > f0;
  BitfieldMember<unsigned,sizeof(data),5 ,5 > f1;
  BitfieldMember<unsigned,sizeof(data),10,1 > f2;
  BitfieldMember<unsigned,sizeof(data),11,1 > f3;
  BitfieldMember<unsigned,sizeof(data),12,13> f4;
  BitfieldMember<unsigned,sizeof(data),25,7 > f5;

  Asdf() = default;

  template <typename T>
  Asdf( const T& other ) { *this = other; }

  template <typename T>
  Asdf& operator=( const T& other ) {
    f0 = other.f0;
    f1 = other.f1;
    f2 = other.f2;
    f3 = other.f3;
    f4 = other.f4;
    f5 = other.f5;

    return *this;
  };
};

struct Fdsa {
  unsigned f0 : 5;
  unsigned f1 : 5;
  unsigned f2 : 1;
  unsigned f3 : 1;
  unsigned f4 : 13;
  unsigned f5 : 7;
};

#define TEST_FIELD( F ) \
  for( fdsa.F = 1; 0 != fdsa.F; ++fdsa.F ) { \
    asdf.F += 1; \
    assert( asdf.F == fdsa.F ); \
    assert( fdsa.F == asdf.F ); \
  }

int main() {
  Asdf asdf;
  Fdsa fdsa;

  assert( sizeof(asdf) == sizeof(fdsa) );

  TEST_FIELD( f0 );
  TEST_FIELD( f1 );
  TEST_FIELD( f2 );
  TEST_FIELD( f3 );
  TEST_FIELD( f4 );
  TEST_FIELD( f5 );

  return 0;
}
