#include "StrUtils.h"

#include <cstring>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr

bool skipBOM(ifstream &in)
{
	char test[4] = { 0 };
	in.read(test, 3);
	if (strcmp(test, "\xEF\xBB\xBF") == 0)
		return true;
	in.seekg(0);
	return false;
}

string trim(const string& str)
{
	const string whitespace = " \t";
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

string trimLeft(const string& str)
{
	const auto strBegin = str.find_first_not_of(" \t");
	if (strBegin == string::npos)
		return ""; // no content
	return str.substr(strBegin, str.length() - strBegin);
}

string trimRight(const string& str)
{
	const auto strEnd = str.find_last_not_of(" \t");
	if (strEnd == string::npos)
		return ""; // no content
	return str.substr(0, strEnd + 1);
}

std::string string_format(const std::string fmt_str, ...) {
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while (1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		strcpy(&formatted[0], fmt_str.c_str());
		va_start(ap, fmt_str);
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return std::string(formatted.get());
}