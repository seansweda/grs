// $Id$

#include "player.h"

// Constructor function
player::player( char* str )
{
    strcpy( name, str );
    memset( team, '\0', TEAMLEN );
    memset( rtn, '\0', TEAMLEN );
    pos = (char*) calloc( 3, sizeof(char));
    memset( pos, '\0', POSLEN );
    ab = h = r = rbi = b2 = b3 = hr = bb = k = sb = cs = pal = par = 0;
}

// Better constructor function
player::player( char* inname, char* inteam, char* inrtn, char* inpos )
{
    strcpy( name, inname );
    strcpy( team, inteam );
    strcpy( rtn, inrtn );
    pos = (char*) calloc( 3, sizeof(char));
    strcpy( pos, inpos );
    ab = h = r = rbi = b2 = b3 = hr = bb = k = sb = cs = pal = par = 0;
    posn = getpos( pos );
}

    void 
player::hit( int i )
{
    h++;
    ab++;
    switch ( i ) {
	case 2: 
	    b2++; 
	    break;
	case 3: 
	    b3++; 
	    break;
	case 4: 
	    hr++; 
	    break;
    }
}

    void 
player::move(char* str)
{
    for ( int i=0; i<2; i++ ) 
	pos[i] = *str + i;
}

    void 
player::sout(FILE *fp)
{
    fprintf(fp,"%-8s %-5s%-5s%-17s%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d",
		pos,team,rtn,name,ab,r,h,rbi,b2,b3,hr,sb,cs,bb,k,pal,par);
}

    char* 
player::nout()
{
    return( name );
}

    char* 
player::pout()
{
    char *p;

    if ( strlen( pos ) < 3 )
	return( pos );
    else
	p = strrchr( pos, int('-') );
    return( ++p );
}

    void 
player::pa( char c )
{
    if ( c=='r' || c=='R' ) 
	par++;
    if ( c=='l' || c=='L' ) 
	pal++;
}

    void 
player::new_pos( char *newpos )
{
    // extend pos string
    pos = (char*) realloc( pos, strlen(pos) + strlen(newpos) );	
    strcat( pos, "-" );
    strcat( pos, newpos );
    posn = getpos( newpos );
}

    int
player::getpos( char *posstr )
{
    switch ( *posstr ) {
	case '1' : 
	    return( 3 );
        case '2' : 
	    return( 4 );
        case '3' : 
	    return( 5 );
        case 's' :
        case 'S' : 
	    return( 6 );
        case 'l' :
        case 'L' : 
	    return( 7 );
        case 'r' :  
        case 'R' : 
	    return( 9 );
        case 'c' : 
        case 'C' : 
	    switch ( *(posstr+1) ) {
		case 'f' :
		case 'F' : 
		    return( 8 );
		default  : 
		    return( 2 );
	    }
	default  : 
	    return( 0 );
    }
}

player::~player()
{
    free(pos);
#ifdef DEBUG
    fprintf(stderr,"deleted %s\n",this->nout());
#endif
}

