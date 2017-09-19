//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
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


void insteadxaml::MainPage::OnLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    /* Encode a pointer to the 'sdlPanel' XAML control inside a C-style
     * string.  This will be passed into the SDL app's main() function,
     * which will have the opportunity to decode, then use it.
     */
    char nativeWindowString[32];
    const char *argv[] = {
        "",
        "--native_window", (const char *)nativeWindowString
    };
    IInspectable *nativeWindow = reinterpret_cast<IInspectable *>(sdlPanel);
    SDL_snprintf(nativeWindowString, SDL_arraysize(nativeWindowString), "%" SDL_PRIu64, (unsigned long long)nativeWindow);
    SDL_main(SDL_arraysize(argv), (char **)argv);
}
