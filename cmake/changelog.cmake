add_custom_target(changelog
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/utils/generate_changelog.sh ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.txt ${PKG_VERSION} ${CMAKE_BINARY_DIR} ${CMAKE_INSTALL_PREFIX}/CHANGELOG.txt
)

