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
SDL_Surface* surf;
SDL_Surface* wallsurf;
SDL_Surface* enemsurf;
SDL_Texture* enemtex;
float zbuf[800];
struct Entity {
	float x;
	float y;
	int health;
	int strength;
	int agility;
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
	int f;
	bool forward;
	bool dead;
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
void constructAlphabet(SDL_Renderer* rend, TTF_Font* font, SDL_Color color, texp& head) {
	for (int i = 33; i < 123; i++) {
		char c = i;
		std::string co;
		co.push_back(c);
		SDL_Surface* surf = TTF_RenderText_Solid(font, co.c_str(), color);
		SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
		SDL_FreeSurface(surf);
		gore.insertTex(head, tex, co);
		//std::cout << co << std::endl;
	}
}
void drawText(SDL_Renderer* rend, texp& head, std::string text, int x, int y, int w, int h, int mw) {
	int sx = x;
	int sy = y;
	int base = 0;
	for (auto& i : text) {
		std::string t;
		t.push_back(i);
		SDL_Rect pos = { sx, sy, w, h };
		SDL_RenderCopy(rend, gore.findTex(head, t.c_str()), NULL, &pos);
		sx += w + 1;
		base += w + 1;
		if (base > mw) {
			sx = x;
			sy += h + 1;
			base = 0;
		}
	}
}


//https://github.com/ssloy/tinyraycaster/wiki/Part-1:-crude-3D-renderings
//https://www.youtube.com/watch?v=gYRrGTC7GtA
//https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/
//https://lodev.org/cgtutor/raycasting.html
//https://www.youtube.com/watch?v=NbSee-XM7WA&list=WL&index=1
//Add level generation
//Add story for between levels 1-10
//Add enemy death animation
//Add sound effects
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
	TTF_Font* font = TTF_OpenFont("DelaGothicOne-Regular.ttf", 12);
	texp texthead = NULL;
	constructAlphabet(rend, font, {150, 200, 250}, texthead);
	bool exitf = false;
	texp atkhead = gore.loadTextureList({ "atk5.png", "atk4.png", "atk3.png", "atk2.png", "atk1.png" }, { 400, 400, 400, 400, 400 }, { 400, 400, 400, 400, 400 }
	, SDL_PIXELFORMAT_RGBA8888, rend, "Sprites/");
	surf = SDL_CreateRGBSurfaceWithFormat(0, 800, 800, 32, SDL_PIXELFORMAT_RGBA8888);
	wallsurf = gore.loadPNG("Sprites/wall1.png", SDL_PIXELFORMAT_RGBA8888, 50, 50);
	enemsurf = gore.loadPNG("Sprites/enemy1.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	SDL_Surface* enemsurf2 = gore.loadPNG("Sprites/enemy1_2.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	SDL_Surface* enemsurf3 = gore.loadPNG("Sprites/enemy1_3.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	SDL_Surface* deadsurf = gore.loadPNG("Sprites/deadenemy.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	enemtex = SDL_CreateTextureFromSurface(rend, enemsurf);
	Entity p = {40, 50, 100};
	p.strength = 1;
	p.agility = 1;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	SDL_Event e;
	std::vector<Animate> animates;
	double delta;
	double btimer = 0, atktimer = 0, atkcool = 1.0;
	bool upgrade = false;
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
				s.f = 1;
				s.dead = false;
				s.ftime = 0.2;
				s.forward = true;
				s.time = 0;
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
	s.dead = false;
	s.frames = 3;
	s.f = 1;
	s.ftime = 0.2;
	s.forward = true;
	s.time = 0;
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
		if (keys[SDL_SCANCODE_W] && !upgrade) {
			p.x += dir.x * delta * 50;
			p.y += dir.y * delta * 50;
			pushvec.x = -(dir.x * delta * 50);
			pushvec.y = -(dir.y * delta * 50);
		}
		else if (keys[SDL_SCANCODE_S] && !upgrade) {
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
			if (keys[SDL_SCANCODE_RIGHT] && !upgrade) {
				float olddirx = dir.x;
				dir.x = dir.x * cos(-delta * 15) - dir.y * sin(-delta * 15);
				dir.y = olddirx * sin(-delta * 15) + dir.y * cos(-delta * 15);
				double oldPlaneX = plane.x;
				plane.x = plane.x * cos(-delta * 15) - plane.y * sin(-delta * 15);
				plane.y = oldPlaneX * sin(-delta * 15) + plane.y * cos(-delta * 15);
				btimer = 0;
			}
			else if (keys[SDL_SCANCODE_LEFT] && !upgrade) {
				double olddirx = dir.x;
				dir.x = dir.x * cos(delta * 15) - dir.y * sin(delta * 15);
				dir.y = olddirx * sin(delta * 15) + dir.y * cos(delta * 15);
				double oldPlaneX = plane.x;
				plane.x = plane.x * cos(delta * 15) - plane.y * sin(delta * 15);
				plane.y = oldPlaneX * sin(delta * 15) + plane.y * cos(delta * 15);
				btimer = 0;
			}
		}
		Sprite* range = nullptr;
		bool drawehealth = false;
		if (atktimer > atkcool) {
			int mx, my;
			if (keys[SDL_SCANCODE_Q] && !upgrade || SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT) && !upgrade) {
				Sprite* sp = nullptr;
				for (auto& i : sprites) {
					if (i.distance < 1.2) {
						sp = &i;
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
			else if (keys[SDL_SCANCODE_E]) {
				for (auto& i : sprites) {
					if (i.distance < 1.2 && i.dead) {
						range = &i;
						upgrade = true;
					}
				}
			}
		}
		

		SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
		SDL_RenderClear(rend);
		raycastDDA(&p, rend);
		
		if(!upgrade) {
			int n = 0;
			for (auto& i : sprites) {
				i.distance = (sqrt(((p.x - i.x) * (p.x - i.x) + (p.y - i.y) * (p.y - i.y)))) / 16;
				//std::cout << i.distance << std::endl;
				if (!i.dead) {
					if (i.distance <= 1.2) {
						range = &i;
						drawehealth = true;
						i.time += delta;
						if (i.time > i.ftime) {
							if (i.forward) {
								i.f++;
								if (i.f > i.frames) {
									i.f = i.frames;
									i.forward = false;
								}
							}
							else {
								i.f--;
								if (i.f < 1) {
									i.f = 1;
									p.health -= 5;
									i.forward = true;
								}
							}
							switch (i.f) {
							case 1:
								i.surf = enemsurf;
								break;
							case 2:
								i.surf = enemsurf2;
								break;
							case 3:
								i.surf = enemsurf3;
								break;
							}
							i.time = 0;
						}
					}
					else {
						i.surf = enemsurf;
					}
					if (i.health <= 0) {
						i.health = 1;
						i.dead = true;
					}
				}
				else {
					i.surf = deadsurf;
				}
				n++;
			}
		}
		std::sort(sprites.begin(), sprites.end(), spriteDistanceSort);
		//std::cout << "X: " << p.x << " Y: " << p.y << std::endl;
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
							Uint32 color = gore.GetPixelSurface(sprites[i].surf, &texY, &texX);
							gore.SetPixelSurface(surf, &y, &stripe, &color);
						}
					}
			}

		}
		SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
		SDL_Rect frect = { 0, 0, 800, 800 };
		SDL_RenderCopy(rend, tex, NULL, &frect);
		SDL_DestroyTexture(tex);
		gore.clearSurface(surf);
		if (upgrade) {
			SDL_SetRenderDrawColor(rend, 150, 255, 50, 0);
			SDL_Rect rect = { 200, 200, 270, 220 };
			SDL_RenderFillRect(rend, &rect);
			//Button to upgrade strength
			//Button upgrade agility
			SDL_SetRenderDrawColor(rend, 255, 50, 150, 0);
			SDL_Rect frect = { 210, 210, 250, 100 };
			SDL_RenderFillRect(rend, &frect);
			drawText(rend, texthead, "Upgrade Strength", 220, 220, 26, 28, 200);
			SDL_SetRenderDrawColor(rend, 100, 50, 250, 0);
			SDL_Rect erect = { 210, 310, 250, 100 };
			SDL_RenderFillRect(rend, &erect);
			drawText(rend, texthead, "Upgrade Agility", 220, 320, 26, 28, 200);
			int mx, my;
			if(SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT)) {
				if (mx < 210 + 250 && mx + 1 > 210 && my < 210 + 100 && my + 1 > 210) {
					p.strength++;
					upgrade = false;
					p.health += 25;
					atktimer = 0;
				}
				else if (mx < 210 + 250 && mx + 1 > 210 && my < 310 + 100 && my + 1 > 310) {
					p.agility++;
					atkcool -= 0.05;
					upgrade = false;
					p.health += 25;
					atktimer = 0;
				}
			}

		}
		std::string temp = "Health: " + std::to_string(p.health);
		drawText(rend, texthead, temp, 10, 10, 20, 20, 500);
		if (drawehealth) {
			std::string temp2 = "Enemy Health: " + std::to_string(range->health);
			drawText(rend, texthead, temp2, 10, 60, 20, 20, 500);
		}
		std::string temp3 = "Strength: " + std::to_string(p.strength);
		drawText(rend, texthead, temp3, 250, 10, 20, 20, 500);
		std::string temp4 = "Agility: " + std::to_string(p.agility);
		drawText(rend, texthead, temp4, 250, 30, 20, 20, 500);
		int j = 0;
		for(auto& i : animates){
			i.time += delta;
			if (i.time > i.ftime) {
				i.sprites = i.sprites->next;
				i.time = 0;
			}
			if (i.sprites == NULL) {
				animates.erase(animates.begin() + j);
				i.targ->health -= 5 * p.strength;
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