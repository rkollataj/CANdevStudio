#define CATCH_CONFIG_RUNNER
#include <projectconfig.h>
#include <fakeit.hpp>
#include <log.h>
#include <QWindow>
#include <QApplication>
#include <QDir>
#include <QSignalSpy>
#include <QCloseEvent>

std::shared_ptr<spdlog::logger> kDefaultLogger;

TEST_CASE("Loading and saving", "[projectconfig]")
{
    QDir dir("configfiles");
    QFile file(dir.absoluteFilePath("projectconfig.cds"));
    ProjectConfig pc(new QWidget);
    QByteArray outConfig;

    CHECK(file.open(QIODevice::ReadOnly) == true);
    auto inConfig = file.readAll();

    REQUIRE_NOTHROW(pc.load(inConfig));
    REQUIRE_NOTHROW(outConfig = pc.save());
    REQUIRE_NOTHROW(pc.clearGraphView());
    REQUIRE_NOTHROW(pc.load(outConfig));
}

TEST_CASE("Color mode", "[projectconfig]")
{
    ProjectConfig pc(new QWidget);

    REQUIRE_NOTHROW(pc.setColorMode(true));
    REQUIRE_NOTHROW(pc.setColorMode(false));
}

TEST_CASE("Close event", "[projectconfig]")
{
    QCloseEvent e;
    ProjectConfig pc(new QWidget);
    QSignalSpy closeSpy(&pc, &ProjectConfig::closeProject);

    pc.closeEvent(&e);

    CHECK(closeSpy.count() == 1);
}

int main(int argc, char* argv[])
{
    bool haveDebug = std::getenv("CDS_DEBUG") != nullptr;
    kDefaultLogger = spdlog::stdout_color_mt("cds");
    if (haveDebug) {
        kDefaultLogger->set_level(spdlog::level::debug);
    }
    cds_debug("Staring unit tests");
    QApplication a(argc, argv);
    return Catch::Session().run(argc, argv);
}

