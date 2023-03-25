inherit pypi setuptools3

SUMMARY = "This mini-package lets you get information about what a frame is currently doing, \
           particularly the AST node being executed"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=a3d6c15f7859ae235a78f2758e5a48cf"

PYPI_PACKAGE = "executing"

SRC_URI[sha256sum] = "19da64c18d2d851112f09c287f8d3dbbdf725ab0e569077efb6cdcbd3497c107"

DEPENDS += " \
	${PYTHON_PN}-setuptools-scm-native \
"