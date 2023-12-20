#include "../Game/world.h"
#include "../Logger/logger.h"
#include "../Utils/CommandLineParser.h"
#include "../Assets/package.h"

static void Run()
{
	World world;
	world.Run();
}

int main(int argc, char** argv)
{
	std::string packageKey("package");
	std::string verboseArg("verbose");
	std::string coloredLogArg("colored_log");

	Logger::SetLevel(Logger::Level::Warning);

	auto args = CommandLineParser::Parse(
		argc,
		argv,
		{{packageKey, "resources.bin"}});

	if (args.Args.find(verboseArg) != args.Args.end()) {
		Logger::SetLevel(Logger::Level::Verbose);
	}

	if (args.Args.find(coloredLogArg) != args.Args.end()) {
		Logger::SetColored(true);
	}

	Package::LoadPackage(args.Keys[packageKey]);

	Run();

	Package::UnloadPackage();

	return 0;
}
