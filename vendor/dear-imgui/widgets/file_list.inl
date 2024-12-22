#include <bx/bx.h>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace ImGui
{
    ImFileInfo::ImFileInfo(const char* name, int64_t size) : Name(name), Size(size) {}

    ImFileInfo::~ImFileInfo() {}

#if defined(_WIN32)
    std::string ConvertWideToNarrow(const wchar_t* wideStr)
    {
        int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
        std::string narrowStr(bufferSize, 0);
        WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, &narrowStr[0], bufferSize, nullptr, nullptr);
        return narrowStr;
    }
#endif

    void ImFileList::ChDir(const char* path)
    {
#if BX_PLATFORM_PS4
        BX_UNUSED(path);
#elif defined(_WIN32)
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile((std::wstring(path, path + strlen(path)) + L"\\*").c_str(), &findFileData);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            FileList.clear();
            do
            {
                const bool isDir = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                std::string fileName = ConvertWideToNarrow(findFileData.cFileName);
                if (fileName == "..")
                {
                    FileList.push_back(ImFileInfo(fileName.c_str(), -1));
                }
                else if (fileName != ".")
                {
                    if (isDir)
                    {
                        FileList.push_back(ImFileInfo(fileName.c_str(), -1));
                    }
                    else
                    {
                        FileList.push_back(ImFileInfo(fileName.c_str(), findFileData.nFileSizeLow));
                    }
                }
            }
            while (FindNextFile(hFind, &findFileData) != 0);
            FindClose(hFind);
        }
#else
        DIR* dir = opendir(path);
        if (NULL != dir)
        {
            FileList.clear();

            for (dirent* item = readdir(dir); NULL != item; item = readdir(dir))
            {
                if (0 == ImStricmp(item->d_name, ".."))
                {
                    FileList.push_back(ImFileInfo(item->d_name, -1));
                }
                else if (0 != ImStricmp(item->d_name, "."))
                {
                    if (item->d_type & DT_DIR)
                    {
                        FileList.push_back(ImFileInfo(item->d_name, -1));
                    }
                    else
                    {
                        struct stat statbuf;
                        stat(item->d_name, &statbuf);
                        FileList.push_back(ImFileInfo(item->d_name, statbuf.st_size));
                    }
                }
            }

            closedir(dir);
        }
#endif // BX_PLATFORM_PS4
    }

    void ImFileList::Draw()
    {
        BeginChild("##file_list", ImVec2(0.0f, 0.0f));
        PushFont(Font::Mono);

        PushItemWidth(-1);
        if (BeginListBox("##empty", ImVec2(0.0f, 0.0f)))
        {
            const float lineHeight = GetTextLineHeightWithSpacing();

            ImString chdir;

            int pos = 0;
            ImGuiListClipper clipper;
            clipper.Begin(FileList.size(), lineHeight);
            clipper.Step();

            for (FileInfoArray::const_iterator it = FileList.begin(), itEnd = FileList.end(); it != itEnd; ++it)
            {
                if (pos >= clipper.DisplayStart && pos < clipper.DisplayEnd)
                {
                    PushID(pos);

                    const bool isDir = -1 == it->Size;
                    bool isSelected = Pos == pos;

                    bool clicked = Selectable(it->Name.CStr(), &isSelected);
                    SameLine(150);
                    if (isDir)
                    {
                        Text("%10s", "<DIR>");
                    }
                    else
                    {
                        Text("%10" PRId64, it->Size);
                    }

                    if (clicked)
                    {
                        if (0 == strcmp(it->Name.CStr(), ".."))
                        {
                            chdir = it->Name;
                        }

                        Pos = pos;

                        if (isDir)
                        {
                            chdir = it->Name;
                        }
                    }

                    PopID();
                }
                ++pos;
            }
            clipper.End();

            EndListBox();

            if (!chdir.IsEmpty())
            {
                ChDir(chdir.CStr());
            }
        }

        PopFont();
        EndChild();
    }

} // namespace ImGui
