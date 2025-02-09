/**
Authors: 
Michael Berg <michael.berg@zalf.de>

Maintainers: 
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of 
Landscape Systems Analysis at the ZALF.
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

#ifndef CLIMATE_COMMON_H_
#define CLIMATE_COMMON_H_

#include <vector>
#include <string>

#include "boost/shared_ptr.hpp"

#include "tools/date.h"

//! All climate data related classes.
namespace Climate
{
  /*!
	 * All abstract climate elements which are currently supported by the
	 * Climate classes. A user of the classes has to specify its requested
	 * climate elements in terms of these enumeration values. Usually
	 * with single elements or a list (vector) of elements.
	 */
  enum AvailableClimateData
  {
		day = 0, month, year, tmin, tavg, tmax, precip, precipOrig, globrad, wind,
		sunhours, cloudamount, relhumid, airpress, vaporpress
	};

	//! just a shortcut to the quite long name
	typedef AvailableClimateData ACD;

	/*!
	 * helper function to calculate the current size of the available climate
	 * data enumeration
	 * (this in instead of adding a dummy AvailableClimateDataSize element as
	 * last element -> to not pollute the enum)
	 * @return number of elements in the AvailableClimateData enumeration
	 */
	inline unsigned int availableClimateDataSize() { return int(vaporpress) + 1; }

	//! just a shortcut for a list (vector) of climate data elements
	typedef std::vector<AvailableClimateData> ACDV;

	/*!
	 * helper function
	 * @return a vector of all climate data elements
	 */
	const ACDV& acds();

	/*!
	 * helper function
	 * @param col availalbe climate data element
	 * @return column name of climate data element in CLM database
	 */
	std::string availableClimateData2CLMDBColName(AvailableClimateData col);

	/*!
	 * helper function
	 * @param col available climate data element
	 * @return column name of climate data element in Werex database
	 */
	std::string availableClimateData2WerexColName(AvailableClimateData col);

	/*!
	 * helper function
	 * @param col available climate data element
	 * @return column name of climate data element in WettReg database
	 */
	std::string availableClimateData2WettRegDBColName(AvailableClimateData col);

	std::pair<std::string, int>
	availableClimateData2CarbiocialDBColNameAndScaleFactor(AvailableClimateData col);

	/*!
	 * helper function
	 * @param col available climate data element
	 * @return column name of climate data element in Star database
	 */
  inline std::string availableClimateData2StarDBColName(AvailableClimateData col)
  {
		return availableClimateData2WettRegDBColName(col);
	}



	/*!
	 * helper function and just a different name for the WettReg mapping right now
	 * is here to have some generic name mapping, even though this might be just
	 * useable for display to the user
	 * @param col available climate data element
	 * @return column name of climate data element in default db
	 */
  inline std::string availableClimateData2DBColName(AvailableClimateData col)
  {
		return availableClimateData2WettRegDBColName(col);
	}

	std::string availableClimateData2Name(AvailableClimateData col);

	std::string availableClimateData2unit(AvailableClimateData col);

	//----------------------------------------------------------------------------

  struct YearRange
  {
    YearRange() : fromYear(0), toYear(0) {}
    YearRange(int f, int t) : fromYear(f), toYear(t) {}
    int fromYear, toYear;
    bool isValid() const { return fromYear > 0 && toYear > 0; }
  };

  YearRange snapToRaster(YearRange yr, int raster = 5);

  //----------------------------------------------------------------------------
  
  //! deep copied access to a range of climate data
  class DataAccessor
  {
	public:
		DataAccessor();

		DataAccessor(const Tools::Date& startDate, const Tools::Date& endDate);

		DataAccessor(const DataAccessor& other);

		~DataAccessor(){}

    bool isValid() const { return noOfStepsPossible() > 0; }

    double dataForTimestep(AvailableClimateData acd, unsigned int stepNo) const;

		std::vector<double> dataAsVector(AvailableClimateData acd) const;

		DataAccessor cloneForRange(unsigned int fromStep,
		                           unsigned int numberOfSteps) const;

    unsigned int noOfStepsPossible() const { return _numberOfSteps; }

		Tools::Date startDate() const { return _startDate; }

		Tools::Date endDate() const { return _endDate; }

		unsigned int julianDayForStep(int stepNo) const;

		void addClimateData(AvailableClimateData acd,
		                    const std::vector<double>& data);

    void addOrReplaceClimateData(AvailableClimateData acd,
                                 const std::vector<double>& data);

    bool hasAvailableClimateData(AvailableClimateData acd) const
    {
      return _acd2dataIndex[acd] >= 0;
    }

	private: //state
		Tools::Date _startDate;
		Tools::Date _endDate;

		typedef std::vector<std::vector<double> > VVD;
		boost::shared_ptr<VVD> _data;

		//! offsets to actual available climate data enum numbers
		std::vector<short> _acd2dataIndex;

		unsigned int _fromStep;
		int _numberOfSteps;
	};

}

#endif
