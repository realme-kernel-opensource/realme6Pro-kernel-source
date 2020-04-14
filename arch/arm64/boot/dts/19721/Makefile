# SPDX-License-Identifier: GPL-2.0
ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
	dtbo-$(CONFIG_ARCH_ATOLL) += \
		atoll-idp-overlay.dtbo \
		atoll-wcd937x-idp-overlay.dtbo \
		atoll-ab-idp-overlay.dtbo \
		atoll-ab-wcd937x-idp-overlay.dtbo

atoll-idp-overlay.dtbo-base := atoll.dtb
atoll-wcd937x-idp-overlay.dtbo-base := atoll.dtb
atoll-ab-idp-overlay.dtbo-base := atoll-ab.dtb
atoll-ab-wcd937x-idp-overlay.dtbo-base := atoll-ab.dtb
else
dtb-$(CONFIG_ARCH_ATOLL)	+= atoll-idp.dtb \
	atoll-wcd937x-idp.dtb \
	atoll-ab-idp.dtb \
	atoll-ab-wcd937x-idp.dtb
endif

ifeq ($(CONFIG_ARM64),y)
always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
else
targets += dtbs
targets += $(addprefix ../, $(dtb-y))

$(obj)/../%.dtb: $(src)/%.dts FORCE
	$(call if_changed_dep,dtc)

ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
$(obj)/%.dtbo:$(src)/../../../../arm64/boot/dts/qcom/%.dts FORCE
	$(call if_changed_dep,dtc)
	$(call if_changed,dtbo_verify)

dtbs: $(addprefix $(obj)/,$(dtb-y)) $(addprefix $(obj)/,$(dtbo-y))
else
dtbs: $(addprefix $(obj)/../,$(dtb-y))
endif
endif
clean-files	:= *.dtb *.dtbo
