/*
 * libevent_if_libev plugin for threadpool, used libev (developed Marc Alexander Lehmann)
 *
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 *
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License ("GPL") version 2 or any later version,
 * in which case the provisions of the GPL are applicable instead of
 * the above. If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use your
 * version of this file under the BSD license, indicate your decision
 * by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL. If you do not delete the
 * provisions above, a recipient may use your version of this file under
 * either the BSD or the GPL.
 */
#include "tpool_event_if.h"
#include "dp_util.h"
#include <ev.h>
#include <unistd.h>
#include <sys/eventfd.h>

struct event_libev_manager_t {
	struct ev_loop * loop;
	int is_stop;
};

typedef struct event_libev_manager_t * EventLibevManager;

struct event_libev_handler_t {
	EventLibevManager base;
	void (*event_callback)(int socketfd, int eventflag, void * event_arg);
	void *arg;
	ev_io watch;
};

typedef struct event_libev_handler_t event_libev_handler_t, *EventLibevHandler; 

static inline int convert_etpoll_eveid2own(int eventflag) {
	int ret_eveflag=0;
	if(eventflag&EV_TPOOL_READ) ret_eveflag |= EV_READ;
	if(eventflag&EV_TPOOL_WRITE) ret_eveflag |= EV_WRITE;
	if(eventflag&EV_TPOOL_HUNGUP) ret_eveflag |= EV_ERROR;
	return ret_eveflag;
}

static inline int convert_etpoll_own2eveid(int eventflag) {
	int ret_eveflag=0;
	if(eventflag&EV_READ)  ret_eveflag |= EV_TPOOL_READ;
	if(eventflag&EV_WRITE) ret_eveflag |= EV_TPOOL_WRITE;
	if(eventflag&EV_ERROR) ret_eveflag |= EV_TPOOL_HUNGUP;
	return ret_eveflag;
}

static int event_if_is_stop(EventLibevManager this) {
	int is_stop;
	is_stop = this->is_stop;
	return is_stop;
}

static void event_if_subscribe_cb(EV_P_ ev_io *w, int revents) {
	EventLibevHandler handler = (EventLibevHandler)w->data;
	if(event_if_is_stop(handler->base)) return;

	int eventflag = convert_etpoll_own2eveid(revents);

	handler->event_callback(handler->watch.fd, eventflag, handler->arg);
}

static void event_if_libev_handler_init(EventLibevManager base, EventLibevHandler handler, EventSubscriber subscriber, void *arg) {
	handler->base=base;
	handler->event_callback = subscriber->event_callback;
	handler->arg = arg;
	/*initialize watch*/
	ev_io_init(&handler->watch, event_if_subscribe_cb, subscriber->fd, convert_etpoll_eveid2own(subscriber->eventflag));
	/*set user data*/
	handler->watch.data = handler;

	/*start watch*/
	ev_io_start(base->loop, &handler->watch);
}

/*! @name API for event if */
/*@{*/
/** event new */
EventInstance event_if_new(void) {
	EventLibevManager instance = calloc(1, sizeof(*instance));
	if(!instance) return NULL;

	instance->loop = ev_loop_new(0);
	if(!instance->loop) goto err;

	return instance;
err:

	free(instance);
	return NULL;
}

/** add new event */
EventHandler event_if_add(EventInstance this, EventSubscriber subscriber, void *arg) {

	EventLibevManager base = (EventLibevManager)this;

	/*add event*/
	EventLibevHandler handler = calloc(1, sizeof(*handler));
	if(!handler) {
		DEBUG_ERRPRINT("Failed to new event!\n" );
		return NULL;
	}

	event_if_libev_handler_init(base, handler, subscriber, arg);
	return handler;
}

/** update registered event */
void * event_if_update(EventInstance this, EventHandler handler, EventSubscriber subscriber, void *arg) {
	DEBUG_ERRPRINT("Update!\n" );
	EventLibevManager base = (EventLibevManager)this;
	EventLibevHandler libev_handler = (EventLibevHandler)handler;
	if(event_if_is_stop((EventLibevManager)this)) return NULL;

	/*stop first*/
	ev_io_stop(base->loop, &libev_handler->watch);

	/*re-add*/
	event_if_libev_handler_init(base, libev_handler, subscriber, arg);

	return handler;
}

/** delete event */
void event_if_del(EventInstance this, EventHandler handler) {
	EventLibevManager base = (EventLibevManager)this;
	EventLibevHandler libev_handler = (EventLibevHandler) handler;
	if(event_if_is_stop((EventLibevManager)this)) return;

	ev_io_stop(base->loop, &libev_handler->watch);
	free(libev_handler);
}

int event_if_getfd(EventHandler handler) {
	return ((EventLibevHandler)handler)->watch.fd;
}

/** main loop to wait event */
int event_if_loop(EventInstance this) {
	EventLibevManager base = (EventLibevManager) this;
	ev_loop (base->loop, 0);
	return 0;
}

/** break event */
void event_if_loopbreak(EventInstance this) {
	EventLibevManager base = (EventLibevManager) this;
	base->is_stop = 1;
	ev_unloop (base->loop, EVUNLOOP_ONE);
}

/** exit after main loop */
void event_if_exit(EventInstance this) {
	EventLibevManager base = (EventLibevManager)this;
	ev_loop_destroy(base->loop);
}

/** free event if instance */
void event_if_free(EventInstance this) {
	EventLibevManager base = (EventLibevManager)this;
	free(base);
}
/*@}*/
