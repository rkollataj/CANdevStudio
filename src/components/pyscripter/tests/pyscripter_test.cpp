#include <QtWidgets/QApplication>
#include <pyscripter.h>
#define CATCH_CONFIG_RUNNER
#include "log.h"
#include <QCanBusFrame>
#include <QSignalSpy>
#include <catch.hpp>
#include <fakeit.hpp>

std::shared_ptr<spdlog::logger> kDefaultLogger;
// needed for QSignalSpy cause according to qtbug 49623 comments
// automatic detection of types is "flawed" in moc
Q_DECLARE_METATYPE(QCanBusFrame);

TEST_CASE("Stubbed methods", "[pyscripter]")
{
    PyScripter c;

    CHECK(c.mainWidget() == nullptr);
    CHECK(c.mainWidgetDocked() == true);
}

TEST_CASE("setConfig - qobj", "[pyscripter]")
{
    PyScripter c;
    QWidget obj;

    obj.setProperty("name", "Test Name");

    c.setConfig(obj);
}

TEST_CASE("setConfig - json", "[pyscripter]")
{
    PyScripter c;
    QJsonObject obj;

    obj["name"] = "Test Name";

    c.setConfig(obj);
}

TEST_CASE("getConfig", "[pyscripter]")
{
    PyScripter c;

    auto abc = c.getConfig();
}

TEST_CASE("getQConfig", "[pyscripter]")
{
    PyScripter c;

    auto abc = c.getQConfig();
}

TEST_CASE("configChanged", "[pyscripter]")
{
    PyScripter c;

    c.configChanged();
}

TEST_CASE("getSupportedProperties", "[pyscripter]")
{
    PyScripter c;

    auto props = c.getSupportedProperties();

    REQUIRE(props.size() == 2);

    REQUIRE(ComponentInterface::propertyName(props[0]) == "name");
    REQUIRE(ComponentInterface::propertyType(props[0]) == QVariant::String);
    REQUIRE(ComponentInterface::propertyEditability(props[0]) == true);
    REQUIRE(ComponentInterface::propertyField(props[0]) == nullptr);
}

int main(int argc, char* argv[])
{
    bool haveDebug = std::getenv("CDS_DEBUG") != nullptr;
    kDefaultLogger = spdlog::stdout_color_mt("cds");
    if (haveDebug) {
        kDefaultLogger->set_level(spdlog::level::debug);
    }
    cds_debug("Starting unit tests");
    QApplication a(argc, argv); // QApplication must exist when constructing QWidgets TODO check QTest
    return Catch::Session().run(argc, argv);
}
