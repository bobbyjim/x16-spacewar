#include <stdio.h>
#include <stdlib.h>
#include <cbm.h>
#include <conio.h>
#include <cx16.h>
#include <time.h>
#include <unistd.h>

#include "sprite.h"
#include "PSG.h"

#define MAX_V                         2

#define	LOAD_TO_MAIN_RAM	           0
#define	LOAD_TO_VERA	              2  

#define  SPRITE_ADDR_BEGIN       0x4000
// 8bpp 16x16
#define  STAR_ADDR               0x4000
#define  ASTEROID_ADDR           0x4100
// 8bpp 32x32
#define  SHIP_ADDR_0             0x4200
#define  SHIP_ADDR_22            0x4600
#define  SHIP_ADDR_45            0x4a00
#define  SHIP_ADDR_68            0x4e00
#define  SHIP_ADDR_90            0x5200

#define  STAR_X                  SPRITE_X_SCALE(312)
#define  STAR_Y                  SPRITE_Y_SCALE(200)

SpriteDefinition star, asteroid, ship;
unsigned wait;


void loadFile(char *fname, unsigned address)
{
   cbm_k_setnam(fname);
   cbm_k_setlfs(0,8,0);
   cbm_k_load(LOAD_TO_MAIN_RAM, address);
}

void initSprite()
{
   ship.mode            = SPRITE_MODE_8BPP;
   ship.dimensions      = SPRITE_32_BY_32;
   ship.block           = SHIP_ADDR_0;
   ship.collision_mask  = 0x0000;
   ship.layer           = SPRITE_LAYER_0;
   ship.palette_offset  = 0;
   ship.x               = SPRITE_X_SCALE(100);
   ship.y               = SPRITE_Y_SCALE(200);

   sprite_define(1, &ship);

   star = ship;
   star.dimensions      = SPRITE_16_BY_16;
   star.block           = STAR_ADDR;
   star.x               = STAR_X;
   star.y               = STAR_Y;

   sprite_define(2, &star);
   
   asteroid = star;
   asteroid.block       = ASTEROID_ADDR;
   asteroid.x           = SPRITE_X_SCALE(50);
   asteroid.y           = SPRITE_Y_SCALE(50);
   asteroid.dx          = 5;
   asteroid.dy          = 2;

   sprite_define(3, &asteroid);
}

void move_asteroid()
{
   if (++wait < 128) return;
   wait = 0;

   sprite_pos(3, &asteroid);
   asteroid.x += asteroid.dx >> 5;
   asteroid.y += asteroid.dy >> 5;

   if (asteroid.x < STAR_X) asteroid.dx++;
   else if (asteroid.x > STAR_X) asteroid.dx--;
   if (asteroid.y < STAR_Y) asteroid.dy++;
   else if (asteroid.y > STAR_Y) asteroid.dy--;
   if (asteroid.dx < 0) asteroid.flip_horiz = 1;
   else if (asteroid.dx > 0) asteroid.flip_horiz = 0;
   if (asteroid.dy < 0) asteroid.flip_vert  = 0;
   else if (asteroid.dy > 0) asteroid.flip_vert  = 1;
}

void move_ship()
{
   int tan;

   tan = (abs(ship.dy) * 4 / abs(ship.dx));

   switch(tan)
   {
      case 0: ship.block = SHIP_ADDR_90; break;
      case 1: ship.block = SHIP_ADDR_68; break;
      case 2: ship.block = SHIP_ADDR_45; break;
      case 4: ship.block = SHIP_ADDR_22; break;
      case 8: ship.block = SHIP_ADDR_0;  break;
   }

   ship.flip_horiz = (ship.dx < 0);
   ship.flip_vert  = (ship.dy > 0);

   sprite_define( 1, &ship ); // in case of block change
   ship.x  += ship.dx;
   ship.y  += ship.dy;

   if (kbhit())
   {
      switch(cgetc())
      {
         case 'w': if (ship.dy > -MAX_V) ship.dy--; break;
         case 's': if (ship.dy <  MAX_V) ship.dy++; break;
         case 'a': if (ship.dx > -MAX_V) ship.dx--; break;
         case 'd': if (ship.dx <  MAX_V) ship.dx++; break;
      }
   }

}

void main()
{
   unsigned char done = 0;

   cbm_k_bsout(CH_FONT_UPPER);

   bgcolor(0); // lets background layer in
   clrscr();

   // load sprites to vera
   cbm_k_setnam("sw-sprites.bin");
   cbm_k_setlfs(0,8,0);
   cbm_k_load(LOAD_TO_VERA, SPRITE_ADDR_BEGIN);

   vera_sprites_enable(1); // cx16.h 
   initSprite();

   while(!done)
   {
      move_ship();
      move_asteroid();
   }
}
