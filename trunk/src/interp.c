#include <sys/types.h>
#ifndef WIN32
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"

/*
 * Externals
 */
void subtract_times( struct timeval *etime, struct timeval *stime );
bool	check_social	args( ( CHAR_DATA *ch, const char *command,
			    char *argument ) );

/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;
CMDTYPE	   *command_hash[126];	/* hash table for cmd_table */
SOCIALTYPE *social_index[27];   /* hash table for socials   */

/*
 * Character not in position for command?
 */
bool check_pos( const CHAR_DATA *ch, short position )
{
  if ( ch->position < position )
    {
      switch( ch->position )
	{
	case POS_DEAD:
	  send_to_char( "A little difficult to do when you are DEAD...\r\n", ch );
	  break;

	case POS_MORTAL:
	case POS_INCAP:
	  send_to_char( "You are hurt far too bad for that.\r\n", ch );
	  break;

	case POS_STUNNED:
	  send_to_char( "You are too stunned to do that.\r\n", ch );
	  break;

	case POS_SLEEPING:
	  send_to_char( "In your dreams, or what?\r\n", ch );
	  break;

	case POS_RESTING:
	  send_to_char( "Nah... You feel too relaxed...\r\n", ch);
	  break;

	case POS_SITTING:
	  send_to_char( "You can't do that sitting down.\r\n", ch);
	  break;

	case POS_FIGHTING:
	  send_to_char( "No way!  You are still fighting!\r\n", ch);
	  break;
	}

      return FALSE;
    }

  return TRUE;
}

extern char lastplayercmd[MAX_INPUT_LENGTH*2];

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    char logname[MAX_INPUT_LENGTH];
    TIMER *timer = NULL;
    CMDTYPE *cmd = NULL;
    int trust;
    int loglvl;
    bool found;
    struct timeval time_used;
    long tmptime;


    if ( !ch )
    {
	bug( "interpret: null ch!", 0 );
	return;
    }

    found = FALSE;

    if ( !cmd )
    {
	/* Changed the order of these ifchecks to prevent crashing. */
	if ( !argument || !strcmp(argument,"") ) 
	{
	    bug( "interpret: null argument!", 0 );
	    return;
	}

	/*
	 * Strip leading spaces.
	 */
	while ( isspace((int) *argument) )
	    argument++;
	if ( argument[0] == '\0' )
	    return;

	timer = get_timerptr( ch, TIMER_DO_FUN );

	/* REMOVE_BIT( ch->affected_by, AFF_HIDE ); */

	/*
	 * Implement freeze command.
	 */
	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
	{
	    send_to_char( "You're totally frozen!\r\n", ch );
	    return;
	}

	/*
	 * Grab the command word.
	 * Special parsing so ' can be a command,
	 *   also no spaces needed after punctuation.
	 */
	strcpy( logline, argument );
	if ( !isalpha((int) argument[0]) && !isdigit((int) argument[0]) )
	{
	    command[0] = argument[0];
	    command[1] = '\0';
	    argument++;
	    while ( isspace((int) *argument) )
		argument++;
	}
	else
	    argument = one_argument( argument, command );

	/*
	 * Look for command in command table.
	 * Check for council powers and/or bestowments
	 */
	trust = get_trust( ch );
	for ( cmd = command_hash[LOWER(command[0])%126]; cmd; cmd = cmd->next )
	    if ( !str_prefix( command, cmd->name )
	    &&   (cmd->level <= trust
	    ||  (!IS_NPC(ch) && ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0'
	    &&    is_name( cmd->name, ch->pcdata->bestowments )
	    &&    cmd->level <= (trust+5)) ) )
	    {
		found = TRUE;
		break;
	    }

	/*
	 * Turn off afk bit when any command performed.
	 */
	if ( IS_SET ( ch->act, PLR_AFK)  && (str_cmp(command, "AFK")))
	{
	    REMOVE_BIT( ch->act, PLR_AFK );
     	    act( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_ROOM );
	}
    }

    /*
     * Log and snoop.
     */
    sprintf( lastplayercmd, "** %s: %s", ch->name, logline );

    if ( found && cmd->log == LOG_NEVER )
	strcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX" );

    loglvl = found ? cmd->log : LOG_NORMAL;

    if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||	 loglvl == LOG_BUILD
    ||   loglvl == LOG_HIGH
    ||   loglvl == LOG_ALWAYS )
    {
        /* Added by Narn to show who is switched into a mob that executes
           a logged command.  Check for descriptor in case force is used. */
        if ( ch->desc && ch->desc->original ) 
          sprintf( log_buf, "Log %s (%s): %s", ch->name,
                   ch->desc->original->name, logline );
        else
          sprintf( log_buf, "Log %s: %s", ch->name, logline );

	/*
	 * Make it so a 'log all' will send most output to the log
	 * file only, and not spam the log channel to death	-Thoric
	 */
	if ( fLogAll && loglvl == LOG_NORMAL
	&&  (IS_NPC(ch) || !IS_SET(ch->act, PLR_LOG)) )
	  loglvl = LOG_ALL;

	  log_string_plus( log_buf, loglvl );
    }

    if ( ch->desc && ch->desc->snoop_by )
    {
  	sprintf( logname, "%s", ch->name);
	write_to_buffer( ch->desc->snoop_by, logname, 0 );
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\r\n",  2 );
    }

    

    if ( timer )
    {
	int tempsub;

	tempsub = ch->substate;
	ch->substate = SUB_TIMER_DO_ABORT;
	(timer->do_fun)(ch,const_char_to_nonconst(""));
	if ( char_died(ch) )
	  return;
	if ( ch->substate != SUB_TIMER_CANT_ABORT )
	{
	  ch->substate = tempsub;
	  extract_timer( ch, timer );
	}
	else
	{
	  ch->substate = tempsub;
	  return;
	}
    }

    /*
     * Look for command in skill and socials table.
     */
    if ( !found )
    {
	if ( !check_skill( ch, command, argument )
	&&   !check_social( ch, command, argument ) )
	{
	    EXIT_DATA *pexit;

	    /* check for an auto-matic exit command */
	    if ( (pexit = find_door( ch, command, TRUE )) != NULL
	    &&   IS_SET( pexit->exit_info, EX_xAUTO ))
	    {
		if ( IS_SET(pexit->exit_info, EX_CLOSED)
		&& (!IS_AFFECTED(ch, AFF_PASS_DOOR)
		||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
		{
		  if ( !IS_SET( pexit->exit_info, EX_SECRET ) )
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
		  else
		    send_to_char( "You cannot do that here.\r\n", ch );
		  return;
		}
		move_char( ch, pexit, 0 );
		return;
	    }
	    send_to_char( "Huh?\r\n", ch );
	}
	return;
    }

    /*
     * Character not in position for command?
     */
    if ( !check_pos( ch, cmd->position ) )
	return;
    
    /* Berserk check for flee.. maybe add drunk to this?.. but too much
       hardcoding is annoying.. -- Altrag */
    if ( !str_cmp(cmd->name, "flee") &&
          IS_AFFECTED(ch, AFF_BERSERK) )
    {
	send_to_char( "You aren't thinking very clearly..\r\n", ch);
	return;
    }

    /*
     * Dispatch the command.
     */
    ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
    ch->last_cmd = cmd->do_fun;
    start_timer(&time_used);
    (*cmd->do_fun) ( ch, argument );
    end_timer(&time_used);
    /*
     * Update the record of how many times this command has been used (haus)
     */
    update_userec(&time_used, &cmd->userec);
    tmptime = UMIN(time_used.tv_sec,19) * 1000000 + time_used.tv_usec;

    /* laggy command notice: command took longer than 1.5 seconds */
    if ( tmptime > 1500000 )
    {
        sprintf(log_buf, "[*****] LAG: %s: %s %s (R:%ld S:%d.%06d)", ch->name,
                cmd->name, (cmd->log == LOG_NEVER ? "XXX" : argument),
		ch->in_room ? ch->in_room->vnum : 0,
		(int) (time_used.tv_sec),(int) (time_used.tv_usec) );
	log_string_plus(log_buf, LOG_NORMAL);
    }
}

CMDTYPE *find_command( const char *command )
{
    CMDTYPE *cmd;
    int hash;

    hash = LOWER(command[0]) % 126;

    for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	if ( !str_prefix( command, cmd->name ) )
	    return cmd;

    return NULL;
}

SOCIALTYPE *find_social( const char *command )
{
    SOCIALTYPE *social;
    int hash;

    if ( command[0] < 'a' || command[0] > 'z' )
	hash = 0;
    else
	hash = (command[0] - 'a') + 1;

    for ( social = social_index[hash]; social; social = social->next )
	if ( !str_prefix( command, social->name ) )
	    return social;

    return NULL;
}

bool check_social( CHAR_DATA *ch, const char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    SOCIALTYPE *social;

    if ( (social=find_social(command)) == NULL )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE) )
    {
	send_to_char( "You are anti-social!\r\n", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\r\n", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\r\n", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\r\n", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social->name, "snore" ) )
	    break;
	send_to_char( "In your dreams, or what?\r\n", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( AT_SOCIAL, social->others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( AT_SOCIAL, social->char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
    }
    else if ( victim == ch )
    {
	act( AT_SOCIAL, social->others_auto,   ch, NULL, victim, TO_ROOM    );
	act( AT_SOCIAL, social->char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( AT_SOCIAL, social->others_found,  ch, NULL, victim, TO_NOTVICT );
	act( AT_SOCIAL, social->char_found,    ch, NULL, victim, TO_CHAR    );
	act( AT_SOCIAL, social->vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) 
        &&   !IS_SET( victim->pIndexData->progtypes, ACT_PROG ) )
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:
		if ( !IS_SET(ch->in_room->room_flags, ROOM_SAFE )
		||    IS_EVIL(ch) )
		  multi_hit( victim, ch, TYPE_UNDEFINED );
		else
		if ( IS_NEUTRAL(ch) )
		{
		    act( AT_ACTION, "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		    act( AT_ACTION, "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		    act( AT_ACTION, "$n slaps you.", victim, NULL, ch, TO_VICT    );
		}
		else
		{
		    act( AT_ACTION, "$n acts like $N doesn't even exist.",  victim, NULL, ch, TO_NOTVICT );
		    act( AT_ACTION, "You just ignore $N.",  victim, NULL, ch, TO_CHAR    );
		    act( AT_ACTION, "$n appears to be ignoring you.", victim, NULL, ch, TO_VICT    );
		}
		break;

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( AT_SOCIAL, social->others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( AT_SOCIAL, social->char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( AT_SOCIAL, social->vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( AT_ACTION, "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( AT_ACTION, "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( AT_ACTION, "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}

void do_timecmd( CHAR_DATA *ch, char *argument )
{
  struct timeval stime;
  struct timeval etime;
  static bool timing;
  extern CHAR_DATA *timechar;
  char arg[MAX_INPUT_LENGTH];
  
  send_to_char("Timing\r\n",ch);
  if ( timing )
    return;
  one_argument(argument, arg);
  if ( !*arg )
  {
    send_to_char( "No command to time.\r\n", ch );
    return;
  }
  if ( !str_cmp(arg, "update") )
  {
    if ( timechar )
      send_to_char( "Another person is already timing updates.\r\n", ch );
    else
    {
      timechar = ch;
      send_to_char( "Setting up to record next update loop.\r\n", ch );
    }
    return;
  }
  set_char_color(AT_PLAIN, ch);
  send_to_char( "Starting timer.\r\n", ch );
  timing = TRUE;
  gettimeofday(&stime, NULL);
  interpret(ch, argument);
  gettimeofday(&etime, NULL);
  timing = FALSE;
  set_char_color(AT_PLAIN, ch);
  send_to_char( "Timing complete.\r\n", ch );
  subtract_times(&etime, &stime);
  ch_printf( ch, "Timing took %d.%06d seconds.\r\n",
      etime.tv_sec, etime.tv_usec );
  return;
}

void start_timer(struct timeval *stime)
{
  if ( !stime )
  {
    bug( "Start_timer: NULL stime.", 0 );
    return;
  }
  gettimeofday(stime, NULL);
  return;
}

time_t end_timer(struct timeval *stime)
{
  struct timeval etime;
  
  /* Mark etime before checking stime, so that we get a better reading.. */
  gettimeofday(&etime, NULL);
  if ( !stime || (!stime->tv_sec && !stime->tv_usec) )
  {
    bug( "End_timer: bad stime.", 0 );
    return 0;
  }
  subtract_times(&etime, stime);
  /* stime becomes time used */
  *stime = etime;
  return (etime.tv_sec*1000000)+etime.tv_usec;
}

void send_timer(struct timerset *vtime, const CHAR_DATA *ch)
{
  struct timeval ntime;
  int carry;
  
  if ( vtime->num_uses == 0 )
    return;

  ntime.tv_sec  = vtime->total_time.tv_sec / vtime->num_uses;
  carry = (vtime->total_time.tv_sec % vtime->num_uses) * 1000000;
  ntime.tv_usec = (vtime->total_time.tv_usec + carry) / vtime->num_uses;

  ch_printf(ch, "Has been used %d times this boot.\r\n", vtime->num_uses);
  ch_printf(ch, "Time (in secs): min %d.%0.6d; avg: %d.%0.6d; max %d.%0.6d"
	    "\r\n", vtime->min_time.tv_sec, vtime->min_time.tv_usec,
	    ntime.tv_sec,
	    ntime.tv_usec, vtime->max_time.tv_sec, vtime->max_time.tv_usec);
}

void update_userec(struct timeval *time_used, struct timerset *userec)
{
  userec->num_uses++;

  if ( !timerisset(&userec->min_time)
       || timercmp(time_used, &userec->min_time, <) )
    {
      userec->min_time.tv_sec  = time_used->tv_sec;
      userec->min_time.tv_usec = time_used->tv_usec;
    }

  if ( !timerisset(&userec->max_time)
       || timercmp(time_used, &userec->max_time, >) )
    {
      userec->max_time.tv_sec  = time_used->tv_sec;
      userec->max_time.tv_usec = time_used->tv_usec;
    }

  userec->total_time.tv_sec  += time_used->tv_sec;
  userec->total_time.tv_usec += time_used->tv_usec;

  while ( userec->total_time.tv_usec >= 1000000 )
    {
      userec->total_time.tv_sec++;
      userec->total_time.tv_usec -= 1000000;
    }
}
