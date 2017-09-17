/*
 * trace.cpp
 *
 *  Created on: Oct 10, 2016
 *      Author: nullifiedcat
 */

#include "trace.h"
#include "logging.h"

#include <cstdint>

#include "sdk.h"

// This file is a mess. I need to fix it. TODO

/* Default Filter */

trace::FilterDefault::FilterDefault() {
	m_pSelf = nullptr;
}

trace::FilterDefault::~FilterDefault() {}

void trace::FilterDefault::SetSelf(IClientEntity* self) {
	if (self == nullptr) {
		logging::Info("nullptr in FilterDefault::SetSelf");
	}
	m_pSelf = self;
}

bool trace::FilterDefault::ShouldHitEntity(IHandleEntity* handle, int mask) {
	IClientEntity *entity;
	ClientClass *clazz;

	if (!handle) return false;
	entity = (IClientEntity*) handle;
	clazz = entity->GetClientClass();
	/* Ignore invisible entities that we don't wanna hit */
	switch(clazz->m_ClassID) {
	// TODO magic numbers: invisible entity ids
	case 64:
	case 225:
	case 55:
		return false;
	}
	/* Do not hit yourself. Idiot. */
	if (entity == m_pSelf) return false;
	return true;
}

TraceType_t trace::FilterDefault::GetTraceType() const {
	return TRACE_EVERYTHING;
}

/* No-Player filter */

trace::FilterNoPlayer::FilterNoPlayer() {
	m_pSelf = nullptr;
}

trace::FilterNoPlayer::~FilterNoPlayer() {};

void trace::FilterNoPlayer::SetSelf(IClientEntity* self) {
	if (self == nullptr) {
		logging::Info("nullptr in FilterNoPlayer::SetSelf");
		return;
	}
	m_pSelf = self;
}

bool trace::FilterNoPlayer::ShouldHitEntity(IHandleEntity* handle, int mask) {
	IClientEntity *entity;
	ClientClass *clazz;

	if (!handle) return false;
	entity = (IClientEntity*) handle;
	clazz = entity->GetClientClass();
	/* Ignore invisible entities that we don't wanna hit */
	switch(clazz->m_ClassID) {
	// TODO magic numbers: invisible entity ids
	case 64:
	case 225:
	case 55:
	case 241:
		return false;
	}
	/* Do not hit yourself. Idiot. */
	if (entity == m_pSelf) return false;
	return true;
}

TraceType_t trace::FilterNoPlayer::GetTraceType() const {
	return TRACE_EVERYTHING;
}

/* Penetration Filter */

trace::FilterPenetration::FilterPenetration() {
	m_pSelf = nullptr;
}

trace::FilterPenetration::~FilterPenetration() {};

void trace::FilterPenetration::SetSelf(IClientEntity* self) {
	if (self == nullptr) {
		logging::Info("nullptr in FilterPenetration::SetSelf");
	}
	m_pSelf = self;
}

bool trace::FilterPenetration::ShouldHitEntity(IHandleEntity* handle, int mask) {
	IClientEntity *entity;
	ClientClass *clazz;

	if (!handle) return false;
	entity = (IClientEntity*) handle;
	clazz = entity->GetClientClass();
	/* Ignore invisible entities that we don't wanna hit */
	switch(clazz->m_ClassID) {
	// TODO magic numbers: invisible entity ids
	case 64:
	case 225:
	case 55:
		return false;
	case 241:
		if (!m_pIgnoreFirst && (entity != m_pSelf)) {
			m_pIgnoreFirst = entity;
		}
	}
	/* Do not hit yourself. Idiot. */
	if (entity == m_pSelf) return false;
	if (entity == m_pIgnoreFirst) return false;
	return true;
}

TraceType_t trace::FilterPenetration::GetTraceType() const {
	return TRACE_EVERYTHING;
}

void trace::FilterPenetration::Reset() {
	m_pIgnoreFirst = 0;
}

trace::FilterDefault trace::filter_default {};
trace::FilterNoPlayer trace::filter_no_player {};
trace::FilterPenetration trace::filter_penetration {};



