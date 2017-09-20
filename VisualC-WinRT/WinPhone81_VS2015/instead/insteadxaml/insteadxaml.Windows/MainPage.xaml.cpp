//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include <sstream>
#include <string>
#include "winrt.h"
#include "MainPage.xaml.h"
#include <SDL.h>

using namespace insteadxaml;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage::MainPage()
{
	InitializeComponent();
}

extern "C" int instead_main(int argc, char** argv);

char* convertFolderNameFromWcharToASCII(Platform::String^ folder, std::string subfolder)
{
	std::wstring folderNameW(folder->Begin());
	std::string folderNameA(folderNameW.begin(), folderNameW.end());
	std::stringstream ss;
	ss << folderNameA;
	if (!subfolder.empty()) {
		ss << "\\" << subfolder;
	}
	std::string s = ss.str();
	return _strdup(s.c_str());
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
	//Windows::Graphics::Display::DisplayInformation^ info = Windows::Graphics::Display::DisplayInformation::NativeOrientation()//::GetForCurrentView();
	auto devices = Windows::Devices::Input::PointerDevice::GetPointerDevices();
	if (!devices) {
		return NULL;
	}
	auto firstDevice = devices->GetAt(0);
	if (!firstDevice) {
		return NULL;
	}
	auto screen = firstDevice->ScreenRect;
	std::stringstream ss;
	float min_length = getMinLength(screen.Width, screen.Height);
	float max_length = getMaxLength(screen.Width, screen.Height);
	ss << ((int)min_length) << "x" << ((int)max_length) << "," << ((int)max_length) << "x" << ((int)min_length);
	return _strdup(ss.str().c_str());
}

char* convertFolderNameFromWcharToASCII(Platform::String^ folder)
{
	return convertFolderNameFromWcharToASCII(folder, "");
}

static char* tmppath = NULL;
static boolean nostdgames = false;
static boolean hires = false;
static boolean standalone = false;
static char* game = NULL;
static boolean nosound = true;
static boolean owntheme = true;
static char* theme = NULL;

extern "C" int create_dir_if_needed(char *path);

void getAppTempDir(char *lpPathBuffer)
{
	strcpy(lpPathBuffer, tmppath);
}

void rotate_landscape(void)
{
	Windows::Graphics::Display::DisplayInformation::AutoRotationPreferences = Windows::Graphics::Display::DisplayOrientations::Landscape;
}

void rotate_portrait(void)
{
	Windows::Graphics::Display::DisplayInformation::AutoRotationPreferences = Windows::Graphics::Display::DisplayOrientations::Portrait;
}

void unlock_rotation(void)
{
	Windows::Graphics::Display::DisplayInformation::AutoRotationPreferences = Windows::Graphics::Display::DisplayOrientations::Landscape | Windows::Graphics::Display::DisplayOrientations::Portrait;
}

void insteadxaml::MainPage::OnLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    /* Encode a pointer to the 'sdlPanel' XAML control inside a C-style
     * string.  This will be passed into the SDL app's main() function,
     * which will have the opportunity to decode, then use it.
     */
    char nativeWindowString[32];
    IInspectable *nativeWindow = reinterpret_cast<IInspectable *>(sdlPanel);
    SDL_snprintf(nativeWindowString, SDL_arraysize(nativeWindowString), "%" SDL_PRIu64, (unsigned long long)nativeWindow);

	int err;
	Platform::String^ installationFolder = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
	Platform::String^ localFolder = Windows::Storage::ApplicationData::Current->LocalFolder->Path;  // C:/Users/user/AppData/Local/Packages/<GUID>/LocalState
	Platform::String^ tempFolder = Windows::Storage::ApplicationData::Current->TemporaryFolder->Path;
	char* curdir = convertFolderNameFromWcharToASCII(installationFolder);
	char* appdata = convertFolderNameFromWcharToASCII(localFolder, "appdata");
	create_dir_if_needed(appdata);
	tmppath = convertFolderNameFromWcharToASCII(tempFolder);
	char* modes = getModesString();
	char* _argv[22];
	int n = 1;
	_argv[0] = curdir;
	_argv[n++] = "-native_window";
	_argv[n++] = nativeWindowString;
	if (nostdgames)
	{
		_argv[n++] = "-nostdgames";
	}
	_argv[n++] = "-fullscreen";
	if (modes)
	{
		_argv[n++] = "-modes";
		_argv[n++] = modes;
	}
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
	// -gamespath
	// -themespath
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
	_argv[n] = NULL;
	err = instead_main(n, _argv);
	free(tmppath);
	free(appdata);
	free(curdir);
}