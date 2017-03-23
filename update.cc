#include "frame.h"
#include "queue.h"
#include "extern.h"

    int
frame::update()
{
    int i, j;
    char spc;

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
	output = stdout;
	frameput();
	return(2);
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
	snprintf( outputstr, MAX_INPUT, "%s ", comment );
	outbuf( outputstr );
	return(1);
    }
    else if ( !(strcmp( event, "pr" )) ) {
	ibl[atbat]->print_lineup();
	int spot = get_spot();
	for (i = 1; i < 4; i++ ) {
	    if ( onbase[i] == ibl[atbat]->findord( spot ) ) {
		putcmd();
		ibl[atbat]->insert( spot, &comment, "pr" );
		onbase[i] = ibl[atbat]->findord( spot );
		runadv();
		frameput();
		snprintf( outputstr, MAX_INPUT, "%s ", comment );
		outbuf( outputstr );
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
	    snprintf( comment, MAX_INPUT, "%s now pitching for %s.",
		    pit->mound->nout(), pit->nout() );
	else {
	    snprintf( outputstr, MAX_INPUT, "%s %s p",
		    pit->mound->nout(), pit->mound->tout() );
	    pit->insert( bats, &comment, "", outputstr );
	}
	runadv();
	frameput();
	snprintf( outputstr, MAX_INPUT, "%s ", comment );
	outbuf( outputstr );
	return(1);
    }
    else if ( !(strcmp( event, "dr" )) || !(strcmp( event, "dc" )) ) {
	pit->print_lineup();
	int spot = get_spot();
	if ( spot > 0 ) {
	    putcmd();
	    if ( !(strcmp( event, "dr" )) )
		pit->insert( spot, &comment );
	    else
		pit->pos_change( spot, &comment );
	}
	runadv();
	frameput();
	snprintf( outputstr, MAX_INPUT, "%s ", comment );
	outbuf( outputstr );
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
    else if ( !(strcmp( event, "eg" )) ) {
#ifdef DEBUG
	print_linescore(stderr);
#endif
	putcmd();
	if ( errflag && runs )
	    pit->unearned(inning);

	snprintf( outputstr, MAX_INPUT, "%s %d %s %d\n",
		ibl[0]->nout(),ibl[0]->score, ibl[1]->nout(),ibl[1]->score );
	outbuf( outputstr, "\n");
	outbuf( "\n" );

	linescore[atbat][inning-1] = runs;
	for ( i = 1; i < 4; i++ )
	    if (onbase[i])
		bat->lob++;

	ibl[0]->decisions();
	ibl[1]->decisions();

	// game over
	cont = 0;
	return(1);
    }
    else if ( !(strcmp( event, "cm" )) ) {
	putcmd();
	fprintf( output, "Input a one line comment\n" );
	memset( outputstr, '\0', MAX_INPUT );
	fgets( outputstr, MAX_INPUT, input );
	sanitize( &outputstr, MAX_INPUT, '\n' );
	snprintf( comment, MAX_INPUT, "%s", outputstr );
	cmd->add( comment );
	fprintf( cmdfp, "%s\n", comment );
	fflush( cmdfp );
	frameput();
	outbuf( comment );
	if ( comment[strlen(outputstr) - 1] == '.' ) {
	    outbuf( "", " ");
	} else {
	    outbuf( "", ". ");
	}
	return(1);
    }
    else if ( !(strcmp( event, "so" )) ) {
	runcat(0);
	if ( !(runchck(baserunning)) )
	    return(0);
	putcmd();
	snprintf( outputstr, MAX_INPUT, "%s K%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s dropped K%c%s", onbase[0]->nout(), spc, location);
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s BB", onbase[0]->nout() );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s IBB", onbase[0]->nout() );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s CI", onbase[0]->nout() );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s HBP", onbase[0]->nout() );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "WP" );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "PB" );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "Balk" );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "On throw" );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "On defensive indifference" );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "SB" );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "CS%c%s", spc, location);
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s K, CS%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s K, SB%c%s", onbase[0]->nout(),spc,location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "Pickoff%c%s", spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "Trying to advance%c%s", spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "On baserunner interference%c%s", spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s 1b%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s 2b%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s 3b%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s HR%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s triple play%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s GDP%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s FDP%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s LDP%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s line drive%c%s", onbase[0]->nout(), spc, location);
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s FC%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s HG%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s RG%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s ground out%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s SG%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s HF%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s PO%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s foul PO%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s SF%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s Sac%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s LF%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s WT%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
	snprintf( outputstr, MAX_INPUT, "%s fly out%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
		snprintf( error, LINEWIDTH, "\"%s\" invalid.  First character must be fielder's position (1-9).", location );
		return(0);
	    }
	}
	else {
	    snprintf( error, LINEWIDTH, "Must supply fielder's position (1-9)." );
	    return(0);
	}
	putcmd();
	snprintf( outputstr, MAX_INPUT, "%s safe on E%c%s", onbase[0]->nout(), spc, location );
	outbuf( outputstr );
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
		snprintf( error, LINEWIDTH, "\"%s\" invalid.  First character must be fielder's position (1-9).", location );
		return(0);
	    }
	}
	else {
	    snprintf( error, LINEWIDTH, "Must supply fielder's position (1-9)." );
	    return(0);
	}
	putcmd();
	snprintf( outputstr, MAX_INPUT, "Error%c%s", spc, location );
	outbuf( outputstr );
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
	    snprintf( error, LINEWIDTH, "No runner at \"%s\"\n", location );
	    return(0);
	}
	putcmd();
	snprintf( outputstr, MAX_INPUT, "%s can't get jump. ", onbase[i]->nout() );
	outbuf( outputstr );
	frameput();
	return(1);
    }
    else if ( !(strcmp(event,"fa")) ) {
	putcmd();
	snprintf( outputstr, MAX_INPUT, "%s fatigues. ", pit->mound->nout() );
	outbuf( outputstr );
	frameput();
	return(1);
    }
    else if ( !(strcmp(event,"ic")) ) {
	putcmd();
	snprintf( outputstr, MAX_INPUT, "Infield in @ 1b/3b. " );
	outbuf( outputstr );
	frameput();
	return(1);
    }
    else if ( !(strcmp(event,"in")) ) {
	if ( strlen(location) > 0 )
	    snprintf( outputstr, MAX_INPUT, "Infield in @ %s. ", location );
	else
	    snprintf( outputstr, MAX_INPUT, "Infield in. " );
	putcmd();
	outbuf( outputstr );
	frameput();
	return(1);
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

	snprintf( outputstr, MAX_INPUT, "%s %d %s %d\n",
		ibl[0]->nout(), ibl[0]->score, ibl[1]->nout(), ibl[1]->score );
	outbuf( outputstr, "\n" );
	outbuf( "\n" );

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
	snprintf( outputstr, MAX_INPUT, "%s %d: ", ibl[atbat]->nout(), inning );
	outbuf( outputstr );
	pit = bat;
	bat = ibl[atbat];
//	runadv();
	onbase[0] = bat->up();
	frameput();
	return(1);
    }
    else if ( !(strcmp( event, "un" )) ) {
	if ( count == 0 ) {
	    snprintf( error, LINEWIDTH, "%s\n", "nothing to undo!" );
	    return( 0 );
	}

	fclose( pbpfp );
	fclose( stsfp );
	fclose( cmdfp );

	cmd->end();
	snprintf( outputstr, MAX_INPUT, "%s", cmd->peek() );
	sanitize( &outputstr, MAX_INPUT );
	cmd->pop();
	count--;
	while ( validate( outputstr ) == 0 ||
		// catch 2-line cmds
		strcmp( cmd->peek(), "ph" ) == 0 ||
		strcmp( cmd->peek(), "np" ) == 0 ||
		strcmp( cmd->peek(), "cm" ) == 0 ||
		// catch 3-line cmds (pr, dr, dc)
		strlen( cmd->peek() ) == 1 ) {
	    snprintf( outputstr, MAX_INPUT, "%s", cmd->peek() );
	    sanitize( &outputstr, MAX_INPUT );
	    cmd->pop();
	    count--;
	}

	// diag
	// cmd->dump();

	list *old = cmd;
	cmd = new list;
	cleanup();

	// write undo file
	snprintf( outputstr, PATH_MAX, "%s.un1", filename );
	if ( (undofp = fopen( outputstr, "w+" )) == NULL ) {
	    fprintf( stderr, "fatal error: could write %s\n", outputstr );
	    exit( 1 );
	}
	old->start();
	while ( strlen( old->peek() ) > 0 ) {
	    fprintf( undofp, "%s\n", old->peek() );
	    old->next();
	}
	fclose( undofp );
	delete( old );

	// read from undo file
	if ( (undofp = fopen( outputstr, "r" )) == NULL ) {
	    fprintf( stderr, "fatal error: could read %s\n", outputstr );
	    exit( 1 );
	}
	openfile( filename );
	input = undofp;
	output = fopen( NULLDEV , "w" );

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
#ifdef DEBUG
	fprintf( stderr, "%d %d %d %d %d %d %d %d\n",
	    count, cont, outs, atbat, inning, runs, linesize, errflag );
#endif
	return(1);
    }
    // final iteration of if(strcmp) block, no match
    else
	return(0);
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

