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
                                            "오픈파일럿을 사용하여 조향보조 기능을 사용합니다. 항상 도로상황을 주시하세요.",
                                            "../assets/offroad/icon_openpilot.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("IsMetric",
                                            "미터법 사용",
                                            "주행속도 표시를 km/h로 변경합니다",
                                            "../assets/offroad/icon_metric.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("CommunityFeaturesToggle",
                                            "커뮤니티 기능 사용",
                                            "커뮤니티기능은 comma.ai에서 공식적으로 지원하는 기능이 아니므로 사용시 주의하세요.",
                                            "../assets/offroad/icon_discord.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("IsLdwEnabled",
                                            "차선이탈 경보 사용",
                                            "40km 이상의 속도로 주행시 방향지시등 조작없이 차선을 이탈하면 차선이탈경보를 보냅니다. (오픈파일럿 비활성상태에서만 사용됩니다)",
                                            "../assets/offroad/icon_ldws.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("AutoLaneChangeEnabled",
                                            "자동차선변경 사용",
                                            "60km 이상의 속도로 주행시 방향지시등을 켜면 3초후에 차선변경을 수행합니다. 이 기능은 안전을위해 후측방감지기능이 있는 차량만 사용하시기바랍니다.",
                                            "../assets/offroad/icon_lca.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
//  toggles_list->addWidget(new LabelControl("이 설정은 둘중하나만 선택가능합니다", ""));
  toggles_list->addWidget(new ParamControl("MadModeEnabled",
                                            "MAD 모드 사용",
                                            "이 기능은 크루즈버튼으로 오픈파일럿이 활성화됩니다.",
                                            "../assets/offroad/icon_warning.png"
                                              ));
/*
  toggles_list->addWidget(new ParamControl("LongControlEnabled",
                                            "Long Control 사용",
                                            "이 기능은 오픈파일럿이 속도를 컨트롤하기때문에 사용시 주의하세요.",
                                            "../assets/offroad/icon_long.png"
                                              ));                                              
  toggles_list->addWidget(horizontal_line());  
  toggles_list->addWidget(new ParamControl("EndToEndToggle",
                                           "Kale 모드 (Alpha)",
                                           "오픈파일럿이 차선없이 운전자가 운전하는것처럼 주행합니다.",
                                           "../assets/offroad/icon_kale.png"));
*/                                           
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

  // offroad-only buttons
  QList<ButtonControl*> offroad_btns;

  offroad_btns.append(new ButtonControl("운전자 모니터링", "미리보기",
                                        "정확한 운전자 모니터링 환경을 위해 운전자 모니터링 카메라를 미리보고 최적의 장착위치를 찾아보세요.", [=]() {
                                          Params().write_db_value("IsDriverViewEnabled", "1", 1); }));

  offroad_btns.append(new ButtonControl("캘리브레이션", "초기화",
                                        "오픈파일럿은 마운트에 장착시 위와 아래쪽은 5˚이내 그리고 왼쪽과 오른쪽은 4˚이내에 장착해야 합니다. 장착위치가 변경되면 캘리브레이션을 진행하세요.", [=]() {
    if (ConfirmationDialog::confirm("초기화하시겠습니까?")) {
      Params().delete_db_value("CalibrationParams");
    }
  }));

  offroad_btns.append(new ButtonControl("트레이닝 가이드", "다시보기",
                                        "오픈파일럿 트레이닝 가이드를 다시보기합니다", [=]() {
    if (ConfirmationDialog::confirm("다시보시겠습니까?")) {
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
  reboot_btn->setStyleSheet("color: white;"
                            "background-color: #2CE22C;");
  power_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("재부팅하시겠습니까?")) {
      Hardware::reboot();
    }
  });

  QPushButton *poweroff_btn = new QPushButton("종료");
  poweroff_btn->setStyleSheet("color: white;"
                              "background-color: #E22C2C;");
  power_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("종료하시겠습니까?")) {
      Hardware::poweroff();
    }
  });

  device_layout->addLayout(power_layout);

  setLayout(device_layout);
  setStyleSheet(R"(
    QPushButton {
      font-weight: 500;    
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
    {"버전", brand + " v" + params.get("Version", false).substr(0, 14)},
    {"Git Remote", params.get("GitRemote", false)},
    {"Git Branch", params.get("GitBranch", false)},
    {"Git Commit", params.get("GitCommit", false).substr(0, 7)},
    {"판다 펌웨어", params.get("PandaFirmwareHex", false)},
    {"시리얼", params.get("HardwareSerial", false)},    
    {"OS 버전", Hardware::get_os_version()},
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

  // Wifi Setting
  layout->addWidget(new ButtonControl("WiFi 설정", "열기", "",
                                      [=]() { HardwareEon::launch_wifi(); }));
  layout->addWidget(horizontal_line());

  // Android Setting
  layout->addWidget(new ButtonControl("안드로이드 설정", "열기", "",
                                      [=]() { HardwareEon::launch_setting(); }));
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
  layout->addWidget(new LateralControlSelect());
  layout->addWidget(horizontal_line());
//  layout->addWidget(new LabelControl("차량의 MFC 카메라의 종류를 선택하세요", ""));
  layout->addWidget(new MfcSelect());
  layout->addWidget(horizontal_line()); 
  layout->addWidget(new PrebuiltToggle());
  layout->addWidget(horizontal_line());
  const char* gitpull = "/data/openpilot/gitpull.sh ''";
  layout->addWidget(new ButtonControl("Git Pull", "실행", "사용중인 브랜치의 최근 수정된 내용으로 변경됩니다.", [=]() {
                                        if (ConfirmationDialog::confirm("진행하시겠습니까?")){
                                          std::system(gitpull);
                                        }
                                      }));
  layout->addWidget(horizontal_line());
  const char* addfunc = "/data/openpilot/addfunc.sh ''";
  layout->addWidget(new ButtonControl("추가 기능", "실행", "각종 추가기능을 설정합니다", [=]() {
                                        if (ConfirmationDialog::confirm("진행하시겠습니까?")) {
                                          std::system(addfunc);
                                        }
                                      }));
  layout->addWidget(horizontal_line());
  const char* panda_flash = "/data/openpilot/panda/board/flash.sh ''";
  layout->addWidget(new ButtonControl("판다 펌웨어 플래싱", "실행", "판다 펌웨어 플래싱을 실행합니다.", [=]() {
                                        if (ConfirmationDialog::confirm("진행하시겠습니까?")) {
                                          std::system(panda_flash);
                                        }
                                      }));
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
    {"정보", new DeveloperPanel()},
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
