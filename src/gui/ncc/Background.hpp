/*
 * Background.hpp
 *
 *  Created on: Apr 28, 2017
 *      Author: nullifiedcat
 */

#ifndef BACKGROUND_HPP_
#define BACKGROUND_HPP_

#include "Menu.hpp"

extern unsigned char _binary_snowflake_start;
extern unsigned char _binary_flame_start;
extern unsigned char _binary_raindrop_start;
extern unsigned char _binary_raindrop2_start;
extern unsigned char _binary_heart_start;

namespace menu { namespace ncc {

class Background : public CBaseWidget {
public:
	struct Particle {
		float x, y;
		float vx, vy;
		int show_in { 0 };
		bool dead { false };
		Texture* texture { nullptr };
		Particle* next { nullptr };
		Particle* prev { nullptr };
		void Update(float dt);
	};
public:
	Background();
	~Background();
	virtual bool AlwaysVisible() override;
	virtual void Draw(int x, int y) override;
	virtual void Update() override;
	void LoadTextures();
	void MakeParticle();
	void KillParticle(Particle* flake);
public:
	// FIXME array or something
	bool textures_loaded { false };
	Texture tx_snowflake;
	Texture tx_raindrop;
	Texture tx_raindrop2;
	Texture tx_flame;
	Texture tx_heart;
	std::chrono::time_point<std::chrono::system_clock> last_update;
	Particle* list { nullptr };
	Particle* list_tail { nullptr };
};

}}

#endif /* BACKGROUND_HPP_ */
