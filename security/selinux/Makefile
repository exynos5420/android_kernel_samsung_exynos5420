#
# Makefile for building the SELinux module as part of the kernel tree.
#

ifeq ($(CONFIG_SECURITY_SELINUX_DEFAULT_PERMISSIVE),y)
EXTRA_CFLAGS += -DSELINUX_DEFAULT_PERMISSIVE=true
endif

ifeq ($(CONFIG_SECURITY_SELINUX_ALWAYS_PERMISSIVE),y)
EXTRA_CFLAGS += -DSELINUX_ALWAYS_PERMISSIVE=true
endif

ifeq ($(CONFIG_SECURITY_SELINUX_DEFAULT_ENFORCE),y)
EXTRA_CFLAGS += -DSELINUX_DEFAULT_ENFORCE=true
endif

ifeq ($(CONFIG_SECURITY_SELINUX_ALWAYS_ENFORCE),y)
EXTRA_CFLAGS += -DSELINUX_ALWAYS_ENFORCE=true
endif

ifeq ($(CONFIG_SECURITY_SELINUX_PRETEND_ENFORCE),y)
EXTRA_CFLAGS += -DSELINUX_PRETEND_ENFORCE=true
endif

obj-$(CONFIG_SECURITY_SELINUX) := selinux.o

selinux-y := avc.o hooks.o selinuxfs.o netlink.o nlmsgtab.o netif.o \
	     netnode.o netport.o exports.o \
	     ss/ebitmap.o ss/hashtab.o ss/symtab.o ss/sidtab.o ss/avtab.o \
	     ss/policydb.o ss/services.o ss/conditional.o ss/mls.o ss/status.o

selinux-$(CONFIG_SECURITY_NETWORK_XFRM) += xfrm.o

selinux-$(CONFIG_NETLABEL) += netlabel.o

ccflags-y := -Isecurity/selinux -Isecurity/selinux/include

$(addprefix $(obj)/,$(selinux-y)): $(obj)/flask.h

quiet_cmd_flask = GEN     $(obj)/flask.h $(obj)/av_permissions.h
      cmd_flask = scripts/selinux/genheaders/genheaders $(obj)/flask.h $(obj)/av_permissions.h

targets += flask.h av_permissions.h
$(obj)/flask.h: $(src)/include/classmap.h FORCE
	$(call if_changed,flask)
