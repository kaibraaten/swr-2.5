#include "mud.h"
#include <string.h>

extern int top_r_vnum;

void room_set_sector( ROOM_INDEX_DATA *room, int sector_type )
{
  room->sector_type = sector_type;

  if( room->area->planet )
    {
      switch( sector_type )
        {
        case SECT_INSIDE:
          room->area->planet->citysize++;
          break;

        case SECT_CITY:
          room->area->planet->citysize++;
          break;

        case SECT_FARMLAND:
          room->area->planet->farmland++;
          break;

        case SECT_UNDERGROUND:
          room->area->planet->wilderness++;
          break;

        default:
          break;
        }
    }
}

void make_colony_supply_shop( PLANET_DATA *planet, ROOM_INDEX_DATA *location )
{
  static const char *room_descr =
    "This visible part of this shop consists of a long desk with a couple\r\n"
    "of computer terminals located along its length. A large set of sliding\r\n"
    "doors conceals the supply room behind. In front of the main desk a\r\n"
    "smaller circular desk houses several mail terminals as the shop also\r\n"
    "doubles as a post office. This shop stocks an assortment of basic\r\n"
    "supplies useful to both travellers and settlers. The shopkeeper will\r\n"
    "also purchase some items that might have some later resale or trade\r\n"
    "value.\r\n";

  location->name = STRALLOC( "Supply Shop" );
  location->description = STRALLOC( room_descr );
  room_set_sector( location, SECT_INSIDE );
  SET_BIT( location->room_flags, ROOM_INDOORS );
  SET_BIT( location->room_flags, ROOM_NO_MOB );
  SET_BIT( location->room_flags, ROOM_SAFE );
  SET_BIT( location->room_flags, ROOM_NOPEDIT );
  SET_BIT( location->room_flags, ROOM_MAIL );
  SET_BIT( location->room_flags, ROOM_TRADE );
  SET_BIT( location->room_flags, ROOM_BANK );
}

void make_colony_center( PLANET_DATA *planet, ROOM_INDEX_DATA *location )
{
  char buf[MAX_STRING_LENGTH];
  static const char *room_descr = 
    "You stand in the main foyer of the colonization center. This is one of\r\n"
    "many similar buildings scattered on planets throughout the galaxy. It and\r\n"
    "the others like it serve two main purposes. The first is as an initial\r\n"
    "living and working space for early settlers to the planet. It provides\r\n"
    "a center of operations during the early stages of a colony while the\r\n"
    "surrounding area is being developed. Its second purpose after the\r\n"
    "initial community has been settled is to provide an information center\r\n"
    "for new citizens and for tourists. Food, transportation, shelter, \r\n"
    "supplies, and information are all contained in one area making it easy\r\n"
    "for those unfamiliar with the planet to adjust. This also makes it a\r\n"
    "very crowded place at times.\r\n";

  snprintf( buf, MAX_STRING_LENGTH, "%s: Colonization Center", planet->name );
  location->name = STRALLOC( buf );
  location->description = STRALLOC( room_descr );
  room_set_sector( location, SECT_INSIDE );
  SET_BIT( location->room_flags, ROOM_INDOORS );
  SET_BIT( location->room_flags, ROOM_NO_MOB );
  SET_BIT( location->room_flags, ROOM_SAFE );
  SET_BIT( location->room_flags, ROOM_NOPEDIT );
  SET_BIT( location->room_flags, ROOM_INFO );
}

void make_colony_shuttle_platform( PLANET_DATA *planet,
				   ROOM_INDEX_DATA *location )
{
  static const char *room_descr =
    "This platform is large enough for several spacecraft to land and take off\r\n"
    "from. Its surface is a hard glossy substance that is mostly smooth except\r\n"
    "a few ripples and impressions that suggest its liquid origin. Power boxes\r\n"
    "are scattered about the platform strung together by long strands of thick\r\n"
    "power cables and fuel hoses. Glowing strips divide the platform into\r\n"
    "multiple landing areas. Hard rubber pathways mark pathways for pilots and\r\n"
    "passengers, leading from the various landing areas to the Colonization\r\n"
    "Center.\r\n";
  location->description = STRALLOC( room_descr );
  location->name = STRALLOC( "Community Shuttle Platform" );
  room_set_sector( location, SECT_CITY );
  SET_BIT( location->room_flags, ROOM_SHIPYARD );
  SET_BIT( location->room_flags, ROOM_CAN_LAND );
  SET_BIT( location->room_flags, ROOM_NO_MOB );
  SET_BIT( location->room_flags, ROOM_NOPEDIT );
}

void make_colony_hotel( PLANET_DATA *planet, ROOM_INDEX_DATA *location )
{
  char buf[MAX_STRING_LENGTH];
  static const char *room_descr =
    "This part of the center serves as a temporary home for new settlers\r\n"
    "until a more permanent residence is found. It is also used as a hotel\r\n"
    "for tourists and visitors. the shape of the hotel is circular with rooms\r\n"
    "located around the perimeter extending several floors above ground level.\r\n"
    "\r\nThis is a good place to rest. You may safely leave and reenter the\r\n"
    "game from here.\r\n";

  location->description = STRALLOC( room_descr );
  snprintf( buf, MAX_STRING_LENGTH, "%s: Center Hotel", planet->name );
  location->name = STRALLOC( buf );
  room_set_sector( location, SECT_INSIDE );
  SET_BIT( location->room_flags, ROOM_INDOORS );
  SET_BIT( location->room_flags, ROOM_HOTEL );
  SET_BIT( location->room_flags, ROOM_SAFE );
  SET_BIT( location->room_flags, ROOM_NO_MOB );
  SET_BIT( location->room_flags, ROOM_NOPEDIT );
}

void make_colony_wilderness( PLANET_DATA *planet,
			     ROOM_INDEX_DATA *location,
			     const char *description )
{
  location->description = STRALLOC( description );
  location->name = STRALLOC( planet->name );
  room_set_sector( location, planet->sector );
}

void make_colony_room_exits( ROOM_INDEX_DATA *location, int room_type )
{
  if( room_type > 5
      && room_type != 17
      && room_type != COLONY_ROOM_SUPPLY_SHOP
      && room_type != COLONY_ROOM_SHUTTLE_PLATFORM
      && room_type != 19
      && room_type != 23 )
    {
      ROOM_INDEX_DATA *troom = get_room_index( top_r_vnum - 5 );
      EXIT_DATA *xit = make_exit( location, troom, DIR_NORTH );
      xit->keyword = STRALLOC( "" );
      xit->description = STRALLOC( "" );
      xit->key = -1;
      xit->exit_info = 0;

      xit = make_exit( troom, location, DIR_SOUTH );
      xit->keyword = STRALLOC( "" );
      xit->description = STRALLOC( "" );
      xit->key = -1;
      xit->exit_info = 0;
    }

  if( room_type != COLONY_ROOM_FIRST
      && room_type != 6
      && room_type != 11
      && room_type != COLONY_ROOM_SUPPLY_SHOP
      && room_type != 15
      && room_type != 16
      && room_type != COLONY_ROOM_HOTEL
      && room_type != 19
      && room_type != 21 )
    {
      ROOM_INDEX_DATA *troom = get_room_index( top_r_vnum - 1 );
      EXIT_DATA *xit = make_exit( location, troom, DIR_WEST );
      xit->keyword = STRALLOC( "" );
      xit->description = STRALLOC( "" );
      xit->key = -1;
      xit->exit_info = 0;
      xit = make_exit( troom, location, DIR_EAST );
      xit->keyword = STRALLOC( "" );
      xit->description = STRALLOC( "" );
      xit->key = -1;
      xit->exit_info = 0;
    }
}

const char * const room_type_name[][2] = {
  /*  0: doesn't exist */
  { "zero", "zero" },

  /*  1: COLONY_ROOM_WILDERNESS */
  { "wilderness", "the planet's default terrain" },

  /*  2: COLONY_ROOM_FARMLAND */
  { "farmland", "cleared farmland" },

  /*  3: COLONY_ROOM_CITY_STREET */
  { "city", "a city street" },

  /*  4: COLONY_ROOM_SHIPYARD */
  { "shipyard", "ships are built here" },

  /*  5: COLONY_ROOM_INSIDE */
  { "inside", "inside a building" },

  /*  6: COLONY_ROOM_HOUSE */
  { "house", "may be used as a player's home" },

  /*  7: COLONY_ROOM_CAVE */
  { "cave", "a mine or dug out tunnel" },

  /*  8: COLONY_ROOM_INFO */
  { "info", "message and information room" },

  /*  9: COLONY_ROOM_MAIL */
  { "mail", "post office" },

  /* 10: COLONY_ROOM_TRADE */
  { "trade", "players can sell resources here" },

  /* 11: COLONY_ROOM_PAWN */
  { "pawn", "will trade useful items" },

  /* 12: COLONY_ROOM_SUPPLY_SHOP */
  { "supply", "a supply store" },

  /* 13: COLONY_ROOM_COLONIZATION_CENTER */
  { "center", "the colonization center" },

  /* 14: COLONY_ROOM_SHUTTLE_PLATFORM */
  { "platform", "ships land here" },

  /* 15: COLONY_ROOM_RESTAURANT */
  { "restaurant", "food is bought here" },

  /* 16: COLONY_ROOM_BAR */
  { "bar", "liquor is sold here" },

  /* 17: COLONY_ROOM_CONTROL */
  { "control", "control tower for patrol ships" },

  /* 18: COLONY_ROOM_HOTEL */
  { "hotel", "players can enter/leave game here" },

  /* 19: COLONY_ROOM_BARRACKS */
  { "barracks", "houses military patrols" },

  /* 20: COLONY_ROOM_GARAGE */
  { "garage", "vehicles are built here" },

  /* 21: COLONY_ROOM_BANK */
  { "bank", "room is a bank" },

  /* 22: COLONY_ROOM_EMPLOYMENT */
  { "employment", "room is an employment office" },

  /* 23: COLONY_ROOM_UNUSED23*/
  { "_unused23", "_unused23" },

  /* 24: COLONY_ROOM_UNUSED24 */
  { "_unused24", "_unused24" },

  /* 25: COLONY_ROOM_LAST */
  { "last", "last" }
};

size_t room_type_name_size( void )
{
  return sizeof( room_type_name ) / sizeof( *room_type_name );
}

int get_room_type( const char *name )
{
  int x = 0;

  for( x = 0; x < room_type_name_size(); ++x )
    if( !str_cmp( name, room_type_name[x][0] ) )
      return x;

  return -1;
}

const char *get_room_type_name( int room_type )
{
  if( room_type < COLONY_ROOM_FIRST || room_type > COLONY_ROOM_LAST )
    return "*out of bounds*";

  return room_type_name[room_type][0];
}

const char *get_room_type_description( int room_type )
{
  if( room_type < COLONY_ROOM_FIRST || room_type > COLONY_ROOM_LAST )
    return "*out of bounds*";

  return room_type_name[room_type][1];
}
