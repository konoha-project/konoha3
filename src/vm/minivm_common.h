typedef struct _FileLog_Map {
	int key;
	const char* value;
} FileLog_Map;

#define FILELOG_MEMORY_SIZE 1024

int filelog_memory_index = 0;
FileLog_Map *filelog_memory[FILELOG_MEMORY_SIZE] = {NULL};

FileLog_Map *new_FileLog_Map(KonohaContext* kctx, int key, const char* value) {
	FileLog_Map *ret = (FileLog_Map *)KMalloc_UNTRACE(sizeof(FileLog_Map));
	ret->key = key;
	ret->value = (const char *)KMalloc_UNTRACE(strlen(value) + 1);
	memset(ret->value, '\0', strlen(value) + 1);
	strncpy(ret->value, value, strlen(value));
	return ret;
}

void emitCoverage_element(KonohaContext* kctx, VirtualMachineInstruction *pc)
{
	int i;
	kfileline_t uline = 0;
	int fileid, id_tmp, id_current;
	const char *filename;
	FileLog_Map *filelog;

	if(pc->count > 0) {
		id_tmp = ((pc->line) >> (sizeof(kushort_t) * 8));
		if(filelog_memory_index == 0) {
			uline = pc->line;
			FileLog_Map *log;
			fileid = (uline >> (sizeof(kushort_t) * 8));
			filename = FileId_t(uline);
			filelog = new_FileLog_Map(kctx, fileid, filename);
			filelog_memory[filelog_memory_index++] = filelog;
		}
		else{
			for(i = 0; i < filelog_memory_index; i++) {
				id_current = filelog_memory[i]->key;
				if(id_tmp == id_current) {
					break;
				}
			}
			if(i == filelog_memory_index) {
				uline = pc->line;
				FileLog_Map *log;
				fileid = id_tmp;
				filename = FileId_t(uline);
				filelog = new_FileLog_Map(kctx, fileid, filename);
				filelog_memory[filelog_memory_index++] = filelog;
			}
		}
	}
}
