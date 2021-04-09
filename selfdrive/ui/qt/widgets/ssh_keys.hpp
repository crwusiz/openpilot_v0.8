#pragma once

#include <QTimer>
#include <QPushButton>
#include <QNetworkAccessManager>

#include "widgets/controls.hpp"
#include "selfdrive/hardware/hw.h"

// SSH enable toggle
class SshToggle : public ToggleControl {
  Q_OBJECT

public:
  SshToggle() : ToggleControl("SSH 사용", "", "", Hardware::get_ssh_enabled()) {
    QObject::connect(this, &SshToggle::toggleFlipped, [=](bool state) {
      Hardware::set_ssh_enabled(state);
    });
  }
};

// prebuilt
class PrebuiltToggle : public ToggleControl {
  Q_OBJECT

public:
  PrebuiltToggle() : ToggleControl("Prebuilt 파일 생성", "Prebuilt 파일을 생성하며 부팅속도를 단축시킵니다. UI수정을 한 경우 기능을 끄십시오.", "../assets/offroad/icon_shell.png", Params().read_db_bool("PutPrebuiltOn")) {
    QObject::connect(this, &PrebuiltToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("PutPrebuiltOn", &value, 1);
    });
  }
};

// ldws MFC
class LDWSToggle : public ToggleControl {
  Q_OBJECT

public:
  LDWSToggle() : ToggleControl("LDWS 차량 설정", "", "../assets/offroad/icon_shell.png", Params().read_db_bool("LdwsCarFix")) {
    QObject::connect(this, &LDWSToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("LdwsCarFix", &value, 1);
    });
  }
};

// 조향제어
class LateralControl : public AbstractControl {
  Q_OBJECT

public:
  LateralControl();

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;

  void refresh();
};

// SSH key management widget
class SshControl : public AbstractControl {
  Q_OBJECT

public:
  SshControl();

private:
  QPushButton btn;
  QString username;
  QLabel username_label;

  // networking
  QTimer* networkTimer;
  QNetworkReply* reply;
  QNetworkAccessManager* manager;

  void refresh();
  void getUserKeys(QString username);

signals:
  void failedResponse(QString errorString);

private slots:
  void timeout();
  void parseResponse();
};
