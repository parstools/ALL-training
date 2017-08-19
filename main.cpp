#include "Exception.h"
#include "Parser.h"

int main()
{
	try {
		Parser parser("gram09.dat");
		parser.goInput("aaaac");
		//parser.goInput("bc");
	}
	catch (Exception &ex)
	{
		printf("exception: %s\n", ex.what());
	}
	return 0;
}

