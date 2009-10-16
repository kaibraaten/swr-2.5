#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "mud.h"

extern	int	_filbuf		args( (FILE *) );

void init_supermob();

/*
 * Globals.
 */

WIZENT *	first_wiz;
WIZENT *	last_wiz;

time_t                  last_restore_all_time = 0;

HELP_DATA *		first_help;
HELP_DATA *		last_help;

SHOP_DATA *		first_shop;
SHOP_DATA *		last_shop;

REPAIR_DATA *		first_repair;
REPAIR_DATA *		last_repair;

TELEPORT_DATA *		first_teleport;
TELEPORT_DATA *		last_teleport;

OBJ_DATA *		extracted_obj_queue;
EXTRACT_CHAR_DATA *	extracted_char_queue;

char			bug_buf		[2*MAX_INPUT_LENGTH];
CHAR_DATA *		first_char;
CHAR_DATA *		last_char;
char *			help_greeting;
char			log_buf		[2*MAX_INPUT_LENGTH];

OBJ_DATA *		first_object;
OBJ_DATA *		last_object;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

int			cur_qobjs;
int			cur_qchars;
int			nummobsloaded;
int			numobjsloaded;
int			physicalobjects;

MAP_INDEX_DATA  *       first_map;	/* maps */

AUCTION_DATA    * 	auction;	/* auctions */

/* criminals */
short   gsn_torture;
short   gsn_disguise;
short   gsn_pickshiplock;
short   gsn_hijack;

/* soldiers and officers */
short   gsn_reinforcements;
short   gsn_postguard;
short   gsn_first_aid;
short   gsn_throw;

short   gsn_quicktalk;
short   gsn_propeganda;

/* pilots and smugglers */
short   gsn_spacecraft;
short   gsn_weaponsystems;
short   gsn_shipmaintenance; 
short   gsn_shipdesign; 
short   gsn_spacecombat;

/* player building skills */
short   gsn_lightsaber_crafting;
short   gsn_spice_refining;
short   gsn_makeblade;
short   gsn_makeblaster;
short   gsn_makelight;
short   gsn_makecomlink;
short   gsn_makearmor;
short   gsn_makeshield;
short   gsn_makecontainer;
short   gsn_makejewelry;

short   gsn_bridge;
short   gsn_survey;
short   gsn_landscape;
short   gsn_construction;
 
/* weaponry */
short			gsn_blasters;
short                  gsn_bowcasters;
short                  gsn_force_pikes;
short			gsn_lightsabers;
short			gsn_vibro_blades;
short			gsn_flexible_arms;
short			gsn_talonous_arms;
short			gsn_bludgeons;

/* thief */
short          	gsn_backstab;
short			gsn_circle;
short			gsn_dodge;
short			gsn_hide;
short			gsn_peek;
short			gsn_pick_lock;
short			gsn_sneak;
short			gsn_steal;
short			gsn_gouge;
short			gsn_poison_weapon;

/* thief & warrior */
short			gsn_enhanced_damage;
short			gsn_kick;
short			gsn_parry;
short			gsn_rescue;
short			gsn_second_attack;
short			gsn_third_attack;
short			gsn_dual_wield;
short                  gsn_bashdoor;
short			gsn_grip; 
short			gsn_berserk;
short			gsn_hitall;
short			gsn_disarm;


/* other   */
short			gsn_aid;
short			gsn_track;
short			gsn_mount;
short			gsn_climb;
short			gsn_slice;

/* spells */
short			gsn_aqua_breath;
short          	gsn_blindness;
short			gsn_charm_person;
short			gsn_invis;
short			gsn_mass_invis;
short			gsn_poison;
short			gsn_sleep;
short			gsn_stun;
short			gsn_possess;
short			gsn_fireball;
short			gsn_lightning_bolt;

/* for searching */
short			gsn_first_spell;
short			gsn_first_skill;
short			gsn_first_weapon;
short			gsn_top_sn;


/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];

AREA_DATA *		first_area;
AREA_DATA *		last_area;
AREA_DATA *		first_build;
AREA_DATA *		last_build;
AREA_DATA *		first_asort;
AREA_DATA *		last_asort;
AREA_DATA *		first_bsort;
AREA_DATA *		last_bsort;

SYSTEM_DATA		sysdata;

int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_room;
int 			top_r_vnum;
int			top_shop;
int			top_repair;
int			top_vroom;

/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];

bool MOBtrigger = TRUE;

/*
 * Local booting procedures.
 */
void	init_mm		args( ( void ) );

void	boot_log	args( ( const char *str, ... ) );
void	load_area	args( ( FILE *fp ) );
void    load_flags      args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_helps	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_mobiles	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_objects	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_rooms	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_shops	args( ( AREA_DATA *tarea, FILE *fp ) );
void 	load_repairs	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_specials	args( ( AREA_DATA *tarea, FILE *fp ) );
bool	load_systemdata	args( ( SYSTEM_DATA *sys ) );
void    load_banlist    args( ( void ) );

void	fix_exits	args( ( void ) );

/*
 * External booting function
 */
void	load_corpses	args( ( void ) );

/*
 * MUDprogram locals
 */

int 		mprog_name_to_type	args ( ( char* name ) );
MPROG_DATA *	mprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
						MOB_INDEX_DATA *pMobIndex ) );
/* int 		oprog_name_to_type	args ( ( char* name ) ); */
MPROG_DATA *	oprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
						OBJ_INDEX_DATA *pObjIndex ) );
/* int 		rprog_name_to_type	args ( ( char* name ) ); */
MPROG_DATA *	rprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
						ROOM_INDEX_DATA *pRoomIndex ) );
void		load_mudprogs           args ( ( AREA_DATA *tarea, FILE* fp ) );
void		load_objprogs           args ( ( AREA_DATA *tarea, FILE* fp ) );
void		load_roomprogs          args ( ( AREA_DATA *tarea, FILE* fp ) );
void   		mprog_read_programs     args ( ( FILE* fp,
						MOB_INDEX_DATA *pMobIndex) );
void   		oprog_read_programs     args ( ( FILE* fp,
						OBJ_INDEX_DATA *pObjIndex) );
void   		rprog_read_programs     args ( ( FILE* fp,
						ROOM_INDEX_DATA *pRoomIndex) );


void shutdown_mud( const char *reason )
{
  FILE *fp;

  if ( (fp = fopen( SHUTDOWN_FILE, "a" )) != NULL )
    {
      fprintf( fp, "%s\n", reason );
      fclose( fp );
    }
}


/*
 * Big mama top level function.
 */
void boot_db( bool fCopyOver )
{
    short wear, x;

    show_hash( 32 );
    unlink( BOOTLOG_FILE );
    boot_log( "---------------------[ Boot Log ]--------------------" );

    log_string( "Loading commands" );
    load_commands();

    log_string( "Loading sysdata configuration..." );

    /* default values */
    sysdata.save_frequency		= 20;	/* minutes */
    sysdata.save_flags			= SV_DEATH | SV_PASSCHG | SV_AUTO
    					| SV_PUT | SV_DROP | SV_GIVE
    					| SV_AUCTION | SV_ZAPDROP | SV_IDLE;
    if ( !load_systemdata(&sysdata) )
    {
	log_string( "Not found.  Creating new configuration." );
	sysdata.alltimemax = 0;
    }

    log_string("Loading socials");
    load_socials();

    log_string("Loading skill table");
    load_skill_table();
    sort_skill_table();

    gsn_first_spell  = 0;
    gsn_first_skill  = 0;
    gsn_first_weapon = 0;
    gsn_top_sn	     = top_sn;

    for ( x = 0; x < top_sn; x++ )
	if ( !gsn_first_spell && skill_table[x]->type == SKILL_SPELL )
	    gsn_first_spell = x;
	else
	if ( !gsn_first_skill && skill_table[x]->type == SKILL_SKILL )
	    gsn_first_skill = x;
	else
	if ( !gsn_first_weapon && skill_table[x]->type == SKILL_WEAPON )
	    gsn_first_weapon = x;

    fBootDb		= TRUE;
    
    top_r_vnum 		= 0;
    nummobsloaded	= 0;
    numobjsloaded	= 0;
    physicalobjects	= 0;
    sysdata.maxplayers	= 0;
    first_object	= NULL;
    last_object		= NULL;
    first_char		= NULL;
    last_char		= NULL;
    first_area		= NULL;
    last_area		= NULL;
    first_build		= NULL;
    last_area		= NULL;
    first_shop		= NULL;
    last_shop		= NULL;
    first_repair	= NULL;
    last_repair		= NULL;
    first_teleport	= NULL;
    last_teleport	= NULL;
    first_asort		= NULL;
    last_asort		= NULL;
    extracted_obj_queue	= NULL;
    extracted_char_queue= NULL;
    cur_qobjs		= 0;
    cur_qchars		= 0;
    cur_char		= NULL;
    cur_obj		= 0;
    cur_obj_serial	= 0;
    cur_char_died	= FALSE;
    cur_obj_extracted	= FALSE;
    cur_room		= NULL;
    quitting_char	= NULL;
    loading_char	= NULL;
    saving_char		= NULL;
    CREATE( auction, AUCTION_DATA, 1);
    auction->item 	= NULL;
    for ( wear = 0; wear < MAX_WEAR; wear++ )
	for ( x = 0; x < MAX_LAYERS; x++ )
	    save_equipment[wear][x] = NULL;

    /*
     * Init random number generator.
     */
    log_string("Initializing random number generator");
    init_mm( );

    /*
     * Set time and weather.
     */
    {
	long lhour, lday, lmonth;

	log_string("Setting time and weather");

	lhour		= (current_time - 650336715)
			/ (PULSE_TICK / PULSE_PER_SECOND);
	time_info.hour	= lhour  % 24;
	lday		= lhour  / 24;
	time_info.day	= lday   % 35;
	lmonth		= lday   / 35;
	time_info.month	= lmonth % 17;
	time_info.year	= lmonth / 17;

	     if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
	else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
	else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
	else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
	else                            weather_info.sunlight = SUN_DARK;

	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if ( time_info.month >= 7 && time_info.month <=12 )
	    weather_info.mmhg += number_range( 1, 50 );
	else
	    weather_info.mmhg += number_range( 1, 80 );

	     if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
	else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
	else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
	else                                  weather_info.sky = SKY_CLOUDLESS;

    }


    /*
     * Assign gsn's for skills which need them.
     */
    {
	log_string("Assigning gsn's");

        ASSIGN_GSN( gsn_survey , "surveying" );
        ASSIGN_GSN( gsn_landscape , "landscape and design" );
        ASSIGN_GSN( gsn_construction , "construction" );
        ASSIGN_GSN( gsn_quicktalk , "quicktalk" );
        ASSIGN_GSN( gsn_bridge , "bridges and exits" );
        ASSIGN_GSN( gsn_propeganda , "propeganda" );
        ASSIGN_GSN( gsn_hijack  , "hijack" );
        ASSIGN_GSN( gsn_makejewelry  , "makejewelry" );
        ASSIGN_GSN( gsn_makeblade  , "makeblade" );
        ASSIGN_GSN( gsn_makeblaster  , "makeblaster" );
        ASSIGN_GSN( gsn_makelight   , "makeflashlight" );
        ASSIGN_GSN( gsn_makecomlink   , "makecomlink" );
        ASSIGN_GSN( gsn_makearmor  , "makearmor" );        
        ASSIGN_GSN( gsn_makeshield  , "makeshield" );
        ASSIGN_GSN( gsn_makecontainer  , "makecontainer" );
        ASSIGN_GSN( gsn_reinforcements  , "reinforcements" );
        ASSIGN_GSN( gsn_postguard   , "post guard" );
        ASSIGN_GSN( gsn_torture   , "torture" );
        ASSIGN_GSN( gsn_throw   , "throw" );
        ASSIGN_GSN( gsn_disguise   , "disguise" );
        ASSIGN_GSN( gsn_first_aid   , "first aid" );
        ASSIGN_GSN( gsn_lightsaber_crafting, "lightsaber crafting" );
        ASSIGN_GSN( gsn_spice_refining,  "spice refining" );
        ASSIGN_GSN( gsn_spacecombat,     "space combat" );
        ASSIGN_GSN( gsn_weaponsystems,   "weapon systems" );
        ASSIGN_GSN( gsn_spacecraft,    "spacecraft" );
        ASSIGN_GSN( gsn_shipdesign,    "ship design" );
        ASSIGN_GSN( gsn_shipmaintenance, "ship maintenance" );
	ASSIGN_GSN( gsn_blasters,	"blasters" );
	ASSIGN_GSN( gsn_bowcasters,	"bowcasters" );
	ASSIGN_GSN( gsn_force_pikes,	"force pikes" );
	ASSIGN_GSN( gsn_lightsabers,	"lightsabers" );
	ASSIGN_GSN( gsn_vibro_blades,	"vibro-blades" );
	ASSIGN_GSN( gsn_flexible_arms,	"flexible arms" );
	ASSIGN_GSN( gsn_talonous_arms,	"talonous arms" );
	ASSIGN_GSN( gsn_bludgeons,	"bludgeons" );
	ASSIGN_GSN( gsn_backstab,	"backstab" );
	ASSIGN_GSN( gsn_circle,		"circle" );
	ASSIGN_GSN( gsn_dodge,		"dodge" );
	ASSIGN_GSN( gsn_hide,		"hide" );
	ASSIGN_GSN( gsn_peek,		"peek" );
	ASSIGN_GSN( gsn_pick_lock,	"picklock" );
	ASSIGN_GSN( gsn_pickshiplock  , "pickshiplock" );
        ASSIGN_GSN( gsn_sneak,		"sneak" );
	ASSIGN_GSN( gsn_steal,		"steal" );
	ASSIGN_GSN( gsn_gouge,		"gouge" );
	ASSIGN_GSN( gsn_poison_weapon, 	"poison weapon" );
	ASSIGN_GSN( gsn_disarm,		"disarm" );
	ASSIGN_GSN( gsn_enhanced_damage, "enhanced damage" );
	ASSIGN_GSN( gsn_kick,		"kick" );
	ASSIGN_GSN( gsn_parry,		"parry" );
	ASSIGN_GSN( gsn_rescue,		"rescue" );
	ASSIGN_GSN( gsn_second_attack, 	"second attack" );
	ASSIGN_GSN( gsn_third_attack, 	"third attack" );
	ASSIGN_GSN( gsn_dual_wield,	"dual wield" );
	ASSIGN_GSN( gsn_bashdoor,	"doorbash" );
	ASSIGN_GSN( gsn_grip,		"grip" ); 
	ASSIGN_GSN( gsn_berserk,	"berserk" );
	ASSIGN_GSN( gsn_hitall,		"hitall" );
	ASSIGN_GSN( gsn_aid,		"aid" );
	ASSIGN_GSN( gsn_track,		"track" );
	ASSIGN_GSN( gsn_mount,		"mount" );
	ASSIGN_GSN( gsn_climb,		"climb" );
	ASSIGN_GSN( gsn_slice,		"slice" );
	ASSIGN_GSN( gsn_fireball,	"fireball" );
	ASSIGN_GSN( gsn_lightning_bolt,	"lightning bolt" );
	ASSIGN_GSN( gsn_aqua_breath,	"aqua breath" );
	ASSIGN_GSN( gsn_blindness,	"blindness" );
	ASSIGN_GSN( gsn_charm_person, 	"affect mind" );
	ASSIGN_GSN( gsn_invis,		"mask" );
	ASSIGN_GSN( gsn_mass_invis,	"group masking" );
	ASSIGN_GSN( gsn_poison,		"poison" );
	ASSIGN_GSN( gsn_sleep,		"sleep" );
	ASSIGN_GSN( gsn_stun,		"stun" );
	ASSIGN_GSN( gsn_possess,	"possess" );
    }

    /*
     * Read in all the area files.
     */
    {
	FILE *fpList;

	log_string("Reading in area files...");
	if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
	{
	    shutdown_mud( "Unable to open area list" );
	    exit( 1 );
	}

	for ( ; ; )
	{
	    strcpy( strArea, fread_word( fpList ) );
	    if ( strArea[0] == '$' )
		break;

	    load_area_file( last_area, strArea );
	    
	}
	fclose( fpList );
    }

    init_supermob();


    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     */
    {
	log_string( "Fixing exits" );
	fix_exits( );
	fBootDb	= FALSE;
	log_string( "Loading boards" );
	load_boards( );
	log_string( "Loading clans" );
	load_clans( );
        log_string( "Loading bans" );
        load_banlist( );
        log_string( "Loading corpses" );
        load_corpses( );
        log_string( "Loading space" );
        load_space( );
        log_string( "Loading ship prototypes" );
        load_prototypes( );
        log_string( "Loading ships" );
        load_ships( );
        log_string( "Loading planet data" );
        load_planets( );
        log_string( "Resetting areas" );
	reset_all( );
	                
        MOBtrigger = TRUE;

	if( fCopyOver )
	  {
	    log_string( "Running copyover_recover." );
	    copyover_recover();
	  }
    }

    /* init_maps ( ); */

    return;
}



/*
 * Load an 'area' header line.
 */
void load_area( FILE *fp )
{
    AREA_DATA *pArea;

    CREATE( pArea, AREA_DATA, 1 );
    pArea->first_room	= NULL;
    pArea->last_room	= NULL;
    pArea->planet       = NULL;
    pArea->name		= fread_string_nohash( fp );
    pArea->filename	= str_dup( strArea );

    LINK( pArea, first_area, last_area, next, prev );
    top_area++;
    return;
}

/*
 * Load area flags. Narn, Mar/96 
 */
void load_flags( AREA_DATA *tarea, FILE *fp )
{
    char *ln;
    int x1, x2;

    if ( !tarea )
    {
	bug( "Load_flags: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }
    ln = fread_line( fp );
    x1=x2=0;
    sscanf( ln, "%d %d",
	&x1, &x2 );
    tarea->flags = x1;
    return;
}

/*
 * Adds a help page to the list if it is not a duplicate of an existing page.
 * Page is insert-sorted by keyword.			-Thoric
 * (The reason for sorting is to keep do_hlist looking nice)
 */
void add_help( HELP_DATA *pHelp )
{
    HELP_DATA *tHelp;
    int match;

    for ( tHelp = first_help; tHelp; tHelp = tHelp->next )
	if ( pHelp->level == tHelp->level
	&&   strcmp(pHelp->keyword, tHelp->keyword) == 0 )
	{
	    bug( "add_help: duplicate: %s.  Deleting.", pHelp->keyword );
	    STRFREE( pHelp->text );
	    STRFREE( pHelp->keyword );
	    DISPOSE( pHelp );
	    return;
	}
	else
	if ( (match=strcmp(pHelp->keyword[0]=='\'' ? pHelp->keyword+1 : pHelp->keyword,
			   tHelp->keyword[0]=='\'' ? tHelp->keyword+1 : tHelp->keyword)) < 0
	||   (match == 0 && pHelp->level > tHelp->level) )
	{
	    if ( !tHelp->prev )
		first_help	  = pHelp;
	    else
		tHelp->prev->next = pHelp;
	    pHelp->prev		  = tHelp->prev;
	    pHelp->next		  = tHelp;
	    tHelp->prev		  = pHelp;
	    break;
	}

    if ( !tHelp )
	LINK( pHelp, first_help, last_help, next, prev );

    top_help++;
}

/*
 * Load a help section.
 */
void load_helps( AREA_DATA *tarea, FILE *fp )
{
    HELP_DATA *pHelp;

    for ( ; ; )
    {
	CREATE( pHelp, HELP_DATA, 1 );
	pHelp->level	= fread_number( fp );
	pHelp->keyword	= fread_string( fp );
	if ( pHelp->keyword[0] == '$' )
	    break;
	pHelp->text	= fread_string( fp );
	if ( pHelp->keyword[0] == '\0' )
	{
	    STRFREE( pHelp->text );
	    STRFREE( pHelp->keyword );
	    DISPOSE( pHelp );
	    continue;
	}

	if ( !str_cmp( pHelp->keyword, "greeting" ) )
	    help_greeting = pHelp->text;
	add_help( pHelp );
    }
    return;
}


/*
 * Add a character to the list of all characters		-Thoric
 */
void add_char( CHAR_DATA *ch )
{
    LINK( ch, first_char, last_char, next, prev );
}


/*
 * Load a mob section.
 */
void load_mobiles( AREA_DATA *tarea, FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    char *ln;
    int x1, x2, x3, x4, x5, x6, x7, x8;

    for ( ; ; )
    {
	char buf[MAX_STRING_LENGTH];
	long vnum;
	char letter;
	int iHash;
	bool oldmob;
	bool tmpBootDb;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_mobiles: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_mob_index( vnum ) )
	{
	    if ( tmpBootDb )
	    {
		bug( "Load_mobiles: vnum %ld duplicated.", vnum );
		shutdown_mud( "duplicate vnum" );
		exit( 1 );
	    }
	    else
	    {
		pMobIndex = get_mob_index( vnum );
		sprintf( buf, "Cleaning mobile: %ld", vnum );
		log_string_plus( buf, LOG_BUILD );
		clean_mob( pMobIndex );
		oldmob = TRUE;
	    }
	}
	else
	{
	  oldmob = FALSE;
	  CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
	}
	fBootDb = tmpBootDb;

	pMobIndex->vnum			= vnum;
	pMobIndex->player_name		= fread_string( fp );
	pMobIndex->short_descr		= fread_string( fp );
	pMobIndex->long_descr		= fread_string( fp );
	pMobIndex->description		= fread_string( fp );

	pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
	pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);

	pMobIndex->act			= fread_number( fp ) | ACT_IS_NPC;
	pMobIndex->affected_by		= fread_number( fp );
	pMobIndex->pShop		= NULL;
	pMobIndex->rShop		= NULL;
	pMobIndex->alignment		= fread_number( fp );
	letter				= fread_letter( fp );
	pMobIndex->level		= fread_number( fp );

	pMobIndex->mobthac0		= fread_number( fp );
	pMobIndex->ac			= fread_number( fp );
	pMobIndex->hitnodice		= fread_number( fp );
	/* 'd'		*/		  fread_letter( fp );
	pMobIndex->hitsizedice		= fread_number( fp );
	/* '+'		*/		  fread_letter( fp );
	pMobIndex->hitplus		= fread_number( fp );
	pMobIndex->damnodice		= fread_number( fp );
	/* 'd'		*/		  fread_letter( fp );
	pMobIndex->damsizedice		= fread_number( fp );
	/* '+'		*/		  fread_letter( fp );
	pMobIndex->damplus		= fread_number( fp );
	pMobIndex->gold			= fread_number( fp );
	pMobIndex->exp			= fread_number( fp );
	pMobIndex->position		= fread_number( fp );
	pMobIndex->defposition		= fread_number( fp );

	/*
	 * Back to meaningful values.
	 */
	pMobIndex->sex			= fread_number( fp );

	if ( letter != 'S' && letter != 'C' && letter != 'Z' )
	{
	    bug( "Load_mobiles: vnum %ld: letter '%c' not Z, S or C.", vnum,
	        letter );
	    shutdown_mud( "bad mob data" );
	    exit( 1 );
	}
	if ( letter == 'C' || letter == 'Z' ) /* Realms complex mob 	-Thoric  */
	{
	    pMobIndex->perm_str			= fread_number( fp );
	    pMobIndex->perm_int			= fread_number( fp );
	    pMobIndex->perm_wis			= fread_number( fp );
	    pMobIndex->perm_dex			= fread_number( fp );
	    pMobIndex->perm_con			= fread_number( fp );
	    pMobIndex->perm_cha			= fread_number( fp );
	    pMobIndex->perm_lck			= fread_number( fp );
 	    pMobIndex->saving_poison_death	= fread_number( fp );
	    pMobIndex->saving_wand		= fread_number( fp );
	    pMobIndex->saving_para_petri	= fread_number( fp );
	    pMobIndex->saving_breath		= fread_number( fp );
	    pMobIndex->saving_spell_staff	= fread_number( fp );
	    ln = fread_line( fp );
	    x1=x2=x3=x4=x5=x6=x7=0;
	    sscanf( ln, "%d %d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6, &x7 );
	    pMobIndex->height		= x3;
	    pMobIndex->weight		= x4;
	    pMobIndex->numattacks	= x7;

	    ln = fread_line( fp );
	    x1=x2=x3=x4=x5=x6=x7=x8=0;
	    sscanf( ln, "%d %d %d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
	    pMobIndex->hitroll		= x1;
	    pMobIndex->damroll		= x2;
	    pMobIndex->xflags		= x3;
	    pMobIndex->resistant	= x4;
	    pMobIndex->immune		= x5;
	    pMobIndex->susceptible	= x6;
	    pMobIndex->attacks		= x7;
	    pMobIndex->defenses		= x8;
	}
	else 
	{
	    pMobIndex->perm_str		= 10;
	    pMobIndex->perm_dex		= 10;
	    pMobIndex->perm_int		= 10;
	    pMobIndex->perm_wis		= 10;
	    pMobIndex->perm_cha		= 10;
	    pMobIndex->perm_con		= 10;
	    pMobIndex->perm_lck		= 10;
	    pMobIndex->xflags		= 0;
	    pMobIndex->resistant	= 0;
	    pMobIndex->immune		= 0;
	    pMobIndex->susceptible	= 0;
	    pMobIndex->numattacks	= 0;
	    pMobIndex->attacks		= 0;
	    pMobIndex->defenses		= 0;
	}
        if ( letter == 'Z' ) /*  STar Wars Reality Complex Mob  */
	{
            ln = fread_line( fp );
	    x1=x2=x3=x4=x5=x6=x7=x8=0;
	    sscanf( ln, "%d %d %d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5,  &x6,  &x7,  &x8);
        }
        
	letter = fread_letter( fp );
	if ( letter == '>' )
	{
	    ungetc( letter, fp );
	    mprog_read_programs( fp, pMobIndex );
	}
	else ungetc( letter,fp );

	if ( !oldmob )
	{
	    iHash			= vnum % MAX_KEY_HASH;
	    pMobIndex->next		= mob_index_hash[iHash];
	    mob_index_hash[iHash]	= pMobIndex;
	    top_mob_index++;
	}
    }

    return;
}



/*
 * Load an obj section.
 */
void load_objects( AREA_DATA *tarea, FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
    char letter;
    char *ln;
    int x1, x2, x3, x4, x5, x6;

    for ( ; ; )
    {
	char buf[MAX_STRING_LENGTH];
	long vnum;
	int iHash;
	bool tmpBootDb;
	bool oldobj;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_objects: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_obj_index( vnum ) )
	{
	    if ( tmpBootDb )
	    {
		bug( "Load_objects: vnum %ld duplicated.", vnum );
		shutdown_mud( "duplicate vnum" );
		exit( 1 );
	    }
	    else
	    {
		pObjIndex = get_obj_index( vnum );
		sprintf( buf, "Cleaning object: %ld", vnum );
		log_string_plus( buf, LOG_BUILD );
		clean_obj( pObjIndex );
		oldobj = TRUE;
	    }
	}
	else
	{
	  oldobj = FALSE;
	  CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
	}
	fBootDb = tmpBootDb;

	pObjIndex->vnum			= vnum;
	pObjIndex->name			= fread_string( fp );
	pObjIndex->short_descr		= fread_string( fp );
	pObjIndex->description		= fread_string( fp );
	pObjIndex->action_desc		= fread_string( fp );

        /* Commented out by Narn, Apr/96 to allow item short descs like 
           Bonecrusher and Oblivion */
	/*pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);*/
	pObjIndex->description[0]	= UPPER(pObjIndex->description[0]);

	ln = fread_line( fp );
	x1=x2=x3=x4=0;
	sscanf( ln, "%d %d %d %d",
		&x1, &x2, &x3, &x4 );
	pObjIndex->item_type		= x1;
	pObjIndex->extra_flags		= x2;
	pObjIndex->wear_flags		= x3;
	pObjIndex->layers		= x4;

	ln = fread_line( fp );
	x1=x2=x3=x4=x5=x6=0;
	sscanf( ln, "%d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6 );
	pObjIndex->value[0]		= x1;
	pObjIndex->value[1]		= x2;
	pObjIndex->value[2]		= x3;
	pObjIndex->value[3]		= x4;
	pObjIndex->value[4]		= x5;
	pObjIndex->value[5]		= x6;
	pObjIndex->weight		= fread_number( fp );
	pObjIndex->weight = UMAX( 1, pObjIndex->weight );
	pObjIndex->cost			= fread_number( fp );
        
        fread_number( fp );  /*extra */
        
	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'A' )
	    {
		AFFECT_DATA *paf;

		CREATE( paf, AFFECT_DATA, 1 );
		paf->type		= -1;
		paf->duration		= -1;
		paf->location		= fread_number( fp );
		if ( paf->location == APPLY_WEAPONSPELL
		||   paf->location == APPLY_WEARSPELL
		||   paf->location == APPLY_REMOVESPELL
		||   paf->location == APPLY_STRIPSN )
		  paf->modifier		= slot_lookup( fread_number(fp) );
		else
		  paf->modifier		= fread_number( fp );
		paf->bitvector		= 0;
		LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
			   next, prev );
		top_affect++;
	    }

	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
			  next, prev );
		top_ed++;
	    }

	    else if ( letter == '>' )
	    {
	        ungetc( letter, fp );
	        oprog_read_programs( fp, pObjIndex );
	    }

	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}

	/*
	 * Translate spell "slot numbers" to internal "skill numbers."
	 */
	switch ( pObjIndex->item_type )
	{
	case ITEM_DEVICE:
	    pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	    break;
	}

	if ( !oldobj )
	{
	  iHash			= vnum % MAX_KEY_HASH;
	  pObjIndex->next	= obj_index_hash[iHash];
	  obj_index_hash[iHash]	= pObjIndex;
	  top_obj_index++;
	}
    }

    return;
}



/*
 * Load a room section.
 */
void load_rooms( AREA_DATA *tarea, FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];
    char *ln;

    if ( !tarea )
    {
	bug( "Load_rooms: no #AREA seen yet." );
	shutdown_mud( "No #AREA" );
	exit( 1 );
    }

    for ( ; ; )
    {
	long vnum;
	char letter;
	int door;
	int iHash;
	bool tmpBootDb;
	bool oldroom;
	int x1, x2, x3, x4, x5, x6;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_rooms: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_room_index( vnum ) != NULL )
	{
	    if ( tmpBootDb )
	    {
	      bug( "Load_rooms: vnum %d duplicated.", vnum );
	      shutdown_mud( "duplicate vnum" );
	      exit( 1 );
	    }
	    else
	    {
	      pRoomIndex = get_room_index( vnum );
	      sprintf( buf, "Cleaning room: %ld", vnum );
	      log_string_plus( buf, LOG_BUILD );
	      if ( pRoomIndex->area )
	         UNLINK( pRoomIndex , pRoomIndex->area->first_room , pRoomIndex->area->last_room , next_in_area , prev_in_area );
	      clean_room( pRoomIndex );
	      oldroom = TRUE;
	    }
	}
	else
	{
	  oldroom = FALSE;
	  CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
	  pRoomIndex->first_person	= NULL;
	  pRoomIndex->last_person	= NULL;
	  pRoomIndex->first_content	= NULL;
	  pRoomIndex->last_content	= NULL;
	  pRoomIndex->next_in_area      = NULL;
	  pRoomIndex->prev_in_area      = NULL;
	  pRoomIndex->next_in_ship      = NULL;
	  pRoomIndex->prev_in_ship      = NULL;
	}

	fBootDb = tmpBootDb;
	pRoomIndex->area		= tarea;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->first_extradesc	= NULL;
	pRoomIndex->last_extradesc	= NULL;

	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description		= fread_string( fp );

	/* Area number			  fread_number( fp ); */
	ln = fread_line( fp );
	x1=x2=x3=x4=x5=x6=0;
	sscanf( ln, "%d %d %d %d %d %d",
	      &x1, &x2, &x3, &x4, &x5, &x6 );

	pRoomIndex->room_flags		= x2;
	pRoomIndex->sector_type		= x3;
	pRoomIndex->tele_delay		= x4;
	pRoomIndex->tele_vnum		= x5;
	pRoomIndex->tunnel		= x6;

	if (pRoomIndex->sector_type < 0 || pRoomIndex->sector_type == SECT_MAX)
	{
	  bug( "Fread_rooms: vnum %d has bad sector_type %d.", vnum ,
	      pRoomIndex->sector_type);
	  pRoomIndex->sector_type = 1;
	}
	pRoomIndex->light		= 0;
	pRoomIndex->first_exit		= NULL;
	pRoomIndex->last_exit		= NULL;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'S' )
		break;

	    if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;
		int locks;

		door = fread_number( fp );
		if ( door < 0 || door > 10 )
		{
		    bug( "Fread_rooms: vnum %d has bad door number %d.", vnum,
		        door );
		    if ( fBootDb )
		      exit( 1 );
		}
		else
		{
		  pexit = make_exit( pRoomIndex, NULL, door );
		  pexit->description	= fread_string( fp );
		  pexit->keyword	= fread_string( fp );
		  pexit->exit_info	= 0;
		  ln = fread_line( fp );
		  x1=x2=x3=x4=0;
		  sscanf( ln, "%d %d %d %d",
		      &x1, &x2, &x3, &x4 );

		  locks			= x1;
		  pexit->key		= x2;
		  pexit->vnum		= x3;
		  pexit->vdir		= door;
		  pexit->distance	= x4;

		  switch ( locks )
		  {
		    case 1:  pexit->exit_info = EX_ISDOOR;                break;
		    case 2:  pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
		    default: pexit->exit_info = locks;
		  }
		}
	    }
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc,
			  next, prev );
		top_ed++;
	    }
	    else if ( letter == '>' )
	    {
	      ungetc( letter, fp );
	      rprog_read_programs( fp, pRoomIndex );
            }
	    else
	    {
		bug( "Load_rooms: vnum %d has flag '%c' not 'DES'.", vnum,
		    letter );
		shutdown_mud( "Room flag not DES" );
		exit( 1 );
	    }

	}

	if ( !oldroom )
	{
	  iHash			 = vnum % MAX_KEY_HASH;
	  pRoomIndex->next	 = room_index_hash[iHash];
	  room_index_hash[iHash] = pRoomIndex;
	  top_room++;
	}
	
	if ( vnum > top_r_vnum )
	    top_r_vnum = vnum;
	
	LINK ( pRoomIndex , tarea->first_room , tarea->last_room, next_in_area , prev_in_area );
    }

    return;
}



/*
 * Load a shop section.
 */
void load_shops( AREA_DATA *tarea, FILE *fp )
{
    SHOP_DATA *pShop;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iTrade;

	CREATE( pShop, SHOP_DATA, 1 );
	pShop->keeper		= fread_number( fp );
	if ( pShop->keeper == 0 )
	    break;
	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	    pShop->buy_type[iTrade]	= fread_number( fp );
	pShop->profit_buy	= fread_number( fp );
	pShop->profit_sell	= fread_number( fp );
	pShop->profit_buy	= URANGE( pShop->profit_sell+5, pShop->profit_buy, 1000 );
	pShop->profit_sell	= URANGE( 0, pShop->profit_sell, pShop->profit_buy-5 );
	pShop->open_hour	= fread_number( fp );
	pShop->close_hour	= fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex		= get_mob_index( pShop->keeper );
	pMobIndex->pShop	= pShop;

	if ( !first_shop )
	    first_shop		= pShop;
	else
	    last_shop->next	= pShop;
	pShop->next		= NULL;
	pShop->prev		= last_shop;
	last_shop		= pShop;
	top_shop++;
    }
    return;
}

/*
 * Load a repair shop section.					-Thoric
 */
void load_repairs( AREA_DATA *tarea, FILE *fp )
{
    REPAIR_DATA *rShop;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iFix;

	CREATE( rShop, REPAIR_DATA, 1 );
	rShop->keeper		= fread_number( fp );
	if ( rShop->keeper == 0 )
	    break;
	for ( iFix = 0; iFix < MAX_FIX; iFix++ )
	  rShop->fix_type[iFix] = fread_number( fp );
	rShop->profit_fix	= fread_number( fp );
	rShop->shop_type	= fread_number( fp );
	rShop->open_hour	= fread_number( fp );
	rShop->close_hour	= fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex		= get_mob_index( rShop->keeper );
	pMobIndex->rShop	= rShop;

	if ( !first_repair )
	  first_repair		= rShop;
	else
	  last_repair->next	= rShop;
	rShop->next		= NULL;
	rShop->prev		= last_repair;
	last_repair		= rShop;
	top_repair++;
    }
    return;
}


/*
 * Load spec proc declarations.
 */
void load_specials( AREA_DATA *tarea, FILE *fp )
{
    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	char letter;

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_specials: letter '%c' not *MS.", letter );
	    exit( 1 );

	case 'S':
	    return;

	case '*':
	    break;

	case 'M':
	    pMobIndex		= get_mob_index	( fread_number ( fp ) );
	    if ( !pMobIndex->spec_fun )
	    {
	       pMobIndex->spec_fun	= spec_lookup	( fread_word   ( fp ) );
	    
	       if ( pMobIndex->spec_fun == 0 )
	       {
		  bug( "Load_specials: 'M': vnum %ld.", pMobIndex->vnum );
		  exit( 1 );
	       }
	    }
	    else if ( !pMobIndex->spec_2 )
	    {
	       pMobIndex->spec_2	= spec_lookup	( fread_word   ( fp ) );
	    
	       if ( pMobIndex->spec_2 == 0 )
	       {
		  bug( "Load_specials: 'M': vnum %ld.", pMobIndex->vnum );
		  exit( 1 );
	       }
	    }
	    
	    break;
	}

	fread_to_eol( fp );
    }
}



/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit, *pexit_next, *rev_exit;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex;
	      pRoomIndex  = pRoomIndex->next )
	{
	    bool fexit;

	    fexit = FALSE;
	    for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit_next )
	    {
		pexit_next = pexit->next;
		pexit->rvnum = pRoomIndex->vnum;
		if ( pexit->vnum <= 0
		||  (pexit->to_room=get_room_index(pexit->vnum)) == NULL )
		{
		    if ( fBootDb )
			boot_log( "Fix_exits: room %d, exit %s leads to bad vnum (%d)",
				pRoomIndex->vnum, dir_name[pexit->vdir], pexit->vnum );
		    
		    bug( "Deleting %s exit in room %ld", dir_name[pexit->vdir],
				pRoomIndex->vnum );
		    extract_exit( pRoomIndex, pexit );
		}
		else
		  fexit = TRUE;
	    }
	    if ( !fexit )
	      SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
	}
    }

    /* Set all the rexit pointers 	-Thoric */
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex;
	      pRoomIndex  = pRoomIndex->next )
	{
	    for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	    {
		if ( pexit->to_room && !pexit->rexit )
		{
		   rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
		   if ( rev_exit )
		   {
			pexit->rexit	= rev_exit;
			rev_exit->rexit	= pexit;
		   }
		}
	    }
	}
    }

    return;
}


/*
 * Get diku-compatable exit by number				-Thoric
 */
EXIT_DATA *get_exit_number( ROOM_INDEX_DATA *room, int xit )
{
    EXIT_DATA *pexit;
    int count;

    count = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
	if ( ++count == xit )
	  return pexit;
    return NULL;
}

/*
 * (prelude...) This is going to be fun... NOT!
 * (conclusion) QSort is f*cked!
 */
int exit_comp( EXIT_DATA **xit1, EXIT_DATA **xit2 )
{
    int d1, d2;

    d1 = (*xit1)->vdir;
    d2 = (*xit2)->vdir;

    if ( d1 < d2 )
      return -1;
    if ( d1 > d2 )
      return 1;
    return 0;
}

void sort_exits( ROOM_INDEX_DATA *room )
{
    EXIT_DATA *pexit; /* *texit */ /* Unused */
    EXIT_DATA *exits[MAX_REXITS];
    int x, nexits;

    nexits = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
    {
	exits[nexits++] = pexit;
	if ( nexits > MAX_REXITS )
	{
	    bug( "sort_exits: more than %d exits in room... fatal", nexits );
	    return;
	}
    }
    qsort( &exits[0], nexits, sizeof( EXIT_DATA * ),
		(int(*)(const void *, const void *)) exit_comp );
    for ( x = 0; x < nexits; x++ )
    {
	if ( x > 0 )
	  exits[x]->prev	= exits[x-1];
	else
	{
	  exits[x]->prev	= NULL;
	  room->first_exit	= exits[x];
	}
	if ( x >= (nexits - 1) )
	{
	  exits[x]->next	= NULL;
	  room->last_exit	= exits[x];
	}
	else
	  exits[x]->next	= exits[x+1];
    }
}

void randomize_exits( ROOM_INDEX_DATA *room, short maxdir )
{
    EXIT_DATA *pexit;
    int nexits, /* maxd, */ d0, d1, count, door; /* Maxd unused */
    int vdirs[MAX_REXITS];

    nexits = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       vdirs[nexits++] = pexit->vdir;

    for ( d0 = 0; d0 < nexits; d0++ )
    {
	if ( vdirs[d0] > maxdir )
	  continue;
	count = 0;
	while ( vdirs[(d1 = number_range( d0, nexits - 1 ))] > maxdir
	||      ++count > 5 );
	if ( vdirs[d1] > maxdir )
	  continue;
	door		= vdirs[d0];
	vdirs[d0]	= vdirs[d1];
	vdirs[d1]	= door;
    }
    count = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       pexit->vdir = vdirs[count++];

    sort_exits( room );
}


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
    CHAR_DATA *mob;

    if ( !pMobIndex )
    {
	bug( "Create_mobile: NULL pMobIndex." );
	exit( 1 );
    }

    CREATE( mob, CHAR_DATA, 1 );
    clear_char( mob );
    mob->pIndexData		= pMobIndex;

    mob->editor			= NULL;
    mob->name			= QUICKLINK( pMobIndex->player_name );
    mob->short_descr		= QUICKLINK( pMobIndex->short_descr );
    mob->long_descr		= QUICKLINK( pMobIndex->long_descr  );
    mob->description		= QUICKLINK( pMobIndex->description );
    mob->spec_fun		= pMobIndex->spec_fun;    
    mob->spec_2		= pMobIndex->spec_2;    
    mob->mpscriptpos		= 0;
    mob->top_level		= number_fuzzy( pMobIndex->level );
    mob->act			= pMobIndex->act;
    mob->affected_by		= pMobIndex->affected_by;
    mob->alignment		= pMobIndex->alignment;
    mob->sex			= pMobIndex->sex;
    mob->mob_clan               = NULL;
    mob->was_sentinel           = NULL;
    mob->plr_home               = NULL;
    mob->guard_data             = NULL;
    
    if ( !pMobIndex->ac )
      mob->armor		= pMobIndex->ac;
    else
      mob->armor		= 100 - mob->top_level*2;
    mob->armor = URANGE ( -100 , mob->armor , 100 );
    if ( !pMobIndex->hitnodice )
      mob->max_hit		= 10 + mob->top_level * number_range(1, 5 ) + number_range(1,mob->top_level);
    else
      mob->max_hit		= pMobIndex->hitnodice * number_range(1, pMobIndex->hitsizedice )
				      + pMobIndex->hitplus;
    mob->max_hit = URANGE ( 1 , mob->max_hit , 1000 );
    mob->hit			= mob->max_hit;
    /* lets put things back the way they used to be! -Thoric */
    mob->gold			= pMobIndex->gold;
    mob->position		= pMobIndex->position;
    mob->defposition		= pMobIndex->defposition;
    mob->barenumdie		= pMobIndex->damnodice;
    mob->baresizedie		= pMobIndex->damsizedice;
    mob->mobthac0		= pMobIndex->mobthac0;
    mob->hitplus		= pMobIndex->hitplus;
    mob->damplus		= pMobIndex->damplus;

    mob->perm_str		= pMobIndex->perm_str;
    mob->perm_dex		= pMobIndex->perm_dex;
    mob->perm_wis		= pMobIndex->perm_wis;
    mob->perm_int		= pMobIndex->perm_int;
    mob->perm_con		= pMobIndex->perm_con;
    mob->perm_cha		= pMobIndex->perm_cha;
    mob->perm_lck 		= pMobIndex->perm_lck;
    mob->hitroll		= pMobIndex->hitroll;
    mob->damroll		= pMobIndex->damroll;
    mob->xflags			= pMobIndex->xflags;
    mob->saving_poison_death	= pMobIndex->saving_poison_death;
    mob->saving_wand		= pMobIndex->saving_wand;
    mob->saving_para_petri	= pMobIndex->saving_para_petri;
    mob->saving_breath		= pMobIndex->saving_breath;
    mob->saving_spell_staff	= pMobIndex->saving_spell_staff;
    mob->height			= pMobIndex->height;
    mob->weight			= pMobIndex->weight;
    mob->resistant		= pMobIndex->resistant;
    mob->immune			= pMobIndex->immune;
    mob->susceptible		= pMobIndex->susceptible;
    mob->attacks		= pMobIndex->attacks;
    mob->defenses		= pMobIndex->defenses;
    mob->numattacks		= pMobIndex->numattacks;
    
    /*
     * Insert in list.
     */
    add_char( mob );
    pMobIndex->count++;
    nummobsloaded++;
    return mob;
}



/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
    OBJ_DATA *obj;

    if ( !pObjIndex )
    {
	bug( "Create_object: NULL pObjIndex." );
	exit( 1 );
    }

    CREATE( obj, OBJ_DATA, 1 );

    obj->pIndexData	= pObjIndex;
    obj->in_room	= NULL;
    obj->level		= level;
    obj->wear_loc	= -1;
    obj->count		= 1;
    cur_obj_serial = UMAX((cur_obj_serial + 1 ) & (BV30-1), 1);
    obj->serial = obj->pIndexData->serial = cur_obj_serial;

    obj->name		= QUICKLINK( pObjIndex->name 	 );
    obj->short_descr	= QUICKLINK( pObjIndex->short_descr );
    obj->description	= QUICKLINK( pObjIndex->description );
    obj->action_desc	= QUICKLINK( pObjIndex->action_desc );
    obj->item_type	= pObjIndex->item_type;
    obj->extra_flags	= pObjIndex->extra_flags;
    obj->wear_flags	= pObjIndex->wear_flags;
    obj->value[0]	= pObjIndex->value[0];
    obj->value[1]	= pObjIndex->value[1];
    obj->value[2]	= pObjIndex->value[2];
    obj->value[3]	= pObjIndex->value[3];
    obj->value[4]	= pObjIndex->value[4];
    obj->value[5]	= pObjIndex->value[5];
    obj->weight		= pObjIndex->weight;
    obj->cost		= pObjIndex->cost;
    /*
    obj->cost		= number_fuzzy( 10 )
			* number_fuzzy( level ) * number_fuzzy( level );
     */

    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
    default:
	bug( "Read_object: vnum %ld bad type.", pObjIndex->vnum );
	bug( "------------------------>     ", obj->item_type );
	break;
	
    case ITEM_NONE:
        break;
        
    case ITEM_LENS:
    case ITEM_RESOURCE:
    case ITEM_CRYSTAL:
    case ITEM_PLASTIC:
    case ITEM_METAL:
    case ITEM_SUPERCONDUCTOR:
    case ITEM_COMLINK:
    case ITEM_MEDPAC:
    case ITEM_FABRIC:
    case ITEM_RARE_METAL:
    case ITEM_MAGNET:
    case ITEM_THREAD:
    case ITEM_OVEN:
    case ITEM_MIRROR:
    case ITEM_CIRCUIT:
    case ITEM_TOOLKIT:
    case ITEM_LIGHT:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
    case ITEM_DRINK_CON:
	break;
    case ITEM_FOOD:
	/*
	 * optional food condition (rotting food)		-Thoric
	 * value1 is the max condition of the food
	 * value4 is the optional initial condition
	 */
	if ( obj->value[4] )
	  obj->timer = obj->value[4];
	else
	  obj->timer = obj->value[1];
	break;
	
    case ITEM_DROID_CORPSE:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_SCRAPS:
    case ITEM_PAPER:
    case ITEM_PEN:
    case ITEM_LOCKPICK:
    case ITEM_SHOVEL:
	break;

    case ITEM_DEVICE:
	obj->value[0]	= number_fuzzy( obj->value[0] );
	obj->value[1]	= number_fuzzy( obj->value[1] );
	obj->value[2]	= obj->value[1];
	break;
    
    case ITEM_BATTERY:
        if ( obj->value[0] <= 0 )
          obj->value[0] = number_fuzzy(95);
        break;
    
    
    case ITEM_AMMO:
        if ( obj->value[0] <=0 )  
          obj->value[0] = number_fuzzy(495);
        break;
        
    case ITEM_WEAPON:
	if ( obj->value[1] && obj->value[2] )
	   obj->value[2] *= obj->value[1];
	else
	{
	   obj->value[1] = number_fuzzy( number_fuzzy( 1 + level/20 ) );
	   obj->value[2] = number_fuzzy( number_fuzzy( 10 + level/10 ) );
	}
	if ( obj->value[1] > obj->value[2] )
	   obj->value[1] = obj->value[2]/3;
	if (obj->value[0] == 0)
	   obj->value[0] = INIT_WEAPON_CONDITION;
	switch (obj->value[3])
	{ 
	  case WEAPON_BLASTER: 
	  case WEAPON_LIGHTSABER: 
	  case WEAPON_VIBRO_BLADE:
	    if ( obj->value[5] <=0 )
	      obj->value[5] = number_fuzzy(1000);
	}
	obj->value[4] = obj->value[5]; 
	break;

    case ITEM_ARMOR:
	if (obj->value[0] == 0)
	    obj->value[0] = obj->value[1];
	obj->timer = obj->value[3];
	break;

    case ITEM_MONEY:
	obj->value[0]	= obj->cost;
	break;
    }

    LINK( obj, first_object, last_object, next, prev );
    ++pObjIndex->count;
    ++numobjsloaded;
    ++physicalobjects;

    return obj;
}


/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
    ch->editor			= NULL;
    ch->hunting			= NULL;
    ch->fearing			= NULL;
    ch->hating			= NULL;
    ch->name			= NULL;
    ch->short_descr		= NULL;
    ch->long_descr		= NULL;
    ch->description		= NULL;
    ch->next			= NULL;
    ch->prev			= NULL;
    ch->first_carrying		= NULL;
    ch->last_carrying		= NULL;
    ch->next_in_room		= NULL;
    ch->prev_in_room		= NULL;
    ch->fighting		= NULL;
    ch->switched		= NULL;
    ch->first_affect		= NULL;
    ch->last_affect		= NULL;
    ch->prev_cmd		= NULL;    /* maps */
    ch->last_cmd		= NULL;
    ch->dest_buf		= NULL;
    ch->dest_buf_2		= NULL;
    ch->spare_ptr		= NULL;
    ch->mount			= NULL;
    ch->affected_by		= 0;
    ch->logon			= current_time;
    ch->armor			= 100;
    ch->position		= POS_STANDING;
    ch->hit			= 100;
    ch->max_hit			= 100;
    ch->mana			= 1000;
    ch->max_mana		= 0;
    ch->move			= 500;
    ch->max_move		= 500;
    ch->height			= 72;
    ch->weight			= 180;
    ch->xflags			= 0;
    ch->barenumdie		= 1;
    ch->baresizedie		= 4;
    ch->substate		= 0;
    ch->tempnum			= 0;
    ch->perm_str		= 10;
    ch->perm_dex		= 10;
    ch->perm_int		= 10;
    ch->perm_wis		= 10;
    ch->perm_cha		= 10;
    ch->perm_con		= 10;
    ch->perm_lck		= 10;
    ch->mod_str			= 0;
    ch->mod_dex			= 0;
    ch->mod_int			= 0;
    ch->mod_wis			= 0;
    ch->mod_cha			= 0;
    ch->mod_con			= 0;
    ch->mod_lck			= 0;
    ch->pagelen                 = 24; 		     /* BUILD INTERFACE */
    ch->inter_page 		= NO_PAGE;           /* BUILD INTERFACE */
    ch->inter_type 		= NO_TYPE;           /* BUILD INTERFACE */
    ch->inter_editing    	= NULL;              /* BUILD INTERFACE */
    ch->inter_editing_vnum	= -1;                /* BUILD INTERFACE */
    ch->inter_substate    	= SUB_NORTH;         /* BUILD INTERFACE */
    ch->plr_home                = NULL;
    return;
}



/*
 * Free a character.
 */
void free_char( CHAR_DATA *ch )
{
    AFFECT_DATA *paf;
    TIMER *timer;
    MPROG_ACT_LIST *mpact, *mpact_next;

    if ( !ch )
    {
      bug( "Free_char: null ch!" );
      return;
    }

    if ( ch->desc )
      bug( "Free_char: char still has descriptor." );

    character_extract_carried_objects( ch );

    while ( (paf = ch->last_affect) != NULL )
	affect_remove( ch, paf );

    while ( (timer = ch->first_timer) != NULL )
	extract_timer( ch, timer );
	
    STRFREE( ch->name		);
    STRFREE( ch->short_descr	);
    STRFREE( ch->long_descr	);
    STRFREE( ch->description	);
    if ( ch->editor )
      stop_editing( ch );

    if ( ch->inter_editing )
      DISPOSE( ch->inter_editing );

    stop_hunting( ch );
    stop_hating ( ch );
    stop_fearing( ch );
    free_fight  ( ch );

    if ( ch->pnote )
	free_note( ch->pnote );

    if ( ch->pcdata )
    {
	STRFREE( ch->pcdata->clan_name	);
        DISPOSE( ch->pcdata->pwd	);  /* no hash */
	DISPOSE( ch->pcdata->email	);  /* no hash */
	DISPOSE( ch->pcdata->bamfin	);  /* no hash */
	DISPOSE( ch->pcdata->bamfout	);  /* no hash */
	DISPOSE( ch->pcdata->rank	);
	STRFREE( ch->pcdata->title	);
	STRFREE( ch->pcdata->bio	); 
	DISPOSE( ch->pcdata->bestowments ); /* no hash */
	DISPOSE( ch->pcdata->homepage	);  /* no hash */
	STRFREE( ch->pcdata->authed_by	);
	STRFREE( ch->pcdata->prompt	);
	if ( ch->pcdata->subprompt )
	   STRFREE( ch->pcdata->subprompt );
	DISPOSE( ch->pcdata );
     }

    for ( mpact = ch->mpact; mpact; mpact = mpact_next )
    {
	mpact_next = mpact->next;
	DISPOSE( mpact->buf );
	DISPOSE( mpact	    );
    }

    DISPOSE( ch );
    return;
}



/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed; ed = ed->next )
	if ( is_name( name, ed->keyword ) )
	    return ed->description;

    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( long vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    if ( vnum < 0 )
      vnum = 0;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
	  pMobIndex;
	  pMobIndex  = pMobIndex->next )
	if ( pMobIndex->vnum == vnum )
	    return pMobIndex;

    if ( fBootDb )
	bug( "Get_mob_index: bad vnum %ld.", vnum );

    return NULL;
}



/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( long vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    if ( vnum < 0 )
      vnum = 0;
    
    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	  pObjIndex;
	  pObjIndex  = pObjIndex->next )
	if ( pObjIndex->vnum == vnum )
	    return pObjIndex;

    if ( fBootDb )
	bug( "Get_obj_index: bad vnum %ld.", vnum );

    return NULL;
}



/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( long vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( vnum < 0 )
      vnum = 0;
    
    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	  pRoomIndex;
	  pRoomIndex  = pRoomIndex->next )
	if ( pRoomIndex->vnum == vnum )
	    return pRoomIndex;

    if ( fBootDb )
	bug( "Get_room_index: bad vnum %ld.", vnum );

    return NULL;
}



/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game.  Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far.   These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 * -- Altrag
 */


/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_letter: EOF encountered on read.\r\n");
          if ( fBootDb )
            exit(1);
          return '\0';
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_number: EOF encountered on read.\r\n");
          if ( fBootDb )
            exit(1);
          return 0;
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	bug( "Fread_number: bad format. (%c)", c );
	if ( fBootDb )
	  exit( 1 );
	return 0;
    }

    while ( isdigit(c) )
    {
        if ( feof(fp) )
        {
          bug("fread_number: EOF encountered on read.\r\n");
          if ( fBootDb )
            exit(1);
          return number;
        }
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}


/*
 * custom str_dup using create					-Thoric
 */
char *str_dup( const char *str )
{
    static char *ret;
    int len;

    if ( !str )
	return NULL;
    
    len = strlen(str)+1;

    CREATE( ret, char, len );
    strcpy( ret, str );
    return ret;
}

/*
 * Read a string from file fp
 */
char *fread_string( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_string: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    return STRALLOC("");
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return STRALLOC( "" );

    for ( ;; )
    {
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	     bug( "fread_string: string too long" );
	     *plast = '\0';
	     return STRALLOC( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    bug( "Fread_string: EOF" );
	    if ( fBootDb )
	      exit( 1 );
	    *plast = '\0';
	    return STRALLOC(buf);
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return STRALLOC( buf );
	}
    }
}

/*
 * Read a string from file fp using str_dup (ie: no string hashing)
 */
char *fread_string_nohash( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_string_no_hash: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    return str_dup("");
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return str_dup( "" );

    for ( ;; )
    {
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	   bug( "fread_string_no_hash: string too long" );
	   *plast = '\0';
	   return str_dup( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    bug( "Fread_string_no_hash: EOF" );
	    if ( fBootDb )
	      exit( 1 );
	    *plast = '\0';
	    return str_dup(buf);
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return str_dup( buf );
	}
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
	if ( feof(fp) )
	{
	    bug("fread_to_eol: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    return;
	}
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

/*
 * Read to end of line into static buffer			-Thoric
 */
char *fread_line( FILE *fp )
{
    static char line[MAX_STRING_LENGTH];
    char *pline;
    char c;
    int ln;

    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_line: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    strcpy(line, "");
	    return line;
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    ungetc( c, fp );
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_line: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    *pline = '\0';
	    return line;
	}
	c = getc( fp );
	*pline++ = c; ln++;
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	    bug( "fread_line: line too long" );
	    break;
	}
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    *pline = '\0';
    return line;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
	if ( feof(fp) )
	{
	    bug("fread_word: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    word[0] = '\0';
	    return word;
	}
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	if ( feof(fp) )
	{
	    bug("fread_word: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    *pword = '\0';
	    return word;
	}
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    bug( "Fread_word: word too long" );
    exit( 1 );
    return NULL;
}


void do_memory( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int hash;

    argument = one_argument( argument, arg );
    ch_printf( ch, "Affects %5d    Areas   %5d\r\n",  top_affect, top_area   );
    ch_printf( ch, "ExtDes  %5d    Exits   %5d\r\n", top_ed,	 top_exit   );
    ch_printf( ch, "Helps   %5d\r\n", top_help  );
    ch_printf( ch, "IdxMobs %5d    Mobs    %5d\r\n", top_mob_index, nummobsloaded );
    ch_printf( ch, "IdxObjs %5d    Objs    %5d (%d)\r\n", top_obj_index, numobjsloaded, physicalobjects );
    ch_printf( ch, "Rooms   %5d    VRooms  %5d\r\n", top_room,   top_vroom   );
    ch_printf( ch, "Shops   %5d    RepShps %5d\r\n", top_shop,   top_repair );
    ch_printf( ch, "CurOq's %5d    CurCq's %5d\r\n", cur_qobjs,  cur_qchars );
    ch_printf( ch, "Players %5d    Maxplrs %5d\r\n", num_descriptors, sysdata.maxplayers );
    ch_printf( ch, "MaxEver %5d    Topsn   %5d (%d)\r\n", sysdata.alltimemax, top_sn, MAX_SKILL );
    ch_printf( ch, "MaxEver time recorded at:   %s\r\n", sysdata.time_of_max );
    if ( !str_cmp( arg, "check" ) )
    {
#ifdef HASHSTR
	send_to_char( check_hash(argument), ch );
#else
	send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
	return;
    }
    if ( !str_cmp( arg, "showhigh" ) )
    {
#ifdef HASHSTR
	show_high_hash( atoi(argument) );
#else
	send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
	return;
    }
    if ( argument[0] != '\0' )
      hash = atoi(argument);
    else
      hash = -1;
    if ( !str_cmp( arg, "hash" ) )
    {
#ifdef HASHSTR
	ch_printf( ch, "Hash statistics:\r\n%s", hash_stats() );
	if ( hash != -1 )
	  hash_dump( hash );
#else
	send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
    }
    return;
}



/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
	case 0:  number -= 1; break;
	case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
/*    int power;
    int number;*/

    if ( ( to = to - from + 1 ) <= 1 )
	return from;

/*    for ( power = 2; power < to; power <<= 1 )
	;

    while ( ( number = number_mm( ) & (power - 1) ) >= to )
	;

    return from + number;*/
    return (number_mm() % to) + from;
}



/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
/*    int percent;

    while ( ( percent = number_mm( ) & (128-1) ) > 99 )
	;

    return 1 + percent;*/
    return number_mm() % 100;
}



/*
 * Generate a random door.
 */
int number_door( void )
{
    int door;

    while ( ( door = number_mm( ) & (16-1) ) > 9 )
	;

    return door;
/*    return number_mm() & 10; */
}



int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static	int	rgiState[2+55];

void init_mm( )
{
    int *piState;
    int iState;

    piState	= &rgiState[2];

    piState[-2]	= 55 - 55;
    piState[-1]	= 55 - 24;

    piState[0]	= ((int) current_time) & ((1 << 30) - 1);
    piState[1]	= 1;
    for ( iState = 2; iState < 55; iState++ )
    {
	piState[iState] = (piState[iState-1] + piState[iState-2])
			& ((1 << 30) - 1);
    }
    return;
}



int number_mm( void )
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState		= &rgiState[2];
    iState1	 	= piState[-2];
    iState2	 	= piState[-1];
    iRand	 	= (piState[iState1] + piState[iState2])
			& ((1 << 30) - 1);
    piState[iState1]	= iRand;
    if ( ++iState1 == 55 )
	iState1 = 0;
    if ( ++iState2 == 55 )
	iState2 = 0;
    piState[-2]		= iState1;
    piState[-1]		= iState2;
    return iRand >> 6;
}



/*
 * Roll some dice.						-Thoric
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
      case 0: return 0;
      case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
	if ( *str == '~' )
	    *str = '-';

    return;
}

/*
 * Encodes the tildes in a string.				-Thoric
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
	if ( *str == '~' )
	    *str = HIDDEN_TILDE;

    return;
}

char *show_tilde( char *str )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufptr;

    bufptr = buf;
    for ( ; *str != '\0'; str++, bufptr++ )
    {
	if ( *str == HIDDEN_TILDE )
	    *bufptr = '~';
	else
	    *bufptr = *str;
    }
    *bufptr = '\0';

    return buf;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	bug( "Str_cmp: null astr." );
	if ( bstr )
	  fprintf( stderr, "str_cmp: astr: (null)  bstr: %s\n", bstr );
	return TRUE;
    }

    if ( !bstr )
    {
	bug( "Str_cmp: null bstr." );
	if ( astr )
	  fprintf( stderr, "str_cmp: astr: %s  bstr: (null)\n", astr );
	return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	bug( "Strn_cmp: null astr." );
	return TRUE;
    }

    if ( !bstr )
    {
	bug( "Strn_cmp: null bstr." );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
	return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
	if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
	    return FALSE;

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return FALSE;
    else
	return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = LOWER(str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}


/*
 * Returns a lowercase string.
 */
char *strlower( const char *str )
{
    static char strlow[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strlow[i] = LOWER(str[i]);
    strlow[i] = '\0';
    return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper( const char *str )
{
    static char strup[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strup[i] = UPPER(str[i]);
    strup[i] = '\0';
    return strup;
}

/*
 * Returns TRUE or FALSE if a letter is a vowel			-Thoric
 */
bool isavowel( char letter )
{
    char c;

    c = tolower( letter );
    if ( c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' )
      return TRUE;
    else
      return FALSE;
}

/*
 * Shove either "a " or "an " onto the beginning of a string	-Thoric
 */
const char *aoran( const char *str )
{
    static char temp[MAX_STRING_LENGTH];

    if ( !str )
    {
	bug( "Aoran(): NULL str" );
	return "";
    }

    if ( isavowel(str[0])
    || ( strlen(str) > 1 && tolower(str[0]) == 'y' && !isavowel(str[1])) )
      strcpy( temp, "an " );
    else
      strcpy( temp, "a " );
    strcat( temp, str );
    return temp;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, const char *file, const char *str )
{
    FILE *fp;

    if ( IS_NPC(ch) || str[0] == '\0' )
	return;

    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	send_to_char( "Could not open the file!\r\n", ch );
    }
    else
    {
	fprintf( fp, "[%5ld] %s: %s\n",
	    ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
	fclose( fp );
    }
}

/*
 * Append a string to a file.
 */
void append_to_file( const char *file, const char *str )
{
    FILE *fp;

    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {}
    else
    {
	fprintf( fp, "%s\n", str );
	fclose( fp );
    }

    return;
}


/*
 * Reports a bug.
 */
void bug( const char *str, ... )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    struct stat fst;

    if ( fpArea != NULL )
    {
	int iLine;
	int iChar;

	if ( fpArea == stdin )
	{
	    iLine = 0;
	}
	else
	{
	    iChar = ftell( fpArea );
	    fseek( fpArea, 0, 0 );
	    for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
	    {
		while ( getc( fpArea ) != '\n' )
		    ;
	    }
	    fseek( fpArea, iChar, 0 );
	}

	sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
	log_string( buf );

	if ( stat( SHUTDOWN_FILE, &fst ) != -1 )	/* file exists */
	{
	    if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
	    {
		fprintf( fp, "[*****] %s\n", buf );
		fclose( fp );
	    }
	}
    }

    strcpy( buf, "[*****] BUG: " );
    {
	va_list param;
    
	va_start(param, str);
	vsprintf( buf + strlen(buf), str, param );
	va_end(param);
    }
    log_string( buf );

    if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf );
	fclose( fp );
    }
}

/*
 * Add a string to the boot-up log				-Thoric
 */
void boot_log( const char *str, ... )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    va_list param;

    strcpy( buf, "[*****] BOOT: " );
    va_start(param, str);
    vsprintf( buf+strlen(buf), str, param );
    va_end(param);
    log_string( buf );

    if ( ( fp = fopen( BOOTLOG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf );
 	fclose( fp );
    }
}

/*
 * Dump a text file to a player, a line at a time		-Thoric
 */
void show_file( CHAR_DATA *ch, const char *filename )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    int c;
    int num = 0;

    if ( (fp = fopen( filename, "r" )) != NULL )
    {
      while ( !feof(fp) )
      {
	while ((buf[num]=fgetc(fp)) != EOF
	&&      buf[num] != '\n'
	&&      buf[num] != '\r'
	&&      num < (MAX_STRING_LENGTH-2))
	  num++;
	c = fgetc(fp);
	if ( (c != '\n' && c != '\r') || c == buf[num] )
	  ungetc(c, fp);
	buf[num++] = '\n';
	buf[num++] = '\r';
	buf[num  ] = '\0';
	send_to_pager( buf, ch );
	num = 0;
      }
    }
}

/*
 * Show the boot log file					-Thoric
 */
void do_dmesg( CHAR_DATA *ch, char *argument )
{
    set_pager_color( AT_LOG, ch );
    show_file( ch, BOOTLOG_FILE );
}

/*
 * Writes a string to the log, extended version			-Thoric
 */
void log_string_plus( const char *str, short log_type )
{
    char *strtime;
    int offset;

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    fprintf( stderr, "%s :: %s\n", strtime, str );
    if ( strncmp( str, "Log ", 4 ) == 0 )
      offset = 4;
    else
      offset = 0;
    switch( log_type )
    {
	default:
	  to_channel( str + offset, CHANNEL_LOG, "Log", 2 );
	  break;
	case LOG_BUILD:
	  to_channel( str + offset, CHANNEL_BUILD, "Build", 2 );
	  break;
	case LOG_COMM:
	  to_channel( str + offset, CHANNEL_COMM, "Comm", 2 );
	  break;
	case LOG_ALL:
	  break;
    }
    return;
}


/* mud prog functions */

/* This routine reads in scripts of MUDprograms from a file */

int mprog_name_to_type ( char *name )
{
      if ( !str_cmp( name, "in_file_prog"   ) )	return IN_FILE_PROG;
   if ( !str_cmp( name, "act_prog"       ) )    return ACT_PROG;
   if ( !str_cmp( name, "speech_prog"    ) )	return SPEECH_PROG;
   if ( !str_cmp( name, "rand_prog"      ) ) 	return RAND_PROG;
   if ( !str_cmp( name, "fight_prog"     ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "hitprcnt_prog"  ) )	return HITPRCNT_PROG;
   if ( !str_cmp( name, "death_prog"     ) )	return DEATH_PROG;
   if ( !str_cmp( name, "entry_prog"     ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "greet_prog"     ) )	return GREET_PROG;
   if ( !str_cmp( name, "all_greet_prog" ) )	return ALL_GREET_PROG;
   if ( !str_cmp( name, "give_prog"      ) ) 	return GIVE_PROG;
   if ( !str_cmp( name, "bribe_prog"     ) )	return BRIBE_PROG;
   if ( !str_cmp( name, "time_prog"     ) )	return TIME_PROG;
   if ( !str_cmp( name, "hour_prog"     ) )	return HOUR_PROG;
   if ( !str_cmp( name, "wear_prog"     ) )	return WEAR_PROG;
   if ( !str_cmp( name, "remove_prog"   ) )	return REMOVE_PROG;
   if ( !str_cmp( name, "sac_prog"      ) )	return SAC_PROG;
   if ( !str_cmp( name, "look_prog"     ) )	return LOOK_PROG;
   if ( !str_cmp( name, "exa_prog"      ) )	return EXA_PROG;
   if ( !str_cmp( name, "zap_prog"      ) )	return ZAP_PROG;
   if ( !str_cmp( name, "get_prog"      ) ) 	return GET_PROG;
   if ( !str_cmp( name, "drop_prog"     ) )	return DROP_PROG;
   if ( !str_cmp( name, "damage_prog"   ) )	return DAMAGE_PROG;
   if ( !str_cmp( name, "repair_prog"   ) )	return REPAIR_PROG;
   if ( !str_cmp( name, "greet_prog"    ) )	return GREET_PROG;
   if ( !str_cmp( name, "randiw_prog"   ) )	return RANDIW_PROG;
   if ( !str_cmp( name, "speechiw_prog" ) )	return SPEECHIW_PROG;
   if ( !str_cmp( name, "pull_prog"	) )     return PULL_PROG;
   if ( !str_cmp( name, "push_prog"	) )     return PUSH_PROG;
   if ( !str_cmp( name, "sleep_prog"    ) )	return SLEEP_PROG;
   if ( !str_cmp( name, "rest_prog"	) )	return REST_PROG;
   if ( !str_cmp( name, "rfight_prog"   ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "enter_prog"    ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "leave_prog"    ) )	return LEAVE_PROG;
   if ( !str_cmp( name, "rdeath_prog"	) )	return DEATH_PROG;
   if ( !str_cmp( name, "script_prog"	) )	return SCRIPT_PROG;
   if ( !str_cmp( name, "use_prog"	) )	return USE_PROG;
   return( ERROR_PROG );
}

MPROG_DATA *mprog_file_read( char *f, MPROG_DATA *mprg,
			    MOB_INDEX_DATA *pMobIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile;
  char        letter;
  MPROG_DATA *mprg_next, *mprg2;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Mob: %ld couldn't open mudprog file", pMobIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty mudprog file." );
       exit( 1 );
     break;
    default:
       bug( "in mudprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "mudprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "mprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in mudprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_mudprogs( AREA_DATA *tarea, FILE *fp )
{
  MOB_INDEX_DATA *iMob;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  long             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_mudprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iMob = get_mob_index( value ) ) == NULL )
      {
	bug( "Load_mudprogs: vnum %ld doesnt exist", value );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iMob->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iMob->mudprogs = working;
      working = mprog_file_read( fread_word( fp ), working, iMob );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file MUDprograms.
 */

void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_mobiles: vnum %ld MUDPROG char", pMobIndex->vnum );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pMobIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_mobiles: vnum %ld MUDPROG type.", pMobIndex->vnum );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = mprog_file_read( fread_string( fp ), mprg,pMobIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_mobiles: vnum %ld bad MUDPROG.", pMobIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
     default:
	pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_mobiles: vnum %ld bad MUDPROG.", pMobIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}



/*************************************************************/
/* obj prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */


MPROG_DATA *oprog_file_read( char *f, MPROG_DATA *mprg,
			    OBJ_INDEX_DATA *pObjIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile;
  char        letter;
  MPROG_DATA *mprg_next, *mprg2;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Obj: %d couldnt open mudprog file", pObjIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty objprog file." );
       exit( 1 );
     break;
    default:
       bug( "in objprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "objprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "objprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	pObjIndex->progtypes = pObjIndex->progtypes | mprg2->type;
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in objprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_objprogs( AREA_DATA *tarea, FILE *fp )
{
  OBJ_INDEX_DATA *iObj;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  long             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_objprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iObj = get_obj_index( value ) ) == NULL )
      {
	bug( "Load_objprogs: vnum %ld doesnt exist", value );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iObj->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iObj->mudprogs = working;
      working = oprog_file_read( fread_word( fp ), working, iObj );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file OBJprograms.
 */

void oprog_read_programs( FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_objects: vnum %ld OBJPROG char", pObjIndex->vnum );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pObjIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_objects: vnum %ld OBJPROG type.", pObjIndex->vnum );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = oprog_file_read( fread_string( fp ), mprg,pObjIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_objects: vnum %ld bad OBJPROG.", pObjIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
     default:
	pObjIndex->progtypes = pObjIndex->progtypes | mprg->type;
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_objects: vnum %ld bad OBJPROG.", pObjIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}


/*************************************************************/
/* room prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */
MPROG_DATA *rprog_file_read( char *f, MPROG_DATA *mprg,
			    ROOM_INDEX_DATA *RoomIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile;
  char        letter;
  MPROG_DATA *mprg_next, *mprg2;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Room: %ld couldnt open roomprog file", RoomIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty roomprog file." );
       exit( 1 );
     break;
    default:
       bug( "in roomprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "roomprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "roomprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	RoomIndex->progtypes = RoomIndex->progtypes | mprg2->type;
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in roomprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}

/* Load a ROOMprogram section from the area file.
 */
void load_roomprogs( AREA_DATA *tarea, FILE *fp )
{
  ROOM_INDEX_DATA *iRoom;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  long             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_objprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iRoom = get_room_index( value ) ) == NULL )
      {
	bug( "Load_roomprogs: vnum %ld doesnt exist", value );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iRoom->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iRoom->mudprogs = working;
      working = rprog_file_read( fread_word( fp ), working, iRoom );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file ROOMprograms.
 */

void rprog_read_programs( FILE *fp, ROOM_INDEX_DATA *pRoomIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_rooms: vnum %ld ROOMPROG char", pRoomIndex->vnum );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pRoomIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_rooms: vnum %ld ROOMPROG type.", pRoomIndex->vnum );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = rprog_file_read( fread_string( fp ), mprg,pRoomIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_rooms: vnum %ld bad ROOMPROG.", pRoomIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
     default:
	pRoomIndex->progtypes = pRoomIndex->progtypes | mprg->type;
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_rooms: vnum %ld bad ROOMPROG.", pRoomIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}


/*************************************************************/
/* Function to delete a room index.  Called from do_rdelete in build.c
   Narn, May/96
*/
bool delete_room( ROOM_INDEX_DATA *room )
{
    ROOM_INDEX_DATA *tmp = NULL, *prev = NULL;
    int iHash = room->vnum % MAX_KEY_HASH;

    /* Take the room index out of the hash list. */
    for( tmp = room_index_hash[iHash]; tmp && tmp != room; tmp = tmp->next )
    {
      prev = tmp;
    }

    if( !tmp )
    {
      bug( "Delete_room: room not found" );
      return FALSE;
    }

    if( prev )
    {
      prev->next = room->next;
    }
    else
    {
      room_index_hash[iHash] = room->next;
    }   
 
    /* Free up the ram for all strings attached to the room. */
    STRFREE( room->name );
    STRFREE( room->description );

    /* Free up the ram held by the room index itself. */
    free( room );

    top_room--;
    return TRUE;
}

/* See comment on delete_room. */
bool delete_obj( OBJ_INDEX_DATA *obj )
{
    return TRUE;
}

/* See comment on delete_room. */
bool delete_mob( MOB_INDEX_DATA *mob )
{
    return TRUE;
}

/*
 * Creat a new room (for online building)			-Thoric
 */
ROOM_INDEX_DATA *make_room( long vnum )
{
	ROOM_INDEX_DATA *pRoomIndex;
	int	iHash;

	CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
	pRoomIndex->first_person	= NULL;
	pRoomIndex->last_person		= NULL;
	pRoomIndex->first_content	= NULL;
	pRoomIndex->last_content	= NULL;
	pRoomIndex->first_extradesc	= NULL;
	pRoomIndex->last_extradesc	= NULL;
	pRoomIndex->first_ship          = NULL;
	pRoomIndex->last_ship		= NULL;
	pRoomIndex->next_in_area         = NULL;
	pRoomIndex->prev_in_area	= NULL;
	  pRoomIndex->next_in_ship      = NULL;
	  pRoomIndex->prev_in_ship      = NULL;
	pRoomIndex->area		= NULL;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= STRALLOC("Floating in a void");
	pRoomIndex->description		= STRALLOC("");
	pRoomIndex->room_flags		= 0;
	pRoomIndex->sector_type		= 1;
	pRoomIndex->light		= 0;
	pRoomIndex->first_exit		= NULL;
	pRoomIndex->last_exit		= NULL;

	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;
	top_room++;

	return pRoomIndex;
}

ROOM_INDEX_DATA *make_ship_room( SHIP_DATA * ship )
{
  ROOM_INDEX_DATA *pRoomIndex;

  CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
  pRoomIndex->first_person	= NULL;
  pRoomIndex->last_person		= NULL;
  pRoomIndex->first_content	= NULL;
  pRoomIndex->last_content	= NULL;
  pRoomIndex->first_extradesc	= NULL;
  pRoomIndex->last_extradesc	= NULL;
  pRoomIndex->first_ship          = NULL;
  pRoomIndex->last_ship		= NULL;
  pRoomIndex->next_in_area        = NULL;
  pRoomIndex->prev_in_area	= NULL;
  pRoomIndex->next_in_ship      	= NULL;
  pRoomIndex->prev_in_ship      	= NULL;
  pRoomIndex->area		= NULL;
  pRoomIndex->vnum		= -1;
  pRoomIndex->name		= STRALLOC( ship->name );
  pRoomIndex->description		= STRALLOC( "" );
  pRoomIndex->room_flags		= 0;
  pRoomIndex->sector_type		= 0;
  pRoomIndex->light		= 0;
  pRoomIndex->first_exit		= NULL;
  pRoomIndex->last_exit		= NULL;
        
  SET_BIT ( pRoomIndex->room_flags , ROOM_SPACECRAFT );
  SET_BIT ( pRoomIndex->room_flags , ROOM_INDOORS );
        
  LINK( pRoomIndex , ship->first_room , ship->last_room , next_in_ship , prev_in_ship );
        
  top_room++;

  return pRoomIndex;
}

/*
 * Create a new INDEX object (for online building)		-Thoric
 * Option to clone an existing index object.
 */
OBJ_INDEX_DATA *make_object( long vnum, long cvnum, char *name )
{
	OBJ_INDEX_DATA *pObjIndex, *cObjIndex;
	char buf[MAX_STRING_LENGTH];
	int	iHash;

	if ( cvnum > 0 )
	  cObjIndex = get_obj_index( cvnum );
	else
	  cObjIndex = NULL;
	CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
	pObjIndex->vnum			= vnum;
	pObjIndex->name			= STRALLOC( name );
	pObjIndex->first_affect		= NULL;
	pObjIndex->last_affect		= NULL;
	pObjIndex->first_extradesc	= NULL;
	pObjIndex->last_extradesc	= NULL;
	if ( !cObjIndex )
	{
	  sprintf( buf, "A %s", name );
	  pObjIndex->short_descr	= STRALLOC( buf  );
	  sprintf( buf, "A %s is here.", name );
	  pObjIndex->description	= STRALLOC( buf );
	  pObjIndex->action_desc	= STRALLOC( "" );
	  pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);
	  pObjIndex->description[0]	= UPPER(pObjIndex->description[0]);
	  pObjIndex->item_type		= ITEM_TRASH;
	  pObjIndex->extra_flags	= ITEM_PROTOTYPE;
	  pObjIndex->wear_flags		= 0;
	  pObjIndex->value[0]		= 0;
	  pObjIndex->value[1]		= 0;
	  pObjIndex->value[2]		= 0;
	  pObjIndex->value[3]		= 0;
	  pObjIndex->weight		= 1;
	  pObjIndex->cost		= 0;
	}
	else
	{
	  EXTRA_DESCR_DATA *ed,  *ced;
	  AFFECT_DATA	   *paf, *cpaf;

	  pObjIndex->short_descr	= QUICKLINK( cObjIndex->short_descr );
	  pObjIndex->description	= QUICKLINK( cObjIndex->description );
	  pObjIndex->action_desc	= QUICKLINK( cObjIndex->action_desc );
	  pObjIndex->item_type		= cObjIndex->item_type;
	  pObjIndex->extra_flags	= cObjIndex->extra_flags
	  				| ITEM_PROTOTYPE;
	  pObjIndex->wear_flags		= cObjIndex->wear_flags;
	  pObjIndex->value[0]		= cObjIndex->value[0];
	  pObjIndex->value[1]		= cObjIndex->value[1];
	  pObjIndex->value[2]		= cObjIndex->value[2];
	  pObjIndex->value[3]		= cObjIndex->value[3];
	  pObjIndex->weight		= cObjIndex->weight;
	  pObjIndex->cost		= cObjIndex->cost;
	  for ( ced = cObjIndex->first_extradesc; ced; ced = ced->next )
	  {
		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= QUICKLINK( ced->keyword );
		ed->description		= QUICKLINK( ced->description );
		LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
			  next, prev );
		top_ed++;
	  }
	  for ( cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next )
	  {
		CREATE( paf, AFFECT_DATA, 1 );
		paf->type		= cpaf->type;
		paf->duration		= cpaf->duration;
		paf->location		= cpaf->location;
		paf->modifier		= cpaf->modifier;
		paf->bitvector		= cpaf->bitvector;
		LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
			   next, prev );
		top_affect++;
	  }
	}
	pObjIndex->count		= 0;
	iHash				= vnum % MAX_KEY_HASH;
	pObjIndex->next			= obj_index_hash[iHash];
	obj_index_hash[iHash]		= pObjIndex;
	top_obj_index++;

	return pObjIndex;
}

/*
 * Create a new INDEX mobile (for online building)		-Thoric
 * Option to clone an existing index mobile.
 */
MOB_INDEX_DATA *make_mobile( long vnum, long cvnum, char *name )
{
	MOB_INDEX_DATA *pMobIndex, *cMobIndex;
	char buf[MAX_STRING_LENGTH];
	int	iHash;

	if ( cvnum > 0 )
	  cMobIndex = get_mob_index( cvnum );
	else
	  cMobIndex = NULL;
	CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
	pMobIndex->vnum			= vnum;
	pMobIndex->count		= 0;
	pMobIndex->killed		= 0;
	pMobIndex->player_name		= STRALLOC( name );
	if ( !cMobIndex )
	{
	  sprintf( buf, "A newly created %s", name );
	  pMobIndex->short_descr	= STRALLOC( buf  );
	  sprintf( buf, "Some god abandoned a newly created %s here.\r\n", name );
	  pMobIndex->long_descr		= STRALLOC( buf );
	  pMobIndex->description	= STRALLOC( "" );
	  pMobIndex->short_descr[0]	= LOWER(pMobIndex->short_descr[0]);
	  pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
	  pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);
	  pMobIndex->act		= ACT_IS_NPC | ACT_PROTOTYPE;
	  pMobIndex->affected_by	= 0;
	  pMobIndex->pShop		= NULL;
	  pMobIndex->rShop		= NULL;
	  pMobIndex->spec_fun		= NULL;
	  pMobIndex->spec_2		= NULL;
	  pMobIndex->mudprogs		= NULL;
	  pMobIndex->progtypes		= 0;
	  pMobIndex->alignment		= 0;
	  pMobIndex->level		= 1;
	  pMobIndex->mobthac0		= 0;
	  pMobIndex->ac			= 0;
	  pMobIndex->hitnodice		= 0;
	  pMobIndex->hitsizedice	= 0;
	  pMobIndex->hitplus		= 0;
	  pMobIndex->damnodice		= 0;
	  pMobIndex->damsizedice	= 0;
	  pMobIndex->damplus		= 0;
	  pMobIndex->gold		= 0;
	  pMobIndex->exp		= 0;
	  pMobIndex->position		= 8;
	  pMobIndex->defposition	= 8;
	  pMobIndex->sex		= 0;
	  pMobIndex->perm_str		= 10;
	  pMobIndex->perm_dex		= 10;
	  pMobIndex->perm_int		= 10;
	  pMobIndex->perm_wis		= 10;
	  pMobIndex->perm_cha		= 10;
	  pMobIndex->perm_con		= 10;
	  pMobIndex->perm_lck		= 10;
	  pMobIndex->xflags		= 0;
	  pMobIndex->resistant		= 0;
	  pMobIndex->immune		= 0;
	  pMobIndex->susceptible	= 0;
	  pMobIndex->numattacks		= 0;
	  pMobIndex->attacks		= 0;
	  pMobIndex->defenses		= 0;
	}
	else
	{
	  pMobIndex->short_descr	= QUICKLINK( cMobIndex->short_descr );
	  pMobIndex->long_descr		= QUICKLINK( cMobIndex->long_descr  );
	  pMobIndex->description	= QUICKLINK( cMobIndex->description );
	  pMobIndex->act		= cMobIndex->act | ACT_PROTOTYPE;
	  pMobIndex->affected_by	= cMobIndex->affected_by;
	  pMobIndex->pShop		= NULL;
	  pMobIndex->rShop		= NULL;
	  pMobIndex->spec_fun		= cMobIndex->spec_fun;
	  pMobIndex->spec_2		= cMobIndex->spec_2;
	  pMobIndex->mudprogs		= NULL;
	  pMobIndex->progtypes		= 0;
	  pMobIndex->alignment		= cMobIndex->alignment;
	  pMobIndex->level		= cMobIndex->level;
	  pMobIndex->mobthac0		= cMobIndex->mobthac0;
	  pMobIndex->ac			= cMobIndex->ac;
	  pMobIndex->hitnodice		= cMobIndex->hitnodice;
	  pMobIndex->hitsizedice	= cMobIndex->hitsizedice;
	  pMobIndex->hitplus		= cMobIndex->hitplus;
	  pMobIndex->damnodice		= cMobIndex->damnodice;
	  pMobIndex->damsizedice	= cMobIndex->damsizedice;
	  pMobIndex->damplus		= cMobIndex->damplus;
	  pMobIndex->gold		= cMobIndex->gold;
	  pMobIndex->exp		= cMobIndex->exp;
	  pMobIndex->position		= cMobIndex->position;
	  pMobIndex->defposition	= cMobIndex->defposition;
	  pMobIndex->sex		= cMobIndex->sex;
	  pMobIndex->perm_str		= cMobIndex->perm_str;
	  pMobIndex->perm_dex		= cMobIndex->perm_dex;
	  pMobIndex->perm_int		= cMobIndex->perm_int;
	  pMobIndex->perm_wis		= cMobIndex->perm_wis;
	  pMobIndex->perm_cha		= cMobIndex->perm_cha;
	  pMobIndex->perm_con		= cMobIndex->perm_con;
	  pMobIndex->perm_lck		= cMobIndex->perm_lck;
	  pMobIndex->xflags		= cMobIndex->xflags;
	  pMobIndex->resistant		= cMobIndex->resistant;
	  pMobIndex->immune		= cMobIndex->immune;
	  pMobIndex->susceptible	= cMobIndex->susceptible;
	  pMobIndex->numattacks		= cMobIndex->numattacks;
	  pMobIndex->attacks		= cMobIndex->attacks;
	  pMobIndex->defenses		= cMobIndex->defenses;
	}
	iHash				= vnum % MAX_KEY_HASH;
	pMobIndex->next			= mob_index_hash[iHash];
	mob_index_hash[iHash]		= pMobIndex;
	top_mob_index++;

	return pMobIndex;
}

/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.						-Thoric
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, short door )
{
	EXIT_DATA *pexit, *texit;
	bool broke;

	CREATE( pexit, EXIT_DATA, 1 );
	pexit->vdir		= door;
	pexit->rvnum		= pRoomIndex->vnum;
	pexit->to_room		= to_room;
	pexit->distance		= 1;
	if ( to_room )
	{
	    pexit->vnum = to_room->vnum;
	    texit = get_exit_to( to_room, rev_dir[door], pRoomIndex->vnum );
	    if ( texit )	/* assign reverse exit pointers */
	    {
		texit->rexit = pexit;
		pexit->rexit = texit;
	    }
	}
	broke = FALSE;
	for ( texit = pRoomIndex->first_exit; texit; texit = texit->next )
	   if ( door < texit->vdir )
	   {
	     broke = TRUE;
	     break;
	   }
	if ( !pRoomIndex->first_exit )
	  pRoomIndex->first_exit	= pexit;
	else
	{
	  /* keep exits in incremental order - insert exit into list */
	  if ( broke && texit )
	  {
	    if ( !texit->prev )
	      pRoomIndex->first_exit	= pexit;
	    else
	      texit->prev->next		= pexit;
	    pexit->prev			= texit->prev;
	    pexit->next			= texit;
	    texit->prev			= pexit;
	    top_exit++;
	    return pexit;
	  }
	  pRoomIndex->last_exit->next	= pexit;
	}
	pexit->next			= NULL;
	pexit->prev			= pRoomIndex->last_exit;
	pRoomIndex->last_exit		= pexit;
	top_exit++;
	return pexit;
}

void fix_area_exits( AREA_DATA *tarea )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit, *rev_exit;
    long rnum;
    bool fexit;

    for ( pRoomIndex = tarea->first_room ; pRoomIndex ; pRoomIndex = pRoomIndex->next_in_area )
    {
	rnum = pRoomIndex->vnum;

	fexit = FALSE;
	for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	{
		fexit = TRUE;
		pexit->rvnum = pRoomIndex->vnum;
		if ( pexit->vnum <= 0 )
	       	  pexit->to_room = NULL;
		else
		  pexit->to_room = get_room_index( pexit->vnum );
	}
	if ( !fexit )
	  SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
    }


    for ( pRoomIndex = tarea->first_room ; pRoomIndex ; pRoomIndex = pRoomIndex->next_in_area )
    {
	rnum = pRoomIndex->vnum;

	for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	{
		if ( pexit->to_room && !pexit->rexit )
		{
		   rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
		   if ( rev_exit )
		   {
			pexit->rexit	= rev_exit;
			rev_exit->rexit	= pexit;
		   }
		}
	}
    }
}

void load_area_file( AREA_DATA *tarea, char *filename )
{
/*    FILE *fpin;
    what intelligent person stopped using fpArea?????
    if fpArea isn't being used, then no filename or linenumber
    is printed when an error occurs during loading the area..
    (bug uses fpArea)
      --TRI  */

    if ( fBootDb )
      tarea = last_area;
    if ( !fBootDb && !tarea )
    {
	bug( "Load_area: null area!" );
	return;
    }

    if ( ( fpArea = fopen( filename, "r" ) ) == NULL )
    {
	bug( "load_area: error loading file (can't open)" );
	bug( filename );
	return;
    }

    for ( ; ; )
    {
	char *word;

	if ( fread_letter( fpArea ) != '#' )
	{
	    bug( filename );
	    bug( "load_area: # not found." );
	    exit( 1 );
	}

	word = fread_word( fpArea );

	     if ( word[0] == '$'               )                 break;
	else if ( !str_cmp( word, "AREA"     ) )
	{
		if ( fBootDb )
		{
		  load_area    (fpArea);
		  tarea = last_area;
		}
		else
		{
		  DISPOSE( tarea->name );
		  tarea->name = fread_string_nohash( fpArea );
		}
	}
	else if ( !str_cmp( word, "FLAGS"    ) ) load_flags   (tarea, fpArea);
	else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (tarea, fpArea);
	else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (tarea, fpArea);
	else if ( !str_cmp( word, "MUDPROGS" ) ) load_mudprogs(tarea, fpArea);
	else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (tarea, fpArea);
	else if ( !str_cmp( word, "OBJPROGS" ) ) load_objprogs(tarea, fpArea);
	else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (tarea, fpArea);
	else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (tarea, fpArea);
	else if ( !str_cmp( word, "REPAIRS"  ) ) load_repairs (tarea, fpArea);
	else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(tarea, fpArea);
	else
	{
	    bug( filename );
	    bug( "load_area: bad section name." );
	    if ( fBootDb )
	      exit( 1 );
	    else
	    {
	      fclose( fpArea );
	      return;
	    }
	}
    }
    fclose( fpArea );
    if ( tarea )
    {
	if ( fBootDb )
	  sort_area( tarea, FALSE );

	fprintf( stderr, "%s\n",
		 filename);
    }
    else
      fprintf( stderr, "(%s)\n", filename );
}



/*
 * Sort by room vnums					-Altrag & Thoric
 */
void sort_area( AREA_DATA *pArea, bool proto )
{
    AREA_DATA *first_sort, *last_sort;
    bool found;

    if ( !pArea )
    {
	bug( "Sort_area: NULL pArea" );
	return;
    }

    if ( proto )
    {
	first_sort = first_bsort;
	last_sort  = last_bsort;
    }
    else
    {
	first_sort = first_asort;
	last_sort  = last_asort;
    }
	
    found = FALSE;
    pArea->next_sort = NULL;
    pArea->prev_sort = NULL;

    if ( !first_sort )
    {
	pArea->prev_sort = NULL;
	pArea->next_sort = NULL;
	first_sort	 = pArea;
	last_sort	 = pArea;
	found = TRUE;
    }

    if ( !found )
    {
	pArea->prev_sort     = last_sort;
	pArea->next_sort     = NULL;
	last_sort->next_sort = pArea;
	last_sort	     = pArea;
    }

    if ( proto )
    {
	first_bsort = first_sort;
	last_bsort  = last_sort;
    }
    else
    {
	first_asort = first_sort;
	last_asort  = last_sort;
    }
}


/*
 * Display vnums currently assigned to areas		-Altrag & Thoric
 * Sorted, and flagged if loaded.
 */
void show_vnums( CHAR_DATA *ch, int low, int high, bool proto, bool shownl,
		 const char *loadst, const char *notloadst )
{
    return;
}

/*
 * Shows prototype vnums ranges, and if loaded
 */

void do_vnums( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 32766;
    if ( arg1[0] != '\0' )
    {
	low = atoi(arg1);
	if ( arg2[0] != '\0' )
	  high = atoi(arg2);
    }
    show_vnums( ch, low, high, TRUE, TRUE, " *", "" );
}

/*
 * Shows installed areas, sorted.  Mark unloaded areas with an X
 */
void do_zones( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    set_pager_color( AT_PLAIN, ch );

    pager_printf(ch, "Zones\r\n" );
    pager_printf(ch, "----------------------------------------------\r\n" );    
    
    for ( pArea = first_area; pArea; pArea = pArea->next )
	pager_printf(ch, "%s\r\n", pArea->filename );

    pager_printf(ch, "----------------------------------------------\r\n" );	
    return;

}


/*
 * Save system info to data file
 */
void save_sysdata( SYSTEM_DATA sys )
{
    FILE *fp;
    char filename[MAX_INPUT_LENGTH];

    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );
    
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_sysdata: fopen" );
    }
    else
    {
	fprintf( fp, "#SYSTEM\n" );
	fprintf( fp, "Highplayers    %d\n", sys.alltimemax		);
	fprintf( fp, "Highplayertime %s~\n", sys.time_of_max		);
	fprintf( fp, "Nameresolving  %d\n", sys.NO_NAME_RESOLVING	);
	fprintf( fp, "Waitforauth    %d\n", sys.WAIT_FOR_AUTH		);
	fprintf( fp, "Saveflags      %d\n", sys.save_flags		);
	fprintf( fp, "Savefreq       %d\n", sys.save_frequency		);
	fprintf( fp, "Officials      %s~\n", sys.officials		);
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
}


void fread_sysdata( SYSTEM_DATA *sys, FILE *fp )
{
  const char *word;
  bool fMatch;

    sys->time_of_max = NULL;
    sys->officials = NULL;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !sys->time_of_max )
		    sys->time_of_max = str_dup("(not recorded)");
		if ( !sys->officials )
		    sys->officials = STRALLOC("");
		return;
	    }
	    break;

	case 'H':
	    KEY( "Highplayers",	   sys->alltimemax,	  fread_number( fp ) );
	    KEY( "Highplayertime", sys->time_of_max,      fread_string_nohash( fp ) );
	    break;

	case 'N':
            KEY( "Nameresolving",  sys->NO_NAME_RESOLVING, fread_number( fp ) );
	    break;

	case 'O':
	    KEY( "Officials", 	   sys->officials,      fread_string( fp ) );
	    break;

	case 'S':
	    KEY( "Saveflags",	   sys->save_flags,	fread_number( fp ) );
	    KEY( "Savefreq",	   sys->save_frequency,	fread_number( fp ) );
	    break;


	case 'W':
	    KEY( "Waitforauth",	   sys->WAIT_FOR_AUTH,	  fread_number( fp ) );
	    break;
	}
	

	if ( !fMatch )
	{
            bug( "Fread_sysdata: no match: %s", word );
	}
    }
}



/*
 * Load the sysdata file
 */
bool load_systemdata( SYSTEM_DATA *sys )
{
    char filename[MAX_INPUT_LENGTH];
    FILE *fp;
    bool found;

    found = FALSE;
    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_sysdata_file: # not found." );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "SYSTEM" ) )
	    {
	    	fread_sysdata( sys, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		bug( "Load_sysdata_file: bad section." );
		break;
	    }
	}
	fclose( fp );
    }

    return found;
}


void load_banlist( void )
{
  BAN_DATA *pban;
  FILE *fp;
  int number;
  char letter;
  
  if ( !(fp = fopen( SYSTEM_DIR BAN_LIST, "r" )) )
    return;
    
  for ( ; ; )
  {
    if ( feof( fp ) )
    {
      bug( "Load_banlist: no -1 found." );
      fclose( fp );
      return;
    }
    number = fread_number( fp );
    if ( number == -1 )
    {
      fclose( fp );
      return;
    }
    CREATE( pban, BAN_DATA, 1 );
    pban->level = number;
    pban->name = fread_string_nohash( fp );
    if ( (letter = fread_letter(fp)) == '~' )
      pban->ban_time = fread_string_nohash( fp );
    else
    {
      ungetc(letter, fp);
      pban->ban_time = str_dup( "(unrecorded)" );
    }
    LINK( pban, first_ban, last_ban, next, prev );
  }
}

void replace_char( char *buf, char replace, char with )
{
  size_t i = 0;

  for( i = 0; i < strlen( buf ); ++i )
    {
      if( buf[i] == replace )
	{
	  buf[i] = with;
	}
    }
}
