using namespace std;
/*
SDL_winrt_main_NonXAML.cpp, placed in the public domain by David Ludwig  3/13/14
*/

#include <sstream>
#include <string>
#include "SDL_main.h"
#include <wrl.h>

/* At least one file in any SDL/WinRT app appears to require compilation
with C++/CX, otherwise a Windows Metadata file won't get created, and
an APPX0702 build error can appear shortly after linking.

The following set of preprocessor code forces this file to be compiled
as C++/CX, which appears to cause Visual C++ 2012's build tools to
create this .winmd file, and will help allow builds of SDL/WinRT apps
to proceed without error.

If other files in an app's project enable C++/CX compilation, then it might
be possible for SDL_winrt_main_NonXAML.cpp to be compiled without /ZW,
for Visual C++'s build tools to create a winmd file, and for the app to
build without APPX0702 errors.  In this case, if
SDL_WINRT_METADATA_FILE_AVAILABLE is defined as a C/C++ macro, then
the #error (to force C++/CX compilation) will be disabled.

Please note that /ZW can be specified on a file-by-file basis.  To do this,
right click on the file in Visual C++, click Properties, then change the
setting through the dialog that comes up.
*/
#ifndef SDL_WINRT_METADATA_FILE_AVAILABLE
#ifndef __cplusplus_winrt
#error SDL_winrt_main_NonXAML.cpp must be compiled with /ZW, otherwise build errors due to missing .winmd files can occur.
#endif
#endif

/* Prevent MSVC++ from warning about threading models when defining our
custom WinMain.  The threading model will instead be set via a direct
call to Windows::Foundation::Initialize (rather than via an attributed
function).

To note, this warning (C4447) does not seem to come up unless this file
is compiled with C++/CX enabled (via the /ZW compiler flag).
*/
#ifdef _MSC_VER
#pragma warning(disable:4447)
#endif

/* Make sure the function to initialize the Windows Runtime gets linked in. */
#ifdef _MSC_VER
#pragma comment(lib, "runtimeobject.lib")
#endif

extern "C" int instead_main(int argc, char** argv);

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if (FAILED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED))) {
		return 1;
	}

	SDL_WinRTRunApp(SDL_main, NULL);
	return 0;
}

char* convertToCString(Platform::String^ string)
{
	if (!string)
	{
		return NULL;
	}
	std::wstring stringW(string->Begin());
	std::string stringA(stringW.begin(), stringW.end());
	return strdup(stringA.c_str());
}

char* convertFolderNameFromPlatformString(Platform::String^ folder, std::string subfolder)
{
	std::wstring folderNameW(folder->Begin());
	std::string folderNameA(folderNameW.begin(), folderNameW.end());
	std::stringstream ss;
	ss << folderNameA;
	if (!subfolder.empty()) {
		ss << "\\" << subfolder;
	}
	std::string s = ss.str();
	return strdup(s.c_str());
}

float getMaxLength(float a, float b)
{
	return (a > b) ? a : b;
}

float getMinLength(float a, float b)
{
	return (a < b) ? a : b;
}

char* getModesString()
{
#if _WIN32_WINNT >= 0x0A00
	// see https://stackoverflow.com/questions/31936154/get-screen-resolution-in-win10-uwp-app
	auto bounds = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->VisibleBounds;
	auto scaleFactor = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawPixelsPerViewPixel;
	auto width = bounds.Width*scaleFactor;
	auto height = bounds.Height*scaleFactor;
#else
	auto devices = Windows::Devices::Input::PointerDevice::GetPointerDevices();
	if (!devices)
	{
		return NULL;
	}
	auto firstDevice = devices->GetAt(0);
	if (!firstDevice)
	{
		return NULL;
	}
	auto screen = firstDevice->ScreenRect;
	auto width = screen.Width;
	auto height = screen.Height;
#endif
	std::stringstream ss;
	float min_length = getMinLength(width, height);
	float max_length = getMaxLength(width, height);
	ss << ((int)min_length) << "x" << ((int)max_length) << "," << ((int)max_length) << "x" << ((int)min_length);
	return strdup(ss.str().c_str());
}

#if _WIN32_WINNT >= 0x0A00
void toggleToFullscreen()
{
	auto av = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
	if (!av->IsFullScreenMode)
	{
		if (av->TryEnterFullScreenMode())
		{
			av->FullScreenSystemOverlayMode = Windows::UI::ViewManagement::FullScreenSystemOverlayMode::Minimal;
		}
	}
	else
	{
		av->ExitFullScreenMode();
		av->FullScreenSystemOverlayMode = Windows::UI::ViewManagement::FullScreenSystemOverlayMode::Standard;
	}
}
#endif

char* convertFolderNameFromPlatformString(Platform::String^ folder)
{
	return convertFolderNameFromPlatformString(folder, "");
}

/*
You must patch SDL_winrtapp_direct3d.cpp to be able to use getGameFile()

void SDL_WinRTApp::OnAppActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    // <AddedCode>
    Platform::String^ key = ref new Platform::String(L"FilePathToOpen");
    Platform::String^ empty = ref new Platform::String(L"");
    if (args != nullptr)
    {
        if (args->Kind == Windows::ApplicationModel::Activation::ActivationKind::File)
        {
            auto fileArgs = (Windows::ApplicationModel::Activation::FileActivatedEventArgs^) args;
            auto strFilePath = fileArgs->Files->GetAt(0)->Path;
            CoreWindow::GetForCurrentThread()->CustomProperties->Insert(key, strFilePath);
        }
        else
        {
            CoreWindow::GetForCurrentThread()->CustomProperties->Insert(key, empty);
        }
    }
    else
    {
        CoreWindow::GetForCurrentThread()->CustomProperties->Insert(key, empty);
    }
	// </AddedCode>
    CoreWindow::GetForCurrentThread()->Activate();
}
*/

char* getGameFile()
{
	auto window = Windows::UI::Core::CoreWindow::GetForCurrentThread();
	auto props = window->CustomProperties;
	Platform::String^ key = ref new Platform::String(L"FilePathToOpen");
	auto path = (Platform::String^) (props->Lookup(key));
	return convertToCString(path);
}

/*
void logToFile(char *parentFolder, char *text)
{
	char* parent = strdup(parentFolder);
	FILE* f = fopen(strcat(parent, "\\file.txt"), "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}

	if (text)
	{
		fprintf(f, "Some text: %s\n", text);
	}
	if (f)
	{
		fclose(f);
	}
	if (parent)
	{
		free(parent);
	}
}
*/

static char* tmppath = NULL;
static boolean nostdgames = false;
static boolean hires = true;
static boolean standalone = false;
static char* game = NULL;
static boolean nosound = false;
static boolean owntheme = true;
static char* theme = NULL;

extern "C" int create_dir_if_needed(char *path);
extern "C" int dir_exists(char *path);

int main(int argc, char *argv[])
{
	int err;
	Platform::String^ installationFolder = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
	Platform::String^ localFolder = Windows::Storage::ApplicationData::Current->LocalFolder->Path;  // C:/Users/user/AppData/Local/Packages/<GUID>/LocalState
	Platform::String^ tempFolder = Windows::Storage::ApplicationData::Current->TemporaryFolder->Path;
	char* curdir = convertFolderNameFromPlatformString(installationFolder);
	char* appdata = convertFolderNameFromPlatformString(localFolder, "appdata");
	char* appdata_games = convertFolderNameFromPlatformString(localFolder, "appdata\\games");
	char* appdata_themes = convertFolderNameFromPlatformString(localFolder, "appdata\\themes");
	create_dir_if_needed(appdata);
	tmppath = convertFolderNameFromPlatformString(tempFolder);
	char* _argv[25];
	int n = 1;
	_argv[0] = curdir;
	if (nostdgames)
	{
		_argv[n++] = "-nostdgames";
	}
	_argv[n++] = "-fullscreen";
#if _WIN32_WINNT >= 0x0A00
	toggleToFullscreen();
#endif
	if (standalone)
	{
		_argv[n++] = "-standalone";
	}
	_argv[n++] = (hires) ? "-hires" : "-nohires";
	if (appdata)
	{
		_argv[n++] = "-appdata";
		_argv[n++] = appdata;
	}
	if (dir_exists(appdata_games))
	{
		_argv[n++] = "-gamespath";
		_argv[n++] = appdata_games;
	}
	if (dir_exists(appdata_themes))
	{
		_argv[n++] = "-themespath";
		_argv[n++] = appdata_themes;
	}
	if (game)
	{
		_argv[n++] = "-game";
		_argv[n++] = game;
	}
	if (nosound)
	{
		_argv[n++] = "-nosound";
	}
	if (owntheme)
	{
		_argv[n++] = "-owntheme";
	}
	if (theme)
	{
		_argv[n++] = "-theme";
		_argv[n++] = theme;
	}
	char* game_file = getGameFile();
	if (game_file)
	{
		_argv[n++] = game_file;
	}
	/*
	char* modes = getModesString();
	if (modes)
	{
	_argv[n++] = "-modes";
	_argv[n++] = modes;
	}
	*/
	_argv[n] = NULL;
	Windows::Graphics::Display::DisplayInformation::AutoRotationPreferences = Windows::Graphics::Display::DisplayOrientations::Portrait;
	err = instead_main(n, _argv);
	//free(modes);
	if (game_file)
	{
		free(game_file);
	}
	free(tmppath);
	free(appdata_themes);
	free(appdata_games);
	free(appdata);
	free(curdir);
	return err;
}

extern "C" void getAppTempDir(char *lpPathBuffer)
{
	strcpy(lpPathBuffer, tmppath);
}
/*
extern "C" void rotate_landscape(void)
{
	Windows::Graphics::Display::DisplayInformation::AutoRotationPreferences = Windows::Graphics::Display::DisplayOrientations::Landscape;
}

extern "C" void rotate_portrait(void)
{
	Windows::Graphics::Display::DisplayInformation::AutoRotationPreferences = Windows::Graphics::Display::DisplayOrientations::Portrait;
}

extern "C" void unlock_rotation(void)
{
	Windows::Graphics::Display::DisplayInformation::AutoRotationPreferences = Windows::Graphics::Display::DisplayOrientations::Landscape | Windows::Graphics::Display::DisplayOrientations::Portrait;
}
*/