#pragma once

#include <string>

namespace StringUtility
{
	std::wstring ConvertToWString(const std::string&);
	std::string ConvertToString(const std::wstring&);
};

