/*!

\author         Oliver Blaser
\date           06.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2022 Oliver Blaser

*/

#include "version.h"

Version::Version()
    : major(0), minor(0), revision(0)
{
}

Version::Version(int major, int minor, int revision)
    : major(major), minor(minor), revision(revision)
{
}

inline int Version::getMajor() const
{
    return major;
}

inline int Version::getMinor() const
{
    return minor;
}

inline int Version::getRevision() const
{
    return revision;
}

std::vector<int> Version::toVector() const
{
    std::vector<int> arr;

    arr.push_back(major);
    arr.push_back(minor);
    arr.push_back(revision);

    return arr;
}

int Version::toArray(int* dest) const
{
    if (!dest) return 1;

    *dest = major;
    *(dest + 1) = minor;
    *(dest + 2) = revision;

    return 0;
}

std::string Version::toString() const
{
    return std::to_string(major) + '.' + std::to_string(minor) + '.' + std::to_string(revision);
}

bool operator<(const Version& left, const Version& right)
{
    if (left.major > right.major) return false;
    if (left.major < right.major) return true;

    if (left.minor > right.minor) return false;
    if (left.minor < right.minor) return true;

    if (left.revision > right.revision) return false;
    if (left.revision < right.revision) return true;

    return false;
}

bool operator>(const Version& left, const Version& right)
{
    if (left.major > right.major) return true;
    if (left.major < right.major) return false;

    if (left.minor > right.minor) return true;
    if (left.minor < right.minor) return false;

    if (left.revision > right.revision) return true;
    if (left.revision < right.revision) return false;

    return false;
}

bool operator<=(const Version& left, const Version& right)
{
    return !(left > right);
}

bool operator>=(const Version& left, const Version& right)
{
    return !(left < right);
}

bool operator==(const Version& left, const Version& right)
{
    if ((left.major == right.major) &&
        (left.minor == right.minor) &&
        (left.revision == right.revision))
    {
        return true;
    }

    return false;
}

bool operator!=(const Version& left, const Version& right)
{
    return !(left == right);
}

std::ostream& operator<<(std::ostream& os, const Version& v)
{
    os << v.toString();
    return os;
}
