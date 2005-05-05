// main.cc

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/param.h>
#include "pitcher.h"
#include "player.h"
#include "team.h"
#include "frame.h"

// variables used outside of main()

char VER[12] = "3.0.0beta1";

FILE	*pbpfp,		// play-by-play output file
	*stsfp,		// stats output file
	*cmdfp,		// commands output file
	*undofp;	// undo output file

FILE *output=stdout;	// where to direct output (default = stdout)
FILE *input=stdin;	// where to read in data
char *ops= "w+";

frame *f0;		// f0 is the current frame to be decoded

char filename[MAXPATHLEN];	// prefix for all output files

char *buffer;		// output buffer

team *ibl[2];		// pointers to the two teams
queue *runners;		// queue to handle inherited runners
player *onbase[4];	// array of pointers to batter & runners


int errflag =0;         // error flag; true if an error was committed 
                        // in the half inning

int undo     = 0, 	// if in the process of undo, set = 1
    cont     = 1,	// continue reading commands?
    outs     = 0, 	
    atbat    = 0, 
    inning   = 1,
    runs     = 0,
    linesize = 9;	// size of linescore array;

int **linescore;	// linescore array;



    void 
play()
{
    cont = 1;		// obviously we want to execute at least once!

    char tempstr[80];

    char cmdfile[MAXPATHLEN];
    strcpy(cmdfile,filename);
    strcat(cmdfile,".cmd");

    while (cont) {
#ifdef DEBUG
#if DEBUG == 2
	ibl[0]->box_score(stderr); 
	ibl[1]->box_score(stderr);
#endif
#endif
	fclose(cmdfp);
	fgets(tempstr,100,input);
	cmdfp=fopen(cmdfile,"a");

	// extra inning, grow the linescore
	if (inning > linesize) {
	    linescore[0] = (int *) realloc(linescore[0], inning * sizeof(int));
	    linescore[1] = (int *) realloc(linescore[1], inning * sizeof(int));
	    linescore[0][linesize] = 0;
	    linescore[1][linesize] = 0;
	    linesize = inning;
	}

	//if (output != stdout) fprintf(stderr,"%s",tempstr);
	f0=new frame(tempstr);

	if (!(feof(input)) && (strlen(tempstr) > 1) && (tempstr[0] != 'u')
		&& (f0->decode()))
	   fprintf(cmdfp,"%s",tempstr);

	switch ( f0->update() ) {
	    case 0:
		    tempstr[strlen(tempstr)-1] = '\0';
		    f0->help(tempstr);
	    case 1:
		    delete(f0);
		    break;
	}

#ifdef DEBUG
    runners->dump();
#endif
    }
}

    void 
quit()
{
    int x, y;
    int val;

    y = 16 + (3 * inning);


    fprintf(stsfp,"    ");
    for (x = 1; x <= inning; x++)
	fprintf(stsfp,"%3d",x);
    fprintf(stsfp,"     R  H  E\n");

    for (x = 1; x <= y; x++)
	fprintf(stsfp,"-");
    fprintf(stsfp,"\n");

    fprintf(stsfp,"%s ",ibl[0]);
    for (x = 0; x < inning; x++)
	fprintf(stsfp,"%3d",linescore[0][x]);
    fprintf(stsfp,"   %3d%3d%3d",ibl[0]->score,ibl[0]->team_hits(),
				 ibl[0]->errors);
    fprintf(stsfp,"\n");

    fprintf(stsfp,"%s ",ibl[1]);
    for (x = 0; x < inning; x++)
	fprintf(stsfp,"%3d",linescore[1][x]);
    fprintf(stsfp,"   %3d%3d%3d",ibl[1]->score,ibl[1]->team_hits(),
				 ibl[1]->errors);
    fprintf(stsfp,"\n");

    for (x = 1; x <= y; x++)
	fprintf(stsfp,"-");
    fprintf(stsfp,"\n");
    for (val=0; val<2; val++) {
	ibl[val]->box_score(stsfp);
	fprintf(stsfp,"LOB: %d\n",ibl[val]->lob);
	fprintf(stsfp,"E: ");
	ibl[val]->printstat(stsfp,0);
	fprintf(stsfp,"PB: ");
	ibl[val]->printstat(stsfp,1);
	fprintf(stsfp,"GIDP: ");
	ibl[val]->printstat(stsfp,2);
	fprintf(stsfp,"SH: ");
	ibl[val]->printstat(stsfp,3);
	fprintf(stsfp,"SF: ");
	ibl[val]->printstat(stsfp,4);
	fprintf(stsfp,"HBP: ");
	ibl[val]->printstat(stsfp,5);
	fprintf(stsfp,"WP: ");
	ibl[val]->printstat(stsfp,6);
	fprintf(stsfp,"IBB: ");
	ibl[val]->printstat(stsfp,7);
	fprintf(stsfp,"BALK: ");
	ibl[val]->printstat(stsfp,8);
    }
    fprintf(stsfp,"\n");
    fprintf(pbpfp,"\n");
}

    void 
setup()
{
    linescore = (int **) malloc (2 * sizeof(int));
    linescore[0] = (int *) calloc (linesize, sizeof(int));
    linescore[1] = (int *) calloc (linesize, sizeof(int));
	
    char tempstr[80];
	
    ibl[0]->make_lineups_pit();
    ibl[1]->make_lineups_pit();
	
    runners = new queue;

    fprintf(output,"\nEnter one line description of game conditions.\n");
    fgets(tempstr,100,input);
    fprintf(cmdfp,"%s",tempstr);
    fprintf(pbpfp,"grs version %s\n",VER);
    fprintf(pbpfp,"%s at %s \n",ibl[0]->nout(),ibl[1]->nout());
    fprintf(pbpfp,"%s\n",tempstr);
    fprintf(pbpfp,"Starting pitchers - %s for %s, and %s for %s\n",
	ibl[0]->mound->nout(),ibl[0]->nout(),ibl[1]->mound->nout(),ibl[1]->nout());

    f0=new frame(ibl[0],ibl[1],pbpfp);
    buffer=(char*)malloc(160);
    *buffer='\0';
    sprintf(tempstr,"\n%s %d: ",ibl[atbat]->nout(),inning);
    outbuf(pbpfp,tempstr);
}
 
    void 
setup(int tm)
{
    char tempstr[80];
	
    int i;
    int flag = 1;

    char nm[2][6];
    strcpy(nm[0],"away\0");
    strcpy(nm[1],"home\0");
 
    tempstr[0]='\0';

    while (flag) {
	fprintf(output,"Enter a 3 letter name for %s team:\n",nm[tm]);
	fgets(tempstr,100,input);
	if (strlen(tempstr) == 4) {
	    flag=0;
	    i = 0;
	    while (!(flag) && (i < 3)) {
		if (!(tempstr[i] >= 'A' && tempstr[i] <= 'Z')) {
		    fprintf(stderr,"Use all caps only\n");
		    if (input == stdin) flag=1; else exit(0);
		}
		i++;
	    }
	}
	else {
	    fprintf(stderr,"Team name must be 3 letters!\n");
	    if (input == stdin) flag=1; else exit(0);
	}
    }
    fprintf(cmdfp,"%s",tempstr);

    //if (ibl[tm]) delete(ibl[tm]);
    ibl[tm] =new team(tempstr);
    ibl[tm]->make_lineups();
}

    void 
outbuf(FILE *fp,char *str,char *punc)
{
    char tempstr[160];
    char *ptr = tempstr;
    int count=0;

    if (*punc == '\n') {
	strcat(buffer, punc);
	fputs(buffer,fp);
	fputs(str,fp);
	*buffer=(char)0;
    }
    else if (strlen(str)+strlen(buffer) > 75) {
	strcpy(tempstr, buffer);
	strcat(tempstr, punc);
	strcat(tempstr, str);

	strncpy(buffer, tempstr, 65);

	while (count != 65) { count++; ptr++; }
	while (!(isspace(*ptr)))
	    buffer[count++] = *(ptr++);

	buffer[count]='\0';
	ptr++;

	buffer=strcat(buffer,"\n");
	fputs(buffer,fp);

	strcpy(buffer, ptr);
    }
    else {
	strcat(buffer,punc);
	strcat(buffer,str);
    }
}

    int
openfile( char *prefix )
{
    char filename[MAXPATHLEN];
    int result;

    strncpy( filename, prefix, MAXPATHLEN - 4 );
    if ( ( pbpfp=fopen(strcat( filename, ".pbp"), ops) ) == NULL )
	result++;
    strncpy( filename, prefix, MAXPATHLEN - 4 );
    if ( ( stsfp=fopen(strcat( filename, ".sts"), ops) ) == NULL )
	result++;
    strncpy( filename, prefix, MAXPATHLEN - 4 );
    if ( ( cmdfp=fopen(strcat( filename, ".cmd"), ops) ) == NULL )
	result++;

    return (result);
}

    int
main(int argc, char *argv[])
{

    int c;
    char *afile = NULL;
    char *hfile = NULL;
    char *cfile = NULL;
    int usage = 0;

    while ((c = getopt(argc, argv, "a:h:f:v")) != EOF)
    switch (c) {
	case 'a':	afile = (char *) malloc(sizeof(MAXPATHLEN));
	    		strncpy( afile, optarg, MAXPATHLEN );
			break;
	case 'h':	hfile = (char *) malloc(sizeof(MAXPATHLEN));	
	    		strncpy( hfile, optarg, MAXPATHLEN );
			break;
	case 'f':	cfile = (char *) malloc(sizeof(MAXPATHLEN));	
	    		strncpy( cfile, optarg, MAXPATHLEN );
			break;
	case 'v':	fprintf(stderr,"%s\n",VER);
			exit(0);
	case '?':	usage++;
    }


    // can't specify -a|-h and -f
    if ( cfile && ( afile || hfile ) ) {
	usage++;
    }

    // no output filename
    if ( !(argv[optind]) ) {
	usage++;
    }
    else {
	strncpy( filename, argv[optind], MAXPATHLEN - 4 );
	if ( openfile(filename) ) {
	    fprintf( stderr, "cannot open %s\n", filename );
	    exit(1);
	}
    }

    if ( usage ) {
	fprintf(stderr,
	    "Usage: grs [ ([-a afile] [-h hfile]) or ([-f cmdfile]) ] outfile\n");
	exit(1);
    }

    if ( cfile ) {
	input = fopen(cfile,"r");
	output = fopen("/dev/null","w");
	if (input == NULL) {
	    fprintf( stderr, "cannot open %s\n", cfile );
	    exit(1);
	}
	setup(0);			// gets lineups for visitors
	setup(1);			// gets lineups for home team
	setup();			// sets up pbpfp, etc.
	play();
	fclose(input);
    }
    else {
	if ( afile ) {
	    if (input == NULL) {
		fprintf( stderr, "cannot open %s\n", afile );
		exit(1);
	    }
	}
	if ( hfile ) {
	    if (input == NULL) {
		fprintf( stderr, "cannot open %s\n", hfile );
		exit(1);
	    }
	}
	if ( afile ) {
	    input = fopen(afile,"r");
	    output = fopen("/dev/null","w");
	}
	setup(0);
	if (input != stdin) fclose(input);

	input = stdin;
	output = stdout;
	if ( hfile ) {
	    input = fopen(afile,"r");
	    output = fopen("/dev/null","w");
	}
	setup(1);
	if (input != stdin) fclose(input);

	input = stdin;
	output = stdout;
	setup();
    }

    input = stdin;
    output = stdout;
    play();
    quit();

    free(afile);
    free(hfile);
    free(cfile);

    fclose(pbpfp);
    fclose(stsfp);
    fclose(cmdfp);
}
