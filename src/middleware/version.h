/*!

\author         Oliver Blaser
\date           06.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#ifndef _VERSION_H_
#define _VERSION_H_

#include <iostream>
#include <string>
#include <vector>

class Version
{
public:
    Version();
    Version(int major, int minor, int revision);

    inline int getMajor() const;
    inline int getMinor() const;
    inline int getRevision() const;

    std::vector<int> toVector() const;
    int toArray(int* dest) const;
    std::string toString() const;

    friend bool operator<(const Version& left, const Version& right);
    friend bool operator>(const Version& left, const Version& right);
    friend bool operator<=(const Version& left, const Version& right);
    friend bool operator>=(const Version& left, const Version& right);
    friend bool operator==(const Version& left, const Version& right);
    friend bool operator!=(const Version& left, const Version& right);
    friend std::ostream& operator<<(std::ostream& os, const Version& v);

private:
    int major;
    int minor;
    int revision;
};

#endif // _VERSION_H_
