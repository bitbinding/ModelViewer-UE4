#pragma once

#include <string>
#include <vector>


class EncodeTransfer
{
public:

	static unsigned short GBKCodeToUTF16Code(unsigned short c0);
	static unsigned short UTF16CodeToGBKCode(unsigned short c0);

	static std::string UTF8ToGBK(const std::string& str0);
	static std::string GBKToUTF8(const std::string& str0);
};
