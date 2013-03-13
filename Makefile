include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=coffeed
PKG_RELEASE:=1
PKG_VERSION:=2.1

include $(INCLUDE_DIR)/package.mk

define Package/coffeed
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Tool for controlling gpio pins
  DEPENDS:= +libstdcpp +kmod-i2c-gpio +kmod-i2c-gpio-custom +kmod-pwm-gpio-custom +libpthread
endef

define Package/coffeed/description
	Tool for controlling gpio pins
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) CFLAGS="-O3 $(TARGET_CFLAGS) -I$(LINUX_DIR)/include"
endef

define Package/coffeed/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/coffeed.init $(1)/etc/init.d/coffeed
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/coffeed.config $(1)/etc/config/coffeed
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/coffeed $(1)/usr/sbin/coffeed
endef

$(eval $(call BuildPackage,coffeed))
