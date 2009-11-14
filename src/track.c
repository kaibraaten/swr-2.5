#include <stdio.h>
#include <string.h>
#include "mud.h"

#define BFS_ERROR	   -1
#define BFS_ALREADY_THERE  -2
#define BFS_NO_PATH	   -3
#define BFS_MARK         BV01

#define TRACK_THROUGH_DOORS

bool mob_snipe( CHAR_DATA * ch, CHAR_DATA * victim );
ch_ret one_hit( CHAR_DATA * ch, CHAR_DATA * victim, int dt );

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/

struct bfs_queue_struct
{
  ROOM_INDEX_DATA *room;
  char dir;
  struct bfs_queue_struct *next;
};

static struct bfs_queue_struct *queue_head = NULL,
  *queue_tail = NULL, *room_queue = NULL;

/* Utility macros */
#define MARK(room)	(SET_BIT(	(room)->room_flags, BFS_MARK) )
#define UNMARK(room)	(REMOVE_BIT(	(room)->room_flags, BFS_MARK) )
#define IS_MARKED(room)	(IS_SET(	(room)->room_flags, BFS_MARK) )

static ROOM_INDEX_DATA *toroom( ROOM_INDEX_DATA * room, short door )
{
  return ( get_exit( room, door )->to_room );
}

static bool valid_edge( ROOM_INDEX_DATA * room, short door )
{
  EXIT_DATA *pexit = get_exit( room, door );
  ROOM_INDEX_DATA *to_room = NULL;

  if( pexit && ( to_room = pexit->to_room ) != NULL
#ifndef TRACK_THROUGH_DOORS
      && !IS_SET( pexit->exit_info, EX_CLOSED )
#endif
      && !IS_MARKED( to_room ) )
    return TRUE;
  else
    return FALSE;
}

static void bfs_enqueue( ROOM_INDEX_DATA * room, char dir )
{
  struct bfs_queue_struct *curr = NULL;

  CREATE( curr, struct bfs_queue_struct, 1 );
  curr->room = room;
  curr->dir = dir;
  curr->next = NULL;

  if( queue_tail )
    {
      queue_tail->next = curr;
      queue_tail = curr;
    }
  else
    queue_head = queue_tail = curr;
}

static void bfs_dequeue( void )
{
  struct bfs_queue_struct *curr = queue_head;

  if( !( queue_head = queue_head->next ) )
    queue_tail = NULL;

  DISPOSE( curr );
}

static void bfs_clear_queue( void )
{
  while( queue_head )
    bfs_dequeue(  );
}

static void room_enqueue( ROOM_INDEX_DATA * room )
{
  struct bfs_queue_struct *curr;

  CREATE( curr, struct bfs_queue_struct, 1 );
  curr->room = room;
  curr->next = room_queue;

  room_queue = curr;
}

static void clean_room_queue( void )
{
  struct bfs_queue_struct *curr, *curr_next;

  for( curr = room_queue; curr; curr = curr_next )
    {
      UNMARK( curr->room );
      curr_next = curr->next;
      DISPOSE( curr );
    }

  room_queue = NULL;
}


static int find_first_step( ROOM_INDEX_DATA * src,
			    ROOM_INDEX_DATA * target, int maxdist )
{
  int curr_dir = 0, count = 0;

  if( !src || !target )
    {
      bug( "Illegal value passed to find_first_step (track.c)", 0 );
      return BFS_ERROR;
    }

  if( src == target )
    return BFS_ALREADY_THERE;

  if( src->area != target->area )
    return BFS_NO_PATH;

  room_enqueue( src );
  MARK( src );

  /* first, enqueue the first steps, saving which direction we're going. */
  for( curr_dir = 0; curr_dir < 10; curr_dir++ )
    if( valid_edge( src, curr_dir ) )
      {
	MARK( toroom( src, curr_dir ) );
	room_enqueue( toroom( src, curr_dir ) );
	bfs_enqueue( toroom( src, curr_dir ), curr_dir );
      }

  count = 0;
  while( queue_head )
    {
      if( ++count > maxdist )
	{
	  bfs_clear_queue(  );
	  clean_room_queue(  );
	  return BFS_NO_PATH;
	}

      if( queue_head->room == target )
	{
	  curr_dir = queue_head->dir;
	  bfs_clear_queue(  );
	  clean_room_queue(  );
	  return curr_dir;
	}
      else
	{
	  for( curr_dir = 0; curr_dir < 10; curr_dir++ )
	    if( valid_edge( queue_head->room, curr_dir ) )
	      {
		MARK( toroom( queue_head->room, curr_dir ) );
		room_enqueue( toroom( queue_head->room, curr_dir ) );
		bfs_enqueue( toroom( queue_head->room, curr_dir ),
			     queue_head->dir );
	      }
	  bfs_dequeue(  );
	}
    }
  clean_room_queue(  );

  return BFS_NO_PATH;
}


void do_track( CHAR_DATA * ch, char *argument )
{
  CHAR_DATA *vict;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int dir, maxdist;

  if( !character_skill_level( ch, gsn_track ) )
    {
      send_to_char( "You do not know of this skill yet.\n\r", ch );
      return;
    }

  one_argument( argument, arg );
  if( arg[0] == '\0' )
    {
      send_to_char( "Whom are you trying to track?\n\r", ch );
      return;
    }

  WAIT_STATE( ch, skill_table[gsn_track]->beats );

  if( !( vict = get_char_world( ch, arg ) ) )
    {
      send_to_char( "You can't find a trail of anyone like that.\n\r", ch );
      return;
    }

  maxdist = 100 + ch->top_level * 30;

  if( !IS_NPC( ch ) )
    maxdist = ( maxdist * character_skill_level( ch, gsn_track ) ) / 100;

  dir = find_first_step( ch->in_room, vict->in_room, maxdist );
  switch ( dir )
    {
    case BFS_ERROR:
      send_to_char( "Hmm... something seems to be wrong.\n\r", ch );
      break;
    case BFS_ALREADY_THERE:
      send_to_char( "You're already in the same room!\n\r", ch );
      break;
    case BFS_NO_PATH:
      sprintf( buf, "You can't sense a trail from here.\n\r" );
      send_to_char( buf, ch );
      learn_from_failure( ch, gsn_track );
      break;
    default:
      ch_printf( ch, "You sense a trail %s from here...\n\r", dir_name[dir] );
      learn_from_success( ch, gsn_track );
      break;
    }
}


void found_prey( CHAR_DATA * ch, CHAR_DATA * victim )
{
  char buf[MAX_STRING_LENGTH];
  char victname[MAX_STRING_LENGTH];

  if( victim == NULL )
    {
      bug( "Found_prey: null victim", 0 );
      return;
    }

  if( ch == NULL )
    {
      bug( "Found_prey: null ch", 0 );
      return;
    }

  if( victim->in_room == NULL )
    {
      bug( "Found_prey: null victim->in_room", 0 );
      return;
    }

  sprintf( victname, "%s",
	   IS_NPC( victim ) ? victim->short_descr : victim->name );

  if( !can_see( ch, victim ) )
    {
      if( number_percent(  ) < 90 )
	return;
      switch ( number_bits( 2 ) )
	{
	case 0:
	  sprintf( buf, "Don't make me find you, %s!", victname );
	  do_say( ch, buf );
	  break;
	case 1:
	  act( AT_ACTION, "$n sniffs around the room for $N.", ch, NULL,
	       victim, TO_NOTVICT );
	  act( AT_ACTION, "You sniff around the room for $N.", ch, NULL,
	       victim, TO_CHAR );
	  act( AT_ACTION, "$n sniffs around the room for you.", ch, NULL,
	       victim, TO_VICT );
	  sprintf( buf, "I can smell your blood!" );
	  do_say( ch, buf );
	  break;
	case 2:
	  sprintf( buf, "I'm going to tear %s apart!", victname );
	  do_yell( ch, buf );
	  break;
	case 3:
	  do_say( ch,
		  const_char_to_nonconst( "Just wait until I find you..." ) );
	  break;
	}
      return;
    }

  if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
      if( number_percent(  ) < 90 )
	return;
      switch ( number_bits( 2 ) )
	{
	case 0:
	  do_say( ch, const_char_to_nonconst( "C'mon out, you coward!" ) );
	  sprintf( buf, "%s is a bloody coward!", victname );
	  do_yell( ch, buf );
	  break;
	case 1:
	  sprintf( buf, "Let's take this outside, %s", victname );
	  do_say( ch, buf );
	  break;
	case 2:
	  sprintf( buf, "%s is a yellow-bellied wimp!", victname );
	  do_yell( ch, buf );
	  break;
	case 3:
	  act( AT_ACTION, "$n takes a few swipes at $N.", ch, NULL, victim,
	       TO_NOTVICT );
	  act( AT_ACTION, "You try to take a few swipes $N.", ch, NULL,
	       victim, TO_CHAR );
	  act( AT_ACTION, "$n takes a few swipes at you.", ch, NULL, victim,
	       TO_VICT );
	  break;
	}
      return;
    }

  act( AT_ACTION, "$n lunges at $N from out of nowhere!", ch, NULL, victim,
       TO_NOTVICT );
  act( AT_ACTION, "You lunge at $N catching $M off guard!", ch, NULL, victim,
       TO_CHAR );
  act( AT_ACTION, "$n lunges at you from out of nowhere!", ch, NULL, victim,
       TO_VICT );

  stop_hunting( ch );
  set_fighting( ch, victim );
  multi_hit( ch, victim, TYPE_UNDEFINED );
  return;
}

void hunt_victim( CHAR_DATA * ch )
{
  bool found;
  CHAR_DATA *tmp;
  short ret;

  if( !ch || !ch->hunting || !ch->hunting->who )
    return;

  /* make sure the char still exists */
  for( found = FALSE, tmp = first_char; tmp && !found; tmp = tmp->next )
    if( ch->hunting->who == tmp )
      found = TRUE;

  if( !found )
    {
      do_say( ch, const_char_to_nonconst( "Damn! My prey is gone!!" ) );
      stop_hunting( ch );
      return;
    }

  if( ch->in_room == ch->hunting->who->in_room )
    {
      if( ch->fighting )
	return;
      found_prey( ch, ch->hunting->who );
      return;
    }

/* hunting with snipe */
  {
    OBJ_DATA *wield;

    wield = get_eq_char( ch, WEAR_WIELD );
    if( wield != NULL && wield->value[3] == WEAPON_BLASTER )
      {
	if( mob_snipe( ch, ch->hunting->who ) == TRUE )
	  return;
      }
    else if( !IS_SET( ch->act, ACT_DROID ) )
      do_hide( ch, const_char_to_nonconst( "" ) );
  }

  ret = find_first_step( ch->in_room, ch->hunting->who->in_room, 5000 );
  if( ret == BFS_NO_PATH )
    {
      EXIT_DATA *pexit;
      int attempt;

      for( attempt = 0; attempt < 25; attempt++ )
	{
	  ret = number_door(  );
	  if( ( pexit = get_exit( ch->in_room, ret ) ) == NULL
	      || !pexit->to_room
	      || IS_SET( pexit->exit_info, EX_CLOSED )
	      || IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) )
	    continue;
	}
    }
  if( ret < 0 )
    {
      do_say( ch, const_char_to_nonconst( "Damn! Lost my prey!" ) );
      stop_hunting( ch );
      return;
    }
  else
    {
      move_char( ch, get_exit( ch->in_room, ret ), FALSE );
      if( char_died( ch ) )
	return;
      if( !ch->hunting )
	{
	  if( !ch->in_room )
	    {
	      char buf[MAX_STRING_LENGTH];
	      sprintf( buf,
		       "Hunt_victim: no ch->in_room!  Mob #%ld, name: %s.  Placing mob in limbo.",
		       ch->pIndexData->vnum, ch->name );
	      bug( buf, 0 );
	      char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
	      return;
	    }
	  do_say( ch, const_char_to_nonconst( "Damn! Lost my prey!" ) );
	  return;
	}
      if( ch->in_room == ch->hunting->who->in_room )
	found_prey( ch, ch->hunting->who );
      return;
    }
}

bool mob_snipe( CHAR_DATA * ch, CHAR_DATA * victim )
{
  short dir, dist;
  short max_dist = 3;
  EXIT_DATA *pexit;
  ROOM_INDEX_DATA *was_in_room;
  ROOM_INDEX_DATA *to_room;
  char buf[MAX_STRING_LENGTH];
  bool pfound = FALSE;

  if( !ch->in_room || !victim->in_room )
    return FALSE;

  if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    return FALSE;

  for( dir = 0; dir <= 10; dir++ )
    {
      if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
	continue;

      if( IS_SET( pexit->exit_info, EX_CLOSED ) )
	continue;

      was_in_room = ch->in_room;

      for( dist = 0; dist <= max_dist; dist++ )
	{
	  if( IS_SET( pexit->exit_info, EX_CLOSED ) )
	    break;

	  if( !pexit->to_room )
	    break;

	  to_room = pexit->to_room;

	  char_from_room( ch );
	  char_to_room( ch, to_room );

	  if( ch->in_room == victim->in_room )
	    {
	      pfound = TRUE;
	      break;
	    }

	  if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
	    break;
	}

      char_from_room( ch );
      char_to_room( ch, was_in_room );

      if( !pfound )
	{
	  char_from_room( ch );
	  char_to_room( ch, was_in_room );
	  continue;
	}

      if( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
	return FALSE;

      if( is_safe( ch, victim ) )
	return FALSE;

      if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
	return FALSE;

      if( ch->position == POS_FIGHTING )
	return FALSE;

      switch ( dir )
	{
	case DIR_NORTH:
	case DIR_EAST:
	  dir += 2;
	  break;
	case DIR_SOUTH:
	case DIR_WEST:
	  dir -= 2;
	  break;
	case DIR_UP:
	case DIR_NORTHWEST:
	  dir += 1;
	  break;
	case DIR_DOWN:
	case DIR_SOUTHEAST:
	  dir -= 1;
	  break;
	case DIR_NORTHEAST:
	  dir += 3;
	  break;
	case DIR_SOUTHWEST:
	  dir -= 3;
	  break;
	}

      char_from_room( ch );
      char_to_room( ch, victim->in_room );

      sprintf( buf, "A blaster shot fires at you from the %s.",
	       dir_name[dir] );
      act( AT_ACTION, buf, victim, NULL, ch, TO_CHAR );
      act( AT_ACTION, "You fire at $N.", ch, NULL, victim, TO_CHAR );
      sprintf( buf, "A blaster shot fires at $N from the %s.",
	       dir_name[dir] );
      act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );

      one_hit( ch, victim, TYPE_UNDEFINED );

      if( char_died( ch ) )
	return TRUE;

      stop_fighting( ch, TRUE );

      if( victim && !char_died( victim ) && victim->hit < 0 )
	{
	  stop_hunting( ch );
	  stop_hating( ch );
	}

      char_from_room( ch );
      char_to_room( ch, was_in_room );

      return TRUE;
    }

  return FALSE;
}
