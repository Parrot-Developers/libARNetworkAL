/*
    Copyright (C) 2016 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <sys/uio.h>

#ifdef BUILD_LIBMUX

#include <libARSAL/ARSAL.h>
#include <libpomp.h>
#include <libmux.h>
#include <libARNetworkAL/ARNETWORKAL_Manager.h>
#include "ARNETWORKAL_MuxNetwork.h"


#define ARNETWORKAL_MUXNETWORK_TAG     "ARNETWORKAL_MuxNetwork"
#define ARNETWORKAL_MUXNETWORK_CHANID  1

#define ARNETWORKAL_MUXNETWORK_TIMEOUT_SEC 3

typedef struct _ARNETWORKAL_MuxNetworkObject_ {
	struct mux_ctx      *ctx;
	struct pomp_buffer  *rxbuf_recv;
	struct pomp_buffer  *rxbuf_pop;
	struct mux_queue    *rxqueue;
	ARNETWORKAL_Manager_OnDisconnect_t  onDisconnect;
	void                *onDisconnectCustomData;
	int                 disconnected;
} ARNETWORKAL_MuxNetworkObject;

/**
 */
static eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_MuxNetwork_PushFrame(
		ARNETWORKAL_Manager_t *manager,
		ARNETWORKAL_Frame_t *frame)
{
	int res = 0;
	ARNETWORKAL_MuxNetworkObject *obj = manager->senderObject;
	struct pomp_buffer *buf = NULL;
	void *data = NULL;
	uint8_t *datau8 = NULL;
	size_t off = 0;
	uint32_t framesize = 0, datasize = 0;

	/* Allocate buffer to copy frame */
	buf = pomp_buffer_new(frame->size);
	if (buf == NULL)
		goto out;

	/* Get data from buffer */
	res = pomp_buffer_get_data(buf, &data, NULL, NULL);
	if (res < 0)
		goto out;
	datau8 = data;

	memcpy(datau8 + off, &frame->type, sizeof(uint8_t));
	off += sizeof(uint8_t);

	memcpy(datau8 + off, &frame->id, sizeof(uint8_t));
	off += sizeof(uint8_t);

	memcpy(datau8 + off, &frame->seq, sizeof(uint8_t));
	off += sizeof(uint8_t);

	framesize = htodl(frame->size);
	memcpy(datau8 + off, &framesize, sizeof(uint32_t));
	off += sizeof(uint32_t);

	datasize = frame->size - offsetof(ARNETWORKAL_Frame_t, dataPtr);
	memcpy(datau8 + off, frame->dataPtr, datasize);
	off += datasize;

	/* Setup length */
	res = pomp_buffer_set_len(buf, off);
	if (res < 0)
		goto out;

	/* Encode and send data */
	res = mux_encode(obj->ctx, ARNETWORKAL_MUXNETWORK_CHANID, buf);
	if (res < 0) {
		ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_MUXNETWORK_TAG,
				"mux_encode(chanid=%u): err=%d(%s)",
				ARNETWORKAL_MUXNETWORK_CHANID,
				res, strerror(-res));

		/* Notify disconnection if not already done */
		if (!obj->disconnected) {
			obj->disconnected = 1;
			if (obj->onDisconnect)
				obj->onDisconnect(manager,
						obj->onDisconnectCustomData);
		}
		goto out;
	}

out:
	if (buf != NULL)
		pomp_buffer_unref(buf);
	return res == 0 ? ARNETWORKAL_MANAGER_RETURN_DEFAULT :
			ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
}

/**
 */
static eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_MuxNetwork_PopFrame(
		ARNETWORKAL_Manager_t *manager,
		ARNETWORKAL_Frame_t *frame)
{
	ARNETWORKAL_MuxNetworkObject *obj = manager->receiverObject;
	uint32_t off = 0;
	uint32_t size = 0;
	const void *data = NULL;
	const uint8_t *datau8 = NULL;
	size_t len = 0;

	/* Check for a buffer retrieved by Receive function */
	if (obj->rxbuf_recv == NULL)
		return ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY;

	/* Move it to pop buffer (that shall be NULL) */
	if (obj->rxbuf_pop != NULL)
		return ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
	obj->rxbuf_pop = obj->rxbuf_recv;
	obj->rxbuf_recv = NULL;

	/* Get data from buffer */
	if (pomp_buffer_get_cdata(obj->rxbuf_pop, &data, &len, NULL) < 0)
		return ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;

	/* Make sure buffer is big enough for frame header */
	if (len < 3 * sizeof(uint8_t) + sizeof(uint32_t))
		return ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;

	datau8 = data;

	memcpy(&frame->type, datau8 + off, sizeof(uint8_t));
	off += sizeof(uint8_t);

	memcpy(&frame->id, datau8 + off, sizeof(uint8_t));
	off += sizeof(uint8_t);

	memcpy(&frame->seq, datau8 + off, sizeof(uint8_t));
	off += sizeof(uint8_t);

	memcpy(&size, datau8 + off, sizeof(uint32_t));
	off += sizeof(uint32_t);
	frame->size = dtohl(size);

	/* Make sure buffer is big enough for frame data */
	if (len < frame->size)
		return ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;

	frame->dataPtr = (uint8_t *)datau8 + off;

	/* Buffer will be released in next call to Receive function
	 * Next call to Pop will return no frame (current buffer moved from
	 * rxbuf_recv to rxbuf_pop)
	 */

	return ARNETWORKAL_MANAGER_RETURN_DEFAULT;
}

/**
 */
static eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_MuxNetwork_Send(
		ARNETWORKAL_Manager_t *manager)
{
	/* Nothing to do here, data was sent in PushFrame */
	return ARNETWORKAL_MANAGER_RETURN_DEFAULT;
}

/**
 */
static eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_MuxNetwork_Receive(
		ARNETWORKAL_Manager_t *manager)
{
	int res = 0;
	ARNETWORKAL_MuxNetworkObject *obj = manager->receiverObject;
	struct timespec timeout = {
		.tv_sec = ARNETWORKAL_MUXNETWORK_TIMEOUT_SEC,
		.tv_nsec = 0,
	};

	/* Release previous buffer if any */
	if (obj->rxbuf_recv != NULL)
		pomp_buffer_unref(obj->rxbuf_recv);
	obj->rxbuf_recv = NULL;
	if (obj->rxbuf_pop != NULL)
		pomp_buffer_unref(obj->rxbuf_pop);
	obj->rxbuf_pop = NULL;

	/* Queue may be NULL if previous wait return EPIPE meaning queue is
	 * being destroyed */
	if (obj->rxqueue == NULL)
		return ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;

	/* Wait for next buffer */
	res = mux_queue_timed_get_buf(obj->rxqueue, &obj->rxbuf_recv, &timeout);
	if ((res == -EPIPE) || (res == -ETIMEDOUT)) {
		ARSAL_PRINT(ARSAL_PRINT_INFO, ARNETWORKAL_MUXNETWORK_TAG,
			    "mux_queue_timed_get_buf(chanid=%u) returned %s.",
			    ARNETWORKAL_MUXNETWORK_CHANID,
			    (res == -EPIPE) ? "EPIPE" : "ETIMEDOUT");
		/* Queue is being destroyed, we should not use it anymore */
		obj->rxqueue = NULL;

		/* Notify disconnection if not already done */
		if (!obj->disconnected) {
			obj->disconnected = 1;
			if (obj->onDisconnect)
				obj->onDisconnect(manager,
						obj->onDisconnectCustomData);
		}
		return ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
	} else if (res < 0) {
		ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_MUXNETWORK_TAG,
			    "mux_queue_timed_get_buf(chanid=%u): err=%d(%s)",
			    ARNETWORKAL_MUXNETWORK_CHANID,
			    res, strerror(-res));
		return ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
	}

	return ARNETWORKAL_MANAGER_RETURN_DEFAULT;
}

/**
 */
static eARNETWORKAL_ERROR ARNETWORKAL_MuxNetwork_Unlock(
		ARNETWORKAL_Manager_t *manager)
{
	ARNETWORKAL_MuxNetworkObject *obj = NULL;
	if (manager == NULL)
		return ARNETWORKAL_ERROR_BAD_PARAMETER;

	obj = manager->senderObject;
	/* close mux channel to unlock network reader thread */
	mux_channel_close(obj->ctx, ARNETWORKAL_MUXNETWORK_CHANID);
	return ARNETWORKAL_OK;
}

/**
 */
static eARNETWORKAL_ERROR ARNETWORKAL_MuxNetwork_GetBandwidth(
		ARNETWORKAL_Manager_t *manager,
		uint32_t *uploadBw,
		uint32_t *downloadBw)
{
	/* TODO */
	*uploadBw = 0;
	*downloadBw = 0;
	return ARNETWORKAL_OK;
}

/**
 */
static void *ARNETWORKAL_MuxNetwork_BandwidthThread(void *manager)
{
	return NULL;
}

/**
 */
static eARNETWORKAL_ERROR ARNETWORKAL_MuxNetwork_SetOnDisconnectCallback(
		ARNETWORKAL_Manager_t *manager,
		ARNETWORKAL_Manager_OnDisconnect_t onDisconnectCallback,
		void *customData)
{
	ARNETWORKAL_MuxNetworkObject *obj = manager->senderObject;
	obj->onDisconnect = onDisconnectCallback;
	obj->onDisconnectCustomData = customData;
	return ARNETWORKAL_OK;
}

/**
 */
static eARNETWORKAL_ERROR ARNETWORKAL_MuxNetwork_SetSendBufferSize(
		ARNETWORKAL_Manager_t *manager,
		int bufferSize)
{
	/* TODO */
	return ARNETWORKAL_OK;
}

/**
 */
static eARNETWORKAL_ERROR ARNETWORKAL_MuxNetwork_SetRecvBufferSize(
		ARNETWORKAL_Manager_t *manager,
		int bufferSize)
{
	/* TODO */
	return ARNETWORKAL_OK;
}

/**
 */
static eARNETWORKAL_ERROR ARNETWORKAL_MuxNetwork_GetSendBufferSize(
		ARNETWORKAL_Manager_t *manager,
		int *bufferSize)
{
	/* TODO */
	*bufferSize = 0;
	return ARNETWORKAL_OK;
}

/**
 */
static eARNETWORKAL_ERROR ARNETWORKAL_MuxNetwork_GetRecvBufferSize(
		ARNETWORKAL_Manager_t *manager,
		int *bufferSize)
{
	/* TODO */
	*bufferSize = 0;
	return ARNETWORKAL_OK;
}

/**
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_InitMuxNetwork(
		ARNETWORKAL_Manager_t *manager, struct mux_ctx *ctx)
{
	ARNETWORKAL_MuxNetworkObject *obj = NULL;

	if (manager == NULL || ctx == NULL)
		return ARNETWORKAL_ERROR_BAD_PARAMETER;

	/* Allocate internal object */
	obj = calloc(1, sizeof(*obj));
	if (obj == NULL)
		return ARNETWORKAL_ERROR_ALLOC;

	/* Setup internal object */
	obj->ctx = ctx;
	mux_ref(obj->ctx);
	manager->senderObject = obj;
	manager->receiverObject = obj;

	/* Open channel and allocate a receive queue */
	mux_channel_open(ctx, ARNETWORKAL_MUXNETWORK_CHANID, NULL, NULL);
	mux_channel_alloc_queue(ctx, ARNETWORKAL_MUXNETWORK_CHANID, 0, &obj->rxqueue);

	manager->maxIds = ARNETWORKAL_MANAGER_DEFAULT_ID_MAX;
	manager->maxBufferSize = 16384;

	/* Setup functions */
	manager->pushFrame = &ARNETWORKAL_MuxNetwork_PushFrame;
	manager->popFrame = &ARNETWORKAL_MuxNetwork_PopFrame;
	manager->send = &ARNETWORKAL_MuxNetwork_Send;
	manager->receive = &ARNETWORKAL_MuxNetwork_Receive;
	manager->unlock = &ARNETWORKAL_MuxNetwork_Unlock;
	manager->getBandwidth = &ARNETWORKAL_MuxNetwork_GetBandwidth;
	manager->bandwidthThread = &ARNETWORKAL_MuxNetwork_BandwidthThread;
	manager->setOnDisconnectCallback = &ARNETWORKAL_MuxNetwork_SetOnDisconnectCallback;
	manager->setSendBufferSize = &ARNETWORKAL_MuxNetwork_SetSendBufferSize;
	manager->setRecvBufferSize = &ARNETWORKAL_MuxNetwork_SetRecvBufferSize;
	manager->getSendBufferSize = &ARNETWORKAL_MuxNetwork_GetSendBufferSize;
	manager->getRecvBufferSize = &ARNETWORKAL_MuxNetwork_GetRecvBufferSize;

	return ARNETWORKAL_OK;
}

/**
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_CancelMuxNetwork(
		ARNETWORKAL_Manager_t *manager)
{
	if (manager == NULL)
		return ARNETWORKAL_ERROR_BAD_PARAMETER;

	/* Nothing to do, the mux layer should have stopped all channels
	 * prior to request stop of network layer */
	return ARNETWORKAL_OK;
}

/**
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_CloseMuxNetwork(
		ARNETWORKAL_Manager_t *manager)
{
	ARNETWORKAL_MuxNetworkObject *obj = NULL;
	if (manager == NULL)
		return ARNETWORKAL_ERROR_BAD_PARAMETER;

	obj = manager->senderObject;
	mux_unref(obj->ctx);
	free(obj);
	manager->senderObject = NULL;
	manager->receiverObject = NULL;

	return ARNETWORKAL_OK;
}

#endif // BUILD_LIBMUX
