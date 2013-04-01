/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

/* ************************************************************************ */
#include "konoha3/konoha.h"
#include "../BitmapGC/bmgc.h"

#ifdef __cplusplus
extern "C" {
#endif

KONOHA_EXPORT(kbool_t) LoadBitmapGenGCModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"BitmapGenGC", "0.1", 0, "bmgengc",
	};
	factory->GCModule.GCInfo            = &ModuleInfo;
	factory->GCModule.Kmalloc           = Kmalloc;
	factory->GCModule.Kzmalloc          = Kzmalloc;
	factory->GCModule.Kfree             = Kfree;
	factory->GCModule.InitGcContext     = KnewGcContext;
	factory->GCModule.DeleteGcContext   = KdeleteGcContext;
	factory->GCModule.ScheduleGC        = KscheduleGC;
	factory->GCModule.AllocObject       = KallocObject;
	factory->GCModule.WriteBarrier      = Kwrite_barrier;
	factory->GCModule.UpdateObjectField = KupdateObjectField;
	factory->GCModule.IsKonohaObject    = KisObject;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
