#include "test.h"
#include "../log.h"

struct tds_object_type tds_obj_test_type = {
        "test_typename",
        sizeof(struct tds_obj_test_data),
        tds_obj_test_init,
        tds_obj_test_destroy,
        tds_obj_test_update,
        tds_obj_test_draw,
        tds_obj_test_msg,
};

void tds_obj_test_init(struct tds_object* ptr) {
	// tds_logf(TDS_LOG_MESSAGE, "calling obj_test init - %x\n", (unsigned int) ptr);
}

void tds_obj_test_destroy(struct tds_object* ptr) {
	// tds_logf(TDS_LOG_MESSAGE, "calling obj_test destroy - %x\n", (unsigned int) ptr);
}

void tds_obj_test_draw(struct tds_object* ptr) {
	// tds_logf(TDS_LOG_MESSAGE, "calling obj_test draw - %x\n", (unsigned int) ptr);
}

void tds_obj_test_update(struct tds_object* ptr) {
	// tds_logf(TDS_LOG_MESSAGE, "calling obj_test update - %x\n", (unsigned int) ptr);
	// tds_logf(TDS_LOG_MESSAGE, "sending message to self via hmgr\n");
	// tds_object_send_msg(ptr, ptr->object_handle, 1, (void*) 0);
}

void tds_obj_test_msg(struct tds_object* ptr, struct tds_object* from, int msg, void* param) {
	// tds_logf(TDS_LOG_MESSAGE, "calling obj_test msg - %x, %x, %d, %x\n", (unsigned int) ptr, (unsigned int) from, msg, (unsigned int) param);
}
