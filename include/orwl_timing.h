/* This may look like nonsense, but it really is -*- mode: C -*-             */
/*                                                                           */
/* Except of parts copied from previous work and as explicitly stated below, */
/* the authors and copyright holders for this work are as follows:           */
/* all rights reserved,  2011 Emmanuel Jeanvoine, INRIA, France              */
/* all rights reserved,  2011 Jens Gustedt, INRIA, France                    */
/*                                                                           */
/* This file is part of the P99 project. You received this file as as        */
/* part of a confidential agreement and you may generally not                */
/* redistribute it and/or modify it, unless under the terms as given in      */
/* the file LICENSE.  It is distributed without any warranty; without        */
/* even the implied warranty of merchantability or fitness for a             */
/* particular purpose.                                                       */
/*                                                                           */
#ifndef   	ORWL_TIMING_H_
#define   	ORWL_TIMING_H_

#include "orwl_time.h"
#include "orwl_document.h"
#include "orwl_new.h"
#include "orwl_atomic.h"

P99_DECLARE_STRUCT(orwl_timing_element);

struct orwl_timing_element {
  atomic_size_t nb;
  atomic_float time;
  atomic_float time2;
  orwl_timing_element* next;
  char const*const name;
};

#define ORWL_TIMING_ELEMENT_INITIALIZER(NAME) { .nb = -1, .name = #NAME }

void orwl_timing_element_insert(orwl_timing_element* );

P99_DECLARE_STRUCT(orwl_timing);

/* This struct is used to declare timing elements that are supposed to
*  be used in inline functions */
struct orwl_timing {
  orwl_timing_element total_acquire;
};


orwl_timing * orwl_timing_info(void);

void orwl_timing_print_stats(void);

enum { orwl_timing_var = 0, orwl_timing_fetched = 0 };

/**
 ** @def ORWL_TIMING(NAME)
 ** @brief Timing of a statement or block
 **
 ** @remark Use this in inline functions that don't allow to declare
 ** @c static variables. For other places use ::ORWL_TIMER.
 **
 ** This macro is used as a prefix of a particular statement or block
 ** of code. @a NAME must correspond to one of the resources that are
 ** listed in the ::orwl_timing.
 **
 ** Care is taken that the function calls are optimized
 ** - ::orwl_timing_info is only called once per block. When timing is
 **   used again inside a block that is itself timed no new call to
 **   that function is issued
 ** - the two calls to ::orwl_gettime are as close as possible to the
 **   application code. I.e we have something like a sequence
 **   (0) ::orwl_timing_info()
 **   (1) ::orwl_gettime()
 **   (2) application code
 **   (3) ::orwl_gettime()
 **   (4) update of statistics
 **
 ** The update of the statistics are done with atomic operations such
 ** that all of this can be done without danger in a threaded
 ** environment.
 **/



#ifdef GETTIMING
# define ORWL_TIMING(NAME)                                                                        \
P00_BLK_START                                                                                     \
P00_BLK_DECL(register orwl_timing*const, _timing,                                                 \
             (orwl_timing_fetched ? orwl_timing_var : orwl_timing_info()))                        \
  P00_BLK_DECL(register orwl_timing*const, orwl_timing_var, _timing)                              \
  P00_BLK_DECL(register bool const, orwl_timing_fetched, true)                                    \
  P00_BLK_DECL(struct timespec, p00_end)                                                          \
  P00_BLK_DECL(struct timespec, p00_start, orwl_gettime())                                        \
  P00_BLK_DECL(atomic_float, p00_sec)                                                             \
  P00_BLK_AFTER(atomic_fetch_add(&(orwl_timing_var->NAME.nb), 1),                                 \
                atomic_fetch_atomic_float_add(&(orwl_timing_var->NAME.time2), p00_sec * p00_sec), \
                atomic_fetch_atomic_float_add(&(orwl_timing_var->NAME.time), p00_sec))            \
  P00_BLK_AFTER(p00_sec = timespec2seconds(timespec_diff(p00_start, p00_end)))                    \
  P00_BLK_AFTER(p00_end = orwl_gettime())                                                         \
  P00_BLK_END
#else
# define ORWL_TIMING(NAME)
#endif

/**
 ** @def ORWL_TIMER(NAME)
 ** @brief Timing of a statement or block
 **
 ** @param NAME should be token but no identifier is declared or
 ** supposed to exist with that name.
 **
 ** This macro is used as a prefix of a particular statement or block
 ** of code.
 **
 ** Care is taken that the function calls are optimized
 ** - A static initialization is only run the first time that this
 **   place is encountered.
 ** - the two calls to ::orwl_gettime are as close as possible to the
 **   application code. I.e we have something like a sequence
 **   (1) ::orwl_gettime()
 **   (2) application code
 **   (3) ::orwl_gettime()
 **   (4) update of statistics
 **
 ** The update of the statistics are done with atomic operations such
 ** that all of this can be done without danger in a threaded
 ** environment.
 **
 ** @see ORWL_TIMING when the context doesn't allow to declare @c
 ** static variables, in particular @c inline functions.
 **/

#ifdef GETTIMING
# define ORWL_TIMER(NAME)                                                                   \
P00_BLK_START                                                                               \
P00_BLK_BEFORE(register orwl_timing_element* elem = 0)                                      \
P99_PREFER(                                                                                 \
           static orwl_timing_element p00_static_elem                                       \
           = ORWL_TIMING_ELEMENT_INITIALIZER(NAME);                                         \
           if (P99_UNLIKELY(p00_static_elem.nb == -1)) {                                    \
             ORWL_CRITICAL {                                                                \
               if (P99_UNLIKELY(p00_static_elem.nb == -1)) {                                \
                 orwl_timing_element_insert(&p00_static_elem);                              \
                 p00_static_elem.nb = 0;                                                    \
               }                                                                            \
             }                                                                              \
           }                                                                                \
           elem = &p00_static_elem;                                                         \
           goto P99_LINEID(p00_label_, NAME);                                               \
           ) P99_LINEID(p00_label_, NAME):                                                  \
            P00_BLK_DECL(struct timespec, p00_end)                                          \
            P00_BLK_DECL(struct timespec, p00_start, orwl_gettime())                        \
            P00_BLK_DECL(atomic_float, p00_sec)                                             \
            P00_BLK_AFTER(atomic_fetch_add(&(elem->nb), 1),                                 \
                          atomic_fetch_atomic_float_add(&(elem->time2), p00_sec * p00_sec), \
                          atomic_fetch_atomic_float_add(&(elem->time), p00_sec))            \
            P00_BLK_AFTER(p00_sec = timespec2seconds(timespec_diff(p00_start, p00_end)))    \
            P00_BLK_AFTER(p00_end = orwl_gettime())                                         \
            P00_BLK_END
#else
# define ORWL_TIMER(NAME)
#endif


#endif 	    /* !ORWL_TIMING_H_ */