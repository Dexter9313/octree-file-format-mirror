
add_custom_target(uninstall
		COMMAND xargs rm < install_manifest.txt && rm install_manifest.txt
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Uninstalling executable..."
		VERBATIM)

