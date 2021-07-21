#include "selfdrive/ui/qt/sidebar.h"

#include <QMouseEvent>

#include "selfdrive/ui/qt/qt_window.h"
#include "selfdrive/common/util.h"
#include "selfdrive/hardware/hw.h"
#include "selfdrive/ui/qt/util.h"

void Sidebar::drawMetric(QPainter &p, const QString &label, const QString &val, QColor c, int y) {
  const QRect rect = {30, y, 240, val.isEmpty() ? (label.contains("\n") ? 124 : 100) : 148};

  p.setPen(Qt::NoPen);
  p.setBrush(QBrush(c));
  p.setClipRect(rect.x() + 6, rect.y(), 18, rect.height(), Qt::ClipOperation::ReplaceClip);
  p.drawRoundedRect(QRect(rect.x() + 6, rect.y() + 6, 100, rect.height() - 12), 10, 10);
  p.setClipping(false);

  QPen pen = QPen(QColor(0xff, 0xff, 0xff, 0x55));
  pen.setWidth(2);
  p.setPen(pen);
  p.setBrush(Qt::NoBrush);
  p.drawRoundedRect(rect, 20, 20);

  p.setPen(QColor(0xff, 0xff, 0xff));
  if (val.isEmpty()) {
    configFont(p, "Open Sans", 35, "Bold");
    const QRect r = QRect(rect.x() + 30, rect.y(), rect.width() - 40, rect.height());
    p.drawText(r, Qt::AlignCenter, label);
  } else {
    configFont(p, "Open Sans", 58, "Bold");
    p.drawText(rect.x() + 50, rect.y() + 71, val);
    configFont(p, "Open Sans", 35, "Regular");
    p.drawText(rect.x() + 50, rect.y() + 50 + 77, label);
  }
}

Sidebar::Sidebar(QWidget *parent) : QFrame(parent) {
  home_img = QImage("../assets/offroad/icon_openpilot.png").scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  settings_img = QImage("../assets/images/button_settings.png").scaled(settings_btn.width(), settings_btn.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  connect(this, &Sidebar::valueChanged, [=] { update(); });

  setFixedWidth(300);
  setMinimumHeight(vwp_h);
  setStyleSheet("background-color: rgb(57, 57, 57);");
}

void Sidebar::mousePressEvent(QMouseEvent *event) {
  if (settings_btn.contains(event->pos())) {
    emit openSettings();
  }
}

void Sidebar::updateState(const UIState &s) {
  auto &sm = *(s.sm);

  auto deviceState = sm["deviceState"].getDeviceState();
  setProperty("netType", network_type[deviceState.getNetworkType()]);
  setProperty("wifiAddr", deviceState.getWifiIpAddress().cStr());
  int strength = (int)deviceState.getNetworkStrength();
  setProperty("netStrength", strength > 0 ? strength + 1 : 0);

  auto last_ping = deviceState.getLastAthenaPingTime();
  if (last_ping == 0) {
    setProperty("connectStr", "오프라인");
    setProperty("connectStatus", warning_color);
  } else {
    bool online = nanos_since_boot() - last_ping < 80e9;
    setProperty("connectStr",  online ? "온라인" : "오류");
    setProperty("connectStatus", online ? good_color : danger_color);
  }

  QColor tempStatus = danger_color;
  auto ts = deviceState.getThermalStatus();
  if (ts == cereal::DeviceState::ThermalStatus::GREEN) {
    tempStatus = good_color;
  } else if (ts == cereal::DeviceState::ThermalStatus::YELLOW) {
    tempStatus = warning_color;
  }
  setProperty("tempStatus", tempStatus);
  setProperty("tempVal", (int)deviceState.getAmbientTempC());

  QString pandaStr = "차량\n연결됨";
  QColor pandaStatus = good_color;
  if (s.scene.pandaType == cereal::PandaState::PandaType::UNKNOWN) {
    pandaStatus = danger_color;
    pandaStr = "차량\n연결안됨";
  } else if (s.scene.started && !sm["liveLocationKalman"].getLiveLocationKalman().getGpsOK()) {
    pandaStatus = warning_color;
    pandaStr = "GPS\nSEARCHING";
//  } else if (s.scene.satelliteCount > 0) {
//  	pandaStr = QString("위성수 %1\n정확도 %2").arg(s.scene.satelliteCount).arg(fmin(10, s.scene.gpsAccuracy), 0, 'f', 2);
//    pandaStatus = sm["liveLocationKalman"].getLiveLocationKalman().getGpsOK() ? good_color : warning_color;
  }
  setProperty("pandaStr", pandaStr);
  setProperty("pandaStatus", pandaStatus);

  m_battery_img = s.scene.deviceState.getBatteryStatus() == "Charging" ? 1 : 0;
  m_batteryPercent = s.scene.deviceState.getBatteryPercent();
}

void Sidebar::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setPen(Qt::NoPen);
  p.setRenderHint(QPainter::Antialiasing);

  // static imgs
  p.setOpacity(0.65);
  p.drawImage(settings_btn.x(), settings_btn.y(), settings_img);
  p.setOpacity(1.0);
  p.drawImage(60, 1080 - 180 - 40, home_img);

/*
  // network
  int x = 58;
  const QColor gray(0x54, 0x54, 0x54);
  for (int i = 0; i < 5; ++i) {
    p.setBrush(i < net_strength ? Qt::white : gray);
    p.drawEllipse(x, 196, 27, 27);
    x += 37;
  }
*/

  p.drawImage(68, 180, battery_imgs[m_battery_img]); // signal_imgs to battery_imgs
  configFont(p, "Open Sans", 32, "Bold");
  p.setPen(QColor(0x00, 0x00, 0x00));
  const QRect r = QRect(80, 193, 100, 50);
  char battery_str[5];
  snprintf(battery_str, sizeof(battery_str), "%d%%", m_batteryPercent);
  p.drawText(r, Qt::AlignCenter, battery_str);

  configFont(p, "Open Sans", 30, "Bold");
  p.setPen(QColor(0xff, 0xff, 0xff));
  const QRect r2 = QRect(0, 267, event->rect().width(), 50);
  if(Hardware::EON() && net_type == network_type[cereal::DeviceState::NetworkType::WIFI])
    p.drawText(r2, Qt::AlignCenter, wifi_addr);
  else
    p.drawText(r2, Qt::AlignCenter, net_type);


  // metrics
  drawMetric(p, "시스템온도", QString("%1°C").arg(temp_val), temp_status, 338);
  drawMetric(p, panda_str, "", panda_status, 518);
  drawMetric(p, "네트워크\n" + connect_str, "", connect_status, 676);
}
