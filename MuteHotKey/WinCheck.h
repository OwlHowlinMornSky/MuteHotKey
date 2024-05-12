#pragma once

#include <string_view>

void ParseErrorCode(long code, std::wstring_view errorText);

void ParseWin32Error(std::wstring_view errorText);
