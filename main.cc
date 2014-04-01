#define VER "3.1_beta"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "pitcher.h"
#include "player.h"
#include "team.h"
#include "frame.h"

// variables used outside of main()

FILE	*pbpfp,		// play-by-play output file
	*stsfp,		// stats output file
	*cmdfp,		// commands output file
	*undofp;	// undo output file

FILE *output = stdout;	// where to direct output (default = stdout)
FILE *input = stdin;	// where to read in data

char filename[PATH_MAX];	// prefix for all output files
team *ibl[2];			// pointers to the two teams

// instantiate static objects
int frame::undo,
    frame::cont,
    frame::outs,
    frame::atbat,
    frame::inning,
    frame::runs,
    frame::linesize,
    frame::errflag;

int** frame::linescore;
queue* frame::runners;		// queue to handle inherited runners
player* frame::onbase[4];	// array of pointers to batter & runners

char* frame::buffer;		// output buffer

    void 
play()
{
    frame *f0;			// f0 is the current frame to be decoded
    frame::cont = 1;		// obviously we want to execute at least once!

    char tempstr[MAX_INPUT];
    memset( tempstr, '\0', MAX_INPUT );

    char cmdfile[PATH_MAX];
    strncpy(cmdfile, filename, PATH_MAX - 5);
    cmdfile[PATH_MAX - 4] = '\0';
    strcat(cmdfile,".cmd");

    while (frame::cont) {
#ifdef DEBUG
#if DEBUG == 2
	ibl[0]->box_score(stderr); 
	ibl[1]->box_score(stderr);
#endif
#endif
	fclose(cmdfp);
	fgets(tempstr, MAX_INPUT, input);
	cmdfp = fopen(cmdfile,"a");

#ifdef DEBUG
	if (output != stdout) 
	    fprintf( stderr, "%s", tempstr );
#endif
	f0 = new frame( tempstr );

	switch ( f0->decode() ) {
	    case 1:
		switch ( f0->update() ) {
		    case 0:
			tempstr[strlen(tempstr) - 1] = '\0';
			f0->help(tempstr);
		    case 1:
			delete(f0);
			break;
		}
		break;
	    default:
		tempstr[strlen(tempstr) - 1] = '\0';
		f0->help(tempstr);
		delete(f0);
		break;
	}

#ifdef DEBUG
    frame::runners->dump();
    fprintf( stderr, "\n" );
#endif
    }
}

    void 
quit()
{
    frame::print_linescore(stsfp);

    for (int val=0; val<2; val++) {
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
    frame *f0;		// f0 is the current frame to be decoded

    char tempstr[MAX_INPUT];
    memset( tempstr, '\0', MAX_INPUT );
	
    ibl[0]->make_lineups_pit();
    ibl[1]->make_lineups_pit();

    fprintf(output, "Enter one line description of game conditions.\n");
    fgets(tempstr, MAX_INPUT, input);
    fprintf(output, "\n");
    fprintf(cmdfp, "%s", tempstr);
    fprintf(pbpfp, "grs version %s\n", VER);
    fprintf(pbpfp, "%s at %s \n", ibl[0]->nout(), ibl[1]->nout());
    fprintf(pbpfp, "%s\n", tempstr);
    fprintf(pbpfp, "Starting pitchers - %s for %s, and %s for %s\n",
	ibl[0]->mound->nout(), ibl[0]->nout(), ibl[1]->mound->nout(),
	ibl[1]->nout());

    f0 = new frame(ibl[0], ibl[1], pbpfp);

    sprintf(tempstr, "\n%s %d: ", ibl[frame::atbat]->nout(), frame::inning);
    f0->outbuf(pbpfp, tempstr);

    delete(f0);
}
 
    void 
setup( int tm )
{
    ibl[tm] = new team(tm);
    ibl[tm]->make_lineups();
}

    int
openfile( char *prefix )
{
    char filename[PATH_MAX];
    int result = 0;

    strncpy( filename, prefix, PATH_MAX - 5 );
    filename[PATH_MAX - 4] = '\0';
    if ( ( pbpfp=fopen(strcat( filename, ".pbp"), "w+" ) ) == NULL )
	result++;
    strncpy( filename, prefix, PATH_MAX - 5 );
    filename[PATH_MAX - 4] = '\0';
    if ( ( stsfp=fopen(strcat( filename, ".sts"), "w+" ) ) == NULL )
	result++;
    strncpy( filename, prefix, PATH_MAX - 5 );
    filename[PATH_MAX - 4] = '\0';
    if ( ( cmdfp=fopen(strcat( filename, ".cmd"), "w+" ) ) == NULL )
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
	case 'a':	afile = (char *) calloc(PATH_MAX, sizeof(char));
	    		strncpy( afile, optarg, PATH_MAX );
			break;
	case 'h':	hfile = (char *) calloc(PATH_MAX, sizeof(char));	
	    		strncpy( hfile, optarg, PATH_MAX );
			break;
	case 'f':	cfile = (char *) calloc(PATH_MAX, sizeof(char));	
	    		strncpy( cfile, optarg, PATH_MAX );
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
	strncpy( filename, argv[optind], PATH_MAX - 5 );
	filename[PATH_MAX - 4] = '\0';
	if ( openfile(filename) ) {
	    fprintf( stderr, "cannot open %s.*\n", filename );
	    exit(1);
	}
    }

    if ( usage ) {
	fprintf(stderr,
	    "Usage: grs [ (-a afile) (-h hfile) | (-f cmdfile) ] outfile\n");
	exit(1);
    }

    if ( cfile ) {
	input = fopen( cfile, "r" );
	output = fopen( NULLDEV , "w" );
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
	    input = fopen( afile, "r" );
	    output = fopen( NULLDEV, "w" );
	    if (input == NULL) {
		fprintf( stderr, "cannot open %s\n", afile );
		exit(1);
	    }
	}
	setup(0);
	if (input != stdin) fclose(input);

	input = stdin;
	output = stdout;
	if ( hfile ) {
	    input = fopen( hfile, "r" );
	    output = fopen( NULLDEV, "w" );
	    if (input == NULL) {
		fprintf( stderr, "cannot open %s\n", hfile );
		exit(1);
	    }
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

    char*
stripcr( char* word, int len )
{
    int c = 1;
    char *ptr = word;

    while ( *ptr != '\n' && *ptr != '\0' && c < len ) {
	ptr++;
	c++;
    }

    *ptr = '\0';

    return( word );
}

