#include "pitcher.h"

// Constructor function
pitcher::pitcher(char* inname, char* inteam, char* inrtn, char inpos)
{
    snprintf( name, NAMELEN, "%s", inname );
    snprintf( team, TEAMLEN, "%s", inteam );
    snprintf( rtn, TEAMLEN, "%s", inrtn );
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

pitcher::~pitcher()
{
#ifdef DEBUG
    fprintf(stderr,"deleted %s\n",this->nout());
#endif
}

