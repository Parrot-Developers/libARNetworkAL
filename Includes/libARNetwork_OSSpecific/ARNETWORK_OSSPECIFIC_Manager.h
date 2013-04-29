/**
 *  @file ARNETWORK_OSSPECIFIC_Manager.h
 *  @brief network manager allow to send over network.
 *  @date 04/29/2013
 *  @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORK_OSSPECIFIC_MANAGER_H_
#define _ARNETWORK_OSSPECIFIC_MANAGER_H_

#include <libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Error.h>

/**
 *  @brief network manager allow to send to network and receive from a network.
 */
typedef struct ARNETWORK_OSSPECIFIC_Manager_t ARNETWORK_OSSPECIFIC_Manager_t;

/**
 *  @brief Create a new Manager
 *  @warning This function allocate memory
 *  @post ARNETWORK_OSSPECIFIC_Manager_Delete() must be called to delete the manager and free the memory allocated.
 *  @param[out] error pointor on the error output.
 *  @return Pointer on the new Manager
 *  @see ARNETWORK_OSSPECIFIC_Manager_Delete()
 */
ARNETWORK_OSSPECIFIC_Manager_t* ARNETWORK_OSSPECIFIC_Manager_New(eARNETWORK_OSSPECIFIC_ERROR *error);

/**
 *  @brief Delete the Manager
 *  @warning This function free memory
 *  @param manager address of the pointer on the Manager
 *  @see ARNETWORK_OSSPECIFIC_Manager_New()
 */
void ARNETWORK_OSSPECIFIC_Manager_Delete(ARNETWORK_OSSPECIFIC_Manager_t **manager);

#endif /** _ARNETWORK_OSSPECIFIC_MANAGER_H_ */
