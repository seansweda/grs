#include "frame.h"

// Constructor which initializes the static fields
frame::frame(team *away, team *home, FILE *fp)
{
    undo = 0; 		// if in the process of undo, set = 1
    cont = 1;		// continue reading commands?
    outs = 0; 	
    atbat = 0; 
    inning = 1;
    runs = 0;
    linesize = 9;	// size of linescore array;
    errflag = 0;

    buffer=(char*) calloc(MAX_INPUT * 2, sizeof(char));
    *buffer='\0';

    linescore = (int **) malloc (2 * sizeof(int));
    linescore[0] = (int *) calloc ( (size_t)linesize, sizeof(int) );
    linescore[1] = (int *) calloc ( (size_t)linesize, sizeof(int) );

    bat = away;
    pit = home;

    pbpfp = fp;

    event = NULL;
    location = NULL;
    baserunning = NULL;
    comment = NULL;
    error = NULL;

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

    // extra inning, grow the linescore
    if ( inning > linesize ) {
	linescore[0] = (int *) realloc(linescore[0], (size_t)(inning * sizeof(int)));
	linescore[1] = (int *) realloc(linescore[1], (size_t)(inning * sizeof(int)));
	linescore[0][linesize] = 0;
	linescore[1][linesize] = 0;
	linesize = inning;
    }

    sscanf( str, "%s %s %s", event, location, baserunning );
    strcpy( event, stripcr( event, MAX_INPUT ) );
    strcpy( location, stripcr( location, MAX_INPUT ) );
    strcpy( baserunning, stripcr( baserunning, MAX_INPUT ) );

    cont=1;
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
	    runner = bat->up(); 
	    i=0;
	}
	else 
	    runner = onbase[ i = (int)( *str - '0' ) ];

	if ( runner ) {				// If runner is onbase
	    j = (int)( *(++str) - '0' );  
	    switch ( *str ) {
		case 'o' :			// If runner made an out
		    outs = outs + 1;
		    if ( newbase[i] == runner && i )
			newbase[i] = NULL;
		    break;
		case 'h' :			// If runner scored a run
		    if ( newbase[i] == runner && i )
			newbase[i] = NULL;
		    break;
		case '1' :			// If runner (batter) went to 1st
		case '2' :			// If runner went to second
		case '3' :			// If runner went to third
		    newbase[j] = runner;
		    if ( newbase[i] == runner && i )
			newbase[i] = NULL;
		    break;
		default :
		    // if we get here, runchck didn't do its job!
		    sprintf( error, "Fatal baserunning error: %s\n", baserunning );
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
			sprintf( tempstr, "%s out", onbase[i]->nout() );
			outbuf( pbpfp, tempstr, ", " );
		    }
		    else if ( i > 0 ) {
			sprintf( tempstr, "%s out", onbase[i]->nout() );
			outbuf( pbpfp, tempstr, ", " );
		    }
		    break;
		case 'h' :		// If runner scored a run
		    run_charged_to = runners->dequeue();
		    run_charged_to->r++;
		    run_charged_to->er++;
		    onbase[i]->r++;
		    bat->score++;
		    runs++;
		    sprintf( tempstr, "%s scores", onbase[i]->nout() );
		    outbuf( pbpfp, tempstr, ", " );
		    break;
		case '1' :		// If runner (batter) went to 1st
		case '2' :		// If runner went to second
		case '3' :		// If runner went to third
		    if ( (i == 0) && (fc > 0) ) {
			sprintf( tempstr, "%s %s", onbase[i]->nout(), b[j] );
			outbuf( pbpfp, tempstr, ", " );
		    }
		    else if ( i > 0 ) {
			sprintf( tempstr, "%s %s", onbase[i]->nout(), b[j] );
			outbuf( pbpfp, tempstr, ", " );
		    }
		   break;
		default : ;
	    }
	}
	str++;
    }
	
    outbuf( pbpfp, "", ". " );

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
frame::runchck(char *runstr)
// Checks baserunning, removes double occurances.
{
    char *str, *temptr, *errptr;
    int copy = 1, retval = 1;
    int done[4];

    for ( int j=0; j<=3; j++ )
       done[j] = 0;

    char temp[MAX_INPUT];
    char errstr[MAX_INPUT];

    temptr = temp;
    *temptr = '\0';
    errptr = errstr;
    *errptr = '\0';

#ifdef DEBUG
    fprintf( stderr, "pre runchck: %s\n", runstr );
#endif
    str = runstr;

    while ( *str ) {
	switch ( *str ) {
	    case 'b': 
		copy = 1;
		if ( !done[0] ) {
		    switch ( *(str+1) ) {
			case 'o':
			case 'h':
			case '1':
			case '2':
			case '3': 
			    break;
			default: 
			    copy = 0; 
			    retval = 0;
			    break;
		    }
		    if ( copy ) {
			done[0] = 1;
			strncat( temp, str, 2 );
			temptr += 2;
			*temptr = '\0';
		    }
		    else {
			strncat( errstr, str, 2 );
			errptr += 2;
			*errptr = '\0';
		    }
		}
		break;
	    case '1':
		copy = 1;
		if ( !done[1] ) {
		    switch ( *(str+1) ) {
			case '1':
			    copy = 0;
			    done[1] = 1;
			    break;
			case 'o':
			case 'h':
			case '2':
			case '3': 
			    break;
			default: 
			    copy = 0; 
			    retval = 0;
			    break;
		    }
		    if ( !(onbase[1]) ) {
			copy = 0;
			retval = 0;
		    }
		    if ( copy ) {
			done[1] = 1;
			strncat( temp, str, 2 );
			temptr += 2;
			*temptr = '\0';
		    }
		    else if ( !done[1] ) {
			strncat( errstr, str, 2 );
			errptr += 2;
			*errptr = '\0';
		    }
		}
		break;
	    case '2':	
		copy = 1;
		if ( !done[2] ) {
		    switch ( *(str+1) ) {
			case '2':
			    copy = 0;
			    done[2] = 1;
			    break;
			case 'o':
			case 'h':
			case '3': 
			    break;
			default: 
			    copy = 0; 
			    retval = 0;
			    break; 
		    }
		    if ( !(onbase[2]) ) { 
			copy = 0;
			retval = 0;
		    }
		    if ( copy ) {
			done[2] = 1;
			strncat( temp, str, 2 );
			temptr += 2;
			*temptr = '\0';
		    }
		    else if ( !done[2] ) {
			strncat( errstr, str, 2 );
			errptr += 2;
			*errptr = '\0';
		    }
		}
		break;
	    case '3':
		copy = 1;
	        if ( !done[3] ) {
		    switch ( *(str+1) ) {
			case '3':
			    copy = 0;
			    done[3] = 1;
			    break;
			case 'o':
			case 'h': 
			    break;
			default: 
			    copy = 0; 
			    retval = 0;
			    break; 
		    }
		    if ( !(onbase[3]) ) { 
			copy = 0; 
			retval = 0;
		    }
		    if ( copy ) {
			done[3] = 1;
			strncat( temp, str, 2 );
			temptr += 2;
			*temptr = '\0';
		    }
		    else if ( !done[3] ) {
			strncat( errstr, str, 2 );
			errptr += 2;
			*errptr = '\0';
		    }
		}
		break;
	    default: 
		retval = 0;
		strncat( errstr, str, 2 );
		errptr += 2;
		*errptr = '\0';
		break;
	}
	str+=2;
    }
	    
    if (retval)
	strcpy( runstr, temp );		// cleaned up baserunning string

	if ( outsonplay(runstr) + outs > 3 ) {
	    sprintf( error, "Too many outs on play.\n" );
	    retval = 0;
	}
    else
	sprintf( error, "Baserunning error: %s\n", errstr );

#ifdef DEBUG
    fprintf( stderr, "post runchck: %s\n", runstr );
#endif
    return(retval);
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
*/
{
    int test = 0;
    int i;
    char temp[MAX_INPUT];
    char *temptr;

    temptr=temp;
    *temptr='\0';


    switch (adv) {
    case 0 : 					// batter out
	    strcat(baserunning,"bo");
	    break;
	    
    case -1 :					// advance if forced
	    if (!onbase[1])
	       strcat(baserunning,"b1");
	    else if (!onbase[2])
	       strcat(baserunning,"b112");
	    else if (!onbase[3])
	       strcat(baserunning,"b11223");
	    else
	       strcat(baserunning,"b112233h");
	    break;
    //else {

    case -2 :					// runners advance 1 base
	    for (i=1;i<=3;i++)
	       if (onbase[i]) {
		    sprintf(temptr++,"%1d",i);
		    test=i+1;
		    if (test >= 4)
		       *temptr++='h';
		    else
		       sprintf(temptr++,"%1d",test);}
	    break;

    case -3 :
    //	strcat(baserunning,"1o");	
	    for (i=1;i<=3;i++)
	       if (onbase[i]) test=i;
	    sprintf(temptr++,"%1d",test);
	    sprintf(temptr++,"o");
	    break;

    case -4 :
	    for (i=1;i<=3;i++)
	       if (onbase[i]) test=i;
	    sprintf(temptr++,"%1d",test);
	    if (++test == 4)
	       sprintf(temptr++,"h");
	    else if (test > 0)
	       sprintf(temptr++,"%1d",test);
	    break;

    default :
	    *temptr++='b';
	    if (adv == 4)
	       *temptr++='h';
	    else
	       sprintf(temptr++,"%1d",adv);

	    for (i=1;i<=3;i++)
	       if (onbase[i]) {
		    sprintf(temptr++,"%1d",i);
		    test=i+adv;
		    if (test >= 4)
		       *temptr++='h';
		    else
		       sprintf(temptr++,"%1d",test);}
    } // switch

    *temptr='\0';
    strcat(baserunning,temp);
}

    int 
frame::three()
{
    if (baserunning[0] == '\0') {
	sprintf(error,"%s must have baserunning data.",event);
	return(0);
    }
    else
	return(1);
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
frame::decode()
{
#ifdef DEBUG
	fprintf(stderr,"decode: %s %s %s\n", event, location, baserunning);
#endif
    if (event[0] == '\0') {
	return(1); 			// No event to decode, ignore.
    }

    if (runchck(location)) {		// Baserunning in location field?
	strcpy( baserunning, location );
	memset( location, '\0', MAX_INPUT );
    }
    else {
	memset( error, '\0', LINEWIDTH );	// clean up after runchck
    }
	    
    // these events are legal with three outs
    if ( !(strcmp(event,"lh")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"la")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"un")) ) { 
	return(1);
    }
    else if ( !(strcmp(event,"en")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"eg")) ) {
	return(1); 
    }
    else if ( !(strcmp(event,"cm")) ) { 
	return(1); 
    }

    // the rest are not
    if ( outs == 3 && !undo ) {
	sprintf( error, "End of inning (3 outs).  Invalid command: %s\n", event );
	return(0);
    }
    else if ( !(strcmp(event,"ph")) ) {
	return(1);  
    }
    else if ( !(strcmp(event,"pr")) ) { 
	return(1);
    }
    else if ( !(strcmp(event,"np")) ) { 
	return(1); 
    }
    else if ( !(strcmp(event,"dr") ) || !(strcmp(event,"dc")) ) { 
	return(1); 
    }
    else if ( !(strcmp(event,"so")) ) {
	return(1); 
    }
    else if ( !(strcmp(event,"kd")) ) { 
	return(1);
    }
    else if ( !(strcmp(event,"bb")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"iw")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"ci")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"hp")) || !(strcmp(event,"hb")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"wp")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"pb")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"bk")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"sb")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"th")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"di")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"cs")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"kc")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"ks")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"pk")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"oa")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"ri")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"1b")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"2b")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"3b")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"hr")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"tp")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"gd")) || !(strcmp(event,"dp")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"fd")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"ld")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"lo")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"fc")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"hg")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"rg")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"go")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"sg")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"hf")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"po")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"fp")) || !(strcmp(event,"pf")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"sf")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"sh")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"lf")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"df")) || !(strcmp(event,"wt")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"fo")) ) {
        return(1);
    }
    else if ( !(strcmp(event,"er")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"ea")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"nj")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"fa")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"in")) ) {
	return(1);
    }
    else if ( !(strcmp(event,"ic")) ) {
	return(1);
    }
    else { 
	sprintf( error, "Invalid command: %s\n", event );
	return(0);
    }
}

    void 
frame::outbuf( FILE *fp, const char *str, const char *punc )
{
    int count;
    char tempstr[MAX_INPUT * 2];
    char *ptr = tempstr;

    if ( *punc == '\n' ) {
	strncat( buffer, punc, 2 );
	fputs( buffer, fp );
	fputs( str, fp );
	*buffer = (char) 0;
    }
    else if ( strlen(str) + strlen(buffer) > LINEWIDTH ) {
	strncpy(tempstr, buffer, MAX_INPUT);
	strncat(tempstr, punc, 2);
	strncat(tempstr, str, MAX_INPUT);

	while ( strlen(tempstr) > LINEWIDTH ) {
	    count = 0;
	    strncpy( buffer, tempstr, LINEWIDTH - 10 );

	    while ( count != (LINEWIDTH - 10) ) { 
		count++; 
		ptr++; 
	    }
	    while ( !(isspace(*ptr)) )
		buffer[count++] = *(ptr++);

	    buffer[count] = '\0';
	    ptr++;

	    buffer = strcat( buffer, "\n" );
	    fputs( buffer, fp );
	    strncpy( tempstr, ptr, MAX_INPUT * 2 );
	}
	strncpy( buffer, tempstr, MAX_INPUT );
    }
    else {
	strncat( buffer, punc, 2 );
	strncat( buffer, str, MAX_INPUT );
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
    }
}
