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


#ifndef _ARNETWORKAL_MUX_NERWORK_H_
#define _ARNETWORKAL_MUX_NERWORK_H_

#include <libARNetworkAL/ARNetworkAL.h>

/* Forward declarations */
struct mux_ctx;

eARNETWORKAL_ERROR ARNETWORKAL_Manager_InitMuxNetwork(
		ARNETWORKAL_Manager_t *manager, struct mux_ctx *ctx);

eARNETWORKAL_ERROR ARNETWORKAL_Manager_CancelMuxNetwork(
		ARNETWORKAL_Manager_t *manager);

eARNETWORKAL_ERROR ARNETWORKAL_Manager_CloseMuxNetwork(
		ARNETWORKAL_Manager_t *manager);

#endif /** _ARNETWORKAL_MUX_NERWORK_H_ */
