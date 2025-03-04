
/*  ev3.c was generated by yupp 1.1c3
    out of ev3.yu-c 
 *//**
 *  \file  ev3.c (ev3.yu-c)
 *  \brief  EV3 file operations.
 *  \author  Vitaly Kravtsov (in4lio@gmail.com)
 *  \copyright  See the LICENSE file.
 */

#define EV3_IMPLEMENT

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "modp_numtoa.h"
#include "ev3.h"

/**
 *  \addtogroup ev3
 *  \{
 */

// EV3 BRICK /////////////////////////////////////
#ifdef __ARM_ARCH_5TEJ__

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

int ev3_init( void )
{
	return ( 1 );
}

void ev3_uninit( void )
{

}

size_t ev3_write_binary( const char *fn, char *data, size_t sz )
{
	FILE *f;
	size_t result;

	f = fopen( fn, "w" );
	if ( f == NULL ) return ( 0 );

	result = fwrite( data, 1, sz, f );
	fclose( f );
	return ( result );
}

size_t ev3_multi_write_binary( uint8_t *sn, uint16_t pos, const char *fn, char *data, size_t sz )
{
	int i = 0;
	size_t result = 0;

	while (( i < DESC_VEC_LEN ) && ( sn[ i ] < DESC_LIMIT )) {
		*modp_uitoa10( sn[ i ], ( char *) fn + pos ) = '/';
		result = ev3_write_binary( fn, data, sz );
		if ( result == 0 ) return ( 0 );
		i++;
	}
	return ( result );
}

size_t ev3_read_binary( const char *fn, char *buf, size_t sz )
{
	FILE *f;
	size_t result;

	f = fopen( fn, "r" );
	if ( f == NULL ) return ( 0 );

	result = fread( buf, 1, sz, f );
	fclose( f );
	return ( result );
}

#define _TEST_KEY( K, R )  (( keyb[ KEY_##K >> 3 ] & ( 1 << ( KEY_##K & 7 ))) ? EV3_KEY_##R : 0 )

size_t ev3_read_keys( uint8_t *buf )
{
	int f;
	uint8_t keyb[( KEY_MAX + 7 ) / 8 ];

	f = open( GPIO_KEYS_PATH, O_RDONLY );
	if ( f < 0 ) return ( 0 );

	ioctl( f, EVIOCGKEY( sizeof( keyb )), &keyb );
	*buf = _TEST_KEY( UP, UP )        | _TEST_KEY( DOWN, DOWN )
	     | _TEST_KEY( LEFT, LEFT )    | _TEST_KEY( RIGHT, RIGHT )
	     | _TEST_KEY( ENTER, CENTER ) | _TEST_KEY( BACKSPACE, BACK );
	close( f );
	return ( sizeof( uint8_t ));
}

#undef _TEST_KEY

static size_t __ev3_listdir( char *fn, void *buf, size_t sz )
{
	DIR *d;
	struct dirent *de;
	char *p;

	d = opendir( fn );
	if ( d == NULL ) return ( 0 );

	p = buf;
	while (( de = readdir( d ))) {
		size_t l = strlen( de->d_name ) + 1;
		if ( sz > l ) {
			sz -= l;
			memcpy( p, de->d_name, l - 1 );
			p[ l - 1 ] = ' ';
			p += l;
		}
	}
	closedir( d );
	return (( void *) p - buf );
}

bool ev3_poweroff( void )
{
	system( "shutdown -h now" );
	return ( true );
}

// CLIENT ////////////////////////////////////////
#else

#include "ev3_link.h"

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__

#include <windows.h>

// UNIX //////////////////////////////////////////
#else

#include <unistd.h>
#define Sleep( msec ) usleep(( msec ) * 1000 )

//////////////////////////////////////////////////
#endif

#define BRICK_WAIT_DELAY  1000  /* msec */
#define BRICK_WAIT_TRIES  10

int ev3_init( void )
{
	int i;

	if ( udp_ev3_open( ev3_brick_addr, ev3_brick_port ) == EOF ) return ( -1 );

	if ( ev3_brick_addr == NULL ) {
		for ( i = 0; i < BRICK_WAIT_TRIES; i++ ) {
			if ( udp_ev3_catch_address()) return ( 1 );
			Sleep( BRICK_WAIT_DELAY );
		}
	}
	return ( 0 );
}

void ev3_uninit( void )
{
	udp_ev3_close();
}

size_t ev3_write_binary( const char *fn, char *data, size_t sz )
{
	return udp_ev3_write(( char *) fn, data, sz );
}

size_t ev3_multi_write_binary( uint8_t *sn, uint16_t pos, const char *fn, char *data, size_t sz )
{
	return udp_ev3_multi_write( sn, pos, ( char *) fn, data, sz );
}

size_t ev3_read_binary( const char *fn, char *buf, size_t sz )
{
	return udp_ev3_read(( char *) fn, buf, sz );
}

size_t ev3_read_keys( uint8_t *buf )
{
	return udp_ev3_read_keys( buf );
}

static size_t __ev3_listdir( char *fn, char *buf, size_t sz )
{
	return udp_ev3_listdir( fn, buf, sz );
}

bool ev3_poweroff( void )
{
	return udp_ev3_poweroff();
}

//////////////////////////////////////////////////
#endif

size_t ev3_write( const char *fn, char *value )
{
	return ev3_write_binary( fn, value, strlen( value ));
}

size_t ev3_multi_write( uint8_t *sn, uint16_t pos, const char *fn, char *value )
{
	return ev3_multi_write_binary( sn, pos, fn, value, strlen( value ));
}

size_t ev3_write_int( const char *fn, int value )
{
	char s[ 21 ];

	modp_itoa10( value, s );
	return ev3_write( fn, s );
}

size_t ev3_multi_write_int( uint8_t *sn, uint16_t pos, const char *fn, int value )
{
	char s[ 21 ];

	modp_itoa10( value, s );
	return ev3_multi_write( sn, pos, fn, s );
}

size_t ev3_write_dword( const char *fn, uint32_t value )
{
	char s[ 20 ];

	modp_uitoa10( value, s );
	return ev3_write( fn, s );
}

size_t ev3_multi_write_dword( uint8_t *sn, uint16_t pos, const char *fn, uint32_t value )
{
	char s[ 20 ];

	modp_uitoa10( value, s );
	return ev3_multi_write( sn, pos, fn, s );
}

size_t ev3_write_float( const char *fn, float value )
{
	char s[ 32 ];

	modp_dtoa2( value, s, 4 );
	return ev3_write( fn, s );
}

size_t ev3_multi_write_float( uint8_t *sn, uint16_t pos, const char *fn, float value )
{
	char s[ 32 ];

	modp_dtoa2( value, s, 4 );
	return ev3_multi_write( sn, pos, fn, s );
}

size_t ev3_write_bool( const char *fn, bool value )
{
	return ev3_write_binary( fn, ( value ) ? "1" : "0", 1 );
}

size_t ev3_multi_write_bool( uint8_t *sn, uint16_t pos, const char *fn, bool value )
{
	return ev3_multi_write_binary( sn, pos, fn, ( value ) ? "1" : "0", 1 );
}

size_t ev3_write_byte( const char *fn, uint8_t value )
{
	return ev3_write_int( fn, value );
}

size_t ev3_multi_write_byte( uint8_t *sn, uint16_t pos, const char *fn, uint8_t value )
{
	return ev3_multi_write_int( sn, pos, fn, value );
}

size_t ev3_write_char_array( const char *fn, char *value )
{
	return ev3_write( fn, value );
}

size_t ev3_multi_write_char_array( uint8_t *sn, uint16_t pos, const char *fn, char *value )
{
	return ev3_multi_write( sn, pos, fn, value );
}

size_t ev3_write_byte_array( const char *fn, uint8_t *value, size_t sz )
{
	return ev3_write_binary( fn, ( char *) value, sz );
}

size_t ev3_multi_write_byte_array( uint8_t *sn, uint16_t pos, const char *fn, uint8_t *value, size_t sz )
{
	return ev3_multi_write_binary( sn, pos, fn, ( char *) value, sz );
}

size_t ev3_read( const char *fn, char *buf, size_t sz )
{
	size_t c;

	if ( sz < 1 ) return ( 0 );

	c = ev3_read_binary( fn, buf, sz );
	buf[ c - 1 ] = '\x00';
	return ( c );
}

size_t ev3_read_int( const char *fn, int *buf )
{
	char s[ 21 ];
	char *end;

	if ( ev3_read( fn, s, sizeof( s ))) {
		*buf = strtol( s, &end, 0 );
		if ( *end ) return ( 0 );

		return ( sizeof( int ));
	}
	return ( 0 );
}

size_t ev3_read_dword( const char *fn, uint32_t *buf )
{
	char s[ 20 ];
	char *end;

	if ( ev3_read( fn, s, sizeof( s ))) {
		*buf = strtoul( s, &end, 0 );
		if ( *end ) return ( 0 );

		return ( sizeof( uint32_t ));
	}
	return ( 0 );
}

size_t ev3_read_float( const char *fn, float *buf )
{
	char s[ 16 ];
	char *end;

	if ( ev3_read( fn, s, sizeof( s ))) {
		*buf = ( float ) strtod( s, &end );
		if ( *end ) return ( 0 );

		return ( sizeof( float ));
	}
	return ( 0 );
}

size_t ev3_read_bool( const char *fn, bool *buf )
{
	int _int;
	if ( ev3_read_int( fn, &_int )) {
		*buf = !!_int;

		return ( sizeof( bool ));
	}
	return ( 0 );
}

size_t ev3_read_byte( const char *fn, uint8_t *buf )
{
	int _int;
	if ( ev3_read_int( fn, &_int )) {
		*buf = ( uint8_t ) _int;

		return ( sizeof( uint8_t ));
	}
	return ( 0 );
}

size_t ev3_read_char_array( const char *fn, char *buf, size_t sz )
{
	return ev3_read( fn, buf, sz );
}

size_t ev3_read_byte_array( const char *fn, uint8_t *buf, size_t sz )
{
	return ev3_read_binary( fn, ( char*) buf, sz );
}

size_t ev3_listdir( const char *fn, char *buf, size_t sz )
{
	size_t c;

	if ( sz < 1 ) return ( 0 );

	c = __ev3_listdir(( char *) fn, buf, sz );
	buf[ c - 1 ] = '\x00';
	return ( c );
}

int ev3_string_suffix( const char *prefix, char **s, uint32_t *buf )
{
	char *end;
	uint32_t l = strlen( prefix );

	if ( memcmp( *s, prefix, l )) return ( 0 );

	*buf = strtoul( *s + l, &end, 0 );
	*s = end;
	if ( **s ) return ( 2 );

	return ( 1 );
}

/** \} */
