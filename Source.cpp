#pragma once
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <math.h>
#include "GoreEngine.h"
#define P2 1.5708
#define P3 4.71239
#undef main
SDL_Renderer* rend;
Gore gore;
bool push = false;
bool atk = false;


struct Entity {
	float x;
	float y;
	int dir;
};
struct Point {
	float x;
	float y;
};
struct IntPoint {
	int x;
	int y;
};
struct Animate {
	int x;
	int y;
	int w;
	int h;
	double time;
	double ftime;
	texp sprites;
};
Point camera = { 22, 3 };
Point dir = { -1, 0 };
Point plane = { 0, 0.66 };
Point pushvec = { 0, 0 };

int FixAng(int a) { if (a > 6.28319) { a -= 6.28319; } if (a < 0) { a += 6.28319; } return a; }
size_t mw = 16;
size_t mh = 16;
int map[16][16] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,0,0,0,1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,};

float dist(float ax, float ay, float bx, float by, float ang) {
	return cos((ang)*(bx-ax) - sin(ang) * (by-ay));
}

void raycastDDA(Entity* p, SDL_Renderer* rend) {
	Point rayDir;
	for (int x = 0; x < 800; x++) {
		camera.x = 2 * x / float(800) - 1;
		rayDir.x = dir.x + plane.x * camera.x;
		rayDir.y = dir.y + plane.y * camera.x;
		Point sidedist;
		IntPoint mp = { (int)(p->x / 16), (int)(p->y / 16) };
		Point deltadist = { sqrt(1 + (rayDir.y * rayDir.y) / (rayDir.x * rayDir.x)), sqrt(1 + (rayDir.x * rayDir.x) / (rayDir.y * rayDir.y)) };
		float perpwalldist;
		IntPoint step = {0, 0};
		if (rayDir.x < 0) {
			step.x = -1;
			sidedist.x = (p->x/16 - mp.x) * deltadist.x;
		}
		else {
			step.x = 1;
			sidedist.x = (mp.x + 1 - p->x/16) * deltadist.x;
		}
		if (rayDir.y < 0) {
			step.y = -1;
			sidedist.y = (p->y/16 - mp.y) * deltadist.y;
		}
		else {
			step.y = 1;
			sidedist.y = (mp.y + 1 - p->y / 16) * deltadist.y;
		}
		bool hit = false;
		int sidetype = 0;
		float distance = 0;
		while (!hit) {
			if (sidedist.x < sidedist.y) {
				sidedist.x += deltadist.x;
				mp.x += step.x;
				sidetype = 0;
			}
			else {
				sidedist.y += deltadist.y;
				mp.y += step.y;
				sidetype = 1;
			}
			if (map[mp.x][mp.y] != 0) {
				hit = true;
				break;
			}
		}
		Uint8 col = 0;
		if (sidetype == 0) { perpwalldist = sidedist.x - deltadist.x; col = 160; }
		else if(sidetype == 1){ perpwalldist = sidedist.y - deltadist.y; col = 210; }
		int lh;
		if (sidetype == 2 && perpwalldist <= 2 && atk) {
			//sidetype 2 will be for enemy tiles
			std::cout << "Hit " << std::endl;
			atk = false;
		}
		if (perpwalldist <= 1) {
			lh = 400;
			p->x += pushvec.x;
			p->y += pushvec.y;
			pushvec.x = 0;
			pushvec.y = 0;
		}
		else {
			lh = (int)800 / perpwalldist;
		}
		SDL_SetRenderDrawColor(rend, col, 100, 100, 0);
		int drawstart = -lh / 2 + 800 / 2;
		int drawend = lh / 2 + 800 / 2;
		SDL_RenderDrawLine(rend, x, drawstart, x, drawend);
	}
	SDL_SetRenderDrawColor(rend, 100, 255, 100, 0);
	SDL_RenderDrawLine(rend, p->x, p->y, p->x + dir.x * 15, p->y + dir.y * 15);
}


//https://github.com/ssloy/tinyraycaster/wiki/Part-1:-crude-3D-renderings
//https://www.youtube.com/watch?v=gYRrGTC7GtA
//https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/
//https://lodev.org/cgtutor/raycasting.html
//https://www.youtube.com/watch?v=NbSee-XM7WA&list=WL&index=1
//get texture and sprites working
//Add combat and health
//Add strength and agility stats
// Strength increases attack power and agility increases attack speed
//Can increase stats by absorbing dead enemies
//Make enemies weird looking glitches
//Make enemies static, maybe
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
	SDL_Window* wind = SDL_CreateWindow("Dungeon Crawler", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);
	rend = SDL_CreateRenderer(wind, -1, 0);
	bool exitf = false;
	texp head = gore.loadTextureList({ "atk5.png", "atk4.png", "atk3.png", "atk2.png", "atk1.png" }, { 400, 400, 400, 400, 400 }, { 400, 400, 400, 400, 400 }
	, SDL_PIXELFORMAT_RGBA8888, rend, "Sprites/");
	Entity p = {20, 60};
	p.dir = 0;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	SDL_Event e;
	std::vector<Animate> animates;
	double delta;
	double btimer = 0, atktimer = 0, atkcool = 1.0;
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
		atktimer += delta;
		SDL_PumpEvents();
		if (keys[SDL_SCANCODE_W]) {
			p.x += dir.x * delta * 50;
			p.y += dir.y * delta * 50;
			pushvec.x = -(dir.x * delta * 50);
			pushvec.y = -(dir.y * delta * 50);
		}
		else if (keys[SDL_SCANCODE_S]) {
			p.x -= dir.x * delta * 50;
			p.y -= dir.y * delta * 50;
			pushvec.x = dir.x * delta * 50;
			pushvec.y = dir.y * delta * 50;
			
		}
		/*else if (keys[SDL_SCANCODE_A]) {
			p.x -= 100 * delta;
			p.nx--;
			btimer = 0;
		}
		else if (keys[SDL_SCANCODE_D]) {
			p.x += 100 * delta;
			p.nx++;
			btimer = 0;
		}*/
		if (btimer > 0.05) {
			if (keys[SDL_SCANCODE_RIGHT]) {
				float olddirx = dir.x;
				dir.x = dir.x * cos(-delta * 50) - dir.y * sin(-delta * 50);
				dir.y = olddirx * sin(-delta * 50) + dir.y * cos(-delta * 50);
				double oldPlaneX = plane.x;
				plane.x = plane.x * cos(-delta * 50) - plane.y * sin(-delta * 50);
				plane.y = oldPlaneX * sin(-delta * 50) + plane.y * cos(-delta * 50);
				btimer = 0;
			}
			else if (keys[SDL_SCANCODE_LEFT]) {
				double olddirx = dir.x;
				dir.x = dir.x * cos(delta * 50) - dir.y * sin(delta * 50);
				dir.y = olddirx * sin(delta * 50) + dir.y * cos(delta * 50);
				double oldPlaneX = plane.x;
				plane.x = plane.x * cos(delta * 50) - plane.y * sin(delta * 50);
				plane.y = oldPlaneX * sin(delta * 50) + plane.y * cos(delta * 50);
				btimer = 0;
			}
		}
		if (atktimer > atkcool) {
			int mx, my;
			if (keys[SDL_SCANCODE_Q] || SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				atk = true;
				//move this to raycasting part
				Animate a;
				a.x = 100;
				a.y = 100;
				a.w = 400;
				a.h = 400;
				a.time = 0;
				a.ftime = 0.05;
				a.sprites = head;
				animates.push_back(a);
				atktimer = 0;
			}
		}

		SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
		SDL_RenderClear(rend);
		//SDL_SetRenderDrawColor(rend, 100, 100, 20, 0);
		//SDL_RenderDrawLineF(rend, p.x, p.y, p.x - p.dx * 10, p.y - p.dy * 10);
		/*for (size_t j = 0; j < mh; j++) {
			for (size_t i = 0; i < mw; i++) {
				if (map[j][i] != 0) {
					size_t rx = i;
					size_t ry = j;
					SDL_Rect rect = { rx, ry, 1, 1 };
					SDL_SetRenderDrawColor(rend, 255, 50, 50, 0);
					SDL_RenderDrawRect(rend, &rect);
				}
			}
		}*/
		raycastDDA(&p, rend);
		int j = 0;
		for(auto& i : animates){
			i.time += delta;
			if (i.time > i.ftime) {
				i.sprites = i.sprites->next;
				i.time = 0;
			}
			if (i.sprites == NULL) {
				animates.erase(animates.begin() + j);
			}
			else {
				SDL_Rect rect = { i.x, i.y, i.w, i.h };
				SDL_RenderCopy(rend, i.sprites->current, NULL, &rect);
			}
			j++;
		}
		SDL_RenderPresent(rend);
	}
	TTF_Quit();
	Mix_CloseAudio();
	SDL_Quit();
	return 0;
}