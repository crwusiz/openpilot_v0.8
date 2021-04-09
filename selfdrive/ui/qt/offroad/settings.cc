#include <string>
#include <iostream>
#include <sstream>
#include <cassert>

#ifndef QCOM
#include "networking.hpp"
#endif
#include "settings.hpp"
#include "widgets/input.hpp"
#include "widgets/toggle.hpp"
#include "widgets/offroad_alerts.hpp"
#include "widgets/controls.hpp"
#include "widgets/ssh_keys.hpp"
#include "common/params.h"
#include "common/util.h"
#include "selfdrive/hardware/hw.h"


QWidget * toggles_panel() {
  QVBoxLayout *toggles_list = new QVBoxLayout();

  toggles_list->addWidget(new ParamControl("OpenpilotEnabledToggle",
                                            "오픈파일럿 사용",
                                            "Use the openpilot system for adaptive cruise control and lane keep driver assistance. Your attention is required at all times to use this feature. Changing this setting takes effect when the car is powered off.",
                                            "../assets/offroad/icon_openpilot.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("IsLdwEnabled",
                                            "차선이탈 경보 사용",
                                            "Receive alerts to steer back into the lane when your vehicle drifts over a detected lane line without a turn signal activated while driving over 31mph (50kph).",
                                            "../assets/offroad/icon_warning.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("IsMetric",
                                            "미터법 사용",
                                            "Display speed in km/h instead of mp/h.",
                                            "../assets/offroad/icon_metric.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("CommunityFeaturesToggle",
                                            "커뮤니티 기능 사용",
                                            "Use features from the open source community that are not maintained or supported by comma.ai and have not been confirmed to meet the standard safety model. These features include community supported cars and community supported hardware. Be extra cautious when using these features",
                                            "../assets/offroad/icon_shell.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("EndToEndToggle",
                                           "\U0001f96c Disable use of lanelines (Alpha) \U0001f96c",
                                           "In this mode openpilot will ignore lanelines and just drive how it thinks a human would.",
                                           "../assets/offroad/icon_road.png"));

//  bool record_lock = Params().read_db_bool("RecordFrontLock");
//  record_toggle->setEnabled(!record_lock);

  QWidget *widget = new QWidget;
  widget->setLayout(toggles_list);
  return widget;
}

DevicePanel::DevicePanel(QWidget* parent) : QWidget(parent) {
  QVBoxLayout *device_layout = new QVBoxLayout;

  Params params = Params();

  QString dongle = QString::fromStdString(params.get("DongleId", false));
  device_layout->addWidget(new LabelControl("Dongle ID", dongle));
  device_layout->addWidget(horizontal_line());

  QString serial = QString::fromStdString(params.get("HardwareSerial", false));
  device_layout->addWidget(new LabelControl("Serial", serial));

  // offroad-only buttons
  QList<ButtonControl*> offroad_btns;

  offroad_btns.append(new ButtonControl("운전자 모니터링", "미리보기",
                                   "Preview the driver facing camera to help optimize device mounting position for best driver monitoring experience. (vehicle must be off)",
                                   [=]() { Params().write_db_value("IsDriverViewEnabled", "1", 1); }));

  offroad_btns.append(new ButtonControl("캘리브레이션", "초기화",
                                   "openpilot requires the device to be mounted within 4° left or right and within 5° up or down. openpilot is continuously calibrating, resetting is rarely required.", [=]() {
    if (ConfirmationDialog::confirm("Are you sure you want to reset calibration?")) {
      Params().delete_db_value("CalibrationParams");
    }
  }));

  offroad_btns.append(new ButtonControl("트레이닝 가이드", "다시보기",
                                        "Review the rules, features, and limitations of openpilot", [=]() {
    if (ConfirmationDialog::confirm("Are you sure you want to review the training guide?")) {
      Params().delete_db_value("CompletedTrainingVersion");
      emit reviewTrainingGuide();
    }
  }));

  QString brand = params.read_db_bool("Passive") ? "대시캠" : "오픈파일럿";
  offroad_btns.append(new ButtonControl(brand + " 제거", "제거", "", [=]() {
    if (ConfirmationDialog::confirm("제거하시겠습니까?")) {
      Params().write_db_value("DoUninstall", "1");
    }
  }));

  for(auto &btn : offroad_btns){
    device_layout->addWidget(horizontal_line());
    QObject::connect(parent, SIGNAL(offroadTransition(bool)), btn, SLOT(setEnabled(bool)));
    device_layout->addWidget(btn);
  }

  // power buttons
  QHBoxLayout *power_layout = new QHBoxLayout();
  power_layout->setSpacing(30);

  QPushButton *reboot_btn = new QPushButton("재부팅");
  power_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("재부팅 하시겠습니까?")) {
      Hardware::reboot();
    }
  });

  QPushButton *poweroff_btn = new QPushButton("전원종료");
  poweroff_btn->setStyleSheet("background-color: #E22C2C;");
  power_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("전원을 종룟하시겠습니까?")) {
      Hardware::poweroff();
    }
  });

  device_layout->addLayout(power_layout);

  setLayout(device_layout);
  setStyleSheet(R"(
    QPushButton {
      padding: 0;
      height: 120px;
      border-radius: 15px;
      background-color: #393939;
    }
  )");
}

DeveloperPanel::DeveloperPanel(QWidget* parent) : QFrame(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  setLayout(main_layout);
  setStyleSheet(R"(QLabel {font-size: 50px;})");
}

void DeveloperPanel::showEvent(QShowEvent *event) {
  Params params = Params();
  std::string brand = params.read_db_bool("Passive") ? "대시캠" : "오픈파일럿";
  QList<QPair<QString, std::string>> dev_params = {
    {"Version", brand + " v" + params.get("Version", false).substr(0, 14)},
    {"Git Branch", params.get("GitBranch", false)},
    {"Git Commit", params.get("GitCommit", false).substr(0, 10)},
    {"Panda Firmware", params.get("PandaFirmwareHex", false)},
    {"OS Version", Hardware::get_os_version()},
  };

  for (int i = 0; i < dev_params.size(); i++) {
    const auto &[name, value] = dev_params[i];
    QString val = QString::fromStdString(value).trimmed();
    if (labels.size() > i) {
      labels[i]->setText(val);
    } else {
      labels.push_back(new LabelControl(name, val));
      layout()->addWidget(labels[i]);
      if (i < (dev_params.size() - 1)) {
        layout()->addWidget(horizontal_line());
      }
    }
  }
}

QWidget * network_panel(QWidget * parent) {
#ifdef QCOM
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setSpacing(30);

  // wifi + tethering buttons
  layout->addWidget(new ButtonControl("WiFi 설정", "열기", "",
                                      [=]() { HardwareEon::launch_wifi(); }));
  layout->addWidget(horizontal_line());

/*
  layout->addWidget(new ButtonControl("테더링 설정", "열기", "",
                                      [=]() { HardwareEon::launch_tethering(); }));
  layout->addWidget(horizontal_line());
*/
  // SSH key management
  layout->addWidget(new SshToggle());
  layout->addWidget(horizontal_line());
  layout->addWidget(new SshControl());
  layout->addWidget(horizontal_line());
  
  const char* gitpull = "/data/openpilot/gitpull.sh ''";
  layout->addWidget(new ButtonControl("Git Pull", "실행", "리모트 Git에서 변경사항이 있으면 로컬에 반영 후 자동 재부팅 됩니다. 변경사항이 없으면 재부팅하지 않습니다. 로컬 파일이 변경된경우 리모트Git 내역을 반영 못할수도 있습니다. 참고바랍니다.",
                                      [=]() { 
                                        if (ConfirmationDialog::confirm("진행하시겠습니까?")){
                                          std::system(gitpull);
                                        }
                                      }));
  layout->addWidget(horizontal_line());                                      
  const char* addfunc = "/data/openpilot/addfunc.sh ''";
  layout->addWidget(new ButtonControl("추가 기능", "실행", "각종 추가기능을 설정합니다",
                                      [=]() {
                                        if (ConfirmationDialog::confirm("진행하시겠습니까?")) {
                                          std::system(addfunc);
                                        }
                                      }));    
  layout->addWidget(horizontal_line());                                      
  const char* panda_flashing = "/data/openpilot/panda_flashing.sh ''";
  layout->addWidget(new ButtonControl("판다 플래싱", "실행", "판다플래싱이 진행되면 판다의 녹색LED가 빠르게 깜빡이며 완료되면 자동 재부팅 됩니다. 절대로 장치의 전원을 끄거나 임의로 분리하지 마시기 바랍니다.",
                                      [=]() {
                                        if (ConfirmationDialog::confirm("진행하시겠습니까?")) {
                                          std::system(panda_flashing);
                                        }
                                      }));
  layout->addWidget(horizontal_line());                                      
  layout->addWidget(new LateralControl());                                                                            
  layout->addStretch(1);

  QWidget *w = new QWidget;
  w->setLayout(layout);
#else
  Networking *w = new Networking(parent);
#endif
  return w;
}

SettingsWindow::SettingsWindow(QWidget *parent) : QFrame(parent) {
  // setup two main layouts
  QVBoxLayout *sidebar_layout = new QVBoxLayout();
  sidebar_layout->setMargin(0);
  panel_widget = new QStackedWidget();
  panel_widget->setStyleSheet(R"(
    border-radius: 30px;
    background-color: #292929;
  )");

  // close button
  QPushButton *close_btn = new QPushButton("X");
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
  QObject::connect(close_btn, SIGNAL(released()), this, SIGNAL(closeSettings()));

  // setup panels
  DevicePanel *device = new DevicePanel(this);
  QObject::connect(device, SIGNAL(reviewTrainingGuide()), this, SIGNAL(reviewTrainingGuide()));

  QPair<QString, QWidget *> panels[] = {
    {"장치", new DevicePanel(this)},
    {"네트워크", network_panel(this)},
    {"토글", toggles_panel()},
    {"개발자", new DeveloperPanel()},
  };

  sidebar_layout->addSpacing(45);
  nav_btns = new QButtonGroup();
  for (auto &[name, panel] : panels) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(R"(
      QPushButton {
        color: grey;
        border: none;
        background: none;
        font-size: 65px;
        font-weight: 500;
        padding-top: 35px;
        padding-bottom: 35px;
      }
      QPushButton:checked {
        color: white;
      }
    )");

    nav_btns->addButton(btn);
    sidebar_layout->addWidget(btn, 0, Qt::AlignRight);

    panel->setContentsMargins(50, 25, 50, 25);
    QScrollArea *panel_frame = new QScrollArea;
    panel_frame->setWidget(panel);
    panel_frame->setWidgetResizable(true);
    panel_frame->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    panel_frame->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    panel_frame->setStyleSheet("background-color:transparent;");

    QScroller *scroller = QScroller::scroller(panel_frame->viewport());
    auto sp = scroller->scrollerProperties();

    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff));

    scroller->grabGesture(panel_frame->viewport(), QScroller::LeftMouseButtonGesture);
    scroller->setScrollerProperties(sp);

    panel_widget->addWidget(panel_frame);

    QObject::connect(btn, &QPushButton::released, [=, w = panel_frame]() {
      panel_widget->setCurrentWidget(w);
    });
  }
  qobject_cast<QPushButton *>(nav_btns->buttons()[0])->setChecked(true);
  sidebar_layout->setContentsMargins(50, 50, 100, 50);

  // main settings layout, sidebar + main panel
  QHBoxLayout *settings_layout = new QHBoxLayout();

  sidebar_widget = new QWidget;
  sidebar_widget->setLayout(sidebar_layout);
  sidebar_widget->setFixedWidth(500);
  settings_layout->addWidget(sidebar_widget);
  settings_layout->addWidget(panel_widget);

  setLayout(settings_layout);
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
