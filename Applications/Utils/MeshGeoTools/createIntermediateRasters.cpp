/**
 * \file
 *
 * \copyright
 * Copyright (c) 2012-2019, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/LICENSE.txt
 */

#include <memory>
#include <string>

#include <tclap/CmdLine.h>

#include "BaseLib/BuildInfo.h"
#include "BaseLib/FileTools.h"
#include "Applications/ApplicationsLib/LogogSetup.h"
#include "Applications/FileIO/AsciiRasterInterface.h"
#include "GeoLib/Raster.h"

int main(int argc, char* argv[])
{
    ApplicationsLib::LogogSetup logog_setup;

    TCLAP::CmdLine cmd(
        "Takes two DEMs located at the exact same spatial position (but at "
        "different elevation) and calculates n raster DEMs located at "
        "equidistant intervals between them (i.e. for n=1, one new raster "
        "located precisely in the middle will be created).\n\n"
        "OpenGeoSys-6 software, version " +
            BaseLib::BuildInfo::ogs_version +
            ".\n"
            "Copyright (c) 2012-2019, OpenGeoSys Community "
            "(http://www.opengeosys.org)",
        ' ', BaseLib::BuildInfo::ogs_version);
    TCLAP::ValueArg<std::string> input1_arg(
        "", "file1", "First DEM-raster file", true, "", "file1.asc");
    cmd.add(input1_arg);
    TCLAP::ValueArg<std::string> input2_arg(
        "", "file2", "Second DEM-raster file", true, "", "file2.asc");
    cmd.add(input2_arg);
    TCLAP::ValueArg<std::string> output_arg("o", "output-file",
                                            "Raster output file (*.asc)", true,
                                            "", "output.asc");
    cmd.add(output_arg);
    TCLAP::ValueArg<std::size_t> number_arg(
        "n", "number", "number of rasters to be calculated", false, 1, "int");
    cmd.add(number_arg);

    cmd.parse(argc, argv);

    std::unique_ptr<GeoLib::Raster> dem1(
        FileIO::AsciiRasterInterface::readRaster(input1_arg.getValue()));
    std::unique_ptr<GeoLib::Raster> dem2(
        FileIO::AsciiRasterInterface::readRaster(input2_arg.getValue()));

    GeoLib::RasterHeader const h1 = dem1->getHeader();
    GeoLib::RasterHeader const h2 = dem2->getHeader();

    bool errors_found(false);
    if (h1.origin[0] != h2.origin[0])
    {
        ERR("Origin x-coordinate is not the same in both raster files.\n");
        errors_found = true;
    }
    if (h1.origin[1] != h2.origin[1])
    {
        ERR("Origin y-coordinate is not the same in both raster files.\n");
        errors_found = true;
    }
    if (h1.cell_size != h2.cell_size)
    {
        ERR("Cellsize is not the same in both raster files.\n");
        errors_found = true;
    }
    if (h1.n_cols != h2.n_cols)
    {
        ERR("Raster width is not the same in both raster files.\n");
        errors_found = true;
    }
    if (h1.n_rows != h2.n_rows)
    {
        ERR("Raster height is not the same in both raster files.\n")
        errors_found = true;
    }

    if (errors_found)
        return 2;

    std::size_t const n = number_arg.getValue();
    std::vector<std::vector<double>> raster;
    for (std::size_t i = 0; i < n; ++i)
    {
        std::vector<double> r;
        r.reserve(h1.n_cols * h1.n_rows);
        raster.push_back(r);
    }

    auto it2 = dem2->begin();
    for (auto it1 = dem1->begin(); it1 != dem1->end(); ++it1)
    {
        if (it2 == dem2->end())
        {
            ERR("Error: File 2 is shorter than File 1.");
            return 1;
        }
        if (*it1 == h1.no_data || *it2 == h2.no_data)
        {
            for (std::size_t i = 0; i < n; ++i)
                raster[i].push_back(h1.no_data);
        }
        else
        {
            double const min = std::min(*it1, *it2);
            double const max = std::max(*it1, *it2);
            double const step = (max - min) / static_cast<double>(n + 1);
            for (std::size_t i = 0; i < n; ++i)
                raster[i].push_back(min + ((i+1) * step));
        }
        it2++;
    }
    if (it2 != dem2->end())
    {
        ERR("Error: File 1 is shorter than File 2.");
        return 1;
    }

    std::string const filename = output_arg.getValue();
    for (std::size_t i = 0; i < n; ++i)
    {
        std::string const basename = BaseLib::dropFileExtension(filename);
        std::string const ext = BaseLib::getFileExtension(filename);

        GeoLib::RasterHeader h (h1);
        GeoLib::Raster r(std::move(h), raster[i].begin(), raster[i].end());
        FileIO::AsciiRasterInterface::writeRasterAsASC(r, basename + std::to_string(i) + "." + ext);
        INFO("Layer %d written.", i+1);
    }
    return 0;
}
