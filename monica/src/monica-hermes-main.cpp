/**
Authors: 
Dr. Claas Nendel <claas.nendel@zalf.de>
Xenia Specka <xenia.specka@zalf.de>
Michael Berg <michael.berg@zalf.de>

Maintainers: 
Currently maintained by the authors.

This file is part of the MONICA model. 
Copyright (C) 2007-2013, Leibniz Centre for Agricultural Landscape Research (ZALF)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>

#include "db/abstract-db-connections.h"

#include "debug.h"
#include "simulation.h"

#include "monica-parameters.h"

using namespace std;

/**
 * Main routine of stand alone model.
 * @param argc Number of program's arguments
 * @param argv Pointer of program's arguments
 */
int main(int argc, char** argv)
{
#if WIN32
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");

    //use the non-default db-conections-core.ini
		Db::dbConnectionParameters("db-connections.ini");
#endif
				
		Monica::runWithHermesData(argc == 2 ? string(argv[1]) + Monica::pathSeparator() : "");
}
