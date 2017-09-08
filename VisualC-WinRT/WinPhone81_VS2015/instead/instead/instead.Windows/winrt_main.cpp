using namespace std;
/*
SDL_winrt_main_NonXAML.cpp, placed in the public domain by David Ludwig  3/13/14
*/

#include <future>
#include <ppltasks.h>
#include <string>
#include "SDL_main.h"
#include "winrt.h"
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

char* convertFolderNameFromWcharToASCII(Platform::String^ folder)
{
	std::wstring folderNameW(folder->Begin());
	std::string folderNameA(folderNameW.begin(), folderNameW.end());
	return strdup(folderNameA.c_str());
}

static char* tmppath = NULL;

Windows::Storage::IStorageItem^ doit1(Windows::Foundation::IAsyncOperation<Windows::Storage::IStorageItem^>^ task)
{
	//auto handle = std::async(std::launch::async, &Windows::Foundation::IAsyncOperation<Windows::Storage::IStorageItem ^>::GetResults);
	//auto result = handle.get();
	Windows::Storage::IStorageItem^ res;
	concurrency::create_task(task).then([](Windows::Storage::IStorageItem^ result)
	{
		auto r = result;
	});
	return res;
	/*
	auto result = task->GetResults();
	task->Close();  // Should be in finally...
	return result;*/
}

Windows::Storage::StorageFile^ doit4(Windows::Foundation::IAsyncOperation<Windows::Storage::StorageFile^>^ task)
{
	auto result = task->GetResults();
	task->Close();  // Should be in finally...
	return result;
}

Windows::Storage::StorageFolder^ doit2(Windows::Foundation::IAsyncOperation<Windows::Storage::StorageFolder^>^ task)
{
	auto result = task->GetResults(); 
	task->Close();  // Should be in finally...
	return result;
}

Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFile^>^ doit3(Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFile^>^>^ task)
{
	auto result = task->GetResults();
	task->Close();  // Should be in finally...
	return result;
}

Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFolder^>^ doit5(Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFolder^>^>^ task)
{
	auto result = task->GetResults();
	task->Close();  // Should be in finally...
	return result;
}

void doit_a(Windows::Foundation::IAsyncAction^ task)
{
	//task->GetResults();
	task->Close();  // Should be in finally...
}

// see https://stackoverflow.com/questions/24098740/how-to-copy-a-folder-in-a-windows-store-app
void copydir(
	Windows::Storage::StorageFolder^ source,
	Windows::Storage::StorageFolder^ destination)
{ 
	// If the destination exists, delete it.
	auto targetFolder = doit1(destination->TryGetItemAsync(source->DisplayName));
	//if (targetFolder is StorageFolder) check omitted
	if (targetFolder) {
		doit_a(targetFolder->DeleteAsync());
	}

	targetFolder = doit2(destination->CreateFolderAsync(source->DisplayName));

	// Get all files (shallow) from source
	auto queryOptions = ref new Windows::Storage::Search::QueryOptions();
	queryOptions->IndexerOption = Windows::Storage::Search::IndexerOption::DoNotUseIndexer;  // Avoid problems cause by out of sync indexer
	queryOptions->FolderDepth = Windows::Storage::Search::FolderDepth::Shallow;
	
	auto queryFiles = source->CreateFileQueryWithOptions(queryOptions);
	auto files = doit3(queryFiles->GetFilesAsync());
	for (int i = 0; i < files->Size; ++i)
	{
		auto file = files->GetAt(i);
		doit4(file->CopyAsync((Windows::Storage::StorageFolder^) targetFolder, file->Name, Windows::Storage::NameCollisionOption::ReplaceExisting));
	}

	// Get all folders (shallow) from source
	auto queryFolders = source->CreateFolderQueryWithOptions(queryOptions);
	auto folders = doit5(queryFolders->GetFoldersAsync());

	// For each folder call copy with new destination as destination
	for (int i = 0; i < folders->Size; ++i)
	{
		auto storageFolder = folders->GetAt(i);
		copydir((Windows::Storage::StorageFolder^) targetFolder, storageFolder);
	}
}

int main(int argc, char *argv[])
{
	int err;
	Windows::Storage::StorageFolder^ installationFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
	Windows::Storage::StorageFolder^ localFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
	//Platform::String^ localFolderPath = localFolder->Path;  // C:/Users/user/AppData/Local/Packages/<GUID>/LocalState
	/*auto tt = std::async(std::launch::async, doit1, installationFolder->TryGetItemAsync("lang")).get();
	copydir((Windows::Storage::StorageFolder^) tt, localFolder);*/
    char* curdir = convertFolderNameFromWcharToASCII(installationFolder->Path);
	Platform::String^ tempFolder = Windows::Storage::ApplicationData::Current->TemporaryFolder->Path;
	tmppath = convertFolderNameFromWcharToASCII(tempFolder);
	char *argv2[] = { curdir };
	err = instead_main(1, argv2);
	free(tmppath);
	free(curdir);
	return err;
}

void getAppTempDir(char *lpPathBuffer)
{
	strcpy(lpPathBuffer, tmppath);
}