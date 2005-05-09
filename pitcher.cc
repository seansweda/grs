// pitcher.cc

#include "pitcher.h"

// Constructor function


pitcher::pitcher(char* str)
{
	strcpy(name,str);
	team[0]='\0';
	rtn[0]='\0';
	throws='\0';
	dec='-';
	out=h=r=er=hr=bb=k=br=0;
}

// Better constructor function

pitcher::pitcher(char* inname, char* inteam, char* inrtn, char inpos)
{
	strcpy(name,inname);
	strcpy(team,inteam);
	strcpy(rtn,inrtn);
	throws=inpos;
	dec='-';
	out=h=r=er=hr=bb=k=br=0;
}

    void 
pitcher::sout(FILE *fp)
{
    fprintf(fp,"%c %-5s%-5s%-17s%3d.%1d%3d%3d%3d%3d%3d%3d",
		dec,team,rtn,name,out/3,out % 3,h,r,er,bb,k,hr);
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

