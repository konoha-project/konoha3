#include "konoha3/konoha.h"
#include "test_konoha.h"

#ifdef __cplusplus
extern "C" {
#endif

static void reset_timer(struct timeval *timer)
{
	unsigned long long t = getTimeMilliSecond();
	timer->tv_sec = t / 1000;
	timer->tv_usec = t % 1000 * 10;
}

static void show_timer(struct timeval *timer, const char *s)
{
	struct timeval time;
	unsigned long long t = getTimeMilliSecond();
	time.tv_sec = t / 1000;
	time.tv_usec = t % 1000 * 10;
	double sec = (time.tv_sec - timer->tv_sec)
		+ (double)(time.tv_usec - timer->tv_usec) / 1000 / 1000;
	fprintf(stderr, "%-16s: %f sec\n", s, sec);
}

#define EACH  4
#define COUNT 20000
#define SUM   199990000

static void each(KonohaContext *kctx, void *thunk, KKeyValue *d)
{
	uintptr_t *val = (uintptr_t *) thunk;
	*val += d->unboxValue;
}

static uintptr_t kObjectProto_GetUnboxValue(KonohaContext *kctx, kObject *o, ksymbol_t key, uintptr_t defval)
{
	KKeyValue *kvs = KLIB kObjectProto_GetKeyValue(kctx, o, key);
	return (kvs == NULL) ?  defval : kvs->unboxValue;
}

static void test_proto_value(KonohaContext *kctx)
{
	struct timeval timer;
	kObject *o0 = new_(Object, 0, GcUnsafe);
	kObject *o1 = new_(Object, 0, GcUnsafe);
	kObject *o2 = new_(Object, 0, GcUnsafe);
	uintptr_t i;

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_SetUnboxValue(kctx, o0, i, KType_Int, i);
	}
	show_timer(&timer, "Map.setN");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		uintptr_t val = kObjectProto_GetUnboxValue(kctx, o0, -(i+1), 0);
		assert(val == 0);
	}
	show_timer(&timer, "Map.get:missN");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		uintptr_t val = kObjectProto_GetUnboxValue(kctx, o0, i, 0);
		assert(val == i);
	}
	show_timer(&timer, "Map.getN");

	reset_timer(&timer);
	for (i = 0; i < EACH; i++) {
		uintptr_t val = 0;
		KLIB kObjectProto_DoEach(kctx, o0, (void *) &val, each);
		assert(val == SUM);
	}
	show_timer(&timer, "Map.eachN");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_RemoveKey(kctx, o0, i);
	}
	show_timer(&timer, "Map.removeN");

	reset_timer(&timer);
	for (i = 0; i < EACH; i++) {
		uintptr_t val = 0;
		KLIB kObjectProto_DoEach(kctx, o0, (void *) &val, each);
		assert(val == 0);
	}
	show_timer(&timer, "Map.each:emptyN");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_SetUnboxValue(kctx, o1, i%4, KType_Int, i%4);
	}
	show_timer(&timer, "Map[4].setN");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		uintptr_t val = kObjectProto_GetUnboxValue(kctx, o1, i%4, 0);
		assert(val == i%4);
	}
	show_timer(&timer, "Map[4].getN");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		uintptr_t val = 0;
		KLIB kObjectProto_DoEach(kctx, o1, (void *) &val, each);
		assert(val == 6);
	}
	show_timer(&timer, "Map[4].eachN");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_RemoveKey(kctx, o1, i);
	}
	show_timer(&timer, "Map[4].removeN");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_SetUnboxValue(kctx, o2, i%16, KType_Int, i%16);
	}
	show_timer(&timer, "Map[16].setN");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		uintptr_t val = kObjectProto_GetUnboxValue(kctx, o2, i%16, 0);
		assert(val == i%16);
	}
	show_timer(&timer, "Map[16].getN");

	reset_timer(&timer);
	for (i = 0; i < COUNT/2; i++) {
		uintptr_t val = 0;
		KLIB kObjectProto_DoEach(kctx, o2, (void *) &val, each);
		assert(val == 120);
	}
	show_timer(&timer, "Map[16].eachN");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_RemoveKey(kctx, o2, i);
	}
	show_timer(&timer, "Map[16].removeN");
}

#if 0
static void each_object(KonohaContext *kctx, void *thunk, KKeyValue *d)
{
	uintptr_t *val = (uintptr_t *) thunk;
	*val += (uintptr_t) d->ObjectValue;
}

static void test_proto_object(KonohaContext *kctx)
{
	struct timeval timer;
	kObject *o0 = new_(Object, 0, GcUnsafe);
	kObject *o1 = new_(Object, 0, GcUnsafe);
	kObject *o2 = new_(Object, 0, GcUnsafe);
	uintptr_t i;

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_SetObject(kctx, o0, i, KType_Object, o0);
	}
	show_timer(&timer, "Map.setO");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		kObject *tmp = KLIB kObject_getObject(kctx, o0, -(i+1), 0);
		assert(tmp == 0);
	}
	show_timer(&timer, "Map.get:missO");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		kObject *tmp = KLIB kObject_getObject(kctx, o0, i, 0);
		assert(tmp == o0);
	}
	show_timer(&timer, "Map.getO");

	reset_timer(&timer);
	for (i = 0; i < EACH; i++) {
		uintptr_t val = 0;
		KLIB kObjectProto_DoEach(kctx, o0, (void *) &val, each_object);
		assert(val == ((uintptr_t)o0) * COUNT);
	}
	show_timer(&timer, "Map.eachO");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_RemoveKey(kctx, o0, i);
	}
	show_timer(&timer, "Map.removeO");

	reset_timer(&timer);
	for (i = 0; i < EACH; i++) {
		uintptr_t val = 0;
		KLIB kObjectProto_DoEach(kctx, o0, (void *) &val, each_object);
		assert(val == 0);
	}
	show_timer(&timer, "Map.each:emptyO");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_SetObject(kctx, o1, i%4, KType_Object, o1);
	}
	show_timer(&timer, "Map[4].setO");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		kObject *tmp = KLIB kObject_getObject(kctx, o1, i%4, 0);
		assert(tmp == o1);
	}
	show_timer(&timer, "Map[4].getO");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		uintptr_t val = 0;
		KLIB kObjectProto_DoEach(kctx, o1, (void *) &val, each_object);
		assert(val == 4*(uintptr_t)o1);
	}
	show_timer(&timer, "Map[4].eachO");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_RemoveKey(kctx, o1, i);
	}
	show_timer(&timer, "Map[4].removeO");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_SetObject(kctx, o2, i%16, KType_Object, o2);
	}
	show_timer(&timer, "Map[16].setO");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		kObject *tmp = KLIB kObject_getObject(kctx, o2, i%16, 0);
		assert(tmp == o2);
	}
	show_timer(&timer, "Map[16].getO");

	reset_timer(&timer);
	for (i = 0; i < COUNT/2; i++) {
		uintptr_t val = 0;
		KLIB kObjectProto_DoEach(kctx, o2, (void *) &val, each_object);
		assert(val == 16*(uintptr_t)o2);
	}
	show_timer(&timer, "Map[16].eachO");

	reset_timer(&timer);
	for (i = 0; i < COUNT; i++) {
		KLIB kObjectProto_RemoveKey(kctx, o2, i);
	}
	show_timer(&timer, "Map[16].removeO");
}
#else
static void test_proto_object(KonohaContext *kctx) {}
#endif

int main(int argc, char const* argv[])
{
	KonohaContext* konoha = CreateContext();
	test_proto_value(konoha);
	test_proto_object(konoha);
	DeleteContext(konoha);
	return 0;
}

#ifdef __cplusplus
}
#endif
