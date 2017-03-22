#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef WIN32
#   include "getopt/getopt.h"
#else
#   include <unistd.h>
#endif
#ifdef USE_GIT
#   include "git_commit.h"
#endif
#include "config.h"
#include "pitcher.h"
#include "player.h"
#include "team.h"
#include "frame.h"
#include "list.h"
#include "version.h"

// variables used outside of main()

FILE	*pbpfp,		// play-by-play output file
	*stsfp,		// stats output file
	*cmdfp,		// commands output file
	*undofp;	// undo output file

FILE *output = stdout;	// where to direct output (default = stdout)
FILE *input = stdin;	// where to read in data

list* cmd = new list;	// in-memory .cmd file

char filename[PATH_MAX];	// prefix for all output files
team *ibl[2];			// pointers to the two teams

// instantiate static objects
int frame::cont,
    frame::outs,
    frame::atbat,
    frame::inning,
    frame::runs,
    frame::linesize,
    frame::errflag,
    frame::count;

int** frame::linescore;
queue* frame::runners;		// queue to handle inherited runners
player* frame::onbase[4];	// array of pointers to batter & runners

char* frame::buffer;		// output buffer

    char*
sanitize( char** word, int len, char delimiter )
{
    int c = 1;
    char *ptr = *word;

    while ( *ptr != '\n' && *ptr != '\0' && c < len ) {
	if ( *ptr == delimiter ) { break; }
	ptr++;
	c++;
    }

    *ptr = '\0';

    return( *word );
}

    int
openfile( char *prefix )
{
    int result = 0;
    char file[PATH_MAX];

    snprintf( file, PATH_MAX, "%s.%s", prefix, "pbp" );
    if ( ( pbpfp=fopen( file, "w+" ) ) == NULL )
	result++;
    snprintf( file, PATH_MAX, "%s.%s", prefix, "sts" );
    if ( ( stsfp=fopen( file, "w+" ) ) == NULL )
	result++;
    snprintf( file, PATH_MAX, "%s.%s", prefix, "cmd" );
    file[PATH_MAX - 4] = '\0';
    if ( ( cmdfp=fopen( file, "w+" ) ) == NULL )
	result++;

    return (result);
}

    int
checkfile( char *prefix )
{
    struct stat sb;
    int result = 0;
    char file[PATH_MAX];

    snprintf( file, PATH_MAX, "%s.%s", prefix, "pbp" );
    if ( stat( file, &sb ) == 0 )
	result++;
    snprintf( file, PATH_MAX, "%s.%s", prefix, "sts" );
    if ( stat( file, &sb ) == 0 )
	result++;
    snprintf( file, PATH_MAX, "%s.%s", prefix, "cmd" );
    if ( stat( file, &sb ) == 0 )
	result++;

    return (result);
}

    void
setup( int tm )
{
    ibl[tm] = new team(tm);
    ibl[tm]->make_lineups();
}

    void
setup()
{
    frame *f0;		// f0 is the current frame to be decoded

    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));

    ibl[0]->make_lineups_pit();
    ibl[1]->make_lineups_pit();

    fprintf(output, "Enter one line description of game conditions.\n");
    memset( inputstr, '\0', MAX_INPUT );
    fgets( inputstr, MAX_INPUT, input );
    sanitize( &inputstr, MAX_INPUT, '\n' );
    fprintf(output, "\n");
    cmd->add( inputstr );
    fprintf(cmdfp, "%s\n", inputstr);
    fflush( cmdfp );
    fprintf(pbpfp, "grs version %s", VER);
#ifdef GIT
    fprintf(pbpfp, " (%s)", GIT);
#endif
    fprintf(pbpfp, "\n");
    fprintf(pbpfp, "%s at %s \n", ibl[0]->nout(), ibl[1]->nout());
    fprintf(pbpfp, "%s\n", inputstr);
    fprintf(pbpfp, "Starting pitchers - %s for %s, and %s for %s\n",
	ibl[0]->mound->nout(), ibl[0]->nout(), ibl[1]->mound->nout(),
	ibl[1]->nout());

    f0 = new frame(ibl[0], ibl[1]);

    snprintf(inputstr, MAX_INPUT, "\n%s %d: ",
	    ibl[frame::atbat]->nout(), frame::inning);
    f0->outbuf( inputstr );

    delete(f0);
}

    void
play()
{
    frame *f0;			// f0 is the current frame to be decoded
    frame::cont = 1;		// obviously we want to execute at least once!

    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));

    while (frame::cont) {
#ifdef DEBUG
#if DEBUG == 2
	ibl[0]->box_score(stderr);
	ibl[1]->box_score(stderr);
#endif
#endif
	memset( inputstr, '\0', MAX_INPUT );
	fgets( inputstr, MAX_INPUT, input );
	sanitize( &inputstr, MAX_INPUT, '\n' );

	f0 = new frame( inputstr );

#ifdef DEBUG
	if (output != stdout)
	    fprintf( stderr, "play:(%d) %s", f0->count, inputstr );
#endif

	switch ( f0->decode() ) {
	    case 1:
		switch ( f0->update() ) {
		    case 0:
			f0->help(inputstr);
		    default:
			break;
		}
		break;
	    default:
		f0->help(inputstr);
		break;
	}
	delete(f0);

#ifdef DEBUG
    frame::runners->dump();
    cmd->dump();
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

    int
main(int argc, char *argv[])
{

    int c;
    char *afile = NULL;
    char *hfile = NULL;
    char *cfile = NULL;
    int usage = 0;

    int overwrite = 0;
    while ((c = getopt(argc, argv, "a:h:f:vo")) != EOF)
    switch (c) {
	case 'a':	afile = (char *) calloc(PATH_MAX, sizeof(char));
			snprintf( afile, PATH_MAX, "%s", optarg );
			break;
	case 'h':	hfile = (char *) calloc(PATH_MAX, sizeof(char));
			snprintf( hfile, PATH_MAX, "%s", optarg );
			break;
	case 'f':	cfile = (char *) calloc(PATH_MAX, sizeof(char));
			snprintf( cfile, PATH_MAX, "%s", optarg );
			break;
	case 'o':	overwrite = 1;
			break;
	case 'v':	fprintf(stderr,"%s",VER);
#ifdef GIT
			fprintf(stderr, " (%s)", GIT);
#endif
			fprintf(stderr, "\n");
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
	snprintf( filename, PATH_MAX - 5, "%s", argv[optind] );

	// don't overwrite existing files
	if ( !overwrite && checkfile(filename) ) {
	    fprintf( stderr, "%s.* file(s) already exist\n", filename );
	    exit(1);
	}

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

    fclose( pbpfp );
    fclose( stsfp );
    fclose( cmdfp );

    exit(0);
}

