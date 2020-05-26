/*
 * trace.h
 *
 *  Created on: Oct 10, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <engine/IEngineTrace.h>

// This file is a mess. I need to fix it. TODO

class IClientEntity;

namespace trace
{

class FilterDefault : public ITraceFilter
{
public:
    IClientEntity *m_pSelf;

public:
    virtual ~FilterDefault();
    FilterDefault();
    virtual bool ShouldHitEntity(IHandleEntity *entity, int mask);
    void SetSelf(IClientEntity *self);
    virtual TraceType_t GetTraceType() const;
};

class FilterNoPlayer : public ITraceFilter
{
public:
    IClientEntity *m_pSelf;

public:
    virtual ~FilterNoPlayer();
    FilterNoPlayer();
    virtual bool ShouldHitEntity(IHandleEntity *entity, int mask);
    void SetSelf(IClientEntity *self);
    virtual TraceType_t GetTraceType() const;
};
class FilterNavigation : public ITraceFilter
{

public:
    virtual ~FilterNavigation();
    FilterNavigation();
    virtual bool ShouldHitEntity(IHandleEntity *entity, int mask);
    virtual TraceType_t GetTraceType() const;
};

class FilterNoEntity : public ITraceFilter
{
public:
    IClientEntity *m_pSelf;

public:
    virtual ~FilterNoEntity();
    FilterNoEntity();
    virtual bool ShouldHitEntity(IHandleEntity *entity, int mask);
    void SetSelf(IClientEntity *self);
    virtual TraceType_t GetTraceType() const;
};

class FilterPenetration : public ITraceFilter
{
public:
    IClientEntity *m_pSelf;
    IClientEntity *m_pIgnoreFirst;

public:
    virtual ~FilterPenetration();
    FilterPenetration();
    virtual bool ShouldHitEntity(IHandleEntity *entity, int mask);
    void SetSelf(IClientEntity *self);
    void Reset();
    virtual TraceType_t GetTraceType() const;
};

extern FilterDefault filter_default;
extern FilterNoPlayer filter_no_player;
extern FilterNavigation filter_navigation;
extern FilterNoEntity filter_no_entity;
extern FilterPenetration filter_penetration;
} // namespace trace
