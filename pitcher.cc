#include "pitcher.h"

// Constructor function


pitcher::pitcher(char* str)
{
	strcpy( name, str );
	memset( team, '\0', TEAMLEN );
	memset( rtn, '\0', TEAMLEN );
	throws = '\0';
	dec = '-';
	out = h = r = er = hr = bb = k = bf = 0;
}

// Better constructor function

pitcher::pitcher(char* inname, char* inteam, char* inrtn, char inpos)
{
	strcpy( name, inname );
	strcpy( team, inteam );
	strcpy( rtn, inrtn );
	throws = inpos;
	dec = '-';
	out = h = r = er = hr = bb = k = bf = 0;
}

    void
pitcher::sout(FILE *fp)
{
    fprintf(fp,"%c %-5s%-5s%-17s%3d.%1d%3d%3d%3d%3d%3d%3d %3d",
		dec,team,rtn,name,out/3,out % 3,h,r,er,bb,k,hr,bf);
}

    char*
pitcher::nout()
{
    return(name);
}

    char*
pitcher::tout()
{
    return(rtn);
}

    void
pitcher::hit(int i)
{
    h++;
    if (i==4) hr++;
}

