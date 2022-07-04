/************************************************************
 *
 * Copyright (c) 2022 Lian Ferrand
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of I Wanna Slay the Dragon of Bangan
 *
 ************************************************************/

#ifndef _GAME_H
#define _GAME_H

#include "loveylib/types.h"
#include "loveylib/vector.h"
#include "loveylib/buffer.h"
#include "loveylib/timer.h"
#include "loveylib/random.h"
#include "loveylib/endian.h"
#include "vertex.h"

static constexpr const u32 GAME_WIDTH = 800;
static constexpr const u32 GAME_HEIGHT = 608;
static constexpr const u32 GAME_FPS = 50;

// Image ID
enum image_id_e : ufast {
  IMG_PSTAND0=0, IMG_PSTAND1, IMG_PSTAND2, IMG_PSTAND3,
  IMG_PWALK0, IMG_PWALK1, IMG_PWALK2, IMG_PWALK3,
  IMG_PJUMP0, IMG_PJUMP1, IMG_PFALL0, IMG_PFALL1,
  IMG_PVINE0, IMG_PVINE1,

  IMG_BULLET0, IMG_BULLET1,

  IMG_SAVE, IMG_SAVEHIT,

  IMG_WARP,

  IMG_GAMEOVER,

  IMG_INTRO0, IMG_INTRO1, IMG_INTRO2,
  IMG_INTRO3, IMG_INTRO4, IMG_INTRO5,

  IMG_JUMPSPELL,
  IMG_SHOOTSPELL,
  IMG_SPEEDSPELL,
  IMG_FINALSPELL,

  IMG_SBULLET0, IMG_SBULLET1,
  IMG_SBKILLER,

  IMG_DRAGON, IMG_WHITEDRAGON,
  IMG_WHITEDRAGON1, IMG_WHITEDRAGON2,

  IMG_THUNDER0, IMG_THUNDER1,

  IMG_COUNT,

  IMG_NONE = 0xfe
};
typedef ufast image_id_t;

// Sprite ID
enum sprite_id_e : ufast {
  SPR_PSTAND = 0,
  SPR_PWALK,
  SPR_PJUMP,
  SPR_PFALL,
  SPR_PVINE,

  SPR_BULLET,
  SPR_SBULLET,

  SPR_THUNDER,

  SPR_COUNT
};
typedef ufast sprite_id_t;

// Sprite information
struct sprite_t {
  // Sprite ID
  sprite_id_t id;

  // Start and end of sprite, inclusive
  image_id_t start, end;

  // Current image
  image_id_t img;

  // Current image frame
  ufast frame;

  // Frames per image
  ufast fpi;
};

// Game input
#define T_BIT(_input) _input ## BIT = 1<<_input
enum input_field_e : ufast {
  INPUT_LEFT = 0,
  INPUT_RIGHT,
  INPUT_UP,
  INPUT_DOWN,
  INPUT_JUMP,
  INPUT_SHOOT,
  INPUT_RESTART,
  INPUT_NEWGAME,

  T_BIT(INPUT_LEFT),
  T_BIT(INPUT_RIGHT),
  T_BIT(INPUT_UP),
  T_BIT(INPUT_DOWN),
  T_BIT(INPUT_JUMP),
  T_BIT(INPUT_SHOOT),
  T_BIT(INPUT_RESTART),
  T_BIT(INPUT_NEWGAME),
};
#undef T_BIT

typedef ufast input_field_t;

struct input_t {
  // pressed: Buttons pressed this frame
  // released: Buttons released this frame
  // down: Buttons currently down
  // nextDown: down on the next frame, excluding
  //           pressed and released buttons on the
  //           next frame
  input_field_t pressed, released, down, nextDown;

  inline void updateDown() {
    // Prioritize pressed buttons, but make sure
    // they're not held down
    down = (nextDown & ~released) | pressed;
    nextDown = down & ~released;
  }
};

// Bounding box
// x = left, y = bottom, z = right, w = top
typedef ivec4 bbox_t;

// Entity ID
enum entity_id_e : u8 {
  ENT_KID = 0,
  ENT_BULLET,
  ENT_SAVE,
  ENT_WARP,
  ENT_GAMEOVER,
  ENT_BLOODEMITTER,
  ENT_INTRO,
  ENT_SPELL,
  ENT_SBULLET,
  ENT_SBKILLER,
  ENT_DRAGON,
  ENT_DRAGONDEFEAT,
  ENT_IDLEKID,
  ENT_THUNDER,
  ENT_DRAGONPART,

  ENT_COUNT
};
typedef u8 entity_id_t;

// Entity forward declaration
struct entity_t;

// Entity initialization
struct entity_init_t {
  // 16 dwords
  union {
    u32 dword[16];
    f32 flt[16];
    vec4 v4[4];
    ivec4 iv4[4];
  };

  // 63-byte string
  char str[63];

  // Entity ID
  entity_id_t ent;

  // Swap initialization data
  void swap() {
    for (uptr i = 0; i < ArraySize(dword); ++i)
      dword[i] = LittleEndian32(dword[i]);
  }
};

typedef void (*entity_init_func_t)(entity_t */*me*/, entity_init_t */*initData*/);

// Entity vtable
typedef void (*entity_update_func_t)(entity_t */*me*/, const input_t */*i*/);
typedef void (*entity_destroy_func_t)(entity_t */*me*/);

struct entity_funcs_t {
  entity_update_func_t update;
  entity_destroy_func_t destroy;
};

// Entity type information
struct entity_info_t {
  entity_id_t id;
  bbox_t bbox; // <0 0> == pos
  entity_funcs_t funcs;
};

// Entity
struct entity_base_t {
  entity_t *prev, *next;

  // w = 0
  vec4 pos;

  // zw = 1 1
  vec4 scale;

  const entity_info_t *info;

  sprite_t spr;
};

static constexpr const iptr ENTITY_DATA_SIZE = 100-sizeof(entity_base_t);
static_assert(ENTITY_DATA_SIZE > 0, "");

struct entity_t {
  entity_base_t b;

  u8 data[ENTITY_DATA_SIZE];
};

// Convenience entity vtable wrappers
static inline void UpdateEntity(entity_t *me, const input_t *i) {
  me->b.info->funcs.update(me, i);
}
static inline void DestroyEntity(entity_t *me) {
  me->b.info->funcs.destroy(me);
}

// Tile ID
enum tile_id_e : u8 {
  TILE_NONE = 0,

  TILE_BLOCK,
  TILE_KILLER,
  TILE_PLATFORM,
  TILE_PROP,
};
typedef u8 tile_id_t;

// Tile mask
enum tile_mask_e : u8 {
  TILE_MASK_NONE,

  TILE_MASK_FULL,

  TILE_MASK_DSPIKE,
  TILE_MASK_USPIKE,
  TILE_MASK_LSPIKE,
  TILE_MASK_RSPIKE,

  TILE_MASK_PLATFORM,
};
typedef u8 tile_mask_t;

// Tile modifier bitfield
enum tile_bit_e : u16 {
  TILE_LVINEBIT = 0x8000,
  TILE_RVINEBIT = 0x4000,
};
typedef u16 tile_bit_t;

static constexpr const u16 TILE_IDSHIFT = 0;
static constexpr const u16 TILE_MASKSHIFT = 8;
static constexpr const u16 TILE_IDMASK = 0xff;
static constexpr const u16 TILE_MASKMASK = 0x3f00;
typedef u16 tile_t;

static constexpr const i32 TILE_SIZE = 32;

// Tile map
static constexpr const uptr TILE_MAP_WIDTH = GAME_WIDTH/TILE_SIZE;
static constexpr const uptr TILE_MAP_HEIGHT = GAME_HEIGHT/TILE_SIZE;
typedef tile_t tile_map_t[TILE_MAP_WIDTH*TILE_MAP_HEIGHT];

// Editor tiles
enum editor_tile_e : ufast {
  ETILE_NONE,
  ETILE_BLOCK1,
  ETILE_BLOCK2,
  ETILE_BLOCK3,
  ETILE_BLOCK4,
  ETILE_BLOCK5,
  ETILE_BLOCK6,
  ETILE_INVIS,
  ETILE_SPIKEDOWN,
  ETILE_SPIKEUP,
  ETILE_SPIKELEFT,
  ETILE_SPIKERIGHT,
  ETILE_LVINE1,
  ETILE_RVINE1,
  ETILE_LVINE2,
  ETILE_RVINE2,
  ETILE_PLATFORM,
  ETILE_BLACK,
  ETILE_FADE,
  ETILE_ENTRANCE,
  ETILE_FADE2,
  ETILE_THANKS,

  ETILE_COUNT
};
typedef ufast editor_tile_t;

// Room, externally loaded
#define ROOM_SIZE(_ent, _quads) (sizeof(room_t)+sizeof(entity_init_t)*(_ent)+sizeof(rquad_t)*(_quads))
struct alignas(64) room_t {
  // BGM filename
  char bgm[63];

  // Image page number
  u8 page;

  // Number of entities
  u32 entityCount;

  // Number of quads
  u32 quadCount;

  // Tile map
  tile_map_t map;

  // 64-byte alignment padding
  u8 pad[2];

  // Entity initialization data buffer
  inline entity_init_t *entities() {return (entity_init_t*)(this+1);}

  // Quad buffer
  inline rquad_t *quads() {return (rquad_t*)(entities()+entityCount);}

  // Swap endianness
  void swap() {
    entityCount = LittleEndian32(entityCount);
    quadCount = LittleEndian32(quadCount);

    for (uptr i = 0; i < ArraySize(map); ++i)
      map[i] = LittleEndian16(map[i]);

    for (uptr i = 0; i < entityCount; ++i)
      entities()[i].swap();

    for (uptr i = 0; i < quadCount; ++i) {
      for (uptr j = 0; j < 4; ++j) {
        quads()[i].v[j].coord.pad[0] = LittleEndian32(quads()[i].v[j].coord.pad[0]);
        quads()[i].v[j].coord.pad[1] = LittleEndian32(quads()[i].v[j].coord.pad[1]);
        quads()[i].v[j].coord.pad[2] = LittleEndian32(quads()[i].v[j].coord.pad[2]);
        quads()[i].v[j].coord.x = LittleEndian16(quads()[i].v[j].coord.x);
        quads()[i].v[j].coord.y = LittleEndian16(quads()[i].v[j].coord.y);
      }
    }
  }
};

// Room container
template<uptr N, uptr N2>
struct room_container_t {
  room_t b;

  entity_init_t entities[N];
  rquad_t quads[N2];

  inline void init() {
    b.entityCount = N;
    b.quadCount = N2;
  }
};

// Save data
static constexpr const u32 SAVE_MAGIC = CBIG_ENDIAN32(0x0550438e);
struct game_save_t {
  entity_init_t kidInit; // Separate kid initialization

  char roomName[44];

  u32 magic; // == SAVE_MAGIC

  inline void swap() {
    kidInit.swap();
  }

  constexpr bfast valid() const {return magic == SAVE_MAGIC;}

  inline void validate() {magic = SAVE_MAGIC;}
};

// Game mode
enum game_mode_e : ufast {
  GAME_PLAY,
  GAME_TITLE,
  GAME_EDITOR,
};
typedef ufast game_mode_t;

enum spell_e : ufast {
  SPELL_NONE = 0,
  SPELL_JUMP,
  SPELL_SHOOT,
  SPELL_SPEED,
  SPELL_FINAL,

  SPELL_COUNT
};
typedef ufast spell_t;

// Global game state
// Valid while game is active
static constexpr const uptr MAX_ENTITIES = 256;
struct game_state_t {
  // Entity buffer
  buffer_container_t<entity_t, MAX_ENTITIES> entityBuf;

  // First and last entity in linked list
  entity_t *firstEntity, *lastEntity;

  // Current room pointer, with filename
  char roomName[48];
  room_t *room;

  // RNG seed
  rng_seed_t seed;

  // Bullet count
  u32 bulletCount;

  // Current save, will be written on exit
  game_save_t save;

  // Current game state
  game_mode_t state;

  // Reset game tick
  bfast resetTick;

  // Currently active spell
  spell_t curSpell;

  // Editor data
  uptr entCount;
  entity_init_t ents[MAX_ENTITIES];

  editor_tile_t map[TILE_MAP_WIDTH*TILE_MAP_HEIGHT];

  // Cursor position
  uptr cur;

  // Current tile
  editor_tile_t curTile;

  // 0: tile mode
  // 1: entity mode
  u32 mode;
};
extern game_state_t *g_state;

void InitGame();
void FreeGame();

void UpdateGame(input_t *input);

#endif //_GAME_H
