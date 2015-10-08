#include <stdio.h>
#include <stdlib.h>

/*--------------------------------------------------------------------------*/
/*  DUMP.C								    */
/*    Dump a packed integer file					    */
/*									    */
/*    Usage:  dump filename						    */
/*									    */
/*- Modification History ---------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  23-Nov-13	Christopher G. Healey	Initial implementation		    */
/*--------------------------------------------------------------------------*/


int main( int argc, char *argv[] )
{
  int   i;				/* Loop counter */
  FILE *inp;				/* Input stream */


  if ( argc != 2 ) {
    printf( "Usage:  dump filename\n" );
    exit( 0 );
  }

  if ( ( inp = fopen( argv[ 1 ], "rb" ) ) == NULL ) {
    printf( "main(), cannot open %s for input\n", argv[ 1 ] );
    exit( 0 );
  }

  fread( &i, sizeof( int ), 1, inp );

  while( !feof( inp ) ) {
    printf( "%d\n", i );
    fread( &i, sizeof( int ), 1, inp );
  }

  fclose( inp );
  return 1;
}
