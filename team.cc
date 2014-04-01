// $Id$

#include "team.h"
#include "extern.h"

// Constructor functions
team::team( int who )
{
    char str[MAX_INPUT];

    int i;
    int flag = 1;

    while ( flag ) {
	flag = 0; 
	fprintf(output, "Enter the three letter code for %s team:\n",
		who == 0 ? "away" : "home" );
	memset( str, '\0', MAX_INPUT );
	fgets( str, MAX_INPUT, input );

	if ( strlen(str) > 3 ) {
	    for ( i=0; i< 3; i++ ) {
		if ( isalpha(str[i]) ) {
		    if ( islower (str[i]) )
			str[i] = toupper(str[i]);   // convert to uppercase
		}
		else {
		    flag = 1;
		}
	    }
	    if ( flag ) {
		    fprintf(stderr, "Team name must be alphanumeric\n");
		    if (input == stdin) 
			flag = 1; 
		    else 
			exit(1);
	    }
	}
	else { 
	    fprintf(stderr, "Team name must be 3 letters!\n"); 
	    if (input == stdin) 
		flag = 1; 
	    else 
		exit(1);
	}
    }

    strcpy( ibl, stripcr( str, TEAMLEN ) );
    fprintf(cmdfp, "%s\n", ibl);
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
}

// Constructor function getting team name; Assumes name is correct
team::team( char *str )
{
    strcpy( ibl, stripcr( str, TEAMLEN ) );
    fprintf(cmdfp, "%s\n", ibl);
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
    char tempstr[MAX_INPUT];
    memset( tempstr, '\0', MAX_INPUT );

    char pos[POSLEN];
    memset( pos, '\0', POSLEN );

    pl_list *oldpl = lineup;

    while ( oldpl->next && oldpl->next->ord <= spot )
	oldpl = oldpl->next;

    fprintf( output, "\nEnter new position for %d: ", spot );
    fgets( tempstr, MAX_INPUT, input );
    strcpy( pos, stripcr( tempstr, POSLEN ) );
#ifdef DEBUG
    fprintf( stderr, "pos_change %d: %s\n", spot, pos );
#endif

    fprintf( cmdfp, "%d\n", spot );
    fprintf( cmdfp, "%s\n", pos );
    fprintf( output, "\n" );

    oldpl->head->new_pos(pos);
#ifdef DEBUG
    fprintf( stderr, "pos_change %d: %s DONE\n", spot, pos );
#endif
    sprintf( tempstr, "%s moves to %s.", oldpl->head->nout(), pos );
    strcat( *comment, tempstr );
}

// Adds a new player at position "spot" in the batting order
    void
team::insert( int spot, char **comment, char *def, char *inputstr )
{
    struct pl_list *newpl, *oldpl;
    int flag;

    char tempstr[MAX_INPUT];

    char mlb[MAX_INPUT];
    char name[MAX_INPUT];
    char pos[MAX_INPUT];

    newpl = (struct pl_list *) malloc( sizeof(struct pl_list) );
    newpl->ord = spot;
    newpl->next = NULL;
    
    if ( !(*inputstr) ) {
	flag = 1;
    	while (flag) {
	    flag=0;
	    fprintf(output, "Player, MLB team & Position for %d: ", spot);
	    memset( tempstr, '\0', MAX_INPUT );
	    memset( mlb, '\0', MAX_INPUT );
	    memset( name, '\0', MAX_INPUT );
	    memset( pos, '\0', MAX_INPUT );
	    fgets( tempstr, MAX_INPUT, input );
	    fprintf( output, "\n" );

	    sscanf( tempstr, "%s %s %s", name, mlb, pos );
	    strcpy( name, stripcr( name, NAMELEN ) );
	    strcpy( mlb, stripcr( mlb, TEAMLEN ) );
	    strcpy( pos, stripcr( pos, POSLEN ) );
#ifdef DEBUG
	    fprintf( stderr, "insert %d: %s, %s, %s\n", spot, name, mlb, pos );
#endif
	    if ( strlen(name) == 0 || strlen(mlb) == 0 || strlen(pos) == 0 ) {
		fprintf( stderr, "formatting error\n" );
#ifdef DEBUG
		fprintf( stderr, "insert %d: %d, %d, %d\n", spot,
	    		strlen(name), strlen(mlb), strlen(pos) );
#endif
		if ( input == stdin ) 
		    flag = 1; 
		else 
		    exit(1);
	    }
	}
	if ( strcmp(def, "ph") )	
	    fprintf( cmdfp, "%d\n", spot );
	fprintf( cmdfp, "%s", tempstr );
    }
    else {
	    sscanf( inputstr, "%s %s %s", name, mlb, pos );
	    strcpy( name, stripcr( name, NAMELEN ) );
	    strcpy( mlb, stripcr( mlb, TEAMLEN ) );
	    strcpy( pos, stripcr( pos, POSLEN ) );
    }

    oldpl=lineup;
    while ( oldpl->next && oldpl->next->ord <= spot ) oldpl=oldpl->next;

    if ( !(strcmp(pos, def)) ) {
	newpl->head = new player( name, ibl, mlb, def );
	sprintf( tempstr, "%s %s for %s, batting %d.",
		name, def, oldpl->head->nout(), spot );
    }
    else if ( pos[0] == 'p' ) {
	newpl->head = new player( name, ibl, mlb, pos );
	sprintf( tempstr, "%s now pitching for %s, batting %d.",
		name, ibl, spot );
    }
    else if ( def[0] == '\0' ) {
	newpl->head = new player( name, ibl, mlb, pos );
	sprintf( tempstr, "%s now %s for %s, batting %d.",
		name, pos, ibl, spot );
    }
    else {
	newpl->head = new player( name, ibl, mlb, def );
	newpl->head->new_pos(pos); 
        sprintf( tempstr, "%s %s for %s, batting %d.",
                name, def, oldpl->head->nout(), spot);
    }

    newpl->next=oldpl->next;
    oldpl->next=newpl;
    if ( up() == oldpl->head ) 
	current=newpl;

#ifdef DEBUG
    fprintf( stderr, "insert %d: %s, %s, %s DONE\n", spot, name, mlb, pos );
#endif
    strcat( *comment, tempstr );
}

// Creates the lineups for a team
    int 
team::make_lineups()
{
    struct pl_list *newpl;
    int spot, flag;

    char tempstr[MAX_INPUT];

    char mlb[MAX_INPUT];
    char name[MAX_INPUT];
    char pos[MAX_INPUT];
    
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
	flag = 1;
    	while ( flag ) {
	    flag = 0;
	    fprintf(output, "Player, MLB team & Position for %d: ", spot);
	    memset( tempstr, '\0', MAX_INPUT );
	    memset( mlb, '\0', MAX_INPUT );
	    memset( name, '\0', MAX_INPUT );
	    memset( pos, '\0', MAX_INPUT );
	    fgets( tempstr, MAX_INPUT, input );
	    // fprintf( output, "\n" );

	    sscanf( tempstr, "%s %s %s", name, mlb, pos );
	    strcpy( name, stripcr( name, NAMELEN ) );
	    strcpy( mlb, stripcr( mlb, TEAMLEN ) );
	    strcpy( pos, stripcr( pos, POSLEN ) );
#ifdef DEBUG
	    fprintf( stderr, "make_lineups: %s, %s, %s\n", name, mlb, pos );
#endif

	    if ( strlen(name) == 0 || strlen(mlb) == 0 || strlen(pos) == 0 ) {
		fprintf( stderr, "formatting error\n" );
#ifdef DEBUG
		fprintf( stderr, "make_lineups: %d, %d, %d\n", 
	    		strlen(name), strlen(mlb), strlen(pos) );
#endif
		if ( input == stdin ) 
		    flag = 1; 
		else 
		    exit(1);
	    }
	}
	fprintf( cmdfp, "%s", tempstr );
	newpl->head = new player( name, ibl, mlb, pos );
    }

    fprintf(output,"\n");
    return 1;
}

// Do the pitchers
    int 
team::make_lineups_pit()
{
    int flag;

    char tempstr[MAX_INPUT];

    char mlb[MAX_INPUT];
    char name[MAX_INPUT];
    char throws[MAX_INPUT];

    flag = 1;
    while ( flag ) {
	flag=0;
	fprintf(output, "Starting pitcher for %s, MLB team & Throws: ", ibl);
	memset( tempstr, '\0', MAX_INPUT );
	memset( mlb, '\0', MAX_INPUT );
	memset( name, '\0', MAX_INPUT );
	memset( throws, '\0', MAX_INPUT );
	fgets( tempstr, MAX_INPUT, input );
	// fprintf( output, "\n" );

	sscanf( tempstr, "%s %s %s", name, mlb, throws );
	strcpy( name, stripcr( name, NAMELEN ) );
	strcpy( mlb, stripcr( mlb, TEAMLEN ) );
	strcpy( throws, stripcr( throws, 2 ) );
#ifdef DEBUG
	fprintf( stderr, "make_lineups_pit: %s, %s, %s\n", name, mlb, throws );
#endif

	if ( strlen(name) == 0 || strlen(mlb) == 0 || strlen(throws) == 0 ) {
	    fprintf( stderr, "formatting error\n" );
#ifdef DEBUG
	    fprintf( stderr, "make_lineups_pit: %d, %d, %d\n", 
	    		strlen(name), strlen(mlb), strlen(throws) );
#endif
	    if ( input == stdin ) 
		flag = 1; 
	    else 
		exit(1);
	}
    }
    fprintf( cmdfp, "%s", tempstr );
    pitchers->head = new pitcher( name, ibl, mlb, throws[0] );
    pitchers->next = 0;
    mound=pitchers->head;
#ifdef DEBUG
    fprintf(stderr,"%d:%s->%d\n",pitchers,pitchers->head->nout(),pitchers->next);
#endif
    fprintf(output,"\n");
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

    player* 
team::back_up()
{
    if (current->ord != 1)
	current=findord_pl(current->ord-1);
    else
	current=findord_pl(9);

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
    int flag;

    char tempstr[MAX_INPUT];

    char mlb[MAX_INPUT];
    char name[MAX_INPUT];
    char throws[MAX_INPUT];
    char bats[MAX_INPUT];

    newpit=(struct pit_list *)malloc(sizeof (struct pit_list));
    newpit->next=NULL;

    flag = 1;
    while ( flag ) {
	flag = 0;
	fprintf(output, "Relief Pitcher, MLB team & Throws: " );
	memset( tempstr, '\0', MAX_INPUT );
	memset( mlb, '\0', MAX_INPUT );
	memset( name, '\0', MAX_INPUT );
	memset( throws, '\0', MAX_INPUT );
	memset( bats, '\0', MAX_INPUT );
	fgets( tempstr, MAX_INPUT, input );
	fprintf( output, "\n" );

	sscanf( tempstr, "%s %s %s %s", name, mlb, throws, bats );
	strcpy( name, stripcr( name, NAMELEN ) );
	strcpy( mlb, stripcr( mlb, TEAMLEN ) );
	strcpy( throws, stripcr( throws, 2 ) );
	strcpy( bats, stripcr( bats, 2 ) );
#ifdef DEBUG
	fprintf( stderr, "new_pit: %s, %s, %s, %s\n", name, mlb, throws, bats );
#endif

	if ( strlen(name) == 0 || strlen(mlb) == 0 || strlen(throws) == 0 ) {
	    fprintf( stderr, "formatting error\n" );
#ifdef DEBUG
	    fprintf( stderr, "new_pit: %d, %d, %d\n", 
	    		strlen(name), strlen(mlb), strlen(throws) );
#endif
	    if ( input == stdin ) 
		flag = 1; 
	    else 
		exit(1);
	}
    }
    fprintf( cmdfp, "%s", tempstr );
    newpit->head = new pitcher( name, ibl, mlb, throws[0] );
    newpit->next = 0;
    oldpit = pitchers;
#ifdef DEBUG
    fprintf(stderr,"\n%d:%s->%d ",oldpit,oldpit->head->nout(),oldpit->next);
#endif
    while (oldpit->next) {
#ifdef DEBUG
    fprintf(stderr,"\n%d:%s->%d ",oldpit,oldpit->head->nout(),oldpit->next);
#endif
	oldpit=oldpit->next;
    }
    oldpit->next = newpit;
    mound = newpit->head;
    return ( bats[0] - '0' );
}

    char* 
team::nout()
{
    return(ibl);
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

    fprintf(output,"\nLineup for %s\n",nout());
    newpl=lineup;
    while (newpl) {
	while (newpl->next && newpl->ord==newpl->next->ord) newpl=newpl->next;
	fprintf(output,"%d. %s, %s\n",newpl->ord,newpl->head->nout(),newpl->head->pout());
	newpl=newpl->next;
    }
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
	fprintf( stderr, "p:%d c:%d\n", prev, curr );
#endif
	if (prev) {
	    while ((curr) && (strcmp(prev->name, pl_name))){
#ifdef DEBUG
		fprintf( stderr, "p:%d c:%d\n", prev, curr );
#endif
		prev = curr;
		curr = curr->next;
	    }
	    if ( !(strcmp(prev->name, pl_name)) ) {
		prev->ord++;}
	    else {
#ifdef DEBUG
		fprintf( stderr, "p:%d c:%d\n", prev, curr );
#endif
		curr = (stat_list*) malloc(sizeof(stat_list));
		curr->ord = 1;
		strcpy(curr->name, pl_name);	
		curr->next = NULL;
		prev->next = curr;
	    }
#ifdef DEBUG
	    fprintf( stderr, "p:%d c:%d\n", prev, curr );
#endif
	}
	else {
#ifdef DEBUG
	    fprintf( stderr, "p:%d c:%d\n", prev, curr );
#endif
	    prev = (stat_list*) malloc(sizeof(stat_list));
	    prev->ord = 1;
	    strcpy(prev->name, pl_name);
	    prev->next = NULL;
	    extra_stats[stat].next = prev;
#ifdef DEBUG
	    fprintf( stderr, "p:%d c:%d\n", prev, curr );
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
    fprintf( stderr, "es:%d %d c:%d\n", stat, extra_stats[stat].ord, curr );
#endif

    if (extra_stats[stat].ord) {
	fprintf(fp,"%d - ",extra_stats[stat].ord);
	if ( curr ) {
	    while( curr->next) {
#ifdef DEBUG
		fprintf( stderr, "c:%d cn:%d\n",curr,curr->next );
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
	return ("\0");
}

    int 
team::what_ord( player *up )
{

   struct pl_list *curr = lineup;

   while (curr->head != up)
	curr = curr->next;

   return (curr->ord);
}

    void 
team::decisions()
{

    struct pit_list *curr = pitchers;
    char wls[MAX_INPUT];

    fprintf( output, "\nEnter W/L/S for appropriate %s pitcher.  <CR> if none.\n", ibl );
    while (curr) {
	fprintf( output, "%s: ", curr->head->nout() );
	memset( wls, '\0', MAX_INPUT );
	fgets( wls, MAX_INPUT, input );
	switch (*wls) {
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
	curr = curr->next;
    }
}
 
    void 
team::unearned( int inning )
{
    int numout = 0;
    int ur = 0;
    char urstr[MAX_INPUT];

    struct pit_list *curr = pitchers;

    while ( curr ) {
	if ( curr->head->out + numout >= (inning-1)*3 ) {
	    fprintf( output, "Enter unearned runs for %s: ", curr->head->nout() );
	    memset( urstr, '\0', MAX_INPUT );
	    fgets( urstr, MAX_INPUT, input );
	    ur = atoi(urstr);
	    while ( ur < 0 || ur > curr->head->er ) {
		fprintf( stderr, "Invalid unearned runs total.\n" );
		fprintf( output, "Enter unearned runs for %s: ", curr->head->nout() );
		memset( urstr, '\0', MAX_INPUT );
		fgets( urstr, MAX_INPUT, input );
		ur = atoi(urstr);
	    }
	    numout += curr->head->out;
	    curr->head->er = curr->head->er-ur;
	    fprintf( cmdfp, "%d\n", ur );
	}
	else
	    numout += curr->head->out;

	curr = curr->next;
    }
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
