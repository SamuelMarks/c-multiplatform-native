function(cmn_package_macos_app TARGET_NAME ICON_PNG_PATH)
    if(APPLE AND NOT IOS)
        set_target_properties(${TARGET_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
        )

        # Generate basic Info.plist
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Info.plist.in"
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
    <key>CFBundleExecutable</key>
    <string>${TARGET_NAME}</string>
    <key>CFBundleIdentifier</key>
    <string>com.example.${TARGET_NAME}</string>
    <key>CFBundleName</key>
    <string>${TARGET_NAME}</string>
    <key>CFBundleIconFile</key>
    <string>AppIcon.icns</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
</dict>
</plist>
")
        configure_file("${CMAKE_CURRENT_BINARY_DIR}/Info.plist.in" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)

        if(ICON_PNG_PATH)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset
                COMMAND sips -z 16 16   ${ICON_PNG_PATH} --out ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset/icon_16x16.png
                COMMAND sips -z 32 32   ${ICON_PNG_PATH} --out ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset/icon_16x16@2x.png
                COMMAND sips -z 32 32   ${ICON_PNG_PATH} --out ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset/icon_32x32.png
                COMMAND sips -z 64 64   ${ICON_PNG_PATH} --out ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset/icon_32x32@2x.png
                COMMAND sips -z 128 128 ${ICON_PNG_PATH} --out ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset/icon_128x128.png
                COMMAND sips -z 256 256 ${ICON_PNG_PATH} --out ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset/icon_128x128@2x.png
                COMMAND sips -z 256 256 ${ICON_PNG_PATH} --out ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset/icon_256x256.png
                COMMAND sips -z 512 512 ${ICON_PNG_PATH} --out ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset/icon_256x256@2x.png
                COMMAND sips -z 512 512 ${ICON_PNG_PATH} --out ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset/icon_512x512.png
                COMMAND sips -z 1024 1024 ${ICON_PNG_PATH} --out ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset/icon_512x512@2x.png
                COMMAND iconutil -c icns ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset -o ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns
                COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns $<TARGET_FILE_DIR:${TARGET_NAME}>/../../Resources/
            )
        endif()

        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND codesign --force --deep -s "-" $<TARGET_FILE_DIR:${TARGET_NAME}>/../..
            COMMAND hdiutil create -volname ${TARGET_NAME} -srcfolder $<TARGET_FILE_DIR:${TARGET_NAME}>/../../.. -ov -format UDZO ${TARGET_NAME}.dmg
        )
    endif()
endfunction()

function(cmn_package_windows_exe TARGET_NAME)
    if(WIN32)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/app.manifest"
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>
<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">
  <dependency>
    <dependentAssembly>
      <assemblyIdentity type=\"win32\" name=\"Microsoft.Windows.Common-Controls\" version=\"6.0.0.0\" processorArchitecture=\"*\" publicKeyToken=\"6595b64144ccf1df\" language=\"*\"/>
    </dependentAssembly>
  </dependency>
  <application xmlns=\"urn:schemas-microsoft-com:asm.v3\">
    <windowsSettings>
      <dpiAware xmlns=\"http://schemas.microsoft.com/SMI/2005/WindowsSettings\">true/PM</dpiAware>
      <dpiAwareness xmlns=\"http://schemas.microsoft.com/SMI/2016/WindowsSettings\">PerMonitorV2, PerMonitor</dpiAwareness>
    </windowsSettings>
  </application>
</assembly>
")
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/app.rc"
"1 24 \"app.manifest\"\n"
        )
        target_sources(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/MANIFEST:NO")


        # CPack configuration
        include(CPack)
        set(CPACK_GENERATOR "NSIS")
    endif()
endfunction()

function(cmn_package_linux_app TARGET_NAME)
    if(UNIX AND NOT APPLE AND NOT ANDROID)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.desktop"
"[Desktop Entry]
Name=${TARGET_NAME}
Exec=${TARGET_NAME}
Icon=${TARGET_NAME}
Type=Application
Terminal=false
")
        # AppImage creation hook
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/AppDir/usr/bin
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/AppDir/usr/share/applications
            COMMAND cp $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/AppDir/usr/bin/
            COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.desktop ${CMAKE_CURRENT_BINARY_DIR}/AppDir/usr/share/applications/
            # Assuming linuxdeploy / appimagetool are available
            # COMMAND linuxdeploy --appdir ${CMAKE_CURRENT_BINARY_DIR}/AppDir --output appimage
        )

        include(CPack)
        set(CPACK_GENERATOR "DEB;RPM")
        set(CPACK_PACKAGE_CONTACT "support@example.com")
    endif()
endfunction()

function(cmn_package_android_apk TARGET_NAME)
    if(ANDROID)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml"
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\" package=\"com.example.${TARGET_NAME}\">
    <application android:label=\"${TARGET_NAME}\" android:hasCode=\"false\">
        <activity android:name=\"android.app.NativeActivity\" android:exported=\"true\">
            <meta-data android:name=\"android.app.lib_name\" android:value=\"${TARGET_NAME}\" />
            <intent-filter>
                <action android:name=\"android.intent.action.MAIN\" />
                <category android:name=\"android.intent.category.LAUNCHER\" />
            </intent-filter>
        </activity>
    </application>
</manifest>
")
        # Pseudo-steps for aapt2, d8, zipalign, apksigner
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND echo "Packaging APK for ${TARGET_NAME}..."
            # COMMAND aapt2 link -o base.apk --manifest AndroidManifest.xml
            # COMMAND d8 ...
            # COMMAND zipalign ...
            # COMMAND apksigner ...
        )
    endif()
endfunction()

function(cmn_package_ios_ipa TARGET_NAME)
    if(APPLE AND IOS)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
    <key>CFBundleExecutable</key>
    <string>${TARGET_NAME}</string>
    <key>CFBundleIdentifier</key>
    <string>com.example.${TARGET_NAME}</string>
    <key>CFBundleName</key>
    <string>${TARGET_NAME}</string>
    <key>UIRequiresFullScreen</key>
    <true/>
</dict>
</plist>
")
        set_target_properties(${TARGET_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
        )
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND codesign --force --sign "-" $<TARGET_FILE_DIR:${TARGET_NAME}>/../..
            COMMAND mkdir -p Payload
            COMMAND cp -R $<TARGET_FILE_DIR:${TARGET_NAME}>/../.. Payload/
            COMMAND zip -r ${TARGET_NAME}.ipa Payload
        )
    endif()
endfunction()

function(cmn_package_web_bundle TARGET_NAME)
    if(EMSCRIPTEN)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/index.html"
"<!DOCTYPE html>
<html>
<body>
    <canvas id=\"canvas\" oncontextmenu=\"event.preventDefault()\"></canvas>
    <script type=\"text/javascript\">
        var Module = {
            canvas: (function() { return document.getElementById('canvas'); })()
        };
    </script>
    <script async type=\"text/javascript\" src=\"${TARGET_NAME}.js\"></script>
</body>
</html>
")
    endif()
endfunction()
