

#ifndef _COMMON_H
#define _COMMON_H

#ifndef __MACH__
#include <numa.h>
#include <numaif.h>
#endif

// The author knows that this is really ugly...
// But what can we do? Lets hope our Macbook
// supporting NUMA soon!
#ifdef __MACH__
#include <math.h>
#include <stdlib.h>


#define numa_alloc_onnode(X,Y) malloc(X)

#define numa_max_node() 0

#define numa_run_on_node(X) 0

#define numa_set_localalloc() 0


#endif

#endif