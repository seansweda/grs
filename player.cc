// $Id$

#include "player.h"

// Constructor function


player::player(char* str)
{
    strcpy( name, str );
    memset( team, '\0', TEAMLEN );
    memset( rtn, '\0', TEAMLEN );
    memset( pos, '\0', POSLEN );
    ab = h = r = rbi = b2 = b3 = hr = bb = k = sb = cs = pal = par = 0;
}

// Better constructor function

player::player(char* inname, char* inteam, char* inrtn, char* inpos)
{
    strcpy( name, inname );
    strcpy( team, inteam );
    strcpy( rtn, inrtn );
    strcpy( pos, inpos );
    ab = h = r = rbi = b2 = b3 = hr = bb = k = sb = cs = pal = par = 0;

    switch (*inpos) {
	case '1' : posn = 3; break;
        case '2' : posn = 4; break;
        case '3' : posn = 5; break;
        case 's' :
        case 'S' : posn = 6; break;
        case 'l' :
        case 'L' : posn = 7; break;
        case 'r' :  
        case 'R' : posn = 9; break;
        case 'c' : 
        case 'C' : switch (*(inpos+1)) {
			case 'f' :
			case 'F' : posn = 8;
				   break;
			default  : posn = 2;
		    }
		    break;
       default  : posn = 0;
    }
}

    void 
player::hit(int i)
{
    h++;
    ab++;
    switch (i) {
	case 2: b2++; break;
	case 3: b3++; break;
	case 4: hr++; break;
    }
}

    void 
player::move(char* str)
{
    for ( int i=0; i<2; i++ ) pos[i] = *str + i;
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
    return(name);
}

    char* 
player::pout()
{
    char *p;

    if ( strlen(pos) < 3 )
	return(pos);
    else
	p = strrchr( pos, int('-') );
    return(++p);
}

    void 
player::pa(char c)
{
    if (c=='r' || c=='R') par++;
    if (c=='l' || c=='L') pal++;
}

    void 
player::new_pos(char *newpos)
{
    strcat(pos,"-");
    strcat(pos,newpos);

    switch (*newpos) {
	case '1' :  posn = 3; break;
	case '2' :  posn = 4; break;
	case '3' :  posn = 5; break;
	case 's' :
	case 'S' :  posn = 6; break;
	case 'l' :
	case 'L' :  posn = 7; break;
	case 'r' :
	case 'R' :  posn = 9; break;
	case 'c' :
	case 'C' :  switch (*(newpos+1)) {
			case 'f' :
			case 'F' : posn = 8;
				   break;
			default  : posn = 2;
		    }
		    break;
	default  : posn = 0;
    }
}
