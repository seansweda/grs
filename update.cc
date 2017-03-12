#include "frame.h"
#include "queue.h"

    int
frame::update()
{
    int i, j;
    char spc;

    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));

    if ( strlen(location) == 0 ) {
	spc = '\0';
    }
    else {
	spc = ' ';
    }

#ifdef DEBUG
    fprintf( stderr, "ev=%s lc=%s br=%s\n",event,location,baserunning);
#endif

    if ( (input != stdin) && feof(input) ) {
	cont = 0;
	output = stdout;
	frameput();
	return(1);
    }

    if ( event[0] == '\0' ) {
	frameput();
	return(1);			// No event to decode, ignore.
    }

    if ( !(strcmp( event, "ph" )) ) {
	putcmd();
	ibl[atbat]->insert( bat->what_ord(onbase[0]), &comment, "ph" );
	onbase[0]=bat->up();
	runadv();
	frameput();
	snprintf( inputstr, MAX_INPUT, "%s ", comment );
	outbuf( pbpfp, inputstr );
	return(1);
    }
    else if ( !(strcmp( event, "pr" )) ) {
	ibl[atbat]->print_lineup();
	while ( inputstr[0] < '0' || inputstr[0] > '9' ) {
	    fprintf( output, "\nEnter batting order of player: " );
	    memset( inputstr, '\0', MAX_INPUT );
	    fgets( inputstr, MAX_INPUT, input );
	    sanitize( &inputstr, POSLEN, '\n' );
	    // fprintf(output,"\n");
	}
	for (i = 1; i < 4; i++ ) {
	    if ( onbase[i] == ibl[atbat]->findord(inputstr[0] - '0') ) {
		putcmd();
		ibl[atbat]->insert( (int) inputstr[0] - '0', &comment, "pr" );
		onbase[i] = ibl[atbat]->findord(inputstr[0] - '0');
		runadv();
		frameput();
		snprintf( inputstr, MAX_INPUT, "%s ", comment );
		outbuf( pbpfp, inputstr );
		return(1);
	    }
	}
	return(0);
    }
    else if ( !(strcmp( event, "np" )) ) {
	putcmd();
	int bats = pit->new_pit();
#ifdef DEBUG
	fprintf( stderr, "batting %d\n", bats );
#endif
	if ( (bats < 1) || (bats > 9) )
	    sprintf( comment, "%s now pitching for %s. ",
			pit->mound->nout(), pit->nout() );
	else {
	    sprintf( inputstr, "%s %s p", pit->mound->nout(), pit->mound->tout() );
	    pit->insert( bats, &comment, "", inputstr );
	}
	runadv();
	frameput();
	snprintf( inputstr, MAX_INPUT, "%s ", comment );
	outbuf( pbpfp, inputstr );
	return(1);
    }
    else if ( !(strcmp( event, "dr" )) || !(strcmp( event, "dc" )) ) {
	pit->print_lineup();
	while ( inputstr[0] < '0' || inputstr[0] > '9' ) {
	    fprintf( output, "\nEnter batting order of player: " );
	    memset( inputstr, '\0', MAX_INPUT );
	    fgets( inputstr, MAX_INPUT, input );
	    sanitize( &inputstr, POSLEN, '\n' );
	    // fprintf(output,"\n");
	}
	if ( inputstr[0] > '0' ) {
	    putcmd();
	    if ( !(strcmp( event, "dr" )) )
		pit->insert( ( inputstr[0] -'0' ), &comment );
	    else
		pit->pos_change( ( inputstr[0] - '0' ), &comment );
	}
	runadv();
	frameput();
	snprintf( inputstr, MAX_INPUT, "%s ", comment );
	outbuf( pbpfp, inputstr );
	return(1);
    }
    else if ( !(strcmp( event, "la" )) ) {
	ibl[0]->print_lineup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "lh" )) ) {
	ibl[1]->print_lineup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "un" )) ) {
	char in_ext[5], out_ext[5];
	int times = 1;

	if ( count == 1 ) {
	    count--;
	    snprintf( error, LINEWIDTH, "%s\n", "nothing to undo!" );
	    return(0);
	}
	else if ( undo ) {
	    cont = 0;
	    return(1);
	}
	else {
	    undo = 1;
	    fclose( pbpfp );
	    fclose( stsfp );
	    fclose( cmdfp );

	    cleanup();

	    snprintf( in_ext, 5, "%s", ".cmd" );
	    snprintf( out_ext, 5, ".un%d", times );

	    while ( !(backup( in_ext, out_ext )) ) {
		snprintf( in_ext, 5, "%s", out_ext );
		snprintf( out_ext, 5, ".un%d", ++times );
		fclose( cmdfp );
		fclose( undofp );

		if ( times > 9 ) {
		    fprintf( stderr, "undo may be looping, bailing!\n" );
		    exit(1);
		}
	    }

	    fclose( undofp );
	    fclose( cmdfp );
	    snprintf( inputstr, PATH_MAX, "%s%s", filename, out_ext );
	    undofp = fopen( inputstr, "r" );
	    openfile( filename );
	    output = fopen( NULLDEV , "w" );
	    input = undofp;
#ifdef DEBUG
	    fprintf( stderr, "setup 0\n" );
#endif
	    setup(0);
#ifdef DEBUG
	    fprintf( stderr, "setup 1\n" );
#endif
	    setup(1);
#ifdef DEBUG
	    fprintf( stderr, "setup 2\n" );
#endif
	    setup();
#ifdef DEBUG
	    fprintf( stderr, "reloading\n" );
#endif
	    play();
	    output = stdout;
	    input = stdin;
	    undo = 0;
	    cont = 1;
	    count--;
#ifdef DEBUG
	    fprintf( stderr, "%d %d %d %d %d %d %d %d\n",
		undo, cont, outs, atbat, inning, runs, linesize, errflag );
#endif
	    return(2);		// we already deleted the frame pointer
				// so return 2
	}
    }
    else if ( !(strcmp( event, "en" )) ) {
#ifdef DEBUG
	print_linescore(stderr);
#endif
#ifndef DEBUG
	if ( outs != 3 ) {
	    snprintf( error, LINEWIDTH, "Use \"eg\" to end inning with less than 2 outs.\n" );
	    return(0);
	}
#endif
	putcmd();
	if ( errflag && runs )
	    pit->unearned(inning);

	sprintf( inputstr, "%s %d %s %d\n", ibl[0]->nout(), ibl[0]->score,
					   ibl[1]->nout(), ibl[1]->score );
	outbuf( pbpfp, inputstr, "\n" );
	outbuf( pbpfp, "", "\n" );

	linescore[atbat][inning - 1] = runs;
	for ( i = 1; i < 4; i++ )
	    if ( onbase[i] )
		bat->lob++;

	runners->clear();

	atbat = (atbat + 1) % 2;
	outs = 0;
	runs = 0;
	errflag = 0;
	for ( i = 0; i < 4; i++ )
	    onbase[i] = NULL;
	if ( !(atbat) )
	    inning++;
	sprintf( inputstr, "%s %d: ", ibl[atbat]->nout(), inning );
	outbuf( pbpfp, inputstr );
	pit = bat;
	bat = ibl[atbat];
//	runadv();
	onbase[0] = bat->up();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "eg" )) ) {
#ifdef DEBUG
	print_linescore(stderr);
#endif
	putcmd();
	cont = 0;
	if ( errflag && runs )
	    pit->unearned(inning);

	sprintf( inputstr, "%s %d %s %d\n", ibl[0]->nout(),ibl[0]->score,
					   ibl[1]->nout(),ibl[1]->score );
	outbuf( pbpfp, inputstr, "\n");
	outbuf( pbpfp, "", "\n" );
	outbuf( pbpfp, " " );

	linescore[atbat][inning-1] = runs;
	for ( i = 1; i < 4; i++ )
	    if (onbase[i])
		bat->lob++;

	ibl[0]->decisions();
	ibl[1]->decisions();

	return(1);
    }
    else if ( !(strcmp( event, "cm" )) ) {
	putcmd();
	fprintf( output, "Input a one line comment\n" );
	memset( inputstr, '\0', MAX_INPUT );
	fgets( inputstr, MAX_INPUT, input );
	sanitize( &inputstr, MAX_INPUT, '\n' );
	snprintf( comment, MAX_INPUT, "%s", inputstr );
	fprintf( cmdfp, "%s", comment );
	frameput();
	comment[strlen(comment) - 1] = ' ';
	outbuf( pbpfp, comment );
	return(1);
    }
    else if ( !(strcmp( event, "so" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s K%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	pit->mound->k++;
	onbase[0]->k++;
	onbase[0]->ab++;
	onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "kd" ))) {
	runcat(1);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s dropped K%c%s", onbase[0]->nout(), spc, location);
	outbuf( pbpfp, inputstr );
	runstats(2);
	pit->mound->k++;
	onbase[0]->k++;
	onbase[0]->ab++;
	onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "bb" )) ) {
	runcat(-1);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s BB", onbase[0]->nout() );
	outbuf( pbpfp, inputstr );
	runstats();
	pit->mound->bb++;
	onbase[0]->bb++;
	onbase[0]->pa(pit->mound->throws);
	rbi();
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "iw" )) ) {
	runcat(-1);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s IBB", onbase[0]->nout() );
	outbuf( pbpfp, inputstr );
	runstats();
	bat->newstat( onbase[0]->nout(), 7 );
	pit->mound->bb++;
	onbase[0]->bb++;
	onbase[0]->pa(pit->mound->throws);
	rbi();
	runadv();
	batterup(0);	// iw doesn't count as BF
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "ci" )) ) {
	errflag = 1;
	runcat(-1);
	putcmd();
	if ( !(runchck(baserunning)) )
	    return(0);
	sprintf( inputstr, "%s CI", onbase[0]->nout() );
	outbuf( pbpfp, inputstr );
	runstats(2);
	pit->errors++;
	pit->newstat( pit->posout(2), 0 );
	onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "hp" )) || !(strcmp( event, "hb" )) ) {
	runcat(-1);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s HBP", onbase[0]->nout() );
	outbuf( pbpfp, inputstr );
	runstats();
	bat->newstat( onbase[0]->nout(), 5 );
	onbase[0]->pa(pit->mound->throws);
	rbi();
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "wp" )) ) {
	runcat(-2);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "WP" );
	outbuf( pbpfp, inputstr );
	runstats();
	pit->newstat( pit->mound->nout(), 6 );
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "pb" )) ) {
	errflag = 1;
	runcat(-2);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "PB" );
	outbuf( pbpfp, inputstr );
	runstats();
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "bk" )) ) {
	runcat(-2);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "Balk" );
	outbuf( pbpfp, inputstr );
	runstats();
	pit->newstat( pit->mound->nout(), 8 );
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "th" )) ) {
	if ( !(*baserunning) )
	    runcat(-2);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "On throw" );
	outbuf( pbpfp, inputstr );
	runstats();
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "di" )) ) {
	if ( !(*baserunning) )
	    runcat(-2);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "On defensive indifference" );
	outbuf( pbpfp, inputstr );
	runstats();
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "sb" )) ) {
	if ( !(*baserunning) )
	    runcat(-4);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "SB" );
	outbuf( pbpfp, inputstr );
	runstats();
	for ( i = 0; i < (int) strlen(baserunning); i += 2 )
	    if ( (( j = baserunning[i] - '0' ) > 0 ) && j < 4 )
		onbase[j]->sb++;
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "cs" )) ) {
	if ( !(*baserunning) )
	    runcat(-3);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "CS%c%s", spc, location);
	outbuf( pbpfp, inputstr );
	runstats();
	for ( i = 0; i < (int) strlen(baserunning); i += 2 )
	    if ( (( j = baserunning[i] - '0' ) > 0 ) && j < 4 && baserunning[i+1] == 'o' )
		onbase[j]->cs++;
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "kc" )) ) {
	if ( !(*baserunning) )
	    runcat(-3);
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s K, CS%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	for ( i = 0; i < (int) strlen(baserunning); i += 2 )
	    if ( (( j = baserunning[i] - '0' ) > 0 ) && j < 4 && baserunning[i+1] == 'o' )
		onbase[j]->cs++;
	pit->mound->k++;
	onbase[0]->k++;
	onbase[0]->ab++;
	onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "ks" )) ) {
	if ( !(*baserunning) )
	    runcat(-4);
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf(inputstr,"%s K, SB%c%s",onbase[0]->nout(),spc,location);
	outbuf( pbpfp, inputstr );
	runstats();
	pit->mound->k++;
	onbase[0]->k++;
	onbase[0]->ab++;
	onbase[0]->pa(pit->mound->throws);
	for ( i = 0; i < (int) strlen(baserunning); i += 2 )
	    if ( (( j = baserunning[i] - '0' ) > 0 ) && j < 4 )
		onbase[j]->sb++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "pk" )) ) {
	if ( !(*baserunning) )
	    runcat(-3);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "Pickoff%c%s", spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "oa" )) ) {
	if ( !(*baserunning) )
	    runcat(-3);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "Trying to advance%c%s", spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "ri" )) ) {
	if ( !(*baserunning) )
	    runcat(-3);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "On baserunner interference%c%s", spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "1b" )) ) {
	runcat(1);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s 1b%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	pit->mound->h++;
	onbase[0]->h++;
	onbase[0]->pa(pit->mound->throws);
	rbi();
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "2b" )) ) {
	runcat(2);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s 2b%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	pit->mound->h++;
	onbase[0]->h++;
	onbase[0]->b2++;
	onbase[0]->pa(pit->mound->throws);
	rbi();
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "3b" )) ) {
	runcat(3);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s 3b%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	pit->mound->h++;
	onbase[0]->h++;
	onbase[0]->b3++;
	onbase[0]->pa(pit->mound->throws);
	rbi();
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "hr" )) ) {
	runcat(4);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s HR%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	pit->mound->h++;
	pit->mound->hr++;
	onbase[0]->h++;
	onbase[0]->hr++;
	onbase[0]->pa(pit->mound->throws);
	rbi();
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "tp" )) ) {
	if ( !(*baserunning) )
	    runcat( "2o1o" );
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s triple play%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "gd" )) || !(strcmp( event, "dp" )) ) {
	if ( !(*baserunning) )
	    runcat( "1o" );
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s GDP%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	bat->newstat( onbase[0]->nout(), 2 );
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "fd" )) ) {
	if ( !(*baserunning) )
	    runcat( "1o" );
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s FDP%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "ld" )) ) {
	if ( !(*baserunning) )
	    runcat( "1o" );
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s LDP%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "lo" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s line drive%c%s", onbase[0]->nout(), spc, location);
	outbuf( pbpfp, inputstr );
	runstats();
	rbi();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "fc" )) ) {
	if ( !(*baserunning) )
	    runcat( "b11o" );
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s FC%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats(1);
	rbi();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "hg" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s HG%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	rbi();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "rg" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s RG%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	rbi();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "go" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s ground out%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	rbi();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "sg" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s SG%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	rbi();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "hf" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s HF%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	rbi();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "po" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s PO%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	rbi();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "fp") ) || !(strcmp( event, "pf" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s foul PO%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	rbi();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "sf" )) ) {
	runcat( "3h" );
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s SF%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	bat->newstat( onbase[0]->nout(), 4 );
	rbi();
	onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "sh" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s Sac%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats(2);
	bat->newstat( onbase[0]->nout(), 3 );
	rbi();
	onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "lf" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s LF%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "df" )) || !(strcmp( event, "wt" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s WT%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "fo" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	sprintf( inputstr, "%s fly out%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "er" )) ) {
	errflag = 1;
	runcat(1);
	if ( !(runchck(baserunning)) )
	    return(0);
	if ( strlen(location) > 0 ) {
	    if ( location[0] < '1' || location[0] > '9' ) {
		sprintf( error, "\"%s\" invalid.  First character must be fielder's position (1-9).", location );
		return(0);
	    }
	}
	else {
	    sprintf( error, "Must supply fielder's position (1-9)." );
	    return(0);
	}
	putcmd();
	sprintf( inputstr, "%s safe on E%c%s", onbase[0]->nout(), spc, location );
	outbuf( pbpfp, inputstr );
	runstats(2);
	pit->errors++;
	who_stat(0, (int) location[0] - '0');
	onbase[0]->pa(pit->mound->throws);
	onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return(1);
    }
    else if (!(strcmp(event,"ea"))) {
	errflag = 1;
	if ( !(*baserunning) )
	    runcat(-2);
	if ( !(runchck(baserunning)) )
	    return(0);
	if ( strlen(location) > 0 ) {
	    if ( location[0] < '1' || location[0] > '9' ) {
		sprintf( error, "\"%s\" invalid.  First character must be fielder's position (1-9).", location );
		return(0);
	    }
	}
	else {
	    sprintf( error, "Must supply fielder's position (1-9)." );
	    return(0);
	}
	putcmd();
	sprintf( inputstr, "Error%c%s", spc, location );
	outbuf( pbpfp, inputstr );
	runstats();
	pit->errors++;
	who_stat(0, (int) location[0] - '0');
	runadv();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "nj" )) ) {
	i = atoi(location);
	if ( i < 1 || i > 3 ) {
	    if ( onbase[3] )
		i = 3;
	    else if ( onbase[2] )
		i = 2;
	    else if ( onbase[1] )
		i = 1;
	    else
		i = 0;
	}
	if ( !i || !(onbase[i]) ) {
	    sprintf( error, "No runner at \"%s\"\n", location );
	    return(0);
	}
	putcmd();
	sprintf( inputstr, "%s can't get jump. ", onbase[i]->nout() );
	outbuf( pbpfp, inputstr );
	frameput();
	return(1);
    }
    else if ( !(strcmp(event,"fa")) ) {
	putcmd();
	sprintf( inputstr, "%s fatigues. ", pit->mound->nout() );
	outbuf( pbpfp, inputstr );
	frameput();
	return(1);
    }
    else if ( !(strcmp(event,"ic")) ) {
	putcmd();
	sprintf( inputstr, "Infield in @ 1b/3b. " );
	outbuf( pbpfp, inputstr );
	frameput();
	return(1);
    }
    else if ( !(strcmp(event,"in")) ) {
	if ( strlen(location) > 0 )
	    sprintf( inputstr, "Infield in @ %s. ", location );
	else
	    sprintf( inputstr, "Infield in. " );
	putcmd();
	outbuf( pbpfp, inputstr );
	frameput();
	return(1);
    }
    else
	return(0);
}

    int
frame::backup( char *in_ext, char *out_ext )
{
    int result;
    size_t cmdlen;
    frame *test;

    char *filestr, *currstr, *nextstr;
    filestr = (char*) calloc(PATH_MAX, sizeof(char));
    currstr = (char*) calloc(MAX_INPUT, sizeof(char));
    nextstr = (char*) calloc(MAX_INPUT, sizeof(char));

    snprintf( filestr, PATH_MAX, "%s%s", filename, out_ext );
    if ( ( undofp = fopen(filestr,"w") ) == NULL ) {
	fprintf( stderr, "fatal error - can't open undo file\n" );
	exit(1);
    }
#ifdef DEBUG_UNDO
    fprintf( stderr, "backup(%s):\n", filestr );
#endif
    snprintf( filestr, PATH_MAX, "%s%s", filename, in_ext );
    cmdfp = fopen(filestr,"r");

    memset( currstr, '\0', MAX_INPUT );
    fgets( currstr, MAX_INPUT, cmdfp );
    sanitize( &currstr, MAX_INPUT, '\n' );

    memset( nextstr, '\0', MAX_INPUT );
    fgets( nextstr, MAX_INPUT, cmdfp );
    sanitize( &nextstr, MAX_INPUT, '\n' );

    while ( !(feof(cmdfp)) ) {
	cmdlen = strlen(currstr);
#ifdef DEBUG_UNDO
	fprintf( stderr, "1: (%d) %s2: %s",
		(int)cmdlen, currstr, nextstr);
#endif
	fprintf( undofp, "%s\n", currstr );
	snprintf( currstr, MAX_INPUT, "%s", nextstr );

	memset( nextstr, '\0', MAX_INPUT );
	fgets( nextstr, MAX_INPUT, cmdfp );
	sanitize( &nextstr, MAX_INPUT, '\n' );
    }

    // on dc/dr skip decode, fail early
    if ( cmdlen == 2 ) {
	return(0);
    }
    test = new frame(currstr);
    result = test->decode();
#ifdef DEBUG_UNDO
    fprintf( stderr, "decode(%s) = %d\n", stripcr(currstr, MAX_INPUT), result );
#endif
    delete(test);
    return(result);
}

    void
frame::putcmd()
{
    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));
    memset( inputstr, '\0', MAX_INPUT );

    sprintf( inputstr, "%s", event );
    if ( strlen(location) > 0 ) {
	sprintf( inputstr, "%s %s", inputstr, location );
    }
    if ( strlen(baserunning) > 0 && strcmp( location, baserunning) ) {
	sprintf( inputstr, "%s %s", inputstr, baserunning );
    }

    fprintf( cmdfp, "%s\n", inputstr );
}

