#define CATCH_CONFIG_RUNNER
#include <QCanBusFrame>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTableView>

#include <QtWidgets/QApplication>
#include <QJsonArray>
#include <context.h>
#include <fakeit.hpp>

#include <log.h>
#include <memory>

#define private public
#include <canrawview.h>
#include <gui/crvguiinterface.h>
#include <uniquefiltermodel.h>
#undef private

#include <iostream>
std::shared_ptr<spdlog::logger> kDefaultLogger;
using namespace fakeit;

#include <QStandardItemModel>

class CanRawViewPrivate;

void addNewFrame(
    uint& rowID, double time, uint frameID, uint data, QStandardItemModel& tvModel, UniqueFilterModel& uniqueModel)
{
    QList<QStandardItem*> list;

    list.append(new QStandardItem(QString::number(rowID++)));
    list.append(new QStandardItem(QString::number(time)));
    list.append(new QStandardItem(std::move(frameID)));
    list.append(new QStandardItem("TX"));
    list.append(new QStandardItem(QString::number(4)));
    list.append(new QStandardItem(QString::number(data)));

    tvModel.appendRow(list);

    //_ui.setSorting(_sortIndex, _ui.getSortOrder());
    uniqueModel.updateFilter(QString::number(frameID), QString::number(time), "TX");
}

TEST_CASE("Initialize table", "[canrawview]")
{
    Mock<CRVGuiInterface> crvMock;
    Fake(Dtor(crvMock));

    Fake(Method(crvMock, setClearCbk));
    Fake(Method(crvMock, setDockUndockCbk));
    Fake(Method(crvMock, setSectionClikedCbk));
    Fake(Method(crvMock, setFilterCbk));
    Fake(Method(crvMock, setModel));
    Fake(Method(crvMock, setSorting));

    Fake(Method(crvMock, initTableView));
    Fake(Method(crvMock, scrollToBottom));
    When(Method(crvMock, mainWidget)).Return(NULL);
    When(Method(crvMock, isViewFrozen)).Return(false).Return(true).Return(true);
    When(Method(crvMock, getSortOrder)).Return(Qt::AscendingOrder);
    When(Method(crvMock, getClickedColumn)).Return("rowID", "time", "id", "dir", "dlc", "data");
    When(Method(crvMock, isColumnHidden)).Return(true, false, false, false, false, false);

    CRVGuiInterface& i = crvMock.get();
    CanRawView canRawView{ CanRawViewCtx(&i) };

    // Verify(Method(crvMock, setSorting).Using(0, Qt::AscendingOrder));
    // Verify(Method(crvMock, getClickedColumn).Using(0));
    // Verify(Method(crvMock, getClickedColumn).Using(0));

    QCanBusFrame frame;
    REQUIRE_NOTHROW(canRawView.startSimulation());
    REQUIRE_NOTHROW(canRawView.frameReceived(frame));
    REQUIRE_NOTHROW(canRawView.frameSent(true, frame));
    REQUIRE_NOTHROW(canRawView.frameSent(false, frame));

    REQUIRE_NOTHROW(canRawView.stopSimulation());
    REQUIRE_NOTHROW(canRawView.frameReceived(frame));
}

TEST_CASE("Unique filter test", "[canrawview]")
{
    QStandardItemModel _tvModel;
    UniqueFilterModel _uniqueModel;
    QTableView _tableView;
    uint rowID = 0;

    _uniqueModel.setSourceModel(&_tvModel);
    _tableView.setModel(&_uniqueModel);

    QCanBusFrame testFrame;
    testFrame.setFrameId(123);

    // rowID, time, frameID, data
    addNewFrame(rowID, 0.20, 1, 0, _tvModel, _uniqueModel);
    addNewFrame(rowID, 0.40, 2, 0, _tvModel, _uniqueModel);
    addNewFrame(rowID, 0.60, 3, 0, _tvModel, _uniqueModel);
    addNewFrame(rowID, 0.80, 2, 20, _tvModel, _uniqueModel); // duplicate frameID 2
    addNewFrame(rowID, 1.00, 1, 10, _tvModel, _uniqueModel); // duplicate frameID 1

    CHECK(_tvModel.rowCount() == 5);
    CHECK(_uniqueModel.isFilterActive() == false);
    _uniqueModel.toggleFilter();
    CHECK(_uniqueModel.isFilterActive() == true);
    CHECK(_uniqueModel._uniques.size() == 3);

    _uniqueModel.clearFilter();
    CHECK(_uniqueModel._uniques.size() == 0);
    CHECK(_tvModel.rowCount() == 5);

    QModelIndex rowIDidx_0 = _tvModel.index(0, 0);
    QModelIndex rowIDidx_1 = _tvModel.index(1, 0);
    uint rowID_0 = _tvModel.data(rowIDidx_0).toUInt();
    uint rowID_1 = _tvModel.data(rowIDidx_1).toUInt();
    CHECK(rowID_0 == rowID_1 - 1);
}

TEST_CASE("Sort test", "[canrawview]")
{
    QStandardItemModel _tvModel;
    UniqueFilterModel _uniqueModel;
    QTableView _tableView;
    uint rowID = 0;

    _uniqueModel.setSourceModel(&_tvModel);
    _tableView.setModel(&_uniqueModel);

    // rowID, time, frameID, data//
    addNewFrame(rowID, 0.20, 10, 1, _tvModel, _uniqueModel);
    addNewFrame(rowID, 1.00, 1, 110, _tvModel, _uniqueModel);
    addNewFrame(rowID, 10.00, 101, 1000, _tvModel, _uniqueModel);
    addNewFrame(rowID, 11.00, 11, 11, _tvModel, _uniqueModel);

    _uniqueModel.toggleFilter();

    for(int i = 0; i < 4; ++i) {
        _uniqueModel.sort(i, Qt::AscendingOrder);
        _uniqueModel.sort(i, Qt::DescendingOrder);
    }

    CHECK(_tvModel.rowCount() == 4);
    CHECK(_uniqueModel.isFilterActive() == false);
    // TODO spy sectionClicked signal...
}

TEST_CASE("setConfig using JSON read with QObject", "[candevice]")
{
    CanRawView crv;
    QObject config;

    config.setProperty("name", "CAN1");
    config.setProperty("fake", "unsupported");

    crv.setConfig(config);

    auto qConfig = crv.getQConfig();

    CHECK(qConfig->property("name").toString() == "CAN1");
    CHECK(qConfig->property("fake").isValid() == false);
}

TEST_CASE("Restore config paths", "[canrawview]")
{
    CanRawView canRawView;
    QJsonObject json;
    QJsonArray columnArray;
    QJsonObject columnItem;

    // No viewColumns
    canRawView.setConfig(json);

    // ViewColumn is not an array
    json["viewColumns"] = "";
    canRawView.setConfig(json);

    // Array item is not an obj
    columnItem["dummy"] = 123;
    columnArray.append(columnItem);
    json["viewColumns"] = columnArray;
    canRawView.setConfig(json);

    // name does not exist
    columnArray.removeFirst();
    columnItem["dummy"] = 123;
    columnItem["dummy2"] = 234;
    columnArray.append(columnItem);
    json["viewColumns"] = columnArray;
    canRawView.setConfig(json);
   
    // name is not a string
    columnArray.removeFirst();
    columnItem["name"] = 123;
    columnItem["dummy2"] = 234;
    columnArray.append(columnItem);
    json["viewColumns"] = columnArray;
    canRawView.setConfig(json);
    
    // vIdx does not exist
    columnArray.removeFirst();
    columnItem["name"] = "rowID";
    columnItem["dummy2"] = 234;
    columnArray.append(columnItem);
    json["viewColumns"] = columnArray;
    canRawView.setConfig(json);
    
    // vIdx is not number
    columnArray.removeFirst();
    columnItem["name"] = "rowID";
    columnItem["vIdx"] = "dsds";
    columnArray.append(columnItem);
    json["viewColumns"] = columnArray;
    canRawView.setConfig(json);
    
    // Column not found
    columnArray.removeFirst();
    columnItem["name"] = "Blah";
    columnItem["vIdx"] = 1;
    columnArray.append(columnItem);
    json["viewColumns"] = columnArray;
    canRawView.setConfig(json);

    // No scrolling item
    columnArray.removeFirst();
    columnItem["name"] = "rowID";
    columnItem["vIdx"] = 1;
    columnArray.append(columnItem);
    columnItem["name"] = "time";
    columnItem["vIdx"] = 2;
    columnArray.append(columnItem);
    columnItem["name"] = "id";
    columnItem["vIdx"] = 3;
    columnArray.append(columnItem);
    columnItem["name"] = "dir";
    columnItem["vIdx"] = 4;
    columnArray.append(columnItem);
    columnItem["name"] = "dlc";
    columnItem["vIdx"] = 5;
    columnArray.append(columnItem);
    columnItem["name"] = "data";
    columnItem["vIdx"] = 6;
    columnArray.append(columnItem);
    json["viewColumns"] = columnArray;
    canRawView.setConfig(json);
}

TEST_CASE("Misc", "[canrawview]")
{
    CanRawView canRawView;

    CHECK(canRawView.mainWidgetDocked() == true);
    CHECK(canRawView.mainWidget() != nullptr);
}

int main(int argc, char* argv[])
{
    bool haveDebug = std::getenv("CDS_DEBUG") != nullptr;
    kDefaultLogger = spdlog::stdout_color_mt("cds");
    if (haveDebug) {
        kDefaultLogger->set_level(spdlog::level::debug);
    }
    cds_debug("Starting canrawview unit tests");
    QApplication a(argc, argv);
    return Catch::Session().run(argc, argv);
}
