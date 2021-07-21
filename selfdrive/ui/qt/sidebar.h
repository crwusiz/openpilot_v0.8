#pragma once

#include <QFrame>
#include <QMap>

#include "selfdrive/ui/ui.h"

class Sidebar : public QFrame {
  Q_OBJECT
  Q_PROPERTY(QString connectStr MEMBER connect_str NOTIFY valueChanged);
  Q_PROPERTY(QColor connectStatus MEMBER connect_status NOTIFY valueChanged);
  Q_PROPERTY(QString pandaStr MEMBER panda_str NOTIFY valueChanged);
  Q_PROPERTY(QColor pandaStatus MEMBER panda_status NOTIFY valueChanged);
  Q_PROPERTY(int tempVal MEMBER temp_val NOTIFY valueChanged);
  Q_PROPERTY(QColor tempStatus MEMBER temp_status NOTIFY valueChanged);
  Q_PROPERTY(QString netType MEMBER net_type NOTIFY valueChanged);
  Q_PROPERTY(int netStrength MEMBER net_strength NOTIFY valueChanged);
  Q_PROPERTY(QString wifiAddr MEMBER wifi_addr NOTIFY valueChanged);

public:
  explicit Sidebar(QWidget* parent = 0);

signals:
  void openSettings();
  void valueChanged();

public slots:
  void updateState(const UIState &s);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

private:
  void drawMetric(QPainter &p, const QString &label, const QString &val, QColor c, int y);

  QImage home_img, settings_img;
  const QMap<cereal::DeviceState::NetworkType, QString> network_type = {
    {cereal::DeviceState::NetworkType::NONE, "────────"},
    {cereal::DeviceState::NetworkType::WIFI, "WiFi"},
    {cereal::DeviceState::NetworkType::ETHERNET, "ETH"},
    {cereal::DeviceState::NetworkType::CELL2_G, "2G"},
    {cereal::DeviceState::NetworkType::CELL3_G, "3G"},
    {cereal::DeviceState::NetworkType::CELL4_G, "LTE"},
    {cereal::DeviceState::NetworkType::CELL5_G, "5G"}
  };

  const QRect settings_btn = QRect(50, 35, 200, 117);
  const QColor good_color = QColor(255, 255, 255);
  const QColor warning_color = QColor(218, 202, 37);
  const QColor danger_color = QColor(201, 34, 49);

  QString connect_str = "오프라인";
  QColor connect_status = warning_color;
  QString panda_str = "차량\n연결안됨";
  QColor panda_status = warning_color;
  int temp_val = 0;
  QColor temp_status = warning_color;
  QString net_type;
  int net_strength = 0;

  QString wifi_addr = "────────";

  const QMap<int, QImage> battery_imgs = {
    {0, QImage("../assets/images/battery.png")},
    {1, QImage("../assets/images/battery_charging.png")},
  };
  int m_batteryPercent = 0;
  int m_battery_img;
};
