// $Id$

#include "frame.h"
#include "queue.h"

    int 
frame::update()
{
    int i,j;
    char spc;

    char tempstr[MAX_INPUT];
    memset( tempstr, '\0', MAX_INPUT );

    if (location[0] == '\0')
        spc = '\0';
    else
        spc = ' ';
 
#ifdef DEBUG
    fprintf( stderr, "ev=%s lc=%s br=%s\n",event,location,baserunning);
#endif

    if ((input != stdin) && feof(input)) {
	cont = 0;
	output = stdout;
	frameput();
	return 1;
    }

    if (event[0] == '\0') {
	frameput();
	return 1; 			// No event to decode, ignore.
    }

    if (!(strcmp(event,"ph"))) {
	memset( location, '\0', MAX_INPUT );
	memset( baserunning, '\0', MAX_INPUT );
	ibl[atbat]->insert( bat->what_ord(onbase[0]), &comment, "ph" );
	onbase[0]=bat->up();
	runadv();
	frameput();
        strcat(comment," ");
        outbuf(pbpfp,comment);
#ifdef DEBUG
        fprintf(stderr, "%s now batting %c for %s.",
	bat->findord(location[0]-'0')->nout(),location[0],bat->nout());
#endif
	return 1;  
    }
    else if (!(strcmp(event,"pr"))) {
	memset( location, '\0', MAX_INPUT );
	memset( baserunning, '\0', MAX_INPUT );
	ibl[atbat]->print_lineup();
	while (location[0] < '0' || location[0] > '9') {
	    fprintf(output,"\nEnter batting order of player: ");
	    fgets(location,MAX_INPUT,input);
	    // fprintf(output,"\n");
	}
	for (i=1; i<4; i++) {
	    if (onbase[i]==ibl[atbat]->findord(location[0]-'0')) {
		ibl[atbat]->insert( (int)location[0]-'0', &comment, "pr" );
		onbase[i]=ibl[atbat]->findord(location[0]-'0');
		runadv();
		frameput();
        	strcat(comment," ");
        	outbuf(pbpfp,comment);
#ifdef DEBUG
	      	fprintf(stderr, "%s now running on %d for %s.",
	 	onbase[i]->nout(),i,bat->nout());
#endif
		return 1;
	    }
	}	
	return 0;
    }
    else if (!(strcmp(event,"np"))) {
	int bats = pit->new_pit();
#ifdef DEBUG
	printf("batting %d",bats);
#endif
	if ((bats < 1) || (bats > 9))
	    sprintf(comment,"%s now pitching for %s. ",
			pit->mound->nout(),pit->nout());
	else {
	    sprintf(tempstr,"%s %s p",pit->mound->nout(),pit->mound->tout());
	    pit->insert( bats, &comment, "", tempstr );
	}
	runadv();
	frameput();
        strcat(comment," ");
        outbuf(pbpfp,comment);
	return 1; 
    }
    else if (!(strcmp(event,"dr")) || !(strcmp(event,"dc"))) {
	memset( location, '\0', MAX_INPUT );
	memset( baserunning, '\0', MAX_INPUT );
	pit->print_lineup();
	while (location[0] < '0' || location[0] > '9') {
	    fprintf(output,"\nEnter batting order of player: ");
	    fgets(location,MAX_INPUT,input);
	    // fprintf(output,"\n");
	}	
	if (location[0] > '0') {
	    if (!(strcmp(event,"dr")))  
		pit->insert( (location[0]-'0'), &comment );
	    else
		pit->pos_change( (location[0]-'0'), &comment );
	}
	else
	    fprintf( cmdfp, "0\n" );
	runadv();
#ifdef DEBUG
        fprintf(stderr, "%s now batting %c for %s.",
	pit->findord(location[0]-'0')->nout(),location[0],pit->nout());
#endif
	frameput();
        strcat(comment," ");
        outbuf(pbpfp,comment);
	return 1;
    }
    else if (!(strcmp(event,"la"))) {
	ibl[0]->print_lineup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"lh"))) {
        ibl[1]->print_lineup();
	frameput();
        return 1;
    }
    else if (!(strcmp(event,"un"))) {
	char in_ext[5], out_ext[5];
	int times = 1;

	if (undo) {cont=0;return 1;} else {
	    undo=1;
	    fclose(pbpfp);
	    fclose(stsfp);
	    fclose(cmdfp);

	    cleanup();

	    sprintf(in_ext,".cmd");
	    sprintf(out_ext,".un%d",times);

	    while (!(backup(in_ext,out_ext))) {
		strcpy(in_ext,out_ext);
		sprintf(out_ext,".un%d",++times);
		fclose (cmdfp);
		fclose (undofp);
	    }

	    fputs("un",undofp);
	    fclose(undofp);
	    fclose(cmdfp);
	    strcpy(tempstr,filename);
	    strcat(tempstr,out_ext);
	    undofp=fopen(tempstr,"r");
	    openfile(filename);
	    output = fopen("/dev/null","w");
	    input = undofp;
#ifdef DEBUG
	    fprintf(stderr,"setting up: ");
#endif
	    setup(0);
#ifdef DEBUG
	    fprintf(stderr,"1 ");
#endif
	    setup(1);
#ifdef DEBUG
	    fprintf(stderr,"2 ");
#endif
	    setup();
#ifdef DEBUG
	    fprintf(stderr,"\nreloading\n");
#endif
	    play();
	    output = stdout;
	    input = stdin;
	    undo=0;
	    cont=1;
#ifdef DEBUG
	    fprintf(stderr,"%d %d %d %d %d %d %d %d\n", 
		undo, cont, outs, atbat, inning, runs, linesize, errflag);
#endif
	    return 2;
	}
    }
    else if (!(strcmp(event,"en"))) {
#ifdef DEBUG
	print_linescore(stderr);
#endif
	if (errflag && runs) pit->unearned(inning);

	sprintf(tempstr,"%s %d %s %d\n",ibl[0]->nout(),ibl[0]->score,
					ibl[1]->nout(),ibl[1]->score);
	outbuf(pbpfp,tempstr,"\n");
	outbuf(pbpfp,"","\n");

	linescore[atbat][inning-1] = runs;
	for (i=1; i<4; i++)
	    if (onbase[i]) bat->lob++;

	runners->clear();

	atbat=(atbat+1)%2;
	outs=0;
	runs = 0;
	errflag=0;
	for (i=0; i<4; i++) onbase[i]=NULL;
	if (!(atbat)) inning++;
	sprintf(tempstr,"%s %d: ",ibl[atbat]->nout(),inning);
	outbuf(pbpfp,tempstr);
	pit=bat;
	bat=ibl[atbat];
//	runadv();
	onbase[0]=bat->up();
	frameput();
	return 1; 
    }
    else if (!(strcmp(event,"eg"))) {
#ifdef DEBUG
	print_linescore(stderr);
#endif
	cont=0;
	if (errflag && runs) pit->unearned(inning);

        sprintf(tempstr,"%s %d %s %d\n",ibl[0]->nout(),ibl[0]->score,
                                        ibl[1]->nout(),ibl[1]->score);
        outbuf(pbpfp,tempstr,"\n");
        outbuf(pbpfp,"","\n");
	outbuf(pbpfp," ");

	linescore[atbat][inning-1] = runs;
	for (i=1; i<4; i++)
	    if (onbase[i]) bat->lob++;

	ibl[0]->decisions();
	ibl[1]->decisions();

	return 1;
    }
    else if (!(strcmp(event,"cm"))) {
	fprintf(output,"Input a one line comment\n");
	fgets(comment,MAX_INPUT,input);
	fprintf(cmdfp,"%s",comment);
	frameput();
	comment[strlen(comment)-1] = ' ';
        outbuf(pbpfp,comment);
	return 1;
    }
    else if (!(strcmp(event,"so"))) {
	runcat(0);
	if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s K%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        pit->mound->k++;
        onbase[0]->k++;
        onbase[0]->ab++;
        onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"kd"))) {
	runcat(1);
	if (!(runchck(baserunning))) return 0;
	sprintf(tempstr,"%s dropped K%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats(2);
        pit->mound->k++;
        onbase[0]->k++;
        onbase[0]->ab++;
        onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"bb"))) {
	runcat(-1);
	if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s BB",onbase[0]->nout());
        outbuf(pbpfp,tempstr);
        runstats();
        pit->mound->bb++;
        onbase[0]->bb++;
        onbase[0]->pa(pit->mound->throws);
        rbi();
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"iw"))) {
        runcat(-1);
	if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s IBB",onbase[0]->nout());
        outbuf(pbpfp,tempstr);
        runstats();
        bat->newstat(onbase[0]->nout(),7);
        pit->mound->bb++;
        onbase[0]->bb++;
        onbase[0]->pa(pit->mound->throws);
        rbi();
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"ci"))) {
	errflag=1;
        runcat(-1);
	if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s CI",onbase[0]->nout());
        outbuf(pbpfp,tempstr);
        runstats(2);
        pit->errors++;
        pit->newstat(pit->posout(2),0);
        onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"hp")) || !(strcmp(event,"hb"))) {
        runcat(-1);
	if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s HBP",onbase[0]->nout());
        outbuf(pbpfp,tempstr);
        runstats();
        bat->newstat(onbase[0]->nout(),5);
        onbase[0]->pa(pit->mound->throws);
        rbi();
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"wp"))) {
	runcat(-2);
	if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"WP");
        outbuf(pbpfp,tempstr);
        runstats();
        pit->newstat(pit->mound->nout(),6);
	runadv();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"pb"))) {
	errflag=1;
        runcat(-2);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"PB");
        outbuf(pbpfp,tempstr);
        runstats();
        pit->errors++;
        runadv();
        frameput();
        return 1;
    }
    else if (!(strcmp(event,"bk"))) {
	runcat(-2);
	if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"Balk");
        outbuf(pbpfp,tempstr);
        runstats();
        pit->newstat(pit->mound->nout(),8);
	runadv();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"sb"))) {
	// if (!(three())) return 0;
	if (!(*baserunning))
	    runcat(-4);
	if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"SB");
        outbuf(pbpfp,tempstr);
        runstats();
        onbase[baserunning[0]-'0']->sb++;

	// Double steal check
	if (strlen(baserunning) > 2)
	    if (((i=baserunning[2]-'0') > 0) && (i < 4)) 
		onbase[i]->sb++;
	// Triple steal check
	if (strlen(baserunning) > 4)
	    if (((j=baserunning[4]-'0') > 0) && (j < 4)) 
		onbase[j]->sb++;
	runadv();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"th"))) {
	// if (!(three())) return 0;
	if (!(*baserunning))
	    runcat(-2);
	if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"On throw");
        outbuf(pbpfp,tempstr);
        runstats();
        rbi();
	runadv();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"cs"))) {
	// if (!(three())) return 0;
	if (!(*baserunning))
	    runcat(-3);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"CS%c%s",spc,location);
        outbuf(pbpfp,tempstr);
        if (!(runners->queue_empty())) runners->dequeue();
        runstats();
        onbase[baserunning[0]-'0']->cs++;
	runadv();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"kc"))) {
        if (!(*baserunning))
	    runcat(-3);
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s K, CS%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        if (!(runners->queue_empty())) runners->dequeue();
        onbase[baserunning[0]-'0']->cs++;
        pit->mound->k++;
        onbase[0]->k++;
        onbase[0]->ab++;
        onbase[0]->pa(pit->mound->throws);
        runadv();
        batterup();
        frameput();
        return 1;
    }
    else if (!(strcmp(event,"ks"))) {
	if (!(*baserunning))
	    runcat(-4);
	if (!(runchck(baserunning))) return 0;
	runcat(0);
        sprintf(tempstr,"%s K, SB%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        pit->mound->k++;
        onbase[0]->k++;
        onbase[0]->ab++;
        onbase[0]->pa(pit->mound->throws);
        onbase[baserunning[0]-'0']->sb++;
	{ 
	    int i;
	    // Double steal check
	    if (((i=baserunning[2]-'0') > 0) && (i < 4)) onbase[i]->sb++;
	    // Triple steal check
	    if (((i=baserunning[4]-'0') > 0) && (i < 4)) onbase[i]->sb++;
	}
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"pk"))) {
	// if (!(three())) return 0;
	if (!(*baserunning))
	    runcat(-3);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"Pickoff%c%s",spc,location);
        outbuf(pbpfp,tempstr);
        if (!(runners->queue_empty())) runners->dequeue();
        runstats();
	runadv();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"oa"))) {
	// if (!(three())) return 0;
	if (!(*baserunning))
	    runcat(-3);
	if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"Trying to advance%c%s",spc,location);
        outbuf(pbpfp,tempstr);
        if (!(runners->queue_empty())) runners->dequeue();
        runstats();
	runadv();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"1b"))) {
	runcat(1);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s 1b%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        pit->mound->h++;
        onbase[0]->h++;
        onbase[0]->pa(pit->mound->throws);
        rbi();
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"2b"))) {
	runcat(2);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s 2b%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
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
	return 1;
    }
    else if (!(strcmp(event,"3b"))) {
	runcat(3);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s 3b%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
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
	return 1;
    }
    else if (!(strcmp(event,"hr"))) {
	runcat(4);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s HR%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
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
	return 1;
    }
    else if (!(strcmp(event,"gd")) || !(strcmp(event,"dp"))) {
	if (!(*baserunning))
	    strcat(baserunning,"1o");
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s GDP%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        if (!(runners->queue_empty())) runners->dequeue();
        runstats();
        bat->newstat(onbase[0]->nout(),2);
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"fd"))) {
	if (!(*baserunning))
	    strcat(baserunning,"1o");
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s FDP%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        if (!(runners->queue_empty())) runners->dequeue();
        runstats();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"ld"))) {
	if (!(*baserunning))
	    strcat(baserunning,"1o");
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s LDP%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        if (!(runners->queue_empty())) runners->dequeue();
        runstats();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"lo"))) {
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s line drive%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        rbi();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"fc"))) {
	if (!(*baserunning))
	    strcat(baserunning,"b11o");
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s FC%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats(1);
        rbi();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"hg"))) {
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s HG%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        rbi();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"rg"))) {
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s RG%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        rbi();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"go"))) {
        runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s ground out%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        rbi();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
        runadv();
        batterup();
        frameput();
        return 1;
    }
    else if (!(strcmp(event,"sg"))) {
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s SG%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        rbi();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"hf"))) {
        runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s HF%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        rbi();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
        batterup();
        frameput();
        return 1;
    }
    else if (!(strcmp(event,"po"))) {
        runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s PO%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        rbi();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
        batterup();
        frameput();
        return 1;
    }
    else if (!(strcmp(event,"fp")) || !(strcmp(event,"pf"))) {
        runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s foul PO%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        rbi();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
        runadv();
        batterup();
        frameput();
        return 1;
    }
    else if (!(strcmp(event,"sf"))) {
	strcat(baserunning,"3h");
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s SF%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        bat->newstat(onbase[0]->nout(),4);
        rbi();
        onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"sh"))) {
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s Sac%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats(2);
        bat->newstat(onbase[0]->nout(),3);
        rbi();
        onbase[0]->pa(pit->mound->throws);
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"lf"))) {
	runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s LF%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"df")) || !(strcmp(event,"wt"))) {
        runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s WT%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
        runadv();
        batterup();
        frameput();
        return 1;
    }
    else if (!(strcmp(event,"fo"))) {
        runcat(0);
        if (!(runchck(baserunning))) return 0;
        sprintf(tempstr,"%s fly out%c%s",onbase[0]->nout(),spc,location);
        outbuf(pbpfp,tempstr);
        runstats();
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
        runadv();
        batterup();
        frameput();
        return 1;
    }
    else if ( !(strcmp(event,"er")) ) {
	errflag = 1;
	runcat(1);
        if ( !(runchck(baserunning)) ) 
	    return 0;
	if ( strlen(location) > 0 ) {
	    if ( location[0] < '1' || location[0] > '9' ) {
		sprintf( error, "\"%s\" invalid.  First character must be fielder's position (1-9).", location );
		return 0;
	    }
	}
	else {
	    sprintf( error, "Must supply fielder's position (1-9)." );
	    return 0;
	}
        sprintf(tempstr, "%s safe on E%c%s", onbase[0]->nout(), spc, location);
        outbuf(pbpfp, tempstr);
        runstats(2);
        pit->errors++;
        who_stat(0, (int) location[0] - '0');
        onbase[0]->pa(pit->mound->throws);
        onbase[0]->ab++;
	runadv();
	batterup();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"ea"))) {
	errflag = 1;
	// if (!(three())) return 0;
	if ( !(*baserunning) )
	    runcat(-2);
        if ( !(runchck(baserunning)) ) 
	    return 0;
	if ( strlen(location) > 0 ) {
	    if ( location[0] < '1' || location[0] > '9' ) {
		sprintf( error, "\"%s\" invalid.  First character must be fielder's position (1-9).", location );
		return 0;
	    }
	}
	else {
	    sprintf( error, "Must supply fielder's position (1-9)." );
	    return 0;
	}
        sprintf(tempstr, "Error%c%s", spc, location);
        outbuf(pbpfp, tempstr);
        runstats();
        pit->errors++;
        who_stat(0, (int) location[0] - '0');
	runadv();
	frameput();
	return 1;
    }
    else if (!(strcmp(event,"nj"))) {
	{ 
	    int i;
	    i=atoi(location);
	    if (i<1 || i>3)
		if (onbase[3]) i=3;
		else if (onbase[2]) i=2;
		else if (onbase[1]) i=1;
		else i=0;
	    if (!i || !(onbase[i])) return 0; 
	    sprintf(tempstr,"%s can't get jump. ",onbase[i]->nout());
	    outbuf(pbpfp,tempstr);
	    frameput();
	}
	return 1;
    }
    else if ( !(strcmp(event,"fa")) ) {
	sprintf( tempstr, "%s fatigues. ", pit->mound->nout() );
        outbuf( pbpfp, tempstr );
	frameput();
	return 1;
    }
    else if ( !(strcmp(event,"ic")) ) {
	sprintf( tempstr, "Infield in @ 1b/3b. " );
        outbuf( pbpfp, tempstr );
	frameput();
	return 1;
    }
    else if ( !(strcmp(event,"in")) ) {
	if ( strlen(location) > 0 )
	    sprintf( tempstr, "Infield in @ %s. ", location );
	else
	    sprintf( tempstr, "Infield in. " );
        outbuf( pbpfp, tempstr );
	frameput();
	return 1;
    }
    else 
	return 0;
}

    int 
frame::backup(char *infile, char *outfile)
{
   int result;
   char tempstr[MAX_INPUT];
   char nextstr[MAX_INPUT];

   frame *test;

    strcpy(tempstr,filename);
    strcat(tempstr,outfile);
    if ((undofp=fopen(tempstr,"w")) == NULL) {
	fprintf(stderr,"fatal error - can't open undo file\n");
	exit(1);
    }
    strcpy(tempstr,filename);
    strcat(tempstr,infile);
    cmdfp=fopen(tempstr,"r");
    fgets(tempstr,MAX_INPUT,cmdfp);
    fgets(nextstr,MAX_INPUT,cmdfp);
    while (!(feof(cmdfp))) {
	fputs(tempstr,undofp);
	strcpy(tempstr,nextstr);
	fgets(nextstr,MAX_INPUT,cmdfp);
    }
    test=new frame(tempstr);
    result=test->decode();
    delete(test);
    return result;
}

