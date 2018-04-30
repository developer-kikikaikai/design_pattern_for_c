#load common setting
include $(top_srcdir)/builder/builder_am.conf

noinst_LTLIBRARIES = libossupport_device_plugin.la libusb_device_plugin.la
#
#lib_LTLIBRARIES = libbuilder.la

libossupport_device_plugin_la_SOURCES = ossupport_device_plugin.c
libossupport_device_plugin_la_LDFLAGS=-module -shared -export-dynamic -rpath $(libdir)

libusb_device_plugin_la_SOURCES = usb_device_plugin.c
libusb_device_plugin_la_LDFLAGS=-module -shared -export-dynamic -rpath $(libdir)
