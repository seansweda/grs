#include "team.h"
#include "extern.h"

// Constructor functions
team::team( int who )
{
    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));

    int i;
    int loop = 1;
    while ( loop ) {
	fprintf(output, "Enter the three letter code for %s team:\n",
		who == 0 ? "away" : "home" );

	memset( inputstr, '\0', MAX_INPUT );
	fgets( inputstr, MAX_INPUT, input );
	sanitize( &inputstr, TEAMLEN );

	if ( strlen(inputstr) != 3 ) {
	    fprintf(stderr, "Team name must be 3 letters!\n");
	    if (input != stdin)
		exit(1);
	    else
		continue;
	} else {
	    for ( i=0; i< 3; i++ ) {
		if ( isalpha( inputstr[i] ) ) {
		    if ( islower( inputstr[i] ) )
			// convert to uppercase
			inputstr[i] = (char)toupper( inputstr[i] );
		}
		else {
		    loop++;
		}
	    }
	    if ( loop > 1 ) {
		loop = 1;
		fprintf(stderr, "Team name must be alphanumeric\n");
		if (input != stdin)
		    exit(1);
		else
		    continue;
	    }
	}
	// if we get here it's all good
	break;
    }

    snprintf( ibl, TEAMLEN, "%s", inputstr );
    cmd->add( ibl );
    fprintf( cmdfp, "%s\n", ibl );
    fflush( cmdfp );
    lineup = (struct pl_list *) malloc(sizeof (struct pl_list));
    lineup->head = NULL;
    lineup->next = NULL;
    lineup->ord = 0;
    current = NULL;
    pitchers = (struct pit_list *) malloc(sizeof (struct pit_list));
    score = 0;
    errors = 0;
    lob = 0;

    for ( i = 0; i <= 8; i++ ) {
	extra_stats[i].ord = 0;
	extra_stats[i].name[0] = '\n';
	extra_stats[i].next = NULL;
    }
    free( inputstr );
}

// Constructor function getting team name; Assumes name is correct
team::team( char *str )
{
    snprintf( ibl, TEAMLEN, "%s", str );
    cmd->add( ibl );
    fprintf( cmdfp, "%s\n", ibl );
    fflush( cmdfp );
    lineup = (struct pl_list *) malloc(sizeof (struct pl_list));
    lineup->head = NULL;
    lineup->next = NULL;
    lineup->ord = 0;
    current = NULL;
    pitchers = (struct pit_list *) malloc(sizeof (struct pit_list));
    score = 0;
    errors = 0;
    lob = 0;

    for ( int i = 0; i <= 8; i++ ) {
	extra_stats[i].ord = 0;
	extra_stats[i].name[0] = '\n';
	extra_stats[i].next = NULL;
    }
}

    void
team::pos_change( int spot, char **comment )
{

    char *pos;
    pos = (char*) calloc(MAX_INPUT, sizeof(char));

    char tempstr[POSLEN];

    pl_list *oldpl = lineup;
    while ( oldpl->next && oldpl->next->ord <= spot )
	oldpl = oldpl->next;

    int loop = 1;
    while ( loop ) {
	fprintf( output, "Enter new position for %d: ", spot );
	memset( pos, '\0', MAX_INPUT );
	fgets( pos, MAX_INPUT, input );
	sanitize( &pos, POSLEN );

	if ( verify_pos( pos ) == 0 ) {
	    fprintf( stderr, "invalid position: %s\n", pos );
	    if ( input != stdin )
		exit(1);
	    else
		continue;
	}
	// if we get here it's all good
	break;
    }

#ifdef DEBUG
    fprintf( stderr, "pos_change %d: %s\n", spot, pos );
#endif

    snprintf( tempstr, POSLEN, "%d", spot );
    cmd->add( tempstr );
    fprintf( cmdfp, "%s\n", tempstr );
    cmd->add( pos );
    fprintf( cmdfp, "%s\n", pos );
    fflush( cmdfp );
    fprintf( output, "\n" );

    oldpl->head->new_pos(pos);
#ifdef DEBUG
    fprintf( stderr, "pos_change %d: %s DONE\n", spot, pos );
#endif
    snprintf( *comment, MAX_INPUT, "%s moves to %s.", oldpl->head->nout(), pos );
    free( pos );
}

// Adds a new player at position "spot" in the batting order
    void
team::insert( int spot, char **comment, const char *def, const char *str )
{
    struct pl_list *newpl, *oldpl;

    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));

    char *name, *mlb, *pos;
    name = (char*) calloc(NAMELEN, sizeof(char));
    mlb = (char*) calloc(TEAMLEN, sizeof(char));
    pos = (char*) calloc(POSLEN, sizeof(char));

    char tempstr[MAX_INPUT];

    newpl = (struct pl_list *) malloc( sizeof(struct pl_list) );
    newpl->ord = spot;
    newpl->next = NULL;

    if ( !(*str) ) {
	int loop = 1;
	while ( loop ) {
	    fprintf(output, "Player, MLB team & Position for %d: ", spot);
	    memset( inputstr, '\0', MAX_INPUT );
	    memset( name, '\0', NAMELEN );
	    memset( mlb, '\0', TEAMLEN );
	    memset( pos, '\0', POSLEN );

	    fgets( inputstr, MAX_INPUT, input );
	    sscanf( inputstr, "%s %s %s", name, mlb, pos );
	    sanitize( &name, NAMELEN );
	    sanitize( &mlb, TEAMLEN );
	    sanitize( &pos, POSLEN );

#ifdef DEBUG
	    fprintf( stderr, "insert %d: %s, %s, %s\n", spot, name, mlb, pos );
#endif
	    if ( strlen(name) == 0 || strlen(mlb) == 0 || strlen(pos) == 0 ) {
		fprintf( stderr, "formatting error\n" );
#ifdef DEBUG
		fprintf( stderr, "insert %d: %d, %d, %d\n", spot,
			(int)strlen(name), (int)strlen(mlb), (int)strlen(pos) );
#endif
		if ( input != stdin )
		    exit(1);
		else
		    continue;
	    }
	    if ( verify_pos( pos ) == 0 ) {
		fprintf( stderr, "invalid position: %s\n", pos );
		if ( input != stdin )
		    exit(1);
		else
		    continue;
	    }
	    // if we get here it's all good
	    fprintf( output, "\n" );
	    break;
	}
	if ( strcmp(def, "ph") ) {
	    snprintf( tempstr, POSLEN, "%d", spot );
	    cmd->add( tempstr );
	    fprintf( cmdfp, "%s\n", tempstr );
	}
	snprintf( tempstr, MAX_INPUT, "%s %s %s", name, mlb, pos );
	cmd->add( tempstr );
	fprintf( cmdfp, "%s\n", tempstr );
	fflush( cmdfp );
    }
    else {
	    sscanf( str, "%s %s %s", name, mlb, pos );
	    sanitize( &name, NAMELEN );
	    sanitize( &mlb, TEAMLEN );
	    sanitize( &pos, POSLEN );
    }

    oldpl=lineup;
    while ( oldpl->next && oldpl->next->ord <= spot ) oldpl=oldpl->next;

    if ( !(strcmp(pos, def)) ) {
	newpl->head = new player( name, ibl, mlb, def );
	snprintf( *comment, MAX_INPUT, "%s %s for %s, batting %d.",
		name, def, oldpl->head->nout(), spot );
    }
    else if ( pos[0] == 'p' ) {
	newpl->head = new player( name, ibl, mlb, pos );
	snprintf( *comment, MAX_INPUT, "%s now pitching for %s, batting %d.",
		name, ibl, spot );
    }
    else if ( def[0] == '\0' ) {
	newpl->head = new player( name, ibl, mlb, pos );
	snprintf( *comment, MAX_INPUT, "%s now %s for %s, batting %d.",
		name, pos, ibl, spot );
    }
    else {
	newpl->head = new player( name, ibl, mlb, def );
	newpl->head->new_pos(pos);
	snprintf( *comment, MAX_INPUT, "%s %s for %s, batting %d.",
		name, def, oldpl->head->nout(), spot);
    }

    newpl->next=oldpl->next;
    oldpl->next=newpl;
    if ( up() == oldpl->head )
	current=newpl;

#ifdef DEBUG
    fprintf( stderr, "insert %d: %s, %s, %s DONE\n", spot, name, mlb, pos );
#endif
    free( inputstr );
    free( name );
    free( mlb );
    free( pos );
}

// Creates the lineups for a team
    int
team::make_lineups()
{
    struct pl_list *newpl;
    int spot;

    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));

    char *name, *mlb, *pos;
    name = (char*) calloc(NAMELEN, sizeof(char));
    mlb = (char*) calloc(TEAMLEN, sizeof(char));
    pos = (char*) calloc(POSLEN, sizeof(char));

    char tempstr[MAX_INPUT];

    current = newpl = lineup;
    fprintf(output,"Enter lineup for %s \n", ibl);
    for ( spot = 1; spot < 10; spot++ ) {
	if ( spot == 1 )
	    newpl = lineup;
	else {
	    newpl->next = (struct pl_list *) malloc(sizeof (struct pl_list) );
	    newpl=newpl->next;
	    newpl->next=NULL;
	}
	newpl->ord = spot;

	int loop = 1;
	while ( loop ) {
	    fprintf(output, "Player, MLB team & Position for %d: ", spot);
	    memset( inputstr, '\0', MAX_INPUT );
	    memset( name, '\0', NAMELEN );
	    memset( mlb, '\0', TEAMLEN );
	    memset( pos, '\0', POSLEN );

	    fgets( inputstr, MAX_INPUT, input );
	    sscanf( inputstr, "%s %s %s", name, mlb, pos );
	    sanitize( &name, NAMELEN );
	    sanitize( &mlb, TEAMLEN );
	    sanitize( &pos, POSLEN );

#ifdef DEBUG
	    fprintf( stderr, "make_lineups: %s, %s, %s\n", name, mlb, pos );
#endif

	    if ( strlen(name) == 0 || strlen(mlb) == 0 || strlen(pos) == 0 ) {
		fprintf( stderr, "formatting error\n" );
#ifdef DEBUG
		fprintf( stderr, "make_lineups: %d, %d, %d\n",
			(int)strlen(name), (int)strlen(mlb), (int)strlen(pos) );
#endif
		if ( input != stdin )
		    exit(1);
		else
		    continue;
	    }
	    if ( verify_pos( pos ) == 0 ) {
		fprintf( stderr, "invalid position: %s\n", pos );
		if ( input != stdin )
		    exit(1);
		else
		    continue;
	    }
	    // if we get here it's all good
	    break;
	}
	snprintf( tempstr, MAX_INPUT, "%s %s %s", name, mlb, pos );
	cmd->add( tempstr );
	fprintf( cmdfp, "%s\n", tempstr );
	fflush( cmdfp );
	newpl->head = new player( name, ibl, mlb, pos );
    }

    fprintf( output, "\n" );
    free( inputstr );
    free( name );
    free( mlb );
    free( pos );
    return 1;
}

// Do the pitchers
    int
team::make_lineups_pit()
{
    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));
    char *name, *mlb, *throws;
    name = (char*) calloc(NAMELEN, sizeof(char));
    mlb = (char*) calloc(TEAMLEN, sizeof(char));
    throws = (char*) calloc(POSLEN, sizeof(char));

    char tempstr[MAX_INPUT];

    int loop = 1;
    while ( loop ) {
	fprintf(output, "Starting pitcher for %s, MLB team & Throws: ", ibl);
	memset( inputstr, '\0', MAX_INPUT );
	memset( name, '\0', NAMELEN );
	memset( mlb, '\0', TEAMLEN );
	memset( throws, '\0', POSLEN );

	fgets( inputstr, MAX_INPUT, input );
	sscanf( inputstr, "%s %s %s", name, mlb, throws );
	sanitize( &name, NAMELEN );
	sanitize( &mlb, TEAMLEN );
	sanitize( &throws, POSLEN );

#ifdef DEBUG
	fprintf( stderr, "make_lineups_pit: %s, %s, %s\n", name, mlb, throws );
#endif

	if ( strlen(name) == 0 || strlen(mlb) == 0 || strlen(throws) == 0 ) {
	    fprintf( stderr, "formatting error\n" );
#ifdef DEBUG
	    fprintf( stderr, "make_lineups_pit: %d, %d, %d\n",
		    (int)strlen(name), (int)strlen(mlb), (int)strlen(throws) );
#endif
	    if ( input != stdin )
		exit(1);
	    else
		continue;
	}
	// if we get here it's all good
	break;
    }
    snprintf( tempstr, MAX_INPUT, "%s %s %c", name, mlb, throws[0] );
    cmd->add( tempstr );
    fprintf( cmdfp, "%s\n", tempstr );
    fflush( cmdfp );
    pitchers->head = new pitcher( name, ibl, mlb, throws[0] );
    pitchers->next = 0;
    mound=pitchers->head;
#ifdef DEBUG
    fprintf(stderr,"%p:%s->%p\n",pitchers,pitchers->head->nout(),pitchers->next);
#endif
    //fprintf(output,"\n");
    free( inputstr );
    free( name );
    free( mlb );
    free( throws );
    return 1;
}


// Outputs the box score
    void
team::box_score( FILE *fp )
{
    struct pl_list *newpl;
    struct pit_list *newpit;

    int ab, h, r, rbi, b2, b3, hr, bb, k, sb, cs, pal, par, out, er, bf;

    ab=h=r=rbi=b2=b3=hr=bb=k=sb=cs=pal=par=out=er=bf=0;

    fprintf(fp,"\n\nBATTERS  %-29s AB  R  H RI 2B 3B HR SB CS BB  K PL PR\n", ibl);
    newpl=lineup;
    while (newpl) {
	ab=ab+newpl->head->ab;
	h=h+newpl->head->h;
	r=r+newpl->head->r;
	rbi=rbi+newpl->head->rbi;
	b2=b2+newpl->head->b2;
	b3=b3+newpl->head->b3;
	hr=hr+newpl->head->hr;
	bb=bb+newpl->head->bb;
	k=k+newpl->head->k;
	sb=sb+newpl->head->sb;
	cs=cs+newpl->head->cs;
	pal=pal+newpl->head->pal;
	par=par+newpl->head->par;
	fprintf(fp,"%i ",newpl->ord);
	newpl->head->sout(fp);
	fprintf(fp,"\n");
	newpl=newpl->next;
    }
    fprintf(fp,"\n");
    fprintf(fp,"TOTALS for %-26s %3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d\n",
			ibl,ab,r,h,rbi,b2,b3,hr,sb,cs,bb,k,pal,par);

    ab=h=r=rbi=b2=b3=hr=bb=k=sb=cs=pal=par=out=er=bf=0;

    fprintf(fp,"\nPITCHERS  %-19s   IP  H  R ER BB  K HR  BF\n", ibl);
    newpit=pitchers;
    while (newpit) {
	out=out+newpit->head->out;
	h=h+newpit->head->h;
	r=r+newpit->head->r;
	er=er+newpit->head->er;
	bb=bb+newpit->head->bb;
	k=k+newpit->head->k;
	hr=hr+newpit->head->hr;
	bf=bf+newpit->head->bf;
	newpit->head->sout(fp);
	fprintf(fp,"\n");
	newpit=newpit->next;
    }
    fprintf(fp,"\n");
    fprintf(fp,"TOTALS for %-17s %3d.%1d%3d%3d%3d%3d%3d%3d %3d\n\n",
		    ibl,out/3,out % 3,h,r,er,bb,k,hr,bf);
}

// Returns the next batter in the order
    player*
team::next_up()
{
    if (current && current->next)
	current=current->next;
    else
	current=lineup;

    while (current->next && current->ord==current->next->ord)
	current=current->next;

    return current->head;
}

// Returns the current batter
    player*
team::up()
{
    return current->head;
}

// Inputs a new pitcher
    int
team::new_pit()
{
    struct pit_list *newpit,*oldpit;

    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));

    char *name, *mlb, *throws, *bats;
    name = (char*) calloc(NAMELEN, sizeof(char));
    mlb = (char*) calloc(TEAMLEN, sizeof(char));
    throws = (char*) calloc(POSLEN, sizeof(char));
    bats = (char*) calloc(POSLEN, sizeof(char));

    char tempstr[MAX_INPUT];

    newpit=(struct pit_list *)malloc(sizeof (struct pit_list));
    newpit->next=NULL;

    int loop = 1;
    while ( loop ) {
	fprintf(output, "Relief Pitcher, MLB team & Throws: " );
	memset( inputstr, '\0', MAX_INPUT );
	memset( name, '\0', NAMELEN );
	memset( mlb, '\0', TEAMLEN );
	memset( throws, '\0', POSLEN );
	memset( bats, '\0', POSLEN );

	fgets( inputstr, MAX_INPUT, input );
	sscanf( inputstr, "%s %s %s %s", name, mlb, throws, bats );
	sanitize( &name, NAMELEN );
	sanitize( &mlb, TEAMLEN );
	sanitize( &throws, POSLEN );
	sanitize( &bats, POSLEN );

#ifdef DEBUG
	fprintf( stderr, "new_pit: %s, %s, %s, %s\n", name, mlb, throws, bats );
#endif
	if ( strlen(name) == 0 || strlen(mlb) == 0 || strlen(throws) == 0 ) {
	    fprintf( stderr, "formatting error\n" );
#ifdef DEBUG
	    fprintf( stderr, "new_pit: %d, %d, %d\n",
		    (int)strlen(name), (int)strlen(mlb), (int)strlen(throws) );
#endif
	    if ( input != stdin )
		exit(1);
	    else
		continue;
	}
	// if we get here it's all good
	fprintf( output, "\n" );
	break;
    }

    if ( strlen(bats) > 0 ) {
	snprintf( tempstr, MAX_INPUT, "%s %s %c %c", name, mlb, throws[0], bats[0] );
	cmd->add( tempstr );
	fprintf( cmdfp, "%s\n", tempstr );
    }
    else {
	snprintf( tempstr, MAX_INPUT, "%s %s %c", name, mlb, throws[0] );
	cmd->add( tempstr );
	fprintf( cmdfp, "%s\n", tempstr );
    }
    fflush( cmdfp );
    newpit->head = new pitcher( name, ibl, mlb, throws[0] );
    newpit->next = 0;
    oldpit = pitchers;
#ifdef DEBUG
    fprintf(stderr,"\n%p:%s->%p ",oldpit,oldpit->head->nout(),oldpit->next);
#endif
    while (oldpit->next) {
#ifdef DEBUG
    fprintf(stderr,"\n%p:%s->%p ",oldpit,oldpit->head->nout(),oldpit->next);
#endif
	oldpit=oldpit->next;
    }
    oldpit->next = newpit;
    mound = newpit->head;

    int val = bats[0] - '0';
    free( inputstr );
    free( name );
    free( mlb );
    free( throws );
    free( bats );
    return ( val );
}

    char*
team::nout()
{
    return(ibl);
}

    int
team::findord( player *up )
{
   struct pl_list *curr = lineup;

   while (curr->head != up)
	curr = curr->next;

   return (curr->ord);
}

    player*
team::findord( int i )
{
    struct pl_list *oldpl;

    oldpl=lineup;
    while (oldpl->next && oldpl->next->ord <= i) oldpl=oldpl->next;
    return(oldpl->head);
}

    struct pl_list*
team::findord_pl( int i )
{
    struct pl_list *oldpl;

    oldpl=lineup;
    while (oldpl->next && oldpl->next->ord <= i) oldpl=oldpl->next;
    return(oldpl);
}

    void
team::print_lineup()
{
    struct pl_list *newpl;

    fprintf(output,"Lineup for %s\n",nout());
    newpl=lineup;
    while (newpl) {
	while (newpl->next && newpl->ord==newpl->next->ord) {
	    newpl=newpl->next;
	}
	fprintf(output,"%d. %s, %s\n",newpl->ord,newpl->head->nout(),newpl->head->pout());
	newpl=newpl->next;
    }
}

    void
team::check_defense()
{
    struct pl_list *newpl;
    int i, c, field[8];
    char missing[25];

    missing[0] = '\0';

    for ( i = 0; i < 8; i++ ) {
	field[i] = 0;
    }

    newpl=lineup;
    while (newpl) {
	while (newpl->next && newpl->ord==newpl->next->ord) {
	    newpl=newpl->next;
	}
	if ( newpl->head->getpos( newpl->head->pout() ) >= 2 ) {
	    field[ newpl->head->getpos( newpl->head->pout() ) - 2 ]++;
	}
	newpl=newpl->next;
    }

    c = 0;
    for ( i = 0; i < 8; i++ ) {
#ifdef DEBUG
	fprintf( stderr, "check_defense: %d/%d : %s\n", i, field[i], missing );
#endif
	if ( field[i] == 0 ) {
	    switch ( i + 2 ) {
		case 2	:
		    snprintf( missing + c, 3, "%s", " c" );
		    c += 2;
		    break;
		case 3	:
		    snprintf( missing + c, 4, "%s", " 1b" );
		    c += 3;
		    break;
		case 4	:
		    snprintf( missing + c, 4, "%s", " 2b" );
		    c += 3;
		    break;
		case 5	:
		    snprintf( missing + c, 4, "%s", " 3b" );
		    c += 3;
		    break;
		case 6	:
		    snprintf( missing + c, 4, "%s", " ss" );
		    c += 3;
		    break;
		case 7	:
		    snprintf( missing + c, 4, "%s", " lf" );
		    c += 3;
		    break;
		case 8	:
		    snprintf( missing + c, 4, "%s", " cf" );
		    c += 3;
		    break;
		case 9	:
		    snprintf( missing + c, 4, "%s", " rf" );
		    c += 3;
		    break;
	    }
	}
    }

    if ( strlen(missing) != 0 ) {
	fprintf(output,"defense missing:%s\n", missing);
    }

}

    int
team::verify_pos( const char *str )
{
    // return 0 if invalid
    // return 1 if valid

    // valid positions
    const char* valid_pos[] = {
	"p", "P",
	"c", "C",
	"1b", "1B",
	"2b", "2B",
	"3b", "3B",
	"ss", "SS",
	"lf", "LF",
	"cf", "CF",
	"rf", "RF",
	"dh", "DH",
	"ph", "PH",
	"pr", "PR"
    };

    int i, tot;
    tot = sizeof valid_pos / sizeof *valid_pos;
    for( i = 0; i != tot; i++ )
	if ( strcmp( str, valid_pos[i] ) == 0 )
	    return( 1 );

    return( 0 );
}

    int
team::team_hits ()
{
    int total = 0;
    struct pl_list *newpl;

    newpl=lineup;

    while(newpl) {
       total = total + newpl->head->h;
       newpl = newpl->next;}

    return (total);
}

    void
team::newstat(char *pl_name, int stat)
{
    struct stat_list *curr, *prev;

    extra_stats[stat].ord++;
    prev = curr = extra_stats[stat].next;

    if ( strlen(pl_name) ) {
#ifdef DEBUG
	fprintf( stderr, "p:%p c:%p\n", prev, curr );
#endif
	if (prev) {
	    while ((curr) && (strcmp(prev->name, pl_name))){
#ifdef DEBUG
		fprintf( stderr, "p:%p c:%p\n", prev, curr );
#endif
		prev = curr;
		curr = curr->next;
	    }
	    if ( !(strcmp(prev->name, pl_name)) ) {
		prev->ord++;}
	    else {
#ifdef DEBUG
		fprintf( stderr, "p:%p c:%p\n", prev, curr );
#endif
		curr = (stat_list*) malloc(sizeof(stat_list));
		curr->ord = 1;
		snprintf( curr->name, NAMELEN, "%s", pl_name );
		curr->next = NULL;
		prev->next = curr;
	    }
#ifdef DEBUG
	    fprintf( stderr, "p:%p c:%p\n", prev, curr );
#endif
	}
	else {
#ifdef DEBUG
	    fprintf( stderr, "p:%p c:%p\n", prev, curr );
#endif
	    prev = (stat_list*) malloc(sizeof(stat_list));
	    prev->ord = 1;
	    snprintf( prev->name, NAMELEN, "%s", pl_name );
	    prev->next = NULL;
	    extra_stats[stat].next = prev;
#ifdef DEBUG
	    fprintf( stderr, "p:%p c:%p\n", prev, curr );
#endif
	}
    }
}

    void
team::printstat( FILE *fp,int stat )
{

    struct stat_list *curr;
    curr = extra_stats[stat].next;

#ifdef DEBUG
    fprintf( stderr, "es:%d %d c:%p\n", stat, extra_stats[stat].ord, curr );
#endif

    if (extra_stats[stat].ord) {
	fprintf(fp,"%d - ",extra_stats[stat].ord);
	if ( curr ) {
	    while( curr->next) {
#ifdef DEBUG
		fprintf( stderr, "c:%p cn:%p\n",curr,curr->next );
#endif
		if (curr->ord > 1) {
		    fprintf(fp,"%s %d, ", curr->name, curr->ord);
		    curr = curr->next;
		}
		else {
		    fprintf(fp,"%s, ",curr->name);
		    curr = curr->next;
		}
	    }
	    if (curr->ord > 1)
		fprintf(fp,"%s %d.\n",curr->name,curr->ord);
	    else
		fprintf(fp,"%s.\n",curr->name);
	}
	else
	    fprintf(fp,"none.\n");
    }

    else
	fprintf(fp,"none.\n");
}

    char*
team::posout( int position )
{

    struct pl_list *curr = lineup;

    while (curr->next) {
	if (curr->ord == curr->next->ord)
	    curr = curr->next;
	else if (curr->head->posn == position)
	    return (curr->head->nout());
	else
	    curr = curr->next;
    }

    if (curr->head->posn == position)
	return (curr->head->nout());
    else
	return ( (char*)NULL );
}

    void
team::decisions()
{

    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));

    struct pit_list *curr = pitchers;

    fprintf( output, "\nEnter W/L/S for appropriate %s pitcher.  <CR> if none.\n", ibl );
    while (curr) {
	fprintf( output, "%s: ", curr->head->nout() );
	memset( inputstr, '\0', MAX_INPUT );
	fgets( inputstr, MAX_INPUT, input );
	sanitize( &inputstr, POSLEN );

	switch (*inputstr) {
		case 'w' :
		case 'W' : curr->head->dec = 'W';
			   break;
		case 'l' :
		case 'L' : curr->head->dec = 'L';
			   break;
		case 's' :
		case 'S' : curr->head->dec = 'S';
			   break;
	}
	if ( curr->head->dec == '-' )
	    fprintf( cmdfp, "\n" );
	else
	    fprintf( cmdfp, "%c\n", curr->head->dec );
	fflush( cmdfp );
	curr = curr->next;
    }
    free( inputstr );
}

    void
team::unearned( int inning )
{
    int numout = 0;
    int ur = 0;

    char *inputstr;
    inputstr = (char*) calloc(MAX_INPUT, sizeof(char));

    char tempstr[MAX_INPUT];

    struct pit_list *curr = pitchers;

    while ( curr ) {
	if ( curr->head->out + numout >= (inning-1)*3 ) {
	    int loop = 1;
	    while( loop ) {
		fprintf( output, "Enter unearned runs for %s: ", curr->head->nout() );
		memset( inputstr, '\0', MAX_INPUT );
		fgets( inputstr, MAX_INPUT, input );
		sanitize( &inputstr, MAX_INPUT );

		if ( isalpha( inputstr[0] ) ) {
		    fprintf( stderr, "Invalid unearned runs total.\n" );
		    continue;
		}
		ur = atoi(inputstr);
		if ( ur < 0 || ur > curr->head->er ) {
		    fprintf( stderr, "Invalid unearned runs total.\n" );
		    continue;
		}
		// if we get here it's all good
		break;
	    }
	    numout += curr->head->out;
	    curr->head->er = curr->head->er-ur;
	    snprintf( tempstr, MAX_INPUT, "%d", ur );
	    cmd->add( tempstr );
	    fprintf( cmdfp, "%s\n", tempstr );
	    fflush( cmdfp );
	}
	else
	    numout += curr->head->out;

	curr = curr->next;
    }
    fprintf( output, "\n" );
    free( inputstr );
}

team::~team()
{
    pl_list *pl;

    while (lineup) {
	pl = lineup;
	delete (pl->head);
	lineup = lineup->next;
	free (pl);
    }

    pit_list *pit;

    while (pitchers) {
	pit = pitchers;
	delete (pit->head);
	pitchers = pitchers->next;
	free (pit);
    }

    stat_list *prev, *curr;

    for (int i=0; i<9; i++) {
	prev = curr = extra_stats[i].next;
	while (curr) {
	   prev = curr;
	   curr = curr->next;
	   free (prev);
	}
    }
#ifdef DEBUG
    fprintf(stderr,"deleted %s\n",this->nout());
#endif
}
