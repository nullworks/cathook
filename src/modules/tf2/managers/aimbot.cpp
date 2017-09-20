 
/*
 *
 *	Here exists functions for the aimbot framework to use
 *		Keep it tidy please!
 *			-onee
 *
 *
 */


/*bool IsTargetStateGood(CachedEntity* entity) {
	
	// Checks for Players
	if (entity->m_Type == ENTITY_PLAYER) {
		// Local player check
		if (entity == LOCAL_E) return false;
		// Dead
		if (!entity->m_bAlivePlayer) return false;
		// Teammates
		if ((int)teammates != 2 && ((!entity->m_bEnemy && !teammates) || (entity->m_bEnemy && teammates))) return false;
		// Distance
		if (EffectiveTargetingRange()) {
			if (entity->m_flDistance > EffectiveTargetingRange()) return false;
		}
		
			// Wait for charge
			if (wait_for_charge && g_pLocalPlayer->holding_sniper_rifle) {
				float cdmg = CE_FLOAT(LOCAL_W, netvar.flChargedDamage) * 3;
				bool maxCharge = cdmg >= 450.0F;

				// Darwins damage correction, Darwins protects against 15% of damage
				if (HasDarwins(entity))
				  cdmg = (cdmg * .85) - 1;
				// Vaccinator damage correction, Vac charge protects against 75% of damage
				if (HasCondition<TFCond_UberBulletResist>(entity)) {
				  cdmg = (cdmg * .25) - 1;
				// Passive bullet resist protects against 10% of damage
				} else if (HasCondition<TFCond_SmallBulletResist>(entity)) {
				  cdmg = (cdmg * .90) - 1;
				}
				// Invis damage correction, Invis spies get protection from 10% of damage
				if (IsPlayerInvisible(entity))
				  cdmg = (cdmg * .80) - 1;
				
				// Check if player will die from headshot or if target has more than 450 health and sniper has max chage
				if ( !(entity->m_iHealth <= 150.0F || entity->m_iHealth <= cdmg || !g_pLocalPlayer->bZoomed || (maxCharge && entity->m_iHealth > 450.0F)) ) {
				  return false;
				}
			}
			
			// Taunting
			if (ignore_taunting && HasCondition<TFCond_Taunting>(entity)) return false;
			// Invulnerable players, ex: uber, bonk
			if (IsPlayerInvulnerable(entity)) return false;
			// cloaked/deadringed players
			if (ignore_cloak || ignore_deadringer) {
				if (IsPlayerInvisible(entity)) {
					// Item id for deadringer is 59 as of time of creation
					if (HasWeapon(entity, 59)) {
						if (ignore_deadringer) return false;
					} else {
						if (ignore_cloak) return false;
					}
				}
			}
			// Vaccinator
			if (g_pLocalPlayer->weapon_mode == weaponmode::weapon_hitscan || LOCAL_W->m_iClassID == CL_CLASS(CTFCompoundBow))
				if (ignore_vaccinator && HasCondition<TFCond_UberBulletResist>(entity)) return false;
		}
		
		return true;
		
	// Check for buildings
	} else if (entity->m_Type == ENTITY_BUILDING) {
		
		// Enabled check
		if ( !(buildings_other || buildings_sentry) ) return false;
		// Teammates, Even with friendly fire enabled, buildings can NOT be damaged
		if (!entity->m_bEnemy) return false;
		// Distance
		if (EffectiveTargetingRange()) {
			if (entity->m_flDistance > (int)EffectiveTargetingRange()) return false;
		}
		
		// Building type
		if (!(buildings_other && buildings_sentry)) {
			// Check if target is a sentrygun
			if (entity->m_iClassID == CL_CLASS(CObjectSentrygun)) {
				if (!buildings_sentry) return false;
			// Other
			} else {
				if (!buildings_other) return false;
			}			
		}
	
		// Grab the prediction var
		AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
				
		// Vis and fov checks
		if (!VischeckPredictedEntity(entity)) return false;
		if ((float)fov > 0.0f && cd.fov > (float)fov) return false;
		
		return true;
	
	// Check for stickybombs
	} else if (entity->m_iClassID == CL_CLASS(CTFGrenadePipebombProjectile)) {
		// Enabled
		if (!stickybot) return false;
		
		// Only hitscan weapons can break stickys so check for them.
		if (!(GetWeaponMode() == weapon_hitscan || GetWeaponMode() == weapon_melee)) return false;

		// Distance
		if (EffectiveTargetingRange()) {
			if (entity->m_flDistance > (int)EffectiveTargetingRange()) return false;
		}
		
		// Teammates, Even with friendly fire enabled, stickys can NOT be destroied
		if (!entity->m_bEnemy) return false;
		
		// Check if target is a pipe bomb
		if (CE_INT(entity, netvar.iPipeType) != 1) return false;
		
		// Moving Sticky?
		if (!CE_VECTOR(entity, netvar.vVelocity).IsZero(1.0f)) return false;
		
		// Grab the prediction var
		AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
				
		// Vis and fov check
		if (!VischeckPredictedEntity(entity)) return false;
		if ((float)fov > 0.0f && cd.fov > (float)fov) return false;
			
		return true;
		
	} else {
		// Target not valid
		return false;
	}
	// An impossible error so just return false
	return false;
}*/