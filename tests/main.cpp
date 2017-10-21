#include <houio/json.h>

#include <iostream>
#include <fstream>





// prints the file content of given houdini file
void printLog( const std::string &path, std::ostream *out )
{
	std::ifstream in( path.c_str(), std::ios_base::in | std::ios_base::binary );
	houio::json::JSONLogger logger(*out);
	houio::json::Parser p;
	p.parse( &in, &logger );
}





int main(void)
{
	printLog( std::string(TESTS_FILE_PATH)+"/test_box.bgeo", &std::cout );
	printLog( std::string(TESTS_FILE_PATH)+"/test_volume.bgeo", &std::cout );
	printLog( std::string(TESTS_FILE_PATH)+"/test_box.geo", &std::cout );
	printLog( std::string(TESTS_FILE_PATH)+"/test_volume.geo", &std::cout );

        return 0;
}
