//SFHR_Modloader.cpp
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>

using namespace std;

const char* version = "v1.3";

//Get the Directory of the .exe file of the Modloader
string GetExecutableDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    size_t pos = string(buffer).find_last_of("\\/");
    return string(buffer).substr(0, pos);
}

//Get all the .dll files in the same directory as the .exe file locates
vector<string> GetModDllFilesInDirectory(const string& directory) {

    vector<string> modDllFiles;
    WIN32_FIND_DATA findFileData;

    //HANDLE hFind = FindFirstFile((directory + "\\[Mod]*.dll").c_str(), &findFileData);
    HANDLE hFind = FindFirstFile((directory + "\\*.dll").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {

            modDllFiles.push_back(findFileData.cFileName);

        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
    }

    return modDllFiles;
}

//Get the process Identifier of the specified process
DWORD GetProcessID(const char* processName) {

    DWORD processID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE) {

        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(processEntry);

        if (Process32First(hSnap, &processEntry)) {

            do {

                if (_stricmp(processEntry.szExeFile, processName) == 0) {

                    processID = processEntry.th32ProcessID;
                    break;
                }

            } while (Process32Next(hSnap, &processEntry));
        }
    }

    CloseHandle(hSnap);
    return processID;
}

int main() {

    const char* processName = "strike force heroes.exe";
    DWORD processID = GetProcessID(processName);

    if (processID == 0) {

        //Create a pop up that prompts error if the target process is not found
        MessageBox(NULL, "Failed to find the game...\nPlease make sure the game is running before running the Modloader", "ERROR: Where are the Heroes?", NULL);
        
        return 0;//Exit the program
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, processID);

    if (hProcess && hProcess != INVALID_HANDLE_VALUE) {

        string exeDirectory = GetExecutableDirectory();
        vector<string> modDllFiles = GetModDllFilesInDirectory(exeDirectory);


        for (const auto& modDllFile : modDllFiles) {

            string modDllPath = (exeDirectory + "\\" + modDllFile);

            void* location = VirtualAllocEx(hProcess, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

            if (location) {
                WriteProcessMemory(hProcess, location, modDllPath.c_str(), modDllPath.size() + 1, 0);
            }

            HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, location, 0, 0);

            if (hThread) {
                CloseHandle(hThread);
            }
        }
    }


    if (hProcess) 
    {
        CloseHandle(hProcess);
    }

    return 0;
}
