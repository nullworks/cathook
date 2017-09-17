/*
 * profiler.h
 *
 *  Created on: Nov 26, 2016
 *      Author: nullifiedcat
 */

#ifndef PROFILER_H_
#define PROFILER_H_

#include "beforecheaders.h"
#include <chrono>
#include <string>
#include "aftercheaders.h"

class ProfilerNode;

class ProfilerSection {
public:
	ProfilerSection(std::string name, ProfilerSection* parent = nullptr);

	void OnNodeDeath(ProfilerNode& node);

	std::chrono::nanoseconds m_min;
	std::chrono::nanoseconds m_max;
	std::chrono::nanoseconds m_sum;
	unsigned m_spewcount;
	unsigned m_calls;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_log;
	std::string m_name;
	ProfilerSection* m_parent;
};

class ProfilerNode {
public:
	ProfilerNode(ProfilerSection& section);
	~ProfilerNode();

	std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
	ProfilerSection& m_section;
};

#define ENABLE_PROFILER true
#if ENABLE_PROFILER
#define PROF_SECTION(id) \
	static ProfilerSection __PROFILER__##id(#id); \
	ProfilerNode __PROFILER_NODE__##id(__PROFILER__##id);
#else
#define PROF_SECTION(id)
#endif

#endif /* PROFILER_H_ */
