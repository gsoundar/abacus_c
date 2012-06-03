/** 
 * GEMINI - GUID
 * -------------
 * by Gokul Soundararajan
 *
 * Generate a globally unique identifier
 *
 **/

#ifndef __GUID_H__
#define __GUID_H__

#define GUID_ARRAY_LEN 33
typedef char guid_t[GUID_ARRAY_LEN];

guid_t *guid_create();
int     guid_compare( guid_t *id1, guid_t *id2 );
int     guid_destroy( guid_t *guid );

#endif /* end __GUID_H__ */
