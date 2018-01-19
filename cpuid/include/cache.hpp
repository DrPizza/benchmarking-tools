#ifndef CACHE_HPP
#define CACHE_HPP

#include "cpuid.hpp"

void print_cache_tlb_info(const cpu_t& cpu);

void enumerate_deterministic_cache(cpu_t& cpu);
void print_deterministic_cache(const cpu_t& cpu);

void print_l1_cache_tlb(const cpu_t& cpu);
void print_l2_cache_tlb(const cpu_t& cpu);

void print_1g_tlb(const cpu_t& cpu);

void enumerate_cache_properties(cpu_t& cpu);
void print_cache_properties(const cpu_t& cpu);

#endif
