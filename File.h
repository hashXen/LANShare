#ifndef FILE_H
#define FILE_H

#include <string>

#include <openssl/

using std::string;

class File
{
public:
    File();
    string Checksum();
};

#endif // FILE_H
