/**
 * @copyright
 * Copyright (c) 2012-2020, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/LICENSE.txt
 */

#pragma once

#include <string>

#include "GeoLib/GEOObjects.h"

void createSetOfTestPointsAndAssociatedNames(GeoLib::GEOObjects& geo_objs,
                                             std::string& name,
                                             std::size_t const pnts_per_edge,
                                             GeoLib::Point const& shift);
