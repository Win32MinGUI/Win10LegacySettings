# Win10LegacySettings
A simple utility which provides quick accessing on the legacy features on Windows 10

![Alt Text](https://github.com/Win32MinGUI/Win10LegacySettings/raw/main/Preview/Win10LegacySettings.png)


- Various settings integrated into one single HTML application
- Quick switch on LAN proxy server settings
- Minimize to system tray
- Various namespace switches of "This PC/My Computer"
- The TrayHook part is heavily influenced by the [RBTray] project
- The keyboard handling is from [OpenJS]


- Win10LegacySettingsLauncher command line options examples
```
When no previous instances exist:
  Win10LegacySettingsLauncher.exe                                     Launch the Win10LegacySettings HTML Application
  Win10LegacySettingsLauncher.exe --minimize-after-milliseconds 4000  Launch the Win10LegacySettings HTML Application and minimize it to tray after 4 seconds

When previous instance already exists:
  Win10LegacySettingsLauncher.exe                                     Bring the existing Win10LegacySettings HTML Application to the front
  Win10LegacySettingsLauncher.exe --minimize                          Send message and minimize the existing Win10LegacySettings HTML Application to tray
```

[RBTray]: <http://rbtray.sourceforge.net/>
[OpenJS]: <http://www.openjs.com/scripts/events/keyboard_shortcuts/>

