#pragma once
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <math.h>
#include "GoreEngine.h"
#define P2 M_PI/2
#define P3 3*M_PI/2
#undef main
SDL_Renderer* rend;
Gore gore;

struct Entity {
	float x;
	float y;
	float dy;
	float dx;
	float angle;
};
int FixAng(int a) { if (a > 6.28319) { a -= 6.28319; } if (a < 0) { a += 6.28319; } return a; }
size_t mw = 16;
size_t mh = 16;
const char map[] = "1111111111111111"\
	"1000100000000001"\
	"1000100000000001"\
	"1000100111111111"\
	"1000000000000001"\
	"1000000000000001"\
	"1011111111111111"\
	"1010000000000001"\
	"1000000000000001"\
	"1111111000111111"\
	"1000000000000001"\
	"1011111000000001"\
	"1000001000000001"\
	"1000001000000001"\
	"1000001000000001"\
	"1111111111111111";

float dist(float ax, float ay, float bx, float by, float ang) {
	return cos((ang)*(bx-ax) - sin(ang) * (by-ay));
}


void castRays(Entity* p, SDL_Renderer* rend) {
	int r, mx, my, mp, dof; float rx, ry, ra, xo, yo;
	float retDist = 0;
	ra = p->angle - (0.0174533 * 30); if (ra < 0) { ra += 6.28319; }  if (ra > 6.28319) { ra -= 6.28319; }
	int lasto = 0;
	int off = 0;
	for (r = 0; r < 60; r++) {
		dof = 0;
		float disH = 10000000, hx = p->x, hy = p->y;
		float Ttan = (1.0 / tan(ra));
		if (ra > M_PI) { ry = (((int)p->y >> 6) << 6) - 0.0001; rx = (p->y - ry) * Ttan + p->x; yo = -64; xo = -yo * Ttan; }
		else if (ra < M_PI) { ry = (((int)p->y >> 6) << 6) + 64; rx = (p->y - ry) * Ttan + p->x; yo = 64; xo = -yo * Ttan; }
		else { rx = p->x; ry = p->y; dof = 8; }
		while (dof < 8) {
			mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mw + mx;
			if (mp > 0 && mp < mw * mh && map[mp] == '1') {
				hx = rx;
				hy = ry;
				disH = dist(p->x, p->y, hx, hy, ra);
				dof = 8;
			}
			else {
				rx += xo; ry += yo; dof++;
			}
		}
		dof = 0; 
		float disV = 10000000, vx = p->x, vy = p->y;
		float nTan = tan(ra);
		if (ra > P2 && ra < P3) { rx = (((int)p->x >> 6) << 6) - 0.0001; ry = (p->x - rx) * nTan + p->y; xo = 64; yo = -xo * nTan; }
		else if (ra < P2 || ra > P3) { rx = (((int)p->x >> 6) << 6) + 64; ry = (p->x - rx) * nTan + p->y; xo = -64; yo = -xo * nTan; }
		else { rx = p->x; ry = p->y; dof = 8; }
		while (dof < 8) {
			mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mw + mx;
			if (mp > 0 && mp < mw * mh && map[mp] == '1') {
				vx = rx;
				vy = ry;
				disV = dist(p->x, p->y, vx, vy, ra);
				dof = 8;
			}
			else {
				rx += xo; ry += yo; dof++;
			}
		}
		if (disV < disH) {
			retDist = disV;
			SDL_SetRenderDrawColor(rend, 255, 50, 255, 0);
			SDL_RenderDrawLine(rend, p->x, p->y, vx, vy);	
		}
		else {
			retDist = disH;
			SDL_SetRenderDrawColor(rend, 40, 50, 255, 0);
			SDL_RenderDrawLine(rend, p->x, p->y, hx, hy);
		}
		//Drawing walls
		int lh = (64 * 1024) / retDist;
		if (lh > 1024) { lh = 1024; }
		int lo = 512 - (lh>>1);
		SDL_SetRenderDrawColor(rend, 255, 50, 100, 0);
		SDL_Rect trect = { r * 17, lo, 17, lo + lh };
		
		//SDL_RenderDrawLine(rend, r * 17, lo, r * 17, lo + lh);
		SDL_RenderFillRect(rend, &trect);

		ra += 0.0174533; if (ra < 0) { ra += 6.28319; }  else if (ra > 6.28319) { ra -= 6.28319; }
	}
}

//https://github.com/ssloy/tinyraycaster/wiki/Part-1:-crude-3D-renderings
//https://lodev.org/cgtutor/raycasting.html
//https://www.youtube.com/watch?v=NbSee-XM7WA&list=WL&index=1
//Fix line segments not being drawn correctly, might be the horizontal rays passing through walls
//https://www.youtube.com/watch?v=gYRrGTC7GtA
//Add culling of fucked up rays, by distance from last ray checking
int main() {
	if (!SDL_Init(SDL_INIT_EVERYTHING)) {
		std::cout << "SDL failed to load" << std::endl;
	}
	if (TTF_Init()) {
		std::cout << "TTF failed to init" << std::endl;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		std::cout << "Mixer failed" << std::endl;
	}
	SDL_Window* wind = SDL_CreateWindow("Dungeon Crawler", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 1024, SDL_WINDOW_SHOWN);
	rend = SDL_CreateRenderer(wind, -1, 0);
	bool exitf = false;
	Entity p = {150.0, 200, 1.5708 };
	p.dx = cos(p.angle) * 5;
	p.dy = sin(p.angle) * 5;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	SDL_Event e;
	double delta;
	double btimer = 0;
	while (!exitf) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				exitf = true;
				break;
			}
		}
		delta = gore.getDelta();
		btimer += delta;
		SDL_PumpEvents();
		if (keys[SDL_SCANCODE_W]) {
			//p.y -= 100 * delta;
			p.x += p.dx * delta * 100; p.y += p.dy * delta * 100;
		}
		else if (keys[SDL_SCANCODE_S]) {
			//p.y += 100 * delta;
			p.x -= p.dx * delta * 100; p.y -= p.dy * delta * 100;
		}
		else if (keys[SDL_SCANCODE_A]) {
			p.x -= 100 * delta * p.dx;
		}
		else if (keys[SDL_SCANCODE_D]) {
			p.x += 100 * delta * p.dx;
		}
		if (btimer > 0.05) {
			if (keys[SDL_SCANCODE_RIGHT]) {
				p.angle -= 0.1;
				if (p.angle < 0) { p.angle += 2 * M_PI; }
				p.dx = cos(p.angle);
				p.dy = -sin(p.angle);
				btimer = 0;
			}
			else if (keys[SDL_SCANCODE_LEFT]) {
				p.angle += 0.1;
				if ((p.angle) > 2 * M_PI) { p.angle -= 2 * M_PI; }
				p.dx = cos(p.angle);
				p.dy = -sin(p.angle);
				btimer = 0;
			}
		}
		SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
		SDL_RenderClear(rend);
		/*for (size_t j = 0; j < mh; j++) {
			for (size_t i = 0; i < mw; i++) {
				if (map[(i) + (j) * mw] != '0') {
					size_t rx = i * 64;
					size_t ry = j * 64;
					SDL_Rect rect = { rx, ry, 64, 64 };
					SDL_SetRenderDrawColor(rend, 255, 50, 50, 0);
					SDL_RenderDrawRect(rend, &rect);
				}
			}
		}*/
		SDL_SetRenderDrawColor(rend, 50, 255, 50, 0);
		SDL_Rect prect = { p.x, p.y, 15, 15 };
		SDL_RenderFillRect(rend, &prect);
		castRays(&p, rend);
		SDL_SetRenderDrawColor(rend, 100, 100, 20, 0);
		SDL_RenderDrawLineF(rend, p.x, p.y, p.x - p.dx * 10, p.y - p.dy * 10);
		SDL_RenderPresent(rend);
	}
	TTF_Quit();
	Mix_CloseAudio();
	SDL_Quit();
	return 0;
}