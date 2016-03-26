#include "aux_misc.h"

#include <sstream>
#include <string>
using namespace std;

string IntToStr(int i)
{
	ostringstream oss;
	oss << i;
	return oss.str();
}
// ---------------------------------------------------------------------