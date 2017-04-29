/*
 * Background.cpp
 *
 *  Created on: Apr 28, 2017
 *      Author: nullifiedcat
 */

#include "Menu.hpp"

namespace menu { namespace ncc {

Background::Background() : CBaseWidget("nc_background"),
	tx_snowflake(&_binary_snowflake_start, 16, 16),
	tx_raindrop(&_binary_raindrop_start, 16, 16),
	tx_flame(&_binary_flame_start, 16, 16),
	tx_heart(&_binary_heart_start, 16, 16),
	tx_raindrop2(&_binary_raindrop2_start, 16, 16) {
	SetSize(draw::width, draw::height);
}

static CatVar particles(CV_SWITCH, "gui_bg_particles", "0", "Particles");
static CatEnum particle_type_enum({"Snowflake", "Raindrop", "Flame", "Heart", "Raindrop 2", "Random"});
static CatVar particle_type(particle_type_enum, "gui_bg_particles_type", "0", "Particles Type", "Defines particle type");
static CatVar particle_chance(CV_INT, "gui_bg_particles_chance", "10", "Particles Spawn Rate", "Defines snowflake spawn rate (HAS TO BE NONZERO!)", 1.0f, 100.0f);
static CatVar particle_pack_size(CV_INT, "gui_bg_particles_pack_size", "10", "Particles Max Pack", "Defines max snowflake spawn pack size (HAS TO BE NONZERO!)", 1.0f, 100.0f);
static CatVar particle_safe(CV_INT, "gui_bg_particles_safe_zone", "100", "Particles Safe Zone", "Defines snowflake safe zone (they will decay after reaching that point)", 0.0f, 400.0f);
static CatVar particle_gravity(CV_FLOAT, "gui_bg_particles_gravity", "700", "Particles Gravity", "Defines snowflake gravity (HAS TO BE NONZERO!)", 0.01f, 2400.0f);
static CatVar particle_jittering(CV_INT, "gui_bg_particles_jittering", "2", "Particles Jittering", "Defines snowflake jittering amount", 0.0f, 10.0f);
static CatVar particle_wind(CV_INT, "gui_bg_particles_wind", "0", "Particles Wind", "Wind strength and direction", -500.0f, 500.0f);
static CatVar particle_jittering_chance(CV_INT, "gui_bg_particles_jittering_chance", "60", "Snowflake Jittering Rate", "Defines snowflake jittering rate (HAS TO BE NONZERO!)", 1.0f, 20.0f);
static CatEnum background_visible_enum({"NEVER", "MENU", "ALWAYS"});
static CatVar background_visible(background_visible_enum, "gui_bg_visible", "1", "Render background", "Render background when");

bool Background::AlwaysVisible() {
	return (int)background_visible == 2;
}

void Background::Update() {
	if (!particles) return;
	auto newtime = std::chrono::system_clock::now();
	std::chrono::duration<float> dt = newtime - last_update;
	Particle* current = list;
	while (current) {
		Particle* next = current->next;
		current->Update(dt.count());
		if (current->dead) {
			KillParticle(current);
		}
		current = next;
	}
	if (!(rand() % (int)particle_chance)) {
		for (int i = 0; i < rand() % (int)particle_pack_size; i++) MakeParticle();
	}
	last_update = newtime;
}

Background::~Background() {
	Particle* current = list;
	while (current) {
		Particle* next = current->next;
		delete current;
		current = next;
	}
}

void Background::LoadTextures() {
	tx_flame.Load();
	tx_raindrop.Load();
	tx_snowflake.Load();
	tx_heart.Load();
	tx_raindrop2.Load();
	textures_loaded = true;
}

void Background::Draw(int x, int y) {
	if (!particles) return;
	if (!textures_loaded) LoadTextures();
	Particle* current = list;
	while (current) {
		Particle* next = current->next;
		if (!current->show_in) {
			int color = colors::white;
			if (current->y > (int)particle_safe) {
				color = colors::Create(255, 255, 255, ((int)particle_safe + 255) - current->y);
			}
			current->texture->Draw((int)current->x, (int)current->y, 16, 16, color);
		}
		current = next;
	}
}

void Background::MakeParticle() {
	Particle* flake = new Particle();
	flake->x = RandFloatRange(0, draw::width);
	flake->y = 3 - (rand() % 6);
	flake->vx = 0;
	flake->vy = 0;
	flake->dead = false;
	flake->next = nullptr;
	flake->show_in = rand() % 4;
	switch (((int)particle_type == 5) ? (rand() % 5) : (int)particle_type) {
	case 1:
		flake->texture = &tx_raindrop;
		break;
	case 2:
		flake->texture = &tx_flame;
		break;
	case 3:
		flake->texture = &tx_heart;
		break;
	case 4:
		flake->texture = &tx_raindrop2;
		break;
	default:
		flake->texture = &tx_snowflake;
	}
	if (list_tail)
		list_tail->next = flake;
	flake->prev = list_tail;
	list_tail = flake;
	if (!list) {
		list = flake;
	}
}

void Background::KillParticle(Particle* flake) {
	if (list_tail == flake) {
		list_tail = flake->prev;
	}
	if (list == flake) {
		list = flake->next;
	}
	if (flake->prev) flake->prev->next = flake->next;
	if (flake->next) flake->next->prev = flake->prev;
	delete flake;
}

void Background::Particle::Update(float dt) {
	if (show_in) show_in--;
	if (particle_wind) {
		vx += (float)particle_wind * dt * 10.0f;
	}
	if (!(rand() % (int)(particle_jittering_chance))) {
		x += (rand() % 2) ? (int)particle_jittering : -(int)particle_jittering;
	}
	vy += (float)particle_gravity * dt;
	x += vx * dt;
	y += vy * dt;
	if (x > draw::width + 16) x = -16;
	if (y > (int)particle_safe + 255) dead = true;
}

}}
