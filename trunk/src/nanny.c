/*
 * Nanny refactoring by Kai Braaten.
 * The SWR2 license applies.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <arpa/telnet.h> // Telnet opt codes

#include "mud.h"

/*
 * Local function prototypes.
 */
static void nanny_get_name( DESCRIPTOR_DATA *d, char *argument );
static void nanny_get_old_password( DESCRIPTOR_DATA *d, char *argument );
static void nanny_confirm_new_name( DESCRIPTOR_DATA *d, char *argument );
static void nanny_get_new_password( DESCRIPTOR_DATA *d, char *argument );
static void nanny_confirm_new_password( DESCRIPTOR_DATA *d, char *argument );
static void nanny_get_new_sex( DESCRIPTOR_DATA *d, char *argument );
static void nanny_add_skills( DESCRIPTOR_DATA *d, char *argument );
static void nanny_get_want_ripansi( DESCRIPTOR_DATA *d, char *argument );
static void nanny_press_enter( DESCRIPTOR_DATA *d, char *argument );
static void nanny_read_imotd( DESCRIPTOR_DATA *d, char *argument );
static void nanny_read_nmotd( DESCRIPTOR_DATA *d, char *argument );
static void nanny_done_motd( DESCRIPTOR_DATA *d, char *argument );

#define MAX_NEST        100
static  OBJ_DATA *      rgObjNest       [MAX_NEST];
extern bool wizlock;
const   char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
//const   char    go_ahead_str    [] = { IAC, GA, '\0' };

/*
 * External functions.
 */
bool check_parse_name( const char *name );
bool check_playing( DESCRIPTOR_DATA *d, const char *name, bool kick );
bool check_multi( DESCRIPTOR_DATA *d, const char *name );
bool check_reconnect( DESCRIPTOR_DATA *d, const char *name, bool fConn );
void show_title( DESCRIPTOR_DATA *d );
void write_ship_list( void );
void mail_count( CHAR_DATA *ch );

/*
 * Main character generation function.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
  while ( isspace(*argument) )
    argument++;

  switch ( d->connected )
    {

    default:
      bug( "Nanny: bad d->connected %d.", d->connected );
      close_socket( d, TRUE );
      return;

    case CON_GET_NAME:
      nanny_get_name( d, argument );
      break;

    case CON_GET_OLD_PASSWORD:
      nanny_get_old_password( d, argument );
      break;

    case CON_CONFIRM_NEW_NAME:
      nanny_confirm_new_name( d, argument );
      break;

    case CON_GET_NEW_PASSWORD:
      nanny_get_new_password( d, argument );
      break;

    case CON_CONFIRM_NEW_PASSWORD:
      nanny_confirm_new_password( d, argument );
      break;

    case CON_GET_NEW_SEX:
      nanny_get_new_sex( d, argument );
      break;

    case CON_ADD_SKILLS:
      nanny_add_skills( d, argument );
      break;

    case CON_GET_WANT_RIPANSI:
      nanny_get_want_ripansi( d, argument );
      break;

    case CON_PRESS_ENTER:
      nanny_press_enter( d, argument );
      break;

    case CON_READ_IMOTD:
      nanny_read_imotd( d, argument );
      break;

    case CON_READ_NMOTD:
      nanny_read_nmotd( d, argument );
      break;

    case CON_DONE_MOTD:
      nanny_done_motd( d, argument );
      break;
    }
}

static void nanny_get_name( DESCRIPTOR_DATA *d, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  BAN_DATA *pban;
  bool fOld, chk;

  if ( argument[0] == '\0' )
    {
      close_socket( d, FALSE );
      return;
    }

  argument[0] = UPPER(argument[0]);

  if ( !check_parse_name( argument ) )
    {
      write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
      return;
    }

  if ( !str_cmp( argument, "New" ) )
    {
      if (d->newstate == 0)
	{
	  /* New player */
	  /* Don't allow new players if DENY_NEW_PLAYERS is true */
	  if (sysdata.DENY_NEW_PLAYERS == TRUE)
	    {
	      sprintf( buf, "The mud is currently preparing for a reboot.\n\r" );
	      write_to_buffer( d, buf, 0 );
	      sprintf( buf, "New players are not accepted during this time.\n\r" );
	      write_to_buffer( d, buf, 0 );
	      sprintf( buf, "Please try again in a few minutes.\n\r" );
	      write_to_buffer( d, buf, 0 );
	      close_socket( d, FALSE );
	    }

	  for ( pban = first_ban; pban; pban = pban->next )
	    {
	      if (
		  ( !str_prefix( pban->name, d->host )
		    || !str_suffix( pban->name, d->host ) ) )
		{
		  write_to_buffer( d,
                                       "Your site has been banned from this Mud.\n\r", 0 );
		  close_socket( d, FALSE );
		  return;
		}
	    }

	  for ( pban = first_tban; pban; pban = pban->next )
	    {
	      if (
		  ( !str_prefix( pban->name, d->host )
		    || !str_suffix( pban->name, d->host ) ) )
		{
		  write_to_buffer( d,
				   "New players have been temporarily banned from your IP.\n\r", 0 );
		  close_socket( d, FALSE );
		  return;
		}
	    }

	  sprintf( buf, "\n\rChoosing a name is one of the most important p\
arts of this game...\n\r"
                       "Make sure to pick a name appropriate to the character you are going\n\r"
                       "to role play, and be sure that it suits our theme.\n\r"
                       "If the name you select is not acceptable, you will be asked to choose\n\r"
                       "another one.\n\r\n\rPlease choose a name for your character: ");
	  write_to_buffer( d, buf, 0 );
	  d->newstate++;
	  d->connected = CON_GET_NAME;
	  return;
	}
      else
	{
	  write_to_buffer(d, "Illegal name, try another.\n\rName: ", 0);
	  return;
	}
    }

  if ( check_playing( d, argument, FALSE ) == BERR )
    {
      write_to_buffer( d, "Name: ", 0 );
      return;
    }

  fOld = load_char_obj( d, argument, TRUE );

  if ( !d->character )
    {
      sprintf( log_buf, "Bad player file %s@%s.", argument, d->host );
      log_string( log_buf );
      write_to_buffer( d, "Your playerfile is corrupt...Please notify Thoric@mud.compulink.com.\n\r", 0 );
      close_socket( d, FALSE );
      return;
    }

  CHAR_DATA *ch = d->character;

  for ( pban = first_ban; pban; pban = pban->next )
    {
      if (
	  ( !str_prefix( pban->name, d->host )
	    || !str_suffix( pban->name, d->host ) )
	  && pban->level >= ch->top_level )
	{
	  write_to_buffer( d,
			   "Your site has been banned from this Mud.\n\r", 0 );
	  close_socket( d, FALSE );
	  return;
	}
    }

  if ( IS_SET(ch->act, PLR_DENY) )
    {
      sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
      log_string_plus( log_buf, LOG_COMM );
      if (d->newstate != 0)
	{
	  write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
	  d->connected = CON_GET_NAME;
	  return;
	}

      write_to_buffer( d, "You are denied access.\n\r", 0 );
      close_socket( d, FALSE );
      return;
    }

  for ( pban = first_tban; pban; pban = pban->next )
    {
      if (
	  ( !str_prefix( pban->name, d->host )
	    || !str_suffix( pban->name, d->host ) )
	  && ch->top_level == 0 )
	{
	  write_to_buffer( d,
                               "You have been temporarily banned from creating new characters.\n\r", 0 );
	  close_socket( d, FALSE );
	  return;
	}
    }

  chk = check_reconnect( d, argument, FALSE );

  if ( chk == BERR )
    return;

  if ( chk )
    {
      fOld = TRUE;
    }
  else
    {
      if ( wizlock && !IS_IMMORTAL(ch) )
	{
	  write_to_buffer( d, "The game is wizlocked.  Only immortals can connect now.\n\r", 0 );
	  write_to_buffer( d, "Please try back later.\n\r", 0 );
	  close_socket( d, FALSE );
	  return;
	}
    }

  if ( fOld )
    {
      if (d->newstate != 0)
	{
	  write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
	  d->connected = CON_GET_NAME;
	  return;
	}

      /* Old player */
      write_to_buffer( d, "Password: ", 0 );
      write_to_buffer( d, echo_off_str, 0 );
      d->connected = CON_GET_OLD_PASSWORD;
      return;
    }
  else
    {
      write_to_buffer( d, "\n\rI don't recognize your name, you must be new here.\n\r\n\r", 0 );
      sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
      write_to_buffer( d, buf, 0 );
      d->connected = CON_CONFIRM_NEW_NAME;
      return;
    }
}

static void nanny_get_old_password( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;
  char buf[MAX_STRING_LENGTH];
  bool fOld, chk;
  write_to_buffer( d, "\n\r", 2 );

  if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
      write_to_buffer( d, "Wrong password.\n\r", 0 );
      /* clear descriptor pointer to get rid of bug message in log */
      d->character->desc = NULL;
      close_socket( d, FALSE );
      return;
    }

  write_to_buffer( d, echo_on_str, 0 );

  if ( check_playing( d, ch->name, TRUE ) )
    return;

  chk = check_reconnect( d, ch->name, TRUE );

  if ( chk == BERR )
    {
      if ( d->character && d->character->desc )
	d->character->desc = NULL;

      close_socket( d, FALSE );
      return;
    }

  if ( chk == TRUE )
    return;

  if ( check_multi( d , ch->name  ) )
    {
      close_socket( d, FALSE );
      return;
    }

  sprintf( buf, "%s", ch->name );
  d->character->desc = NULL;
  free_char( d->character );
  fOld = load_char_obj( d, buf, FALSE );
  ch = d->character;
  sprintf( log_buf, "%s@%s(%s) has connected.", ch->name, d->host,
	   d->user );
  log_string_plus( log_buf, LOG_COMM );
  show_title(d);

  if ( ch->pcdata->area )
    do_loadarea (ch , "" );
}

static void nanny_confirm_new_name( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;
  char buf[MAX_STRING_LENGTH];

  switch ( *argument )
    {
    case 'y': case 'Y':
      sprintf( buf, "\n\rMake sure to use a password that won't be easily guessed by someone else."
	       "\n\rPick a good password for %s: %s",
	       ch->name, echo_off_str );
      write_to_buffer( d, buf, 0 );
      d->connected = CON_GET_NEW_PASSWORD;
      break;

    case 'n': case 'N':
      write_to_buffer( d, "Ok, what IS it, then? ", 0 );
      /* clear descriptor pointer to get rid of bug message in log */
      d->character->desc = NULL;
      free_char( d->character );
      d->character = NULL;
      d->connected = CON_GET_NAME;
      break;

    default:
      write_to_buffer( d, "Please type Yes or No. ", 0 );
      break;
    }
}

static void nanny_get_new_password( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;

  write_to_buffer( d, "\n\r", 2 );

  if ( strlen(argument) < 5 )
    {
      write_to_buffer( d, "Password must be at least five characters long.\n\rPassword: ", 0 );
      return;
    }

  const char *p;
  const char *pwdnew = crypt( argument, ch->name );

  for ( p = pwdnew; *p != '\0'; p++ )
    {
      if ( *p == '~' )
	{
	  write_to_buffer( d, "New password not acceptable, try again.\n\rPassword: ", 0 );
	  return;
	}
    }

  DISPOSE( ch->pcdata->pwd );
  ch->pcdata->pwd   = str_dup( pwdnew );
  write_to_buffer( d, "\n\rPlease retype the password to confirm: ", 0 );
  d->connected = CON_CONFIRM_NEW_PASSWORD;
}

static void nanny_confirm_new_password( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;

  write_to_buffer( d, "\n\r", 2 );

  if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
      write_to_buffer( d, "Passwords don't match.\n\rRetype password: ", 0 );
      d->connected = CON_GET_NEW_PASSWORD;
      return;
    }

  write_to_buffer( d, echo_on_str, 0 );
  write_to_buffer( d, "\n\rWhat is your sex (M/F/N)? ", 0 );
  d->connected = CON_GET_NEW_SEX;
}

static void nanny_get_new_sex( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;

  switch ( argument[0] )
    {
    case 'm': case 'M': ch->sex = SEX_MALE;    break;
    case 'f': case 'F': ch->sex = SEX_FEMALE;  break;
    case 'n': case 'N': ch->sex = SEX_NEUTRAL; break;
    default:
      write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
      return;
    }

  write_to_buffer( d, "\n\rYou may choose one of the following skill packages to start with.\n\r", 0 );
  write_to_buffer( d, "You may learn a limited number of skills from other professions later.\n\r\n\r", 0 );

  write_to_buffer( d, "Architect            Tailor              Weaponsmith\n\r", 0 );
  write_to_buffer( d, "Soldier              Medic               Assassin\n\r", 0 );
  write_to_buffer( d, "Pilot                Senator             Spy\n\r", 0 );
  write_to_buffer( d, "Thief                Pirate\n\r\n\r", 0 );

  d->connected = CON_ADD_SKILLS;
  ch->pcdata->num_skills = 0;
  ch->pcdata->adept_skills = 0;
  ch->perm_frc = number_range(-2000, 20);
}

static void nanny_add_skills( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;
  char buf[MAX_STRING_LENGTH];

  if ( argument[0] == '\0' )
    {
      write_to_buffer( d, "Please pick a skill package: ", 0);
      return;
    }

  if (!str_cmp( argument, "help") )
    {
      do_help(ch, argument);
      return;
    }
  else if ( !str_prefix( argument , "architect" ) )
    {
      ch->pcdata->learned[gsn_survey] = 50;
      ch->pcdata->learned[gsn_landscape] = 50;
      ch->pcdata->learned[gsn_construction] = 50;
      ch->pcdata->learned[gsn_bridge] = 50;
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "%s the architect",ch->name );
      set_title( ch, buf );
    }
  else if ( !str_prefix( argument , "soldier" ) )
    {
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->learned[gsn_blasters] = 50;
      ch->pcdata->learned[gsn_enhanced_damage] = 50;
      ch->pcdata->learned[gsn_kick] = 50;
      ch->pcdata->learned[gsn_second_attack] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "%s the soldier",ch->name );
      set_title( ch, buf );
    }
  else if ( !str_prefix( argument , "medic" ) )
    {
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->learned[gsn_aid] = 50;
      ch->pcdata->learned[gsn_first_aid] = 50;
      ch->pcdata->learned[gsn_rescue] = 50;
      ch->pcdata->learned[gsn_dodge] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "%s the field medic",ch->name );
      set_title( ch, buf );
    }
  else if ( !str_prefix( argument , "assassin" ) )
    {
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->learned[gsn_backstab] = 50;
      ch->pcdata->learned[gsn_vibro_blades] = 50;
      ch->pcdata->learned[gsn_poison_weapon] = 50;
      ch->pcdata->learned[gsn_track] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "%s the assasin",ch->name );
      set_title( ch, buf );
    }
  else if ( !str_prefix( argument , "pilot" ) )
    {
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->learned[gsn_shipmaintenance] = 50;
      ch->pcdata->learned[gsn_shipdesign] = 50;
      ch->pcdata->learned[gsn_weaponsystems] = 50;
      ch->pcdata->learned[gsn_spacecombat] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "%s the pilot",ch->name );
      set_title( ch, buf );
    }
  else if ( !str_prefix( argument , "senator" ) )
    {
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->learned[gsn_survey] = 50;
      ch->pcdata->learned[gsn_postguard] = 50;
      ch->pcdata->learned[gsn_reinforcements] = 50;
      ch->pcdata->learned[gsn_propeganda] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "Senator %s",ch->name );
      set_title( ch, buf );
    }
  else if ( !str_prefix( argument , "spy" ) )
    {
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->learned[gsn_sneak] = 50;
      ch->pcdata->learned[gsn_peek] = 50;
      ch->pcdata->learned[gsn_pick_lock] = 50;
      ch->pcdata->learned[gsn_hide] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "%s the spy",ch->name );
      set_title( ch, buf );
    }
  else if ( !str_prefix( argument , "thief" ) )
    {
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->learned[gsn_steal] = 50;
      ch->pcdata->learned[gsn_peek] = 50;
      ch->pcdata->learned[gsn_pick_lock] = 50;
      ch->pcdata->learned[gsn_hide] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "%s the kleptomaniac",ch->name );
      set_title( ch, buf );
    }
  else if ( !str_prefix( argument , "pirate" ) )
    {
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->learned[gsn_hijack] = 50;
      ch->pcdata->learned[gsn_pickshiplock] = 50;
      ch->pcdata->learned[gsn_weaponsystems] = 50;
      ch->pcdata->learned[gsn_pick_lock] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "%s the pirate",ch->name );
      set_title( ch, buf );
    }
  else if ( !str_prefix( argument , "tailor" ) )
    {
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->learned[gsn_makearmor] = 50;
      ch->pcdata->learned[gsn_makecontainer] = 50;
      ch->pcdata->learned[gsn_makejewelry] = 50;
      ch->pcdata->learned[gsn_quicktalk] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "%s the tailor",ch->name );
      set_title( ch, buf );
    }
  else if ( !str_prefix( argument , "weaponsmith" ) )
    {
      ch->pcdata->learned[gsn_spacecraft] = 50;
      ch->pcdata->learned[gsn_makeblaster] = 50;
      ch->pcdata->learned[gsn_makeblade] = 50;
      ch->pcdata->learned[gsn_makeshield] = 50;
      ch->pcdata->learned[gsn_quicktalk] = 50;
      ch->pcdata->num_skills = 5;
      sprintf( buf, "%s the weapons dealer",ch->name );
      set_title( ch, buf );
    }
  else
    {
      write_to_buffer( d, "Invalid choice... Please pick a skill package: ", 0);
      return;
    }

  ch->perm_str = 10;
  ch->perm_int = 10;
  ch->perm_wis = 10;
  ch->perm_dex = 10;
  ch->perm_con = 10;
  ch->perm_cha = 10;

  write_to_buffer( d, "\n\rWould you like ANSI or no graphic/color support, (R/A/N)? ", 0 );
  d->connected = CON_GET_WANT_RIPANSI;
}

static void nanny_get_want_ripansi( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;

  switch ( argument[0] )
    {
    case 'a': case 'A': SET_BIT(ch->act,PLR_ANSI);  break;
    case 'n': case 'N': break;
    default:
      write_to_buffer( d, "Invalid selection.\n\rANSI or NONE? ", 0 );
      return;
    }

  sprintf( log_buf, "%s@%s new character.", ch->name, d->host);
  log_string_plus( log_buf, LOG_COMM);
  to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 2 );
  write_to_buffer( d, "Press [ENTER] ", 0 );
  show_title(d);
  ch->top_level = 0;
  ch->position = POS_STANDING;
  d->connected = CON_PRESS_ENTER;
}

static void nanny_press_enter( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;

  if ( IS_SET(ch->act, PLR_ANSI) )
    send_to_pager( "\033[2J", ch );
  else
    send_to_pager( "\014", ch );

  send_to_pager( "\n\r&WMessage of the Day&w\n\r", ch );
  do_help( ch, "motd" );
  send_to_pager( "\n\r&WPress [ENTER] &Y", ch );

  if ( IS_IMMORTAL(ch) )
    d->connected = CON_READ_IMOTD;
  else if ( ch->top_level == 0 )
    d->connected = CON_READ_NMOTD;
  else
    d->connected = CON_DONE_MOTD;
}

static void nanny_read_imotd( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;
  send_to_pager( "&WImmortal Message of the Day&w\n\r", ch );
  do_help( ch, "imotd" );
  send_to_pager( "\n\r&WPress [ENTER] &Y", ch );
  d->connected = CON_DONE_MOTD;
}

static void nanny_read_nmotd( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;
  do_help( ch, "nmotd" );
  send_to_pager( "\n\r&WPress [ENTER] &Y", ch );
  d->connected = CON_DONE_MOTD;
}

static void nanny_done_motd( DESCRIPTOR_DATA *d, char *argument )
{
  CHAR_DATA *ch = d->character;

  write_to_buffer( d, "\n\rWelcome...\n\r\n\r", 0 );
  add_char( ch );
  d->connected      = CON_PLAYING;

  if ( ch->top_level == 0 )
    {
      OBJ_DATA *obj;
      SHIP_DATA * ship;
      SHIP_PROTOTYPE * prototype;
      char shipname [MAX_STRING_LENGTH];
      prototype = get_ship_prototype( "NU-b13 Starfighter" );

      if ( prototype )
	{
	  ship = make_ship( prototype );
	  ship_to_room( ship, ROOM_NEWBIE_SHIPYARD );
	  ship->location = ROOM_NEWBIE_SHIPYARD;
	  ship->lastdoc = ROOM_NEWBIE_SHIPYARD;

	  sprintf( shipname , "%ss %s %s", ch->name, prototype->name,
		   ship->filename );

	  STRFREE( ship->owner );
	  ship->owner = STRALLOC( ch->name );
	  STRFREE( ship->name );
	  ship->name = STRALLOC( shipname );
	  save_ship( ship );
	  write_ship_list();
	}

      ch->gold = 20;

      ch->perm_lck = number_range(6, 18);
      ch->perm_frc = URANGE( 0 , ch->perm_frc , 20 );

      ch->top_level = 1;
      ch->hit       = ch->max_hit;
      ch->move      = ch->max_move;
      if ( ch->perm_frc > 0 )
	ch->max_mana = 100 + 100*ch->perm_frc;
      else
	ch->max_mana = 0;
      ch->mana      = ch->max_mana;

      /* Added by Narn.  Start new characters with autoexit and autgold
	 already turned on.  Very few people don't use those. */
      SET_BIT( ch->act, PLR_AUTOGOLD );
      SET_BIT( ch->act, PLR_AUTOEXIT );

      obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_DAGGER), 0 );
      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_WIELD );

      obj = create_object( get_obj_index(OBJ_VNUM_LIGHT), 0 );
      obj_to_char( obj, ch );

      /* comlink */
      OBJ_INDEX_DATA *obj_ind = get_obj_index( 23 );

      if ( obj_ind != NULL )
	{
	  obj = create_object( obj_ind, 0 );
	  obj_to_char( obj, ch );
	}

      if (!sysdata.WAIT_FOR_AUTH)
	{
	  char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	  ch->pcdata->auth_state = 3;
	}
      else
	{
	  char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	  ch->pcdata->auth_state = 1;
	  SET_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
	}
      /* Display_prompt interprets blank as default */
      ch->pcdata->prompt = STRALLOC("");
    }
  else if ( ch->in_room && !IS_IMMORTAL( ch ) )
    {
      char_to_room( ch, ch->in_room );
    }
  else
    {
      char_to_room( ch, get_room_index( wherehome(ch) ) );
    }

  if ( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
    remove_timer( ch, TIMER_SHOVEDRAG );

  if ( get_timer( ch, TIMER_PKILLED ) > 0 )
    remove_timer( ch, TIMER_PKILLED );
  if ( ch->plr_home != NULL )
    {
      char filename[256];
      FILE *fph;
      ROOM_INDEX_DATA *storeroom = ch->plr_home;
      OBJ_DATA *obj;
      OBJ_DATA *obj_next;

      for ( obj = storeroom->first_content; obj; obj = obj_next )
	{
	  obj_next = obj->next_content;
	  extract_obj( obj );
	}

      sprintf( filename, "%s%c/%s.home", PLAYER_DIR, tolower(ch->name[0]),
	       capitalize( ch->name ) );
      if ( ( fph = fopen( filename, "r" ) ) != NULL )
	{
	  int iNest;
	  bool found;
	  OBJ_DATA *tobj, *tobj_next;

	  rset_supermob(storeroom);
	  for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	  found = TRUE;
	  for ( ; ; )
	    {
	      char letter;
	      char *word;

	      letter = fread_letter( fph );
	      if ( letter == '*' )
		{
		  fread_to_eol( fph );
		  continue;
		}

	      if ( letter != '#' )
		{
		  bug( "Load_plr_home: # not found.", 0 );
		  bug( ch->name, 0 );
		  break;
		}

	      word = fread_word( fph );
	      if ( !str_cmp( word, "OBJECT" ) )     /* Objects      */
		fread_obj  ( supermob, fph, OS_CARRY );
	      else
		if ( !str_cmp( word, "END"    ) )   /* Done         */
		  break;
		else
		  {
		    bug( "Load_plr_home: bad section.", 0 );
		    bug( ch->name, 0 );
		    break;
		  }
	    }

	  fclose( fph );

	  for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
	    {
	      tobj_next = tobj->next_content;
	      obj_from_char( tobj );
	      obj_to_room( tobj, storeroom );
	    }

	  release_supermob();
	}
    }

  act( AT_ACTION, "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
  do_look( ch, "auto" );
  mail_count(ch);
}
