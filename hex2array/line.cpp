#ifdef _WIN32
#include "stdafx.h"
#define SPRINTF sprintf
#else
#define SPRINTF sprintf
#endif

#include "Line.h"
#include "string.h"


Line::Line( int bc )
{
    bytecount = bc;
    data = new char[bc];
}
/*
Line::Line( const Line &l )
{
    data = new char[l.bytecount];
    memcpy( data, l.data, sizeof( l.data ) );
}*/

Line::~Line()
{
    delete[] data;
}