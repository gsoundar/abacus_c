/**
 * GEMINI - GUID
 * -------------
 * by Gokul Soundararajan
 *
 * A GUID generator by using md5 underneath
 * No one should depend on md5.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/utsname.h>
#include <assert.h>
#include <pthread.h>
#include <md5.h>
#include <guid.h>

static pthread_mutex_t __guid_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned long __guid_count = 0;

guid_t *guid_create() {
  
  char buffer[1024], signature[16];
  struct utsname name;
  unsigned long rnd, count;

  /* data for md5 (hostname, random) */
  {
    pthread_mutex_lock( &__guid_lock );
    __guid_count++; count = __guid_count;
    pthread_mutex_unlock( &__guid_lock );
  }
  uname( &name );
  rnd = random();
  sprintf( buffer, "%s-%lu-%lu", name.nodename, count, rnd );
  buffer[1023] = '\0';

  /* md5 it! */
  md5_buffer( buffer, strlen(buffer), (void *) signature );

  /* make it into a string */
  guid_t *guid = NULL;
  guid = (guid_t *) malloc( sizeof(guid_t) );
  if(guid) {
    md5_sig_to_string( signature, *guid, sizeof(guid_t) );    
  }

  return guid;

} /* end guid_create() */

int guid_compare( guid_t *id1, guid_t *id2 ) {

  assert( id1 && id2 );
  return strncmp( *id1, *id2, GUID_ARRAY_LEN );

} /* end guid_compare() */


int guid_destroy( guid_t *guid ) {

  assert(guid);

  free(guid);

  return 0;

} /* end guid_destroy() */

