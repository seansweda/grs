#include "frame.h"
#include "commands.h"
#include "list.h"
#include "extern.h"

// Constructor which initializes the static fields
frame::frame(team *away, team *home)
{
    cont = 1;		// continue reading commands?
    outs = 0;
    atbat = 0;
    inning = 1;
    runs = 0;
    linesize = 9;	// size of linescore array;
    errflag = 0;
    count = 0;

    buffer=(char*) calloc(MAX_INPUT * 2, sizeof(char));
    *buffer='\0';

    linescore = (int **) malloc (2 * sizeof(int));
    linescore[0] = (int *) calloc ( (size_t)linesize, sizeof(int) );
    linescore[1] = (int *) calloc ( (size_t)linesize, sizeof(int) );

    bat = away;
    pit = home;

    event = NULL;
    location = NULL;
    baserunning = NULL;
    comment = NULL;
    error = NULL;
    outputstr = NULL;

    runners = new queue;

    for ( int i=0; i<4; i++ )
	onbase[i]=NULL;
    onbase[0]=ibl[atbat]->up();

    frameput();
}

// Constructor call in all other cases

frame::frame(char *str)
{
    event = (char*) calloc(MAX_INPUT, sizeof(char));
    location = (char*) calloc(MAX_INPUT, sizeof(char));
    baserunning = (char*) calloc(MAX_INPUT, sizeof(char));
    comment = (char*) calloc(MAX_INPUT, sizeof(char));
    error = (char*) calloc(LINEWIDTH, sizeof(char));
    outputstr = (char*) calloc(MAX_INPUT, sizeof(char));

    // extra inning, grow the linescore
    if ( inning > linesize ) {
	linescore[0] = (int *) realloc(linescore[0], (size_t)(inning * sizeof(int)));
	linescore[1] = (int *) realloc(linescore[1], (size_t)(inning * sizeof(int)));
	linescore[0][linesize] = 0;
	linescore[1][linesize] = 0;
	linesize = inning;
    }

    sscanf( str, "%s %s %s", event, location, baserunning );
    sanitize( &event, MAX_INPUT );
    sanitize( &location, MAX_INPUT );
    sanitize( &baserunning, MAX_INPUT );

    cont = 1;
    bat=ibl[atbat];
    pit=ibl[(atbat+1)%2];

}

    int
frame::runadv()
// update the onbase array based on baserunning
{
    int i, j;
    char *str;
    player *runner;
    player *newbase[4];

    for ( i=0; i<4; i++ )
	newbase[i] = onbase[i];

    str=baserunning;

    while ( *str ) {
	if ( *str == 'b' ) {
	    i = 0;
	    runner = bat->up();
	}
	else {
	    i = (int)( *str - '0' );
	    runner = onbase[i];
	}

	if ( runner ) {				// If runner is onbase
	    j = (int)( *(++str) - '0' );
	    switch ( *str ) {
		case 'o' :			// If runner made an out
		    outs = outs + 1;
		    if ( newbase[i] == runner && i && i != j )
			newbase[i] = NULL;
		    break;
		case 'h' :			// If runner scored a run
		    if ( newbase[i] == runner && i && i != j )
			newbase[i] = NULL;
		    break;
		case '1' :			// If runner went to first
		case '2' :			// If runner went to second
		case '3' :			// If runner went to third
		    newbase[j] = runner;
		    if ( newbase[i] == runner && i && i != j )
			newbase[i] = NULL;
		    break;
		default :
		    // if we get here, runcheck didn't do its job!
		    snprintf( error, LINEWIDTH, "Fatal baserunning error: %s\n", baserunning );
		    exit(1);
	    }
	}
	else {
	    return(0);
	}

	str++;
    }

    for ( i=0; i<4; i++ )
	onbase[i] = newbase[i];
    return(1);
}

// runstats(1) takes care of a fielders choice, (it keeps the ownership
// of the baserunner for the previous pitcher for inherited runs to work
// runstats(2) is for any play where it is not obvious what happened
// to the batter, i.e. it outputs in .pbp the result for the batter

    void
frame::runstats( int fc )
{
    int i, j;
    const char *(b[4]);
    char tempstr[MAX_INPUT];
    char *str;

    pitcher *run_charged_to;

    b[1] = "to first";
    b[2] = "to second";
    b[3] = "to third";

    str = baserunning;
    while ( *str ) {
	if ( *str == 'b' ) {
	    i = 0;
	    if ( (*(str+1) != 'o') && (fc != 1) )
		    runners->add(pit->mound);
	}
	else
	    i = (int) (*str - '0');

	if ( onbase[i] ) {		// If runner is onbase
	    str++;
	    j = (int) *str - '0';
	    switch ( *str ) {
		case 'o' :		// If runner made an out
		    pit->mound->out++;
		    if ( (i == 0) && (fc > 0) ) {
			snprintf( tempstr, MAX_INPUT, "%s out", onbase[i]->nout() );
			outbuf( tempstr, ", " );
		    }
		    else if ( i > 0 ) {
			snprintf( tempstr, MAX_INPUT, "%s out", onbase[i]->nout() );
			outbuf( tempstr, ", " );
		    }
		    break;
		case 'h' :		// If runner scored a run
		    run_charged_to = runners->dequeue();
		    run_charged_to->r++;
		    run_charged_to->er++;
		    onbase[i]->r++;
		    bat->score++;
		    runs++;
		    snprintf( tempstr, MAX_INPUT, "%s scores", onbase[i]->nout() );
		    outbuf( tempstr, ", " );
		    break;
		case '1' :		// If runner (batter) went to 1st
		case '2' :		// If runner went to second
		case '3' :		// If runner went to third
		    if ( (i == 0) && (fc > 0) ) {
			snprintf( tempstr, MAX_INPUT, "%s %s", onbase[i]->nout(), b[j] );
			outbuf( tempstr, ", " );
		    }
		    else if ( i > 0 ) {
			snprintf( tempstr, MAX_INPUT, "%s %s", onbase[i]->nout(), b[j] );
			outbuf( tempstr, ", " );
		    }
		   break;
		default : ;
	    }
	}
	str++;
    }

    outbuf( "", ". " );

    // at this point we need fc binary
    if ( fc != 1 ) {
	fc = 0;
    }

    // if there were baserunning outs, dequeue runners
    for ( i = 0; i < (outsonplay(baserunning) - batterout(baserunning) - fc); i++ )
	runners->dequeue();

    // if this is a fc play where nobody is out, we need to queue a pitcher
    if ( (fc == 1) && (outsonplay(baserunning) == 0) ) {
	runners->add(pit->mound);
    }

    return;
}

    void
frame::frameput()
{
    fprintf( output, "Pit: %-15s ", pit->mound->nout() );
    fprintf( output, "IP: %2d.%d  BF: %2d   ",
	    pit->mound->out / 3, pit->mound->out % 3, pit->mound->bf );
    fprintf( output, "%s: %2d  %s: %2d   ",
		ibl[0]->nout(), ibl[0]->score,
		ibl[1]->nout(), ibl[1]->score );
    fprintf( output, "In: %s %d  Outs: %d\n",
		atbat == 0 ? "Top" : "Bot", inning, outs);
    fprintf( output, "Bat: %-15s ", onbase[0]->nout() );
    for ( int i=1; i<4; i++ )
	fprintf( output, "%1d: %-15s ", i,
		onbase[i] ? onbase[i]->nout() : "    ");
    fprintf( output, "\n" );
    ibl[ atbat == 0 ? 1 : 0 ]->check_defense();
}

    void
frame::rbi()
{
    char *str;

    str=baserunning;
    while (*str) {
	if (*str=='h')
		onbase[0]->rbi++;
	str++;
    }
}

    void
frame::batterup( int bf )
{
    // call with bf=0 for events that should not count as BF
    if ( bf ) {
	pit->mound->bf++;
    }
    bat->next_up();
    onbase[0]=bat->up();
}

    void
frame::help(char *str)
{
    if ( *error == '\0' ) {
	fprintf( output, "Invalid input string: %s\n", str );
    }
    else {
	fprintf( output, "%s\n", error );
    }

    //if (input != stdin) exit(0);

    frameput();
}

    int
frame::base( const char c )
{
    switch( c ) {
	case 'o':
	    return 0;
	case 'h':
	    return 4;
	case '1':
	    return 1;
	case '2':
	    return 2;
	case '3':
	    return 3;
	default:
	    return 0;
    }
}

    int
frame::runcheck(char *runstr, int copy)
// Checks baserunning, removes double occurances (first one wins).
// copy 0 = just check
// copy 1 = rewrite (default)
{
    char *str, *temptr, *errptr;

    // -1       = not done
    //  0       = out
    //  1,2,3,4 = base advanced
    int dest[4] = { -1, -1, -1, -1 };

    char temp[MAX_INPUT];
    memset( temp, '\0', MAX_INPUT );
    char errstr[MAX_INPUT];
    memset( errstr, '\0', MAX_INPUT );

    temptr = temp;
    errptr = errstr;

#ifdef DEBUG
    fprintf( stderr, "pre runcheck: %s\n", runstr );
#endif
    str = runstr;

    while ( *str ) {
	switch ( *str ) {
	    case 'b':
		if ( dest[0] == -1 ) {
		    switch ( *(str+1) ) {
			case 'o':
			case 'h':
			case '1':
			case '2':
			case '3':
			    dest[0] = base( *(str+1) );
			    break;
			default:
			    snprintf( errptr, 3, "%s", str );
			    errptr += 2;
			    break;
		    }
		    if ( dest[0] >= 0 ) {
			snprintf( temptr, 3, "%s", str );
			temptr += 2;
		    }
		}
		break;
	    case '1':
		if ( !(onbase[1]) ) {
		    snprintf( errptr, 3, "%s", str );
		    errptr += 2;
		}
		else if ( dest[1] == -1 ) {
		    switch ( *(str+1) ) {
			case 'o':
			case 'h':
			case '1':
			case '2':
			case '3':
			    dest[1] = base( *(str+1) );
			    break;
			default:
			    snprintf( errptr, 3, "%s", str );
			    errptr += 2;
			    break;
		    }
		    if ( dest[1] >= 0 ) {
			snprintf( temptr, 3, "%s", str );
			temptr += 2;
		    }
		}
		break;
	    case '2':
		if ( !(onbase[2]) ) {
		    snprintf( errptr, 3, "%s", str );
		    errptr += 2;
		}
		else if ( dest[2] == -1 ) {
		    switch ( *(str+1) ) {
			case 'o':
			case 'h':
			case '2':
			case '3':
			    dest[2] = base( *(str+1) );
			    break;
			default:
			    snprintf( errptr, 3, "%s", str );
			    errptr += 2;
			    break;
		    }
		    if ( dest[2] >= 0 ) {
			snprintf( temptr, 3, "%s", str );
			temptr += 2;
		    }
		}
		break;
	    case '3':
		if ( !(onbase[3]) ) {
		    snprintf( errptr, 3, "%s", str );
		    errptr += 3;
		}
		else if ( dest[3] == -1 ) {
		    switch ( *(str+1) ) {
			case 'o':
			case 'h':
			case '3':
			    dest[3] = base( *(str+1) );
			    break;
			default:
			    snprintf( errptr, 3, "%s", str );
			    errptr += 2;
			    break;
		    }
		    if ( dest[3] >= 0 ) {
			snprintf( temptr, 3, "%s", str );
			temptr += 2;
		    }
		}
		break;
	    default:
		snprintf( errptr, 3, "%s", str );
		errptr += 2;
		break;
	}
	str += 2;
    }

    if ( outsonplay( temp ) + outs > 3 ) {
	snprintf( errstr, MAX_INPUT, "%s [too many outs on play]", runstr );
    }

#ifdef DEBUG
    fprintf( stderr, "post runcheck: %s (%s)\n", temp, errstr );
#endif

    if ( strlen( errstr ) == 0 ) {
	if ( copy == 1 ) {
	    // copy cleaned up baserunning string
	    snprintf( runstr, MAX_INPUT, "%s", temp );
	}
	return( 1 );
    }
    else {
	snprintf( error, LINEWIDTH, "Baserunning error: %s\n", errstr );
	return( 0 );
    }
}

    void
frame::runcat(int adv)
/*
    runcat(-4) lead runner advances 1 base
    runcat(-3) lead runner out
    runcat(-2) everyone but the batter advances 1 base (wp, pb, etc.)
    runcat(-1) advance 1 base if forced only (bb, hb, etc.)
    runcat(0) batter is out
    runcat(x) everyone advances x base(s)
    runcat(string) append string
*/
{
    int i;
    int test = 0;
    size_t b = strlen(baserunning);

    char temp[MAX_INPUT];
    char *temptr = temp;
    *temptr='\0';

    switch (adv) {
    case 0 :					// batter out
	    snprintf( baserunning + b, 3, "%s", "bo" );
	    return;

    case -1 :					// advance if forced
	    if (!onbase[1])
		snprintf( baserunning + b, 3, "%s", "b1" );
	    else if (!onbase[2])
		snprintf( baserunning + b, 5, "%s", "b112" );
	    else if (!onbase[3])
		snprintf( baserunning + b, 7, "%s", "b11223" );
	    else
		snprintf( baserunning + b, 9, "%s", "b112233h" );
	    return;

    case -2 :					// runners advance 1 base
	    for (i=1;i<=3;i++)
		if (onbase[i]) {
		    snprintf( temptr++, 2, "%1d", i );
		    test=i+1;
		    if (test >= 4)
			snprintf( temptr++, 2, "h" );
		    else
			snprintf( temptr++, 2, "%1d", test );
		}
	    break;

    case -3 :
	    for (i=1;i<=3;i++)
		if (onbase[i]) test=i;
	    snprintf( temptr++, 2, "%1d", test );
	    snprintf( temptr++, 2, "o" );
	    break;

    case -4 :
	    for (i=1;i<=3;i++)
		if (onbase[i]) test=i;
	    snprintf( temptr++, 2, "%1d", test );
	    if (++test == 4)
		snprintf( temptr++, 2, "h" );
	    else if (test > 0)
		snprintf( temptr++, 2, "%1d", test );
	    break;

    default :
	    snprintf( temptr++, 2, "b" );
	    if (adv == 4)
		snprintf( temptr++, 2, "h" );
	    else
		snprintf( temptr++, 2, "%1d", adv );
	    for (i=1;i<=3;i++)
		if (onbase[i]) {
		    snprintf( temptr++, 2, "%1d", i );
		    test=i+adv;
		    if (test >= 4)
			snprintf( temptr++, 2, "h" );
		    else
			snprintf( temptr++, 2, "%1d", test );
	       }
    } // switch

    *temptr='\0';
    snprintf( baserunning + b, MAX_INPUT, "%s", temp);
}

    void
frame::runcat( const char *str )
// append string to baserunning
{
    size_t b = strlen(baserunning);
    snprintf( baserunning + b, MAX_INPUT, "%s", str );
}

    void
frame::who_stat( int stat, int who )
// adds stat to player at fielding position "who"
{
#ifdef DEBUG
   fprintf( stderr, "who_stat: %d\n", who );
#endif
   switch (who) {
	case 0 :
	    pit->newstat( (char*)NULL, stat );
	    break;
	case 1 :
	    pit->newstat( pit->mound->nout(), stat );
	    break;
	default:
#ifdef DEBUG
	   fprintf( stderr, "who_stat: newstat \"%s\", %d\n", pit->posout(who), stat );
#endif
	    pit->newstat( pit->posout(who), stat );
   }
}

    void
frame::cleanup()
{
	delete(runners);

	delete(ibl[0]);
	ibl[0] = 0;
	delete(ibl[1]);
	ibl[1] = 0;

	free(linescore);
	free(buffer);
}

    int
frame::validate( const char *str )
{
    // return 0 if invalid
    // return 1 if valid
    // return 3 if valid anytime (i.e. w/3 outs)

    int i, tot;

    tot = sizeof valid / sizeof *valid;
    for( i = 0; i != tot; i++ )
	if ( strcmp( str, valid[i] ) == 0 )
	    return( 1 );

    tot = sizeof valid3 / sizeof *valid3;
    for( i = 0; i != tot; i++ )
	if ( strcmp( str, valid3[i] ) == 0 )
	    return( 3 );

    if ( strlen( str ) == 0 )
	return ( 3 );
    else
	return( 0 );
}

    int
frame::decode()
{
#ifdef DEBUG
	fprintf(stderr,"decode: %s %s %s\n", event, location, baserunning);
#endif

    int result;

    // Baserunning in location field?
    if ( runcheck( location, 0 ) ) {
	snprintf( baserunning, MAX_INPUT, "%s", location );
	memset( location, '\0', MAX_INPUT );
    }
    /*
    else {
	memset( error, '\0', LINEWIDTH );// clean up after runcheck
    }
    */

    result = validate( event );

    if ( result == 3 || (result == 1 && outs != 3) ) {
	return( 1 );
    }
    else if ( result == 1 && outs == 3 ) {
	snprintf( error, LINEWIDTH, "End of inning (3 outs).  Invalid command: %s\n", event );
	return( 0 );
    }
    else {
	snprintf( error, LINEWIDTH, "Invalid command: %s\n", event );
	return( 0 );
    }
}

    void
frame::outbuf( const char *str, const char *punc )
{
    int c;
    char bigbuf[MAX_INPUT * 2];
    char tempstr[MAX_INPUT * 2];
    char *ptr;

    if ( *punc == '\n' ) {
	if ( strlen( buffer ) > 0 ) {
	    snprintf( buffer + strlen(buffer), 2, "%s", punc );
	    fputs( buffer, pbpfp );
	}
	fputs( str, pbpfp );
	memset( buffer, '\0', MAX_INPUT * 2 );
    }
    else if ( *punc == '.' && strlen(buffer) > LINEWIDTH - 10 ) {
	snprintf( tempstr, MAX_INPUT * 2, "%s%c\n", buffer, punc[0] );
	fputs( tempstr, pbpfp );
	snprintf( buffer, MAX_INPUT * 2, "%s", str );
    }
    else {
	snprintf( bigbuf, MAX_INPUT * 2, "%s%s%s", buffer, punc, str );

	while ( strlen( bigbuf ) > LINEWIDTH - 5 ) {
	    snprintf( tempstr, MAX_INPUT * 2, "%s", bigbuf );

	    c = LINEWIDTH - 15;
	    ptr = tempstr + c;
	    while ( !( isspace(*ptr) ) ) {
		c++;
		ptr++;
	    }

	    ptr++;
	    tempstr[c] = '\0';

	    snprintf( buffer, MAX_INPUT * 2, "%s\n", tempstr );
	    fputs( buffer, pbpfp );
	    snprintf( bigbuf, MAX_INPUT * 2, "%s", ptr );
	}
	snprintf( buffer, MAX_INPUT * 2, "%s", bigbuf);
    }
}

    void
frame::print_linescore(FILE *fp)
{
    int x, y;

    y = 16 + (3 * inning);

    fprintf(fp,"    ");
    for (x = 1; x <= inning; x++)
	fprintf(fp,"%3d",x);
    fprintf(fp,"     R  H  E\n");

    for (x = 1; x <= y; x++)
	fprintf(fp,"-");
    fprintf(fp,"\n");

    fprintf(fp,"%s ",ibl[0]->nout());
    for (x = 0; x < inning; x++)
	fprintf(fp,"%3d",linescore[0][x]);
    fprintf(fp,"   %3d%3d%3d",ibl[0]->score,ibl[0]->team_hits(),
				 ibl[0]->errors);
    fprintf(fp,"\n");

    fprintf(fp,"%s ",ibl[1]->nout());
    for (x = 0; x < inning; x++)
	fprintf(fp,"%3d",linescore[1][x]);
    fprintf(fp,"   %3d%3d%3d",ibl[1]->score,ibl[1]->team_hits(),
				 ibl[1]->errors);
    fprintf(fp,"\n");

    for (x = 1; x <= y; x++)
	fprintf(fp,"-");
    fprintf(fp,"\n");
}

    int
frame::outsonplay( char *brun )
{
    int num = 0;
    char *ptr;

    ptr = brun;
    while ( *ptr != '\0' ) {
	if ( *ptr++ == 'o' ) {
	    num++;
	}
    }

#ifdef DEBUG
    fprintf( stderr, "oop: %d batter: %d\n", num, batterout( brun ) );
#endif

    return(num);
}

    int
frame::batterout( char *brun )
{
    if ( strstr( brun, "bo" ) != 0 ) {
	return(1);
    }
    else {
	return(0);
    }
}

    int
frame::get_spot()
{
    int val, loop;
    char *spot;
    spot = (char*) calloc(MAX_INPUT, sizeof(char));

    loop = 1;
    while ( loop ) {
	fprintf( output, "Enter batting order of player: " );
	memset( spot, '\0', MAX_INPUT );
	fgets( spot, MAX_INPUT, input );
	sanitize( &spot, POSLEN );
	val = spot[0] - '0';
	if ( strlen( spot ) == 1 && val >= 0 && val <= 9 )
	    break;
    }

    free( spot );
    return( val );
}

    void
frame::putcmd()
{
    size_t o;
    char *output;
    output = (char*) calloc(MAX_INPUT, sizeof(char));
    memset( output, '\0', MAX_INPUT );

    snprintf( output, MAX_INPUT, "%s", event );
    o = strlen( output );
    if ( strlen(location) > 0 ) {
	snprintf( output + o, MAX_INPUT - o, " %s", location );
	o = strlen( output );
    }
    if ( strlen(baserunning) > 0 && strcmp( location, baserunning) ) {
	snprintf( output + o, MAX_INPUT - o, " %s", baserunning );
    }

    cmd->add( output );
    fprintf( cmdfp, "%s\n", output );
    fflush( cmdfp );
    // increment count when we write to cmd file
    count++;
    free( output );
}

frame::~frame()
{
    if ( event ) {
#ifdef DEBUG
	fprintf(stderr,"~frame: %s %s %s\n", event, location, baserunning);
#endif
	free(event);
	free(location);
	free(baserunning);
	free(comment);
	free(error);
	free(outputstr);
    }
}
