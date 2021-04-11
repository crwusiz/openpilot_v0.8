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
  SshToggle() : ToggleControl("SSH 사용", "", "../assets/offroad/icon_ssh.png", Hardware::get_ssh_enabled()) {
    QObject::connect(this, &SshToggle::toggleFlipped, [=](bool state) {
      Hardware::set_ssh_enabled(state);
    });
  }
};

// prebuilt
class PrebuiltToggle : public ToggleControl {
  Q_OBJECT

public:
  PrebuiltToggle() : ToggleControl("Prebuilt 파일 생성", "Prebuilt 파일을 생성하며 부팅속도를 단축시킵니다.", "../assets/offroad/icon_prebuilt.png", Params().read_db_bool("PutPrebuilt")) {
    QObject::connect(this, &PrebuiltToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("PutPrebuilt", &value, 1);
    });
  }
};

/*
// LDWS MFC
class LdwsToggle : public ToggleControl {
  Q_OBJECT

public:
  LdwsToggle() : ToggleControl("LDWS MFC", "", "../assets/offroad/icon_ldwsmfc.png", Params().read_db_bool("LdwsMfc")) {
    QObject::connect(this, &LdwsToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("LdwsMfc", &value, 1);
    });
  }
};

// LFA MFC
class LfaToggle : public ToggleControl {
  Q_OBJECT

public:
  LfaToggle() : ToggleControl("LFA MFC", "", "../assets/offroad/icon_lfamfc.png", Params().read_db_bool("LfaMfc")) {
    QObject::connect(this, &LfaToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("LfaMfc", &value, 1);
    });
  }
};

// MadMode
class MadModeToggle : public ToggleControl {
  Q_OBJECT

public:
  MadModeToggle() : ToggleControl("MAD 모드 사용", "", "../assets/offroad/icon_warning.png", Params().read_db_bool("MadModeEnabled")) {
    QObject::connect(this, &MadModeToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("MadModeEnabled", &value, 1);
    });
  }
};

// LongControl
class LongControlToggle : public ToggleControl {
  Q_OBJECT

public:
  LongControlToggle() : ToggleControl("LongControl 사용", "", "../assets/offroad/icon_long.png", Params().read_db_bool("LongControlEnabled")) {
    QObject::connect(this, &LongControlToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("LongControlEnabled", &value, 1);
    });
  }
};
*/

// MfcSelect
class MfcSelect : public AbstractControl {
  Q_OBJECT

public:
  MfcSelect();

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;

  void refresh();
};

// 조향로직
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
