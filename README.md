# CMSLauncherLite
CN MapleStory v15-v69 Launcher

## Feature
- Remove locale check: Still displays unknown text
- Redirect: Modify connection ip
- Bypass AntiCheat: Let's successfully enter the game without interruption 
- Mount IMG file(optional): Check if `Data` folder exists? img mode or wz default mode
- Wnd optimization
    - Window Mode: Enable older clients to run in windowed mode at the recent displays
    - Skip Startup Logo: Quickly enter the login screen without displaying the logo
    - Fix Cursor Focus Issue: The cursor loses focus in v15-v35 when returning to the game window from outside
    - Customize game window title and Enable multiple game instances

## Test version
The supported version range is v15-v69   
Please tell me if you find any errors during login to these versions
### AntiCheat
- v15 removed the security check
- v48 removed the GameGuard update
- Versions above v48 can't use AOBScan during startup
    - Please use `gMapleR.WriteCode(uAddress, wCode)` as a replacement for AOBScan 
    - Alternatively use the unpacked version and rename it to `MapleStory.exe`
    - The AOB list is the same for v48-v69
### IMG
- Supported version range is v15-v48
- Use custom encryption key
- Convert `Data.wz` to `Data` folder using `HaRepacker`

## How to use
### Debug
1. Set `IS_DEV` is true
2. Set `REMOTE_DIR` to MapleStory directory
3. Set options in `Config.h`
4. Build solution with `Release x86` mode
5. Run it in Visual Studio will inject hook by remote
6. Recommend using [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) to see debug log

### Release
1. Set `IS_DEV` is false
2. Build solution with `Release x86` mode
3. Put `Launcher.exe` and `Hook.dll` into MapleStory directory
4. Run `Launcher.exe` to connect login server