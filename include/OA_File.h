#ifndef _OPENAPI_FILE_H_
#define _OPENAPI_FILE_H_

#include <string>
#include <vector>

namespace OpenAPIxx {

    class FileInfo {
    public:
    };

class File{
public:
	File(const std::string& path);
    int create();
    int getName();
    int getParentDir();
    int getInfo();
    int getType();
    bool isExist();
    int copyTo();
    int moveTo();
    int remove();

    static int IsExist(const std::string& path, bool& existFlag);
	static int Copy(const std::string& src, const std::string& dest);
    static int Move(const std::string& src, const std::string& dest);
    static int Remove(const std::string& file);
};

class Dir : public File{
public:
    Dir(const std::string& path);
    int getContentList(std::vector<OpenAPIxx::File>& files);
};

}

typedef OpenAPIxx::File OAFile;

#endif//_OPENAPI_FILE_H_
