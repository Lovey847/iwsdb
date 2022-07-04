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

#ifdef NDEBUG
#define USE_EDITOR 0
#define USE_SAVE 1
#else
#define USE_EDITOR 0
#define USE_SAVE (!USE_EDITOR)
#endif

#include "game.h"
#include "mem.h"
#include "draw.h"
#include "audio.h"
#include "log.h"
#include "str.h"
#include "loveylib/file.h"

#include <cstring>
#include <cmath>

#undef M_PI
#define M_PI 3.14159265358979323846f

static const sprite_t S_Sprites[SPR_COUNT] = {
  {SPR_PSTAND, IMG_PSTAND0, IMG_PSTAND3, IMG_PSTAND0, 0, 5},
  {SPR_PWALK, IMG_PWALK0, IMG_PWALK3, IMG_PWALK0, 0, 2},
  {SPR_PJUMP, IMG_PJUMP0, IMG_PJUMP1, IMG_PJUMP0, 0, 2},
  {SPR_PFALL, IMG_PFALL0, IMG_PFALL1, IMG_PFALL0, 0, 2},
  {SPR_PVINE, IMG_PVINE0, IMG_PVINE1, IMG_PVINE0, 0, 2},
  {SPR_BULLET, IMG_BULLET0, IMG_BULLET1, IMG_BULLET0, 0, 1},
  {SPR_SBULLET, IMG_SBULLET0, IMG_SBULLET1, IMG_SBULLET0, 0, 1},
  {SPR_THUNDER, IMG_THUNDER0, IMG_THUNDER1, IMG_THUNDER0, 0, 2},
};

// Blood texture coordinates
static constexpr const u16 BLOOD_X = 239;
static constexpr const u16 BLOOD_Y = 107;
static const rquad_t S_BloodQuad = {{
    {{-2.f/GAME_WIDTH, -2.f/GAME_HEIGHT, 0.f, BLOOD_X, BLOOD_Y}},
    {{2.f/GAME_WIDTH, 0.f, 0.f, BLOOD_X, BLOOD_Y}},
    {{-2.f/GAME_WIDTH, 0.f, 0.f, BLOOD_X, BLOOD_Y}},
    {{2.f/GAME_WIDTH, 2.f/GAME_HEIGHT, 0.f, BLOOD_X, BLOOD_Y}}
  }};

#ifdef NDEBUG
static constexpr const bfast DEBUG_KEYS = false;
#else
static constexpr const bfast DEBUG_KEYS = true;
#endif

static const char * const INITIAL_ROOM = "data/room/intro0.rm";
static const char * const EDITOR_LEVEL = "data/room/13.rm";
static const char * const EDITOR_DESTINATION = "data/room/.rm";
static constexpr const bfast OPEN_EDITOR_LEVEL = true;
static const char * const EDITOR_BGM = "data/bgm/world1.wav";
static constexpr const u8 EDITOR_PAGE = 0;
static const tile_t S_TileCode[ETILE_COUNT] = {
  TILE_NONE,
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT),
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT),
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT),
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT),
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT),
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT),
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT),
  (TILE_KILLER<<TILE_IDSHIFT)|(TILE_MASK_DSPIKE<<TILE_MASKSHIFT),
  (TILE_KILLER<<TILE_IDSHIFT)|(TILE_MASK_USPIKE<<TILE_MASKSHIFT),
  (TILE_KILLER<<TILE_IDSHIFT)|(TILE_MASK_LSPIKE<<TILE_MASKSHIFT),
  (TILE_KILLER<<TILE_IDSHIFT)|(TILE_MASK_RSPIKE<<TILE_MASKSHIFT),
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT)|TILE_LVINEBIT,
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT)|TILE_RVINEBIT,
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT)|TILE_LVINEBIT,
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT)|TILE_RVINEBIT,
  (TILE_PLATFORM<<TILE_IDSHIFT)|(TILE_MASK_PLATFORM<<TILE_MASKSHIFT),
  TILE_PROP<<TILE_IDSHIFT,
  TILE_PROP<<TILE_IDSHIFT,
  (TILE_BLOCK<<TILE_IDSHIFT)|(TILE_MASK_FULL<<TILE_MASKSHIFT),
  TILE_PROP,
  TILE_PROP,
};

#define T_IMG(left, top, right, bottom, leftCoord, topCoord, rightCoord, bottomCoord) \
  {{                                                                    \
      {{(left)*2.f/GAME_WIDTH, (top)*2.f/GAME_HEIGHT, 0.f, (leftCoord), (topCoord)}}, \
      {{(right)*2.f/GAME_WIDTH, (top)*2.f/GAME_HEIGHT, 0.f, (rightCoord), (topCoord)}}, \
      {{(left)*2.f/GAME_WIDTH, (bottom)*2.f/GAME_HEIGHT, 0.f, (leftCoord), (bottomCoord)}}, \
      {{(right)*2.f/GAME_WIDTH, (bottom)*2.f/GAME_HEIGHT, 0.f, (rightCoord), (bottomCoord)}} \
    }}
#define T_IMGROT(left, top, right, bottom, leftCoord, topCoord, rightCoord, bottomCoord) \
  {{                                                                    \
      {{(left)*2.f/GAME_WIDTH, (top)*2.f/GAME_HEIGHT, 0.f, (leftCoord), (bottomCoord)}}, \
      {{(right)*2.f/GAME_WIDTH, (top)*2.f/GAME_HEIGHT, 0.f, (leftCoord), (topCoord)}}, \
      {{(left)*2.f/GAME_WIDTH, (bottom)*2.f/GAME_HEIGHT, 0.f, (rightCoord), (bottomCoord)}}, \
      {{(right)*2.f/GAME_WIDTH, (bottom)*2.f/GAME_HEIGHT, 0.f, (rightCoord), (topCoord)}} \
    }}
static const rquad_t S_TileQuad[ETILE_COUNT] = {
  {},

  // ETILE_BLOCK1
  T_IMG(0, 0, 32, -32, 0, 0, 32, 32),
  // ETILE_BLOCK2
  T_IMG(0, 0, 32, -32, 32, 0, 64, 32),
  // ETILE_BLOCK3
  T_IMG(0, 0, 32, -32, 64, 0, 96, 32),
  // ETILE_BLOCK4
  T_IMG(0, 0, 32, -32, 96, 0, 128, 32),
  // ETILE_BLOCK5
  T_IMG(0, 0, 32, -32, 0, 32, 32, 64),
  // ETILE_BLOCK6
  T_IMG(0, 0, 32, -32, 32, 32, 64, 64),
  // ETILE_INVIS
  T_IMG(0, 0, 32, -32, 0, 2048-32, 32, 2048),

  // ETILE_SPIKEDOWN
  T_IMG(0, 0, 32, -32, 2, 130, 34, 162),
  // ETILE_SPIKEUP
  T_IMG(0, 0, 32, -32, 2, 162, 34, 130),
  // ETILE_SPIKELEFT
  T_IMGROT(0, 0, 32, -32, 2, 130, 34, 162),
  // ETILE_SPIKERIGHT
  T_IMGROT(0, 0, 32, -32, 2, 162, 34, 130),

  // ETILE_LVINE1
  T_IMG(0, 0, 32, -32, 0, 164, 32, 196),
  // ETILE_RVINE1
  T_IMG(0, 0, 32, -32, 0, 292, 32, 324),
  // ETILE_LVINE2
  T_IMG(0, 0, 32, -32, 32, 164, 64, 196),
  // ETILE_RVINE2
  T_IMG(0, 0, 32, -32, 32, 292, 64, 324),

  // ETILE_PLATFORM
  T_IMG(0, 0, 32, -16, 142, 130, 174, 146),

  // ETILE_BLACK (400x608 black rectangle)
  T_IMG(0, 0, 400, -608, 486, 194, 486, 194),

  // ETILE_FADE (World 1 to 2 gradient)
  T_IMG(0, 0, 32, -128, 161, 390, 161, 1190),

  // ETILE_ENTRANCE
  T_IMG(0, 0, 128, -128, 1062, 0, 1190, 128),

  // ETILE_FADE2 (World 3 to outro gradient)
  T_IMGROT(0, 0, 192, -192, 163, 1190, 163, 390),

  // ETILE_THANKS
  T_IMG(0, 0, 778, -564, 260, 610, 260+778, 610+564),
};

// For completly stationary entities
static const rquad_t S_EntityQuad[ENT_COUNT] = {
  // ENT_KID, ENT_BULLET, ENT_SAVE
  {}, {}, {},

  // ENT_WARP
  T_IMG(2.f, -2.f, 28.f, -29.f, 114, 130, 140, 157),
};
#undef T_IMG
#undef T_IMGROT

// Set sprite, normally NOP if spr == out->id
// You can override this behavior with the EXPLICIT template parameter
static constexpr const u32 EXPLICIT = 0x4456;
template<u32 explicitSet = 0>
static void SetSprite(sprite_t *out, sprite_id_t spr) {
  if ((explicitSet != EXPLICIT) && (out->id == spr)) return;

  *out = S_Sprites[spr];
}

// Set sprite to single image
static void SetImageSprite(sprite_t *out, image_id_t img) {
  out->id = SPR_COUNT; // This isn't a sprite
  out->start = out->end = out->img = img;
  out->frame = 0;
  out->fpi = 0xff;
}

// Set sprite to nothing
static inline void SetNullSprite(sprite_t *out) {
  SetImageSprite(out, IMG_NONE);
}

// Update sprite image, used after drawing
static void UpdateSprite(sprite_t *s) {
  if (++s->frame >= s->fpi) {
    s->frame = 0;

    if (++s->img > s->end)
      s->img = s->start;
  }
}

// Check collision with tile mask
static bfast TileMaskCol(const ivec4 bbox, i32 x, i32 y, tile_mask_t m) {
  ivec4 tileBbox;
  tileBbox.v[0] = x;
  tileBbox.v[1] = y;

  tileBbox = ShuffleVec4<0x0101>(tileBbox)*IVEC4_1(TILE_SIZE) + IVEC4(0, 0, TILE_SIZE, TILE_SIZE);

  // bbox must be within tile's range
  ASSERT((bbox.v[0] < tileBbox.v[2]) && (bbox.v[2] > tileBbox.v[0]) &&
         (bbox.v[1] < tileBbox.v[3]) && (bbox.v[3] > tileBbox.v[1]));

  // Check collision mask
  switch (m) {
  case TILE_MASK_NONE: return false;
  case TILE_MASK_FULL: return true;

  case TILE_MASK_USPIKE: {
    ivec4 rect = (bbox-ShuffleVec4<0x0101>(tileBbox))*IVEC4(1, -1, 1, -1)+IVEC4(0, 32, 0, 32);
    i32 bottom = rect.v[1];
    rect.v[1] = rect.v[3];
    rect.v[3] = bottom;

    if (rect.v[2] < 16) {
      bottom = (16 - rect.v[2])*2;
    } else {
      // If rect.v[2] is to the right of the spike,
      // and rect.v[0] is to the left, the crossover must collide
      if (rect.v[0] <= 16) return true;

      bottom = (rect.v[0] - 16)*2;
    }

    return rect.v[3] > bottom;
  }
  case TILE_MASK_DSPIKE: {
    ivec4 rect = (bbox-ShuffleVec4<0x0101>(tileBbox))*IVEC4(1, -1, 1, -1)+IVEC4(0, 32, 0, 32);
    i32 top = rect.v[1];
    rect.v[1] = rect.v[3];
    rect.v[3] = top;

    if (rect.v[2] < 16) {
      top = rect.v[2]*2;
    } else {
      if (rect.v[0] <= 16) return true;

      top = (32 - rect.v[0])*2;
    }

    return rect.v[1] < top;
  }
  case TILE_MASK_LSPIKE: {
    ivec4 rect = (bbox-ShuffleVec4<0x0101>(tileBbox))*IVEC4(1, -1, 1, -1)+IVEC4(0, 32, 0, 32);
    i32 right = rect.v[1];
    rect.v[1] = rect.v[3];
    rect.v[3] = right;

    if (rect.v[3] < 16) {
      right = (16 - rect.v[3])*2;
    } else {
      if (rect.v[1] <= 16) return true;

      right = (rect.v[1] - 16)*2;
    }

    return rect.v[2] > right;
  }
  case TILE_MASK_RSPIKE: {
    ivec4 rect = (bbox-ShuffleVec4<0x0101>(tileBbox))*IVEC4(1, -1, 1, -1)+IVEC4(0, 32, 0, 32);
    i32 left = rect.v[1];
    rect.v[1] = rect.v[3];
    rect.v[3] = left;

    if (rect.v[3] < 16) {
      left = rect.v[3]*2;
    } else {
      if (rect.v[1] <= 16) return true;

      left = (32 - rect.v[1])*2;
    }

    return rect.v[0] < left;
  }

  case TILE_MASK_PLATFORM: return bbox.v[3]-tileBbox.v[3] > -16;
  }

  return false;
}

// Check for collision with tile type in tile map
static tile_t *TileCol(ivec4 inBbox, tile_map_t map, tile_id_t id, tile_bit_t bit = 0xffff) {
  id <<= TILE_IDSHIFT;

  ivec4 bbox = inBbox;

  // Convert bbox to tile map units
  bbox -= IVEC4(0, 0, 1, 1);
  bbox /= IVEC4_1(TILE_SIZE);

  // Bounds check
  if (((uptr)bbox.v[0] >= TILE_MAP_WIDTH) ||
      ((uptr)bbox.v[1] >= TILE_MAP_HEIGHT) ||
      (bbox.v[2] < 0) ||
      (bbox.v[3] < 0)) return NULL;

  // Clip bounds
  if (bbox.v[0] < 0) bbox.v[0] = 0;
  else if ((uptr)bbox.v[2] >= TILE_MAP_WIDTH) bbox.v[2] = TILE_MAP_WIDTH-1;
  if (bbox.v[1] < 0) bbox.v[1] = 0;
  else if ((uptr)bbox.v[3] >= TILE_MAP_WIDTH) bbox.v[3] = TILE_MAP_WIDTH-1;

  // Check each tile bbox resides in
  // Top to bottom
  for (uptr y = bbox.v[1]; y <= (uptr)bbox.v[3]; ++y) {
    for (uptr x = bbox.v[0]; x <= (uptr)bbox.v[2]; ++x) {
      tile_t &t = map[y*TILE_MAP_WIDTH + x];
      if ((t&bit) &&
          ((t&TILE_IDMASK) == id) &&
          TileMaskCol(inBbox, x, y, (t&TILE_MASKMASK)>>TILE_MASKSHIFT))
        return &t;
    }
  }

  return NULL;
}

// Entity initialization table
static void InitKid(entity_t *me, entity_init_t *data);
static void InitBullet(entity_t *me, entity_init_t *data);
static void InitSave(entity_t *me, entity_init_t *data);
static void InitWarp(entity_t *me, entity_init_t *data);
static void InitGameover(entity_t *me, entity_init_t *data);
static void InitBloodEmitter(entity_t *me, entity_init_t *data);
static void InitIntro(entity_t *me, entity_init_t *data);
static void InitSpell(entity_t *me, entity_init_t *data);
static void InitSBullet(entity_t *me, entity_init_t *data);
static void InitSBKiller(entity_t *me, entity_init_t *data);
static void InitDragon(entity_t *me, entity_init_t *data);
static void InitDragonDefeat(entity_t *me, entity_init_t *data);
static void InitIdleKid(entity_t *me, entity_init_t *data);
static void InitThunder(entity_t *me, entity_init_t *data);
static void InitDragonPart(entity_t *me, entity_init_t *data);
static const entity_init_func_t S_EntityInit[ENT_COUNT] = {
  InitKid, InitBullet, InitSave, InitWarp, InitGameover, InitBloodEmitter,
  InitIntro, InitSpell, InitSBullet, InitSBKiller, InitDragon, InitDragonDefeat,
  InitIdleKid, InitThunder, InitDragonPart,
};

// Add entity to list, and initialize entity
static entity_t *AddEntity(entity_init_t *initData) {
  // Allocate entity in buffer
  entity_t *e = GetBufferItem(g_state->entityBuf);
  if (!e) LOG_ERROR("Entity buffer is full!");

  // Put entity at the end of the list
  if (!g_state->firstEntity) {
    g_state->firstEntity = g_state->lastEntity = e;
    e->b.prev = NULL;
    e->b.next = NULL;
  } else {
    e->b.prev = g_state->lastEntity;
    e->b.next = NULL;
    g_state->lastEntity->b.next = e;
    g_state->lastEntity = e;
  }

  // Initialize entity
  S_EntityInit[initData->ent](e, (entity_init_t*)initData);

  // If the entity destroyed itself in initialization, return NULL
  if (!BufferItemExists(g_state->entityBuf, e)) return NULL;

  return e;
}

// Remove entity from list
static void RemoveEntity(entity_t *e) {
  ASSERT(BufferItemExists(g_state->entityBuf, e));

  // Destroy entity
  DestroyEntity(e);

  // Remove entity from list
  if (e->b.prev)
    e->b.prev->b.next = e->b.next;
  else {
    // If there's no previous element, that means
    // this is g_state->firstEntity
    g_state->firstEntity = e->b.next;
  }

  if (e->b.next)
    e->b.next->b.prev = e->b.prev;
  else {
    // If there's no next element, that means
    // this is g_state->lastEntity
    g_state->lastEntity = e->b.prev;
  }

  FreeBufferItem(g_state->entityBuf, e);
}

// Check collision against two entities
static bfast EntityCol(entity_t *a, entity_t *b) {
  if (a == b) return false;

  const ivec4 abox = ShuffleVec4<0x0101>(ToIvec4(a->b.pos+VEC4_1(0.5f))) + a->b.info->bbox;
  const ivec4 bbox = ShuffleVec4<0x0101>(ToIvec4(b->b.pos+VEC4_1(0.5f))) + b->b.info->bbox;

  return ((abox.v[0] < bbox.v[2]) && (abox.v[2] > bbox.v[0]) &&
          (abox.v[1] < bbox.v[3]) && (abox.v[3] > bbox.v[1]));
}

// Check collision against entity type
// Returns NULL when no collision takes place
static entity_t *EntityCol(entity_t *me, entity_id_t id) {
  for (entity_t *e = g_state->firstEntity; e; e = e->b.next)
    if ((e->b.info->id == id) && EntityCol(me, e)) return e;

  return NULL;
}

// No update or destruction
static void NoUpdate(entity_t*, const input_t*) {}
static void NoDestroy(entity_t*) {}

// Dragon part
struct dragon_part_t {
  entity_base_t b;

  f32 spd;
};

static void UpdateDragonPart(entity_t *me, const input_t*);
static const entity_info_t S_DragonPartInfo = {
  ENT_DRAGONPART,
  {},
  {UpdateDragonPart, NoDestroy},
};

static void InitDragonPart(entity_t *me, entity_init_t *i) {
  dragon_part_t *d = (dragon_part_t*)me;

  d->b.pos = i->v4[0];
  d->b.scale = VEC4_1(1);
  d->b.info = &S_DragonPartInfo;
  d->spd = i->flt[4];

  SetImageSprite(&d->b.spr, i->dword[5]);
}

static void UpdateDragonPart(entity_t *me, const input_t *) {
  dragon_part_t *d = (dragon_part_t*)me;
  d->b.pos.v[1] += d->spd;
}

// Thunder
static constexpr const ufast THUNDER_LIFETIME = 50;

struct thunder_t {
  entity_base_t b;

  ufast life;
};

static void UpdateThunder(entity_t *me, const input_t*);
static const entity_info_t S_ThunderInfo = {
  ENT_THUNDER,
  {},
  {UpdateThunder, NoDestroy},
};

static void InitThunder(entity_t *me, entity_init_t *data) {
  (void)data;
  thunder_t *t = (thunder_t*)me;

  t->b.pos = VEC4(0, 608.f, -0.99f, 0);
  t->b.scale = VEC4_1(1);
  t->b.info = &S_ThunderInfo;
  t->life = THUNDER_LIFETIME;

  SetSprite(&t->b.spr, SPR_THUNDER);

  PlaySound(SND_THUNDER);
}

static void UpdateThunder(entity_t *me, const input_t*) {
  thunder_t *t = (thunder_t*)me;

  if (!--t->life) RemoveEntity(me);
}

// Idle kid
static const entity_info_t S_IdleKidInfo = {
  ENT_IDLEKID,
  {},
  {NoUpdate, NoDestroy},
};

static void InitIdleKid(entity_t *me, entity_init_t *i) {
  me->b.pos = i->v4[0];
  me->b.scale = VEC4(1, 1, 1, 1);
  me->b.info = &S_IdleKidInfo;

  SetSprite<EXPLICIT>(&me->b.spr, SPR_PSTAND);
}

// Mikoo being defeated
static constexpr const u32 DRAGONDEFEAT_ROOMTIMER = 500;
static constexpr const u32 DRAGONDEFEAT_BGTIMER = 400;
static constexpr const u32 DRAGONDEFEAT_PARTTIMER = 255;
static constexpr const u32 DRAGONDEFEAT_SPEAKTIMER = 11;
static constexpr const f32 DRAGONDEFEAT_CHANGEBGR = -0.00996f;
static constexpr const f32 DRAGONDEFEAT_CHANGEBGG = 0.00439f;
static constexpr const f32 DRAGONDEFEAT_CHANGEBGB = 0.00769f;
struct dragon_defeat_t {
  entity_base_t b;

  vec4 pos;
  f32 offsetMul;
  u32 timer;

  f32 curBgR, curBgG, curBgB;
};
static_assert(sizeof(dragon_defeat_t) <= sizeof(entity_t), "");

static void UpdateDragonDefeat(entity_t *me, const input_t*);

static const entity_info_t S_DragonDefeatInfo = {
  ENT_DRAGONDEFEAT,
  {},
  {UpdateDragonDefeat, NoDestroy},
};

static void InitDragonDefeat(entity_t *me, entity_init_t *i) {
  dragon_defeat_t *d = (dragon_defeat_t*)me;
  d->b.pos = i->v4[0];
  d->b.scale = VEC4(1, 1, 1, 1);
  d->b.info = &S_DragonDefeatInfo;

  d->pos = me->b.pos;
  d->offsetMul = 0.f;
  d->timer = 0;

  d->curBgR = 0.996f;
  d->curBgG = 0.561f;
  d->curBgB = 0.231f;

  SetImageSprite(&me->b.spr, IMG_WHITEDRAGON);

  StopSound(SND_MIKOO);
}

static void LoadRoom(const char *roomName);
static void UpdateDragonDefeat(entity_t *me, const input_t*) {
  dragon_defeat_t *d = (dragon_defeat_t*)me;
  vec4 offset = ZeroVec4();

  ++d->timer;

  if (d->timer == DRAGONDEFEAT_PARTTIMER) {
    entity_init_t partInit;

    partInit.v4[0] = d->b.pos;
    partInit.flt[4] = 16.f;
    partInit.dword[5] = IMG_WHITEDRAGON1;
    partInit.ent = ENT_DRAGONPART;
    AddEntity(&partInit);

    partInit.v4[0].v[1] -= 167.f;
    partInit.flt[4] = -16.f;
    partInit.dword[5] = IMG_WHITEDRAGON2;
    AddEntity(&partInit);

    SetNullSprite(&d->b.spr);
  }

  d->offsetMul += 0.333f;

  offset.v[0] = (f32)((i32)(Random(&g_state->seed)&65535)-32767)*d->offsetMul*(1.f/32768.f);
  offset.v[1] = (f32)((i32)(Random(&g_state->seed)&65535)-32767)*d->offsetMul*(1.f/32768.f);

  d->b.pos = d->pos+offset;

  if (d->timer == DRAGONDEFEAT_SPEAKTIMER) PlaySound(SND_MIKOODEFEATED);
  else if (d->timer == DRAGONDEFEAT_ROOMTIMER) {
    LoadRoom("data/room/clear.rm");
    g_state->resetTick = true;
  } else if (d->timer >= DRAGONDEFEAT_BGTIMER) {
    SetClearColor(d->curBgR += DRAGONDEFEAT_CHANGEBGR,
                  d->curBgG += DRAGONDEFEAT_CHANGEBGG,
                  d->curBgB += DRAGONDEFEAT_CHANGEBGB);
  }
}

// Mikoo
static const entity_info_t S_DragonInfo = {
  ENT_DRAGON,
  {},
  {NoUpdate, NoDestroy},
};

static void InitDragon(entity_t *me, entity_init_t *i) {
  me->b.pos = i->v4[0];
  me->b.scale = VEC4(1, 1, 1, 1);
  me->b.info = &S_DragonInfo;

  SetImageSprite(&me->b.spr, IMG_DRAGON);

  PlaySound(SND_MIKOO);
}

// Bullet spell killer entity
static constexpr const bbox_t SBKILLER_BBOX = {0, -32, 32, 0};

static const entity_info_t S_SBKillerInfo = {
  ENT_SBKILLER,
  SBKILLER_BBOX,
  {NoUpdate, NoDestroy},
};

static void InitSBKiller(entity_t *me, entity_init_t *i) {
  me->b.pos = i->v4[0];
  me->b.scale = VEC4(1, 1, 1, 1);
  me->b.info = &S_SBKillerInfo;

  SetImageSprite(&me->b.spr, IMG_SBKILLER);
}

// Bullet spell entity
static constexpr const f32 BULLET_SPD = 16.f;
static constexpr const bbox_t SBULLET_BBOX = {-4, -4, 4, 4};
static constexpr const u32 BULLET_LIFETIME = 40;

struct sbullet_t {
  entity_base_t b;

  f32 spd;
  u32 life; // Starts at BULLET_LIFETIME
};

static void UpdateSBullet(entity_t *me, const input_t *i);

static const entity_info_t S_SBulletInfo = {
  ENT_SBULLET,
  SBULLET_BBOX,
  {UpdateSBullet, NoDestroy},
};

static void InitSBullet(entity_t *me, entity_init_t *i) {
  sbullet_t *b = (sbullet_t*)me;

  b->b.pos = i->v4[0];
  b->spd = i->flt[4];
  b->b.scale = VEC4(b->spd/BULLET_SPD, 1, 1, 1);
  b->b.info = &S_SBulletInfo;
  b->life = BULLET_LIFETIME;

  SetSprite(&b->b.spr, SPR_SBULLET);
  PlaySound(SND_SHOOTSPELL);
}

static bbox_t GetSBulletBbox(vec4 pos) {
  return ToIvec4(ShuffleVec4<0x0101>(pos)+VEC4_1(0.5f))+SBULLET_BBOX;
}

static void UpdateSBullet(entity_t *me, const input_t *i) {
  (void)i;

  sbullet_t *b = (sbullet_t*)me;

  b->b.pos.v[0] += b->spd;

  entity_t *killer = EntityCol(me, ENT_SBKILLER);
  if (killer) {
    PlaySound(SND_BREAKBLOCK);
    StopSound(SND_SHOOTSPELL);
    RemoveEntity(killer);
    RemoveEntity(me);
    return;
  }

  if (!--b->life || TileCol(GetSBulletBbox(b->b.pos), g_state->room->map, TILE_BLOCK)) {
    RemoveEntity(me);
    return;
  }
}

// Spell entity
static constexpr const bbox_t SPELL_BBOX = {4, -27, 27, -4};

struct spell_ent_t {
  entity_base_t b;

  spell_t spell;
};

static const entity_info_t S_SpellInfo = {
  ENT_SPELL,
  SPELL_BBOX,
  {NoUpdate, NoDestroy},
};

static void InitSpell(entity_t *me, entity_init_t *data) {
  spell_ent_t *s = (spell_ent_t*)me;

  s->b.pos = data->v4[0];
  s->b.scale = VEC4(1, 1, 1, 1);
  s->b.info = &S_SpellInfo;
  s->spell = data->dword[4];

  SetImageSprite(&s->b.spr, IMG_JUMPSPELL-1+s->spell);
}

// Intro entity
static const entity_info_t S_IntroInfo = {
  ENT_INTRO,
  {},
  {NoUpdate, NoDestroy}
};

static void InitIntro(entity_t *me, entity_init_t *data) {
  (void)data;

  // Get intro image from room name
  image_id_t img = IMG_INTRO0 + g_state->roomName[sizeof("data/room/intro")-1]-'0';
  ASSERT((img >= IMG_INTRO0) && (img <= IMG_INTRO5));

  me->b.pos = VEC4(0.f, 608.f-32.f, 0.f, 0.f);
  me->b.scale = VEC4_1(1.f);
  me->b.info = &S_IntroInfo;

  SetImageSprite(&me->b.spr, img);
}

// Blood emitter entity
static constexpr const ufast EMITTER_LIFETIME = 20;
static constexpr const ufast EMITTER_PARTICLEFREQ = 40;
static constexpr const f32 EMITTER_BLOODGRAVITYBASE = -0.2f/GAME_HEIGHT;
static constexpr const f32 EMITTER_BLOODGRAVITYADD = -0.4f/GAME_HEIGHT;

struct blood_particle_info_t {
  vec4 speed; // = <hspeed vspeed gravity 0>
};

struct blood_particles_t {
  rquad_t quads[EMITTER_LIFETIME*EMITTER_PARTICLEFREQ];
  blood_particle_info_t info[EMITTER_LIFETIME*EMITTER_PARTICLEFREQ];
};

struct blood_emitter_t {
  entity_base_t b;

  blood_particles_t *particles;
  uptr particleCount;
};

static void UpdateBloodEmitter(entity_t *me, const input_t *i);
static void DestroyBloodEmitter(entity_t *me);
static const entity_info_t S_BloodEmitterInfo = {
  ENT_BLOODEMITTER,
  {},
  {UpdateBloodEmitter, DestroyBloodEmitter},
};

static void InitBloodEmitter(entity_t *me, entity_init_t *i) {
  blood_emitter_t *b = (blood_emitter_t*)me;

  b->b.pos = i->v4[0]*VEC4(2.f/GAME_WIDTH, 2.f/GAME_HEIGHT, 1.f, 0.f)-VEC4(1.f, 1.f, 0.f, 0.f);
  b->b.info = &S_BloodEmitterInfo;
  b->particles = (blood_particles_t*)Alloc(sizeof(blood_particles_t));
  b->particleCount = 0;

  SetNullSprite(&b->b.spr);
}

static void UpdateBloodEmitter(entity_t *me, const input_t *input) {
  (void)input;

  blood_emitter_t *b = (blood_emitter_t*)me;

  uptr i;
  if (b->particleCount < EMITTER_LIFETIME*EMITTER_PARTICLEFREQ) {
    for (uptr j = 0; j < EMITTER_PARTICLEFREQ; ++j) {
      i = b->particleCount++;

      b->particles->info[i].speed = ZeroVec4();
      const f32 dir = M_PI * (f32)(Random(&g_state->seed)&65535)*(2.f/65536.f);
      const f32 spd = 6.f * (f32)(Random(&g_state->seed)&65535)*(2.f/65536.f);
      b->particles->info[i].speed.v[2] =
        EMITTER_BLOODGRAVITYADD*(f32)(Random(&g_state->seed)&65535)*(1.f/65536.f) +
        EMITTER_BLOODGRAVITYBASE;;

      b->particles->info[i].speed.v[0] = cosf(dir)*spd/GAME_WIDTH;
      b->particles->info[i].speed.v[1] = sinf(dir)*spd/GAME_HEIGHT;

      b->particles->quads[i] = S_BloodQuad;
      b->particles->quads[i].v[0].pos += b->b.pos;
      b->particles->quads[i].v[1].pos += b->b.pos;
      b->particles->quads[i].v[2].pos += b->b.pos;
      b->particles->quads[i].v[3].pos += b->b.pos;
    }
  }

  // Update all quads
  for (i = 0; i < b->particleCount; ++i) {
    vec4 spd = b->particles->info[i].speed&VEC4_I(-1, -1, 0, 0);
    b->particles->quads[i].v[0].pos += spd;
    b->particles->quads[i].v[1].pos += spd;
    b->particles->quads[i].v[2].pos += spd;
    b->particles->quads[i].v[3].pos += spd;

    b->particles->info[i].speed.v[1] += b->particles->info[i].speed.v[2];
  }

  // Draw all quads
  DrawQuads(b->particles->quads, b->particleCount);
}

static void DestroyBloodEmitter(entity_t *me) {
  blood_emitter_t *b = (blood_emitter_t*)me;
  Free(b->particles);
}

// Gameover entity
static constexpr ufast GAMEOVER_TIMER = 30;

struct gameover_t {
  entity_base_t b;

  ufast timer;
};

static void UpdateGameover(entity_t *me, const input_t *i);

static const entity_info_t S_GameoverInfo = {
  ENT_GAMEOVER,
  {},
  {UpdateGameover, NoDestroy},
};

static void InitGameover(entity_t *me, entity_init_t *data) {
  (void)data;

  gameover_t *g = (gameover_t*)me;

  g->b.pos = VEC4(GAME_WIDTH/2, GAME_HEIGHT/2, -1.f, 0);
  g->b.scale = VEC4(1, 1, 1, 1);
  g->b.info = &S_GameoverInfo;
  g->timer = 0;

  SetNullSprite(&g->b.spr);
}

static void UpdateGameover(entity_t *me, const input_t*) {
  gameover_t *g = (gameover_t*)me;

  if (g->timer++ >= GAMEOVER_TIMER)
    SetImageSprite(&g->b.spr, IMG_GAMEOVER);
}

// Warp entity
static constexpr const ivec4 WARP_BBOX = {6, -26, 26, -6};

struct warp_t {
  entity_base_t b;

  // Warp destination
  char destination[32];
};
static_assert(sizeof(warp_t) <= sizeof(entity_t), "");

static const entity_info_t S_WarpInfo = {
  ENT_WARP,
  WARP_BBOX,
  {NoUpdate, NoDestroy},
};

static void InitWarp(entity_t *me, entity_init_t *i) {
  warp_t *w = (warp_t*)me;

  w->b.pos = i->v4[0];
  w->b.scale = VEC4(1, 1, 1, 1);
  strcpy(w->destination, i->str);
  w->b.info = &S_WarpInfo;

//  SetImageSprite(&w->b.spr, IMG_WARP);
  SetNullSprite(&w->b.spr);
}

// Save entity
static constexpr const i32 SAVE_IDLEFRAMES = 30;
static constexpr const i32 SAVE_LIGHTFRAMES = 60;
static constexpr const ivec4 SAVE_BBOX = {0, -31, 32, 0};

struct save_t {
  entity_base_t b;

  // Idle frame counter
  i32 idleFrames;

  // Lit up frame counter
  i32 lightFrames;
};
static_assert(sizeof(save_t) <= sizeof(entity_t), "");

static void UpdateSave(entity_t *me, const input_t*);

static const entity_info_t S_SaveInfo = {
  ENT_SAVE,
  SAVE_BBOX,
  {UpdateSave, NoDestroy},
};

static void InitSave(entity_t *me, entity_init_t *i) {
  save_t *s = (save_t*)me;

  s->b.pos = i->v4[0];
  s->b.scale = VEC4(1, 1, 1, 1);
  s->b.info = &S_SaveInfo;

  s->idleFrames = 0;
  s->lightFrames = 0;

  SetImageSprite(&s->b.spr, IMG_SAVE);
}

static void UpdateSave(entity_t *me, const input_t*) {
  save_t *s = (save_t*)me;

  --s->idleFrames;
  --s->lightFrames;

  if ((s->b.spr.img == IMG_SAVEHIT) && (s->lightFrames <= 0))
    SetImageSprite(&s->b.spr, IMG_SAVE);
}

// Saves game
static void WriteSave();
static void SaveGame(entity_t *me) {
  save_t *s = (save_t*)me;

  // If we're currently idle, don't save the game
  if (s->idleFrames > 0) return;

  // Set idle frames, lit up frames, and sprite
  s->idleFrames = SAVE_IDLEFRAMES;
  s->lightFrames = SAVE_LIGHTFRAMES;
  SetImageSprite(&s->b.spr, IMG_SAVEHIT);

  PlaySound(SND_SAVE);

  // Write save file
  WriteSave();
}

// Bullet entity
static constexpr const ivec4 BULLET_BBOX = {-1, -3, 3, 1};
static constexpr const u32 BULLET_CAP = 4;

struct bullet_t {
  entity_base_t b;

  f32 spd; // BULLET_SPD or -BULLET_SPD
  u32 life; // Starts at BULLET_LIFETIME
  char savesGame; // Can this bullet save the game?
};
static_assert(sizeof(bullet_t) <= sizeof(entity_t), "");

static void UpdateBullet(entity_t *me, const input_t*);
static void DestroyBullet(entity_t*);

static const entity_info_t S_BulletInfo = {
  ENT_BULLET,
  BULLET_BBOX,
  {UpdateBullet, DestroyBullet},
};

static void InitBullet(entity_t *me, entity_init_t *i) {
  bullet_t *b = (bullet_t*)me;

  b->b.pos = i->v4[0];
  b->b.scale = VEC4(1, 1, 1, 1);
  b->b.info = &S_BulletInfo;
  b->spd = i->flt[4];
  b->life = BULLET_LIFETIME;
  b->savesGame = i->str[0];

  SetSprite(&b->b.spr, SPR_BULLET);

  ++g_state->bulletCount;

  PlaySound(SND_SHOOT);
}

static ivec4 GetBulletBbox(vec4 pos) {
  return ShuffleVec4<0x0101>(ToIvec4(pos))+BULLET_BBOX;
}

static void UpdateBullet(entity_t *me, const input_t*) {
  bullet_t *b = (bullet_t*)me;

  b->b.pos.v[0] += b->spd;
  if (!--b->life || TileCol(GetBulletBbox(me->b.pos), g_state->room->map, TILE_BLOCK)) RemoveEntity(me);

  if (!b->savesGame) return;

  entity_t *s = EntityCol(me, ENT_SAVE);
  if (s) SaveGame(s);
}

static void DestroyBullet(entity_t*) {--g_state->bulletCount;}

// Kid entity
static constexpr const f32 KID_SPD = 3.f;
static constexpr const f32 KID_JUMPHEIGHT = 8.5f;
static constexpr const f32 KID_DJUMPHEIGHT = 7.f;
static constexpr const f32 KID_GRAVITY = -0.4f;
static constexpr const f32 KID_MAXVSP = -9.f+KID_GRAVITY;
static constexpr const f32 KID_VINEVSP = -2.f;
static constexpr const f32 KID_FALLCHANGE = 0.45f;
static constexpr const f32 KID_VINEJUMPHEIGHT = 9.f;
static constexpr const f32 KID_VINEJUMPSPD = 15.f;
static constexpr const bbox_t KID_BBOX = {12-17, 23-32, 23-17, 23-11};
static constexpr const f32 KID_JUMPSPELLHEIGHT = 12.f;
static constexpr const ufast KID_BOOSTTIME = 15;
static constexpr const f32 KID_BOOSTSPD = 4.f;

struct kid_t {
  entity_base_t b;

  f32 vspeed, boostSpeed;
  bfast onGround, djump, platformSnapped;
  ufast boostTimer;
};
static_assert(sizeof(kid_t) <= sizeof(entity_t), "");

static void UpdateKid(entity_t *me, const input_t *i);

static const entity_info_t S_KidInfo = {
  ENT_KID,
  KID_BBOX,
  {UpdateKid, NoDestroy}
};

static void InitKid(entity_t *me, entity_init_t *i) {
  kid_t *k = (kid_t*)me;

  // If another kid entity exists, replace that kid
  for (entity_t *e = g_state->firstEntity; e; e = e->b.next) {
    if ((e != me) && (e->b.info->id == ENT_KID)) {
      RemoveEntity(e);
      break;
    }
  }

  k->b.pos = i->v4[0];
  k->b.scale = i->v4[1];
  k->b.info = &S_KidInfo;

  k->onGround = false;
  k->djump = true;
  k->platformSnapped = false;
  k->vspeed = 0.f;
  k->boostSpeed = 0.f;
  k->boostTimer = 0;

  SetSprite<EXPLICIT>(&me->b.spr, SPR_PSTAND);
}

static inline ivec4 GetKidBbox(vec4 pos) {
  return ToIvec4(ShuffleVec4<0x0101>(pos)+VEC4_1(0.5f)) + KID_BBOX;
}

static void MoveKid(kid_t *me, vec4 offset) {
  me->onGround = false;

  // If there's a block collision, do collision processing
  vec4 newPos = me->b.pos+(offset&VEC4_I(-1, -1, 0, 0));
  if (TileCol(GetKidBbox(me->b.pos+offset), g_state->room->map, TILE_BLOCK)) {
    newPos = me->b.pos+(offset&VEC4_I(-1, 0, 0, 0));

    // Save fractional coordinates
    vec4 frac = me->b.pos-ToVec4(ToIvec4(me->b.pos));

    ivec4 bbox = GetKidBbox(newPos);
    tile_t *t;
    if ((t = TileCol(bbox, g_state->room->map, TILE_BLOCK))) {
      if (offset.v[0] > 0)
        newPos.v[0] = AlignUpMask((i32)newPos.v[0]-KID_BBOX.v[2], TILE_SIZE-1)-KID_BBOX.v[2];
      else
        newPos.v[0] = AlignDownMask((i32)newPos.v[0]-KID_BBOX.v[0], TILE_SIZE-1)-KID_BBOX.v[0];
    }

    newPos += (offset&VEC4_I(0, -1, 0, 0));
    bbox = GetKidBbox(newPos);

    if (TileCol(bbox, g_state->room->map, TILE_BLOCK)) {
      if (offset.v[1] > 0) {
        newPos.v[1] = AlignUpMask((i32)newPos.v[1], TILE_SIZE-1)-KID_BBOX.v[3]+frac.v[1];
        if (frac.v[1] > 0.5f) newPos.v[1] -= 1.f;
      } else {
        newPos.v[1] = AlignDownMask((i32)newPos.v[1]+TILE_SIZE/2, TILE_SIZE-1)-KID_BBOX.v[1]+frac.v[1];
        if (frac.v[1] > 0.5f) newPos.v[1] -= 1.f;
      }

      me->vspeed = 0.f;
    }
  }

  // Collide with walls and the ceiling
  if (newPos.v[0] < -KID_BBOX.v[0]) newPos.v[0] = -KID_BBOX.v[0];
  else if (newPos.v[0] > GAME_WIDTH-KID_BBOX.v[2]) newPos.v[0] = GAME_WIDTH-KID_BBOX.v[2];
  if (newPos.v[1] > GAME_HEIGHT-KID_BBOX.v[3]) {
    newPos.v[1] = GAME_HEIGHT-KID_BBOX.v[3];
    me->vspeed = 0.f;
  }

  // Check if we're on the ground
  // This may seem unnecessary, since we have a ground collision check earlier,
  // but sometimes, when bunny hopping, you'd double jump if I set onGround there
  const bbox_t bbox = GetKidBbox(newPos)-IVEC4(0, 1, 0, 0);
  if (TileCol(bbox, g_state->room->map, TILE_BLOCK))
    me->onGround = me->djump = true;
  else if (TileCol(bbox, g_state->room->map, TILE_PLATFORM))
    me->onGround = true;

  me->b.pos = newPos;
}

static void LoadRoom(const char *roomName);
static void UpdateKid(entity_t *me, const input_t *i) {
  kid_t *k = (kid_t*)me;
  sprite_id_t destSpr = SPR_PSTAND;
  vec4 offset = ZeroVec4();

  if (i->down&INPUT_RIGHTBIT) {
    offset.v[0] = KID_SPD;
    me->b.scale.v[0] = 1.f;
    destSpr = SPR_PWALK;
  } else if (i->down&INPUT_LEFTBIT) {
    offset.v[0] = -KID_SPD;
    me->b.scale.v[0] = -1.f;
    destSpr = SPR_PWALK;
  }

  // Add boost speed
  if (k->boostTimer) {
    --k->boostTimer;
    offset.v[0] += k->boostSpeed;
  }

  if (i->pressed&INPUT_JUMPBIT) {
    if (k->onGround) {
      k->vspeed = KID_JUMPHEIGHT;
      PlaySound(SND_JUMP);
      k->djump = true;
    } else if (k->djump ||
               // Infinite jump (same as god mode key)
               (DEBUG_KEYS && (i->down&INPUT_DOWNBIT))) {
      k->vspeed = KID_DJUMPHEIGHT;
      k->djump = false;
      PlaySound(SND_DJUMP);
    }
  }

  // Shoot bullet
  if ((i->pressed&INPUT_SHOOTBIT) && (g_state->bulletCount < BULLET_CAP) &&
      !(DEBUG_KEYS && (i->down&INPUT_DOWNBIT)))
  {
    // If we're colliding with a save, save the game
    entity_t *s = EntityCol(me, ENT_SAVE);
    if (s) SaveGame(s);

#if 0
    entity_init_t init;
    init.v4[0] = me->b.pos;
    init.flt[2] = -0.1f; // Bullet depth
    init.flt[4] = BULLET_SPD*me->b.scale.v[0];
    init.ent = ENT_BULLET;
    init.str[0] = !s; // Can this bullet save the game?

    AddEntity(&init);
#endif

    // Otherwise, cast spell
    else {
      switch (g_state->curSpell) {
      case SPELL_JUMP:
        PlaySound(SND_JUMPSPELL);
        k->vspeed = KID_JUMPSPELLHEIGHT;
        break;
      case SPELL_SHOOT: {
        entity_init_t init;
        init.v4[0] = me->b.pos;
        init.flt[2] = -0.1f;
        init.flt[4] = BULLET_SPD*me->b.scale.v[0];
        init.ent = ENT_SBULLET;

        AddEntity(&init);
        break;
      }
      case SPELL_SPEED:
        k->boostTimer = KID_BOOSTTIME;
        k->boostSpeed = KID_BOOSTSPD*me->b.scale.v[0];
        PlaySound(SND_SPEEDSPELL);
        break;
      case SPELL_FINAL:
        LoadRoom("data/room/ending.rm");
        g_state->resetTick = true;
        g_state->curSpell = SPELL_NONE;
        PlaySound(SND_THUNDER);
        return;
      default: PlaySound(SND_NOSPELL); break;
      }

      // Reset spell after casting
      g_state->curSpell = SPELL_NONE;
    }
  }

  // If I shoot while holding down, increment g_state->curSpell
  if (DEBUG_KEYS && (i->pressed&INPUT_SHOOTBIT) && (i->down&INPUT_DOWNBIT))
    ++g_state->curSpell;

  if ((i->released&INPUT_JUMPBIT) && (k->vspeed > 0.f)) k->vspeed *= KID_FALLCHANGE;

  if (!k->platformSnapped) {
    if (k->vspeed > 0.05f) destSpr = SPR_PJUMP;
    else if (k->vspeed < -0.05f) destSpr = SPR_PFALL;
  } else if (k->vspeed != 0.f) k->platformSnapped = false;

  // Vine checks
  bbox_t bbox = GetKidBbox(me->b.pos);
  if (TileCol(bbox+IVEC4(-1, 0, -1, 0), g_state->room->map, TILE_BLOCK, TILE_RVINEBIT)) {
    k->vspeed = KID_VINEVSP;
    destSpr = SPR_PVINE;

    // Did we jump off?
    if ((i->pressed&INPUT_RIGHTBIT) && (i->down&INPUT_JUMPBIT)) {
      offset.v[0] = KID_VINEJUMPSPD;
      k->vspeed = KID_VINEJUMPHEIGHT;
      destSpr = SPR_PJUMP;
      PlaySound(SND_VINEJUMP);
    }
  } else if (TileCol(bbox+IVEC4(1, 0, 1, 0), g_state->room->map, TILE_BLOCK, TILE_LVINEBIT)) {
    k->vspeed = KID_VINEVSP;
    destSpr = SPR_PVINE;

    // Did we jump off?
    if ((i->pressed&INPUT_LEFTBIT) && (i->down&INPUT_JUMPBIT)) {
      offset.v[0] = -KID_VINEJUMPSPD;
      k->vspeed = KID_VINEJUMPHEIGHT;
      destSpr = SPR_PJUMP;
      PlaySound(SND_VINEJUMP);
    }
  }

  k->vspeed += KID_GRAVITY;
  if (k->vspeed < KID_MAXVSP) k->vspeed = KID_MAXVSP;

  offset.v[1] = k->vspeed;

  // Quicksave
  if (DEBUG_KEYS && (i->pressed&INPUT_UPBIT))
    WriteSave();

  MoveKid(k, offset);

  // Touch warp before anything else
  warp_t *w = (warp_t*)EntityCol(me, ENT_WARP);
  if (w) {
    LoadRoom(w->destination);
    g_state->resetTick = true; // Entity list was refreshed, reset game tick
    return; // This kid shouldn't exist anymore
  }

  // Platform collision
  bbox = GetKidBbox(me->b.pos);
  tile_t *t;
  if ((t = TileCol(bbox, g_state->room->map, TILE_PLATFORM))) {
    f32 py = ((t-g_state->room->map)/TILE_MAP_WIDTH)*32.f+32.f;
    if (k->b.pos.v[1]-k->vspeed*0.5f >= py) {
      k->b.pos.v[1] = py+9.f;
      k->vspeed = 0.f;
      k->platformSnapped = true;
      k->djump = true;
    }
  }

  if ((me->b.pos.v[1] < 0) ||
      // God mode (same as infinite jump key)
      (!(DEBUG_KEYS && (i->down&INPUT_DOWNBIT)) &&
       (TileCol(GetKidBbox(me->b.pos), g_state->room->map, TILE_KILLER) ||
        EntityCol(me, ENT_SBKILLER))))
  {
    PlaySound(SND_DEATH);

    entity_init_t ent;
    ent.ent = ENT_GAMEOVER;
    AddEntity(&ent);

    ent.v4[0] = me->b.pos;
    ent.ent = ENT_BLOODEMITTER;
    AddEntity(&ent);

    RemoveEntity(me);
    return;
  }

  // Spell collision
  entity_t *e;
  if (!g_state->curSpell && (e = EntityCol(me, ENT_SPELL))) {
    g_state->curSpell = ((spell_ent_t*)e)->spell;
    RemoveEntity(e);
    PlaySound(SND_GETSPELL);
  }

  SetSprite(&me->b.spr, destSpr);
}

// Set room clear color, based on room name
static void SetRoomClearColor(const char *filename) {
  if ((strlen(filename) >= 15) && !memcmp("data/room/intro", filename, 15))
    SetClearColor(0.f, 0.f, 0.f);
  else if ((strlen(filename) >= 11) && !memcmp("data/room/2", filename, 11))
    SetClearColor(0.08f/2.f, 0.182f/2.f, 0.2f/2.f);
  else if ((strlen(filename) >= 11) && !memcmp("data/room/3", filename, 11))
    SetClearColor(0.2f, 0.037f, 0.f);
  else if ((strlen(filename) >= 15) && !memcmp("data/room/clear", filename, 15))
    SetClearColor(0, 1, 1);
  else
    SetClearColor(0.996f, 0.561f, 0.231f);
}

// Load room
static void LoadRoom(const char *filename) {
  // Load room file
  if (!g_state->room || USE_EDITOR || strcmp(g_state->roomName, filename)) {
    SetRoomClearColor(filename);

    strcpy(g_state->roomName, filename);

    // Free room, if it exists
    if (g_state->room) Free(g_state->room);

    stream_t f = {};
    if (!OpenFile(&f, filename, FILE_READ_ONLY))
      LOG_ERROR(FMT.s("Cannot open room ").s(filename).STR);

    // Get file size
    f.f->seek(&f, 0, ORIGIN_END);
    const iptr fileSize = f.f->tell(&f);
    f.f->seek(&f, 0, ORIGIN_SET);

    // Make sure file size is valid
    if (fileSize <= (iptr)sizeof(room_t))
      LOG_ERROR(FMT.s(filename).s(" isn't a room!").STR);

    // Allocate room
    g_state->room = (room_t*)Alloc(fileSize);

    // Read & swap room
    f.f->read(&f, g_state->room, fileSize);
    CloseFile(&f);

    g_state->room->swap();

    // Set image page
    SetPage(g_state->room->page);
  }

  if (g_state->state == GAME_PLAY) {
    // Remove all currently existing entities
    while (g_state->firstEntity) RemoveEntity(g_state->firstEntity);

    // Room loaded, add room entities
    for (uptr i = 0; i < g_state->room->entityCount; ++i)
      AddEntity(&g_state->room->entities()[i]);
  }

  // Play BGM
  PlayBGM(g_state->room->bgm);

  // Get rid of current spell
  g_state->curSpell = SPELL_NONE;
}

// Write save game
static void WriteSave() {
  // Find the kid
  entity_t *e;
  for (e = g_state->firstEntity; e; e = e->b.next)
    if (e->b.info->id == ENT_KID) break;

  // If kid wasn't found, don't save
  if (!e) return;

  // Validate save
  g_state->save.validate();

  // Save kid position and scale
  // Round position's y coordinate
  g_state->save.kidInit.v4[0] = e->b.pos;
  g_state->save.kidInit.flt[1] = (i32)(g_state->save.kidInit.flt[1]+0.5f);
  g_state->save.kidInit.v4[1] = e->b.scale;
  g_state->save.kidInit.ent = ENT_KID;

  // Save room name
  strcpy(g_state->save.roomName, g_state->roomName);
}

// Load saved game
static void LoadSave() {
  // If the save is invalid, load save file
  if (!g_state->save.valid()) {
    // Load save file
    stream_t saveFile = {};
    if (USE_SAVE && OpenFile(&saveFile, "save.dat", FILE_READ_ONLY)) {
      LOG_INFO("Loading save");

      // Load save data
      saveFile.f->read(&saveFile, &g_state->save, sizeof(g_state->save));
      CloseFile(&saveFile);

      // Swap save data
      g_state->save.swap();
    }

    // If the save still isn't valid, load initial room
    if (!g_state->save.valid()) {
      LOG_INFO("Loading initial room");
      LoadRoom(INITIAL_ROOM);
      return;
    }
  }

  // Load room
  LoadRoom(g_state->save.roomName);

  // Add saved kid entity
  AddEntity(&g_state->save.kidInit);
}

game_state_t *g_state;

void InitGame() {
  // Allocate g_state
  g_state = (game_state_t*)Alloc(sizeof(game_state_t));
  memset(g_state, 0, sizeof(game_state_t));

  // Initialize entity buffer
  InitBuffer(g_state->entityBuf);

  // Randomize RNG seed
  g_state->seed = RandomSeed();

  // Set game state
  g_state->state = GAME_TITLE;

  // Set game page
  SetPage(0);
}

void FreeGame() {
  Free(g_state);
}

static void TickGame(input_t *input) {
  // Restart game
  if (input->pressed&INPUT_RESTARTBIT)
    LoadSave();

  // Update all entities in list
  for (entity_t *e = g_state->firstEntity; e; e = e->b.next) {
    // Reset game tick if needed
    if (g_state->resetTick) {
      g_state->resetTick = false;
      e = g_state->firstEntity;
    }

    // Update entity sprite
    UpdateSprite(&e->b.spr);

    UpdateEntity(e, input);
  }

  // Start new game
  if (input->pressed&INPUT_NEWGAMEBIT) g_state->state = GAME_TITLE;

  // Reset pressed and released button
  input->pressed = input->released = 0;
}

// Find quad for tile
static rquad_t *GetTileQuad(rquad_t *quads, uptr quadCount, uptr tilePos) {
  f32 quadX = (f32)(tilePos%TILE_MAP_WIDTH)*2.f/TILE_MAP_WIDTH-1.f;
  f32 quadY = 2.f/TILE_MAP_HEIGHT + (f32)(tilePos/TILE_MAP_WIDTH)*2.f/TILE_MAP_HEIGHT-1.f;

  for (rquad_t *q = quads+quadCount; q-- != quads;)
    if ((q->v[0].pos.v[0] == quadX) &&
        (q->v[0].pos.v[1] == quadY))
      return q;

  ASSERT(!"No tile found!");
}

// Get editor tile from tile and it's quad
static editor_tile_t EditorTileFromTileQuad(rquad_t *q, tile_t t) {
  switch (t) {
  case (TILE_MASK_FULL<<TILE_MASKSHIFT)|(TILE_BLOCK<<TILE_IDSHIFT):
    switch (q->v[0].coord.x|(q->v[0].coord.y<<16)) {
    case 0: return ETILE_BLOCK1;
    case 64: return ETILE_BLOCK3;
    case 32: return ETILE_BLOCK2;
    case 96: return ETILE_BLOCK4;
    case 0|(32<<16): return ETILE_BLOCK5;
    case 32|(32<<16): return ETILE_BLOCK6;
    case 1062: return ETILE_ENTRANCE;

    default: return ETILE_NONE;
    }

  case (TILE_MASK_DSPIKE<<TILE_MASKSHIFT)|(TILE_KILLER<<TILE_IDSHIFT): return ETILE_SPIKEDOWN;
  case (TILE_MASK_USPIKE<<TILE_MASKSHIFT)|(TILE_KILLER<<TILE_IDSHIFT): return ETILE_SPIKEUP;
  case (TILE_MASK_LSPIKE<<TILE_MASKSHIFT)|(TILE_KILLER<<TILE_IDSHIFT): return ETILE_SPIKELEFT;
  case (TILE_MASK_RSPIKE<<TILE_MASKSHIFT)|(TILE_KILLER<<TILE_IDSHIFT): return ETILE_SPIKERIGHT;

  case (TILE_MASK_FULL<<TILE_MASKSHIFT)|(TILE_BLOCK<<TILE_IDSHIFT)|TILE_LVINEBIT:
    switch (q->v[0].coord.x) {
    case 0: return ETILE_LVINE1;
    case 32: return ETILE_LVINE2;

    default: return ETILE_NONE;
    }

  case (TILE_MASK_FULL<<TILE_MASKSHIFT)|(TILE_BLOCK<<TILE_IDSHIFT)|TILE_RVINEBIT:
    switch (q->v[0].coord.x) {
    case 0: return ETILE_RVINE1;
    case 32: return ETILE_RVINE2;

    default: return ETILE_NONE;
    }

  case (TILE_MASK_PLATFORM<<TILE_MASKSHIFT)|(TILE_PLATFORM<<TILE_IDSHIFT):
    return ETILE_PLATFORM;

  case TILE_PROP<<TILE_IDSHIFT:
    switch (q->v[0].coord.x|(q->v[0].coord.y<<16)) {
    case 486|(194<<16): return ETILE_BLACK;

    default: return ETILE_NONE;
    }

  default: return ETILE_NONE;
  }
}

static void TickTitle(input_t *input) {
  static const rquad_t titleQuad = {{
      {{-1.f, 1.f, 0.f, 260, 0}},
      {{1.f, 1.f, 0.f, 1060, 0}},
      {{-1.f, -1.f, 0.f, 260, 608}},
      {{1.f, -1.f, 0.f, 1060, 608}},
    }};
  DrawQuads(&titleQuad, 1);

  PlayBGM("data/bgm/title.wav");

  if (input->pressed&INPUT_JUMPBIT) {
    if (!USE_EDITOR) {
      // Set new game state
      g_state->state = GAME_PLAY;
      LoadSave();
    } else {
      if (OPEN_EDITOR_LEVEL) {
        // Load editor room
        LoadRoom(EDITOR_LEVEL);

        // Load editor data
        g_state->entCount = g_state->room->entityCount;
        memcpy(g_state->ents, g_state->room->entities(), sizeof(entity_init_t)*g_state->entCount);

        for (uptr i = 0; i < TILE_MAP_WIDTH*TILE_MAP_HEIGHT; ++i) {
          if (g_state->room->map[i])
            g_state->map[i] =
              EditorTileFromTileQuad(GetTileQuad(g_state->room->quads(), g_state->room->quadCount, i),
                                     g_state->room->map[i]);
          else
            g_state->map[i] = ETILE_NONE;
        }
      } else {
        g_state->entCount = 0;
        memset(g_state->ents, 0, sizeof(g_state->ents));
        memset(g_state->map, 0, sizeof(g_state->map));
      }

      PlayBGM(EDITOR_BGM);

      // Set editor state
      g_state->state = GAME_EDITOR;

      SetRoomClearColor(EDITOR_LEVEL);
    }
  }

  // Reset pressed and released button
  input->pressed = input->released = 0;
}

static void DrawEditorEntity(entity_init_t *e, vec4 pos = {}, bfast usePos = false) {
  image_id_t img = IMG_COUNT;
  vec4 scale = e->v4[1];

  switch (e->ent) {
  case ENT_KID:
  case ENT_IDLEKID: img = IMG_PSTAND0; break;
  case ENT_BULLET: img = IMG_BULLET0; break;
  case ENT_SAVE: img = IMG_SAVE; break;
  case ENT_WARP: img = IMG_WARP; break;
  case ENT_INTRO: img = IMG_INTRO0; break;
  case ENT_SPELL:
    img = IMG_JUMPSPELL-1+e->dword[4];
    scale = VEC4(1, 1, 1, 1);
    break;
  case ENT_SBKILLER: img = IMG_SBKILLER; break;
  case ENT_DRAGON: img = IMG_DRAGON; break;
  case ENT_DRAGONDEFEAT: img = IMG_WHITEDRAGON; break;
  case ENT_THUNDER: img = IMG_THUNDER0; break;
  }

  if (!usePos) pos = e->v4[0]*VEC4(2.f/GAME_WIDTH, 2.f/GAME_HEIGHT, 1.f, 0)-VEC4(1, 1, 0, 0);
  if (img != IMG_COUNT) DrawImage(pos, scale, img);
}

static bfast IsStaticEntity(entity_init_t *e) {
  return ((e->ent == ENT_WARP));
}

static f32 TileDepth(editor_tile_t etile) {
  switch (etile) {
  case ETILE_BLACK: return 0.f;
  case ETILE_ENTRANCE: return -0.99f;
  default: return 0.25f;
  }
}

static void TickEditor(input_t *input) {
  // Draw entities
  for (uptr i = 0; i < g_state->entCount; ++i)
    DrawEditorEntity(&g_state->ents[i]);

  // Draw tiles
  for (uptr i = 0; i < TILE_MAP_WIDTH*TILE_MAP_HEIGHT; ++i) {
    rquad_t tileQuad;
    vec4 drawPos = VEC4(-1.f, -1.f + 2.f/TILE_MAP_HEIGHT, 0.f, 0.f);
    drawPos.v[0] += 2.f/TILE_MAP_WIDTH * (i%TILE_MAP_WIDTH);
    drawPos.v[1] += 2.f/TILE_MAP_HEIGHT * (i/TILE_MAP_WIDTH);

    drawPos.v[2] = TileDepth(g_state->map[i]);

    tileQuad = S_TileQuad[g_state->map[i]];
    tileQuad.v[0].pos += drawPos;
    tileQuad.v[1].pos += drawPos;
    tileQuad.v[2].pos += drawPos;
    tileQuad.v[3].pos += drawPos;

    if (!g_state->map[i]) continue;

    DrawQuads(&tileQuad, 1);
  }

  // Special keys
  if (input->down&INPUT_JUMPBIT) {
    if (input->pressed&INPUT_UPBIT) {
      if (g_state->mode == 0) ++g_state->curTile;
      else ++g_state->ents[g_state->entCount].ent;
    } else if (input->pressed&INPUT_DOWNBIT) {
      if (g_state->mode == 0) --g_state->curTile;
      else --g_state->ents[g_state->entCount].ent;
    }

    // Change spell
    if (input->pressed&INPUT_LEFTBIT) --g_state->curSpell;
    else if (input->pressed&INPUT_RIGHTBIT) ++g_state->curSpell;

    // Write room
    if (input->pressed&INPUT_NEWGAMEBIT) {
      // Count tile quads
      uptr tileCnt = 0;
      for (uptr i = 0; i < TILE_MAP_WIDTH*TILE_MAP_HEIGHT; ++i)
        if (g_state->map[i] != ETILE_NONE) ++tileCnt;

      // Also count static entities
      for (uptr i = 0; i < g_state->entCount; ++i)
        if (IsStaticEntity(&g_state->ents[i])) ++tileCnt;
      
      room_t *out = (room_t*)Alloc(ROOM_SIZE(g_state->entCount, tileCnt));
      out->entityCount = g_state->entCount;
      out->quadCount = tileCnt;
      out->page = EDITOR_PAGE;
      strcpy(out->bgm, EDITOR_BGM);

      memcpy(out->entities(), g_state->ents, sizeof(entity_init_t)*g_state->entCount);

      // Read tile quads
      rquad_t *q = out->quads();
      for (uptr i = 0; i < TILE_MAP_WIDTH*TILE_MAP_HEIGHT; ++i) {
        if (g_state->map[i] != ETILE_NONE) {
          vec4 drawPos = ZeroVec4();

          drawPos.v[0] = (f32)(i%TILE_MAP_WIDTH)*2.f/TILE_MAP_WIDTH-1.f;
          drawPos.v[1] = 2.f/TILE_MAP_HEIGHT + (f32)(i/TILE_MAP_WIDTH)*2.f/TILE_MAP_HEIGHT-1.f;

          drawPos.v[2] = TileDepth(g_state->map[i]);
          *q = S_TileQuad[g_state->map[i]];
          q->v[0].pos += drawPos;
          q->v[1].pos += drawPos;
          q->v[2].pos += drawPos;
          q++->v[3].pos += drawPos;
        }

        out->map[i] = S_TileCode[g_state->map[i]];
      }

      // Read entity quads
      for (uptr i = 0; i < g_state->entCount; ++i) {
        if (IsStaticEntity(&g_state->ents[i])) {
          vec4 drawPos = ZeroVec4();

          drawPos.v[0] = g_state->ents[i].flt[0]*(2.f/GAME_WIDTH)-1.f;
          drawPos.v[1] = g_state->ents[i].flt[1]*(2.f/GAME_HEIGHT)-1.f;

          drawPos.v[2] = 0.1f;
          *q = S_EntityQuad[g_state->ents[i].ent];
          q->v[0].pos += drawPos;
          q->v[1].pos += drawPos;
          q->v[2].pos += drawPos;
          q++->v[3].pos += drawPos;
        }
      }

      stream_t f = {};
      if (OpenFile(&f, EDITOR_LEVEL, FILE_WRITE_ONLY)) {
        out->swap();
        f.f->write(&f, out, ROOM_SIZE(g_state->entCount, tileCnt));
        CloseFile(&f);
        LOG_STATUS("Level written");
      } else LOG_STATUS("!! Unable to save level! !!");

      Free(out);
    }

    // Start gameplay
    if (input->pressed&INPUT_RESTARTBIT) {
      g_state->state = GAME_PLAY;
      LoadRoom(EDITOR_LEVEL);
      WriteSave();
    }

    // Remove tile/entity
    if (input->pressed&INPUT_SHOOTBIT) {
      if (g_state->mode == 0) g_state->map[g_state->cur] = 0;
      else {
        const i32 x = g_state->cur%TILE_MAP_WIDTH;
        const i32 y = g_state->cur/TILE_MAP_WIDTH;
        for (uptr i = 0; i < g_state->entCount; ++i) {
          if ((((i32)g_state->ents[i].flt[0])/TILE_SIZE == x) &&
              (((i32)g_state->ents[i].flt[1])/TILE_SIZE == y))
          {
            memmove(g_state->ents+i, g_state->ents+i+1, (--g_state->entCount-i)*sizeof(entity_init_t));
            break;
          }
        }
      }
    }
  } else {
    if (input->pressed&INPUT_RESTARTBIT) g_state->mode ^= 1;

    if (input->pressed&INPUT_UPBIT) g_state->cur += TILE_MAP_WIDTH;
    else if (input->pressed&INPUT_DOWNBIT) g_state->cur -= TILE_MAP_WIDTH;

    if (input->pressed&INPUT_LEFTBIT) --g_state->cur;
    else if (input->pressed&INPUT_RIGHTBIT) ++g_state->cur;

    if (input->pressed&INPUT_SHOOTBIT) {
      if (g_state->mode == 0) g_state->map[g_state->cur] = g_state->curTile;
      else ++g_state->entCount;
    }
  }

  // Draw cursor
  vec4 drawPos = VEC4(-1.f, -1.f+2.f/TILE_MAP_HEIGHT, 0.1f, 0.f);
  drawPos.v[0] += 2.f/TILE_MAP_WIDTH * (g_state->cur%TILE_MAP_WIDTH);
  drawPos.v[1] += 2.f/TILE_MAP_HEIGHT * (g_state->cur/TILE_MAP_WIDTH);

  if (g_state->mode == 0) {
    // Tile mode
    rquad_t tileQuad = S_TileQuad[g_state->curTile];
    tileQuad.v[0].pos += drawPos;
    tileQuad.v[1].pos += drawPos;
    tileQuad.v[2].pos += drawPos;
    tileQuad.v[3].pos += drawPos;

    DrawQuads(&tileQuad, 1);
  } else {
    // Entity mode
    g_state->ents[g_state->entCount].v4[0] = ZeroVec4();
    g_state->ents[g_state->entCount].v4[1] = VEC4(1, 1, 1, 1);
    g_state->ents[g_state->entCount].flt[0] = (g_state->cur%TILE_MAP_WIDTH)*32.f;
    g_state->ents[g_state->entCount].flt[1] = (g_state->cur/TILE_MAP_WIDTH)*32.f;

    // Entity-specific things
    switch (g_state->ents[g_state->entCount].ent) {
    case ENT_KID:
    case ENT_IDLEKID: g_state->ents[g_state->entCount].v4[0] += VEC4(17.f, -23.f, -0.1f, 0.f); break;
    case ENT_SAVE: g_state->ents[g_state->entCount].flt[2] = 0.5f; break;
    case ENT_WARP: strcpy(g_state->ents[g_state->entCount].str, EDITOR_DESTINATION); break;
    case ENT_SPELL: g_state->ents[g_state->entCount].dword[4] = g_state->curSpell; break;
    }

    DrawEditorEntity(&g_state->ents[g_state->entCount]);
  }

  // Reset input
  input->pressed = input->released = 0;
}

static const rquad_t S_SpellQuad[SPELL_COUNT-1] = {
  // SPELL_JUMP
  {{
      {{-1.f+64.f/GAME_WIDTH, 1.f-64.f/GAME_HEIGHT, -0.99f, 134, 168}},
      {{-1.f+160.f/GAME_WIDTH, 1.f-64.f/GAME_HEIGHT, -0.99f, 134+24, 168}},
      {{-1.f+64.f/GAME_WIDTH, 1.f-160.f/GAME_HEIGHT, -0.99f, 134, 168+24}},
      {{-1.f+160.f/GAME_WIDTH, 1.f-160.f/GAME_HEIGHT, -0.99f, 134+24, 168+24}},
    }},

  // SPELL_SHOOT
  {{
      {{-1.f+64.f/GAME_WIDTH, 1.f-64.f/GAME_HEIGHT, -0.99f, 160, 168}},
      {{-1.f+160.f/GAME_WIDTH, 1.f-64.f/GAME_HEIGHT, -0.99f, 160+24, 168}},
      {{-1.f+64.f/GAME_WIDTH, 1.f-160.f/GAME_HEIGHT, -0.99f, 160, 168+24}},
      {{-1.f+160.f/GAME_WIDTH, 1.f-160.f/GAME_HEIGHT, -0.99f, 160+24, 168+24}},
    }},

  // SPELL_SPEED
  {{
      {{-1.f+64.f/GAME_WIDTH, 1.f-64.f/GAME_HEIGHT, -0.99f, 186, 168}},
      {{-1.f+160.f/GAME_WIDTH, 1.f-64.f/GAME_HEIGHT, -0.99f, 186+24, 168}},
      {{-1.f+64.f/GAME_WIDTH, 1.f-160.f/GAME_HEIGHT, -0.99f, 186, 168+24}},
      {{-1.f+160.f/GAME_WIDTH, 1.f-160.f/GAME_HEIGHT, -0.99f, 186+24, 168+24}},
    }},

  // SPELL_FINAL
  {{
      {{-1.f+64.f/GAME_WIDTH, 1.f-64.f/GAME_HEIGHT, -0.99f, 212, 168}},
      {{-1.f+160.f/GAME_WIDTH, 1.f-64.f/GAME_HEIGHT, -0.99f, 212+24, 168}},
      {{-1.f+64.f/GAME_WIDTH, 1.f-160.f/GAME_HEIGHT, -0.99f, 212, 168+24}},
      {{-1.f+160.f/GAME_WIDTH, 1.f-160.f/GAME_HEIGHT, -0.99f, 212+24, 168+24}},
    }},
};

static const rquad_t S_SpellTutQuad = {{
    {{-1.f+64.f/GAME_WIDTH, 1.f-175.f/GAME_HEIGHT, -0.99f, 730, 1489}},
    {{-1.f+(64.f+94.f*4.f)/GAME_WIDTH, 1.f-175.f/GAME_HEIGHT, -0.99f, 730+94, 1489}},
    {{-1.f+64.f/GAME_WIDTH, 1.f-(175.f+14.f*4.f)/GAME_HEIGHT, -0.99f, 730, 1489+14}},
    {{-1.f+(64.f+94.f*4.f)/GAME_WIDTH, 1.f-(175.f+14.f*4.f)/GAME_HEIGHT, -0.99f, 730+94, 1489+14}},
  }};

void UpdateGame(input_t *input) {
  switch (g_state->state) {
  case GAME_PLAY:
    // Quick reset to 1-1
    if ((input->down&(INPUT_UPBIT|INPUT_DOWNBIT|INPUT_NEWGAMEBIT)) ==
        (INPUT_UPBIT|INPUT_DOWNBIT|INPUT_NEWGAMEBIT))
    {
      LoadRoom("data/room/11.rm");
      input->pressed &= ~INPUT_NEWGAMEBIT;
    }

    TickGame(input);

    // Draw all entities in list
    for (entity_t *e = g_state->firstEntity; e; e = e->b.next) {
      if (e->b.spr.img == IMG_NONE) continue;

      const vec4 pos = e->b.pos*VEC4(2.f/GAME_WIDTH, 2.f/GAME_HEIGHT, 1.f, 0)-VEC4(1, 1, 0, 0);
      DrawImage(pos, e->b.scale, e->b.spr.img);
    }

    // Draw tiles
    DrawQuads(g_state->room->quads(), g_state->room->quadCount);

    // Draw spell, if present
    if (g_state->curSpell) {
      DrawQuads(&S_SpellQuad[g_state->curSpell-1], 1);

      // If we're on level 2, show spell tutorial text
      if (!strcmp(g_state->roomName, "data/room/12.rm"))
        DrawQuads(&S_SpellTutQuad, 1);
    }
    break;

  case GAME_EDITOR:
    TickEditor(input);
    break;

  case GAME_TITLE: TickTitle(input); break;
  }

  RenderGame();
}
