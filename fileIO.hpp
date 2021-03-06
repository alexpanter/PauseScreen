#ifndef FILEIO_HPP
#define FILEIO_HPP

#include <string>
#include <fstream>
#include <iostream>

namespace fileIO
{
    // returns the extension for a target file string
    std::string getFileExtension(std::string file)
    {
        return file.substr(file.find_last_of(".") + 1);
    }

    // auxillary function to read from a (shader) file
    std::string readFileContents(const char* path)
    {
        std::string content;
        std::ifstream fileStream(path, std::ios::in);

        if(!fileStream.is_open()) {
            std::string msg = "Could not read file '";
            msg += path;
            msg += "'.";

            PRINT_MSG_CERR(msg);
            return "";
        }

        std::string line = "";
        while(!fileStream.eof()) {
            std::getline(fileStream, line);
            content.append(line + "\n");
        }

        fileStream.close();
        return content;
    }

    // intended functionality to retrieve platform-dependent
    // directory separator
    char getPlatformSeparator()
    {
        return '/';
    }

    // creating a platform-dependent string path from the
    // specified target file
    std::string getPlatformFilePath(const char* file)
    {
        char sep = getPlatformSeparator();

        std::string filepath;

        for(const char* ptr = file; *ptr != '\0'; ptr++) {
            if(*ptr == '|') {
                filepath += sep;
            } else {
                filepath += *ptr;
            }
        }

        return filepath;
    }

    // almost same as above, e.g. usage:
    // getPlatformPath("path1.path2.path3") -> "path1/path2/path3/"
    std::string getPlatformPath(const char* path)
    {
        return getPlatformFilePath(path) + getPlatformSeparator();
    }
}

#endif // FILEIO_HPP
