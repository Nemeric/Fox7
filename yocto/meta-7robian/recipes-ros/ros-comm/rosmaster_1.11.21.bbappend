FILESEXTRAPATHS_append := "${THISDIR}/rosmaster:"
SRC_URI += "file://50-ros.sh"

do_install_append() {
  install -d ${D}/etc/profile.d/
  install -m 644 ${WORKDIR}/50-ros.sh ${D}/etc/profile.d/50-ros.sh
}

FILES_${PN} += "/etc/profile.d/50-ros.sh"
