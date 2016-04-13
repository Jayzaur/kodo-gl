#pragma once

#include <cstdint>
#include <cmath>

struct Region
{
	int32_t X;
	int32_t Y;
	int32_t W;
	int32_t H;

	Region() : X( 0 ), Y( 0 ), W( 0 ), H( 0 ) {}
	Region( int32_t x, int32_t y, int32_t width, int32_t height ) : X( x ), Y( y ), W( width ), H( height ) { }
};

struct NormalizedRegion
{
	float_t L, T, R, B;

	NormalizedRegion() : L( 0 ), T( 0 ), R( 0 ), B( 0 ) {}
	NormalizedRegion( float_t left, float_t top, float_t right, float_t bottom ) : L( left ), T( top ), R( right ), B( bottom ) { }
};

