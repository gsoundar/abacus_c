/**
 * ABACUS
 * ------
 * by Gokul Soundararajan
 *
 * A simple event and task monitoring
 * framework
 *
 **/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include <abacus.h>


/* Function Prototypes */
/* ------------------------------------------------------------------ */

static double       __ab_time_ms ();
static unsigned int __ab_key_hash(struct hashtab *h, void *key );
static int          __ab_key_comp(struct hashtab *h, void *key1, void *key2);

/* Function Implementation */
/* ------------------------------------------------------------------ */

/* core */
/* ------------------------------------------------------------------ */
abacus* abacus_init(int num_tasks, int num_events, int num_class) {

  double cur_time;

  abacus *ab = calloc( 1, sizeof(abacus) );
  assert(ab && "ASSERT: Could not malloc space for ab!!");

  /* initialize everything */
  /* now */
  cur_time = __ab_time_ms();

  /* core */
  pthread_mutex_init( &ab->lock, NULL);
  ab->abacus_start_time = cur_time;
  ab->max_event_types = num_events;
  ab->max_class_types = num_class;
  ab->max_task_types = num_tasks;

  cur_time = abacus_time(ab);

  /* events */
  {
    unsigned i = 0, j=0;
    ab->evt_counts = (unsigned **) calloc(num_events, sizeof(unsigned*));
    assert(ab->evt_counts && "ASSERT: Could not malloc space for evt_counts!!");
    ab->evt_reset_time = (double **) calloc(num_events, sizeof(double *));
    assert(ab->evt_counts && "ASSERT: Could not malloc space for evt_reset_time!!");
    for(i=0; i < num_events; i++) {
      ab->evt_counts[i] = (unsigned *) calloc(num_class, sizeof(unsigned));
      assert(ab->evt_counts[i] && "ASSERT: Could not malloc space for evt_counts[i]");
      ab->evt_reset_time[i] = (double *) calloc(num_class, sizeof(double));
      assert(ab->evt_counts[i] && "ASSERT: Could not malloc space for evt_reset_time[i]");
      /* set everything to zero */
      for(j=0; j < num_class; j++) {
	ab->evt_counts[i][j] = 0;
	ab->evt_reset_time[i][j] = cur_time;
      }
    }
    ab->evt_global_reset_time = cur_time;
  }

  /* tasks */
  {
    unsigned i = 0, j = 0;
    ab->tsk_counts = (unsigned **) calloc(num_tasks, sizeof(unsigned*));
    assert(ab->tsk_counts && "ASSERT: Could not malloc space for tsk_counts!!");
    ab->tsk_delays = (double **) calloc(num_tasks, sizeof(double *));
    assert(ab->tsk_delays && "ASSERT: Could not malloc space for tsk_delays!!");
    ab->tsk_reset_time = (double **) calloc(num_tasks, sizeof(double *));
    assert(ab->tsk_reset_time && "ASSERT: Could not malloc space for tsk_reset_time!!");
    for(i=0; i < num_tasks; i++) {
      ab->tsk_counts[i] = (unsigned *) calloc(num_class, sizeof(unsigned));
      assert(ab->tsk_counts[i] && "ASSERT: Could not malloc space for tsk_counts[i]");
      ab->tsk_delays[i] = (double *) calloc(num_class, sizeof(double));
      assert(ab->tsk_delays[i] && "ASSERT: Could not malloc space for tsk_delays[i]");
      ab->tsk_reset_time[i] = (double *) calloc(num_class, sizeof(double));
      assert(ab->tsk_reset_time[i] && "ASSERT: Could not malloc space for tsk_reset_time[i]");
      /* set everything to zero */
      for(j=0; j < num_class; j++) {
	ab->tsk_counts[i][j] = 0;
	ab->tsk_delays[i][j] = 0;
	ab->tsk_reset_time[i][j] = cur_time;
      }
      ab->tsk_global_reset_time = cur_time;
    }

    ab->crumb_table = hashtab_create( __ab_key_hash, __ab_key_comp,
				      ABACUS_TRACKING_TABLE_SIZE );
    assert(ab->crumb_table && "ASSERT: Could not malloc space for crumb_table!!");

  }

  return ab;

}


int     abacus_resetall(abacus *ab) {

  return (abacus_event_resetall(ab) + abacus_task_resetall(ab));

}


double   abacus_time(abacus *ab) {
  double t = (__ab_time_ms() - ab->abacus_start_time);
  //fprintf(stdout, "time: %10.3lf\n", t); fflush(stdout);
  return t;
}

/* events */
/* ------------------------------------------------------------------ */
int abacus_event_add(abacus *ab, int event, int class ) {

  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");
  assert((event < ab->max_event_types && event >= 0) && "ASSERT: event is out-of-range!!");
  assert((class < ab->max_class_types && class >= 0) && "ASSERT: class is out-of-range!!");

  /* increment count */
  pthread_mutex_lock( &ab->lock );
  ab->evt_counts[event][class]++;
  pthread_mutex_unlock( &ab->lock );

  return 0;

}


unsigned abacus_event_count(abacus *ab, int event, int class ) {

  unsigned count = 0;

  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");
  assert((event < ab->max_event_types && event >= 0) && "ASSERT: event is out-of-range!!");
  assert((class < ab->max_class_types && class >= 0) && "ASSERT: class is out-of-range!!");

  /* report count */
  pthread_mutex_lock( &ab->lock );
  count = ab->evt_counts[event][class];
  pthread_mutex_unlock( &ab->lock );

  return count;
}


int abacus_event_reset(abacus *ab, int event, int class ) {

  double cur_time;

  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");
  assert((event < ab->max_event_types && event >= 0) && "ASSERT: event is out-of-range!!");
  assert((class < ab->max_class_types && class >= 0) && "ASSERT: class is out-of-range!!");

  /* now */
  cur_time = abacus_time(ab);

  /* reset count */
  pthread_mutex_lock( &ab->lock );
  ab->evt_counts[event][class] = 0;
  ab->evt_reset_time[event][class] = cur_time;
  pthread_mutex_unlock( &ab->lock );

  return 0;

}

int abacus_event_resetall(abacus *ab) {

  unsigned i = 0, j = 0;
 
  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");

  /* now */
  double cur_time = abacus_time(ab);

  /* reset all counts */
  pthread_mutex_lock( &ab->lock );
  ab->evt_global_reset_time = cur_time;
  for(i = 0; i < ab->max_event_types; i++) {
    for(j=0; j < ab->max_class_types; j++) {
      ab->evt_counts[i][j] = 0;
      ab->evt_reset_time[i][j] = cur_time;
    }
  }
  pthread_mutex_unlock( &ab->lock );

  return 0;
}

double  abacus_event_period(abacus *ab, int event, int class ) {

  double cur_time, period;
 
  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");
  assert((event < ab->max_event_types && event >= 0) && "ASSERT: event is out-of-range!!");
  assert((class < ab->max_class_types && class >= 0) && "ASSERT: class is out-of-range!!");

  /* now */
  cur_time = abacus_time(ab);

  /* find measurement period */
  pthread_mutex_lock( &ab->lock );
  period = cur_time - ab->evt_reset_time[event][class];
  pthread_mutex_unlock( &ab->lock );

  return period;  

}

double   abacus_event_periodall(abacus *ab) {

  double cur_time, period;
 
  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");

  /* now */
  cur_time = abacus_time(ab);

  /* find measurement period */
  pthread_mutex_lock( &ab->lock );
  period = cur_time - ab->evt_global_reset_time;
  pthread_mutex_unlock( &ab->lock );

  return period;  

}

/* tasks */
/* ------------------------------------------------------------------ */
int     abacus_task_add(abacus *ab, guid_t *id ) {

  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");

  /* make new task */
  int i = 0, j = 0;
  crumb_t *crumb = (crumb_t *) calloc( 1, sizeof(crumb_t));
  assert(crumb && "ASSERT: Could not malloc space for crumb!!");
  crumb->tsk_counts = (unsigned **) calloc( ab->max_task_types, sizeof(unsigned *));
  assert(crumb->tsk_counts && "ASSERT: Could not malloc space for tsk_counts");
  crumb->tsk_start_time = (double **) calloc(ab->max_task_types, sizeof(double*));
  assert(crumb->tsk_start_time && "ASSERT: Could not malloc space for tsk_start_time");
  crumb->tsk_stop_time = (double **) calloc(ab->max_task_types, sizeof(double*));
  assert(crumb->tsk_stop_time && "ASSERT: Could not malloc space for tsk_stop_time");
  for(i=0; i < ab->max_task_types; i++) {
    crumb->tsk_counts[i] = (unsigned *) calloc( ab->max_class_types, sizeof(unsigned));
    assert(crumb->tsk_counts[i] && "ASSERT: Could not malloc space for tsk_counts[i]");
    crumb->tsk_start_time[i] = (double *) calloc(ab->max_class_types,sizeof(double));
    assert(crumb->tsk_start_time[i] && "ASSERT: Could not malloc space for tsk_start_time[i]");
    crumb->tsk_stop_time[i] = (double *) calloc(ab->max_class_types,sizeof(double));
    assert(crumb->tsk_stop_time[i] && "ASSERT: Could not malloc space for tsk_stop_time[i]");
    for(j=0; j < ab->max_class_types; j++) {
      crumb->tsk_counts[i][j] = 0;
      crumb->tsk_start_time[i][j] = 0;
      crumb->tsk_stop_time[i][j] = 0;
    }
  }
  
  /* copy tracking id */
  memcpy( crumb->key, *id, sizeof(guid_t));
  
  /* add task */
  pthread_mutex_lock( &ab->lock );
  int status = hashtab_insert( ab->crumb_table, &crumb->key, crumb );
  assert(status == 0 && "ASSERT: Could not insert crumb into table!!"); 
  pthread_mutex_unlock( &ab->lock );

  return 0;
}


int abacus_task_delete(abacus *ab, guid_t *id) {

  crumb_t *crumb;
  unsigned i = 0, j = 0;

  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");
  
  pthread_mutex_lock( &ab->lock );

  /* find it */
  crumb = hashtab_delete( ab->crumb_table, id ); 
  assert(crumb && "ASSERT: Could not find crumb to delete from table!!"); 

  /* TODO: copy data from crumb into abacus */  
  for(i=0; i < ab->max_task_types; i++) {
    for( j=0; j < ab->max_class_types; j++) { 
      assert(crumb->tsk_counts[i][j] == 0 || crumb->tsk_counts[i][j] == 1 );
      if(crumb->tsk_counts[i][j] == 1) {
	ab->tsk_counts[i][j] += crumb->tsk_counts[i][j];
	ab->tsk_delays[i][j] += (crumb->tsk_stop_time[i][j] - crumb->tsk_start_time[i][j]);
      }
    }
  }

  pthread_mutex_unlock( &ab->lock );

  /* free memory */
  for(i=0; i < ab->max_task_types; i++) {
    free( crumb->tsk_counts[i] );
    free( crumb->tsk_start_time[i] );
    free( crumb->tsk_stop_time[i] );
  }
  free(crumb->tsk_counts);
  free(crumb->tsk_start_time);
  free(crumb->tsk_stop_time);
  free(crumb);

  return 0;
}


int     abacus_task_start(abacus *ab, guid_t *id, int task, int class) {

  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");
  assert(id && "ASSERT: guid is invalid!!");
  assert((task < ab->max_task_types && task >= 0) && "ASSERT: task is out-of-range!!");
  assert((class < ab->max_class_types && class >= 0) && "ASSERT: class is out-of-range!!");

  /* record task start */
  pthread_mutex_lock( &ab->lock );
  crumb_t *crumb = hashtab_search( ab->crumb_table, id);
  assert(crumb && "ASSERT: Could not find crumb in table!!");
  crumb->tsk_start_time[task][class] = abacus_time(ab);
  pthread_mutex_unlock( &ab->lock );

  return 0;
}

int     abacus_task_end(abacus *ab, guid_t *id, int task, int class) {

  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");
  assert(id && "ASSERT: guid is invalid!!");
  assert((task < ab->max_task_types && task >= 0) && "ASSERT: task is out-of-range!!");
  assert((class < ab->max_class_types && class >= 0) && "ASSERT: class is out-of-range!!");

  /* record task stop */
  pthread_mutex_lock( &ab->lock );
  crumb_t *crumb = hashtab_search( ab->crumb_table, id);
  assert(crumb && "ASSERT: Could not find crumb in table!!");
  crumb->tsk_stop_time[task][class] = abacus_time(ab);
  crumb->tsk_counts[task][class]++;
  pthread_mutex_unlock( &ab->lock );

  return 0;
}

double   abacus_task_avgdelay(abacus *ab, int task, int class) {

  double avg_delay = 0.0;

  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");
  assert((task < ab->max_task_types && task >= 0) && "ASSERT: task is out-of-range!!");
  assert((class < ab->max_class_types && class >= 0) && "ASSERT: class is out-of-range!!");

  /* report avg delay */
  pthread_mutex_lock( &ab->lock );
  avg_delay = ab->tsk_delays[task][class]/ab->tsk_counts[task][class];
  pthread_mutex_unlock( &ab->lock );

  return avg_delay;

}

int     abacus_task_reset(abacus *ab, int task, int class) {

  double cur_time;

  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");
  assert((task < ab->max_task_types && task >= 0) && "ASSERT: task is out-of-range!!");
  assert((class < ab->max_class_types && class >= 0) && "ASSERT: class is out-of-range!!");

  /* now */
  cur_time = abacus_time(ab);

  /* reset count */
  pthread_mutex_lock( &ab->lock );
  ab->tsk_counts[task][class] = 0;
  ab->tsk_delays[task][class] = 0.0;
  ab->tsk_reset_time[task][class] = cur_time;
  pthread_mutex_unlock( &ab->lock );

  return 0;

}


int     abacus_task_resetall(abacus *ab) {
  
  double cur_time;
  unsigned i = 0, j = 0;

  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");

  /* now */
  cur_time = abacus_time(ab);

  /* reset count */
  pthread_mutex_lock( &ab->lock );
  ab->tsk_global_reset_time = cur_time;
  for(i=0; i < ab->max_task_types; i++) {
    for(j=0; j < ab->max_class_types; j++) {
      ab->tsk_counts[i][j] = 0;
      ab->tsk_delays[i][j] = 0.0;
      ab->tsk_reset_time[i][j] = cur_time;
    }
  }
  pthread_mutex_unlock( &ab->lock );

  return 0;
}


double  abacus_task_period(abacus *ab, int task, int class) {
  
  double cur_time, period;
 
  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");
  assert((task < ab->max_task_types && task >= 0) && "ASSERT: task is out-of-range!!");
  assert((class < ab->max_class_types && class >= 0) && "ASSERT: class is out-of-range!!");

  /* now */
  cur_time = abacus_time(ab);

  /* find measurement period */
  pthread_mutex_lock( &ab->lock );
  period = cur_time - ab->tsk_reset_time[task][class];
  pthread_mutex_unlock( &ab->lock );

  return period;  
}

double abacus_task_periodall(abacus *ab) {

  double cur_time, period;
 
  /* check inputs */
  assert(ab && "ASSERT: ab is invalid!!");

  /* now */
  cur_time = abacus_time(ab);

  /* find measurement period */
  pthread_mutex_lock( &ab->lock );
  period = cur_time - ab->tsk_global_reset_time;
  pthread_mutex_unlock( &ab->lock );

  return period;  
}


/* helpers */
/* ------------------------------------------------------------------ */

static double __ab_time_ms() {

  double t;
  struct timeval tm;

  gettimeofday( &tm, NULL );
  t = (tm.tv_sec * 1000.0) + (tm.tv_usec/1000.0);

  return t;

}


static unsigned int __ab_key_hash(struct hashtab *h, void *key ) {

  guid_t *guid = (guid_t *) key;
  unsigned hash = 0, i = 0, sum = 0;

  for(i=0; i < sizeof(guid_t); i++ ) {
    sum += (unsigned) (*guid)[i];
  }

  hash = ( sum % (h->size) );

  return hash;
}

static int __ab_key_comp(struct hashtab *h, void *key1, void *key2) {

  guid_t *g1 = (guid_t *) key1;
  guid_t *g2 = (guid_t *) key2;

  return memcmp( *g1, *g2, sizeof(guid_t) );

}

