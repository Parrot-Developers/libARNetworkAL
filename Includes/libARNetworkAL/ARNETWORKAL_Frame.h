/**
 * @file ARNETWORKAL_Frame.h
 * @brief define the network frame protocol
 * @date 04/30/2013
 * @author maxime.maitre@parrot.com
 */

#ifndef _ARNETWORKAL_FRAME_H_
#define _ARNETWORKAL_FRAME_H_

#include <inttypes.h>

/**
 * @brief type of frame send by the ARNETWORKAL_Manager
 */
typedef enum
{
    ARNETWORKAL_FRAME_TYPE_UNINITIALIZED = 0, /**< Unknown type. Don't use */
    ARNETWORKAL_FRAME_TYPE_ACK, /**< Acknowledgment type. Internal use only */
    ARNETWORKAL_FRAME_TYPE_DATA, /**< Data type. Main type for data that does not require an acknowledge */
    ARNETWORKAL_FRAME_TYPE_DATA_LOW_LATENCY, /**< Low latency data type. Should only be used when needed */
    ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK, /**< Data that should have an acknowledge type. This type can have a long latency */
    ARNETWORKAL_FRAME_TYPE_MAX, /**< Unused, iterator maximum value */
}eARNETWORKAL_FRAME_TYPE;

/**
 * @brief frame send by the ARNETWORKAL_Manager
 */
typedef struct
{
    uint8_t type; /**< frame type eARNETWORK_FRAME_TYPE */
    uint8_t id; /**< identifier of the buffer sending the frame */
    uint8_t seq; /**< sequence number of the frame */
    uint32_t size; /**< size of the frame */
    uint8_t *dataPtr; /**< pointer on the data of the frame */
}
// Doxygen does not like the __attribute__ tag
#ifndef DOXYGEN
__attribute__((__packed__))
#endif
ARNETWORKAL_Frame_t;

#endif /** _ARNETWORKAL_FRAME_H_ */
