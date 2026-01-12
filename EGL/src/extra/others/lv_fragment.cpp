/**
 * @file lv_fragment.c
 *
 */

#include "extra/others/lv_fragment.h"

#if EG_USE_FRAGMENT

static void cb_delete_assertion(EGEvent *event);

lv_fragment_t *lv_fragment_create(const lv_fragment_class_t *cls, void *args)
{
	EG_ASSERT_NULL(cls);
	EG_ASSERT_NULL(cls->create_obj_cb);
	EG_ASSERT(cls->instance_size > 0);
	lv_fragment_t *instance = EG_AllocMem(cls->instance_size);
	EG_ZeroMem(instance, cls->instance_size);
	instance->cls = cls;
	instance->child_manager = lv_fragment_manager_create(instance);
	if(cls->constructor_cb) {
		cls->constructor_cb(instance, args);
	}
	return instance;
}

void lv_fragment_del(lv_fragment_t *fragment)
{
	EG_ASSERT_NULL(fragment);
	if(fragment->managed) {
		lv_fragment_manager_remove(fragment->managed->manager, fragment);
		return;
	}
	if(fragment->obj) {
		lv_fragment_del_obj(fragment);
	}
	/* Objects will leak if this function called before objects deleted */
	const lv_fragment_class_t *cls = fragment->cls;
	if(cls->destructor_cb) {
		cls->destructor_cb(fragment);
	}
	lv_fragment_manager_del(fragment->child_manager);
	EG_FreeMem(fragment);
}

lv_fragment_manager_t *lv_fragment_get_manager(lv_fragment_t *fragment)
{
	EG_ASSERT_NULL(fragment);
	EG_ASSERT_NULL(fragment->managed);
	return fragment->managed->manager;
}

EGObject *const *lv_fragment_get_container(lv_fragment_t *fragment)
{
	EG_ASSERT_NULL(fragment);
	EG_ASSERT_NULL(fragment->managed);
	return fragment->managed->container;
}

lv_fragment_t *lv_fragment_get_parent(lv_fragment_t *fragment)
{
	EG_ASSERT_NULL(fragment);
	EG_ASSERT_NULL(fragment->managed);
	return lv_fragment_manager_get_parent_fragment(fragment->managed->manager);
}

EGObject *lv_fragment_create_obj(lv_fragment_t *fragment, EGObject *container)
{
	lv_fragment_managed_states_t *states = fragment->managed;
	if(states) {
		states->destroying_obj = false;
	}
	const lv_fragment_class_t *cls = fragment->cls;
	EGObject *obj = cls->create_obj_cb(fragment, container);
	EG_ASSERT_NULL(obj);
	fragment->obj = obj;
	lv_fragment_manager_create_obj(fragment->child_manager);
	if(states) {
		states->obj_created = true;
		lv_obj_add_event_cb(obj, cb_delete_assertion, EG_EVENT_DELETE, NULL);
	}
	if(cls->obj_created_cb) {
		cls->obj_created_cb(fragment, obj);
	}
	return obj;
}

void lv_fragment_del_obj(lv_fragment_t *fragment)
{
	EG_ASSERT_NULL(fragment);
	lv_fragment_manager_del_obj(fragment->child_manager);
	lv_fragment_managed_states_t *states = fragment->managed;
	if(states) {
		if(!states->obj_created) return;
		states->destroying_obj = true;
		bool cb_removed = lv_obj_remove_event_cb(fragment->obj, cb_delete_assertion);
		EG_ASSERT(cb_removed);
	}
	EG_ASSERT_NULL(fragment->obj);
	const lv_fragment_class_t *cls = fragment->cls;
	if(cls->obj_will_delete_cb) {
		cls->obj_will_delete_cb(fragment, fragment->obj);
	}
	lv_obj_del(fragment->obj);
	if(cls->obj_deleted_cb) {
		cls->obj_deleted_cb(fragment, fragment->obj);
	}
	if(states) {
		states->obj_created = false;
	}
	fragment->obj = NULL;
}

void lv_fragment_recreate_obj(lv_fragment_t *fragment)
{
	EG_ASSERT_NULL(fragment);
	EG_ASSERT_NULL(fragment->managed);
	lv_fragment_del_obj(fragment);
	lv_fragment_create_obj(fragment, *fragment->managed->container);
}

static void cb_delete_assertion(EGEvent *event)
{
	EG_UNUSED(event);
	EG_ASSERT_MSG(0, "Please delete objects with lv_fragment_destroy_obj");
}

#endif 
