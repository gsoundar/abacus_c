/**
 * ABACUS
 * ------
 * by Gokul Soundararajan
 *
 * A simple event and task monitoring
 * framework
 *
 **/

#ifndef __ABACUS_H__
#define __ABACUS_H__

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <guid.h>
#include <hashtable.h>

#define ABACUS_TRACKING_TABLE_SIZE 4096

struct hashtab;


typedef struct crumb_t {
  /* key */
  guid_t key;
  /* times */
  unsigned **tsk_counts;
  double   **tsk_start_time;
  double   **tsk_stop_time;
} crumb_t;

typedef struct abacus_ops {
  /* need to compare task ids */
  unsigned int (*key_hash) (struct hashtab *h, void *key );
  int          (*key_comp) (struct hashtab *h, void *key1, void *key2);
} ab_ops;

typedef struct abacus {
  /* lock */
  pthread_mutex_t lock;
  /* config */
  double abacus_start_time;
  int max_event_types;
  int max_class_types;
  int max_task_types;
  /* events */
  unsigned **evt_counts;
  double   **evt_reset_time;
  double     evt_global_reset_time;
  /* tasks */
  unsigned **tsk_counts;
  double   **tsk_delays;
  double   **tsk_reset_time;
  double     tsk_global_reset_time;
  /* task table */
  struct hashtab *crumb_table;
} abacus;


/* Function Prototypes */
/* ------------------------------------------------------------------ */

/* core */
abacus*  abacus_init(int num_tasks, int num_events, int num_class);
int      abacus_resetall(abacus *ab);
double   abacus_time(abacus *ab);
/* events */
int      abacus_event_add(abacus *ab, int event, int class );
unsigned abacus_event_count(abacus *ab, int event, int class );
int      abacus_event_reset(abacus *ab, int event, int class );
int      abacus_event_resetall(abacus *ab);
double   abacus_event_period(abacus *ab, int event, int class );
double   abacus_event_periodall(abacus *ab);
/* tasks */
int      abacus_task_add(abacus *ab, guid_t *id );
int      abacus_task_delete(abacus *ab, guid_t *id);
int      abacus_task_start(abacus *ab, guid_t *id, int task, int class);
int      abacus_task_end(abacus *ab, guid_t *id, int task, int class);
double   abacus_task_avgdelay(abacus *ab, int task, int class);
int      abacus_task_reset(abacus *ab, int task, int class);
int      abacus_task_resetall(abacus *ab);
double   abacus_task_period(abacus *ab, int task, int class);
double   abacus_task_periodall(abacus *ab);
#endif
