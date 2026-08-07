#pragma once

#include <immintrin.h>
#include <filesystem>
#include <cli11/CLI11.hpp>
namespace fs = std::filesystem;

namespace st {

	class StSettingsManager {
	public:

		static void Initialize(int argc, char* argv[])
		{
			getManager() = StSettingsManager(argc,argv);
		}

		static StSettingsManager& getManager() {
			static StSettingsManager settingsManager{};
			return settingsManager;
		}

		uint32_t cubemapResolution;
		size_t kmeansNodeCount;
		size_t kmeansIterations;
		float cellSize;
		float brushProbeGenerationGridSize;
		float probeHeight;
		__m128 maxProbeZ;
		bool headless;
		bool exportProbes;
		fs::path bspPath;

	private:
		StSettingsManager() {
			cubemapResolution = 1024;
			kmeansNodeCount = 8;
			kmeansIterations = 32;
			cellSize = 128.f;
			brushProbeGenerationGridSize = 16.f;
			probeHeight = 64.f;
			headless = true;
			exportProbes = false;
			maxProbeZ = _mm_set1_ps(2000.f);
		}
		StSettingsManager(int argc, char* argv[]):StSettingsManager()
		{

			CLI::App app{"Creator for streaming files for Titanfall 2"};
			argv = app.ensure_utf8(argv);
			app.add_option("-r,--resolution",cubemapResolution,"Resolution of the rendered Cubemap");
			app.add_option("-c,--probeCount", kmeansNodeCount, "Max Number of probes  per Cell");
			app.add_option("-i,--iterations", kmeansIterations, "Number of iterations for reducing probe-options");
			app.add_option("-s,--cellSize", cellSize, "Cell size");
			app.add_option("-b,--brushProbeGenerationGridSize",brushProbeGenerationGridSize,"How far apart probe-options are generated on brushes");
			app.add_option("-H,--probeHeight",probeHeight,"Height of probes above geometry");
			bool windowed = false;
			app.add_flag("-w,--window",windowed,"Show window with generated images");
			app.add_flag("-e,--exportProbes",exportProbes,"Store generated probes to file");
			headless = !windowed;
			app.add_option("bspPath", bspPath, "Path to bsp file")->required()->check(CLI::ExistingFile);
			try
			{
				app.parse(argc,argv);
			}catch (const CLI::ParseError &e)
			{
				std::stringstream info;
				std::stringstream err;
				app.exit(e,info,err);
				if (info.str().size())
					spdlog::info("{}",info.str());
				if (err.str().size())
					spdlog::error("{}",err.str());
				exit(1);
			}
		}
	};

}