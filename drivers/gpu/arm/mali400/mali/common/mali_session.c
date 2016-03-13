/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "mali_osk.h"
#include "mali_osk_list.h"
#include "mali_session.h"

_MALI_OSK_LIST_HEAD(mali_sessions);

_mali_osk_lock_t *mali_sessions_lock;

_mali_osk_errcode_t mali_session_initialize(void)
{
	_MALI_OSK_INIT_LIST_HEAD(&mali_sessions);

	mali_sessions_lock = _mali_osk_lock_init(_MALI_OSK_LOCKFLAG_READERWRITER | _MALI_OSK_LOCKFLAG_ORDERED, 0, _MALI_OSK_LOCK_ORDER_SESSIONS);

	if (NULL == mali_sessions_lock) return _MALI_OSK_ERR_NOMEM;

	return _MALI_OSK_ERR_OK;
}

void mali_session_terminate(void)
{
	_mali_osk_lock_term(mali_sessions_lock);
}

void mali_session_add(struct mali_session_data *session)
{
	mali_session_lock();
	_mali_osk_list_add(&session->link, &mali_sessions);
	mali_session_unlock();
}

void mali_session_remove(struct mali_session_data *session)
{
	mali_session_lock();
	_mali_osk_list_delinit(&session->link);
	mali_session_unlock();
}
