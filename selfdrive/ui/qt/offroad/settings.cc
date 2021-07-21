#include "selfdrive/ui/qt/offroad/settings.h"

#include <cassert>
#include <string>

#include <QDebug>

#ifndef QCOM
#include "selfdrive/ui/qt/offroad/networking.h"
#endif

#ifdef ENABLE_MAPS
#include "selfdrive/ui/qt/maps/map_settings.h"
#endif

#include "selfdrive/common/params.h"
#include "selfdrive/common/util.h"
#include "selfdrive/hardware/hw.h"
#include "selfdrive/ui/qt/widgets/controls.h"
#include "selfdrive/ui/qt/widgets/input.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"
#include "selfdrive/ui/qt/widgets/ssh_keys.h"
#include "selfdrive/ui/qt/widgets/toggle.h"
#include "selfdrive/ui/ui.h"
#include "selfdrive/ui/qt/util.h"

TogglesPanel::TogglesPanel(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);

  QList<ParamControl*> toggles;

  toggles.append(new ParamControl("OpenpilotEnabledToggle",
                                  "오픈파일럿 사용",
                                  "오픈파일럿을 사용하여 조향보조 기능을 사용합니다. 항상 핸들을 잡고 도로를 주시하세요.",
                                  "../assets/offroad/icon_openpilot.png",
                                  this));
/*
  toggles.append(new ParamControl("IsRHD",
                                  "Enable Right-Hand Drive",
                                  "Allow openpilot to obey left-hand traffic conventions and perform driver monitoring on right driver seat.",
                                  "../assets/offroad/icon_openpilot_mirrored.png",
                                  this));
*/
  toggles.append(new ParamControl("IsMetric",
                                  "미터법 사용",
                                  "주행속도 표시를 ㎞/h로 변경합니다",
                                  "../assets/offroad/icon_metric.png",
                                  this));
  toggles.append(new ParamControl("CommunityFeaturesToggle",
                                  "커뮤니티 기능 사용",
                                  "커뮤니티기능은 comma.ai에서 공식적으로 지원하는 기능이 아니므로 사용시 주의하세요.",
                                  "../assets/offroad/icon_discord.png",
                                  this));
  toggles.append(new ParamControl("IsLdwEnabled",
                                  "차선이탈 경보 사용",
                                  "40㎞/h 이상의 속도로 주행시 방향지시등 조작없이 차선을 이탈하면 차선이탈경보를 보냅니다. (오픈파일럿 비활성상태에서만 사용됩니다)",
                                  "../assets/offroad/icon_ldws.png",
                                  this));
  toggles.append(new ParamControl("AutoLaneChangeEnabled",
                                  "자동 차선변경 사용",
                                  "60㎞/h 이상의 속도로 주행시 방향지시등을 작동하면 3초후에 차선변경을 수행합니다. 안전한 사용을위해 후측방감지기능이 있는 차량만 사용하시기바랍니다.",
                                  "../assets/offroad/icon_lca.png",
                                  this));
  toggles.append(new ParamControl("UploadRaw",
                                  "Upload Raw Logs",
                                  "Upload full logs at my.comma.ai/useradmin.",
                                  "../assets/offroad/icon_network.png",
                                  this));
/*
  ParamControl *record_toggle = new ParamControl("RecordFront",
                                                 "Record and Upload Driver Camera",
                                                "Upload data from the driver facing camera and help improve the driver monitoring algorithm.",
                                                "../assets/offroad/icon_monitoring.png",
                                                this);
  toggles.append(record_toggle);
  toggles.append(new ParamControl("EndToEndToggle",
                                   "\U0001f96c Disable use of lanelines (Alpha) \U0001f96c",
                                   "In this mode openpilot will ignore lanelines and just drive how it thinks a human would.",
                                   "../assets/offroad/icon_road.png",
                                   this));

  if (Hardware::TICI()) {
    toggles.append(new ParamControl("EnableWideCamera",
                                    "Enable use of Wide Angle Camera",
                                    "Use wide angle camera for driving and ui.",
                                    "../assets/offroad/icon_openpilot.png",
                                    this));
    QObject::connect(toggles.back(), &ToggleControl::toggleFlipped, [=](bool state) {
      Params().remove("CalibrationParams");
    });
  }

#ifdef ENABLE_MAPS
  toggles.append(new ParamControl("NavSettingTime24h",
                                  "Show ETA in 24h format",
                                  "Use 24h format instead of am/pm",
                                  "../assets/offroad/icon_metric.png",
                                  this));
#endif

  bool record_lock = Params().getBool("RecordFrontLock");
  record_toggle->setEnabled(!record_lock);
*/
  for(ParamControl *toggle : toggles) {
    if(main_layout->count() != 0) {
      main_layout->addWidget(horizontal_line());
    }
    main_layout->addWidget(toggle);
  }
}

DevicePanel::DevicePanel(QWidget* parent) : QWidget(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  Params params = Params();

  QString dongle = QString::fromStdString(params.get("DongleId", false));
  main_layout->addWidget(new LabelControl("동글 ID", dongle));
/*
  main_layout->addWidget(horizontal_line());

  QString serial = QString::fromStdString(params.get("HardwareSerial", false));
  main_layout->addWidget(new LabelControl("Serial", serial));
*/
  // offroad-only buttons

  auto dcamBtn = new ButtonControl("운전자 모니터링 카메라 미리보기", "실행", "운전자 모니터링 카메라를 미리보고 최적의 장착위치를 찾아보세요.");
  connect(dcamBtn, &ButtonControl::released, [=]() { emit showDriverView(); });

  QString resetCalibDesc = "범위 (pitch) ↕ 5˚ (yaw) ↔ 4˚이내";
  auto resetCalibBtn = new ButtonControl("캘리브레이션 초기화", "실행", resetCalibDesc);
  connect(resetCalibBtn, &ButtonControl::released, [=]() {
    if (ConfirmationDialog::confirm("실행하시겠습니까?", this)) {
      Params().remove("CalibrationParams");
    }
  });
  connect(resetCalibBtn, &ButtonControl::showDescription, [=]() {
    QString desc = resetCalibDesc;
    std::string calib_bytes = Params().get("CalibrationParams");
    if (!calib_bytes.empty()) {
      try {
        AlignedBuffer aligned_buf;
        capnp::FlatArrayMessageReader cmsg(aligned_buf.align(calib_bytes.data(), calib_bytes.size()));
        auto calib = cmsg.getRoot<cereal::Event>().getLiveCalibration();
        if (calib.getCalStatus() != 0) {
          double pitch = calib.getRpyCalib()[1] * (180 / M_PI);
          double yaw = calib.getRpyCalib()[2] * (180 / M_PI);
          desc += QString("\n현재 캘리브레이션된 위치는 [ %2 %1° / %4 %3° ] 입니다.")
                                .arg(QString::number(std::abs(pitch), 'g', 1), pitch > 0 ? "↑" : "↓",
                                     QString::number(std::abs(yaw), 'g', 1), yaw > 0 ? "→" : "←");
        }
      } catch (kj::Exception) {
        qInfo() << "캘리브레이션 상태가 유효하지않습니다";
      }
    }
    resetCalibBtn->setDescription(desc);
  });

  ButtonControl *retrainingBtn = nullptr;
  if (!params.getBool("Passive")) {
    retrainingBtn = new ButtonControl("트레이닝 가이드", "실행", "");
    connect(retrainingBtn, &ButtonControl::released, [=]() {
      if (ConfirmationDialog::confirm("실행하시겠습니까?", this)) {
        Params().remove("CompletedTrainingVersion");
        emit reviewTrainingGuide();
      }
    });
  }

  auto uninstallBtn = new ButtonControl(getBrand() + " 제거", "실행");
  connect(uninstallBtn, &ButtonControl::released, [=]() {
    if (ConfirmationDialog::confirm("실행하시겠습니까?", this)) {
      Params().putBool("DoUninstall", true);
    }
  });

  for (auto btn : {dcamBtn, resetCalibBtn, retrainingBtn, uninstallBtn}) {
    if (btn) {
      main_layout->addWidget(horizontal_line());
      connect(parent, SIGNAL(offroadTransition(bool)), btn, SLOT(setEnabled(false)));
      main_layout->addWidget(btn);
    }
  }

  // power buttons
  QHBoxLayout *power_layout = new QHBoxLayout();
  power_layout->setSpacing(30);

  QPushButton *reboot_btn = new QPushButton("재부팅");
  reboot_btn->setStyleSheet("height: 120px;border-radius: 15px; background-color: #2CE22C;");
  power_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("재부팅하시겠습니까?", this)) {
      Hardware::reboot();
    }
  });

  QPushButton *poweroff_btn = new QPushButton("종료");
  poweroff_btn->setStyleSheet("height: 120px;border-radius: 15px; background-color: #E22C2C;");
  power_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("종료하시겠습니까?", this)) {
      Hardware::poweroff();
    }
  });

  main_layout->addLayout(power_layout);
}

SoftwarePanel::SoftwarePanel(QWidget* parent) : QWidget(parent) {
  gitRemoteLbl = new LabelControl("Git Remote");
  gitBranchLbl = new LabelControl("Git Branch");
  gitCommitLbl = new LabelControl("Git Commit");
  osVersionLbl = new LabelControl("NEOS Version");
  versionLbl = new LabelControl("Version");
  lastUpdateLbl = new LabelControl("Last Update Check", "", "The last time openpilot successfully checked for an update. The updater only runs while the car is off.");
  updateBtn = new ButtonControl("Check for Update", "");
  connect(updateBtn, &ButtonControl::released, [=]() {
    if (params.getBool("IsOffroad")) {
      const QString paramsPath = QString::fromStdString(params.getParamsPath());
      fs_watch->addPath(paramsPath + "/d/LastUpdateTime");
      fs_watch->addPath(paramsPath + "/d/UpdateFailedCount");
      updateBtn->setText("CHECKING");
      updateBtn->setEnabled(false);
    }
    std::system("pkill -1 -f selfdrive.updated");
  });

  QVBoxLayout *main_layout = new QVBoxLayout(this);
  QWidget *widgets[] = {versionLbl, osVersionLbl, gitRemoteLbl, gitBranchLbl, gitCommitLbl};
  for (int i = 0; i < std::size(widgets); ++i) {
    main_layout->addWidget(widgets[i]);
  }

  // add
  const char* gitpull = "/data/openpilot/gitpull.sh ''";
  auto gitpullbtn = new ButtonControl("Git Pull", "실행");
  QObject::connect(gitpullbtn, &ButtonControl::released, [=]() {
    if (ConfirmationDialog::confirm("실행하시겠습니까?", this)){
      std::system(gitpull);
    }
  });
  main_layout->addWidget(gitpullbtn);

  const char* panda_flash = "/data/openpilot/panda/board/flash.sh ''";
  auto pandaflashbtn = new ButtonControl("판다 펌웨어 플래싱", "실행");
  QObject::connect(pandaflashbtn, &ButtonControl::released, [=]() {
    if (ConfirmationDialog::confirm("실행하시겠습니까?", this)){
      std::system(panda_flash);
    }
  });
  main_layout->addWidget(pandaflashbtn);

  const char* panda_recover = "/data/openpilot/panda/board/recover.sh ''";
  auto pandarecoverbtn = new ButtonControl("판다 펌웨어 복구", "실행");
  QObject::connect(pandarecoverbtn, &ButtonControl::released, [=]() {
    if (ConfirmationDialog::confirm("실행하시겠습니까?", this)){
      std::system(panda_recover);
    }
  });
  main_layout->addWidget(pandarecoverbtn);

  const char* addfunc = "cp -f /data/openpilot/installer/fonts/driver_monitor.py /data/openpilot/selfdrive/monitoring";
  auto addfuncbtn = new ButtonControl("추가 기능", "실행");
  QObject::connect(addfuncbtn, &ButtonControl::released, [=]() {
    if (ConfirmationDialog::confirm("실행하시겠습니까?", this)){
      std::system(addfunc);
      Hardware::reboot();
    }
  });
  main_layout->addWidget(addfuncbtn);

  const char* realdata_clear = "rm -rf /sdcard/realdata/*";
  auto realdataclearbtn = new ButtonControl("주행로그 삭제", "실행");
  QObject::connect(realdataclearbtn, &ButtonControl::released, [=]() {
    if (ConfirmationDialog::confirm("실행하시겠습니까?", this)){
      std::system(realdata_clear);
    }
  });
  main_layout->addWidget(realdataclearbtn);

  setStyleSheet(R"(QLabel {font-size: 50px;})");

  fs_watch = new QFileSystemWatcher(this);
  QObject::connect(fs_watch, &QFileSystemWatcher::fileChanged, [=](const QString path) {
    int update_failed_count = params.get<int>("UpdateFailedCount").value_or(0);
    if (path.contains("UpdateFailedCount") && update_failed_count > 0) {
      lastUpdateLbl->setText("failed to fetch update");
      updateBtn->setText("CHECK");
      updateBtn->setEnabled(true);
    } else if (path.contains("LastUpdateTime")) {
      updateLabels();
    }
  });
}

void SoftwarePanel::showEvent(QShowEvent *event) {
  updateLabels();
}

void SoftwarePanel::updateLabels() {
  QString lastUpdate = "";
  auto tm = params.get("LastUpdateTime");
  if (!tm.empty()) {
    lastUpdate = timeAgo(QDateTime::fromString(QString::fromStdString(tm + "Z"), Qt::ISODate));
  }

  versionLbl->setText(getBrandVersion());
  lastUpdateLbl->setText(lastUpdate);
  updateBtn->setText("확인");
  updateBtn->setEnabled(true);
  gitRemoteLbl->setText(QString::fromStdString(params.get("GitRemote").substr(19)));
  gitBranchLbl->setText(QString::fromStdString(params.get("GitBranch")));
  gitCommitLbl->setText(QString::fromStdString(params.get("GitCommit")).left(7));
  osVersionLbl->setText(QString::fromStdString(Hardware::get_os_version()).trimmed());
}

QWidget * network_panel(QWidget * parent) {
  QWidget *w = new QWidget(parent);
  QVBoxLayout *layout = new QVBoxLayout(w);
  layout->setSpacing(30);
#ifdef QCOM
  // wifi + tethering buttons
  auto wifiBtn = new ButtonControl("\U0001f4f6 WiFi 설정", "열기");
  QObject::connect(wifiBtn, &ButtonControl::released, [=]() { HardwareEon::launch_wifi(); });
  layout->addWidget(wifiBtn);

  auto androidBtn = new ButtonControl("\U00002699 안드로이드 설정", "열기");
  QObject::connect(androidBtn, &ButtonControl::released, [=]() { HardwareEon::launch_setting(); });
  layout->addWidget(androidBtn);
  layout->addWidget(horizontal_line());
/*
  auto tetheringBtn = new ButtonControl("Tethering Settings", "OPEN");
  QObject::connect(tetheringBtn, &ButtonControl::released, [=]() { HardwareEon::launch_tethering(); });
  layout->addWidget(tetheringBtn);
  layout->addWidget(horizontal_line());
*/
#endif

  // SSH key management
  layout->addWidget(new SshToggle());
  layout->addWidget(new SshControl());
  layout->addWidget(horizontal_line());
  layout->addWidget(new LateralControlSelect());
  layout->addWidget(new MfcSelect());
  layout->addWidget(new LongControlSelect());
  layout->addWidget(horizontal_line());
  layout->addWidget(new PrebuiltToggle());
  layout->addWidget(new ShutdowndToggle());
  layout->addWidget(new LoggerToggle());
  layout->addWidget(new GpsToggle());
  layout->addWidget(new UiTpmsToggle());
  layout->addStretch(1);
  return w;
}

void SettingsWindow::showEvent(QShowEvent *event) {
  panel_widget->setCurrentIndex(0);
  nav_btns->buttons()[0]->setChecked(true);
}

SettingsWindow::SettingsWindow(QWidget *parent) : QFrame(parent) {

  // setup two main layouts
  sidebar_widget = new QWidget;
  QVBoxLayout *sidebar_layout = new QVBoxLayout(sidebar_widget);
  sidebar_layout->setMargin(0);
  panel_widget = new QStackedWidget();
  panel_widget->setStyleSheet(R"(
    border-radius: 30px;
    background-color: #292929;
  )");

  // close button
  QPushButton *close_btn = new QPushButton("◀");
  close_btn->setStyleSheet(R"(
    font-size: 90px;
    font-weight: bold;
    border 1px grey solid;
    border-radius: 100px;
    background-color: #292929;
  )");
  close_btn->setFixedSize(200, 200);
  sidebar_layout->addSpacing(45);
  sidebar_layout->addWidget(close_btn, 0, Qt::AlignCenter);
  QObject::connect(close_btn, &QPushButton::released, this, &SettingsWindow::closeSettings);

  // setup panels
  DevicePanel *device = new DevicePanel(this);
  QObject::connect(device, &DevicePanel::reviewTrainingGuide, this, &SettingsWindow::reviewTrainingGuide);
  QObject::connect(device, &DevicePanel::showDriverView, this, &SettingsWindow::showDriverView);

  QList<QPair<QString, QWidget *>> panels = {
    {"장치", device},
    {"설정", network_panel(this)},
    {"토글", new TogglesPanel(this)},
    {"정보", new SoftwarePanel(this)},
  };

#ifdef ENABLE_MAPS
  if (!Params().get("MapboxToken").empty()) {
    auto map_panel = new MapPanel(this);
    panels.push_back({"Navigation", map_panel});
    QObject::connect(map_panel, &MapPanel::closeSettings, this, &SettingsWindow::closeSettings);
  }
#endif
  const int padding = panels.size() > 3 ? 25 : 35;

  nav_btns = new QButtonGroup();
  for (auto &[name, panel] : panels) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setChecked(nav_btns->buttons().size() == 0);
    btn->setStyleSheet(QString(R"(
      QPushButton {
        color: grey;
        border: none;
        background: none;
        font-size: 60px;
        font-weight: 500;
        padding-top: %1px;
        padding-bottom: %1px;
      }
      QPushButton:checked {
        color: white;
      }
    )").arg(padding));

    nav_btns->addButton(btn);
    sidebar_layout->addWidget(btn, 0, Qt::AlignRight);

    panel->setContentsMargins(50, 25, 50, 25);

    ScrollView *panel_frame = new ScrollView(panel, this);
    panel_widget->addWidget(panel_frame);

    QObject::connect(btn, &QPushButton::released, [=, w = panel_frame]() {
      btn->setChecked(true);
      panel_widget->setCurrentWidget(w);
    });
  }
  sidebar_layout->setContentsMargins(50, 50, 100, 50);

  // main settings layout, sidebar + main panel
  QHBoxLayout *main_layout = new QHBoxLayout(this);

  sidebar_widget->setFixedWidth(500);
  main_layout->addWidget(sidebar_widget);
  main_layout->addWidget(panel_widget);

  setStyleSheet(R"(
    * {
      color: white;
      font-size: 50px;
    }
    SettingsWindow {
      background-color: black;
    }
  )");
}

void SettingsWindow::hideEvent(QHideEvent *event) {
#ifdef QCOM
  HardwareEon::close_activities();
#endif
}
