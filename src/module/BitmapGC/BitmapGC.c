/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
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
#ifdef __cplusplus
extern "C" {
#endif

#include "minikonoha/minikonoha.h"

//static int verbose_gc = 0;

#include "bmgc.h"

/* ------------------------------------------------------------------------ */

kbool_t LoadBitmapGCModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"BitmapGC", "0.1", 0, "bmgc",
	};
	factory->GCInfo            = &ModuleInfo;
	factory->Kmalloc = Kmalloc;
	factory->Kzmalloc = Kzmalloc;
	factory->Kfree = Kfree;
	factory->InitGcContext = KnewGcContext;
	factory->DeleteGcContext = KdeleteGcContext;
	factory->ScheduleGC = KscheduleGC;
	factory->AllocObject = KallocObject;
	factory->WriteBarrier = Kwrite_barrier;   // check this
	factory->UpdateObjectField = KupdateObjectField;  // check this
	factory->IsKonohaObject = KisObject;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
