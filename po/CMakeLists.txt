find_package(Gettext REQUIRED)
file(GLOB POFILES "*.po")
gettext_create_translations(kdoubanfm.pot ALL ${POFILES})
