mod_konoha.la: mod_konoha.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_konoha.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_konoha.la
