#pragma once
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <math.h>
#include <algorithm>
#include "GoreEngine.h"
#define P2 1.5708
#define P3 4.71239
#undef main
SDL_Renderer* rend;
Gore gore;
bool push = false;
bool atk = false;
bool enemonscreen = false;
SDL_Surface* surf;
SDL_Surface* wallsurf;
SDL_Surface* enemsurf;
SDL_Texture* enemtex;
float zbuf[800];
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
struct Sprite {
	int x;
	int y;
	int w;
	int h;
	float distance;
	int health;
	SDL_Surface* surf;
	double ftime;
	double time;
	int frames;
};
struct Animate {
	int x;
	int y;
	int w;
	int h;
	double time;
	double ftime;
	Sprite* targ;
	texp sprites;

};
std::vector<Sprite> sprites;
Point camera = { 22, 3 };
Point dir = { -1, 0 };
Point plane = { 0, 0.66 };
Point pushvec = { 0, 0 };

struct {
	bool operator()(Sprite a, Sprite b) const { return a.distance > b.distance; }
} spriteDistanceSort;


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
	1,0,0,3,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,1,0,0,0,0,2,0,0,0,1,
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
		if (x < 600 && x > 300 && perpwalldist <= 1) {
			lh = 400;
			p->x += pushvec.x;
			p->y += pushvec.y;
			pushvec.x = 0;
			pushvec.y = 0;
		}
		else {
			lh = (int)800 / perpwalldist;
		}
		int drawstart = -lh / 2 + 800 / 2;
		int drawend = lh / 2 + 800 / 2;
		if (map[mp.x][mp.y] == 3) {
			SDL_Surface* cursurf = nullptr;
			switch (map[mp.x][mp.y]) {
			case 3:
				cursurf = wallsurf;
				break;
			}
			int pitch = 200;
			float wallX = 0;
			if (sidetype == 0) {
				wallX = p->y + perpwalldist * rayDir.y;
			}
			else {
				wallX = p->x + perpwalldist * rayDir.x;
			}
			//remove this for extremely glitched out textures
			wallX -= (int)wallX;
			//50 is texture width
			int texX = int(wallX * 50.0);
			if (sidetype == 0 && rayDir.x > 0) texX = 50 - texX - 1;
			if (sidetype == 1 && rayDir.y < 0) texX = 50 - texX - 1;
			float step = 1.0 * 50 / lh;
			float texPos = (drawstart - pitch - 50 / 2 + lh / 2) * step;
			for (int y = drawstart; y < drawend; y++) {
				int texY = (int)texPos & (50 - 1);
				texPos += step;
				Uint32 color = gore.GetPixelSurface(cursurf, &texY, &texX);
				if (y >= 0 && y <= 800 && x >= 0 && x <= 800) {
					gore.SetPixelSurface(surf, &y, &x, &color);
				}
			}
		}
		else {
			SDL_SetRenderDrawColor(rend, col, 100, 100, 0);
			SDL_RenderDrawLine(rend, x, drawstart, x, drawend);
		}
		zbuf[x] = perpwalldist;
	}
}


//https://github.com/ssloy/tinyraycaster/wiki/Part-1:-crude-3D-renderings
//https://www.youtube.com/watch?v=gYRrGTC7GtA
//https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/
//https://lodev.org/cgtutor/raycasting.html
//https://www.youtube.com/watch?v=NbSee-XM7WA&list=WL&index=1
//Add combat and health
//Add strength and agility stats
//Strength increases attack power and agility increases attack speed
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
	texp atkhead = gore.loadTextureList({ "atk5.png", "atk4.png", "atk3.png", "atk2.png", "atk1.png" }, { 400, 400, 400, 400, 400 }, { 400, 400, 400, 400, 400 }
	, SDL_PIXELFORMAT_RGBA8888, rend, "Sprites/");
	surf = SDL_CreateRGBSurfaceWithFormat(0, 800, 800, 32, SDL_PIXELFORMAT_RGBA8888);
	wallsurf = gore.loadPNG("Sprites/wall1.png", SDL_PIXELFORMAT_RGBA8888, 50, 50);
	enemsurf = gore.loadPNG("Sprites/enemy1.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	SDL_Surface* enemsurf2 = gore.loadPNG("Sprites/enemy1_2.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	SDL_Surface* enemsurf3 = gore.loadPNG("Sprites/enemy1_3.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	enemtex = SDL_CreateTextureFromSurface(rend, enemsurf);
	Entity p = {40, 50};
	p.dir = 0;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	SDL_Event e;
	std::vector<Animate> animates;
	double delta;
	double btimer = 0, atktimer = 0, atkcool = 1.0;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (map[i][j] == 2) {
				map[i][j] = 0;
				Sprite s;
				s.x = (i*16) - 16;
				s.y = j*16;
				s.surf = enemsurf;
				s.w = 300;
				s.h = 400;
				s.health = 100;
				s.frames = 3;
				s.distance = 0;
				sprites.push_back(s);
			}
		}
	}
	Sprite s;
	s.x = 80;
	s.y = 121;
	s.surf = enemsurf;
	s.w = 300;
	s.h = 400;
	s.frames = 3;
	s.health = 100;
	s.distance = 4;
	sprites.push_back(s);
	std::sort(sprites.begin(), sprites.end(), spriteDistanceSort);
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
		if (btimer > 0.1) {
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
				Sprite* sp = nullptr;
				bool skip = true;
				for (auto& i : sprites) {
					if (i.distance < 1.2) {
						skip = false;
						sp = &i;
					}
				}
				if (!skip) {
					atk = true;
					//move this to raycasting part
					Animate a;
					a.x = 100;
					a.y = 100;
					a.w = 400;
					a.h = 400;
					a.time = 0;
					a.ftime = 0.05;
					a.sprites = atkhead;
					a.targ = sp;
					animates.push_back(a);
					atktimer = 0;
				}
			}
		}
		

		SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
		SDL_RenderClear(rend);
		raycastDDA(&p, rend);
		int n = 0;
		for (auto& i : sprites) {
			i.distance = (sqrt(((p.x - i.x) * (p.x - i.x) + (p.y - i.y) * (p.y - i.y)))) / 16;
			//std::cout << i.distance << std::endl;
			
			if (i.health <= 0) {
				sprites.erase(sprites.begin() + n);
			}
			n++;
		}
		std::sort(sprites.begin(), sprites.end(), spriteDistanceSort);
		//std::cout << "X: " << p.x << " Y: " << p.y << std::endl;
		enemonscreen = false;
		for (int i = 0; i < sprites.size(); i++) {
			float spx = sprites[i].x - p.x;
			float spy = sprites[i].y - p.y;
			float invDet = 1.0 / (plane.x * dir.y - dir.x * plane.y);
			float tfx = invDet * (dir.y * spx - dir.x * spy);
			float tfy = invDet * (-plane.y * spx + plane.x * spy);

			int spritescreenx = int((800 / 2) * (1 + tfx / tfy));
			int spriteheight = std::abs(int(10000 / tfy));

			int drawstarty = -spriteheight / 2 + 800 / 2;
			if (drawstarty < 0) drawstarty = 0;
			int drawEndY = spriteheight / 2 + 800 / 2;
			if (drawEndY >= 800) drawEndY = 800 - 1;
			
			int spriteWidth = std::abs(int(10000 / (tfy)));
			
			int drawStartX = -spriteWidth / 2 + spritescreenx;
			if (drawStartX < 0) drawStartX = 0;
			int drawEndX = spriteWidth / 2 + spritescreenx;
			if (drawEndX >= 800) drawEndX = 800 - 1;

			int n = 1;
			int cy = 0;
			for (int stripe = drawStartX; stripe < drawEndX; stripe++, n++)
			{
				int texX = int(256 * (stripe - (-spriteWidth / 2 + spritescreenx)) * 300 / spriteWidth) / 256;
				if (tfy > 0 && stripe > 0 && stripe < 800 && sprites[i].distance < zbuf[stripe])
					for (int y = drawstarty; y < drawEndY; y++) //for every pixel of the current stripe
					{
						int d = (y) * 256 - 800 * 128 + spriteheight * 128; //256 and 128 factors to avoid floats
						int texY = ((d * 400) / spriteheight) / 256;
						if (texY >= 0 && texY <= 400 && texX >= 0 && texX <= 300) {
							Uint32 color = gore.GetPixelSurface(enemsurf, &texY, &texX);
							gore.SetPixelSurface(surf, &y, &stripe, &color);
							enemonscreen = true;
						}
					}
			}

		}
		SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
		SDL_Rect frect = { 0, 0, 800, 800 };
		SDL_RenderCopy(rend, tex, NULL, &frect);
		SDL_DestroyTexture(tex);
		gore.clearSurface(surf);
		int j = 0;
		for(auto& i : animates){
			i.time += delta;
			if (i.time > i.ftime) {
				i.sprites = i.sprites->next;
				i.time = 0;
			}
			if (i.sprites == NULL) {
				animates.erase(animates.begin() + j);
				i.targ->health -= 10;
				std::cout << i.targ->health << std::endl;
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